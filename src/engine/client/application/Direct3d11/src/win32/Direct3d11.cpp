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
#include "Direct3d11_ImageWriter.h"
#include "Direct3d11_Metrics.h"
#include "Direct3d11_QueryPool.h"
#include "Direct3d11_SceneTarget.h"
#include "Direct3d11_ShaderCompiler.h"
#include "Direct3d11_StateCache.h"
#include "Direct3d11_StateObjectCache.h"
#include "Direct3d11_SwapChain.h"
#include "Direct3d11_Unimplemented.h"
#include "SetupDll.h"

#include "clientGraphics/Gl_dll.def"
#include "clientGraphics/ShaderConstantRegisters.h"

// ======================================================================

namespace Direct3d11Namespace
{
	Gl_api                      ms_glApi;
	bool                        ms_installed;

	ID3DUserDefinedAnnotation  *ms_annotation;

	// Fill, cull and scissor-enable together select one rasterizer state object.
	// They are remembered here because the engine records its own copy AFTER
	// calling us and never pushes them again, not even after a resize -- and it
	// initialises its cull mode to GCM_counterClockwise as a static without ever
	// sending that default down, so the backend has to come up already matching.
	GlFillMode                  ms_fillMode = GFM_solid;
	GlCullMode                  ms_cullMode = GCM_counterClockwise;
	bool                        ms_scissorEnabled;

	float                       ms_currentTime;

	bool                        verify();
	void                        remove();
	void                        fillApiTable();
}
using namespace Direct3d11Namespace;

// ======================================================================

extern "C" __declspec(dllexport) Gl_api const * GetApi();

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

Gl_api const * GetApi()
{
	ms_glApi.verify  = verify;
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

	int  getShaderCapability()                { return Direct3d11_Device::getShaderCapability(); }
	bool requiresVertexAndPixelShaders()      { return true; }
	int  getVideoMemoryInMegabytes()          { return Direct3d11_Device::getVideoMemoryInMegabytes(); }
	void getOtherAdapterRects(stdvector<RECT>::fwd &otherAdapterRects) { Direct3d11_Device::getOtherAdapterRects(otherAdapterRects); }

	void flushResources(bool fullReset)       { Direct3d11_SwapChain::flushResources(fullReset); }
	bool isGdiVisible()                       { return Direct3d11_SwapChain::isWindowed(); }

	// There is no lost device in D3D11. A removed device is fatal, and anything
	// short of that never invalidates a resource, so this is permanently false.
	bool wasDeviceReset()                     { return false; }

	void addDeviceLostCallback(Gl_api::CallbackFunction callbackFunction)        { Direct3d11_Device::addDeviceLostCallback(callbackFunction); }
	void removeDeviceLostCallback(Gl_api::CallbackFunction callbackFunction)     { Direct3d11_Device::removeDeviceLostCallback(callbackFunction); }
	void addDeviceRestoredCallback(Gl_api::CallbackFunction callbackFunction)    { Direct3d11_Device::addDeviceRestoredCallback(callbackFunction); }
	void removeDeviceRestoredCallback(Gl_api::CallbackFunction callbackFunction) { Direct3d11_Device::removeDeviceRestoredCallback(callbackFunction); }

	bool supportsMipmappedCubeMaps()          { return true; }
	bool supportsTwoSidedStencil()            { return true; }
	bool supportsStreamOffsets()              { return true; }
	bool supportsDynamicTextures()            { return true; }
	bool supportsAntialias()                  { return Direct3d11_Device::supportsAntialias(); }

	// Scissoring is a field of the rasterizer state in D3D11 rather than a state
	// of its own, and every combination of fill, cull and scissor-enable is now
	// pre-created, so this can be answered honestly.
	bool supportsScissorRect()                { return true; }

	// False sends the engine down the OS cursor path it already has, which is
	// what we want: a hardware cursor here would mean owning cursor textures.
	bool supportsHardwareMouseCursor()        { return false; }
	bool showMouseCursor(bool)                { return false; }

	void resize(int width, int height)        { Direct3d11_SwapChain::resize(width, height); }
	void setWindowedMode(bool windowed)       { Direct3d11_SwapChain::setWindowedMode(windowed); }

	void clearViewport(bool clearColor, uint32 colorValue, bool clearDepth, real depthValue, bool clearStencil, uint32 stencilValue)
	{
		Direct3d11_SwapChain::clearViewport(clearColor, colorValue, clearDepth, depthValue, clearStencil, stencilValue);
	}

	void beginScene()
	{
		Direct3d11_Metrics::beginFrame();
		Direct3d11_QueryPool::beginFrame();
		Direct3d11_ConstantBuffers::beginFrame();
		Direct3d11_SwapChain::beginScene();
	}

	void endScene()
	{
		Direct3d11_SwapChain::endScene();
		Direct3d11_QueryPool::endFrame();
	}

	bool present()                            { return Direct3d11_SwapChain::present(); }

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
		// The colour is consumed by the pixel epilogue, which arrives with the
		// alpha test work; the density constant is live now.
		UNREF(color);
		Direct3d11_ConstantBuffers::setFog(enabled, static_cast<float>(density));
	}

	void setVertexShaderUserConstants(int index, float c0, float c1, float c2, float c3)
	{
		FATAL(index < 0 || index > (VCSR_userConstant7 - VCSR_userConstant0), ("Direct3d11: vertex user constant index %d is outside the eight the register file reserves.", index));

		float const values[4] = { c0, c1, c2, c3 };
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
			ID3D11DeviceContext1 * const context = Direct3d11_Device::getContext();
			if (context)
			{
				D3D11_RECT rect;
				rect.left   = x;
				rect.top    = y;
				rect.right  = x + width;
				rect.bottom = y + height;
				context->RSSetScissorRects(1, &rect);
			}
		}

		applyRasterizerState();
	}

	// Debug markers are wired in every configuration, not just developer builds:
	// a RenderDoc or PIX capture is the primary tool for the parity work, and
	// unnamed draw ranges make one nearly useless.
	void pixSetMarker(WCHAR const *name)      { if (ms_annotation) ms_annotation->SetMarker(name); }
	void pixBeginEvent(WCHAR const *name)     { if (ms_annotation) IGNORE_RETURN(ms_annotation->BeginEvent(name)); }
	void pixEndEvent(WCHAR const *)           { if (ms_annotation) IGNORE_RETURN(ms_annotation->EndEvent()); }
}

