// ======================================================================
//
// Direct3d11.cpp
// copyright (c) 2026 Galaxies Reborn
//
// The entry point clientGraphics reaches this DLL through, and the Gl_api table
// it hands back.
//
// The table is filled inside install(), not in GetApi. It has to be: the moment
// install returns true, Graphics calls getShaderCapability,
// getVideoMemoryInMegabytes and requiresVertexAndPixelShaders with no null
// check at all, then showMouseCursor and setBrightnessContrastGamma.
//
// Slots this commit does not implement are not empty. Each is a named function
// that counts itself and says which slot it was, through
// Direct3d11_Unimplemented -- see that header for why an empty body is the one
// thing a bring-up must not have.
//
// ======================================================================

#include "FirstDirect3d11.h"
#include "Direct3d11.h"

#include "ConfigDirect3d11.h"
#include "Direct3d11_ConstantBuffers.h"
#include "Direct3d11_Device.h"
#include "Direct3d11_DynamicIndexBufferData.h"
#include "Direct3d11_DynamicVertexBufferData.h"
#include "Direct3d11_ImageWriter.h"
#include "Direct3d11_InputLayoutCache.h"
#include "Direct3d11_Metrics.h"
#include "Direct3d11_PixelShaderProgramData.h"
#include "Direct3d11_QueryPool.h"
#include "Direct3d11_SceneTarget.h"
#include "Direct3d11_ShaderCache.h"
#include "Direct3d11_ShaderCompiler.h"
#include "Direct3d11_ShaderImplementationData.h"
#include "Direct3d11_StateCache.h"
#include "Direct3d11_StateObjectCache.h"
#include "Direct3d11_SwapChain.h"
#include "Direct3d11_StaticIndexBufferData.h"
#include "Direct3d11_IndexOptimizer.h"
#include "Direct3d11_LightManager.h"
#include "Direct3d11_MouseCursor.h"
#include "Direct3d11_PointSprite.h"
#include "Direct3d11_RenderTarget.h"
#include "Direct3d11_SceneTarget.h"
#include "Direct3d11_StaticShaderData.h"
#include "Direct3d11_VertexBufferVectorData.h"
#include "Direct3d11_Transforms.h"
#include "Direct3d11_StaticVertexBufferData.h"
#include "Direct3d11_TextureData.h"
#include "Direct3d11_VertexShaderData.h"
#include "Direct3d11_Unimplemented.h"
#include "Direct3d11_VertexBufferDescriptorMap.h"
#include "SetupDll.h"

#include "clientGraphics/Gl_dll.def"
#include "clientGraphics/ShaderConstantRegisters.h"

// ======================================================================

namespace Direct3d11Namespace
{
	constexpr int cs_fogReportIntervalCalls = 300;
	constexpr int cs_fogReportMaximumCount = 20;
	constexpr double cs_colorChannelMaximum = 255.0;

	Gl_api ms_glApi;
	bool ms_installed;

	ID3DUserDefinedAnnotation *ms_annotation;

	// Fill, cull and scissor-enable together select one rasterizer state object.
	// They are remembered here because the engine records its own copy AFTER
	// calling us and never pushes them again, not even after a resize -- and it
	// initialises its cull mode to GCM_counterClockwise as a static without ever
	// sending that default down, so the backend has to come up already matching.
	GlFillMode ms_fillMode = GFM_solid;
	GlCullMode ms_cullMode = GCM_counterClockwise;
	bool ms_scissorEnabled;

	float ms_currentTime;

	// The engine's "this vertex buffer and this shader cannot be drawn together" reporting. The
	// flag belongs to the appearance being drawn, so it is a pointer to the appearance's own
	// bool rather than state of ours; raising it is what stops the appearance retrying every
	// frame.
	const StaticShader *ms_badVertexShaderStaticShader;
	bool *ms_badVertexBufferVertexShaderCombination;
	const char *ms_badVertexBufferAppearanceName;
	bool ms_warnedAboutMissingStaticShaderData;

	bool verify();
	void remove();
	void fillApiTable();
} // namespace Direct3d11Namespace
using namespace Direct3d11Namespace;

// ======================================================================

extern "C" __declspec(dllexport) Gl_api const *GetApi();

// Reported out of band rather than as a Gl_api member, and checked by
// Graphics::install before it calls through any slot. Gl_api has more than one
// binary layout: five slots exist only under _DEBUG and four more only when
// PRODUCTION is 0, and _DEBUG is defined for both the Debug and Optimized
// configurations while PRODUCTION is 1 only for Release. So Release has one
// layout and the other two share another, selected by the same DEBUG_LEVEL that
// picks the _r/_o/_d suffix in this DLL's name. A client and a backend built at
// different levels would otherwise load cleanly and then call the wrong function
// through every shifted slot. It cannot be a struct field: Headless
// blanket-fills Gl_api as void** over sizeof(Gl_api)/sizeof(void*).
extern "C" __declspec(dllexport) unsigned int GetGlApiStructSize();

// ======================================================================

Gl_api const *GetApi()
{
	ms_glApi.verify = verify;
	ms_glApi.install = Direct3d11::install;
	return &ms_glApi;
}

// ----------------------------------------------------------------------

unsigned int GetGlApiStructSize()
{
	return static_cast<unsigned int>(sizeof(Gl_api));
}

// ----------------------------------------------------------------------
/**
 * Report whether this backend can run at all.
 *
 * Always true. Graphics::install treats false as a hard FATAL rather than a
 * graceful decline, so this is not a channel for "not supported here" -- that
 * belongs in install, which can explain itself first.
 */

// ----------------------------------------------------------------------
/**
 * The time the engine last gave us, in seconds.
 *
 * Texture scroll rates are turned into offsets against it, so the material path needs the
 * same value the shader's currentTime register was seeded from rather than a second clock.
 */

void Direct3d11::reportBadVertexBufferVertexShaderCombination()
{
	if (!ms_badVertexBufferVertexShaderCombination || *ms_badVertexBufferVertexShaderCombination)
		return;

	*ms_badVertexBufferVertexShaderCombination = true;

	WARNING(true, ("Direct3d11: the vertex buffer and vertex shader for appearance '%s' cannot be drawn together, so it is being flagged and skipped from here on.",
				   ms_badVertexBufferAppearanceName ? ms_badVertexBufferAppearanceName : "<unnamed>"));

	// DX9 goes further and substitutes ms_badVertexShaderStaticShader so the object renders in
	// an obvious error material instead of vanishing (Direct3d9.cpp:3972-3997). That is not done
	// here: the substitution has to re-enter setStaticShader from inside the draw it is
	// interrupting, and an invisible object that has named itself in the log is a better default
	// for a shipping client than a bright placeholder. The shader is still recorded, so the
	// substitution can be added without another plumbing change.
}

