// ======================================================================
//
// Direct3d11_SceneTarget.cpp
// copyright (c) 2026 Galaxies Reborn
//
// ======================================================================

#include "FirstDirect3d11.h"
#include "Direct3d11_SceneTarget.h"

#include "ConfigDirect3d11.h"
#include "Direct3d11_Device.h"
#include "Direct3d11_Metrics.h"
#include "Direct3d11_StateCache.h"

#include <d3dcompiler.h>
#include <math.h>

// ======================================================================

namespace Direct3d11_SceneTargetNamespace
{
	int                        ms_width;
	int                        ms_height;
	int                        ms_sampleCount = 1;

	// What was asked for, as opposed to what was achieved. Seeded from the config at install and
	// then owned by setAntialiasEnabled, so that a rebuild does not silently undo a runtime
	// toggle by re-reading the config.
	bool                       ms_antialiasRequested = false;

	ID3D11Texture2D           *ms_colorBuffer;
	ID3D11RenderTargetView    *ms_colorView;
	ID3D11ShaderResourceView  *ms_colorResourceView;

	// Only allocated when multisampling is on: the multisampled colour buffer
	// cannot be sampled, so it resolves into this one before the composite.
	ID3D11Texture2D           *ms_resolvedBuffer;
	ID3D11ShaderResourceView  *ms_resolvedResourceView;

	ID3D11Texture2D           *ms_depthBuffer;
	ID3D11DepthStencilView    *ms_depthView;
	DXGI_FORMAT                ms_depthFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	ID3D11Texture2D           *ms_readbackBuffer;

	ID3D11VertexShader        *ms_compositeVertexShader;

	// Two pixel shaders rather than one with a branch. The copy path has to be
	// provably free of arithmetic on the pixel values, and "provably" means there
	// is no code in it that could do any.
	ID3D11PixelShader         *ms_compositeCopyShader;
	ID3D11PixelShader         *ms_compositeCorrectShader;
	ID3D11SamplerState        *ms_pointSampler;
	ID3D11Texture2D           *ms_colorCorrectionTexture;
	ID3D11ShaderResourceView  *ms_colorCorrectionResourceView;
	ID3D11DepthStencilState   *ms_compositeDepthState;
	ID3D11BlendState          *ms_compositeBlendState;
	ID3D11RasterizerState     *ms_compositeRasterizerState;

	float                      ms_brightness = 1.0f;
	float                      ms_contrast = 1.0f;
	float                      ms_gamma = 1.0f;
	bool                       ms_colorCorrectionIsIdentity = true;
	uint8                      ms_colorCorrectionTable[256];

	bool                       createBuffers();
	void                       releaseBuffers();
	bool                       createCompositeResources();
	void                       releaseCompositeResources();
	void                       uploadColorCorrectionTable();