// ======================================================================
// Slots this commit does not implement. Each names itself when reached.
// ======================================================================

namespace Direct3d11Namespace
{
	// Presentation and image capture. presentToWindow is deliberately NOT aliased
	// to present the way D3D9 does it: a DXGI swap chain is bound to one window,
	// so aliasing would present a tool's viewport into the game window instead.
	// SwgClient never calls it; the nine callers are all editors and viewers.
	bool presentToWindow(HWND, int, int)                                   { DX11_NOT_IMPLEMENTED("presentToWindow"); return false; }

	// Render targets.
	void setRenderTarget(Texture *, CubeFace, int)                         { DX11_NOT_IMPLEMENTED("setRenderTarget"); }
	bool copyRenderTargetToNonRenderTargetTexture()                        { DX11_NOT_IMPLEMENTED("copyRenderTargetToNonRenderTargetTexture"); return false; }

	// Fixed-function-era rasterizer state.
	void setPointSize(real)                                                { DX11_NOT_IMPLEMENTED("setPointSize"); }
	void setPointSizeMax(real)                                             { DX11_NOT_IMPLEMENTED("setPointSizeMax"); }
	void setPointSizeMin(real)                                             { DX11_NOT_IMPLEMENTED("setPointSizeMin"); }
	void setPointScaleEnable(bool)                                         { DX11_NOT_IMPLEMENTED("setPointScaleEnable"); }
	void setPointScaleFactor(real, real, real)                             { DX11_NOT_IMPLEMENTED("setPointScaleFactor"); }
	void setPointSpriteEnable(bool)                                        { DX11_NOT_IMPLEMENTED("setPointSpriteEnable"); }
	void setAntialiasEnabled(bool)                                         { DX11_NOT_IMPLEMENTED("setAntialiasEnabled"); }

