// ======================================================================
//
// Direct3d11_ConstantBuffers.cpp
// copyright (c) 2026 Galaxies Reborn
//
// ======================================================================

#include "FirstDirect3d11.h"
#include "Direct3d11_ConstantBuffers.h"

#include "Direct3d11_Device.h"
#include "Direct3d11_Metrics.h"
#include "PaddedVector.h"

#include "clientGraphics/ShaderConstantRegisters.h"
#include "clientGraphics/ShaderImplementation.h"

#include <math.h>

// ======================================================================

namespace Direct3d11_ConstantBuffersNamespace
{
	int const cms_bytesPerRow = 16;
	constexpr int cs_matrixFloatCount = 16;
	constexpr float cs_colorChannelMaximum = 255.0f;

	// Sized from the register enumeration rather than from a literal. The vertex
	// file has to reach c95 because registers.inc aliases four scalars onto it;
	// VSCR_MAX alone stops at 68 and would leave every one of those reading zero.
	int const cms_vertexRows = VSCR_CBUFFER_ROWS;
	int const cms_pixelRows = PSCR_CBUFFER_ROWS;

	// The rows the per-object ring owns. D3D9 put these two matrices at c0..c7 in
	// the register file; here they are a separate buffer, so the register file's
	// front eight rows are never touched per draw.
	int const cms_perObjectFirstRow = VSCR_objectWorldCameraProjectionMatrix;
	int const cms_perObjectRows = 8;
	int const cms_perObjectBytes = cms_perObjectRows * cms_bytesPerRow;

	// Constant buffer offsets must be a multiple of sixteen constants.
	int const cms_perObjectStride = 256;
	int const cms_perObjectSlices = 1024;

	float *ms_vertexShadow;
	float *ms_pixelShadow;

	int ms_vertexDirtyFirst;
	int ms_vertexDirtyLast;
	int ms_pixelDirtyFirst;
	int ms_pixelDirtyLast;

	ID3D11Buffer *ms_vertexGlobals;
	ID3D11Buffer *ms_pixelGlobals;

	// The pixel epilogue at b1. One row, holding the alpha test as a (scale, bias) pair the
	// injected shader code applies with a single multiply-add and a clip. It is a separate
	// buffer rather than a row of $Globals because $Globals byte offsets ARE the register
	// ABI every shader in the corpus depends on, and there is no reason to move that
	// contract for something the backend owns outright.
	ID3D11Buffer *ms_pixelEpilogue;
	float ms_epilogueShadow[4];

	// The second row of b1: fog colour in rgb, the enable in alpha. Starts fully unfogged so a
	// scene that never calls setFog is not tinted by a zeroed colour at full strength.
	float ms_fogShadow[4] = {0.0f, 0.0f, 0.0f, 0.0f};

	// What the engine last asked for, kept apart from ms_fogShadow -- which is what is actually
	// uploaded -- so a pass's FM_Black or FM_White override can be applied and then lifted
	// without having lost the scene's own colour.
	float ms_fogSceneColor[3] = {0.0f, 0.0f, 0.0f};
	bool ms_fogSceneEnabled = false;
	int ms_fogColorMode = 0; // ShaderImplementation::Pass::FogMode

	void applyFogColor();
	bool ms_epilogueDirty;
	bool ms_epilogueBound;

	// The state the pair is derived from. Both the material reference and the engine fade
	// opacity move it, so it is derived at flush from whichever changed rather than at
	// either call site.
	int ms_alphaTestFunction;
	float ms_alphaTestReference;
	float ms_alphaFadeOpacity = 1.0f;

	// The ring, when the device can offset-bind and no-overwrite it. Otherwise a
	// small set of single-slice buffers renamed with DISCARD in turn.
	ID3D11Buffer *ms_perObjectRing;
	ID3D11Buffer *ms_perObjectRotating[4];
	int ms_perObjectCursor;
	int ms_perObjectRotatingIndex;
	bool ms_perObjectUsesRing;
	bool ms_perObjectBoundThisDraw;

	float ms_perObject[cms_perObjectRows * 4];

	bool ms_installed;

