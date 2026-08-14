// ======================================================================
//
// SseMath.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================
//
// Rewritten from x86 inline assembly to portable SSE intrinsics so the
// file builds on both x86 and x64 (MSVC dropped inline asm in x64).
// The algorithms are unchanged - each former __asm block has been
// translated 1:1 to the matching <xmmintrin.h> / <intrin.h> intrinsic.

#include "sharedMath/FirstSharedMath.h"
#include "sharedMath/SseMath.h"

#include "sharedMath/Transform.h"
#include "sharedMath/Vector.h"

#include <intrin.h>	   // __cpuid
#include <xmmintrin.h> // SSE intrinsics

// ======================================================================

#define SSE_ALIGN  __declspec(align(16))
#define SSE_VARIABLE_COUNT 5

// ======================================================================

namespace
{
	SSE_ALIGN float sseVariable[SSE_VARIABLE_COUNT][4];
}

// ======================================================================
/**
 * Retrieve whether the hardware can do SSE math.
 *
 * @return  true if SSE math processing is available; false otherwise.
 */

bool SseMath::canDoSseMath()
{
	// Was: __asm { mov eax, 1; cpuid; mov featureBits, edx } wrapped in
	// a try/catch so an illegal-instruction trap on pre-CPUID hardware
	// could be caught. CPUID has been universal on Windows-supported
	// hardware for many years, and __cpuid() is the portable intrinsic
	// equivalent.
	int cpuInfo[4] = {0, 0, 0, 0};
	__cpuid(cpuInfo, 1);
	const unsigned int featureBits = static_cast<unsigned int>(cpuInfo[3]); // edx

	const bool cpuHasSse = ((featureBits & 0x02000000u) != 0);
	const bool cpuHasSaveRestore = ((featureBits & 0x01000000u) != 0);

	return cpuHasSse && cpuHasSaveRestore;
}

// ======================================================================
//
// Helper: load three rows of a 3x4 affine transform into xmm0/1/2.
// The original asm reads the Transform layout directly via offsets
// 0/16/32. Transform stores its three rows as float[4] (16 bytes each),
// so we can pull them with aligned loads.

namespace
{
	inline void loadTransformRows(const Transform &transform,
								  __m128 &row0, __m128 &row1, __m128 &row2)
	{
		const float *base = reinterpret_cast<const float *>(&transform);
		row0 = _mm_loadu_ps(base + 0);
		row1 = _mm_loadu_ps(base + 4);
		row2 = _mm_loadu_ps(base + 8);
	}

	// Splat a single float across all 4 lanes of an xmm register.
	// Equivalent to: movss xmm6, scale; shufps xmm6, xmm6, 0x00.
	inline __m128 splat4(float v)
	{
		return _mm_set1_ps(v);
	}
} // namespace

// ----------------------------------------------------------------------

Vector SseMath::rotateTranslateScale_l2p(const Transform &transform, const Vector &vector, float scale)
{
	__m128 row0, row1, row2;
	loadTransformRows(transform, row0, row1, row2);

	// Source vector, w=1 for translate.
	__m128 src = _mm_setr_ps(vector.x, vector.y, vector.z, 1.0f);

	const __m128 scaleVec = splat4(scale);

	const __m128 r0 = _mm_mul_ps(_mm_mul_ps(src, row0), scaleVec);
	const __m128 r1 = _mm_mul_ps(_mm_mul_ps(src, row1), scaleVec);
	const __m128 r2 = _mm_mul_ps(_mm_mul_ps(src, row2), scaleVec);

	// Horizontal add, same as original.
	SSE_ALIGN float r0a[4];
	_mm_store_ps(r0a, r0);
	SSE_ALIGN float r1a[4];
	_mm_store_ps(r1a, r1);
	SSE_ALIGN float r2a[4];
	_mm_store_ps(r2a, r2);

	return Vector(
		r0a[0] + r0a[1] + r0a[2] + r0a[3],
		r1a[0] + r1a[1] + r1a[2] + r1a[3],
		r2a[0] + r2a[1] + r2a[2] + r2a[3]);
}

// ----------------------------------------------------------------------

Vector SseMath::rotateScale_l2p(const Transform &transform, const Vector &vector, float scale)
{
	__m128 row0, row1, row2;
	loadTransformRows(transform, row0, row1, row2);

	// w=0 means no translate component.
	__m128 src = _mm_setr_ps(vector.x, vector.y, vector.z, 0.0f);

	const __m128 scaleVec = splat4(scale);

	const __m128 r0 = _mm_mul_ps(_mm_mul_ps(src, row0), scaleVec);
	const __m128 r1 = _mm_mul_ps(_mm_mul_ps(src, row1), scaleVec);
	const __m128 r2 = _mm_mul_ps(_mm_mul_ps(src, row2), scaleVec);

	SSE_ALIGN float r0a[4];
	_mm_store_ps(r0a, r0);
	SSE_ALIGN float r1a[4];
	_mm_store_ps(r1a, r1);
	SSE_ALIGN float r2a[4];
	_mm_store_ps(r2a, r2);

	// w=0 so the [3] lane is zero; original sums lanes [0..2] only.
	return Vector(
		r0a[0] + r0a[1] + r0a[2],
		r1a[0] + r1a[1] + r1a[2],
		r2a[0] + r2a[1] + r2a[2]);
}