// ----------------------------------------------------------------------

float Direct3d11::getCurrentTimeValue()
{
	return ms_currentTime;
}

bool Direct3d11Namespace::verify()
{
	return true;
}

// ======================================================================
// Slots this commit implements.
// ======================================================================

namespace Direct3d11Namespace
{
	void displayModeChanged()
	{
		// Reached from Os on WM_DISPLAYCHANGE. DXGI notices a mode change itself
		// and the swap chain follows the client area, so there is nothing to do.
		// A real empty implementation rather than an accounted one: Os calls this
		// whenever the user changes desktop resolution, and an accounting stub
		// would report a missing feature that is not missing.
	}

	int getShaderCapability()
	{
		return Direct3d11_Device::getShaderCapability();
	}
	bool requiresVertexAndPixelShaders()
	{
		return true;
	}
	int getVideoMemoryInMegabytes()
	{
		return Direct3d11_Device::getVideoMemoryInMegabytes();
	}
	void getOtherAdapterRects(stdvector<RECT>::fwd &otherAdapterRects)
	{
		Direct3d11_Device::getOtherAdapterRects(otherAdapterRects);
	}

	void flushResources(bool fullReset)
	{
		Direct3d11_SwapChain::flushResources(fullReset);
	}
	bool isGdiVisible()
	{
		return Direct3d11_SwapChain::isWindowed();
	}

	// There is no lost device in D3D11. A removed device is fatal, and anything
	// short of that never invalidates a resource, so this is permanently false.
	bool wasDeviceReset()
	{
		return false;
	}

	void addDeviceLostCallback(Gl_api::CallbackFunction callbackFunction)
	{
		Direct3d11_Device::addDeviceLostCallback(callbackFunction);
	}
	void removeDeviceLostCallback(Gl_api::CallbackFunction callbackFunction)
	{
		Direct3d11_Device::removeDeviceLostCallback(callbackFunction);
	}
	void addDeviceRestoredCallback(Gl_api::CallbackFunction callbackFunction)
	{
		Direct3d11_Device::addDeviceRestoredCallback(callbackFunction);
	}
	void removeDeviceRestoredCallback(Gl_api::CallbackFunction callbackFunction)
	{
		Direct3d11_Device::removeDeviceRestoredCallback(callbackFunction);
	}

	bool supportsMipmappedCubeMaps()
	{
		return true;
	}
	bool supportsTwoSidedStencil()
	{
		return true;
	}
	bool supportsStreamOffsets()
	{
		return true;
	}
	bool supportsDynamicTextures()
	{
		return true;
	}
	bool supportsAntialias()
	{
		return Direct3d11_Device::supportsAntialias();
	}

	// Scissoring is a field of the rasterizer state in D3D11 rather than a state
	// of its own, and every combination of fill, cull and scissor-enable is now
	// pre-created, so this can be answered honestly.
	bool supportsScissorRect()
	{
		return true;
	}

	// False sends the engine down the OS cursor path it already has, which is
	// what we want: a hardware cursor here would mean owning cursor textures.
	bool supportsHardwareMouseCursor()
	{
		return false;
	}
	bool showMouseCursor(bool)
	{
		return false;
	}

	void resize(int width, int height)
	{
		Direct3d11_SwapChain::resize(width, height);
	}
	void setWindowedMode(bool windowed)
	{
		Direct3d11_SwapChain::setWindowedMode(windowed);
	}

	void clearViewport(bool clearColor, uint32 colorValue, bool clearDepth, real depthValue, bool clearStencil, uint32 stencilValue)
	{
		Direct3d11_SwapChain::clearViewport(clearColor, colorValue, clearDepth, depthValue, clearStencil, stencilValue);
	}

	void beginScene()
	{
		Direct3d11_Metrics::beginFrame();
		Direct3d11_QueryPool::beginFrame();
		Direct3d11_ConstantBuffers::beginFrame();
		Direct3d11_LightManager::beginFrame();
		Direct3d11_DynamicVertexBufferData::beginFrame();
		Direct3d11_DynamicIndexBufferData::beginFrame();
		Direct3d11_SwapChain::beginScene();

		// Last, so the per-frame reset above does not clear the interval that starts here.
		Direct3d11_Metrics::markSceneBegin();
	}

	void endScene()
	{
		Direct3d11_SwapChain::endScene();
		Direct3d11_QueryPool::endFrame();

		Direct3d11_Metrics::markSceneEnd();
	}

	bool present()
	{
		// Present blocks: on vsync, and on a GPU that has fallen behind the queue. Bracketed rather
		// than scope-timed so that the engine's work between endScene and here is measurable too.
		Direct3d11_Metrics::markPresentBegin();
		bool const presented = Direct3d11_SwapChain::present();
		Direct3d11_Metrics::markPresentDone();

		return presented;
	}

	void setViewport(int x, int y, int width, int height, real minZ, real maxZ)
	{
		Direct3d11_SwapChain::setViewport(x, y, width, height, minZ, maxZ);

		// The rasterizer viewport is only half of it. The engine's 2D shaders map
		// pixels to clip space through vertex register c9, which the shipped assets
		// have baked in, so without this the whole UI collapses to clip zero.
		Direct3d11_ConstantBuffers::setViewportData(x, y, width, height);
	}

	void setFog(bool enabled, real density, PackedArgb const &color)
	{
		// Both halves are live: the density goes to c10 for the vertex programs' calculateFog,
		// and the colour plus the enable go to the pixel epilogue, which does the blend D3D9's
		// fixed-function fog stage used to do.
		// What the engine is actually asking for. The backend's own handling is already ruled out --
		// the conversion divides by 255, the shadow keeps r/g/b/enable in order, and the upload puts
		// it in the second row of SwgPixelEpilogue where swgFogColor is declared -- so if distant
		// geometry comes out the wrong colour, the value arriving here is the thing to look at.
		// Sampled rather than per-call: setFog is called often and this is a diagnostic.
		if (ConfigDirect3d11::getReportFrameTiming())
		{
			static int calls = 0;
			static int reportsRemaining = cs_fogReportMaximumCount;

			if (reportsRemaining > 0 && ((calls++ % cs_fogReportIntervalCalls) == 0))
			{
				--reportsRemaining;
				WARNING(true, ("Direct3d11 FOG: enabled %d, density %.4f, colour %u %u %u (%.3f %.3f %.3f).",
							   enabled ? 1 : 0, static_cast<double>(density),
							   color.getR(), color.getG(), color.getB(),
							   static_cast<double>(color.getR()) / cs_colorChannelMaximum,
							   static_cast<double>(color.getG()) / cs_colorChannelMaximum,
							   static_cast<double>(color.getB()) / cs_colorChannelMaximum));
			}
		}

		Direct3d11_ConstantBuffers::setFog(
			enabled && !ConfigDirect3d11::getDebugDisableFog(),
			static_cast<float>(density) * ConfigDirect3d11::getFogDensityScale(),
			static_cast<float>(color.getR()) / static_cast<float>(cs_colorChannelMaximum),
			static_cast<float>(color.getG()) / static_cast<float>(cs_colorChannelMaximum),
			static_cast<float>(color.getB()) / static_cast<float>(cs_colorChannelMaximum));
	}