	// Transforms, lighting and per-draw material state.
	void setWorldToCameraTransform(const Transform &, const Vector &)      { DX11_NOT_IMPLEMENTED("setWorldToCameraTransform"); }
	void setProjectionMatrix(const GlMatrix4x4 &)                          { DX11_NOT_IMPLEMENTED("setProjectionMatrix"); }
	void setObjectToWorldTransformAndScale(const Transform &, const Vector &) { DX11_NOT_IMPLEMENTED("setObjectToWorldTransformAndScale"); }
	void setAlphaFadeOpacity(bool, float)                                  { DX11_NOT_IMPLEMENTED("setAlphaFadeOpacity"); }
	void setBloomEnabled(bool)                                             { DX11_NOT_IMPLEMENTED("setBloomEnabled"); }
	void setLights(const stdvector<const Light*>::fwd &)                   { DX11_NOT_IMPLEMENTED("setLights"); }
	void setTextureTransform(int, bool, int, bool, const real *)           { DX11_NOT_IMPLEMENTED("setTextureTransform"); }

	// Textures and shaders.
	void setGlobalTexture(Tag, const Texture &)                            { DX11_NOT_IMPLEMENTED("setGlobalTexture"); }
	void releaseAllGlobalTextures()                                        { DX11_NOT_IMPLEMENTED("releaseAllGlobalTextures"); }
	void getOneToOneUVMapping(int, int, real &u0, real &v0, real &u1, real &v1) { DX11_NOT_IMPLEMENTED("getOneToOneUVMapping"); u0 = 0.0f; v0 = 0.0f; u1 = 1.0f; v1 = 1.0f; }
	bool setMouseCursor(const Texture &, int, int)                         { DX11_NOT_IMPLEMENTED("setMouseCursor"); return false; }
	void setBadVertexShaderStaticShader(const StaticShader *)              { DX11_NOT_IMPLEMENTED("setBadVertexShaderStaticShader"); }
	void setStaticShader(const StaticShader &, int)                        { DX11_NOT_IMPLEMENTED("setStaticShader"); }

	// Buffers.
	void setVertexBuffer(HardwareVertexBuffer const &)                     { DX11_NOT_IMPLEMENTED("setVertexBuffer"); }
	void setVertexBufferVector(VertexBufferVector const &)                 { DX11_NOT_IMPLEMENTED("setVertexBufferVector"); }
	void setIndexBuffer(const HardwareIndexBuffer &)                       { DX11_NOT_IMPLEMENTED("setIndexBuffer"); }
	void optimizeIndexBuffer(WORD *, int)                                  { DX11_NOT_IMPLEMENTED("optimizeIndexBuffer"); }
	int  getMaximumVertexBufferStreamCount()                               { DX11_NOT_IMPLEMENTED("getMaximumVertexBufferStreamCount"); return 0; }

