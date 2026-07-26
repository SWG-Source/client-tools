// ======================================================================
//
// Direct3d11_ShaderImplementationData.cpp
// copyright (c) 2026 Galaxies Reborn
//
// ======================================================================

#include "FirstDirect3d11.h"
#include "Direct3d11_ShaderImplementationData.h"

#include "Direct3d11_Metrics.h"
#include "Direct3d11_PixelShaderProgramData.h"
#include "Direct3d11_StateCache.h"
#include "Direct3d11_StateObjectCache.h"
#include "Direct3d11_StateTables.h"

#include <vector>

// ======================================================================

namespace Direct3d11_ShaderImplementationDataNamespace
{
	int  ms_liveInstanceCount;
	bool ms_installed;

	// Reported once each rather than per pass. Both are real gaps with designed answers;
	// what they must not be is silent, and what they equally must not be is a warning per
	// implementation across a corpus of seventeen thousand shaders.
	bool ms_reportedFlatShading;

	float const cms_noBlendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
}
using namespace Direct3d11_ShaderImplementationDataNamespace;

// ======================================================================

void Direct3d11_ShaderImplementationData::install()
{
	DEBUG_FATAL(ms_installed, ("Direct3d11_ShaderImplementationData::install called twice"));
	ms_installed = true;

	ms_reportedFlatShading = false;
}

// ----------------------------------------------------------------------

void Direct3d11_ShaderImplementationData::remove()
{
	DEBUG_WARNING(ms_liveInstanceCount != 0, ("Direct3d11: %d shader implementation(s) are still alive at shutdown.", ms_liveInstanceCount));
	ms_installed = false;
}

// ----------------------------------------------------------------------

int Direct3d11_ShaderImplementationData::getLiveInstanceCount()
{
	return ms_liveInstanceCount;
}

// ======================================================================