	void setVertexShaderUserConstants(int index, float c0, float c1, float c2, float c3)
	{
		FATAL(index < 0 || index > (VCSR_userConstant7 - VCSR_userConstant0), ("Direct3d11: vertex user constant index %d is outside the eight the register file reserves.", index));

		float const values[4] = {c0, c1, c2, c3};
		Direct3d11_ConstantBuffers::setVertexShaderConstants(VCSR_userConstant0 + index, values, 1);
	}

	void setPixelShaderUserConstants(VectorRgba const *constants, int count)
	{
		NOT_NULL(constants);

		// PSCR_userConstant is deliberately last in the pixel enumeration so that
		// several rows can follow it.
		FATAL(PSCR_userConstant + count > PSCR_CBUFFER_ROWS, ("Direct3d11: %d pixel user constant row(s) at register %d run past the end of the pixel register file.", count, PSCR_userConstant));

		Direct3d11_ConstantBuffers::setPixelShaderConstants(PSCR_userConstant, constants, count);
	}

	void update(float elapsedTime)
	{
		// Accumulated monotonically and uploaded once per frame, before the scene,
		// exactly where D3D9 does it. Every scrolling and animated material reads
		// this, and the texture scroll constant is derived from the same value, so
		// a frame is reproducible from its time alone.
		ms_currentTime += elapsedTime;
		Direct3d11_ConstantBuffers::setCurrentTime(ms_currentTime);
	}

	void setBrightnessContrastGamma(float brightness, float contrast, float gamma)
	{
		// Note this now does something DX9 does not: DX9 calls SetGammaRamp, which
		// has no effect at all on a windowed swap chain, and the client ships
		// windowed. So a player who has ever moved these sliders has a persisted
		// value that has never done anything and will start to. Intended, but a
		// real behaviour change, and the reason every parity capture pins the
		// three values to 1.
		Direct3d11_SceneTarget::setBrightnessContrastGamma(brightness, contrast, gamma);
	}

	bool screenShot(GlScreenShotFormat format, int quality, const char *fileName)
	{
		return Direct3d11_ImageWriter::screenShot(format, quality, fileName);
	}

	bool writeImage(char const *fileName, int const width, int const height, int const pitch, int const *pixelsARGB, bool const alphaExtend, Gl_imageFormat const imageFormat, Rectangle2d const *subRect)
	{
		return Direct3d11_ImageWriter::writeImage(fileName, width, height, pitch, pixelsARGB, alphaExtend, imageFormat, subRect);
	}

	bool lockBackBuffer(Gl_pixelRect &pixels, const RECT *lockRect)
	{
		return Direct3d11_ImageWriter::lockBackBuffer(pixels, lockRect);
	}

	bool unlockBackBuffer()
	{
		return Direct3d11_ImageWriter::unlockBackBuffer();
	}

	// Fill, cull and scissor are one rasterizer state object in D3D11, so all
	// three setters converge on the same lookup. The values are remembered
	// because the engine records its own copy AFTER calling us and never pushes
	// them again -- not even after a resize -- so this is the only place that
	// knows what was last asked for.
	void applyRasterizerState()
	{
		Direct3d11_StateCache::setRasterizerState(Direct3d11_StateObjectCache::getRasterizerState(ms_fillMode, ms_cullMode, ms_scissorEnabled));
	}

	void setFillMode(GlFillMode fillMode)
	{
		ms_fillMode = fillMode;
		applyRasterizerState();
	}

	void setCullMode(GlCullMode cullMode)
	{
		// GlCullMode names the winding that is CULLED, not the front face. With
		// FrontCounterClockwise FALSE, GCM_clockwise becomes D3D11_CULL_FRONT.
		ms_cullMode = cullMode;
		applyRasterizerState();
	}

	void setScissorRect(bool enabled, int x, int y, int width, int height)
	{
		++Direct3d11_Metrics::scissorSetCalls;

		// D3D9 ignored the rectangle when disabling and left the old one in the
		// device; the enable was a separate render state. Here the enable lives in
		// the rasterizer state, so it selects a different object.
		ms_scissorEnabled = enabled;

		if (enabled)
		{
			ID3D11DeviceContext1 *const context = Direct3d11_Device::getContext();
			if (context)
			{
				D3D11_RECT rect;
				rect.left = x;
				rect.top = y;
				rect.right = x + width;
				rect.bottom = y + height;
				context->RSSetScissorRects(1, &rect);
			}
		}

		applyRasterizerState();
	}

	// Debug markers are wired in every configuration, not just developer builds:
	// a RenderDoc or PIX capture is the primary tool for the parity work, and
	// unnamed draw ranges make one nearly useless.
	void pixSetMarker(WCHAR const *name)
	{
		if (ms_annotation)
			ms_annotation->SetMarker(name);
	}
	void pixBeginEvent(WCHAR const *name)
	{
		if (ms_annotation)
			IGNORE_RETURN(ms_annotation->BeginEvent(name));
	}
	void pixEndEvent(WCHAR const *)
	{
		if (ms_annotation)
			IGNORE_RETURN(ms_annotation->EndEvent());
	}
} // namespace Direct3d11Namespace

// ======================================================================
// Slots this commit does not implement. Each names itself when reached.
// ======================================================================

namespace Direct3d11Namespace
{
	// Presentation and image capture. presentToWindow is deliberately NOT aliased
	// to present the way D3D9 does it: a DXGI swap chain is bound to one window,
	// so aliasing would present a tool's viewport into the game window instead.
	// SwgClient never calls it; the nine callers are all editors and viewers.
	bool presentToWindow(HWND, int, int)
	{
		DX11_NOT_IMPLEMENTED("presentToWindow");
		return false;
	}

