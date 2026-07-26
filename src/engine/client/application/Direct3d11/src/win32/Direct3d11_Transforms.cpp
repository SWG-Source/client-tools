// ======================================================================
//
// Direct3d11_Transforms.cpp
// copyright (c) 2026 Galaxies Reborn
//
// ======================================================================

#include "FirstDirect3d11.h"
#include "Direct3d11_Transforms.h"

#include "Direct3d11_ConstantBuffers.h"
#include "Direct3d11_LightManager.h"
#include "Direct3d11_Metrics.h"
#include "PaddedVector.h"
#include "clientGraphics/Graphics.def"
#include "clientGraphics/ShaderConstantRegisters.h"
#include "sharedMath/Transform.h"
#include "sharedMath/Vector.h"

// ======================================================================

namespace Direct3d11_TransformsNamespace
{
	// Row-major, sixteen floats, m[(row * 4) + column]. The same byte order D3DXMATRIX has,
	// which is what makes these uploadable without a conversion step.
	float ms_objectToWorld[16];
	float ms_worldToCamera[16];
	float ms_projection[16];

	// projection * worldToCamera, rebuilt whenever either changes. Kept because it is the half
	// of the concatenation that does not depend on the object, and objects outnumber cameras.
	float ms_worldToProjection[16];

	bool  ms_dirty;

	void  multiply(float *result, float const *lhs, float const *rhs);
	void  setIdentityFourthRow(float *matrix);
}

using namespace Direct3d11_TransformsNamespace;

// ======================================================================
/**
 * result = lhs * rhs, row-major, matching D3DXMatrixMultiply exactly.
 *
 * Written through a local so result may alias either operand. The copy is sixteen floats and
 * the compiler removes it where it can; the alternative is a rule every caller has to remember.
 */

void Direct3d11_TransformsNamespace::multiply(float *result, float const *lhs, float const *rhs)
{
	float product[16];

	for (int row = 0; row < 4; ++row)
		for (int column = 0; column < 4; ++column)
		{
			float sum = 0.0f;
			for (int k = 0; k < 4; ++k)
				sum += lhs[(row * 4) + k] * rhs[(k * 4) + column];

			product[(row * 4) + column] = sum;
		}

	memcpy(result, product, sizeof(product));
}

// ----------------------------------------------------------------------

void Direct3d11_TransformsNamespace::setIdentityFourthRow(float *matrix)
{
	matrix[12] = 0.0f;
	matrix[13] = 0.0f;
	matrix[14] = 0.0f;
	matrix[15] = 1.0f;
}

// ======================================================================

void Direct3d11_Transforms::install()
{
	memset(ms_objectToWorld,    0, sizeof(ms_objectToWorld));
	memset(ms_worldToCamera,    0, sizeof(ms_worldToCamera));
	memset(ms_projection,       0, sizeof(ms_projection));
	memset(ms_worldToProjection, 0, sizeof(ms_worldToProjection));

	// The setters below fill twelve floats each and never touch the fourth row, so it is
	// established here and left alone for the life of the process. See the header for why the
	// row must be (0,0,0,1) rather than zero.
	setIdentityFourthRow(ms_objectToWorld);
	setIdentityFourthRow(ms_worldToCamera);

	// The projection matrix is fully written by its setter, including its fourth row, so it
	// gets no such treatment. It stays zero until the engine sets one, which it does before
	// the first draw of every frame.

	ms_dirty = true;
}

// ======================================================================
/**
 * The world-to-camera transform, and the camera's world position.
 *
 * The position goes out as a vertex constant of its own rather than being derived from the
 * matrix: c8 is what the corpus reads for per-vertex view vectors, and DX9 uploads it here.
 */