Direct3d11_ShaderImplementationData::Direct3d11_ShaderImplementationData(ShaderImplementation const &implementation)
:
	ShaderImplementationGraphicsData(),
	m_passes(NULL),
	m_passCount(0)
{
	++ms_liveInstanceCount;

	// m_pass is private on ShaderImplementation and there is no accessor; the engine names
	// this class a friend for exactly this, as it does the DX9 one.
	int const passCount = implementation.m_pass ? static_cast<int>(implementation.m_pass->size()) : 0;
	if (passCount <= 0)
		return;

	m_passes = new Pass[passCount];
	m_passCount = passCount;

	for (int i = 0; i < passCount; ++i)
	{
		ShaderImplementation::Pass const &source = *(*implementation.m_pass)[static_cast<size_t>(i)];
		Pass &pass = m_passes[i];

		pass.blendState = NULL;
		pass.depthStencilState = NULL;
		pass.pixelShaderProgram = NULL;
		pass.alphaBlendEnable = source.m_alphaBlendEnable;
		pass.alphaTestEnable = source.m_alphaTestEnable;
		pass.alphaTestFunction = static_cast<int>(source.m_alphaTestFunction);

		// ----------------------------------------------------------
		// Blend

		D3D11_BLEND_DESC blend;
		Zero(blend);

		// Independent per-target blending is off: this engine has one render target, and
		// leaving it off means target 0's description applies to all of them, which is what
		// D3D9's single set of blend states meant.
		blend.IndependentBlendEnable = FALSE;
		blend.AlphaToCoverageEnable = FALSE;

		D3D11_RENDER_TARGET_BLEND_DESC &target = blend.RenderTarget[0];
		target.BlendEnable = source.m_alphaBlendEnable ? TRUE : FALSE;
		target.SrcBlend  = Direct3d11_StateTables::getBlend(static_cast<int>(source.m_alphaBlendSource));
		target.DestBlend = Direct3d11_StateTables::getBlend(static_cast<int>(source.m_alphaBlendDestination));
		target.BlendOp   = Direct3d11_StateTables::getBlendOperation(static_cast<int>(source.m_alphaBlendOperation));

		// D3D9 had one blend equation covering colour and alpha; D3D11 splits them. The alpha
		// equation therefore has to be set from the colour one, because leaving the alpha fields
		// at their D3D11 defaults would blend alpha as ONE/ZERO and quietly change every
		// translucent surface's destination alpha.
		//
		// But it cannot be a straight copy, and this is where the first run of the client
		// stopped: D3D11 REJECTS the *_COLOR blend factors on the alpha channel outright, with
		// E_INVALIDARG out of CreateBlendState. The alpha channel has no colour to read, so
		// D3D11_BLEND_SRC_COLOR and its four relatives are simply not legal there.
		//
		// The mapping below is what D3D9 meant. D3D9 applied one factor to all four channels, so
		// D3DBLEND_SRCCOLOR read the source colour per channel -- and for the alpha channel, the
		// source's alpha IS its per-channel value. So SRC_COLOR becomes SRC_ALPHA and so on: not
		// an approximation, the same arithmetic named the way D3D11 requires.
		target.SrcBlendAlpha  = Direct3d11_StateTables::getAlphaChannelBlend(target.SrcBlend);
		target.DestBlendAlpha = Direct3d11_StateTables::getAlphaChannelBlend(target.DestBlend);
		target.BlendOpAlpha   = target.BlendOp;

		// The engine's write-enable mask is already the D3D9 COLORWRITEENABLE bit layout,
		// which is bit for bit the D3D11 one: red 1, green 2, blue 4, alpha 8.
		target.RenderTargetWriteMask = static_cast<UINT8>(source.m_writeEnable & 0x0f);

		pass.blendState = Direct3d11_StateObjectCache::getBlendState(blend);

		// ----------------------------------------------------------
		// Depth and stencil

		D3D11_DEPTH_STENCIL_DESC depth;
		Zero(depth);

		depth.DepthEnable    = source.m_zEnable ? TRUE : FALSE;
		depth.DepthWriteMask = source.m_zWrite ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
		depth.DepthFunc      = Direct3d11_StateTables::getCompare(static_cast<int>(source.m_zCompare));

		depth.StencilEnable    = source.m_stencilEnable ? TRUE : FALSE;
		depth.StencilReadMask  = static_cast<UINT8>(source.m_stencilMask & 0xff);
		depth.StencilWriteMask = static_cast<UINT8>(source.m_stencilWriteMask & 0xff);

		depth.FrontFace.StencilFunc        = Direct3d11_StateTables::getCompare(static_cast<int>(source.m_stencilCompareFunction));
		depth.FrontFace.StencilPassOp      = Direct3d11_StateTables::getStencilOperation(static_cast<int>(source.m_stencilPassOperation));
		depth.FrontFace.StencilDepthFailOp = Direct3d11_StateTables::getStencilOperation(static_cast<int>(source.m_stencilZFailOperation));
		depth.FrontFace.StencilFailOp      = Direct3d11_StateTables::getStencilOperation(static_cast<int>(source.m_stencilFailOperation));

		if (source.m_stencilTwoSidedMode)
		{
			depth.BackFace.StencilFunc        = Direct3d11_StateTables::getCompare(static_cast<int>(source.m_stencilCounterClockwiseCompareFunction));
			depth.BackFace.StencilPassOp      = Direct3d11_StateTables::getStencilOperation(static_cast<int>(source.m_stencilCounterClockwisePassOperation));
			depth.BackFace.StencilDepthFailOp = Direct3d11_StateTables::getStencilOperation(static_cast<int>(source.m_stencilCounterClockwiseZFailOperation));
			depth.BackFace.StencilFailOp      = Direct3d11_StateTables::getStencilOperation(static_cast<int>(source.m_stencilCounterClockwiseFailOperation));
		}
		else
		{
			// D3D9's two-sided stencil is a switch: with it off, the CCW states are ignored
			// and both faces use the front-face operations. D3D11 has no such switch and
			// always consults BackFace, so the front-face description is copied into it.
			// Leaving BackFace zeroed would give it comparison function 0, which is not a
			// valid D3D11_COMPARISON_FUNC, and the state object creation would fail.
			depth.BackFace = depth.FrontFace;
		}

		pass.depthStencilState = Direct3d11_StateObjectCache::getDepthStencilState(depth);

		// ----------------------------------------------------------
		// The pixel program

		if (source.m_pixelShader)
			pass.pixelShaderProgram = source.m_pixelShader->m_program;

		// ----------------------------------------------------------
		// The two states D3D11 cannot express

		if (source.m_shadeMode == ShaderImplementation::Pass::SM_Flat && !ms_reportedFlatShading)
		{
			ms_reportedFlatShading = true;
			WARNING(true, ("Direct3d11: a shader pass asks for flat shading, which in D3D11 is a nointerpolation modifier on the shader output rather than a state, so it cannot be honoured without a compiled variant. Those surfaces will be smooth shaded. Reported once."));
		}
	}
}