	// Render targets.
	void setRenderTarget(Texture *texture, CubeFace cubeFace, int mipmapLevel)
	{
		Direct3d11_RenderTarget::setRenderTarget(texture, cubeFace, mipmapLevel);
	}
	bool copyRenderTargetToNonRenderTargetTexture()
	{
		return Direct3d11_RenderTarget::copyRenderTargetToNonRenderTargetTexture();
	}

	// Fixed-function-era rasterizer state.
	// D3D11 has no point size and no point sprites at all; a geometry shader expands them. See
	// Direct3d11_PointSprite.h -- including why the four attenuation states are recorded and not
	// implemented.
	void setPointSize(real size)
	{
		Direct3d11_PointSprite::setSize(static_cast<float>(size));
	}
	void setPointSizeMax(real size)
	{
		Direct3d11_PointSprite::setSizeMaximum(static_cast<float>(size));
	}
	void setPointSizeMin(real size)
	{
		Direct3d11_PointSprite::setSizeMinimum(static_cast<float>(size));
	}
	void setPointScaleEnable(bool enabled)
	{
		Direct3d11_PointSprite::setScaleEnabled(enabled);
	}
	void setPointScaleFactor(real a, real b, real c)
	{
		Direct3d11_PointSprite::setScaleFactor(static_cast<float>(a), static_cast<float>(b), static_cast<float>(c));
	}
	void setPointSpriteEnable(bool enabled)
	{
		Direct3d11_PointSprite::setEnabled(enabled);
	}
	// Rebuilds the scene target rather than the device, because the swap chain here is
	// single-sampled by design and the scene target is the only multisampled surface. D3D9 had
	// to recreate the device, since there the multisample mode belonged to the swap chain.
	void setAntialiasEnabled(bool enabled)
	{
		if (!Direct3d11_Device::supportsAntialias())
			return;

		IGNORE_RETURN(Direct3d11_SceneTarget::setAntialiasEnabled(enabled));
		Direct3d11_RenderTarget::sceneTargetRebuilt();
	}

	// Transforms, lighting and per-draw material state.
	// The concatenation these three feed is deferred to prepareToDraw; see
	// Direct3d11_Transforms.h for why, and for the three parity details that are easy to get
	// wrong.
	void setWorldToCameraTransform(const Transform &transform, const Vector &cameraPosition)
	{
		Direct3d11_Transforms::setWorldToCameraTransform(transform, cameraPosition);
	}
	void setProjectionMatrix(const GlMatrix4x4 &projectionMatrix)
	{
		Direct3d11_Transforms::setProjectionMatrix(projectionMatrix);
	}
	void setObjectToWorldTransformAndScale(const Transform &objectToWorld, const Vector &scale)
	{
		Direct3d11_Transforms::setObjectToWorldTransformAndScale(objectToWorld, scale);
	}
	void setAlphaFadeOpacity(bool enabled, float opacity)
	{
		// Two consumers. The shipped pixel constants expose alphaFadeOpacityEnabled and
		// alphaFadeOpacity as components of packed registers 1 and 2, which shaders read
		// directly; and the alpha test reference is scaled by the same opacity, exactly as
		// DX9 does before writing D3DRS_ALPHAREF.
		// The two packed components belong to the light manager, which owns those whole
		// registers and would otherwise overwrite anything written here. It reads these back
		// and folds them in on the next selection.
		Direct3d11_LightManager::setAlphaFadeOpacity(enabled, opacity);

		// The alpha test reference is scaled by the same opacity, exactly as DX9 does before
		// writing D3DRS_ALPHAREF, and that is this backend's business rather than the light
		// manager's.
		Direct3d11_ConstantBuffers::setAlphaFadeOpacity(enabled ? opacity : 1.0f);
	}
	void setBloomEnabled(bool enabled)
	{
		Direct3d11_LightManager::setBloomEnabled(enabled);
	}
	void setLights(const stdvector<const Light *>::fwd &lightList)
	{
		Direct3d11_LightManager::setLights(lightList);
	}
	// A real no-op, not an unwritten one. DX9's whole body is inside #ifdef FFP -- it sets
	// D3DTSS_TEXTURETRANSFORMFLAGS and a texture-stage matrix, both fixed-function state -- so
	// in a shader-only build it already does nothing but UNREF its arguments. Nothing in the
	// engine calls it either.
	void setTextureTransform(int, bool, int, bool, const real *)
	{
	}

	// Textures and shaders.
	void setGlobalTexture(Tag tag, const Texture &texture)
	{
		Direct3d11_TextureData::setGlobalTexture(tag, texture);
	}
	void releaseAllGlobalTextures()
	{
		Direct3d11_TextureData::releaseAllGlobalTextures();
	}
	// The UV range that samples every texel of a texture exactly once, at texel centres.
	//
	// Ported unchanged, and the half-texel here is not the D3D9 one people remember: that was a
	// rasterisation offset on vertex POSITIONS, which D3D10 removed. This is the texel centre in
	// UV space, (i + 0.5) / n, which is the same in every API. Nothing in the engine calls it,
	// but it is four lines and getting it wrong later would show up as a subtle blur.
	void getOneToOneUVMapping(int textureWidth, int textureHeight, real &u0, real &v0, real &u1, real &v1)
	{
		u0 = 0.5f / static_cast<real>(textureWidth);
		v0 = 0.5f / static_cast<real>(textureHeight);
		u1 = (static_cast<real>(textureWidth - 1) + 0.5f) / static_cast<real>(textureWidth);
		v1 = (static_cast<real>(textureHeight - 1) + 0.5f) / static_cast<real>(textureHeight);
	}
	// False is a supported answer, not only an error: CuiLayer_CursorInterface draws the cursor
	// as interface geometry when this declines. See Direct3d11_MouseCursor.h.
	bool setMouseCursor(const Texture &texture, int hotSpotX, int hotSpotY)
	{
		return Direct3d11_MouseCursor::set(texture, hotSpotX, hotSpotY);
	}
	// The two halves of the engine's "this combination cannot be drawn" reporting. Recorded
	// here; prepareToDraw raises the flag when an input layout cannot be built, which is this
	// backend's version of the failure DX9 detects.
	void setBadVertexShaderStaticShader(const StaticShader *shader)
	{
		ms_badVertexShaderStaticShader = shader;
	}
	void setStaticShader(const StaticShader &shader, int pass)
	{
		// The material owns everything applied here, including the pass state it asks its
		// implementation for, so there is one call rather than two and no chance of applying
		// them in the wrong order.
		StaticShaderGraphicsData *const data = Direct3d11::getStaticShaderGraphicsData(shader);
		if (!data)
		{
			++Direct3d11_Metrics::droppedDraws;

			// Named once, because a counter with a required value of zero is only useful if a
			// nonzero value can be explained. This happens when a material is applied before its
			// backend data exists, which the engine allows: StaticShader::bindGraphicsData runs
			// from the loader, and a draw can reach here in the window before it has.
			if (!ms_warnedAboutMissingStaticShaderData)
			{
				ms_warnedAboutMissingStaticShaderData = true;
				WARNING(true, ("Direct3d11: a material was applied before its backend data existed, so the draw is being skipped and counted in droppedDraws. Reported once."));
			}

			return;
		}

		IGNORE_RETURN(static_cast<Direct3d11_StaticShaderData *>(data)->apply(pass));

		// After the apply, because apply is what records the material state the selection
		// depends on. DX9 orders it the same way.
		Direct3d11_LightManager::setObeysLightScale(shader.obeysLightScale());
	}