	void markVertexDirty(int firstRow, int rowCount);
	void markPixelDirty(int firstRow, int rowCount);
	void flushPixelEpilogue();
	bool createBuffers();
	void releaseBuffers();
	void flushPerObject();
} // namespace Direct3d11_ConstantBuffersNamespace
using namespace Direct3d11_ConstantBuffersNamespace;

// ======================================================================

void Direct3d11_ConstantBuffersNamespace::markVertexDirty(int firstRow, int rowCount)
{
	int const lastRow = firstRow + rowCount - 1;

	if (ms_vertexDirtyFirst < 0 || firstRow < ms_vertexDirtyFirst)
		ms_vertexDirtyFirst = firstRow;
	if (lastRow > ms_vertexDirtyLast)
		ms_vertexDirtyLast = lastRow;
}

// ----------------------------------------------------------------------

void Direct3d11_ConstantBuffersNamespace::markPixelDirty(int firstRow, int rowCount)
{
	int const lastRow = firstRow + rowCount - 1;

	if (ms_pixelDirtyFirst < 0 || firstRow < ms_pixelDirtyFirst)
		ms_pixelDirtyFirst = firstRow;
	if (lastRow > ms_pixelDirtyLast)
		ms_pixelDirtyLast = lastRow;
}

// ======================================================================

bool Direct3d11_ConstantBuffersNamespace::createBuffers()
{
	ID3D11Device1 *const device = Direct3d11_Device::getDevice();
	NOT_NULL(device);

	D3D11_BUFFER_DESC description;
	Zero(description);
	description.Usage = D3D11_USAGE_DYNAMIC;
	description.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	description.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	description.ByteWidth = static_cast<UINT>(cms_vertexRows * cms_bytesPerRow);
	HRESULT hresult = device->CreateBuffer(&description, NULL, &ms_vertexGlobals);
	if (FAILED(hresult) || !ms_vertexGlobals)
	{
		WARNING(true, ("Direct3d11: the vertex constant buffer could not be created (%s).", Direct3d11_Device::describeHresult(hresult)));
		return false;
	}
	++Direct3d11_Metrics::constantBufferCreations;

	description.ByteWidth = static_cast<UINT>(cms_pixelRows * cms_bytesPerRow);
	hresult = device->CreateBuffer(&description, NULL, &ms_pixelGlobals);
	if (FAILED(hresult) || !ms_pixelGlobals)
	{
		WARNING(true, ("Direct3d11: the pixel constant buffer could not be created (%s).", Direct3d11_Device::describeHresult(hresult)));
		return false;
	}
	++Direct3d11_Metrics::constantBufferCreations;

	// Two rows: the alpha test pair and the fog colour.
	description.ByteWidth = static_cast<UINT>(2 * cms_bytesPerRow);
	hresult = device->CreateBuffer(&description, NULL, &ms_pixelEpilogue);
	if (FAILED(hresult) || !ms_pixelEpilogue)
	{
		WARNING(true, ("Direct3d11: the pixel epilogue constant buffer could not be created (%s).", Direct3d11_Device::describeHresult(hresult)));
		return false;
	}
	++Direct3d11_Metrics::constantBufferCreations;

	// A ring needs both capabilities. Without either, a per-draw slice cannot be
	// appended and bound by offset, and the fallback is renaming a small buffer
	// per draw -- which works, costs more, and is reported rather than assumed.
	ms_perObjectUsesRing = Direct3d11_Device::supportsConstantBufferOffsetting() && Direct3d11_Device::supportsConstantBufferNoOverwrite();

	if (ms_perObjectUsesRing)
	{
		description.ByteWidth = static_cast<UINT>(cms_perObjectStride * cms_perObjectSlices);
		hresult = device->CreateBuffer(&description, NULL, &ms_perObjectRing);
		if (FAILED(hresult) || !ms_perObjectRing)
		{
			WARNING(true, ("Direct3d11: the per-object constant ring could not be created (%s), falling back to rotating buffers.", Direct3d11_Device::describeHresult(hresult)));
			ms_perObjectUsesRing = false;
		}
		else
			++Direct3d11_Metrics::constantBufferCreations;
	}

	if (!ms_perObjectUsesRing)
	{
		description.ByteWidth = static_cast<UINT>(cms_perObjectStride);
		for (int i = 0; i < 4; ++i)
		{
			hresult = device->CreateBuffer(&description, NULL, &ms_perObjectRotating[i]);
			if (FAILED(hresult) || !ms_perObjectRotating[i])
			{
				WARNING(true, ("Direct3d11: per-object constant buffer %d could not be created (%s).", i, Direct3d11_Device::describeHresult(hresult)));
				return false;
			}
			++Direct3d11_Metrics::constantBufferCreations;
		}

		WARNING(true, ("Direct3d11: per-draw constants are using rotating buffers with DISCARD rather than an offset-bound ring. This is the slower path; it is in use because this device lacks constant buffer offsetting or no-overwrite constant mapping."));
	}

	return true;
}