void Direct3d11_Transforms::setWorldToCameraTransform(Transform const &transform, Vector const &cameraPosition)
{
	Transform::matrix_t const &source = transform.getMatrix();

	for (int row = 0; row < 3; ++row)
		for (int column = 0; column < 4; ++column)
			ms_worldToCamera[(row * 4) + column] = static_cast<float>(source[row][column]);

	// The light manager keeps the camera position in world space to build the object-space
	// view vector in the dot3 block. DX9 calls this from the same setter.
	Direct3d11_LightManager::setCameraPosition(cameraPosition);

	PaddedVector const paddedPosition(cameraPosition);
	Direct3d11_ConstantBuffers::setVertexShaderConstants(VSCR_cameraPosition, &paddedPosition, 1);

	// Reversed relative to the fixed-function order. See the header.
	multiply(ms_worldToProjection, ms_projection, ms_worldToCamera);

	// DX9 marks only the object transform dirty, and gets away with it because every draw sets
	// an object transform first. Marking it here as well costs one 128-byte write per camera
	// change -- once or twice a frame, not once a draw -- and removes a whole class of stale
	// matrix bug for anything that ever draws without setting a transform.
	ms_dirty = true;

	++Direct3d11_Metrics::setTransformCalls;
}

// ----------------------------------------------------------------------
/**
 * The projection matrix, copied straight across.
 *
 * Both this and the camera setter rebuild worldToProjection, so the engine may set them in
 * either order. DX9 does the same, for the same reason.
 */

void Direct3d11_Transforms::setProjectionMatrix(GlMatrix4x4 const &projectionMatrix)
{
	for (int row = 0; row < 4; ++row)
		for (int column = 0; column < 4; ++column)
			ms_projection[(row * 4) + column] = projectionMatrix.matrix[row][column];

	multiply(ms_worldToProjection, ms_projection, ms_worldToCamera);

	ms_dirty = true;

	++Direct3d11_Metrics::setTransformCalls;
}

// ----------------------------------------------------------------------
/**
 * The object-to-world transform, with a non-uniform scale folded in.
 *
 * Each row's three rotation components are scaled by that row's axis; the fourth column, the
 * translation, is copied through unscaled.
 */

void Direct3d11_Transforms::setObjectToWorldTransformAndScale(Transform const &objectToWorld, Vector const &scale)
{
	Transform::matrix_t const &source = objectToWorld.getMatrix();

	float const axisScale[3] = { scale.x, scale.y, scale.z };

	for (int row = 0; row < 3; ++row)
	{
		ms_objectToWorld[(row * 4) + 0] = static_cast<float>(source[row][0]) * axisScale[row];
		ms_objectToWorld[(row * 4) + 1] = static_cast<float>(source[row][1]) * axisScale[row];
		ms_objectToWorld[(row * 4) + 2] = static_cast<float>(source[row][2]) * axisScale[row];
		ms_objectToWorld[(row * 4) + 3] = static_cast<float>(source[row][3]);
	}

	// The light manager needs the unscaled transform, not the matrix built above: it rotates
	// world-space light directions into object space, and a non-uniform scale would skew them.
	Direct3d11_LightManager::setObjectToWorldTransform(objectToWorld);

	ms_dirty = true;

	++Direct3d11_Metrics::setTransformCalls;
}

// ======================================================================

void Direct3d11_Transforms::flush()
{
	if (!ms_dirty)
		return;

	float objectWorldCameraProjection[16];
	multiply(objectWorldCameraProjection, ms_worldToProjection, ms_objectToWorld);

	Direct3d11_ConstantBuffers::setPerObjectTransforms(objectWorldCameraProjection, ms_objectToWorld);

	ms_dirty = false;
}

// ======================================================================

float const *Direct3d11_Transforms::getObjectToWorldMatrix()
{
	return ms_objectToWorld;
}

// ----------------------------------------------------------------------

float const *Direct3d11_Transforms::getWorldToCameraMatrix()
{
	return ms_worldToCamera;
}

// ----------------------------------------------------------------------

float const *Direct3d11_Transforms::getWorldToProjectionMatrix()
{
	return ms_worldToProjection;
}

// ======================================================================