// ----------------------------------------------------------------------

void SseMath::skinPositionNormal_l2p(const Transform &transform, const Vector &sourcePosition, const Vector &sourceNormal, float scale, Vector &destPosition, Vector &destNormal)
{
	__m128 row0, row1, row2;
	loadTransformRows(transform, row0, row1, row2);

	const __m128 scaleVec = splat4(scale);

	// Position: w=1 -> picks up translate column.
	{
		__m128 src = _mm_setr_ps(sourcePosition.x, sourcePosition.y, sourcePosition.z, 1.0f);

		const __m128 r0 = _mm_mul_ps(_mm_mul_ps(src, row0), scaleVec);
		const __m128 r1 = _mm_mul_ps(_mm_mul_ps(src, row1), scaleVec);
		const __m128 r2 = _mm_mul_ps(_mm_mul_ps(src, row2), scaleVec);

		SSE_ALIGN float r0a[4];
		_mm_store_ps(r0a, r0);
		SSE_ALIGN float r1a[4];
		_mm_store_ps(r1a, r1);
		SSE_ALIGN float r2a[4];
		_mm_store_ps(r2a, r2);

		destPosition.x = r0a[0] + r0a[1] + r0a[2] + r0a[3];
		destPosition.y = r1a[0] + r1a[1] + r1a[2] + r1a[3];
		destPosition.z = r2a[0] + r2a[1] + r2a[2] + r2a[3];
	}

	// Normal: original stored 1.0 in the w lane and then summed only
	// the first three lanes - effectively dropping the translate column.
	{
		__m128 src = _mm_setr_ps(sourceNormal.x, sourceNormal.y, sourceNormal.z, 1.0f);

		const __m128 r0 = _mm_mul_ps(_mm_mul_ps(src, row0), scaleVec);
		const __m128 r1 = _mm_mul_ps(_mm_mul_ps(src, row1), scaleVec);
		const __m128 r2 = _mm_mul_ps(_mm_mul_ps(src, row2), scaleVec);

		SSE_ALIGN float r0a[4];
		_mm_store_ps(r0a, r0);
		SSE_ALIGN float r1a[4];
		_mm_store_ps(r1a, r1);
		SSE_ALIGN float r2a[4];
		_mm_store_ps(r2a, r2);

		destNormal.x = r0a[0] + r0a[1] + r0a[2];
		destNormal.y = r1a[0] + r1a[1] + r1a[2];
		destNormal.z = r2a[0] + r2a[1] + r2a[2];
	}
}

// ----------------------------------------------------------------------

void SseMath::skinPositionNormalAdd_l2p(const Transform &transform, const Vector &sourcePosition, const Vector &sourceNormal, float scale, Vector &destPosition, Vector &destNormal)
{
	__m128 row0, row1, row2;
	loadTransformRows(transform, row0, row1, row2);

	const __m128 scaleVec = splat4(scale);

	// Position: accumulate into destPosition (note `+=` vs `=`).
	{
		__m128 src = _mm_setr_ps(sourcePosition.x, sourcePosition.y, sourcePosition.z, 1.0f);

		const __m128 r0 = _mm_mul_ps(_mm_mul_ps(src, row0), scaleVec);
		const __m128 r1 = _mm_mul_ps(_mm_mul_ps(src, row1), scaleVec);
		const __m128 r2 = _mm_mul_ps(_mm_mul_ps(src, row2), scaleVec);

		SSE_ALIGN float r0a[4];
		_mm_store_ps(r0a, r0);
		SSE_ALIGN float r1a[4];
		_mm_store_ps(r1a, r1);
		SSE_ALIGN float r2a[4];
		_mm_store_ps(r2a, r2);

		destPosition.x += r0a[0] + r0a[1] + r0a[2] + r0a[3];
		destPosition.y += r1a[0] + r1a[1] + r1a[2] + r1a[3];
		destPosition.z += r2a[0] + r2a[1] + r2a[2] + r2a[3];
	}

	// Normal: accumulate into destNormal.
	{
		__m128 src = _mm_setr_ps(sourceNormal.x, sourceNormal.y, sourceNormal.z, 1.0f);

		const __m128 r0 = _mm_mul_ps(_mm_mul_ps(src, row0), scaleVec);
		const __m128 r1 = _mm_mul_ps(_mm_mul_ps(src, row1), scaleVec);
		const __m128 r2 = _mm_mul_ps(_mm_mul_ps(src, row2), scaleVec);

		SSE_ALIGN float r0a[4];
		_mm_store_ps(r0a, r0);
		SSE_ALIGN float r1a[4];
		_mm_store_ps(r1a, r1);
		SSE_ALIGN float r2a[4];
		_mm_store_ps(r2a, r2);

		destNormal.x += r0a[0] + r0a[1] + r0a[2];
		destNormal.y += r1a[0] + r1a[1] + r1a[2];
		destNormal.z += r2a[0] + r2a[1] + r2a[2];
	}
}

// ======================================================================