// ----------------------------------------------------------------------

void Direct3d11_ConstantBuffersNamespace::releaseBuffers()
{
	for (int i = 0; i < 4; ++i)
		if (ms_perObjectRotating[i])
		{
			ms_perObjectRotating[i]->Release();
			ms_perObjectRotating[i] = NULL;
		}

	if (ms_perObjectRing)
	{
		ms_perObjectRing->Release();
		ms_perObjectRing = NULL;
	}
	if (ms_pixelGlobals)
	{
		ms_pixelGlobals->Release();
		ms_pixelGlobals = NULL;
	}
	if (ms_pixelEpilogue)
	{
		ms_pixelEpilogue->Release();
		ms_pixelEpilogue = NULL;
	}
	if (ms_vertexGlobals)
	{
		ms_vertexGlobals->Release();
		ms_vertexGlobals = NULL;
	}
}

// ======================================================================

void Direct3d11_ConstantBuffers::install()
{
	DEBUG_FATAL(ms_installed, ("Direct3d11_ConstantBuffers::install called twice"));

	ms_vertexShadow = new float[cms_vertexRows * 4];
	ms_pixelShadow = new float[cms_pixelRows * 4];

	memset(ms_vertexShadow, 0, cms_vertexRows * cms_bytesPerRow);
	memset(ms_pixelShadow, 0, cms_pixelRows * cms_bytesPerRow);
	memset(ms_perObject, 0, sizeof(ms_perObject));

	ms_vertexDirtyFirst = -1;
	ms_vertexDirtyLast = -1;
	ms_pixelDirtyFirst = -1;
	ms_pixelDirtyLast = -1;

	FATAL(!createBuffers(), ("Direct3d11: the constant buffers could not be created."));

	Direct3d11_ConstantBuffers::seedBackendOwnedConstants();

	ms_installed = true;
}

// ----------------------------------------------------------------------

void Direct3d11_ConstantBuffers::remove()
{
	releaseBuffers();

	delete[] ms_pixelShadow;
	ms_pixelShadow = NULL;
	delete[] ms_vertexShadow;
	ms_vertexShadow = NULL;

	ms_installed = false;
}

// ======================================================================
/**
 * Write the constants this backend owns rather than receives.
 *
 * These four rows are exactly the ones D3D9 re-establishes in
 * Direct3d9_StateCache::restoreDevice, and nothing else: every other constant is
 * rewritten by the ordinary drawing flow. Reproducing that set, rather than
 * reseeding everything, keeps the two backends in step about what is stale after
 * a rebuild.
 */

void Direct3d11_ConstantBuffers::seedBackendOwnedConstants()
{
	// c95, whose components registers.inc aliases as c0_0, c0_5, c1_0 and cLog2e.
	// The last is log2(e), which the fog exponent conversion needs; without this
	// row every 0.5 bias and every 1.0 constant in nine assembly modules reads
	// zero. Expression copied from Direct3d9_StateCache.cpp.
	float const literalConstants[4] = {0.0f, 0.5f, 1.0f, 1.0f / static_cast<float>(log(2.0f))};
	Direct3d11_ConstantBuffers::setVertexShaderConstants(VSCR_literalConstants, literalConstants, 1);

	// The unit vectors, which terrain_dot3 consumes. Note the fourth component is
	// ZERO, not one: D3D9 uploads a PaddedVector whose pad member is initialised
	// to 0.0f, so w is 0. Writing 1.0f there would be an easy and invisible error.
	PaddedVector const unitX(1.0f, 0.0f, 0.0f);
	PaddedVector const unitY(0.0f, 1.0f, 0.0f);
	PaddedVector const unitZ(0.0f, 0.0f, 1.0f);
	Direct3d11_ConstantBuffers::setVertexShaderConstants(VSCR_unitX, &unitX, 1);
	Direct3d11_ConstantBuffers::setVertexShaderConstants(VSCR_unitY, &unitY, 1);
	Direct3d11_ConstantBuffers::setVertexShaderConstants(VSCR_unitZ, &unitZ, 1);
}

