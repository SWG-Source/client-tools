// ======================================================================
//
// Direct3d11_Transforms.h
// copyright (c) 2026 Galaxies Reborn
//
// The camera, projection and object transforms, concatenated into the two matrices every
// vertex program reads from c0 and c4.
//
// ----------------------------------------------------------------------
// Why this is a file and not four setters
//
// Nothing renders without it. c0 holds objectWorldCameraProjection, and every vertex program
// in the corpus begins by dotting the incoming position against c0..c3. Until this class runs,
// c0 is zero and every vertex collapses to the origin -- so this is a harder prerequisite than
// textures, lighting or blending, all of which merely look wrong when absent.
//
// The three sources arrive independently and at different rates: the projection once per
// viewport change, the camera once per frame per camera, the object transform once per object.
// Concatenating on arrival would multiply three matrices per object; concatenating lazily in
// prepareToDraw multiplies one. DX9 defers for the same reason, and this mirrors its structure
// so the two can be compared setter for setter.
//
// ----------------------------------------------------------------------
// No transposes, and why that is correct rather than lucky
//
// D3D9's fixed-function pipeline wanted matrices in one layout and its vertex shaders wanted
// the transpose, which is why Direct3d9.cpp carries an #ifdef FFP fork through every one of
// these functions and calls D3DXMatrixMultiplyTranspose on the FFP side. This backend is
// vertex-and-pixel-shader only, so only the VSPS half of each fork is ported -- untransposed
// throughout.
//
// That layout survives into D3D11 untouched. The engine uploads rows, c0 being row 1, and the
// programs read them with dp4, which is a row dotted against the vertex. HLSL's default
// column-major packing plus D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY places register cN at
// $Globals byte offset 16N, so the bytes the engine writes are the bytes the program reads.
// Adding a transpose here, or compiling with PACK_MATRIX_ROW_MAJOR, would break it.
//
// ----------------------------------------------------------------------
// The three details that are easy to get wrong
//
//   The fourth row is written once, at install, and never again. DX9's VSPS converters fill
//   twelve floats and leave the last row alone -- the assignments are in the source, commented
//   out, with the initialisation hoisted to install (Direct3d9.cpp:1557-1568). It is load
//   bearing: the row must be (0,0,0,1), because the projection matrix's z bias sits in its
//   fourth column and a zero fourth row would multiply that bias away. Leaving the row zeroed
//   produces a scene that transforms but never resolves depth correctly.
//
//   The VSPS multiply order is the reverse of the FFP one. worldToProjection is
//   projection * worldToCamera, not worldToCamera * projection (Direct3d9.cpp:3233-3235 forks
//   on exactly this). Math convention, M*v, because that is what dp4-against-rows means.
//
//   Scale does not touch translation. convertScaleAndTransformToMatrix scales the three
//   rotation columns of each row and copies the fourth straight through, so an object's
//   position is in world units regardless of its scale.
//
// ======================================================================

#ifndef INCLUDED_Direct3d11_Transforms_H
#define INCLUDED_Direct3d11_Transforms_H

// ======================================================================

class Transform;
class Vector;
struct GlMatrix4x4;

// ======================================================================

class Direct3d11_Transforms
{
public:
	static void install();

	// The three engine entry points, one per source matrix.
	static void setWorldToCameraTransform(Transform const &transform, Vector const &cameraPosition);
	static void setProjectionMatrix(GlMatrix4x4 const &projectionMatrix);
	static void setObjectToWorldTransformAndScale(Transform const &objectToWorld, Vector const &scale);

	// Concatenate and upload, if anything changed since the last draw. Called from
	// prepareToDraw before the constant buffers flush, so the per-object slice it writes goes
	// out in the same map.
	static void flush();

	// Row-major, sixteen floats. Exposed for the light manager, which needs the object's
	// world placement, and for anything later that has to place a point in clip space without
	// going through a draw.
	static float const *getObjectToWorldMatrix();
	static float const *getWorldToCameraMatrix();
	static float const *getWorldToProjectionMatrix();
};

// ======================================================================

#endif