	// Draws.
	void drawPointList()                                                   { DX11_NOT_IMPLEMENTED("drawPointList"); }
	void drawLineList()                                                    { DX11_NOT_IMPLEMENTED("drawLineList"); }
	void drawLineStrip()                                                   { DX11_NOT_IMPLEMENTED("drawLineStrip"); }
	void drawTriangleList()                                                { DX11_NOT_IMPLEMENTED("drawTriangleList"); }
	void drawTriangleStrip()                                               { DX11_NOT_IMPLEMENTED("drawTriangleStrip"); }
	void drawTriangleFan()                                                 { DX11_NOT_IMPLEMENTED("drawTriangleFan"); }
	void drawQuadList()                                                    { DX11_NOT_IMPLEMENTED("drawQuadList"); }
	void drawIndexedPointList()                                            { DX11_NOT_IMPLEMENTED("drawIndexedPointList"); }
	void drawIndexedLineList()                                             { DX11_NOT_IMPLEMENTED("drawIndexedLineList"); }
	void drawIndexedLineStrip()                                            { DX11_NOT_IMPLEMENTED("drawIndexedLineStrip"); }
	void drawIndexedTriangleList()                                         { DX11_NOT_IMPLEMENTED("drawIndexedTriangleList"); }
	void drawIndexedTriangleStrip()                                        { DX11_NOT_IMPLEMENTED("drawIndexedTriangleStrip"); }
	void drawIndexedTriangleFan()                                          { DX11_NOT_IMPLEMENTED("drawIndexedTriangleFan"); }
	void drawPartialPointList(int, int)                                    { DX11_NOT_IMPLEMENTED("drawPartialPointList"); }
	void drawPartialLineList(int, int)                                     { DX11_NOT_IMPLEMENTED("drawPartialLineList"); }
	void drawPartialLineStrip(int, int)                                    { DX11_NOT_IMPLEMENTED("drawPartialLineStrip"); }
	void drawPartialTriangleList(int, int)                                 { DX11_NOT_IMPLEMENTED("drawPartialTriangleList"); }
	void drawPartialTriangleStrip(int, int)                                { DX11_NOT_IMPLEMENTED("drawPartialTriangleStrip"); }
	void drawPartialTriangleFan(int, int)                                  { DX11_NOT_IMPLEMENTED("drawPartialTriangleFan"); }
	void drawPartialIndexedPointList(int, int, int, int, int)              { DX11_NOT_IMPLEMENTED("drawPartialIndexedPointList"); }
	void drawPartialIndexedLineList(int, int, int, int, int)               { DX11_NOT_IMPLEMENTED("drawPartialIndexedLineList"); }
	void drawPartialIndexedLineStrip(int, int, int, int, int)              { DX11_NOT_IMPLEMENTED("drawPartialIndexedLineStrip"); }
	void drawPartialIndexedTriangleList(int, int, int, int, int)           { DX11_NOT_IMPLEMENTED("drawPartialIndexedTriangleList"); }
	void drawPartialIndexedTriangleStrip(int, int, int, int, int)          { DX11_NOT_IMPLEMENTED("drawPartialIndexedTriangleStrip"); }
	void drawPartialIndexedTriangleFan(int, int, int, int, int)            { DX11_NOT_IMPLEMENTED("drawPartialIndexedTriangleFan"); }

	// The resource factories. Fatal rather than null: the engine stores what
	// these return and dereferences it later, so a null converts a missing
	// feature into an access violation somewhere unrelated.
	ShaderImplementationGraphicsData *createShaderImplementationGraphicsData(const ShaderImplementation &)        { DX11_NOT_IMPLEMENTED_FATAL("createShaderImplementationGraphicsData"); return NULL; }
	StaticShaderGraphicsData *createStaticShaderGraphicsData(const StaticShader &)                                { DX11_NOT_IMPLEMENTED_FATAL("createStaticShaderGraphicsData"); return NULL; }
	StaticVertexBufferGraphicsData *createStaticVertexBufferData(const StaticVertexBuffer &)                      { DX11_NOT_IMPLEMENTED_FATAL("createStaticVertexBufferData"); return NULL; }
	DynamicVertexBufferGraphicsData *createDynamicVertexBufferData(const DynamicVertexBuffer &)                   { DX11_NOT_IMPLEMENTED_FATAL("createDynamicVertexBufferData"); return NULL; }
	VertexBufferVectorGraphicsData *createVertexBufferVectorData(VertexBufferVector const &)                      { DX11_NOT_IMPLEMENTED_FATAL("createVertexBufferVectorData"); return NULL; }
	StaticIndexBufferGraphicsData *createStaticIndexBufferData(const StaticIndexBuffer &)                         { DX11_NOT_IMPLEMENTED_FATAL("createStaticIndexBufferData"); return NULL; }
	DynamicIndexBufferGraphicsData *createDynamicIndexBufferData()                                                { DX11_NOT_IMPLEMENTED_FATAL("createDynamicIndexBufferData"); return NULL; }
	TextureGraphicsData *createTextureData(const Texture &, const TextureFormat *, int)                           { DX11_NOT_IMPLEMENTED_FATAL("createTextureData"); return NULL; }
	ShaderImplementationPassVertexShaderGraphicsData *createVertexShaderData(ShaderImplementationPassVertexShader const &) { DX11_NOT_IMPLEMENTED_FATAL("createVertexShaderData"); return NULL; }
	ShaderImplementationPassPixelShaderProgramGraphicsData *createPixelShaderProgramData(ShaderImplementationPassPixelShaderProgram const &) { DX11_NOT_IMPLEMENTED_FATAL("createPixelShaderProgramData"); return NULL; }

#ifdef _DEBUG
	// The five slots that exist only under _DEBUG. Graphics.cpp wraps three of
	// them in NOT_NULL, so a developer build cannot even load a backend that
	// leaves them unassigned -- which is why the prior DX11 attempt, which never
	// defined any of them, was Release-only by construction.
	//
	// Two of the five are answered by the state and texture work and are honest
	// absences until then. The other three come from the metrics.