// ======================================================================

void Direct3d11_ConstantBuffers::setVertexShaderConstants(int index, void const *data, int numberOfConstants)
{
	NOT_NULL(data);
	DEBUG_FATAL(index < 0 || numberOfConstants < 0, ("Direct3d11: bad vertex constant range %d + %d", index, numberOfConstants));

	// A write past the end of the register file is fatal rather than clamped.
	// D3D9 FATALs on the same condition through FATAL_DX_HR, and a silent clamp
	// would drop constants the shader then reads as whatever was there before.
	FATAL(index + numberOfConstants > cms_vertexRows, ("Direct3d11: a vertex constant write of %d row(s) at register %d runs past the end of the %d-row register file.", numberOfConstants, index, cms_vertexRows));

	if (!numberOfConstants)
		return;

	int const bytes = numberOfConstants * cms_bytesPerRow;
	float *const destination = ms_vertexShadow + (index * 4);

	// Only mark dirty when something actually changed. D3D9 had no shadow and so
	// no way to notice; here the comparison is far cheaper than the upload it
	// avoids, and the light manager re-sends identical blocks constantly.
	if (memcmp(destination, data, bytes) == 0)
		return;

	memcpy(destination, data, bytes);
	markVertexDirty(index, numberOfConstants);
}

// ----------------------------------------------------------------------

void Direct3d11_ConstantBuffers::setPixelShaderConstants(int index, void const *data, int numberOfConstants)
{
	NOT_NULL(data);
	DEBUG_FATAL(index < 0 || numberOfConstants < 0, ("Direct3d11: bad pixel constant range %d + %d", index, numberOfConstants));
	FATAL(index + numberOfConstants > cms_pixelRows, ("Direct3d11: a pixel constant write of %d row(s) at register %d runs past the end of the %d-row register file.", numberOfConstants, index, cms_pixelRows));

	if (!numberOfConstants)
		return;

	int const bytes = numberOfConstants * cms_bytesPerRow;
	float *const destination = ms_pixelShadow + (index * 4);

	if (memcmp(destination, data, bytes) == 0)
		return;

	memcpy(destination, data, bytes);
	markPixelDirty(index, numberOfConstants);
}

// ----------------------------------------------------------------------

void Direct3d11_ConstantBuffers::setPixelShaderConstantComponent(int index, int component, float value)
{
	DEBUG_FATAL(index < 0 || index >= cms_pixelRows, ("Direct3d11: pixel constant register %d is outside the %d-row register file.", index, cms_pixelRows));
	DEBUG_FATAL(component < 0 || component > 3, ("Direct3d11: pixel constant component %d is not 0 to 3.", component));

	float *const destination = ms_pixelShadow + (index * 4) + component;
	if (*destination == value)
		return;

	*destination = value;
	markPixelDirty(index, 1);
}

// ======================================================================

void Direct3d11_ConstantBuffers::setPerObjectTransforms(float const *objectWorldCameraProjection, float const *objectWorldMatrix)
{
	NOT_NULL(objectWorldCameraProjection);
	NOT_NULL(objectWorldMatrix);

	memcpy(ms_perObject, objectWorldCameraProjection, sizeof(float) * cs_matrixFloatCount);
	memcpy(ms_perObject + cs_matrixFloatCount, objectWorldMatrix, sizeof(float) * cs_matrixFloatCount);

	ms_perObjectBoundThisDraw = false;
}

// ----------------------------------------------------------------------
/**
 * Put the per-object slice somewhere the shader can read it.
 *
 * On the ring path this appends at the cursor with WRITE_NO_OVERWRITE, which does
 * not rename the buffer, and binds a sub-range by offset. That is the whole point:
 * a draw costs sixteen bytes of bookkeeping rather than a buffer rename.
 *
 * On the fallback path it renames one of a handful of small buffers with DISCARD
 * and binds it whole.
 */