// ----------------------------------------------------------------------

Direct3d11_ShaderImplementationData::~Direct3d11_ShaderImplementationData()
{
	--ms_liveInstanceCount;

	// The state objects belong to Direct3d11_StateObjectCache and are shared between every
	// pass that resolved to the same description, so they are not released here.
	delete [] m_passes;
	m_passes = NULL;
	m_passCount = 0;
}

// ======================================================================

bool Direct3d11_ShaderImplementationData::isAlphaTestEnabled(int passNumber) const
{
	if (passNumber < 0 || passNumber >= m_passCount)
		return false;

	return m_passes[passNumber].alphaTestEnable;
}

// ----------------------------------------------------------------------

int Direct3d11_ShaderImplementationData::getAlphaTestFunction(int passNumber) const
{
	if (passNumber < 0 || passNumber >= m_passCount)
		return static_cast<int>(ShaderImplementation::Pass::C_Always);

	return m_passes[passNumber].alphaTestFunction;
}

// ----------------------------------------------------------------------

bool Direct3d11_ShaderImplementationData::isAlphaBlendEnabled(int passNumber) const
{
	if (passNumber < 0 || passNumber >= m_passCount)
		return false;

	return m_passes[passNumber].alphaBlendEnable;
}

// ----------------------------------------------------------------------

void Direct3d11_ShaderImplementationData::apply(int passNumber, uint32 stencilReference) const
{
	DEBUG_FATAL(passNumber < 0 || passNumber >= m_passCount, ("Direct3d11: pass %d applied on an implementation with %d pass(es).", passNumber, m_passCount));
	if (passNumber < 0 || passNumber >= m_passCount)
		return;

	Pass const &pass = m_passes[passNumber];

	// The blend factor is unused: no engine blend mode names BLENDFACTOR, so any constant
	// value is equivalent and a fixed one keeps the state cache's comparison meaningful.
	Direct3d11_StateCache::setBlendState(pass.blendState, cms_noBlendFactor, 0xffffffff);
	Direct3d11_StateCache::setDepthStencilState(pass.depthStencilState, stencilReference);

	ID3D11PixelShader *pixelShader = NULL;
	if (pass.pixelShaderProgram)
	{
		Direct3d11_PixelShaderProgramData const * const data =
			static_cast<Direct3d11_PixelShaderProgramData const *>(pass.pixelShaderProgram->m_graphicsData);

		if (data)
			pixelShader = data->getPixelShader();
	}

	// A null is bound rather than skipped. Leaving the previous pass's pixel shader in place
	// would draw this pass with the wrong program, which looks like a shading bug a long way
	// from its cause; a null draws nothing, and the draw path counts it.
	Direct3d11_StateCache::setPixelShader(pixelShader);
}

// ======================================================================