	void setTexturesEnabled(bool)                                          { DX11_NOT_IMPLEMENTED("setTexturesEnabled"); }

	bool ms_showMipmapLevels;
	void showMipmapLevels(bool enabled)                                    { ms_showMipmapLevels = enabled; }
	bool getShowMipmapLevels()                                             { return ms_showMipmapLevels; }

	void setBadVertexBufferVertexShaderCombination(bool *, const char *)   { DX11_NOT_IMPLEMENTED("setBadVertexBufferVertexShaderCombination"); }

	void getRenderedVerticesPointsLinesTrianglesCalls(int &vertices, int &points, int &lines, int &triangles, int &calls)
	{
		// Points and lines are not counted separately yet -- the draw paths that
		// would distinguish them do not exist. Reported as zero rather than
		// folded into triangles, so the display cannot imply a number that was
		// never measured.
		vertices  = Direct3d11_Metrics::vertices;
		points    = 0;
		lines     = 0;
		triangles = Direct3d11_Metrics::triangles;
		calls     = Direct3d11_Metrics::drawCalls + Direct3d11_Metrics::drawIndexedCalls;
	}
#endif

#if PRODUCTION == 0
	bool createVideoBuffers(int, int)                                      { DX11_NOT_IMPLEMENTED("createVideoBuffers"); return false; }
	void fillVideoBuffers()                                                { DX11_NOT_IMPLEMENTED("fillVideoBuffers"); }
	bool getVideoBufferData(void *, size_t)                                { DX11_NOT_IMPLEMENTED("getVideoBufferData"); return false; }
	void releaseVideoBuffers()                                             { DX11_NOT_IMPLEMENTED("releaseVideoBuffers"); }
#endif
}

// ======================================================================

