// ======================================================================
//
// ClientProceduralTerrainAppearance_LevelOfDetail.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ClientProceduralTerrainAppearance_LevelOfDetail_H
#define INCLUDED_ClientProceduralTerrainAppearance_LevelOfDetail_H

// ======================================================================

#include "clientTerrain/ClientProceduralTerrainAppearance.h"
#include "sharedTerrain/TerrainQuadTree.h"

#include <vector>

class Object;
class Volume;

// ======================================================================

struct ClientProceduralTerrainAppearance::LevelOfDetail
{
	explicit LevelOfDetail (ClientProceduralTerrainAppearance& cpta);
	~LevelOfDetail ();

	void          removeAllObjectsFromWorld (TerrainQuadTree::Node* node) const;
	bool          selectActualLevelOfDetail (const Camera* camera, const Object* referenceObject, const Volume* frustum, TerrainQuadTree::Node* node);

	void          setUseHeightBias (bool useHeightBias);
	bool          getUseHeightBias () const;
	void          setThreshold (float threshold);
	static float  getThreshold ();
	void          setHeightBiasMax (int heightBiasMax);
	int           getHeightBiasMax () const;
	void          setHeightBiasFactor (float heightBiasFactor);
	float         getHeightBiasFactor () const;
	void          setForceHighThreshold (float forceHighThreshold);
	static float  getForceHighThreshold ();
	void          setDirty (bool dirty);
	bool          isDirty () const;
	const ChunkRequestInfoList& getChunkRequestInfoList () const;

private:

	LevelOfDetail ();
	LevelOfDetail (const LevelOfDetail &);
	LevelOfDetail & operator= (const LevelOfDetail &);

	int           requestSubNodeChunks (const TerrainQuadTree::Node & node, int priority);
	bool          attemptSplit (TerrainQuadTree::Node * nw);
	bool          processSplitList ();
	int           computeDesiredLevelOfDetail (TerrainQuadTree::Node & snode, float & closeness) const;

private:

	typedef ClientProceduralTerrainAppearance::ClientChunk ClientChunk;
	typedef std::vector<TerrainQuadTree::Node *> SplitList;

	ClientProceduralTerrainAppearance& m_cpta;
	ChunkRequestInfoList* m_chunkRequestInfoList;
	const Object*    m_referenceObject;
	const Volume*    m_frustum;
	bool             m_dirty;
	float            m_buildRadiusSquared;
	SplitList        m_currentSplitList;
	SplitList        m_nextSplitList;
	float            m_thresholdInternal;
	int              m_heightBiasThresholdInternal;
	float            m_forceHighRange;
	float            m_forceHighRangeSquared;

	static bool      ms_useHeightBias;
	static float     ms_forceHighThreshold;
	static float     ms_forceHighRange;
	static float     ms_forceHighRangeSquared;
	static float     ms_threshold;
	static float     ms_heightBiasFactor;
	static int       ms_heightBiasMax;
};

// ======================================================================

#endif

