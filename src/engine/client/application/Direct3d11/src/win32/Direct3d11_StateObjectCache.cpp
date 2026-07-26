// ======================================================================
//
// Direct3d11_StateObjectCache.cpp
// copyright (c) 2026 Galaxies Reborn
//
// ======================================================================

#include "FirstDirect3d11.h"
#include "Direct3d11_StateObjectCache.h"

#include "Direct3d11_Device.h"
#include "Direct3d11_Metrics.h"
#include "Direct3d11_StateTables.h"

#include "clientGraphics/Graphics.def"

#include <map>

// ======================================================================

namespace Direct3d11_StateObjectCacheNamespace
{
	// Comparison by raw bytes. The descriptors are PODs with no padding holes
	// that matter, because every one of them is zeroed before being filled.
	template <typename Description>
	struct ByteLess
	{
		bool operator()(Description const &lhs, Description const &rhs) const
		{
			return memcmp(&lhs, &rhs, sizeof(Description)) < 0;
		}
	};

	typedef std::map<D3D11_BLEND_DESC, ID3D11BlendState *, ByteLess<D3D11_BLEND_DESC> >                 BlendMap;
	typedef std::map<D3D11_DEPTH_STENCIL_DESC, ID3D11DepthStencilState *, ByteLess<D3D11_DEPTH_STENCIL_DESC> > DepthStencilMap;
	typedef std::map<D3D11_SAMPLER_DESC, ID3D11SamplerState *, ByteLess<D3D11_SAMPLER_DESC> >           SamplerMap;

	BlendMap         ms_blendStates;
	DepthStencilMap  ms_depthStencilStates;
	SamplerMap       ms_samplerStates;

	// 2 fill modes x 3 cull modes x 2 scissor states, indexed rather than hashed.
	int const cms_fillModeCount = 2;
	int const cms_cullModeCount = 3;
	int const cms_scissorCount  = 2;
	ID3D11RasterizerState *ms_rasterizerStates[cms_fillModeCount][cms_cullModeCount][cms_scissorCount];

	bool ms_installed;

	void createRasterizerStates();
}
using namespace Direct3d11_StateObjectCacheNamespace;

// ======================================================================
/**
 * Pre-create every rasterizer state the engine can ask for.
 *
 * Fill, cull and scissor-enable are engine state that changes during a frame:
 * Graphics issues setCullMode in save/restore pairs around shadow volumes and
 * ribbons, and scissoring toggles per UI element. Twelve objects created once is
 * the whole cost of never creating one inside a frame.
 */

void Direct3d11_StateObjectCacheNamespace::createRasterizerStates()
{
	ID3D11Device1 * const device = Direct3d11_Device::getDevice();
	NOT_NULL(device);

	for (int fill = 0; fill < cms_fillModeCount; ++fill)
		for (int cull = 0; cull < cms_cullModeCount; ++cull)
			for (int scissor = 0; scissor < cms_scissorCount; ++scissor)
			{
				D3D11_RASTERIZER_DESC description;
				Zero(description);

				description.FillMode = (fill == GFM_wire) ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;

				// GlCullMode names the winding that is CULLED, not the front face.
				// With FrontCounterClockwise FALSE, D3D11's "front" is clockwise, so
				// GCM_clockwise -- cull clockwise -- is CULL_FRONT. Getting this
				// backwards makes the world invisible rather than visibly wrong,
				// which is a much harder bug to recognise.
				switch (cull)
				{
					case GCM_none:             description.CullMode = D3D11_CULL_NONE;  break;
					case GCM_clockwise:        description.CullMode = D3D11_CULL_FRONT; break;
					case GCM_counterClockwise: description.CullMode = D3D11_CULL_BACK;  break;
					default:                   description.CullMode = D3D11_CULL_BACK;  break;
				}

				description.FrontCounterClockwise = FALSE;
				description.DepthClipEnable       = TRUE;
				description.ScissorEnable         = (scissor != 0) ? TRUE : FALSE;
				description.MultisampleEnable     = FALSE;
				description.AntialiasedLineEnable = FALSE;

				HRESULT const hresult = device->CreateRasterizerState(&description, &ms_rasterizerStates[fill][cull][scissor]);

				// Fatal rather than cached as null. Binding a null rasterizer state
				// silently reverts the device to its default -- solid, cull back,
				// no scissor -- which is a plausible-looking image that is not the
				// one that was asked for.
				FATAL(FAILED(hresult) || !ms_rasterizerStates[fill][cull][scissor], ("Direct3d11: rasterizer state fill %d cull %d scissor %d could not be created (%s).", fill, cull, scissor, Direct3d11_Device::describeHresult(hresult)));

				++Direct3d11_Metrics::stateObjectCreations;
			}
}

// ======================================================================

void Direct3d11_StateObjectCache::install()
{
	DEBUG_FATAL(ms_installed, ("Direct3d11_StateObjectCache::install called twice"));

	Direct3d11_StateTables::verifyTables();
	createRasterizerStates();

	ms_installed = true;
}

// ----------------------------------------------------------------------

void Direct3d11_StateObjectCache::remove()
{
	for (BlendMap::iterator i = ms_blendStates.begin(); i != ms_blendStates.end(); ++i)
		if (i->second)
			i->second->Release();
	ms_blendStates.clear();

	for (DepthStencilMap::iterator i = ms_depthStencilStates.begin(); i != ms_depthStencilStates.end(); ++i)
		if (i->second)
			i->second->Release();
	ms_depthStencilStates.clear();

	for (SamplerMap::iterator i = ms_samplerStates.begin(); i != ms_samplerStates.end(); ++i)
		if (i->second)
			i->second->Release();
	ms_samplerStates.clear();

	for (int fill = 0; fill < cms_fillModeCount; ++fill)
		for (int cull = 0; cull < cms_cullModeCount; ++cull)
			for (int scissor = 0; scissor < cms_scissorCount; ++scissor)
				if (ms_rasterizerStates[fill][cull][scissor])
				{
					ms_rasterizerStates[fill][cull][scissor]->Release();
					ms_rasterizerStates[fill][cull][scissor] = NULL;
				}

	ms_installed = false;
}

