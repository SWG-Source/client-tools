// ======================================================================
//
// Direct3d11_IndexOptimizer.cpp
// copyright (c) 2026 Galaxies Reborn
//
// ======================================================================

#include "FirstDirect3d11.h"
#include "Direct3d11_IndexOptimizer.h"

#include <cmath>
#include <vector>

// ======================================================================

namespace Direct3d11_IndexOptimizerNamespace
{
	// Forsyth's published constants. The cache size is modelled larger than most real
	// post-transform FIFOs on purpose: over-estimating loses a little, under-estimating leaves
	// hits unclaimed.
	int   const cms_cacheSize         = 32;
	float const cms_lastTriangleScore = 0.75f;
	float const cms_cacheDecayPower   = 1.5f;
	float const cms_valenceBoostScale = 2.0f;
	float const cms_valenceBoostPower = 0.5f;

	// Below this there is nothing to reorder -- one triangle has no ordering, and two share at
	// most an edge either way.
	int   const cms_minimumTriangles  = 3;

	float findVertexScore(int activeTriangles, int cachePosition);
}

using namespace Direct3d11_IndexOptimizerNamespace;

// ======================================================================
/**
 * How much a vertex is worth having in the cache.
 *
 * Two terms. The cache term rewards a vertex that is already resident, most strongly for the
 * three that the immediately preceding triangle used. The valence term rewards a vertex with
 * few triangles left, which pulls the algorithm towards finishing regions instead of leaving a
 * scatter of stragglers whose neighbours have all been emitted.
 */

float Direct3d11_IndexOptimizerNamespace::findVertexScore(int activeTriangles, int cachePosition)
{
	// Every triangle using it has been emitted; it must never be chosen again.
	if (activeTriangles <= 0)
		return -1.0f;

	float score = 0.0f;

	if (cachePosition >= 0)
	{
		if (cachePosition < 3)
		{
			// The last triangle's own vertices, all scored equally so that the choice between
			// them falls to valence rather than to the order they happened to be listed in.
			score = cms_lastTriangleScore;
		}
		else
		{
			float const scaler = 1.0f / static_cast<float>(cms_cacheSize - 3);
			score = 1.0f - (static_cast<float>(cachePosition - 3) * scaler);
			score = powf(score, cms_cacheDecayPower);
		}
	}

	score += cms_valenceBoostScale * powf(static_cast<float>(activeTriangles), -cms_valenceBoostPower);
	return score;
}

// ======================================================================

