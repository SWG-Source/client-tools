// ======================================================================
//
// Direct3d11_StateTables.cpp
// copyright (c) 2026 Galaxies Reborn
//
// ======================================================================

#include "FirstDirect3d11.h"
#include "Direct3d11_StateTables.h"

// ======================================================================

namespace Direct3d11_StateTablesNamespace
{
	// The engine's enumerations, by index. Kept here as names so the tables below
	// can be read against them without dragging ShaderImplementation.h in.
	//
	// ShaderImplementation::Compare        C_Never .. C_Always            (8)
	// ShaderImplementation::Blend          B_Zero .. B_SourceAlphaSaturate (11)
	// ShaderImplementation::BlendOperation BO_Add .. BO_Max               (5)
	// ShaderImplementation::StencilOperation SO_Keep .. SO_DecrementWrap  (8)
	// StaticShaderTemplate::TextureAddress TA_wrap .. TA_mirrorOnce, TA_invalid (6)
	// StaticShaderTemplate::TextureFilter  TF_none .. TF_gaussianCubic, TF_invalid (7)

	// Copied from Direct3d9_ShaderImplementationData.cpp:31-41.
	// Indices 5 and 6 are swapped with respect to their engine names. This is the
	// asset contract. See the header.
	D3D11_COMPARISON_FUNC const cms_compare[] =
		{
			D3D11_COMPARISON_NEVER,			// 0 C_Never
			D3D11_COMPARISON_LESS,			// 1 C_Less
			D3D11_COMPARISON_EQUAL,			// 2 C_Equal
			D3D11_COMPARISON_LESS_EQUAL,	// 3 C_LessOrEqual
			D3D11_COMPARISON_GREATER,		// 4 C_Greater
			D3D11_COMPARISON_NOT_EQUAL,		// 5 C_GreaterOrEqual  <- DX9 maps this to NOTEQUAL
			D3D11_COMPARISON_GREATER_EQUAL, // 6 C_NotEqual        <- DX9 maps this to GREATEREQUAL
			D3D11_COMPARISON_ALWAYS			// 7 C_Always
	};

	// Copied from Direct3d9_ShaderImplementationData.cpp:43-56.
	D3D11_BLEND const cms_blend[] =
		{
			D3D11_BLEND_ZERO,			// B_Zero
			D3D11_BLEND_ONE,			// B_One
			D3D11_BLEND_SRC_COLOR,		// B_SourceColor
			D3D11_BLEND_INV_SRC_COLOR,	// B_InverseSourceColor
			D3D11_BLEND_SRC_ALPHA,		// B_SourceAlpha
			D3D11_BLEND_INV_SRC_ALPHA,	// B_InverseSourceAlpha
			D3D11_BLEND_DEST_ALPHA,		// B_DestinationAlpha
			D3D11_BLEND_INV_DEST_ALPHA, // B_InverseDestinationAlpha
			D3D11_BLEND_DEST_COLOR,		// B_DestinationColor
			D3D11_BLEND_INV_DEST_COLOR, // B_InverseDestinationColor
			D3D11_BLEND_SRC_ALPHA_SAT	// B_SourceAlphaSaturate
	};

	// Copied from Direct3d9_ShaderImplementationData.cpp:58-65.
	D3D11_BLEND_OP const cms_blendOperation[] =
		{
			D3D11_BLEND_OP_ADD,			 // BO_Add
			D3D11_BLEND_OP_SUBTRACT,	 // BO_Subtract
			D3D11_BLEND_OP_REV_SUBTRACT, // BO_ReverseSubtract
			D3D11_BLEND_OP_MIN,			 // BO_Min
			D3D11_BLEND_OP_MAX			 // BO_Max
	};

	// Copied from Direct3d9_ShaderImplementationData.cpp:74-84. Note the last two:
	// D3D9's INCR/DECR wrap, and its INCRSAT/DECRSAT saturate. D3D11 names them
	// the other way round -- INCR_SAT saturates and INCR wraps -- so the mapping
	// looks like a straight rename but is not one.
	D3D11_STENCIL_OP const cms_stencilOperation[] =
		{
			D3D11_STENCIL_OP_KEEP,	   // SO_Keep
			D3D11_STENCIL_OP_ZERO,	   // SO_Zero
			D3D11_STENCIL_OP_REPLACE,  // SO_Replace
			D3D11_STENCIL_OP_INCR_SAT, // SO_IncrementSaturate
			D3D11_STENCIL_OP_DECR_SAT, // SO_DecrementSaturate
			D3D11_STENCIL_OP_INVERT,   // SO_Invert
			D3D11_STENCIL_OP_INCR,	   // SO_IncrementWrap
			D3D11_STENCIL_OP_DECR	   // SO_DecrementWrap
	};