// ======================================================================

ID3D11BlendState *Direct3d11_StateObjectCache::getBlendState(D3D11_BLEND_DESC const &description)
{
	BlendMap::const_iterator const i = ms_blendStates.find(description);
	if (i != ms_blendStates.end())
		return i->second;

	FATAL(static_cast<int>(ms_blendStates.size()) >= MAX_BLEND_STATES, ("Direct3d11: the blend state cache is full at %d entries. Something dynamic has leaked into the key -- this is a design defect, not a capacity to raise.", MAX_BLEND_STATES));

	ID3D11Device1 * const device = Direct3d11_Device::getDevice();
	NOT_NULL(device);

	ID3D11BlendState *state = NULL;
	HRESULT const hresult = device->CreateBlendState(&description, &state);
	FATAL(FAILED(hresult) || !state, ("Direct3d11: CreateBlendState failed (%s). Binding null instead would silently revert to no blending.", Direct3d11_Device::describeHresult(hresult)));

	++Direct3d11_Metrics::stateObjectCreations;
	IGNORE_RETURN(ms_blendStates.insert(std::make_pair(description, state)));
	return state;
}

// ----------------------------------------------------------------------

ID3D11DepthStencilState *Direct3d11_StateObjectCache::getDepthStencilState(D3D11_DEPTH_STENCIL_DESC const &description)
{
	DepthStencilMap::const_iterator const i = ms_depthStencilStates.find(description);
	if (i != ms_depthStencilStates.end())
		return i->second;

	FATAL(static_cast<int>(ms_depthStencilStates.size()) >= MAX_DEPTH_STENCIL_STATES, ("Direct3d11: the depth-stencil state cache is full at %d entries. Something dynamic has leaked into the key.", MAX_DEPTH_STENCIL_STATES));

	ID3D11Device1 * const device = Direct3d11_Device::getDevice();
	NOT_NULL(device);

	ID3D11DepthStencilState *state = NULL;
	HRESULT const hresult = device->CreateDepthStencilState(&description, &state);

	// This one is the most dangerous to paper over. The D3D11 default has depth
	// testing ON, writes ON and LESS -- so a null bind would break every
	// depth-disabled sky pass and every z-fail stencil shadow, and it would look
	// like a content bug.
	FATAL(FAILED(hresult) || !state, ("Direct3d11: CreateDepthStencilState failed (%s). Binding null instead would enable depth testing and writing, breaking the sky passes and the stencil shadows.", Direct3d11_Device::describeHresult(hresult)));

	++Direct3d11_Metrics::stateObjectCreations;
	IGNORE_RETURN(ms_depthStencilStates.insert(std::make_pair(description, state)));
	return state;
}

// ----------------------------------------------------------------------

ID3D11SamplerState *Direct3d11_StateObjectCache::getSamplerState(D3D11_SAMPLER_DESC const &description)
{
	SamplerMap::const_iterator const i = ms_samplerStates.find(description);
	if (i != ms_samplerStates.end())
		return i->second;

	FATAL(static_cast<int>(ms_samplerStates.size()) >= MAX_SAMPLER_STATES, ("Direct3d11: the sampler state cache is full at %d entries. Something dynamic has leaked into the key.", MAX_SAMPLER_STATES));

	ID3D11Device1 * const device = Direct3d11_Device::getDevice();
	NOT_NULL(device);

	ID3D11SamplerState *state = NULL;
	HRESULT const hresult = device->CreateSamplerState(&description, &state);
	FATAL(FAILED(hresult) || !state, ("Direct3d11: CreateSamplerState failed (%s).", Direct3d11_Device::describeHresult(hresult)));

	++Direct3d11_Metrics::stateObjectCreations;
	IGNORE_RETURN(ms_samplerStates.insert(std::make_pair(description, state)));
	return state;
}

// ----------------------------------------------------------------------

ID3D11RasterizerState *Direct3d11_StateObjectCache::getRasterizerState(int fillMode, int cullMode, bool scissorEnabled)
{
	DEBUG_FATAL(fillMode < 0 || fillMode >= cms_fillModeCount, ("Direct3d11: fill mode %d out of range", fillMode));
	DEBUG_FATAL(cullMode < 0 || cullMode >= cms_cullModeCount, ("Direct3d11: cull mode %d out of range", cullMode));

	return ms_rasterizerStates[fillMode][cullMode][scissorEnabled ? 1 : 0];
}

// ======================================================================

int Direct3d11_StateObjectCache::getBlendStateCount()
{
	return static_cast<int>(ms_blendStates.size());
}

// ----------------------------------------------------------------------

int Direct3d11_StateObjectCache::getDepthStencilStateCount()
{
	return static_cast<int>(ms_depthStencilStates.size());
}

// ----------------------------------------------------------------------

int Direct3d11_StateObjectCache::getRasterizerStateCount()
{
	return cms_fillModeCount * cms_cullModeCount * cms_scissorCount;
}

// ----------------------------------------------------------------------

int Direct3d11_StateObjectCache::getSamplerStateCount()
{
	return static_cast<int>(ms_samplerStates.size());
}

// ======================================================================