	// Buffers.
	// D3DX did this in DX9 and does not exist for D3D11, but the operation never needed a
	// graphics API: it permutes an array of uint16. See Direct3d11_IndexOptimizer.h.
	void optimizeIndexBuffer(WORD *indices, int numberOfIndices)
	{
		Direct3d11_IndexOptimizer::optimize(reinterpret_cast<uint16 *>(indices), numberOfIndices);
	}
	int getMaximumVertexBufferStreamCount()
	{
		// Not an accounted stub, and it never should have been one: the engine calls
		// this during install to size a per-stream shadow array, and gates its
		// multi-stream skinned path on the answer being above one. Reporting zero
		// sized that array to nothing and quietly disabled skinned multi-stream.
		//
		// DX9 reports the device's MaxStreams with a floor of one. D3D11 has no such
		// cap -- the input assembler always has 32 slots -- so the honest answer is
		// what this backend's vector path actually binds.
		return Direct3d11_InputLayoutCache::MAX_STREAMS;
	}

	// Draws.
	// The primitive counts come from the bound buffers, exactly as they do in DX9: binding a
	// vertex or index buffer records the slice, and a draw with no arguments covers all of it.
	// The partial forms carry their own range instead.

	void drawPointList()
	{
		Direct3d11::draw(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST, 0, Direct3d11::getSliceNumberOfVertices(), 0);
	}
	void drawLineList()
	{
		Direct3d11::draw(D3D11_PRIMITIVE_TOPOLOGY_LINELIST, 0, Direct3d11::getSliceNumberOfVertices(), 0);
	}
	void drawLineStrip()
	{
		Direct3d11::draw(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP, 0, Direct3d11::getSliceNumberOfVertices(), 0);
	}
	void drawTriangleList()
	{
		Direct3d11::draw(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, 0, Direct3d11::getSliceNumberOfVertices(), Direct3d11::getSliceNumberOfVertices() / 3);
	}
	void drawTriangleStrip()
	{
		Direct3d11::draw(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, 0, Direct3d11::getSliceNumberOfVertices(), Direct3d11::getSliceNumberOfVertices() - 2);
	}
	void drawTriangleFan()
	{
		Direct3d11::drawFan(0, Direct3d11::getSliceNumberOfVertices());
	}
	void drawQuadList()
	{
		Direct3d11::drawQuads(0, Direct3d11::getSliceNumberOfVertices() / 4);
	}

	void drawIndexedPointList()
	{
		Direct3d11::drawIndexed(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST, 0, Direct3d11::getSliceNumberOfIndices(), 0, 0);
	}
	void drawIndexedLineList()
	{
		Direct3d11::drawIndexed(D3D11_PRIMITIVE_TOPOLOGY_LINELIST, 0, Direct3d11::getSliceNumberOfIndices(), 0, 0);
	}
	void drawIndexedLineStrip()
	{
		Direct3d11::drawIndexed(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP, 0, Direct3d11::getSliceNumberOfIndices(), 0, 0);
	}
	void drawIndexedTriangleList()
	{
		Direct3d11::drawIndexed(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, 0, Direct3d11::getSliceNumberOfIndices(), 0, Direct3d11::getSliceNumberOfIndices() / 3);
	}
	void drawIndexedTriangleStrip()
	{
		Direct3d11::drawIndexed(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, 0, Direct3d11::getSliceNumberOfIndices(), 0, Direct3d11::getSliceNumberOfIndices() - 2);
	}
	void drawIndexedTriangleFan()
	{
		Direct3d11::drawIndexedFanUnsupported();
	}

	void drawPartialPointList(int startVertex, int primitiveCount)
	{
		Direct3d11::draw(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST, startVertex, primitiveCount, 0);
	}
	void drawPartialLineList(int startVertex, int primitiveCount)
	{
		Direct3d11::draw(D3D11_PRIMITIVE_TOPOLOGY_LINELIST, startVertex, primitiveCount * 2, 0);
	}
	void drawPartialLineStrip(int startVertex, int primitiveCount)
	{
		Direct3d11::draw(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP, startVertex, primitiveCount + 1, 0);
	}
	void drawPartialTriangleList(int startVertex, int primitiveCount)
	{
		Direct3d11::draw(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, startVertex, primitiveCount * 3, primitiveCount);
	}
	void drawPartialTriangleStrip(int startVertex, int primitiveCount)
	{
		Direct3d11::draw(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, startVertex, primitiveCount + 2, primitiveCount);
	}
	void drawPartialTriangleFan(int startVertex, int primitiveCount)
	{
		Direct3d11::drawFan(startVertex, primitiveCount + 2);
	}

	void drawPartialIndexedPointList(int baseIndex, int, int, int startIndex, int primitiveCount)
	{
		Direct3d11::drawIndexed(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST, startIndex, primitiveCount, baseIndex, 0);
	}
	void drawPartialIndexedLineList(int baseIndex, int, int, int startIndex, int primitiveCount)
	{
		Direct3d11::drawIndexed(D3D11_PRIMITIVE_TOPOLOGY_LINELIST, startIndex, primitiveCount * 2, baseIndex, 0);
	}
	void drawPartialIndexedLineStrip(int baseIndex, int, int, int startIndex, int primitiveCount)
	{
		Direct3d11::drawIndexed(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP, startIndex, primitiveCount + 1, baseIndex, 0);
	}
	void drawPartialIndexedTriangleList(int baseIndex, int, int, int startIndex, int primitiveCount)
	{
		Direct3d11::drawIndexed(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, startIndex, primitiveCount * 3, baseIndex, primitiveCount);
	}
	void drawPartialIndexedTriangleStrip(int baseIndex, int, int, int startIndex, int primitiveCount)
	{
		Direct3d11::drawIndexed(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, startIndex, primitiveCount + 2, baseIndex, primitiveCount);
	}
	void drawPartialIndexedTriangleFan(int, int, int, int, int)
	{
		Direct3d11::drawIndexedFanUnsupported();
	}