	// Copied from Direct3d9_StaticShaderData.cpp:46-55, including TA_invalid
	// falling back to wrap.
	D3D11_TEXTURE_ADDRESS_MODE const cms_textureAddress[] =
		{
			D3D11_TEXTURE_ADDRESS_WRAP,		   // TA_wrap
			D3D11_TEXTURE_ADDRESS_MIRROR,	   // TA_mirror
			D3D11_TEXTURE_ADDRESS_CLAMP,	   // TA_clamp
			D3D11_TEXTURE_ADDRESS_BORDER,	   // TA_border
			D3D11_TEXTURE_ADDRESS_MIRROR_ONCE, // TA_mirrorOnce
			D3D11_TEXTURE_ADDRESS_WRAP		   // TA_invalid
	};

	// The filter indices, from Direct3d9_StaticShaderData.cpp:57-67. D3D9 mapped
	// each of min/mag/mip through this one table; D3D11 needs the three combined
	// into a single D3D11_FILTER, so the table is kept as a classification rather
	// than as D3D11 values.
	enum FilterKind
	{
		FK_none,
		FK_point,
		FK_linear,
		FK_anisotropic
	};

	FilterKind const cms_filterKind[] =
		{
			FK_none,		// TF_none
			FK_point,		// TF_point
			FK_linear,		// TF_linear
			FK_anisotropic, // TF_anisotropic
			FK_none,		// TF_flatCubic       -- DX9 declines these two
			FK_none,		// TF_gaussianCubic
			FK_linear		// TF_invalid
	};

	int const cms_compareCount = sizeof(cms_compare) / sizeof(cms_compare[0]);
	int const cms_blendCount = sizeof(cms_blend) / sizeof(cms_blend[0]);
	int const cms_blendOperationCount = sizeof(cms_blendOperation) / sizeof(cms_blendOperation[0]);
	int const cms_stencilOperationCount = sizeof(cms_stencilOperation) / sizeof(cms_stencilOperation[0]);
	int const cms_textureAddressCount = sizeof(cms_textureAddress) / sizeof(cms_textureAddress[0]);
	int const cms_filterKindCount = sizeof(cms_filterKind) / sizeof(cms_filterKind[0]);
} // namespace Direct3d11_StateTablesNamespace
using namespace Direct3d11_StateTablesNamespace;

// ======================================================================
/**
 * Assert the tables are the shape and the content they are supposed to be.
 *
 * The Compare check is the one that matters. If someone "tidies" indices 5 and 6
 * to match their engine names, this stops the client at install with an
 * explanation, rather than quietly changing the depth and stencil comparison of
 * every pass that uses either value.
 */

void Direct3d11_StateTables::verifyTables()
{
	FATAL(cms_compareCount != 8, ("Direct3d11: the compare table has %d entries, expected 8.", cms_compareCount));
	FATAL(cms_blendCount != 11, ("Direct3d11: the blend table has %d entries, expected 11.", cms_blendCount));
	FATAL(cms_blendOperationCount != 5, ("Direct3d11: the blend operation table has %d entries, expected 5.", cms_blendOperationCount));
	FATAL(cms_stencilOperationCount != 8, ("Direct3d11: the stencil operation table has %d entries, expected 8.", cms_stencilOperationCount));
	FATAL(cms_textureAddressCount != 6, ("Direct3d11: the texture address table has %d entries, expected 6.", cms_textureAddressCount));
	FATAL(cms_filterKindCount != 7, ("Direct3d11: the filter table has %d entries, expected 7.", cms_filterKindCount));

	// The DX9 swap, asserted rather than trusted.
	FATAL(cms_compare[5] != D3D11_COMPARISON_NOT_EQUAL, ("Direct3d11: compare index 5 must map to NOT_EQUAL, matching Direct3d9_ShaderImplementationData.cpp:38. Shader assets store the index, so changing this changes what every pass using it has always meant."));
	FATAL(cms_compare[6] != D3D11_COMPARISON_GREATER_EQUAL, ("Direct3d11: compare index 6 must map to GREATER_EQUAL, matching Direct3d9_ShaderImplementationData.cpp:39. See index 5."));

	// The stencil wrap/saturate pairing, which reads like a rename and is not.
	FATAL(cms_stencilOperation[3] != D3D11_STENCIL_OP_INCR_SAT, ("Direct3d11: stencil index 3 is SO_IncrementSaturate and must saturate."));
	FATAL(cms_stencilOperation[6] != D3D11_STENCIL_OP_INCR, ("Direct3d11: stencil index 6 is SO_IncrementWrap and must wrap. The shadow volume counting passes depend on wrapping."));
}

// ======================================================================

D3D11_COMPARISON_FUNC Direct3d11_StateTables::getCompare(int engineCompare)
{
	DEBUG_FATAL(engineCompare < 0 || engineCompare >= cms_compareCount, ("Direct3d11: compare index %d out of range", engineCompare));
	return cms_compare[engineCompare];
}

// ----------------------------------------------------------------------

D3D11_BLEND Direct3d11_StateTables::getBlend(int engineBlend)
{
	DEBUG_FATAL(engineBlend < 0 || engineBlend >= cms_blendCount, ("Direct3d11: blend index %d out of range", engineBlend));
	return cms_blend[engineBlend];
}

// ----------------------------------------------------------------------