void Direct3d11_ConstantBuffersNamespace::flushPerObject()
{
	ID3D11DeviceContext1 *const context = Direct3d11_Device::getContext();
	if (!context)
		return;

	if (ms_perObjectUsesRing)
	{
		if (ms_perObjectCursor >= cms_perObjectSlices)
		{
			// Wrapped. A DISCARD here is correct and is the only one this buffer
			// should ever see in a frame; more than one means the ring is too small
			// for the frame's draw count.
			ms_perObjectCursor = 0;
			++Direct3d11_Metrics::ringDiscards;

			D3D11_MAPPED_SUBRESOURCE mapped;
			Zero(mapped);
			if (SUCCEEDED(context->Map(ms_perObjectRing, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
			{
				memcpy(mapped.pData, ms_perObject, cms_perObjectBytes);
				context->Unmap(ms_perObjectRing, 0);
			}
		}
		else
		{
			D3D11_MAPPED_SUBRESOURCE mapped;
			Zero(mapped);
			if (SUCCEEDED(context->Map(ms_perObjectRing, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &mapped)))
			{
				memcpy(static_cast<uint8 *>(mapped.pData) + (ms_perObjectCursor * cms_perObjectStride), ms_perObject, cms_perObjectBytes);
				context->Unmap(ms_perObjectRing, 0);
				++Direct3d11_Metrics::ringNoOverwrites;
			}
		}

		UINT const firstConstant = static_cast<UINT>((ms_perObjectCursor * cms_perObjectStride) / cms_bytesPerRow);
		UINT const constantCount = static_cast<UINT>(cms_perObjectStride / cms_bytesPerRow);
		UINT const slot = Direct3d11_ConstantBuffers::PER_OBJECT_SLOT;

		context->VSSetConstantBuffers1(slot, 1, &ms_perObjectRing, &firstConstant, &constantCount);

		++ms_perObjectCursor;
	}
	else
	{
		ms_perObjectRotatingIndex = (ms_perObjectRotatingIndex + 1) & 3;
		ID3D11Buffer *const buffer = ms_perObjectRotating[ms_perObjectRotatingIndex];

		D3D11_MAPPED_SUBRESOURCE mapped;
		Zero(mapped);
		if (SUCCEEDED(context->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
		{
			memcpy(mapped.pData, ms_perObject, cms_perObjectBytes);
			context->Unmap(buffer, 0);
			++Direct3d11_Metrics::ringDiscards;
		}

		UINT const slot = Direct3d11_ConstantBuffers::PER_OBJECT_SLOT;
		context->VSSetConstantBuffers(slot, 1, &buffer);
	}

	Direct3d11_Metrics::constantBufferBytes += cms_perObjectBytes;
	++Direct3d11_Metrics::constantBufferUpdates;

	ms_perObjectBoundThisDraw = true;
}

// ======================================================================
/**
 * Upload whatever changed, and nothing else.
 *
 * The register files are renamed with DISCARD rather than partially updated,
 * which sounds wasteful and is not: it needs no optional capability, and it only
 * happens when a row actually changed, which after the per-draw matrices moved to
 * b3 means on a shader or light change rather than on every draw. Only the dirty
 * span is copied, so an unchanged tail costs nothing to skip.
 */

void Direct3d11_ConstantBuffersNamespace::flushPixelEpilogue()
{
	ID3D11DeviceContext1 *const context = Direct3d11_Device::getContext();
	if (!context || !ms_pixelEpilogue)
		return;

	if (ms_epilogueDirty)
	{
		// Six of the engine eight compare functions reduce to clip(a * scale + bias). Equal
		// and NotEqual do not, and an alpha test has no ordinary use for either; if one ever
		// appears it is reported and treated as passing, which draws too much rather than
		// discarding the wrong pixels.
		//
		// The reference is scaled by the fade opacity exactly as DX9 does
		// (Direct3d9.cpp:3939), and normalised: the engine stores it as an 8-bit value while
		// the shader compares a float.
		float const reference = (ms_alphaTestReference * ms_alphaFadeOpacity) / cs_colorChannelMaximum;

		// One step of an 8-bit alpha, which is the precision D3D9 compared at, so a strict
		// test differs from a non-strict one by exactly the smallest representable amount.
		float const epsilon = 1.0f / cs_colorChannelMaximum;

		float scale = 0.0f;
		float bias = 1.0f;

		switch (ms_alphaTestFunction)
		{
			case ShaderImplementation::Pass::C_Never:
				scale = 0.0f;
				bias = -1.0f;
				break;
			case ShaderImplementation::Pass::C_Always:
				scale = 0.0f;
				bias = 1.0f;
				break;
			case ShaderImplementation::Pass::C_GreaterOrEqual:
				scale = 1.0f;
				bias = -reference;
				break;
			case ShaderImplementation::Pass::C_Greater:
				scale = 1.0f;
				bias = -(reference + epsilon);
				break;
			case ShaderImplementation::Pass::C_LessOrEqual:
				scale = -1.0f;
				bias = reference;
				break;
			case ShaderImplementation::Pass::C_Less:
				scale = -1.0f;
				bias = reference - epsilon;
				break;

			case ShaderImplementation::Pass::C_Equal:
			case ShaderImplementation::Pass::C_NotEqual:
			default:
			{
				static bool reported = false;
				if (!reported)
				{
					reported = true;
					WARNING(true, ("Direct3d11: an alpha test asks for compare function %d, which cannot be written as a single multiply-add and which an alpha test has no ordinary use for. Treating it as always passing. Reported once.", ms_alphaTestFunction));
				}
				scale = 0.0f;
				bias = 1.0f;
			}
			break;
		}

		ms_epilogueShadow[0] = scale;
		ms_epilogueShadow[1] = bias;
		ms_epilogueShadow[2] = 0.0f;
		ms_epilogueShadow[3] = 0.0f;

		// Two rows: the alpha test pair, then the fog colour with the enable in alpha. Written
		// together because they share the buffer and a partial map is not a thing.
		float rows[8];
		memcpy(rows, ms_epilogueShadow, sizeof(ms_epilogueShadow));
		memcpy(rows + 4, ms_fogShadow, sizeof(ms_fogShadow));

		D3D11_MAPPED_SUBRESOURCE mapped;
		Zero(mapped);
		if (SUCCEEDED(context->Map(ms_pixelEpilogue, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
		{
			memcpy(mapped.pData, rows, sizeof(rows));
			context->Unmap(ms_pixelEpilogue, 0);

			Direct3d11_Metrics::constantBufferBytes += static_cast<int>(sizeof(rows));
			++Direct3d11_Metrics::constantBufferUpdates;
		}

		ms_epilogueDirty = false;
	}

	// Bound once. Every pixel shader declares the buffer, so the binding never changes and
	// re-binding it per draw would be pure traffic.
	if (!ms_epilogueBound)
	{
		ms_epilogueBound = true;
		UINT const slot = Direct3d11_ConstantBuffers::PIXEL_EPILOGUE_SLOT;
		context->PSSetConstantBuffers(slot, 1, &ms_pixelEpilogue);
	}
}

// ----------------------------------------------------------------------

void Direct3d11_ConstantBuffers::setAlphaTest(int compareFunction, int reference)
{
	if (ms_alphaTestFunction == compareFunction && ms_alphaTestReference == static_cast<float>(reference))
		return;

	ms_alphaTestFunction = compareFunction;
	ms_alphaTestReference = static_cast<float>(reference);
	ms_epilogueDirty = true;
}

// ----------------------------------------------------------------------

void Direct3d11_ConstantBuffers::setAlphaFadeOpacity(float opacity)
{
	if (ms_alphaFadeOpacity == opacity)
		return;

	ms_alphaFadeOpacity = opacity;
	ms_epilogueDirty = true;
}

// ----------------------------------------------------------------------

void Direct3d11_ConstantBuffers::flush()
{
	ID3D11DeviceContext1 *const context = Direct3d11_Device::getContext();
	if (!context)
		return;

	if (ms_vertexDirtyFirst >= 0)
	{
		// MAP_WRITE_DISCARD hands back a FRESH, UNINITIALISED buffer. Whatever was in it is gone, so a
		// partial copy does not leave the untouched rows alone -- it leaves them undefined.
		//
		// This used to copy only rows 0..dirtyLast, in the belief that bounding the upload by the dirty
		// extent saved bandwidth the way DX9's per-register SetVertexShaderConstantF did. It does not
		// mean that under DISCARD, and the cost was every constant ABOVE the dirty span, on every draw:
		// the hemispheric lighting block at c60..c63, the dot3 colours at c42..c43, and the unit
		// vectors seeded once at install into c49..c51. Any draw whose dirty span happened to stop
		// below those read garbage. That is why terrain and bump-lit walls rendered lavender while
		// unlit surfaces in the same frame were correct, and why the wrongness was stable per material
		// rather than flickering -- a material's dirty span is the same every frame.
		//
		// The whole shadow goes up now. It is 96 rows of 16 bytes for the vertex file and 25 for the
		// pixel one: about 1.5 KB and 400 bytes per flush, which is not worth being clever about. If it
		// ever is, the answer is UpdateSubresource on a non-dynamic buffer, or a NO_OVERWRITE ring --
		// not a partial DISCARD, which cannot be made correct.

		D3D11_MAPPED_SUBRESOURCE mapped;
		Zero(mapped);
		if (SUCCEEDED(context->Map(ms_vertexGlobals, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
		{
			int const rows = cms_vertexRows;
			memcpy(mapped.pData, ms_vertexShadow, rows * cms_bytesPerRow);
			context->Unmap(ms_vertexGlobals, 0);

			Direct3d11_Metrics::constantBufferBytes += rows * cms_bytesPerRow;
			++Direct3d11_Metrics::constantBufferUpdates;
		}

		UINT const slot = VERTEX_GLOBALS_SLOT;
		context->VSSetConstantBuffers(slot, 1, &ms_vertexGlobals);

		ms_vertexDirtyFirst = -1;
		ms_vertexDirtyLast = -1;
	}

	if (ms_pixelDirtyFirst >= 0)
	{
		// The whole shadow, for the reason given above the vertex upload.
		D3D11_MAPPED_SUBRESOURCE mapped;
		Zero(mapped);
		if (SUCCEEDED(context->Map(ms_pixelGlobals, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
		{
			int const rows = cms_pixelRows;
			memcpy(mapped.pData, ms_pixelShadow, rows * cms_bytesPerRow);
			context->Unmap(ms_pixelGlobals, 0);

			Direct3d11_Metrics::constantBufferBytes += rows * cms_bytesPerRow;
			++Direct3d11_Metrics::constantBufferUpdates;
		}

		UINT const slot = PIXEL_GLOBALS_SLOT;
		context->PSSetConstantBuffers(slot, 1, &ms_pixelGlobals);

		ms_pixelDirtyFirst = -1;
		ms_pixelDirtyLast = -1;
	}

	flushPixelEpilogue();

	if (!ms_perObjectBoundThisDraw)
		flushPerObject();
}

// ----------------------------------------------------------------------

void Direct3d11_ConstantBuffers::beginFrame()
{
	// The ring restarts each frame. The GPU is at most a couple of frames behind
	// and the ring holds far more slices than a frame uses, so restarting is not
	// the same as overwriting work in flight -- and if it ever were, the discard
	// counter would say so.
	ms_perObjectCursor = 0;
	ms_perObjectBoundThisDraw = false;
}

// ======================================================================
/**
 * The 2D pixel-to-clip transform, at c9.
 *
 * Component order and signs copied exactly from D3D9. The Y scale is negative
 * because screen Y grows downward, and the biases are asymmetric for the same
 * reason. Without this row every 2D vertex -- the whole UI, and any full-screen
 * pass -- collapses to clip zero, with nothing reported anywhere.
 */

void Direct3d11_ConstantBuffers::setViewportData(int x, int y, int width, int height)
{
	// Guard the divisions rather than uploading infinities: a zero-sized viewport
	// is transient, and a NaN in the register file persists.
	if (width <= 0 || height <= 0)
		return;

	float const xOffset = (static_cast<float>(x) * 2.0f) / static_cast<float>(width);
	float const yOffset = (static_cast<float>(y) * 2.0f) / static_cast<float>(height);

	// The half-pixel shift D3D9 got from its rasteriser and D3D10 removed.
	//
	// D3D9 sampled pixel centres at integer coordinates; D3D10 and later sample at half-integers.
	// The engine's 2D geometry is authored for the former, so under D3D11 adjacent quads no longer
	// meet: the loading screen, which is drawn as tiles, showed a hard one-pixel line of background
	// along every tile boundary. Shifting the mapping half a pixel puts the edges back where D3D9
	// put them.
	//
	// In x the scale is 2/width, so half a pixel is 1/width; in y the scale is negative, so the
	// correction is added rather than subtracted.
	float const halfPixelX = 1.0f / static_cast<float>(width);
	float const halfPixelY = 1.0f / static_cast<float>(height);

	float const viewportData[4] =
		{
			2.0f / static_cast<float>(width),
			-2.0f / static_cast<float>(height),
			-1.0f - xOffset - halfPixelX,
			1.0f + yOffset + halfPixelY};

	setVertexShaderConstants(VSCR_viewportData, viewportData, 1);
}

// ----------------------------------------------------------------------
/**
 * Fog density, at c10.
 *
 * Packed { 0, 0, density, density * density }, and written ONLY when fog is being
 * enabled -- which is what D3D9 does. Disabling fog there clears a render state
 * and leaves this constant live, so the shader's fog term still has a density in
 * it and the disable works by some other means. Zeroing the row here instead
 * would look tidier and would change what every scoped fog disable does.
 */

/**
 * Resolve the scene colour and any pass override into the colour the epilogue reads.
 *
 * FM_Black and FM_White are what an additive or multiplicative pass needs so that fog contributes
 * nothing or multiplies by one respectively. Without them every blended layer fogs with the scene
 * colour and the contributions stack.
 */

void Direct3d11_ConstantBuffersNamespace::applyFogColor()
{
	float red = ms_fogSceneColor[0];
	float green = ms_fogSceneColor[1];
	float blue = ms_fogSceneColor[2];

	switch (ms_fogColorMode)
	{
		case ShaderImplementation::Pass::FM_Black:
			red = green = blue = 0.0f;
			break;

		case ShaderImplementation::Pass::FM_White:
			red = green = blue = 1.0f;
			break;

		default:
			break;
	}

	float const alpha = ms_fogSceneEnabled ? 1.0f : 0.0f;

	if (ms_fogShadow[0] == red && ms_fogShadow[1] == green && ms_fogShadow[2] == blue && ms_fogShadow[3] == alpha)
		return;

	ms_fogShadow[0] = red;
	ms_fogShadow[1] = green;
	ms_fogShadow[2] = blue;
	ms_fogShadow[3] = alpha;
	ms_epilogueDirty = true;
}

// ----------------------------------------------------------------------

void Direct3d11_ConstantBuffers::setFogColorMode(int fogMode)
{
	if (ms_fogColorMode == fogMode)
		return;

	ms_fogColorMode = fogMode;
	applyFogColor();
}

// ----------------------------------------------------------------------

void Direct3d11_ConstantBuffers::setFog(bool enabled, float density, float red, float green, float blue)
{
	// c10 is what calculateFog reads in the vertex programs: the density in z and its square in
	// w, which is the layout DX9 uploads. Written even when fog is off, so that turning it off
	// does not leave the last density behind -- DX9 returned early here and relied on
	// D3DRS_FOGENABLE to stop the blend, and there is no such state in D3D11.
	float const fog[4] = {0.0f, 0.0f, density, density * density};
	setVertexShaderConstants(VSCR_fog, fog, 1);

	// The colour and the enable go to the pixel epilogue, which is the half D3D9 did in fixed
	// function. The enable rides in alpha so that the epilogue's lerp collapses to a no-op
	// rather than needing a branch.
	ms_fogSceneColor[0] = red;
	ms_fogSceneColor[1] = green;
	ms_fogSceneColor[2] = blue;
	ms_fogSceneEnabled = enabled;

	// Whatever pass override is in force still applies; this only changes what "normal" resolves to.
	applyFogColor();
}

// ----------------------------------------------------------------------

void Direct3d11_ConstantBuffers::setCurrentTime(float currentTime)
{
	// { time, 0, 0, 0 }, once per frame, before the scene. Monotonic and never
	// wrapped, exactly as D3D9 accumulates it -- a modulo here would make every
	// animated material jump at the wrap.
	float const time[4] = {currentTime, 0.0f, 0.0f, 0.0f};
	setVertexShaderConstants(VSCR_currentTime, time, 1);
}

// ======================================================================