	// The resource factories. Fatal rather than null: the engine stores what
	// these return and dereferences it later, so a null converts a missing
	// feature into an access violation somewhere unrelated.
	ShaderImplementationGraphicsData *createShaderImplementationGraphicsData(const ShaderImplementation &implementation)
	{
		return new Direct3d11_ShaderImplementationData(implementation);
	}
	StaticShaderGraphicsData *createStaticShaderGraphicsData(const StaticShader &shader)
	{
		return new Direct3d11_StaticShaderData(shader);
	}
	StaticVertexBufferGraphicsData *createStaticVertexBufferData(const StaticVertexBuffer &vertexBuffer)
	{
		return new Direct3d11_StaticVertexBufferData(vertexBuffer);
	}
	DynamicVertexBufferGraphicsData *createDynamicVertexBufferData(const DynamicVertexBuffer &vertexBuffer)
	{
		return new Direct3d11_DynamicVertexBufferData(vertexBuffer);
	}
	VertexBufferVectorGraphicsData *createVertexBufferVectorData(VertexBufferVector const &)
	{
		return new Direct3d11_VertexBufferVectorData;
	}
	StaticIndexBufferGraphicsData *createStaticIndexBufferData(const StaticIndexBuffer &indexBuffer)
	{
		return new Direct3d11_StaticIndexBufferData(indexBuffer);
	}
	DynamicIndexBufferGraphicsData *createDynamicIndexBufferData()
	{
		return new Direct3d11_DynamicIndexBufferData();
	}
	TextureGraphicsData *createTextureData(const Texture &texture, const TextureFormat *runtimeFormats, int numberOfRuntimeFormats)
	{
		return new Direct3d11_TextureData(texture, runtimeFormats, numberOfRuntimeFormats);
	}
	ShaderImplementationPassVertexShaderGraphicsData *createVertexShaderData(ShaderImplementationPassVertexShader const &vertexShader)
	{
		return new Direct3d11_VertexShaderData(vertexShader);
	}
	ShaderImplementationPassPixelShaderProgramGraphicsData *createPixelShaderProgramData(ShaderImplementationPassPixelShaderProgram const &program)
	{
		return new Direct3d11_PixelShaderProgramData(program);
	}

#ifdef _DEBUG
	// The five slots that exist only under _DEBUG. Graphics.cpp wraps three of
	// them in NOT_NULL, so a developer build cannot even load a backend that
	// leaves them unassigned -- which is why the prior DX11 attempt, which never
	// defined any of them, was Release-only by construction.
	//
	// Two of the five are answered by the state and texture work and are honest
	// absences until then. The other three come from the metrics.

	void setTexturesEnabled(bool enabled)
	{
		Direct3d11_StateCache::setTexturesEnabled(enabled);
	}

	bool ms_showMipmapLevels;
	void showMipmapLevels(bool enabled)
	{
		ms_showMipmapLevels = enabled;
	}
	bool getShowMipmapLevels()
	{
		return ms_showMipmapLevels;
	}

	void setBadVertexBufferVertexShaderCombination(bool *flag, const char *appearanceName)
	{
		ms_badVertexBufferVertexShaderCombination = flag;
		ms_badVertexBufferAppearanceName = appearanceName;
	}

	void getRenderedVerticesPointsLinesTrianglesCalls(int &vertices, int &points, int &lines, int &triangles, int &calls)
	{
		// Points and lines are not counted separately yet -- the draw paths that
		// would distinguish them do not exist. Reported as zero rather than
		// folded into triangles, so the display cannot imply a number that was
		// never measured.
		vertices = Direct3d11_Metrics::vertices;
		points = 0;
		lines = 0;
		triangles = Direct3d11_Metrics::triangles;
		calls = Direct3d11_Metrics::drawCalls + Direct3d11_Metrics::drawIndexedCalls;
	}
#endif

#if PRODUCTION == 0
	bool createVideoBuffers(int, int)
	{
		DX11_NOT_IMPLEMENTED("createVideoBuffers");
		return false;
	}
	void fillVideoBuffers()
	{
		DX11_NOT_IMPLEMENTED("fillVideoBuffers");
	}
	bool getVideoBufferData(void *, size_t)
	{
		DX11_NOT_IMPLEMENTED("getVideoBufferData");
		return false;
	}
	void releaseVideoBuffers()
	{
		DX11_NOT_IMPLEMENTED("releaseVideoBuffers");
	}
#endif
} // namespace Direct3d11Namespace

// ======================================================================