D3D11_BLEND Direct3d11_StateTables::getAlphaChannelBlend(D3D11_BLEND colorBlend)
{
	// Only the five colour-reading factors need renaming; every other factor is already legal
	// on the alpha channel and passes through unchanged. SRC1 is dual-source blending, which
	// nothing in this engine uses, but it is in the table because omitting it would leave the
	// same E_INVALIDARG waiting for whoever adds it.
	switch (colorBlend)
	{
		case D3D11_BLEND_SRC_COLOR:
			return D3D11_BLEND_SRC_ALPHA;
		case D3D11_BLEND_INV_SRC_COLOR:
			return D3D11_BLEND_INV_SRC_ALPHA;
		case D3D11_BLEND_DEST_COLOR:
			return D3D11_BLEND_DEST_ALPHA;
		case D3D11_BLEND_INV_DEST_COLOR:
			return D3D11_BLEND_INV_DEST_ALPHA;
		case D3D11_BLEND_SRC1_COLOR:
			return D3D11_BLEND_SRC1_ALPHA;
		case D3D11_BLEND_INV_SRC1_COLOR:
			return D3D11_BLEND_INV_SRC1_ALPHA;
		default:
			return colorBlend;
	}
}

// ----------------------------------------------------------------------

D3D11_BLEND_OP Direct3d11_StateTables::getBlendOperation(int engineBlendOperation)
{
	DEBUG_FATAL(engineBlendOperation < 0 || engineBlendOperation >= cms_blendOperationCount, ("Direct3d11: blend operation index %d out of range", engineBlendOperation));
	return cms_blendOperation[engineBlendOperation];
}

// ----------------------------------------------------------------------

D3D11_STENCIL_OP Direct3d11_StateTables::getStencilOperation(int engineStencilOperation)
{
	DEBUG_FATAL(engineStencilOperation < 0 || engineStencilOperation >= cms_stencilOperationCount, ("Direct3d11: stencil operation index %d out of range", engineStencilOperation));
	return cms_stencilOperation[engineStencilOperation];
}

// ----------------------------------------------------------------------

D3D11_TEXTURE_ADDRESS_MODE Direct3d11_StateTables::getTextureAddress(int engineTextureAddress)
{
	DEBUG_FATAL(engineTextureAddress < 0 || engineTextureAddress >= cms_textureAddressCount, ("Direct3d11: texture address index %d out of range", engineTextureAddress));
	return cms_textureAddress[engineTextureAddress];
}

// ----------------------------------------------------------------------

bool Direct3d11_StateTables::isMipFilterDisabled(int engineMipFilter)
{
	DEBUG_FATAL(engineMipFilter < 0 || engineMipFilter >= cms_filterKindCount, ("Direct3d11: mip filter index %d out of range", engineMipFilter));
	return cms_filterKind[engineMipFilter] == FK_none;
}

// ----------------------------------------------------------------------
/**
 * Compose min, mag and mip into one D3D11_FILTER.
 *
 * D3D11 has no independent min/mag/mip filter states, so the eight legal
 * combinations of point and linear are enumerated explicitly. Anisotropy in D3D11
 * is all-or-nothing across min and mag, so it is selected when either asks for
 * it, matching what D3D9 did in practice.
 *
 * TF_none as a MIP filter means no mipmapping, which is expressed by clamping the
 * LOD range rather than by a filter bit -- the caller handles that via
 * isMipFilterDisabled. Here it degrades to point so the value is always legal.
 */

D3D11_FILTER Direct3d11_StateTables::getFilter(int engineMinFilter, int engineMagFilter, int engineMipFilter, bool &isAnisotropic)
{
	DEBUG_FATAL(engineMinFilter < 0 || engineMinFilter >= cms_filterKindCount, ("Direct3d11: min filter index %d out of range", engineMinFilter));
	DEBUG_FATAL(engineMagFilter < 0 || engineMagFilter >= cms_filterKindCount, ("Direct3d11: mag filter index %d out of range", engineMagFilter));
	DEBUG_FATAL(engineMipFilter < 0 || engineMipFilter >= cms_filterKindCount, ("Direct3d11: mip filter index %d out of range", engineMipFilter));

	FilterKind const minKind = cms_filterKind[engineMinFilter];
	FilterKind const magKind = cms_filterKind[engineMagFilter];
	FilterKind const mipKind = cms_filterKind[engineMipFilter];

	isAnisotropic = (minKind == FK_anisotropic || magKind == FK_anisotropic);
	if (isAnisotropic)
		return D3D11_FILTER_ANISOTROPIC;

	bool const minLinear = (minKind == FK_linear);
	bool const magLinear = (magKind == FK_linear);
	bool const mipLinear = (mipKind == FK_linear);

	if (!minLinear && !magLinear && !mipLinear)
		return D3D11_FILTER_MIN_MAG_MIP_POINT;
	if (!minLinear && !magLinear && mipLinear)
		return D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
	if (!minLinear && magLinear && !mipLinear)
		return D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
	if (!minLinear && magLinear && mipLinear)
		return D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
	if (minLinear && !magLinear && !mipLinear)
		return D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
	if (minLinear && !magLinear && mipLinear)
		return D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
	if (minLinear && magLinear && !mipLinear)
		return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;

	return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
}

// ======================================================================
