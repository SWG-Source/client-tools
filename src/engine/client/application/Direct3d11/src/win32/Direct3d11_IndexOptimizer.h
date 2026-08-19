// ======================================================================
//
// Direct3d11_IndexOptimizer.h
// copyright (c) 2026 Galaxies Reborn
//
// Reorder a triangle list so the GPU's post-transform vertex cache hits more often.
//
// ----------------------------------------------------------------------
// Why this is written out rather than called
//
// DX9 does this with D3DX: it builds a throwaway ID3DXMesh, copies the indices in, calls
// OptimizeInplace with D3DXMESHOPT_VERTEXCACHE, and copies them back
// (Direct3d9.cpp's optimizeIndexBuffer). D3DX does not exist for D3D11 and was deprecated
// before it shipped, so there is nothing to call.
//
// That turns out not to matter, because the operation has nothing to do with the graphics API.
// It permutes an array of uint16 in place. No device, no resource, no driver. So this is the
// algorithm itself -- Tom Forsyth's linear-speed vertex cache optimisation, which is the
// standard answer and the same family of heuristic D3DX used.
//
// The one caller is SoftwareBlendSkeletalShaderPrimitive, on skinned meshes, once when the
// primitive is built. It is not in a frame path.
//
// ----------------------------------------------------------------------
// What it optimises for
//
// A GPU keeps a small cache of recently transformed vertices. A triangle whose vertices are
// all still in that cache costs no vertex shading at all. Index order decides how often that
// happens, and a mesh straight out of an exporter is usually ordered by material or by
// authoring convenience rather than by locality.
//
// The cache is modelled at 32 entries, which is the usual choice: it is larger than the real
// FIFO on most hardware, and over-estimating degrades gracefully where under-estimating leaves
// wins on the table. The scoring constants are Forsyth's published ones.
//
// The output is always a permutation of the input -- the same triangles, reordered, with each
// triangle's own winding preserved. That is asserted in debug builds, because a bug here would
// silently corrupt geometry rather than merely make it slower.
//
// ======================================================================

#ifndef INCLUDED_Direct3d11_IndexOptimizer_H
#define INCLUDED_Direct3d11_IndexOptimizer_H

// ======================================================================

class Direct3d11_IndexOptimizer
{
public:
	// Reorder in place. indexCount must be a multiple of three; anything else is refused and
	// left untouched, which is what DX9's DEBUG_FATAL on the same condition amounts to in a
	// build where that compiles out.
	static void optimize(uint16 *indices, int indexCount);

private:
	Direct3d11_IndexOptimizer();
	Direct3d11_IndexOptimizer(Direct3d11_IndexOptimizer const &);
	Direct3d11_IndexOptimizer &operator=(Direct3d11_IndexOptimizer const &);
};

// ======================================================================

#endif