void Direct3d11Namespace::fillApiTable()
{
	ms_glApi.remove = remove;
	ms_glApi.displayModeChanged = displayModeChanged;

	ms_glApi.getShaderCapability = getShaderCapability;
	ms_glApi.requiresVertexAndPixelShaders = requiresVertexAndPixelShaders;
	ms_glApi.getOtherAdapterRects = getOtherAdapterRects;
	ms_glApi.getVideoMemoryInMegabytes = getVideoMemoryInMegabytes;
	ms_glApi.isGdiVisible = isGdiVisible;
	ms_glApi.wasDeviceReset = wasDeviceReset;

	ms_glApi.addDeviceLostCallback = addDeviceLostCallback;
	ms_glApi.removeDeviceLostCallback = removeDeviceLostCallback;
	ms_glApi.addDeviceRestoredCallback = addDeviceRestoredCallback;
	ms_glApi.removeDeviceRestoredCallback = removeDeviceRestoredCallback;

	ms_glApi.flushResources = flushResources;

#ifdef _DEBUG
	ms_glApi.setTexturesEnabled = setTexturesEnabled;
	ms_glApi.showMipmapLevels = showMipmapLevels;
	ms_glApi.getShowMipmapLevels = getShowMipmapLevels;
	ms_glApi.setBadVertexBufferVertexShaderCombination = setBadVertexBufferVertexShaderCombination;
	ms_glApi.getRenderedVerticesPointsLinesTrianglesCalls = getRenderedVerticesPointsLinesTrianglesCalls;
#endif

	ms_glApi.setBrightnessContrastGamma = setBrightnessContrastGamma;

	ms_glApi.supportsMipmappedCubeMaps = supportsMipmappedCubeMaps;
	ms_glApi.supportsScissorRect = supportsScissorRect;
	ms_glApi.supportsHardwareMouseCursor = supportsHardwareMouseCursor;
	ms_glApi.supportsTwoSidedStencil = supportsTwoSidedStencil;
	ms_glApi.supportsStreamOffsets = supportsStreamOffsets;
	ms_glApi.supportsDynamicTextures = supportsDynamicTextures;

	ms_glApi.resize = resize;
	ms_glApi.setWindowedMode = setWindowedMode;

	ms_glApi.setFillMode = setFillMode;
	ms_glApi.setCullMode = setCullMode;

	ms_glApi.setPointSize = setPointSize;
	ms_glApi.setPointSizeMax = setPointSizeMax;
	ms_glApi.setPointSizeMin = setPointSizeMin;
	ms_glApi.setPointScaleEnable = setPointScaleEnable;
	ms_glApi.setPointScaleFactor = setPointScaleFactor;
	ms_glApi.setPointSpriteEnable = setPointSpriteEnable;

	ms_glApi.clearViewport = clearViewport;

	ms_glApi.update = update;
	ms_glApi.beginScene = beginScene;
	ms_glApi.endScene = endScene;

	ms_glApi.lockBackBuffer = lockBackBuffer;
	ms_glApi.unlockBackBuffer = unlockBackBuffer;

	ms_glApi.present = present;
	ms_glApi.presentToWindow = presentToWindow;
	ms_glApi.setRenderTarget = setRenderTarget;
	ms_glApi.copyRenderTargetToNonRenderTargetTexture = copyRenderTargetToNonRenderTargetTexture;

	ms_glApi.screenShot = screenShot;

	ms_glApi.createShaderImplementationGraphicsData = createShaderImplementationGraphicsData;
	ms_glApi.createStaticShaderGraphicsData = createStaticShaderGraphicsData;
	ms_glApi.setBadVertexShaderStaticShader = setBadVertexShaderStaticShader;
	ms_glApi.setStaticShader = setStaticShader;

	ms_glApi.setMouseCursor = setMouseCursor;
	ms_glApi.showMouseCursor = showMouseCursor;

	ms_glApi.setViewport = setViewport;
	ms_glApi.setScissorRect = setScissorRect;
	ms_glApi.setWorldToCameraTransform = setWorldToCameraTransform;
	ms_glApi.setProjectionMatrix = setProjectionMatrix;
	ms_glApi.setFog = setFog;
	ms_glApi.setObjectToWorldTransformAndScale = setObjectToWorldTransformAndScale;
	ms_glApi.setGlobalTexture = setGlobalTexture;
	ms_glApi.releaseAllGlobalTextures = releaseAllGlobalTextures;
	ms_glApi.setTextureTransform = setTextureTransform;
	ms_glApi.setVertexShaderUserConstants = setVertexShaderUserConstants;
	ms_glApi.setPixelShaderUserConstants = setPixelShaderUserConstants;

	ms_glApi.setAlphaFadeOpacity = setAlphaFadeOpacity;

	ms_glApi.setLights = setLights;

	ms_glApi.createStaticVertexBufferData = createStaticVertexBufferData;
	ms_glApi.createDynamicVertexBufferData = createDynamicVertexBufferData;
	ms_glApi.createVertexBufferVectorData = createVertexBufferVectorData;
	// Members of the class, not the namespace: they need friend access to read
	// m_graphicsData out of the engine's buffer objects.
	ms_glApi.setVertexBuffer = Direct3d11::setVertexBuffer;
	ms_glApi.setVertexBufferVector = Direct3d11::setVertexBufferVector;

	ms_glApi.createStaticIndexBufferData = createStaticIndexBufferData;
	ms_glApi.createDynamicIndexBufferData = createDynamicIndexBufferData;
	ms_glApi.setIndexBuffer = Direct3d11::setIndexBuffer;

	ms_glApi.getOneToOneUVMapping = getOneToOneUVMapping;
	ms_glApi.createTextureData = createTextureData;

	ms_glApi.createVertexShaderData = createVertexShaderData;
	ms_glApi.createPixelShaderProgramData = createPixelShaderProgramData;

	ms_glApi.drawPointList = drawPointList;
	ms_glApi.drawLineList = drawLineList;
	ms_glApi.drawLineStrip = drawLineStrip;
	ms_glApi.drawTriangleList = drawTriangleList;
	ms_glApi.drawTriangleStrip = drawTriangleStrip;
	ms_glApi.drawTriangleFan = drawTriangleFan;
	ms_glApi.drawQuadList = drawQuadList;

	ms_glApi.drawIndexedPointList = drawIndexedPointList;
	ms_glApi.drawIndexedLineList = drawIndexedLineList;
	ms_glApi.drawIndexedLineStrip = drawIndexedLineStrip;
	ms_glApi.drawIndexedTriangleList = drawIndexedTriangleList;
	ms_glApi.drawIndexedTriangleStrip = drawIndexedTriangleStrip;
	ms_glApi.drawIndexedTriangleFan = drawIndexedTriangleFan;

	ms_glApi.drawPartialPointList = drawPartialPointList;
	ms_glApi.drawPartialLineList = drawPartialLineList;
	ms_glApi.drawPartialLineStrip = drawPartialLineStrip;
	ms_glApi.drawPartialTriangleList = drawPartialTriangleList;
	ms_glApi.drawPartialTriangleStrip = drawPartialTriangleStrip;
	ms_glApi.drawPartialTriangleFan = drawPartialTriangleFan;

	ms_glApi.drawPartialIndexedPointList = drawPartialIndexedPointList;
	ms_glApi.drawPartialIndexedLineList = drawPartialIndexedLineList;
	ms_glApi.drawPartialIndexedLineStrip = drawPartialIndexedLineStrip;
	ms_glApi.drawPartialIndexedTriangleList = drawPartialIndexedTriangleList;
	ms_glApi.drawPartialIndexedTriangleStrip = drawPartialIndexedTriangleStrip;
	ms_glApi.drawPartialIndexedTriangleFan = drawPartialIndexedTriangleFan;

	ms_glApi.getMaximumVertexBufferStreamCount = getMaximumVertexBufferStreamCount;
	ms_glApi.optimizeIndexBuffer = optimizeIndexBuffer;

	ms_glApi.setBloomEnabled = setBloomEnabled;

	ms_glApi.pixSetMarker = pixSetMarker;
	ms_glApi.pixBeginEvent = pixBeginEvent;
	ms_glApi.pixEndEvent = pixEndEvent;

	ms_glApi.writeImage = writeImage;

	ms_glApi.supportsAntialias = supportsAntialias;
	ms_glApi.setAntialiasEnabled = setAntialiasEnabled;

#if PRODUCTION == 0
	ms_glApi.createVideoBuffers = createVideoBuffers;
	ms_glApi.fillVideoBuffers = fillVideoBuffers;
	ms_glApi.getVideoBufferData = getVideoBufferData;
	ms_glApi.releaseVideoBuffers = releaseVideoBuffers;
#endif
}