	// The composite's own shader. This is backend-owned code, not a game asset:
	// it never reads a .sht, never participates in the shader corpus, and has no
	// permutations. It stays inline so there is exactly one copy of it and no
	// build step to keep in sync.
	//
	// A full-screen triangle from SV_VertexID rather than a quad from a vertex
	// buffer: three vertices, no input layout, no buffer to bind, and no
	// dependency on any of the resource machinery that does not exist yet.
	//
	// Colour correction is a 256-entry lookup sampled POINT, not a pow() per
	// pixel. That is deliberate -- DX9 quantises the curve into 8 bits once and
	// applies it as a table, so evaluating it per pixel in floating point would
	// produce values DX9 cannot produce.
	char const * const cms_compositeShaderSource =
		"Texture2D    SceneTexture           : register(t0);\n"
		"Texture2D    ColorCorrectionTexture : register(t1);\n"
		"SamplerState PointSampler           : register(s0);\n"
		"\n"
		"struct Vertex\n"
		"{\n"
		"	float4 position : SV_Position;\n"
		"	float2 texCoord : TEXCOORD0;\n"
		"};\n"
		"\n"
		"Vertex vertexMain(uint vertexId : SV_VertexID)\n"
		"{\n"
		"	Vertex output;\n"
		"	float2 corner = float2((vertexId << 1) & 2, vertexId & 2);\n"
		"	output.position = float4(corner * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);\n"
		"	output.texCoord = corner;\n"
		"	return output;\n"
		"}\n"
		"\n"
		"float4 pixelMainCopy(Vertex input) : SV_Target\n"
		"{\n"
		"	return float4(SceneTexture.Sample(PointSampler, input.texCoord).rgb, 1.0f);\n"
		"}\n"
		"\n"
		"float4 pixelMainCorrect(Vertex input) : SV_Target\n"
		"{\n"
		"	float3 scene = SceneTexture.Sample(PointSampler, input.texCoord).rgb;\n"
		"	// Index the table the same way DX9 does: by the 8-bit channel value.\n"
		"	float3 corrected;\n"
		"	corrected.r = ColorCorrectionTexture.Sample(PointSampler, float2((floor(scene.r * 255.0f + 0.5f) + 0.5f) / 256.0f, 0.5f)).r;\n"
		"	corrected.g = ColorCorrectionTexture.Sample(PointSampler, float2((floor(scene.g * 255.0f + 0.5f) + 0.5f) / 256.0f, 0.5f)).r;\n"
		"	corrected.b = ColorCorrectionTexture.Sample(PointSampler, float2((floor(scene.b * 255.0f + 0.5f) + 0.5f) / 256.0f, 0.5f)).r;\n"
		"	return float4(corrected, 1.0f);\n"
		"}\n";
}
using namespace Direct3d11_SceneTargetNamespace;

// ======================================================================
/**
 * Build the colour correction table exactly as D3D9 does.
 *
 * Direct3d9.cpp:2080-2099, reproduced arithmetic for arithmetic:
 *
 *   f steps by 1/256, so entry i is driven by i/256 and NOT by i/255
 *   result = pow(0.5 + contrast * ((f * brightness) - 0.5), 1 / gamma)
 *   entry  = clamp(0, int(result * 255), 255)
 *
 * The 1/256 step matters. At the default settings the table is NOT the identity:
 * entry i becomes floor(i * 255 / 256), so 255 maps to 254 and 1 maps to 0. Any
 * "obvious" i/255 formulation would produce a different table and a screenshot
 * that cannot be compared against DX9's.
 *
 * Where DX9 evaluates pow() on a negative base -- reachable with contrast above
 * 1 at the dark end -- it gets a NaN and casts it to int, which is undefined.
 * The base is clamped to zero here instead. That is the one intentional
 * difference, and it only affects inputs where DX9's own result is not defined.
 */

void Direct3d11_SceneTarget::setBrightnessContrastGamma(float brightness, float contrast, float gamma)
{
	ms_brightness = brightness;
	ms_contrast   = contrast;
	ms_gamma      = gamma;

	// Identity is a property of the SETTINGS, not of the table. Verified against
	// DX9's arithmetic: at brightness = contrast = gamma = 1 the table is
	// floor(i * 255 / 256), so 255 of its 256 entries differ from the identity --
	// 255 becomes 254, 128 becomes 127, 1 becomes 0. Anyone who "optimises" this
	// check by testing whether the table is the identity will never skip the pass,
	// and every frame will come out a step darker than DX9's.
	ms_colorCorrectionIsIdentity = (brightness == 1.0f && contrast == 1.0f && gamma == 1.0f);

	float const step = 1.0f / 256.0f;
	float const oneOverGamma = (gamma != 0.0f) ? (1.0f / gamma) : 1.0f;

	float f = 0.0f;
	for (int i = 0; i < 256; ++i, f += step)
	{
		float base = 0.5f + contrast * ((f * brightness) - 0.5f);
		if (base < 0.0f)
			base = 0.0f;

		float const result = powf(base, oneOverGamma);

		int value = static_cast<int>(result * 255.0f);
		if (value < 0)
			value = 0;
		if (value > 255)
			value = 255;

		ms_colorCorrectionTable[i] = static_cast<uint8>(value);
	}

	if (ms_colorCorrectionTexture)
		uploadColorCorrectionTable();
}