void Direct3d11Namespace::fillApiTable()
{
	ms_glApi.remove                            = remove;
	ms_glApi.displayModeChanged                = displayModeChanged;

	ms_glApi.getShaderCapability               = getShaderCapability;
	ms_glApi.requiresVertexAndPixelShaders     = requiresVertexAndPixelShaders;
	ms_glApi.getOtherAdapterRects              = getOtherAdapterRects;
	ms_glApi.getVideoMemoryInMegabytes         = getVideoMemoryInMegabytes;
	ms_glApi.isGdiVisible                      = isGdiVisible;
	ms_glApi.wasDeviceReset                    = wasDeviceReset;

	ms_glApi.addDeviceLostCallback             = addDeviceLostCallback;
	ms_glApi.removeDeviceLostCallback          = removeDeviceLostCallback;
	ms_glApi.addDeviceRestoredCallback         = addDeviceRestoredCallback;
	ms_glApi.removeDeviceRestoredCallback      = removeDeviceRestoredCallback;

	ms_glApi.flushResources                    = flushResources;

#ifdef _DEBUG
	ms_glApi.setTexturesEnabled                = setTexturesEnabled;
	ms_glApi.showMipmapLevels                  = showMipmapLevels;
	ms_glApi.getShowMipmapLevels               = getShowMipmapLevels;
	ms_glApi.setBadVertexBufferVertexShaderCombination = setBadVertexBufferVertexShaderCombination;
	ms_glApi.getRenderedVerticesPointsLinesTrianglesCalls = getRenderedVerticesPointsLinesTrianglesCalls;
#endif

	ms_glApi.setBrightnessContrastGamma        = setBrightnessContrastGamma;

	ms_glApi.supportsMipmappedCubeMaps         = supportsMipmappedCubeMaps;
	ms_glApi.supportsScissorRect               = supportsScissorRect;
	ms_glApi.supportsHardwareMouseCursor       = supportsHardwareMouseCursor;
	ms_glApi.supportsTwoSidedStencil           = supportsTwoSidedStencil;
	ms_glApi.supportsStreamOffsets             = supportsStreamOffsets;
	ms_glApi.supportsDynamicTextures           = supportsDynamicTextures;

	ms_glApi.resize                            = resize;
	ms_glApi.setWindowedMode                   = setWindowedMode;

	ms_glApi.setFillMode                       = setFillMode;
	ms_glApi.setCullMode                       = setCullMode;

	ms_glApi.setPointSize                      = setPointSize;
	ms_glApi.setPointSizeMax                   = setPointSizeMax;
	ms_glApi.setPointSizeMin                   = setPointSizeMin;
	ms_glApi.setPointScaleEnable               = setPointScaleEnable;
	ms_glApi.setPointScaleFactor               = setPointScaleFactor;
	ms_glApi.setPointSpriteEnable              = setPointSpriteEnable;

	ms_glApi.clearViewport                     = clearViewport;

	ms_glApi.update                            = update;
	ms_glApi.beginScene                        = beginScene;
	ms_glApi.endScene                          = endScene;

	ms_glApi.lockBackBuffer                    = lockBackBuffer;
	ms_glApi.unlockBackBuffer                  = unlockBackBuffer;

	ms_glApi.present                           = present;
	ms_glApi.presentToWindow                   = presentToWindow;
	ms_glApi.setRenderTarget                   = setRenderTarget;
	ms_glApi.copyRenderTargetToNonRenderTargetTexture = copyRenderTargetToNonRenderTargetTexture;

	ms_glApi.screenShot                        = screenShot;

	ms_glApi.createShaderImplementationGraphicsData = createShaderImplementationGraphicsData;
	ms_glApi.createStaticShaderGraphicsData    = createStaticShaderGraphicsData;
	ms_glApi.setBadVertexShaderStaticShader    = setBadVertexShaderStaticShader;
	ms_glApi.setStaticShader                   = setStaticShader;

	ms_glApi.setMouseCursor                    = setMouseCursor;
	ms_glApi.showMouseCursor                   = showMouseCursor;

	ms_glApi.setViewport                       = setViewport;
	ms_glApi.setScissorRect                    = setScissorRect;
	ms_glApi.setWorldToCameraTransform         = setWorldToCameraTransform;
	ms_glApi.setProjectionMatrix               = setProjectionMatrix;
	ms_glApi.setFog                            = setFog;
	ms_glApi.setObjectToWorldTransformAndScale = setObjectToWorldTransformAndScale;
	ms_glApi.setGlobalTexture                  = setGlobalTexture;
	ms_glApi.releaseAllGlobalTextures          = releaseAllGlobalTextures;
	ms_glApi.setTextureTransform               = setTextureTransform;
	ms_glApi.setVertexShaderUserConstants      = setVertexShaderUserConstants;
	ms_glApi.setPixelShaderUserConstants       = setPixelShaderUserConstants;

	ms_glApi.setAlphaFadeOpacity               = setAlphaFadeOpacity;

	ms_glApi.setLights                         = setLights;

	ms_glApi.createStaticVertexBufferData      = createStaticVertexBufferData;
	ms_glApi.createDynamicVertexBufferData     = createDynamicVertexBufferData;
	ms_glApi.createVertexBufferVectorData      = createVertexBufferVectorData;
	ms_glApi.setVertexBuffer                   = setVertexBuffer;
	ms_glApi.setVertexBufferVector             = setVertexBufferVector;

	ms_glApi.createStaticIndexBufferData       = createStaticIndexBufferData;
	ms_glApi.createDynamicIndexBufferData      = createDynamicIndexBufferData;
	ms_glApi.setIndexBuffer                    = setIndexBuffer;

	ms_glApi.getOneToOneUVMapping              = getOneToOneUVMapping;
	ms_glApi.createTextureData                 = createTextureData;

	ms_glApi.createVertexShaderData            = createVertexShaderData;
	ms_glApi.createPixelShaderProgramData      = createPixelShaderProgramData;

	ms_glApi.drawPointList                     = drawPointList;
	ms_glApi.drawLineList                      = drawLineList;
	ms_glApi.drawLineStrip                     = drawLineStrip;
	ms_glApi.drawTriangleList                  = drawTriangleList;
	ms_glApi.drawTriangleStrip                 = drawTriangleStrip;
	ms_glApi.drawTriangleFan                   = drawTriangleFan;
	ms_glApi.drawQuadList                      = drawQuadList;

	ms_glApi.drawIndexedPointList              = drawIndexedPointList;
	ms_glApi.drawIndexedLineList               = drawIndexedLineList;
	ms_glApi.drawIndexedLineStrip              = drawIndexedLineStrip;
	ms_glApi.drawIndexedTriangleList           = drawIndexedTriangleList;
	ms_glApi.drawIndexedTriangleStrip          = drawIndexedTriangleStrip;
	ms_glApi.drawIndexedTriangleFan            = drawIndexedTriangleFan;

	ms_glApi.drawPartialPointList              = drawPartialPointList;
	ms_glApi.drawPartialLineList               = drawPartialLineList;
	ms_glApi.drawPartialLineStrip              = drawPartialLineStrip;
	ms_glApi.drawPartialTriangleList           = drawPartialTriangleList;
	ms_glApi.drawPartialTriangleStrip          = drawPartialTriangleStrip;
	ms_glApi.drawPartialTriangleFan            = drawPartialTriangleFan;

	ms_glApi.drawPartialIndexedPointList       = drawPartialIndexedPointList;
	ms_glApi.drawPartialIndexedLineList        = drawPartialIndexedLineList;
	ms_glApi.drawPartialIndexedLineStrip       = drawPartialIndexedLineStrip;
	ms_glApi.drawPartialIndexedTriangleList    = drawPartialIndexedTriangleList;
	ms_glApi.drawPartialIndexedTriangleStrip   = drawPartialIndexedTriangleStrip;
	ms_glApi.drawPartialIndexedTriangleFan     = drawPartialIndexedTriangleFan;

	ms_glApi.getMaximumVertexBufferStreamCount = getMaximumVertexBufferStreamCount;
	ms_glApi.optimizeIndexBuffer               = optimizeIndexBuffer;

	ms_glApi.setBloomEnabled                   = setBloomEnabled;

	ms_glApi.pixSetMarker                      = pixSetMarker;
	ms_glApi.pixBeginEvent                     = pixBeginEvent;
	ms_glApi.pixEndEvent                       = pixEndEvent;

	ms_glApi.writeImage                        = writeImage;

	ms_glApi.supportsAntialias                 = supportsAntialias;
	ms_glApi.setAntialiasEnabled               = setAntialiasEnabled;

#if PRODUCTION == 0
	ms_glApi.createVideoBuffers                = createVideoBuffers;
	ms_glApi.fillVideoBuffers                  = fillVideoBuffers;
	ms_glApi.getVideoBufferData                = getVideoBufferData;
	ms_glApi.releaseVideoBuffers               = releaseVideoBuffers;
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

	Direct3d11_StateObjectCache::install();
	Direct3d11_StateCache::install();
	Direct3d11_ShaderCompiler::install();
	Direct3d11_ConstantBuffers::install();

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

	WARNING(true, ("Direct3d11: device and swap chain are up at %dx%d %s. Resources, shaders and draws are not implemented yet, so the first asset load will stop with a named slot.",
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

	Direct3d11_ConstantBuffers::remove();
	Direct3d11_ShaderCompiler::remove();
	Direct3d11_StateCache::remove();
	Direct3d11_StateObjectCache::remove();
	Direct3d11_SwapChain::remove();
	Direct3d11_Device::remove();

	ms_installed = false;
}

// ======================================================================