// ======================================================================
/**
 * Bring up the device and the swap chain.
 *
 * The table is filled before returning true, because the engine calls five slots
 * immediately afterwards with no null check.
 *
 * On failure the user has to be told from here. Graphics::install just returns
 * false, SetupClientGraphics returns false, and ClientMain skips the entire game
 * and exits zero -- a silent exit indistinguishable from a crash. DX9 puts its
 * own MessageBox here for exactly this reason.
 */

bool Direct3d11::install(Gl_install *gl_install)
{
	NOT_NULL(gl_install);
	DEBUG_FATAL(ms_installed, ("Direct3d11::install called twice"));

	ConfigDirect3d11::install();

	if (!Direct3d11_Device::install())
	{
		IGNORE_RETURN(MessageBox(NULL, "The Direct3D 11 device could not be created. See warning.log for the reason.", "Star Wars Galaxies", MB_OK | MB_ICONSTOP));
		return false;
	}

	if (!Direct3d11_SwapChain::install(gl_install))
	{
		IGNORE_RETURN(MessageBox(NULL, "The Direct3D 11 swap chain could not be created. See warning.log for the reason.", "Star Wars Galaxies", MB_OK | MB_ICONSTOP));
		Direct3d11_Device::remove();
		return false;
	}

	// Debug markers, wired in every configuration; a null annotation interface
	// just means no debugger is attached.
	if (Direct3d11_Device::getContext())
		IGNORE_RETURN(Direct3d11_Device::getContext()->QueryInterface(__uuidof(ID3DUserDefinedAnnotation), reinterpret_cast<void **>(&ms_annotation)));

	Direct3d11_VertexBufferDescriptorMap::install();
	Direct3d11_InputLayoutCache::install();
	Direct3d11_DynamicVertexBufferData::install();
	Direct3d11_DynamicIndexBufferData::install();
	Direct3d11_StateObjectCache::install();
	Direct3d11_StateCache::install();
	Direct3d11_ShaderCompiler::install();

	// After the compiler: validating the manifest reads includes through its include handler.
	Direct3d11_ShaderCache::install();
	Direct3d11_ConstantBuffers::install();

	// After the state cache, because a texture's destructor unbinds itself through it,
	// and before anything can create a texture. This is also where the engine's
	// format-support table gets filled in; every entry in it starts unsupported, so
	// until this runs no texture can choose a format at all.
	Direct3d11_TextureData::install();

	// After the compiler, which they use, and after the device, which they create shader
	// objects on.
	Direct3d11_ShaderImplementationData::install();
	Direct3d11_StaticShaderData::install();
	Direct3d11_Transforms::install();
	Direct3d11_LightManager::install();

	// After the swap chain, because the resting render target is the scene target and that does
	// not exist until the swap chain has built it.
	Direct3d11_RenderTarget::install();

	// After the shader compiler, which it uses, and at install rather than on first use: a
	// shader compile inside a frame is what Direct3d11_Metrics::shaderCompiles exists to catch.
	Direct3d11_PointSprite::install();
	Direct3d11_VertexShaderData::install();
	Direct3d11_PixelShaderProgramData::install();

	// Come up already holding the state the engine believes is set: it
	// initialises fill to solid and cull to counter-clockwise as statics and never
	// pushes either down.
	applyRasterizerState();

	Direct3d11_Metrics::install();

	// GPU timing is instrumentation, not a requirement: a driver that will not
	// give us queries costs us a measurement, not a renderer.
	IGNORE_RETURN(Direct3d11_QueryPool::install());

	fillApiTable();

	ms_installed = true;

	// Deliberately does NOT enumerate what is missing.
	//
	// Three revisions of this line have now been wrong, each because it was a hand-written list
	// that outlived the code: it claimed resources and draws were unimplemented a dozen commits
	// after they landed, and then claimed point sprites and the hardware cursor were missing on
	// the very run that first exercised both. A list maintained by hand beside code that changes
	// is a list that lies, and this is the first line anyone reads in the log.
	//
	// So the unimplemented slots report themselves instead -- DX11_NOT_IMPLEMENTED already logs
	// each one by name the first time it is actually called, which is both accurate by
	// construction and better evidence, because it says what this run REACHED rather than what
	// someone believed was absent when they last edited a comment.
	WARNING(true, ("Direct3d11: up at %dx%d %s. Unimplemented entry points name themselves if they are reached.",
				   Direct3d11_SwapChain::getWidth(), Direct3d11_SwapChain::getHeight(), Direct3d11_SwapChain::isWindowed() ? "windowed" : "fullscreen"));

	return true;
}

// ----------------------------------------------------------------------
/**
 * Tear down, from any state.
 *
 * clientGraphics registers this on the ExitChain as critical before the DLL is
 * even loaded, so it can be reached without a successful install and it runs
 * while the process is fataling. On that path the engine's own non-critical
 * teardown is skipped, so engine objects may still hold resources created here.
 * Releasing anyway is correct; failing is not, because a FATAL inside a FATAL
 * loses the original diagnostic.
 */

void Direct3d11Namespace::remove()
{
	Direct3d11_UnimplementedSlot::report();
	Direct3d11_Metrics::remove();
	Direct3d11_QueryPool::remove();

	if (ms_annotation)
	{
		ms_annotation->Release();
		ms_annotation = NULL;
	}

	// Before the state cache, mirroring the install order: draining the global texture
	// registry can destroy textures, and a texture's destructor unbinds itself through
	// the cache.
	Direct3d11_PointSprite::remove();
	Direct3d11_MouseCursor::remove();
	Direct3d11_RenderTarget::remove();
	Direct3d11::releaseDrawResources();
	Direct3d11_StaticShaderData::remove();
	Direct3d11_ShaderImplementationData::remove();
	Direct3d11_PixelShaderProgramData::remove();
	Direct3d11_VertexShaderData::remove();

	Direct3d11_TextureData::remove();

	Direct3d11_ConstantBuffers::remove();
	// Before the compiler: writing the manifest reads the compiler's include cache.
	Direct3d11_ShaderCache::remove();

	Direct3d11_ShaderCompiler::remove();
	Direct3d11_StateCache::remove();
	Direct3d11_StateObjectCache::remove();
	Direct3d11_DynamicIndexBufferData::remove();
	Direct3d11_DynamicVertexBufferData::remove();
	Direct3d11_InputLayoutCache::remove();
	Direct3d11_VertexBufferDescriptorMap::remove();
	Direct3d11_SwapChain::remove();
	Direct3d11_Device::remove();

	ms_installed = false;
}

// ======================================================================