// ----------------------------------------------------------------------

uint8 const *Direct3d11_SceneTarget::getColorCorrectionTable()
{
	return ms_colorCorrectionTable;
}

// ----------------------------------------------------------------------

bool Direct3d11_SceneTarget::isColorCorrectionIdentity()
{
	return ms_colorCorrectionIsIdentity;
}

// ======================================================================

bool Direct3d11_SceneTargetNamespace::createBuffers()
{
	ID3D11Device1 * const device = Direct3d11_Device::getDevice();
	NOT_NULL(device);

	// Multisampling defaults off, matching what the DX9 build effectively does:
	// its present parameters set D3DMULTISAMPLE_NONE in both windowed and
	// fullscreen paths, so nothing has been antialiased for twenty years and
	// turning it on is a change to measure, not a default to assume.
	ms_sampleCount = ms_antialiasRequested ? ConfigDirect3d11::getAntiAliasSampleCount() : 1;
	if (ms_sampleCount > 1 && !Direct3d11_Device::isSampleCountSupported(ms_sampleCount))
	{
		WARNING(true, ("Direct3d11: %dx multisampling is not supported for the scene target format, falling back to none.", ms_sampleCount));
		ms_sampleCount = 1;
	}

	D3D11_TEXTURE2D_DESC colorDescription;
	Zero(colorDescription);
	colorDescription.Width      = static_cast<UINT>(ms_width);
	colorDescription.Height     = static_cast<UINT>(ms_height);
	colorDescription.MipLevels  = 1;
	colorDescription.ArraySize  = 1;
	// Plain UNORM, never _SRGB. Every shipped .dds carries no colour space
	// metadata and the engine has lit in gamma space throughout its life; an
	// sRGB view would change every texture at once and be indistinguishable from
	// a port defect.
	colorDescription.Format     = DXGI_FORMAT_R8G8B8A8_UNORM;
	colorDescription.SampleDesc.Count = static_cast<UINT>(ms_sampleCount);
	colorDescription.Usage      = D3D11_USAGE_DEFAULT;
	colorDescription.BindFlags  = D3D11_BIND_RENDER_TARGET | ((ms_sampleCount > 1) ? 0 : D3D11_BIND_SHADER_RESOURCE);

	HRESULT hresult = device->CreateTexture2D(&colorDescription, NULL, &ms_colorBuffer);
	if (FAILED(hresult) || !ms_colorBuffer)
	{
		WARNING(true, ("Direct3d11: the scene colour buffer could not be created (%s).", Direct3d11_Device::describeHresult(hresult)));
		return false;
	}

	hresult = device->CreateRenderTargetView(ms_colorBuffer, NULL, &ms_colorView);
	if (FAILED(hresult) || !ms_colorView)
	{
		WARNING(true, ("Direct3d11: the scene colour render target view could not be created (%s).", Direct3d11_Device::describeHresult(hresult)));
		return false;
	}

	if (ms_sampleCount > 1)
	{
		D3D11_TEXTURE2D_DESC resolvedDescription = colorDescription;
		resolvedDescription.SampleDesc.Count = 1;
		resolvedDescription.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		hresult = device->CreateTexture2D(&resolvedDescription, NULL, &ms_resolvedBuffer);
		if (FAILED(hresult) || !ms_resolvedBuffer)
		{
			WARNING(true, ("Direct3d11: the multisample resolve target could not be created (%s).", Direct3d11_Device::describeHresult(hresult)));
			return false;
		}

		hresult = device->CreateShaderResourceView(ms_resolvedBuffer, NULL, &ms_resolvedResourceView);
		if (FAILED(hresult) || !ms_resolvedResourceView)
		{
			WARNING(true, ("Direct3d11: the multisample resolve view could not be created (%s).", Direct3d11_Device::describeHresult(hresult)));
			return false;
		}
	}
	else
	{
		hresult = device->CreateShaderResourceView(ms_colorBuffer, NULL, &ms_colorResourceView);
		if (FAILED(hresult) || !ms_colorResourceView)
		{
			WARNING(true, ("Direct3d11: the scene colour shader resource view could not be created (%s).", Direct3d11_Device::describeHresult(hresult)));
			return false;
		}
	}

	// Stencil is required, not optional: the shadow volume passes count into it.
	DXGI_FORMAT const depthFormats[] = { DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_FORMAT_D32_FLOAT_S8X24_UINT };
	for (int i = 0; i < 2; ++i)
	{
		D3D11_TEXTURE2D_DESC depthDescription;
		Zero(depthDescription);
		depthDescription.Width      = static_cast<UINT>(ms_width);
		depthDescription.Height     = static_cast<UINT>(ms_height);
		depthDescription.MipLevels  = 1;
		depthDescription.ArraySize  = 1;
		depthDescription.Format     = depthFormats[i];
		depthDescription.SampleDesc.Count = static_cast<UINT>(ms_sampleCount);
		depthDescription.Usage      = D3D11_USAGE_DEFAULT;
		depthDescription.BindFlags  = D3D11_BIND_DEPTH_STENCIL;

		if (SUCCEEDED(device->CreateTexture2D(&depthDescription, NULL, &ms_depthBuffer)) && ms_depthBuffer)
		{
			ms_depthFormat = depthFormats[i];
			break;
		}
	}

	if (!ms_depthBuffer)
	{
		WARNING(true, ("Direct3d11: no depth-stencil format with a stencil channel could be created for the scene target."));
		return false;
	}

	hresult = device->CreateDepthStencilView(ms_depthBuffer, NULL, &ms_depthView);
	if (FAILED(hresult) || !ms_depthView)
	{
		WARNING(true, ("Direct3d11: the scene depth-stencil view could not be created (%s).", Direct3d11_Device::describeHresult(hresult)));
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------

void Direct3d11_SceneTargetNamespace::releaseBuffers()
{
	if (ms_readbackBuffer)       { ms_readbackBuffer->Release();       ms_readbackBuffer = NULL; }
	if (ms_depthView)            { ms_depthView->Release();            ms_depthView = NULL; }
	if (ms_depthBuffer)          { ms_depthBuffer->Release();          ms_depthBuffer = NULL; }
	if (ms_resolvedResourceView) { ms_resolvedResourceView->Release(); ms_resolvedResourceView = NULL; }
	if (ms_resolvedBuffer)       { ms_resolvedBuffer->Release();       ms_resolvedBuffer = NULL; }
	if (ms_colorResourceView)    { ms_colorResourceView->Release();    ms_colorResourceView = NULL; }
	if (ms_colorView)            { ms_colorView->Release();            ms_colorView = NULL; }
	if (ms_colorBuffer)          { ms_colorBuffer->Release();          ms_colorBuffer = NULL; }
}

// ----------------------------------------------------------------------

void Direct3d11_SceneTargetNamespace::uploadColorCorrectionTable()
{
	ID3D11DeviceContext1 * const context = Direct3d11_Device::getContext();
	if (!context || !ms_colorCorrectionTexture)
		return;

	// R8_UNORM, 256x1. The shader reads .r and writes it to all three channels
	// after looking each up separately, exactly as DX9 indexes one table per
	// channel.
	D3D11_BOX box;
	box.left = 0;
	box.top = 0;
	box.front = 0;
	box.right = 256;
	box.bottom = 1;
	box.back = 1;
	context->UpdateSubresource(ms_colorCorrectionTexture, 0, &box, ms_colorCorrectionTable, 256, 256);
}

// ----------------------------------------------------------------------

bool Direct3d11_SceneTargetNamespace::createCompositeResources()
{
	ID3D11Device1 * const device = Direct3d11_Device::getDevice();
	NOT_NULL(device);

	UINT const flags = D3DCOMPILE_OPTIMIZATION_LEVEL3 | D3DCOMPILE_WARNINGS_ARE_ERRORS;
	size_t const sourceLength = strlen(cms_compositeShaderSource);

	struct Compile
	{
		char const *entryPoint;
		char const *target;
	};

	ID3DBlob *vertexBlob = NULL;
	ID3DBlob *copyBlob = NULL;
	ID3DBlob *correctBlob = NULL;
	ID3DBlob *errors = NULL;

	// vs_4_0 / ps_4_0, the same profiles the asset corpus will use. There is
	// nothing in this shader that needs more, and pinning one profile keeps the
	// composite comparable across machines.
	HRESULT hresult = D3DCompile(cms_compositeShaderSource, sourceLength, "Direct3d11_Composite", NULL, NULL, "vertexMain", "vs_4_0", flags, 0, &vertexBlob, &errors);
	if (FAILED(hresult))
	{
		WARNING(true, ("Direct3d11: the composite vertex shader failed to compile (%s): %s", Direct3d11_Device::describeHresult(hresult), errors ? static_cast<char const *>(errors->GetBufferPointer()) : "no detail"));
		if (errors)
			errors->Release();
		return false;
	}

	hresult = D3DCompile(cms_compositeShaderSource, sourceLength, "Direct3d11_Composite", NULL, NULL, "pixelMainCopy", "ps_4_0", flags, 0, &copyBlob, &errors);
	if (FAILED(hresult))
	{
		WARNING(true, ("Direct3d11: the composite copy pixel shader failed to compile (%s): %s", Direct3d11_Device::describeHresult(hresult), errors ? static_cast<char const *>(errors->GetBufferPointer()) : "no detail"));
		if (errors)
			errors->Release();
		vertexBlob->Release();
		return false;
	}

	hresult = D3DCompile(cms_compositeShaderSource, sourceLength, "Direct3d11_Composite", NULL, NULL, "pixelMainCorrect", "ps_4_0", flags, 0, &correctBlob, &errors);
	if (FAILED(hresult))
	{
		WARNING(true, ("Direct3d11: the composite colour correction pixel shader failed to compile (%s): %s", Direct3d11_Device::describeHresult(hresult), errors ? static_cast<char const *>(errors->GetBufferPointer()) : "no detail"));
		if (errors)
			errors->Release();
		copyBlob->Release();
		vertexBlob->Release();
		return false;
	}

	if (errors)
		errors->Release();

	hresult = device->CreateVertexShader(vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), NULL, &ms_compositeVertexShader);
	FATAL(FAILED(hresult), ("Direct3d11: CreateVertexShader for the composite failed (%s).", Direct3d11_Device::describeHresult(hresult)));

	hresult = device->CreatePixelShader(copyBlob->GetBufferPointer(), copyBlob->GetBufferSize(), NULL, &ms_compositeCopyShader);
	FATAL(FAILED(hresult), ("Direct3d11: CreatePixelShader for the composite copy failed (%s).", Direct3d11_Device::describeHresult(hresult)));

	hresult = device->CreatePixelShader(correctBlob->GetBufferPointer(), correctBlob->GetBufferSize(), NULL, &ms_compositeCorrectShader);
	FATAL(FAILED(hresult), ("Direct3d11: CreatePixelShader for the composite correction failed (%s).", Direct3d11_Device::describeHresult(hresult)));

	vertexBlob->Release();
	copyBlob->Release();
	correctBlob->Release();

	// POINT sampling on a one-to-one copy. A LINEAR sampler on a full-screen blit
	// of matching dimensions is not free and is not a no-op at the edges.
	D3D11_SAMPLER_DESC samplerDescription;
	Zero(samplerDescription);
	samplerDescription.Filter   = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDescription.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDescription.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDescription.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDescription.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDescription.MaxLOD   = D3D11_FLOAT32_MAX;
	hresult = device->CreateSamplerState(&samplerDescription, &ms_pointSampler);
	FATAL(FAILED(hresult), ("Direct3d11: the composite sampler could not be created (%s).", Direct3d11_Device::describeHresult(hresult)));

	D3D11_TEXTURE2D_DESC tableDescription;
	Zero(tableDescription);
	tableDescription.Width     = 256;
	tableDescription.Height    = 1;
	tableDescription.MipLevels = 1;
	tableDescription.ArraySize = 1;
	tableDescription.Format    = DXGI_FORMAT_R8_UNORM;
	tableDescription.SampleDesc.Count = 1;
	tableDescription.Usage     = D3D11_USAGE_DEFAULT;
	tableDescription.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	hresult = device->CreateTexture2D(&tableDescription, NULL, &ms_colorCorrectionTexture);
	FATAL(FAILED(hresult), ("Direct3d11: the colour correction table texture could not be created (%s).", Direct3d11_Device::describeHresult(hresult)));

	hresult = device->CreateShaderResourceView(ms_colorCorrectionTexture, NULL, &ms_colorCorrectionResourceView);
	FATAL(FAILED(hresult), ("Direct3d11: the colour correction table view could not be created (%s).", Direct3d11_Device::describeHresult(hresult)));

	uploadColorCorrectionTable();

	// Depth off, no blending, no culling. The composite must not
	// inherit whatever the scene left behind.
	D3D11_DEPTH_STENCIL_DESC depthState;
	Zero(depthState);
	depthState.DepthEnable    = FALSE;
	depthState.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depthState.DepthFunc      = D3D11_COMPARISON_ALWAYS;
	depthState.StencilEnable  = FALSE;
	hresult = device->CreateDepthStencilState(&depthState, &ms_compositeDepthState);
	FATAL(FAILED(hresult), ("Direct3d11: the composite depth state could not be created (%s).", Direct3d11_Device::describeHresult(hresult)));

	D3D11_BLEND_DESC blendState;
	Zero(blendState);
	blendState.RenderTarget[0].BlendEnable = FALSE;
	blendState.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hresult = device->CreateBlendState(&blendState, &ms_compositeBlendState);
	FATAL(FAILED(hresult), ("Direct3d11: the composite blend state could not be created (%s).", Direct3d11_Device::describeHresult(hresult)));

	D3D11_RASTERIZER_DESC rasterizerState;
	Zero(rasterizerState);
	rasterizerState.FillMode = D3D11_FILL_SOLID;
	rasterizerState.CullMode = D3D11_CULL_NONE;
	rasterizerState.FrontCounterClockwise = FALSE;
	rasterizerState.DepthClipEnable = TRUE;
	hresult = device->CreateRasterizerState(&rasterizerState, &ms_compositeRasterizerState);
	FATAL(FAILED(hresult), ("Direct3d11: the composite rasterizer state could not be created (%s).", Direct3d11_Device::describeHresult(hresult)));

	// Counted even though these are install-time: the invariant is "nothing is
	// created during a frame", and a counter that is never incremented cannot
	// demonstrate that.
	Direct3d11_Metrics::stateObjectCreations += 3;
	Direct3d11_Metrics::shaderCompiles += 3;

	return true;
}

// ----------------------------------------------------------------------

void Direct3d11_SceneTargetNamespace::releaseCompositeResources()
{
	if (ms_compositeRasterizerState)    { ms_compositeRasterizerState->Release();    ms_compositeRasterizerState = NULL; }
	if (ms_compositeBlendState)         { ms_compositeBlendState->Release();         ms_compositeBlendState = NULL; }
	if (ms_compositeDepthState)         { ms_compositeDepthState->Release();         ms_compositeDepthState = NULL; }
	if (ms_colorCorrectionResourceView) { ms_colorCorrectionResourceView->Release(); ms_colorCorrectionResourceView = NULL; }
	if (ms_colorCorrectionTexture)      { ms_colorCorrectionTexture->Release();      ms_colorCorrectionTexture = NULL; }
	if (ms_pointSampler)                { ms_pointSampler->Release();                ms_pointSampler = NULL; }
	if (ms_compositeCorrectShader)      { ms_compositeCorrectShader->Release();      ms_compositeCorrectShader = NULL; }
	if (ms_compositeCopyShader)         { ms_compositeCopyShader->Release();         ms_compositeCopyShader = NULL; }
	if (ms_compositeVertexShader)       { ms_compositeVertexShader->Release();       ms_compositeVertexShader = NULL; }
}

// ======================================================================

bool Direct3d11_SceneTarget::install(int width, int height)
{
	ms_width  = width;
	ms_height = height;

	ms_antialiasRequested = ConfigDirect3d11::getAntiAlias();

	// The table has to exist before anything can composite, and Graphics::install
	// pushes the persisted brightness/contrast/gamma at us right after install
	// returns anyway.
	Direct3d11_SceneTarget::setBrightnessContrastGamma(ms_brightness, ms_contrast, ms_gamma);

	if (!createBuffers())
		return false;

	if (!createCompositeResources())
		return false;

	return true;
}

// ----------------------------------------------------------------------

void Direct3d11_SceneTarget::remove()
{
	releaseCompositeResources();
	releaseBuffers();
}

// ----------------------------------------------------------------------

bool Direct3d11_SceneTarget::resize(int width, int height)
{
	if (width == ms_width && height == ms_height)
		return true;

	releaseBuffers();

	ms_width  = width;
	ms_height = height;

	return createBuffers();
}

// ----------------------------------------------------------------------

ID3D11RenderTargetView *Direct3d11_SceneTarget::getRenderTargetView()
{
	return ms_colorView;
}

// ----------------------------------------------------------------------

int Direct3d11_SceneTarget::getSampleCount()
{
	return ms_sampleCount;
}

// ----------------------------------------------------------------------

bool Direct3d11_SceneTarget::setAntialiasEnabled(bool enabled)
{
	if (enabled == ms_antialiasRequested)
		return true;

	ms_antialiasRequested = enabled;

	releaseBuffers();

	if (!createBuffers())
	{
		WARNING(true, ("Direct3d11: the scene target could not be rebuilt after multisampling was turned %s.", enabled ? "on" : "off"));
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------

DXGI_FORMAT Direct3d11_SceneTarget::getDepthFormat()
{
	return ms_depthFormat;
}

// ----------------------------------------------------------------------

ID3D11DepthStencilView *Direct3d11_SceneTarget::getDepthStencilView()
{
	return ms_depthView;
}

// ----------------------------------------------------------------------

int Direct3d11_SceneTarget::getWidth()
{
	return ms_width;
}

// ----------------------------------------------------------------------

int Direct3d11_SceneTarget::getHeight()
{
	return ms_height;
}

// ======================================================================
/**
 * Put the scene on whatever target is bound, correcting colour if configured.
 *
 * At identity settings this runs the copy shader, which does nothing to any
 * channel value: sample, write, no arithmetic. That is what keeps a gl11
 * screenshot byte-comparable with a gl05 screenshot, since D3D9's gamma ramp is
 * inert in windowed mode and the client ships windowed.
 */

void Direct3d11_SceneTarget::composite()
{
	ID3D11DeviceContext1 * const context = Direct3d11_Device::getContext();
	if (!context || !ms_compositeVertexShader || !ms_compositeCopyShader)
		return;

	// Multisampled colour cannot be sampled; resolve first.
	ID3D11ShaderResourceView *sceneView = ms_colorResourceView;
	if (ms_sampleCount > 1 && ms_resolvedBuffer)
	{
		context->ResolveSubresource(ms_resolvedBuffer, 0, ms_colorBuffer, 0, DXGI_FORMAT_R8G8B8A8_UNORM);
		sceneView = ms_resolvedResourceView;
	}

	if (!sceneView)
		return;

	ID3D11ShaderResourceView *resources[2] = { sceneView, ms_colorCorrectionResourceView };
	context->PSSetShaderResources(0, 2, resources);
	context->PSSetSamplers(0, 1, &ms_pointSampler);

	context->IASetInputLayout(NULL);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->VSSetShader(ms_compositeVertexShader, NULL, 0);
	context->PSSetShader(ms_colorCorrectionIsIdentity ? ms_compositeCopyShader : ms_compositeCorrectShader, NULL, 0);
	context->GSSetShader(NULL, NULL, 0);

	context->OMSetDepthStencilState(ms_compositeDepthState, 0);
	float const blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	context->OMSetBlendState(ms_compositeBlendState, blendFactor, 0xffffffff);
	context->RSSetState(ms_compositeRasterizerState);

	if (ms_colorCorrectionIsIdentity)
		++Direct3d11_Metrics::compositesCopied;
	else
		++Direct3d11_Metrics::compositesCorrected;

	context->Draw(3, 0);
	++Direct3d11_Metrics::drawCalls;

	// Leave nothing bound that the next frame's scene would inherit as a
	// read-write hazard on its own render target.
	ID3D11ShaderResourceView *nothing[2] = { NULL, NULL };
	context->PSSetShaderResources(0, 2, nothing);

	// This pass binds its blend, depth and rasterizer state straight to the
	// context rather than through the state cache, so the cache's shadow is now
	// wrong. Telling it so is not optional: a stale shadow makes the next real
	// bind look redundant and get skipped, which is the one way a redundancy
	// cache produces a wrong image instead of merely a slow one.
	Direct3d11_StateCache::invalidate();
}

// ----------------------------------------------------------------------
/**
 * Copy the scene target into a mappable staging texture.
 *
 * Used by screenShot, writeImage and lockBackBuffer. The staging texture is
 * kept and reused rather than created per call, because every one of those paths
 * can be reached repeatedly and a per-call allocation would show up as a hitch.
 */

ID3D11Texture2D *Direct3d11_SceneTarget::createReadbackCopy()
{
	ID3D11Device1 * const device = Direct3d11_Device::getDevice();
	ID3D11DeviceContext1 * const context = Direct3d11_Device::getContext();
	if (!device || !context || !ms_colorBuffer)
		return NULL;

	if (!ms_readbackBuffer)
	{
		D3D11_TEXTURE2D_DESC description;
		Zero(description);
		description.Width      = static_cast<UINT>(ms_width);
		description.Height     = static_cast<UINT>(ms_height);
		description.MipLevels  = 1;
		description.ArraySize  = 1;
		description.Format     = DXGI_FORMAT_R8G8B8A8_UNORM;
		description.SampleDesc.Count = 1;
		description.Usage      = D3D11_USAGE_STAGING;
		description.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

		HRESULT const hresult = device->CreateTexture2D(&description, NULL, &ms_readbackBuffer);
		if (FAILED(hresult) || !ms_readbackBuffer)
		{
			WARNING(true, ("Direct3d11: the readback staging texture could not be created (%s).", Direct3d11_Device::describeHresult(hresult)));
			return NULL;
		}
	}

	if (ms_sampleCount > 1 && ms_resolvedBuffer)
	{
		context->ResolveSubresource(ms_resolvedBuffer, 0, ms_colorBuffer, 0, DXGI_FORMAT_R8G8B8A8_UNORM);
		context->CopyResource(ms_readbackBuffer, ms_resolvedBuffer);
	}
	else
		context->CopyResource(ms_readbackBuffer, ms_colorBuffer);

	return ms_readbackBuffer;
}

// ======================================================================