void Direct3d11_IndexOptimizer::optimize(uint16 *indices, int indexCount)
{
	if (!indices || indexCount <= 0)
		return;

	if ((indexCount % 3) != 0)
	{
		WARNING(true, ("Direct3d11: an index buffer of %d indices was handed to the vertex cache optimiser, which is not a whole number of triangles. Left unchanged.", indexCount));
		return;
	}

	int const triangleCount = indexCount / 3;
	if (triangleCount < cms_minimumTriangles)
		return;

	// ------------------------------------------------------------------
	// Per-vertex triangle lists, built with a counting pass so the whole thing is two flat
	// allocations rather than a vector per vertex.

	int vertexCount = 0;
	for (int i = 0; i < indexCount; ++i)
		if (static_cast<int>(indices[i]) >= vertexCount)
			vertexCount = static_cast<int>(indices[i]) + 1;

	std::vector<int> triangleListOffset(static_cast<size_t>(vertexCount) + 1, 0);
	for (int i = 0; i < indexCount; ++i)
		++triangleListOffset[static_cast<size_t>(indices[i]) + 1];

	for (int v = 0; v < vertexCount; ++v)
		triangleListOffset[static_cast<size_t>(v) + 1] += triangleListOffset[static_cast<size_t>(v)];

	std::vector<int> triangleList(static_cast<size_t>(indexCount), 0);
	{
		std::vector<int> cursor(triangleListOffset.begin(), triangleListOffset.end() - 1);
		for (int t = 0; t < triangleCount; ++t)
			for (int corner = 0; corner < 3; ++corner)
			{
				int const vertex = static_cast<int>(indices[(t * 3) + corner]);
				triangleList[static_cast<size_t>(cursor[static_cast<size_t>(vertex)]++)] = t;
			}
	}

	// ------------------------------------------------------------------
	// Live state: how many triangles each vertex still has, where it sits in the modelled
	// cache, and its current score.

	std::vector<int>   activeTriangles(static_cast<size_t>(vertexCount), 0);
	std::vector<int>   cachePosition(static_cast<size_t>(vertexCount), -1);
	std::vector<float> vertexScore(static_cast<size_t>(vertexCount), 0.0f);

	for (int v = 0; v < vertexCount; ++v)
	{
		activeTriangles[static_cast<size_t>(v)] = triangleListOffset[static_cast<size_t>(v) + 1] - triangleListOffset[static_cast<size_t>(v)];
		vertexScore[static_cast<size_t>(v)] = findVertexScore(activeTriangles[static_cast<size_t>(v)], -1);
	}

	std::vector<float> triangleScore(static_cast<size_t>(triangleCount), 0.0f);
	std::vector<bool>  triangleEmitted(static_cast<size_t>(triangleCount), false);

	for (int t = 0; t < triangleCount; ++t)
		triangleScore[static_cast<size_t>(t)] =
			vertexScore[static_cast<size_t>(indices[(t * 3) + 0])] +
			vertexScore[static_cast<size_t>(indices[(t * 3) + 1])] +
			vertexScore[static_cast<size_t>(indices[(t * 3) + 2])];

	// Two buffers, swapped, so the per-triangle cache rebuild below does not allocate.
	std::vector<int> cache;
	std::vector<int> nextCache;
	cache.reserve(static_cast<size_t>(cms_cacheSize) + 4);
	nextCache.reserve(static_cast<size_t>(cms_cacheSize) + 4);

	std::vector<uint16> output(static_cast<size_t>(indexCount), 0);

	// ------------------------------------------------------------------
	// The greedy loop: take the best triangle adjacent to a cached vertex, and when there is
	// none, walk forward with a cursor that never goes backwards. That cursor is what keeps the
	// whole thing linear rather than quadratic.
	//
	// Forsyth's paper adds a dead-end stack here, resuming from recently abandoned candidates
	// instead of the cursor. It is a refinement on the fallback only -- it cannot change which
	// triangle is chosen while the cache has any unemitted neighbour, which is the great
	// majority of steps -- and it is left out rather than guessed at.

	int scanCursor = 0;
	int outputPosition = 0;
	int best = -1;

	for (int emitted = 0; emitted < triangleCount; ++emitted)
	{
		while (best < 0)
		{
			// Guaranteed to terminate: emitted < triangleCount means at least one is unemitted,
			// and the cursor only ever advances past emitted ones.
			if (!triangleEmitted[static_cast<size_t>(scanCursor)])
				best = scanCursor;
			else
				++scanCursor;
		}

		triangleEmitted[static_cast<size_t>(best)] = true;

		int const corner0 = static_cast<int>(indices[(best * 3) + 0]);
		int const corner1 = static_cast<int>(indices[(best * 3) + 1]);
		int const corner2 = static_cast<int>(indices[(best * 3) + 2]);

		output[static_cast<size_t>(outputPosition++)] = static_cast<uint16>(corner0);
		output[static_cast<size_t>(outputPosition++)] = static_cast<uint16>(corner1);
		output[static_cast<size_t>(outputPosition++)] = static_cast<uint16>(corner2);

		--activeTriangles[static_cast<size_t>(corner0)];
		--activeTriangles[static_cast<size_t>(corner1)];
		--activeTriangles[static_cast<size_t>(corner2)];

		// Rebuild the cache with this triangle's vertices at the front, followed by the previous
		// contents minus those three.
		//
		// Rebuilt in one pass rather than three erase-and-insert steps. An erase shifts every
		// entry after it, which invalidates the positions recorded in cachePosition, and the
		// next corner would then remove the wrong entry -- a corruption that produces valid
		// geometry in a worse order, so nothing would ever report it.
		//
		// The duplicate checks are not defensive: a degenerate triangle with two equal corners
		// is legal in an index buffer and would otherwise be entered twice.

		nextCache.clear();
		nextCache.push_back(corner0);
		if (corner1 != corner0)
			nextCache.push_back(corner1);
		if (corner2 != corner0 && corner2 != corner1)
			nextCache.push_back(corner2);

		for (size_t i = 0; i < cache.size(); ++i)
		{
			int const vertex = cache[i];
			if (vertex != corner0 && vertex != corner1 && vertex != corner2)
				nextCache.push_back(vertex);
		}

		if (static_cast<int>(nextCache.size()) > cms_cacheSize)
			nextCache.resize(static_cast<size_t>(cms_cacheSize));

		// Everything that was resident is unknown again until the new positions are written, so
		// a vertex that fell out this step is left correctly at -1.
		for (size_t i = 0; i < cache.size(); ++i)
			cachePosition[static_cast<size_t>(cache[i])] = -1;

		cache.swap(nextCache);

		for (size_t i = 0; i < cache.size(); ++i)
			cachePosition[static_cast<size_t>(cache[i])] = static_cast<int>(i);

		// Rescore every vertex now in the cache, and every unemitted triangle touching one.
		// This is the only place scores change, and the work is bounded by the cache size and
		// the local valence rather than by the mesh.
		best = -1;
		float bestScore = -1.0f;

		for (size_t i = 0; i < cache.size(); ++i)
		{
			int const vertex = cache[i];

			float const newScore = findVertexScore(activeTriangles[static_cast<size_t>(vertex)], cachePosition[static_cast<size_t>(vertex)]);
			float const delta = newScore - vertexScore[static_cast<size_t>(vertex)];
			vertexScore[static_cast<size_t>(vertex)] = newScore;

			int const begin = triangleListOffset[static_cast<size_t>(vertex)];
			int const end   = triangleListOffset[static_cast<size_t>(vertex) + 1];

			for (int j = begin; j < end; ++j)
			{
				int const triangle = triangleList[static_cast<size_t>(j)];
				if (triangleEmitted[static_cast<size_t>(triangle)])
					continue;

				triangleScore[static_cast<size_t>(triangle)] += delta;

				if (triangleScore[static_cast<size_t>(triangle)] > bestScore)
				{
					bestScore = triangleScore[static_cast<size_t>(triangle)];
					best = triangle;
				}
			}
		}
	}

	DEBUG_FATAL(outputPosition != indexCount, ("Direct3d11: the vertex cache optimiser emitted %d of %d indices.", outputPosition, indexCount));

	memcpy(indices, &output[0], static_cast<size_t>(indexCount) * sizeof(uint16));
}

// ======================================================================
