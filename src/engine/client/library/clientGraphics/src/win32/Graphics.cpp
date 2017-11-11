// ======================================================================
//
// Graphics.cpp
// Portions copyright 1998 Bootprint Entertainment
// Portions copyright 2001-2003 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/Graphics.h"

#include "clientGraphics/Camera.h"
#include "clientGraphics/DynamicIndexBuffer.h"
#include "clientGraphics/DynamicVertexBuffer.h"
#include "clientGraphics/ConfigClientGraphics.h"
#include "clientGraphics/Gl_dll.def"
#include "clientGraphics/Graphics.def"
#include "clientGraphics/GraphicsOptionTags.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/Texture.h"
#include "clientGraphics/TextureFormatInfo.h"
#include "clientGraphics/TextureList.h"
#include "clientGraphics/VertexBuffer.h"
#include "sharedCollision/BoxExtent.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/ConfigSharedFoundation.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/Production.h"
#include "sharedMath/Rectangle2d.h"
#include "sharedMath/Vector.h"
#include "sharedMath/VectorArgb.h"
#include "sharedUtility/LocalMachineOptionManager.h"

#include "StringId.h"

#include <algorithm>
#include <cstdio>
#include <vector>

#ifdef _DEBUG
#include "clientGraphics/ShaderTemplateList.h"
#endif

// ======================================================================

namespace GraphicsNamespace
{
	void  remove();
	void  displayModeChanged();
	void  windowedModeChanged(bool windowed);
	void  windowPositionChanged();
	void  defaultTranslatePointFromGameToScreen(int &x, int &y);
	void  predrawCheck();
	void  realPredrawCheck();
	void  acquiredFocus();
	void  constrainMouseCursor();

#ifdef _DEBUG
	void  setDebugMouseCursorPosition(int x, int y);
#endif

	const Tag TAG_DX8 = TAG3(D,X,8);
	const Tag TAG_DX9 = TAG3(D,X,9);

	HINSTANCE                                 ms_dll;
	const Gl_api                             *ms_api;

	int                                       ms_frameNumber;

	int                                       ms_currentRenderTargetWidth;
	int                                       ms_currentRenderTargetHeight;
	int                                       ms_currentRenderTargetMaxWidth;
	int                                       ms_currentRenderTargetMaxHeight;
	int                                       ms_frameBufferMaxWidth;
	int                                       ms_frameBufferMaxHeight;
	int                                       ms_colorBufferBitDepth;
	int                                       ms_alphaBufferBitDepth;
	int                                       ms_zBufferBitDepth;
	int                                       ms_stencilBufferBitDepth;
	Graphics::TranslatePointFromGameToScreen  ms_translatePointFromGameToScreen;

	int                                       ms_rasterMajor;
	GlFillMode                                ms_fillMode = GFM_solid;
	GlCullMode                                ms_cullMode = GCM_counterClockwise;

	float                                     ms_pointSize;
	float                                     ms_pointSizeMin;
	float                                     ms_pointSizeMax;
	bool                                      ms_pointScaleEnable;
	float                                     ms_pointScaleFactorA;
	float                                     ms_pointScaleFactorB;
	float                                     ms_pointScaleFactorC;
	bool                                      ms_pointSpriteEnable;

	bool                                      ms_shaderValidated;

	bool                                      ms_windowed;
	bool                                      ms_engineOwnsWindow;
	HWND                                      ms_savedWindow;
	int                                       ms_windowX = INT_MAX;
	int                                       ms_windowY = INT_MAX;

	int                                       ms_viewportX0;
	int                                       ms_viewportY0;
	int                                       ms_viewportWidth;
	int                                       ms_viewportHeight;

	bool                                      ms_hardwareMouseCursorEnabled;
	bool                                      ms_mouseCursorConstrained;
	bool                                      ms_allowMouseCursorConstrained;
	RECT                                      ms_mouseCursorConstrainedRect;
#ifdef _DEBUG
	int                                       ms_debugMouseCursorPositionX;
	int                                       ms_debugMouseCursorPositionY;
	bool                                      ms_drawDebugMouseCursorPosition;
#endif

	bool                                      ms_disableFog;
	bool                                      ms_supportsHardwareMouseCursor;
	int                                       ms_shaderCapability;
	bool                                      ms_requiresVertexAndPixelShaders;
	int                                       ms_videoMemoryInMegabytes;

	float                                     ms_brightness = Graphics::getDefaultBrightness();
	float                                     ms_contrast = Graphics::getDefaultContrast();
	float                                     ms_gamma = Graphics::getDefaultGamma();

	int                                       ms_screenShotFormat = static_cast<int>(GSSF_jpg);
	int                                       ms_screenShotQuality = 100;

	StringId                                  ms_lastError = StringId::cms_invalid;

	bool ms_fogEnabled;
	float ms_fogDensity;
	PackedArgb ms_fogColor;

#ifdef _DEBUG

	bool                                      ms_haveTransformAndScale;
	bool                                      ms_haveCamera;
	bool                                      ms_haveShader;
	bool                                      ms_haveBegin;
	int                                       ms_activeVertexBufferDynamicId;
#endif
}
using namespace GraphicsNamespace;

// ======================================================================
// Install the Graphics Layer subsystem
//
// Remarks:
//
//   This routine should be called early in the game's initialization.  It
//   will add Graphics::remove() to be called the ExitChain at termination.
//
// See Also:
//
//   Graphics::remove()

bool Graphics::install()
{
	if (ConfigClientGraphics::getLoadGpa())
	{
		HMODULE hgpa = LoadLibrary("gpanalyzer.dll");
		if (!hgpa)
			MessageBox(0, "Failed to load GPA", "Error", MB_OK);
	}

	ms_engineOwnsWindow = Os::engineOwnsWindow();
	ms_translatePointFromGameToScreen = GraphicsNamespace::defaultTranslatePointFromGameToScreen;

	ExitChain::add(GraphicsNamespace::remove, "Graphics::remove", 0, true);

	ms_frameBufferMaxWidth   = ConfigClientGraphics::getScreenWidth();
	ms_frameBufferMaxHeight  = ConfigClientGraphics::getScreenHeight();
	ms_windowed              = ConfigClientGraphics::getWindowed();
	ms_colorBufferBitDepth   = ConfigClientGraphics::getColorBufferBitDepth();
	ms_alphaBufferBitDepth   = ConfigClientGraphics::getAlphaBufferBitDepth();
	ms_zBufferBitDepth       = ConfigClientGraphics::getZBufferBitDepth();
	ms_stencilBufferBitDepth = ConfigClientGraphics::getStencilBufferBitDepth();
	ms_rasterMajor           = ConfigClientGraphics::getRasterMajor();

	Os::setAcquiredFocusHookFunction(acquiredFocus);
	Os::setGetHardwareMouseCursorEnabled(getHardwareMouseCursorEnabled);
#ifdef _DEBUG
	Os::setSetSystemMouseCursorPositionHookFunction2(setDebugMouseCursorPosition);
#endif

	// determine the name of the DLL to load
#if   DEBUG_LEVEL == 0
	#define DLL_NAME_FORMAT ".\\gl%02d_r.dll"
#elif DEBUG_LEVEL == 1
	#define DLL_NAME_FORMAT ".\\gl%02d_o.dll"
#elif DEBUG_LEVEL == 2
	#define DLL_NAME_FORMAT ".\\gl%02d_d.dll"
#else
	#error "unknown debug level"
#endif

	if (ms_rasterMajor == 4)
	{
		WARNING(true, ("DX8 is no longer supported.  Please use DX9 (rasterMajor 5 or above)"));
		ms_rasterMajor = 5;
	}

	if (ms_rasterMajor >= 5 && ms_rasterMajor <= 7)
	{
		GraphicsOptionTags::set(TAG_DX8, false);
		GraphicsOptionTags::set(TAG_DX9, true);
	}
	else
		DEBUG_FATAL(true, ("unknown rasterizer"));

	// load the graphics layer dll
	char library[16];
	sprintf(library, DLL_NAME_FORMAT, ms_rasterMajor);
	ms_dll = LoadLibrary(library);
	if (!ms_dll)
	{
#if PRODUCTION
		MessageBox(NULL, "The graphics subsystem could not be initialized.  This is most likely caused by DirectX 9 not being properly installed.", NULL, MB_OK | MB_ICONSTOP);
		return false;
#else
		char *error = Os::getLastError();
		FATAL(true, ("Failed loading library %s (%s)", library, error));
#endif
	}

	IGNORE_RETURN(DebugHelp::loadSymbolsForDll(library));

	// get the function pointer into the ms_dll
	typedef const Gl_api *(*GetApi)();
	GetApi getApi = reinterpret_cast<GetApi>(GetProcAddress(ms_dll, "GetApi"));
	DEBUG_FATAL(!getApi, ("GetProcAddress returned NULL"));

	// call the getApi function
	ms_api = getApi();
	FATAL(!ms_api, ("GetApi returned NULL"));

	// verify the rasterizer will work
	NOT_NULL(ms_api->verify);
	const bool result = ms_api->verify();
	FATAL(!result, ("ms_api->verity returned NULL"));

	// Fetch the options
	LocalMachineOptionManager::registerOption(ms_brightness, "ClientGraphics", "brightness");
	LocalMachineOptionManager::registerOption(ms_contrast, "ClientGraphics", "contrast");
	LocalMachineOptionManager::registerOption(ms_gamma, "ClientGraphics", "gamma");
	LocalMachineOptionManager::registerOption(ms_screenShotFormat, "ClientGraphics", "screenShotFormat");
	LocalMachineOptionManager::registerOption(ms_screenShotQuality, "ClientGraphics", "screenShotQuality");
	LocalMachineOptionManager::registerOption(ms_windowX, "ClientGraphics", "windowX");
	LocalMachineOptionManager::registerOption(ms_windowY, "ClientGraphics", "windowY");

	// setup the install struct
	Gl_install gl_install;
	Zero(gl_install);
	gl_install.window                   = Os::getWindow();
	gl_install.width                    = ms_frameBufferMaxWidth;
	gl_install.height                   = ms_frameBufferMaxHeight;
	gl_install.colorBufferBitDepth      = ms_colorBufferBitDepth;
	gl_install.alphaBufferBitDepth      = ms_alphaBufferBitDepth;
	gl_install.zBufferBitDepth          = ms_zBufferBitDepth;
	gl_install.stencilBufferBitDepth    = ms_stencilBufferBitDepth;
	gl_install.windowed                 = ms_engineOwnsWindow ? ms_windowed : true;
	gl_install.skipInitialClearViewport = ConfigClientGraphics::getSkipInitialClearViewport();
	gl_install.engineOwnsWindow         = ms_engineOwnsWindow;
	gl_install.borderlessWindow         = ConfigClientGraphics::getBorderlessWindow();
	gl_install.windowX                  = ms_windowX;
	gl_install.windowY                  = ms_windowY;
	gl_install.windowedModeChanged      = windowedModeChanged;

	// install the rasterizer
	NOT_NULL(ms_api->install);
	if (!ms_api->install(&gl_install))
		return false;

	ms_frameBufferMaxWidth  = gl_install.width;
	ms_frameBufferMaxHeight = gl_install.height;
	ms_currentRenderTargetWidth  = gl_install.width;
	ms_currentRenderTargetHeight = gl_install.height;
	ms_currentRenderTargetMaxWidth  = gl_install.width;
	ms_currentRenderTargetMaxHeight = gl_install.height;

	ms_windowed = gl_install.windowed;

#if PRODUCTION == 0
	Os::setIsGdiVisibleHookFunction(isGdiVisible);
#endif
	Os::setGetOtherAdapterRectsHookFunction(ms_api->getOtherAdapterRects);
	Os::setDisplayModeChangedHookFunction(GraphicsNamespace::displayModeChanged);
	if (ms_engineOwnsWindow)
		Os::setWindowPositionChangedHookFunction(GraphicsNamespace::windowPositionChanged);

	ms_supportsHardwareMouseCursor = ms_api->supportsHardwareMouseCursor && ms_api->supportsHardwareMouseCursor();
	ms_shaderCapability = ms_api->getShaderCapability();
	ms_videoMemoryInMegabytes = ms_api->getVideoMemoryInMegabytes();

	ms_requiresVertexAndPixelShaders = ms_api->requiresVertexAndPixelShaders();
	ms_allowMouseCursorConstrained = ms_engineOwnsWindow;
	setHardwareMouseCursorEnabled(true);
	unconstrainMouseCursor();
	showMouseCursor(false);

#ifdef _DEBUG
	DebugFlags::registerFlag(ms_disableFog,                   "ClientGraphics", "disableFog");
	DebugFlags::registerFlag(ms_drawDebugMouseCursorPosition, "ClientGraphics", "drawDebugMouseCursorPosition");
#endif

	setBrightnessContrastGamma(ms_brightness, ms_contrast, ms_gamma);

	// @todo fix for bad default config file data.  This can come out in a couple of weeks.
	if (static_cast<int>(ms_screenShotFormat) > static_cast<int>(GSSF_jpg))
		ms_screenShotFormat = GSSF_jpg;

	return true;
}

// ----------------------------------------------------------------------
/**
 * Remove the Graphics Layer subsystem.
 *
 * This routine will be automatically called by the ExitChain and should
 * not be directly called by the user.
 *
 * @see Graphics::install()
 */

void GraphicsNamespace::remove(void)
{
	// unconstrain the mouse cursor
	ms_allowMouseCursorConstrained = false;
	ClipCursor(NULL);

#if PRODUCTION == 0
	Os::setIsGdiVisibleHookFunction(NULL);
#endif
	Os::setGetOtherAdapterRectsHookFunction(NULL);
	Os::setDisplayModeChangedHookFunction(NULL);

	// allow gl ms_api to release itself
	if (ms_api && ms_api->remove)
		ms_api->remove();
	ms_api = NULL;

	if (ms_dll)
	{
		// keep the graphics library around when fataling so the call stack is trackable
		if (!ExitChain::isFataling())
		{
			const BOOL result = FreeLibrary(ms_dll);
			UNREF(result);
			DEBUG_FATAL(!result, ("FreeLibrary failed"));
		}
		ms_dll = NULL;
	}

#ifdef _DEBUG
	DebugFlags::unregisterFlag(ms_disableFog);
#endif
}

// ----------------------------------------------------------------------

void GraphicsNamespace::displayModeChanged()
{
	if (ms_api && ms_api->displayModeChanged)
		ms_api->displayModeChanged();
}

// ----------------------------------------------------------------------

void GraphicsNamespace::windowedModeChanged(bool windowed)
{
	ms_windowed = windowed;
}

// ----------------------------------------------------------------------

void GraphicsNamespace::windowPositionChanged()
{
	RECT rect;
	GetWindowRect(Os::getWindow(), &rect);
	ms_windowX = rect.left;
	ms_windowY = rect.top;
}

// ----------------------------------------------------------------------

int Graphics::getShaderCapability()
{
	return ms_shaderCapability;
}

// ----------------------------------------------------------------------

int Graphics::getVideoMemoryInMegabytes()
{
	return ms_videoMemoryInMegabytes;
}

// ----------------------------------------------------------------------

bool Graphics::requiresVertexAndPixelShaders()
{
	return ms_requiresVertexAndPixelShaders;
}

// ----------------------------------------------------------------------

void Graphics::setTranslatePointFromGameToScreen(TranslatePointFromGameToScreen translatePointFromGameToScreen)
{
	ms_translatePointFromGameToScreen = translatePointFromGameToScreen;
}

// ----------------------------------------------------------------------
// Get the one-based frame number for the current frame

int Graphics::getFrameNumber()
{
	return ms_frameNumber;
}

// ----------------------------------------------------------------------

int Graphics::getFrameBufferMaxWidth()
{
	return ms_frameBufferMaxWidth;
}

// ----------------------------------------------------------------------

int Graphics::getFrameBufferMaxHeight()
{
	return ms_frameBufferMaxHeight;
}

// ----------------------------------------------------------------------

int Graphics::getCurrentRenderTargetWidth()
{
	return ms_currentRenderTargetWidth;
}

// ----------------------------------------------------------------------

int Graphics::getCurrentRenderTargetHeight()
{
	return ms_currentRenderTargetHeight;
}

// ----------------------------------------------------------------------

int Graphics::getCurrentRenderTargetMaxWidth()
{
	return ms_currentRenderTargetMaxWidth;
}

// ----------------------------------------------------------------------

int Graphics::getCurrentRenderTargetMaxHeight()
{
	return ms_currentRenderTargetMaxHeight;
}

// ----------------------------------------------------------------------
/**
 * Make sure the graphics layer is prepared to draw.
 *
 * This routine is present so that in release builds the compiler can completely optimize out the function call.
 * @see Graphics::realPredrawCheck()
 * @internal
 */

void GraphicsNamespace::predrawCheck()
{
#ifdef _DEBUG
	realPredrawCheck();
#endif
}

// ----------------------------------------------------------------------

void GraphicsNamespace::acquiredFocus()
{
	constrainMouseCursor();
}

// ----------------------------------------------------------------------
/**
 * Check whether the engine is running in a window or not.
 *
 * @return True if running in a window, otherwise false
 */

bool Graphics::isWindowed(void)
{
	return ms_windowed;
}

// ----------------------------------------------------------------------

void Graphics::toggleWindowedMode()
{
	setWindowedMode(!ms_windowed);
}

// ----------------------------------------------------------------------

void Graphics::resize(int newWidth, int newHeight)
{
	NOT_NULL(ms_api->resize);
	ms_viewportWidth = ms_currentRenderTargetWidth = ms_currentRenderTargetMaxWidth = ms_frameBufferMaxWidth = newWidth;
	ms_viewportHeight = ms_currentRenderTargetHeight = ms_currentRenderTargetMaxHeight = ms_frameBufferMaxHeight = newHeight;
	ms_api->resize(newWidth, newHeight);

	bool const mouseWasContrained = ms_mouseCursorConstrained;

	unconstrainMouseCursor();
	if (mouseWasContrained)
	{
		constrainMouseCursor(0, 0, newWidth, newHeight);
	}
}

// ----------------------------------------------------------------------

void Graphics::getBrightnessContrastGamma(float &brightness, float &contrast, float &gamma)
{
	brightness = ms_brightness;
	contrast = ms_contrast;
	gamma = ms_gamma;
}

// ----------------------------------------------------------------------

void Graphics::setBrightnessContrastGamma(float brightness, float contrast, float gamma)
{
	ms_brightness = brightness;
	ms_contrast = contrast;
	ms_gamma = gamma;

	NOT_NULL(ms_api->setBrightnessContrastGamma);
	ms_api->setBrightnessContrastGamma(brightness, contrast, gamma);
}

//----------------------------------------------------------------------

float Graphics::getBrightness()
{
	return ms_brightness;
}

//----------------------------------------------------------------------

float Graphics::getContrast()
{
	return ms_contrast;
}

//----------------------------------------------------------------------

float Graphics::getGamma()
{
	return ms_gamma;
}

//----------------------------------------------------------------------

float Graphics::getDefaultBrightness()
{
	return 1.0f;
}

//----------------------------------------------------------------------

float Graphics::getDefaultContrast()
{
	return 1.0f;
}

//----------------------------------------------------------------------

float Graphics::getDefaultGamma()
{
	return 1.0f;
}

//----------------------------------------------------------------------

void Graphics::setBrightness(float brightness)
{
	ms_brightness = brightness;
	NOT_NULL(ms_api->setBrightnessContrastGamma);
	ms_api->setBrightnessContrastGamma(ms_brightness, ms_contrast, ms_gamma);
}

//----------------------------------------------------------------------

void Graphics::setContrast(float contrast)
{
	ms_contrast = contrast;
	NOT_NULL(ms_api->setBrightnessContrastGamma);
	ms_api->setBrightnessContrastGamma(ms_brightness, ms_contrast, ms_gamma);
}

//----------------------------------------------------------------------

void Graphics::setGamma(float gamma)
{
	ms_gamma = gamma;
	NOT_NULL(ms_api->setBrightnessContrastGamma);
	ms_api->setBrightnessContrastGamma(ms_brightness, ms_contrast, ms_gamma);
}

// ----------------------------------------------------------------------

void Graphics::setWindowedMode(bool newWindowed)
{
	NOT_NULL(ms_api->setWindowedMode);
	ms_api->setWindowedMode(newWindowed);
	GraphicsNamespace::constrainMouseCursor();
}

// ----------------------------------------------------------------------

bool Graphics::supportsMipmappedCubeMaps()
{
	NOT_NULL(ms_api->supportsMipmappedCubeMaps);
	return ms_api->supportsMipmappedCubeMaps();
}

// ----------------------------------------------------------------------

bool Graphics::supportsScissorRect()
{
	if (ms_api == NULL)
		return false;

	return ms_api->supportsScissorRect();
}

// ----------------------------------------------------------------------

bool Graphics::supportsTwoSidedStencil()
{
	return ms_api->supportsTwoSidedStencil();
}

// ----------------------------------------------------------------------

bool Graphics::supportsStreamOffsets()
{
	return ms_api->supportsStreamOffsets();
}

// ----------------------------------------------------------------------

bool Graphics::supportsDynamicTextures()
{
	return ms_api->supportsDynamicTextures();
}

// ----------------------------------------------------------------------
/**
 * Check whether GDI is currently visible or not.
 *
 * @return True if GDI is currently visible
 */

bool Graphics::isGdiVisible(void)
{
	NOT_NULL(ms_api->isGdiVisible);
	return ms_api->isGdiVisible();
}

// ----------------------------------------------------------------------

bool Graphics::wasDeviceReset()
{
	NOT_NULL(ms_api->wasDeviceReset);
	return ms_api->wasDeviceReset();
}

// ----------------------------------------------------------------------

void Graphics::addDeviceLostCallback(CallbackFunction callbackFunction)
{
	ms_api->addDeviceLostCallback(callbackFunction);
}

// ----------------------------------------------------------------------

void Graphics::removeDeviceLostCallback(CallbackFunction callbackFunction)
{
	ms_api->removeDeviceLostCallback(callbackFunction);
}

// ----------------------------------------------------------------------

void Graphics::addDeviceRestoredCallback(CallbackFunction callbackFunction)
{
	ms_api->addDeviceRestoredCallback(callbackFunction);
}

// ----------------------------------------------------------------------

void Graphics::removeDeviceRestoredCallback(CallbackFunction callbackFunction)
{
	ms_api->removeDeviceRestoredCallback(callbackFunction);
}

// ----------------------------------------------------------------------

StringId const & Graphics::getLastError()
{
	int volatile a = 0;

	if (a == 1)
	{
		ms_lastError.setTable("engine");
		ms_lastError.setText("desktop_not_32bit");
	}
	else
		if (a == 2)
		{
			ms_lastError.setTable("engine");
			ms_lastError.setText("desktop_too_small");
		}

	return ms_lastError;
}

// ----------------------------------------------------------------------

void Graphics::setLastError(char const * stringTable, char const * stringText)
{
	DEBUG_REPORT_LOG(true, ("graphics::setLastError %s %s\n", stringTable, stringText));
	ms_lastError.setTable(stringTable);
	ms_lastError.setText(stringText);
}

// ----------------------------------------------------------------------

void Graphics::clearLastError()
{
	ms_lastError = StringId::cms_invalid;
}

// ----------------------------------------------------------------------

void Graphics::flushResources(bool fullReset)
{
	if (ms_api->flushResources)
		ms_api->flushResources(fullReset);
}

// ----------------------------------------------------------------------
/**
 * Get the new fill mode.
 *
 * @return fill mode
 */

GlFillMode Graphics::getFillMode()
{
	return ms_fillMode;
}

// ----------------------------------------------------------------------
/**
 * Set the new fill mode.
 *
 * @param newFillMode  new fill mode
 */

void Graphics::setFillMode(GlFillMode newFillMode)
{
	NOT_NULL(ms_api->setFillMode);
	ms_api->setFillMode(newFillMode);
	ms_fillMode = newFillMode;
}

// ----------------------------------------------------------------------
/**
 * Get the new cull mode.
 *
 * @return cull mode
 */

GlCullMode Graphics::getCullMode()
{
	return ms_cullMode;
}

// ----------------------------------------------------------------------
/**
 * Set the new cull mode.
 *
 * @param newCullMode  new cull mode
 */

void Graphics::setCullMode(GlCullMode newCullMode)
{
	NOT_NULL(ms_api->setCullMode);
	ms_api->setCullMode(newCullMode);
	ms_cullMode = newCullMode;
}

// ----------------------------------------------------------------------

float Graphics::getPointSize()
{
	return ms_pointSize;
}

// ----------------------------------------------------------------------

void Graphics::setPointSize(float size)
{
	NOT_NULL(ms_api->setPointSize);
	ms_api->setPointSize(size);
	ms_pointSize = size;
}

// ----------------------------------------------------------------------

float Graphics::getPointSizeMin()
{
	return ms_pointSizeMin;
}

// ----------------------------------------------------------------------

void Graphics::setPointSizeMin(float size)
{
	NOT_NULL(ms_api->setPointSizeMin);
	ms_api->setPointSizeMin(size);
	ms_pointSizeMin = size;
}

// ----------------------------------------------------------------------

float Graphics::getPointSizeMax()
{
	return ms_pointSizeMax;
}

// ----------------------------------------------------------------------

void Graphics::setPointSizeMax(float size)
{
	NOT_NULL(ms_api->setPointSizeMax);
	ms_api->setPointSizeMax(size);
	ms_pointSizeMax = size;
}

// ----------------------------------------------------------------------

bool Graphics::getPointScaleEnable()
{
	return ms_pointScaleEnable;
}

// ----------------------------------------------------------------------

void Graphics::setPointScaleEnable(bool bEnable)
{
	NOT_NULL(ms_api->setPointScaleEnable);
	ms_api->setPointScaleEnable(bEnable);
	ms_pointScaleEnable = bEnable;
}

// ----------------------------------------------------------------------

void Graphics::getPointScaleFactor( float & outA, float & outB, float & outC )
{
	outA = ms_pointScaleFactorA;
	outB = ms_pointScaleFactorB;
	outC = ms_pointScaleFactorC;
}

// ----------------------------------------------------------------------

void Graphics::setPointScaleFactor( float A, float B, float C )
{
	NOT_NULL(ms_api->setPointScaleFactor);
	ms_api->setPointScaleFactor(A,B,C);
	ms_pointScaleFactorA = A;
	ms_pointScaleFactorB = B;
	ms_pointScaleFactorC = C;
}

// ----------------------------------------------------------------------

bool Graphics::getPointSpriteEnable()
{
	return ms_pointSpriteEnable;
}

// ----------------------------------------------------------------------

void Graphics::setPointSpriteEnable(bool bEnable)
{
	NOT_NULL(ms_api->setPointSpriteEnable);
	ms_api->setPointSpriteEnable(bEnable);
	ms_pointSpriteEnable = bEnable;
}

// ----------------------------------------------------------------------

void Graphics::update(float elapsedTime)
{
	NOT_NULL(ms_api->update);
	ms_api->update(elapsedTime);
	++ms_frameNumber;
}

// ----------------------------------------------------------------------

void Graphics::beginScene()
{
#ifdef _DEBUG
	ms_haveBegin = true;
#endif

	NOT_NULL(ms_api->beginScene);
	ms_api->beginScene();
}

// ----------------------------------------------------------------------

void Graphics::endScene()
{
#ifdef _DEBUG
	if (ms_drawDebugMouseCursorPosition)
	{
		setStaticShader(ShaderTemplateList::get2dVertexColorStaticShader());

		int const x = ms_debugMouseCursorPositionX;
		int const y = ms_debugMouseCursorPositionY;
		int const size = 10;
		Graphics::drawLine(x-size, y, x+size+1, y, VectorArgb::solidRed);
		Graphics::drawLine(x, y-size, x, y+size+1, VectorArgb::solidRed);
	}
#endif

	NOT_NULL(ms_api->endScene);
	ms_api->endScene();

#ifdef _DEBUG
	ms_haveTransformAndScale = false;
	ms_haveCamera = false;
	ms_haveShader = false;
	ms_haveBegin = false;
#endif
}

// ----------------------------------------------------------------------

void Graphics::setRenderTarget(Texture *texture, CubeFace cubeFace, int mipmapLevel)
{
	NOT_NULL(ms_api->setRenderTarget);
	ms_api->setRenderTarget(texture, cubeFace, mipmapLevel);

	if (texture)
	{
		const uint uTextureWidth  = static_cast<uint>(texture->getWidth());
		const uint uTextureHeight = static_cast<uint>(texture->getHeight());
		const uint uMipmapLevel   = static_cast<uint>(mipmapLevel);

		ms_currentRenderTargetWidth = ms_currentRenderTargetMaxWidth = std::max(1U, uTextureWidth >> uMipmapLevel);
		ms_currentRenderTargetHeight = ms_currentRenderTargetMaxHeight = std::max(1U, uTextureHeight >> uMipmapLevel);
	}
	else
	{
		ms_currentRenderTargetWidth = ms_currentRenderTargetMaxWidth = ms_frameBufferMaxWidth;
		ms_currentRenderTargetHeight = ms_currentRenderTargetMaxHeight = ms_frameBufferMaxHeight;
	}
}

// ----------------------------------------------------------------------

bool Graphics::copyRenderTargetToNonRenderTargetTexture()
{
	NOT_NULL(ms_api->copyRenderTargetToNonRenderTargetTexture);
	return ms_api->copyRenderTargetToNonRenderTargetTexture();
}

// ----------------------------------------------------------------------

bool Graphics::lockBackBuffer(Gl_pixelRect &o_pixels, const Gl_rect *i_lockRect)
{
	NOT_NULL(ms_api->lockBackBuffer);

	RECT *pLockRect=0;
	RECT lockRect;
	if (i_lockRect)
	{
		lockRect.left   =i_lockRect->x0;
		lockRect.top    =i_lockRect->y0;
		lockRect.bottom =i_lockRect->x1;
		lockRect.right  =i_lockRect->y1;
		pLockRect=&lockRect;
	}
	return ms_api->lockBackBuffer(o_pixels, pLockRect);
}

// ----------------------------------------------------------------------

bool Graphics::unlockBackBuffer()
{
	NOT_NULL(ms_api->unlockBackBuffer);
	return ms_api->unlockBackBuffer();
}

// ----------------------------------------------------------------------

bool Graphics::present()
{
	NOT_NULL(ms_api->present);
	return ms_api->present();
}

// ----------------------------------------------------------------------

bool Graphics::present(HWND window, int const width, int const height)
{
	DEBUG_FATAL(!window, ("window handle is NULL"));

	NOT_NULL(ms_api->presentToWindow);
	return ms_api->presentToWindow(window, width, height);
}

// ----------------------------------------------------------------------

GlScreenShotFormat Graphics::getScreenShotFormat()
{
	return static_cast<GlScreenShotFormat>(ms_screenShotFormat);
}

// ----------------------------------------------------------------------

void Graphics::setScreenShotFormat(GlScreenShotFormat screenShotFormat)
{
	ms_screenShotFormat = screenShotFormat;
}

// ----------------------------------------------------------------------

int Graphics::getScreenShotQuality()
{
	return ms_screenShotQuality;
}

// ----------------------------------------------------------------------

void Graphics::setScreenShotQuality(int quality)
{
	ms_screenShotQuality = clamp(-1, quality, 100);
}

// ----------------------------------------------------------------------
/**
 * Snap a screenshot.
 *
 * This routine will write the contents of the screen to a disk file of the
 * specified name.  The format may vary depending upon which DLL is loaded.
 *
 * @return True if the screen shot was sucessfully written to disk, otherwise false
 */

bool Graphics::screenShot(const char *fileName)
{
	NOT_NULL(ms_api->screenShot);
	return ms_api->screenShot(static_cast<GlScreenShotFormat>(ms_screenShotFormat), ms_screenShotQuality, fileName);
}

// ----------------------------------------------------------------------
/**
 * Clear the viewport as specified.
 *
 * Warning: some hardware, particularly the PowerVR, don't support
 * a user-accessible depth buffer.  In such a case, the depth clear
 * will not have the intended effect.
 *
 * @param clearColor    True to clear the color buffer, otherwise false.
 * @param colorValue    Color to set the frame buffer to.
 * @param clearDepth    True to clear the depth buffer, otherwise false.
 * @param depthValue    The value to set the depth buffer to [0.0f .. 1.0f].
 * @param clearStencil  True to clear the stencil buffer, otherwise false.
 * @param stencilValue  The value to set the stencil buffer depth buffer to.
 */

void Graphics::clearViewport(bool clearColor, uint32 colorValue, bool clearDepth, float depthValue, bool clearStencil, uint32 stencilValue)
{
	NOT_NULL(ms_api->clearViewport);
	ms_api->clearViewport(clearColor, colorValue, clearDepth, depthValue, clearStencil, stencilValue);
}

// ----------------------------------------------------------------------
/**
 * Tell the GL to enable or disable textures.
 */

#ifdef _DEBUG
void Graphics::setTexturesEnabled(bool enabled)
{
	NOT_NULL(ms_api->setTexturesEnabled);
	ms_api->setTexturesEnabled(enabled);
}
#endif

// ----------------------------------------------------------------------
/**
 * Specify that all new Gl texture data created should hue the mipmap levels
 * so that it is obvious which mipmap is being used to render.
 */

#ifdef _DEBUG
void Graphics::showMipmapLevels(bool enabled)
{
	if (ms_api->showMipmapLevels)
		ms_api->showMipmapLevels(enabled);
}
#endif

// ----------------------------------------------------------------------
/**
 * Specify that all new Gl texture data created should hue the mipmap levels
 * so that it is obvious which mipmap is being used to render.
 */

#ifdef _DEBUG
bool Graphics::getShowMipmapLevels()
{
	// this routine is used from MFC code and can get called before the GL is installed, so make it safe
	if (ms_api && ms_api->getShowMipmapLevels)
		return ms_api->getShowMipmapLevels();
	return false;
}
#endif

// ----------------------------------------------------------------------
/**
 * Set an appearance name to assist tracking down shader/vertex buffer data errors.
 *
 * This function will let the graphics DLL know the name of the appearance
 * that will be drawn so it can generate better error messages.
 *
 * @param debugAppearanceName The appearance name to display in error messages.
 */

#ifdef _DEBUG

void Graphics::setBadVertexBufferVertexShaderCombination(bool *flag, const char *debugAppearanceName)
{
	NOT_NULL(ms_api);
	NOT_NULL(ms_api->setBadVertexBufferVertexShaderCombination);
	ms_api->setBadVertexBufferVertexShaderCombination(flag, debugAppearanceName);
}

#endif

// ----------------------------------------------------------------------

void Graphics::setBadVertexShaderStaticShader(const StaticShader *shader)
{
	NOT_NULL(ms_api->setBadVertexShaderStaticShader);
	ms_api->setBadVertexShaderStaticShader(shader);
}

// ----------------------------------------------------------------------
/**
 * Set the new shader for drawing.
 *
 * All subsequent drawing will use this shader's attributes to determine the
 * drawing attributes for the primitve.
 *
 * @param newShader  New shader for drawing primitives
 */

void Graphics::setStaticShader(const StaticShader &shader, int pass)
{
	NOT_NULL(ms_api->setStaticShader);

	ms_shaderValidated = shader.isValid();

	if (ms_shaderValidated)
		ms_api->setStaticShader(shader, pass);

#ifdef _DEBUG
	ms_haveShader = true;
#endif
}

// ----------------------------------------------------------------------
/**
 * Clear the current shader.
 *
 * This routine causes the engine to require a new shader to be set before
 * any more primitive drawing may occur.
 */

#ifdef _DEBUG
void Graphics::clearStaticShader()
{
	ms_haveShader = false;
}
#endif

// ----------------------------------------------------------------------

#ifdef _DEBUG
void Graphics::getRenderedVerticesPointsLinesTrianglesCalls(int &vertices, int &points, int &lines, int &triangles, int &calls)
{
	NOT_NULL(ms_api->getRenderedVerticesPointsLinesTrianglesCalls);
	ms_api->getRenderedVerticesPointsLinesTrianglesCalls(vertices, points, lines, triangles, calls);
}
#endif

// ----------------------------------------------------------------------

void Graphics::setHardwareMouseCursorEnabled(bool enabled)
{
	ms_hardwareMouseCursorEnabled = ms_supportsHardwareMouseCursor && ConfigClientGraphics::getUseHardwareMouseCursor() && enabled;
}

// ----------------------------------------------------------------------

bool Graphics::getHardwareMouseCursorEnabled()
{
	return ms_hardwareMouseCursorEnabled;
}

// ----------------------------------------------------------------------

bool Graphics::setMouseCursor(const Texture &mouseCursorTexture, int hotSpotX, int hotSpotY)
{
	NOT_NULL(ms_api);

	DEBUG_FATAL(mouseCursorTexture.getNativeFormat() != TF_ARGB_8888, ("Mouse cursors must be ARGB 8888 textures (%s)", mouseCursorTexture.getName()));
	DEBUG_FATAL(mouseCursorTexture.getWidth() != 32 || mouseCursorTexture.getHeight() != 32, ("Mouse cursors must be 32x32 (%s %dx%d)", mouseCursorTexture.getName(), mouseCursorTexture.getWidth(), mouseCursorTexture.getHeight()));

	if (ms_hardwareMouseCursorEnabled)
	{
		if (!ConfigClientGraphics::getHardwareMouseCursorUseOriginalAlpha())
		{
			// hack each texture set as a mouse cursor to have 0/1 in the alpha channel
			if (!mouseCursorTexture.m_cursorHack)
			{
				Texture &t = const_cast<Texture &>(mouseCursorTexture);

				// lock the texture
				Texture::LockData lockData(TF_ARGB_8888, 0, 0, 0, 32, 32, false);
				t.lock(lockData);
				int const pitch = lockData.getPitch();
				byte *pixels = reinterpret_cast<byte *>(lockData.getPixelData());

				// touch all the pixels testing the alpha channel and convertin to 0 or 255
				for (int y = 0; y < 32; ++y)
				{
					uint32 *pixel = reinterpret_cast<uint32 *>(pixels + (y * pitch));
					for (int x = 0; x < 32; ++x, ++pixel)
					{
						if (((*pixel >> 24) & 0xff) < 128)
							*pixel &= (0x00ffffff);
						else
							*pixel |= (0xff000000);
					}
				}

				t.unlock(lockData);
				t.m_cursorHack = true;
			}
		}

		NOT_NULL(ms_api->setMouseCursor);
		return ms_api->setMouseCursor(mouseCursorTexture, hotSpotX, hotSpotY);
	}

	return false;
}

// ----------------------------------------------------------------------

bool Graphics::showMouseCursor(bool cursorVisible)
{
	NOT_NULL(ms_api);
	NOT_NULL(ms_api->showMouseCursor);
	if (ms_hardwareMouseCursorEnabled)
		return ms_api->showMouseCursor(cursorVisible);

	return false;
}

// ----------------------------------------------------------------------

void GraphicsNamespace::defaultTranslatePointFromGameToScreen(int &x, int &y)
{
	POINT p = { x, y };
	ClientToScreen(Os::getWindow(), &p);
	x = p.x;
	y = p.y;
}

// ----------------------------------------------------------------------

void Graphics::setSystemMouseCursorPosition(int x, int y)
{
	if (Os::isFocused())
	{
		ms_translatePointFromGameToScreen(x, y);
		SetCursorPos(x, y);
	}
}

// ----------------------------------------------------------------------

void Graphics::setAllowMouseCursorConstrained(bool allowMouseCursorConstrained)
{
	if (ms_allowMouseCursorConstrained != allowMouseCursorConstrained)
	{
		ms_allowMouseCursorConstrained = allowMouseCursorConstrained;
		GraphicsNamespace::constrainMouseCursor();
	}
}

// ----------------------------------------------------------------------

void GraphicsNamespace::constrainMouseCursor()
{
	if (ms_allowMouseCursorConstrained && ConfigClientGraphics::getConstrainMouseCursorToWindow())
	{
		int x = 0;
		int y = 0;
		ms_translatePointFromGameToScreen(x, y);

		RECT r = ms_mouseCursorConstrainedRect;
		r.left    += x;
		r.top     += y;
		r.right   += x;
		r.bottom  += y;
		ClipCursor(&r);
	}
	else
		ClipCursor(NULL);
}

// ----------------------------------------------------------------------

void Graphics::constrainMouseCursor(int x0, int y0, int x1, int y1)
{
	ms_mouseCursorConstrained = true;
	ms_mouseCursorConstrainedRect.left   = x0;
	ms_mouseCursorConstrainedRect.top    = y0;
	ms_mouseCursorConstrainedRect.right  = x1;
	ms_mouseCursorConstrainedRect.bottom = y1;
	GraphicsNamespace::constrainMouseCursor();
}

// ----------------------------------------------------------------------

void Graphics::unconstrainMouseCursor()
{
	ms_mouseCursorConstrained = false;
	ms_mouseCursorConstrainedRect.left   = 0;
	ms_mouseCursorConstrainedRect.top    = 0;
	ms_mouseCursorConstrainedRect.right  = ms_frameBufferMaxWidth-1;
	ms_mouseCursorConstrainedRect.bottom = ms_frameBufferMaxHeight-1;
	GraphicsNamespace::constrainMouseCursor();
}

// ----------------------------------------------------------------------

#ifdef _DEBUG
void GraphicsNamespace::setDebugMouseCursorPosition(int x, int y)
{
	ms_debugMouseCursorPositionX = x;
	ms_debugMouseCursorPositionY = y;
}
#endif

// ----------------------------------------------------------------------
/**
 * Set the viewport for the graphics engine.
 *
 * @param x      Left edge of the viewport
 * @param y      Top edge of the viewport
 * @param width  Width of the viewport in pixels
 * @param height Height of the viewport in pixels
 * @param minZ   Minimum value of the Z-buffer to use
 * @param mazZ   Maximum value of the Z-buffer to use.
 */

void Graphics::setViewport(int x, int y, int width, int height, float minZ, float maxZ)
{
	DEBUG_FATAL(x < 0, ("viewport x < 0"));
	DEBUG_FATAL(y < 0, ("viewport y < 0"));
	DEBUG_FATAL(x+width  > ms_currentRenderTargetMaxWidth, ("viewport too wide %d > %d", x+width, ms_currentRenderTargetMaxWidth));
	DEBUG_FATAL(y+height > ms_currentRenderTargetMaxHeight, ("viewport too tall %d > %d", y+height, ms_currentRenderTargetMaxHeight));

	NOT_NULL(ms_api->setViewport);
	ms_api->setViewport(x, y, width, height, minZ, maxZ);

	ms_viewportX0     = x;
	ms_viewportY0     = y;
	ms_viewportWidth  = width;
	ms_viewportHeight = height;
}

// ----------------------------------------------------------------------

void Graphics::setScissorRect(bool enabled, int x, int y, int width, int height)
{
	DEBUG_FATAL(enabled && x < 0, ("viewport x < 0"));
	DEBUG_FATAL(enabled && y < 0, ("viewport y < 0"));
	DEBUG_FATAL(enabled && x+width  > ms_currentRenderTargetMaxWidth, ("viewport too wide %d > %d 0", x+width, ms_currentRenderTargetMaxWidth));
	DEBUG_FATAL(enabled && y+height > ms_currentRenderTargetMaxHeight, ("viewport too tall %d > %d", y+height, ms_currentRenderTargetMaxHeight));

	NOT_NULL(ms_api->setScissorRect);
	ms_api->setScissorRect(enabled, x, y, width, height);
}

// ----------------------------------------------------------------------

void Graphics::setWorldToCameraTransform(const Transform &transform, const Vector &cameraPosition)
{
	NOT_NULL(ms_api);
	NOT_NULL(ms_api->setWorldToCameraTransform);
	ms_api->setWorldToCameraTransform(transform, cameraPosition);
}

// ----------------------------------------------------------------------

void Graphics::setProjectionMatrix(const GlMatrix4x4 &projectionMatrix)
{
	NOT_NULL(ms_api);
	NOT_NULL(ms_api->setProjectionMatrix);
	ms_api->setProjectionMatrix(projectionMatrix);
}

// ----------------------------------------------------------------------

void Graphics::setFog(bool enabled, float density, const PackedArgb &color)
{
	NOT_NULL(ms_api);
	NOT_NULL(ms_api->setFog);
	ms_api->setFog(ms_disableFog ? false : enabled, density, color);

	ms_fogEnabled = enabled;
	ms_fogDensity = density;
	ms_fogColor = color;
}

// ----------------------------------------------------------------------

void Graphics::getFog(bool & enabled, float & density, PackedArgb & color)
{
	enabled = ms_fogEnabled;
	density = ms_fogDensity;
	color = ms_fogColor;
}
// ----------------------------------------------------------------------
/**
 * Set the object-to-world transform and scaling information.
 *
 * These values are used to process primitives that are specified in object
 * space.  The scale will be applied first, and then the vertices will be transformed
 * using the object-to-world transform.
 *
 * @param objectToWorld  New object-to-world transform for object-space vertices
 * @param scale  Scale to be applied to the object-space vertices
 */

void Graphics::setObjectToWorldTransformAndScale(const Transform &objectToWorld, const Vector &scale)
{
	NOT_NULL(ms_api->setObjectToWorldTransformAndScale);
	ms_api->setObjectToWorldTransformAndScale(objectToWorld, scale);
#ifdef _DEBUG
	ms_haveTransformAndScale = true;
#endif
}

// ----------------------------------------------------------------------
/**
 * Set a tag's global texture.
 *
 * Global textures are supported through specially tagged textures, those that
 * begin with an underscore.  These textures are automatically shared across
 * all textures that reference them.  This is useful for things like environment
 * maaps (_ENV), as well
 *
 * @param tag The tag for the global texture to set.
 * @param texture The texture to use for the specified tag.
 */

void Graphics::setGlobalTexture(Tag tag, const Texture &texture)
{
	NOT_NULL(ms_api->setGlobalTexture);
	ms_api->setGlobalTexture(tag, texture);
}

// ----------------------------------------------------------------------

void Graphics::releaseAllGlobalTextures()
{
	NOT_NULL(ms_api->releaseAllGlobalTextures);
	ms_api->releaseAllGlobalTextures();
}

// ----------------------------------------------------------------------

void Graphics::setTextureTransform(int stage, bool enabled, int dimension, bool projected, const float *transform)
{
	NOT_NULL(ms_api->setTextureTransform);
	ms_api->setTextureTransform(stage, enabled, dimension, projected, transform);
}

// ----------------------------------------------------------------------

void Graphics::setVertexShaderUserConstants(int index, float c0, float c1, float c2, float c3)
{
	NOT_NULL(ms_api->setVertexShaderUserConstants);
	ms_api->setVertexShaderUserConstants(index, c0, c1, c2, c3);
}

// ----------------------------------------------------------------------

void Graphics::setPixelShaderUserConstants(VectorRgba const * constants, int count)
{
	NOT_NULL(ms_api->setPixelShaderUserConstants);
	ms_api->setPixelShaderUserConstants(constants, count);
}

// ----------------------------------------------------------------------

ShaderImplementationGraphicsData  *Graphics::createShaderImplementationGraphicsData(const ShaderImplementation &shaderImplementation)
{
	NOT_NULL(ms_api->createShaderImplementationGraphicsData);
	return ms_api->createShaderImplementationGraphicsData(shaderImplementation);
}

// ----------------------------------------------------------------------
/**
 * Create the GlData for a StaticShader
 *
 * @param shader StaticShader to create the GlData for
 */

StaticShaderGraphicsData *Graphics::createStaticShaderGraphicsData(const StaticShader &shader)
{
	NOT_NULL(ms_api->createStaticShaderGraphicsData);
	return ms_api->createStaticShaderGraphicsData(shader);
}

// ----------------------------------------------------------------------

void Graphics::setLights(const stdvector<const Light*>::fwd &lightList)
{
	NOT_NULL(ms_api->setLights);
	ms_api->setLights(lightList);
}

// ----------------------------------------------------------------------

void Graphics::setAlphaFadeOpacity(bool enabled, float opacity)
{
	NOT_NULL(ms_api->setAlphaFadeOpacity);
	ms_api->setAlphaFadeOpacity(enabled, opacity);
}

// ----------------------------------------------------------------------
/**
 * Create the GlData for a StaticVertexBuffer.
 *
 * @internal
 * @param vertexBuffer  StaticVertexBuffer to create the GlData for.
 */

StaticVertexBufferGraphicsData *Graphics::createVertexBufferData(const StaticVertexBuffer &vertexBuffer)
{
	NOT_NULL(ms_api->createStaticVertexBufferData);
	return ms_api->createStaticVertexBufferData(vertexBuffer);
}

// ----------------------------------------------------------------------
/**
 * Create the GlData for a DynamicVertexBuffer.
 *
 * @internal
 * @param vertexBuffer  DynamicVertexBuffer to create the GlData for.
 */

DynamicVertexBufferGraphicsData *Graphics::createVertexBufferData(const DynamicVertexBuffer &vertexBuffer)
{
	NOT_NULL(ms_api->createDynamicVertexBufferData);
	return ms_api->createDynamicVertexBufferData(vertexBuffer);
}

// ----------------------------------------------------------------------

VertexBufferVectorGraphicsData *Graphics::createVertexBufferVectorData(VertexBufferVector const & vertexBufferVector)
{
	NOT_NULL(ms_api->createVertexBufferVectorData);
	return ms_api->createVertexBufferVectorData(vertexBufferVector);
}

// ----------------------------------------------------------------------
/**
 * Create the GlData for an IndexBuffer.
 *
 * @internal
 * @param indexBuffer IndexBuffer to create the GlData for.
 */

StaticIndexBufferGraphicsData *Graphics::createIndexBufferData(const StaticIndexBuffer &indexBuffer)
{
	NOT_NULL(ms_api);
	NOT_NULL(ms_api->createStaticIndexBufferData);
	return ms_api->createStaticIndexBufferData(indexBuffer);
}

// ----------------------------------------------------------------------
/**
 * Create the GlData for an IndexBuffer.
 *
 * @internal
 * @param indexBuffer IndexBuffer to create the GlData for.
 */

DynamicIndexBufferGraphicsData *Graphics::createIndexBufferData()
{
	NOT_NULL(ms_api);
	NOT_NULL(ms_api->createDynamicIndexBufferData);
	return ms_api->createDynamicIndexBufferData();
}

// ----------------------------------------------------------------------
/**
 * Set the max number of indices in a dynamic index buffer.
 *
 * @param numberOfIndices The number of vertices in the dynamic vertex buffer.
 */

void Graphics::setDynamicIndexBufferSize(int numberOfIndices)
{
	ms_api->setDynamicIndexBufferSize(numberOfIndices);
}

// ----------------------------------------------------------------------
/**
 * Retrieve the required u0,v0 and u1,v1 to map a texture with the given
 * width and height to a polygon that is guaranteed to draw width x height
 * pixels.
 *
 * @param textureWidth  [IN]  width of the texture in pixels
 * @param textureHeight  [IN]  height of the texture in pixels
 * @param u0  [OUT] u mapping to left of texture
 * @param v0  [OUT] v mapping to top of texture
 * @param u1  [OUT] u mapping to right of texture
 * @param v1  [OUT] v mapping to bottom of texture
 */

void Graphics::getOneToOneUVMapping(int textureWidth, int textureHeight, float &u0, float &v0, float &u1, float &v1)
{
	NOT_NULL(ms_api->getOneToOneUVMapping);
	ms_api->getOneToOneUVMapping(textureWidth, textureHeight, u0, v0, u1, v1);
}
// ----------------------------------------------------------------------
/**
 * Create the GlData for a Texture.
 *
 * @internal
 * @param texture The engine texture which to create the graphics texture data for.
 */

TextureGraphicsData *Graphics::createTextureData(const Texture &texture, const TextureFormat *runtimeFormats, int numberOfRuntimeFormats)
{
	NOT_NULL(ms_api->createTextureData);
	return ms_api->createTextureData(texture, runtimeFormats, numberOfRuntimeFormats);
}

// ----------------------------------------------------------------------

ShaderImplementationPassVertexShaderGraphicsData *Graphics::createVertexShaderData(ShaderImplementationPassVertexShader const &vertexShader)
{
	NOT_NULL(ms_api->createVertexShaderData);
	return ms_api->createVertexShaderData(vertexShader);
}

// ----------------------------------------------------------------------

ShaderImplementationPassPixelShaderProgramGraphicsData *Graphics::createPixelShaderProgramData(ShaderImplementationPassPixelShaderProgram const &pixelShaderProgram)
{
	NOT_NULL(ms_api->createPixelShaderProgramData);
	return ms_api->createPixelShaderProgramData(pixelShaderProgram);
}

// ----------------------------------------------------------------------
/**
 * Make sure the graphics layer is prepared to draw.
 *
 * This routine checks internal state values to make sure that the appropriate data has been set up for drawing.
 * @internal
 */

void GraphicsNamespace::realPredrawCheck()
{
	DEBUG_FATAL(!ms_haveBegin, ("Cannot draw -- not with beginScene/endScene pair"));
	DEBUG_FATAL(!ms_haveShader, ("Cannot draw -- no shader is set"));
	DEBUG_FATAL(ms_activeVertexBufferDynamicId && ms_activeVertexBufferDynamicId != DynamicVertexBuffer::getDynamicGlobalId(), ("Attempting to draw with a set dynamic VB after another one has been locked"));
}

// ----------------------------------------------------------------------
/**
 * Draw a 2d point.
 *
 * This routine will use the currently selected shader to draw the point.
 * The caller must supply the vertex ARGB components, but the shader
 * may not use them.
 *
 * @param x       2d X position of the point
 * @param y       2d Y position of the point
 * @param diffuse Diffuse color for the point
 */

const float Z = 1.0f;
const float OOZ = 1.0f;

void Graphics::drawPoint(
	float             x,    // 2d X position of the point
	float             y,    // 2d Y position of the point
	const VectorArgb &diffuse  // ARGB data for the point
	)
{
	VertexBufferFormat format;
	format.setPosition();
	format.setTransformed();
	format.setColor0();
	DynamicVertexBuffer  vertexBuffer(format);

	vertexBuffer.lock(1);

		VertexBufferWriteIterator v = vertexBuffer.begin();

		v.setPosition(static_cast<float>(x), static_cast<float>(y), Z);
		v.setOoz(OOZ);
		v.setColor0(diffuse);

	vertexBuffer.unlock();

	setVertexBuffer(vertexBuffer);
	drawPointList();
}

// ----------------------------------------------------------------------

void Graphics::drawPoint(int x, int y, const VectorArgb &argb)
{
	drawPoint(static_cast<float>(x), static_cast<float>(y), argb);
}

// ----------------------------------------------------------------------
/**
 * Draw a 2d line.
 *
 * This routine will use the currently selected shader to draw the point.
 * The caller must supply the vertex ARGB components, but the shader
 * may not use them.
 *
 * @param x0p     2d X position of the first endpoint
 * @param y0p     2d Y position of the first endpoint
 * @param x1p     2d X position of the second endpoint
 * @param y1p     2d Y position of the second endpoint
 * @param diffuse Diffuse color for the line
 */

void Graphics::drawLine(float x0p, float y0p, float x1p, float y1p, const VectorArgb &diffuse)
{
	VertexBufferFormat format;
	format.setPosition();
	format.setTransformed();
	format.setColor0();
	DynamicVertexBuffer vertexBuffer(format);

	vertexBuffer.lock(2);

		VertexBufferWriteIterator v = vertexBuffer.begin();

		v.setPosition(static_cast<float>(x0p), static_cast<float>(y0p), Z);
		v.setOoz(OOZ);
		v.setColor0(diffuse);
		++v;

		v.setPosition(static_cast<float>(x1p), static_cast<float>(y1p), Z);
		v.setOoz(OOZ);
		v.setColor0(diffuse);

	vertexBuffer.unlock();

	setVertexBuffer(vertexBuffer);
	drawLineList();
}

// ----------------------------------------------------------------------

void Graphics::drawLine(int x0p, int y0p, int x1p, int y1p, const VectorArgb &diffuse)
{
	drawLine(static_cast<float>(x0p), static_cast<float>(y0p), static_cast<float>(x1p), static_cast<float>(y1p), diffuse);
}

// ----------------------------------------------------------------------
/**
 * Draw a 2d circle
 *
 * @todo        This routine should really be rewritten to not plot points!
 * @param sx    2d X position of the circle's center
 * @param sy    2d Y position of the circle's center
 * @param r     Radius of the circle
 * @param color Color0 color for the line
 */

void Graphics::drawCircle(int sx, int sy, int r, const VectorArgb &diffuse)
{
#define CIRCLE_ADD_POINT(x,y) if (x > ms_viewportX0 && x < ms_viewportX0 + ms_viewportWidth && y > ms_viewportY0 && y < ms_viewportY0 + ms_viewportHeight) pointList.push_back(std::make_pair(x, y))

	std::vector<std::pair<int, int> > pointList;

	int x     = 0;
	int y     = r;
	int delta = 2 * (1 - r);
	int limit = 0;
	CIRCLE_ADD_POINT (x+sx, y+sy);
	CIRCLE_ADD_POINT (-x+sx, y+sy);
	CIRCLE_ADD_POINT (x+sx, -y+sy);
	CIRCLE_ADD_POINT (-x+sx, -y+sy);
	while (y >= limit)
	{
		if (delta < 0)
		{
			const int lambda = 2*delta + 2*y - 1;
			if (lambda > 0)
			{
				x++;
				y--;
				delta += 2*x - 2*y + 2;
			}
			else
			{
				x++;
				delta += 2*x + 1;
			}
		}
		else
			if (delta > 0)
			{
				const int lambda = 2*delta - 2*x - 1;
				if (lambda > 0)
				{
					y--;
					delta -= 2*y + 1;
				}
				else
				{
					x++;
					y--;
					delta += 2*x - 2*y + 2;
				}
			}
			else
			{
				x++;
				y--;
				delta += 2*x - 2*y + 2;
			}

		CIRCLE_ADD_POINT (x+sx, y+sy);
		CIRCLE_ADD_POINT (-x+sx, y+sy);
		CIRCLE_ADD_POINT (x+sx, -y+sy);
		CIRCLE_ADD_POINT (-x+sx, -y+sy);
	}

	if (pointList.empty())
		return;

	VertexBufferFormat format;
	format.setPosition();
	format.setTransformed();
	format.setColor0();
	DynamicVertexBuffer vertexBuffer(format);

	vertexBuffer.lock(pointList.size());

		VertexBufferWriteIterator v = vertexBuffer.begin();

		uint i;
		for (i = 0; i < pointList.size(); ++i)
		{
			v.setPosition(static_cast<float>(pointList[i].first), static_cast<float>(pointList[i].second), Z);
			v.setOoz(OOZ);
			v.setColor0(diffuse);
			++v;
		}

	vertexBuffer.unlock ();

	setVertexBuffer(vertexBuffer);
	drawPointList();
}

// ----------------------------------------------------------------------
/**
 * Draw a 2d rectangle.
 *
 * This routine will use the currently selected shader to draw the rectangle.
 * The caller must supply the vertex ARGB components, but the shader
 * may not use them.
 *
 * @param x0     2d X position of the first endpoint
 * @param y0     2d Y position of the first endpoint
 * @param x1     2d X position of the second endpoint
 * @param y1     2d Y position of the second endpoint
 * @param argb   Diffuse color for the line
 */

void Graphics::drawRectangle(float x0, float y0, float x1, float y1, const VectorArgb &argb)
{
	// @todo implement the other draw rectangle in terms of this one
	Rectangle2d rectangle;
	rectangle.x0 = x0;
	rectangle.y0 = y0;
	rectangle.x1 = x1;
	rectangle.y1 = y1;

	drawRectangle (rectangle, argb);
}

// ----------------------------------------------------------------------

void Graphics::drawRectangle(int x0, int y0, int x1, int y1, const VectorArgb &argb)
{
	drawRectangle(static_cast<float>(x0), static_cast<float>(y0), static_cast<float>(x1), static_cast<float>(y1), argb);
}

// ----------------------------------------------------------------------
/**
 * Draw a 2d rectangle.
 *
 * This routine will use the currently selected shader to draw the rectangle.
 * The caller must supply the vertex ARGB components, but the shader
 * may not use them.
 *
 * @param rectangle  2d rectangle
 * @param diffuse    Diffuse color for the line
 */

void Graphics::drawRectangle(const Rectangle2d &rectangle, const VectorArgb &diffuse)
{
	VertexBufferFormat format;
	format.setPosition();
	format.setTransformed();
	format.setColor0();
	DynamicVertexBuffer vertexBuffer(format);

	vertexBuffer.lock(5);

		VertexBufferWriteIterator v = vertexBuffer.begin();

		v.setPosition(rectangle.x0, rectangle.y0, Z);
		v.setOoz(OOZ);
		v.setColor0(diffuse);
		++v;

		v.setPosition(rectangle.x1, rectangle.y0, Z);
		v.setOoz(OOZ);
		v.setColor0(diffuse);
		++v;

		v.setPosition(rectangle.x1, rectangle.y1, Z);
		v.setOoz(OOZ);
		v.setColor0(diffuse);
		++v;

		v.setPosition(rectangle.x0, rectangle.y1, Z);
		v.setOoz(OOZ);
		v.setColor0(diffuse);
		++v;

		v.setPosition(rectangle.x0, rectangle.y0, Z);
		v.setOoz(OOZ);
		v.setColor0(diffuse);
		++v;

	vertexBuffer.unlock();

	setVertexBuffer(vertexBuffer);
	drawLineStrip();
}

// ----------------------------------------------------------------------
/**
 * Draw a 3d point.
 *
 * This routine will use the currently selected shader, object-to-world
 * transform, and camera data to draw the point.
 *
 * The caller must supply the vertex ARGB components, but the shader
 * may not use them.
 *
 * @param position  Object space position of the point
 * @param diffuse   Diffuse color for the point
 */

void Graphics::drawPoint(const Vector &position, const VectorArgb &diffuse)
{
	VertexBufferFormat format;
	format.setPosition();
	format.setColor0();
	DynamicVertexBuffer vertexBuffer(format);

	vertexBuffer.lock(1);

		VertexBufferWriteIterator v = vertexBuffer.begin();
		v.setPosition(position);
		v.setColor0(diffuse);

	vertexBuffer.unlock();

	setVertexBuffer(vertexBuffer);
	drawPointList();
}

// ----------------------------------------------------------------------
/**
 * Draw a 3d line.
 *
 * This routine will use the currently selected shader, object-to-world
 * transform, and camera data to draw the point.
 *
 * The caller must supply the vertex ARGB components, but the shader
 * may not use them.
 *
 * @param v0      Object space position of the first endpoint
 * @param v1      Object space position of the second endpoint
 * @param diffuse Diffuse color for the line
 */

void Graphics::drawLine(const Vector &v0, const Vector &v1, const VectorArgb &diffuse)
{
	VertexBufferFormat format;
	format.setPosition();
	format.setColor0();
	DynamicVertexBuffer vertexBuffer(format);

	vertexBuffer.lock(2);

		VertexBufferWriteIterator v = vertexBuffer.begin();

		v.setPosition(v0);
		v.setColor0(diffuse);
		++v;

		v.setPosition(v1);
		v.setColor0(diffuse);

	vertexBuffer.unlock();

	setVertexBuffer(vertexBuffer);
	drawLineList();
}

// ----------------------------------------------------------------------
/**
 * Draw a frame-of-reference.
 *
 * This routine will draw 3 lines of the specified length to describe
 * the frame of reference of the current object-to-world transform.  The
 * line along the X axis will be red, the Y axis is green, and the Z axis
 * is blue.
 *
 * @param radius  Length of the
 */

void Graphics::drawFrame(float radius, uint8 alpha)
{
	VertexBufferFormat format;
	format.setPosition();
	format.setColor0();
	DynamicVertexBuffer vertexBuffer(format);

	PackedArgb const red(alpha, 255, 0, 0);
	PackedArgb const green(alpha, 0, 255, 0);
	PackedArgb const blue(alpha, 0, 0, 255);

	vertexBuffer.lock(6);

		VertexBufferWriteIterator v = vertexBuffer.begin();

		v.setPosition(Vector::zero);
		v.setColor0(red);
		++v;

		v.setPosition(radius, 0.0f, 0.0f);
		v.setColor0(red);
		++v;

		v.setPosition(Vector::zero);
		v.setColor0(green);
		++v;

		v.setPosition(0.0f, radius, 0.0f);
		v.setColor0(green);
		++v;

		v.setPosition(Vector::zero);
		v.setColor0(blue);
		++v;

		v.setPosition(0.0f, 0.0f, radius);
		v.setColor0(blue);

	vertexBuffer.unlock();

	setVertexBuffer(vertexBuffer);
	drawLineList();
}

// ----------------------------------------------------------------------
/**
 * Render an outline cube.
 *
 * This routine will render an outlined cube with the specified radius,
 * which is interpreted as half the length of the cube's edge.  The points
 * of the cube will be computed in object space, and thus will be modified
 * by the object-to-world transform and scale.
 *
 * @param point   Center of the cube
 * @param radius  Half the edge length of the cube
 * @param diffuse Color for the cube line segments
 */

void Graphics::drawCube(const Vector &point, float radius, const VectorArgb &diffuse)
{
	VertexBufferFormat format;
	format.setPosition();
	format.setColor0();
	DynamicVertexBuffer vertexBuffer(format);

	vertexBuffer.lock(8);

		VertexBufferWriteIterator v = vertexBuffer.begin();

		v.setPosition(point - Vector(-radius,  radius,  radius));
		v.setColor0(diffuse);
		++v;

		v.setPosition(point - Vector( radius,  radius,  radius));
		v.setColor0(diffuse);
		++v;

		v.setPosition(point - Vector( radius,  radius, -radius));
		v.setColor0(diffuse);
		++v;

		v.setPosition(point - Vector(-radius,  radius, -radius));
		v.setColor0(diffuse);
		++v;

		v.setPosition(point - Vector(-radius, -radius,  radius));
		v.setColor0(diffuse);
		++v;

		v.setPosition(point - Vector( radius, -radius,  radius));
		v.setColor0(diffuse);
		++v;

		v.setPosition(point - Vector( radius, -radius, -radius));
		v.setColor0(diffuse);
		++v;

		v.setPosition(point - Vector(-radius, -radius, -radius));
		v.setColor0(diffuse);

	vertexBuffer.unlock();

	const Index indices[24] =
	{
		0, 1,
		0, 3,
		0, 4,
		2, 1,
		2, 3,
		2, 6,
		5, 1,
		5, 4,
		5, 6,
		7, 3,
		7, 6,
		7, 4
	};

	DynamicIndexBuffer indexBuffer;
	indexBuffer.lock (24);
		memcpy(indexBuffer.begin(), indices, sizeof(indices));
	indexBuffer.unlock ();

	setVertexBuffer(vertexBuffer);
	setIndexBuffer(indexBuffer);
	drawIndexedLineList();
}

// ----------------------------------------------------------------------
/**
 * Render a circle.
 *
 * This routine will render a circle in 3-space given a center point,
 * a normal vector, a radius, the number of segments in the circle, and
 * the circle color.  This function makes use of a static buffer to allow rendering
 * of arbitrarily-sided circles without resorting to dynamic memory.  The points
 * of the circle will be computed in object space, and thus will be modified by
 * the object-to-world transform and scale.
 *
 * @param center center point of the circle
 * @param normal vector that is normal to the circle
 * @param radius radius of the circle
 * @param segments the number of segments used to build the circle
 * @param argb the color of the circle
 */

void Graphics::drawCircle(const Vector &center, const Vector &normal, float radius, int segments, const VectorArgb &argb)
{
	UNREF (center);
	UNREF (normal);
	UNREF (radius);
	UNREF (segments);
	UNREF (argb);

	const float width = radius * 2;
	drawBox (center + Vector (-radius, -radius, -radius), width, width, width, argb);
}

// ----------------------------------------------------------------------
/**
 * Render a debug sphere.
 * The ring for the X axis will be red, Y green, and Z blue.  The other rings will be grey.
 *
 * @param center The center point of the sphere.
 * @param radius The radius of the sphere.
 * @param rings The number of rings on each axis.  This value must be even and than 1.
 * @param segments The number of segments for each rich.  This value must be even and than 2.
 */

void Graphics::drawSphere(const Vector &center, const float radius, const int rings, const int segments)
{
	DEBUG_FATAL(rings & 1, ("rings must be even"));
	DEBUG_FATAL(rings < 2, ("rings must >= 2"));
	DEBUG_FATAL(segments < 3, ("rings must >= 3"));

	VertexBufferFormat format;
	format.setPosition();
	format.setColor0();
	DynamicVertexBuffer vertexBuffer(format);

	const  PackedArgb grey(255, 128, 128, 128);
	const float       segmentStep = PI_TIMES_2 / segments;
	const float       ringStep = PI / rings;
	Transform         transform;

	// do the rings around the X axis
	{
		for (int i = 0; i < rings; ++i, transform.pitch_l(ringStep))
		{
			vertexBuffer.lock(segments+1);
			{
				VertexBufferWriteIterator vbi = vertexBuffer.begin();
				float angle = 0;

				PackedArgb color = grey;
				if (i == 0)
					color = PackedArgb::solidRed;
				if (i == 5)
					color = PackedArgb::solidBlue;

				for (int j = 0; j < segments; ++j, ++vbi, angle += segmentStep)
				{
					vbi.setPosition(center + transform.rotateTranslate_l2p(Vector(radius * cos(angle), 0, radius * sin(angle))));
					vbi.setColor0(color);
				}

				vbi.setPosition(center + transform.rotateTranslate_l2p(Vector(radius * cos(0.0f), 0, radius * sin(0.0f))));
				vbi.setColor0(color);
			}

			vertexBuffer.unlock();
			setVertexBuffer(vertexBuffer);
			drawLineStrip();
		}
	}

	// do the rings around the Z axis
	{
		transform.resetRotateTranslate_l2p();
		transform.roll_l(ringStep);
		for (int i = 1; i < rings; ++i, transform.roll_l(ringStep))
		{
			vertexBuffer.lock(segments+1);
			{
				VertexBufferWriteIterator vbi = vertexBuffer.begin();
				float angle = 0;

				PackedArgb color = grey;
				if (i == 5)
					color = PackedArgb::solidGreen;

				for (int j = 0; j < segments; ++j, ++vbi, angle += segmentStep)
				{
					vbi.setPosition(center + transform.rotateTranslate_l2p(Vector(radius * cos(angle), 0, radius * sin(angle))));
					vbi.setColor0(color);
				}

				vbi.setPosition(center + transform.rotateTranslate_l2p(Vector(radius * cos(0.0f), 0, radius * sin(0.0f))));
				vbi.setColor0(color);
			}

			vertexBuffer.unlock();
			setVertexBuffer(vertexBuffer);
			drawLineStrip();
		}
	}
}

void Graphics::drawSphere2(const Vector &center, const float radius, const int tessTheta, const int nLongitudes, const int nLatitudes, const VectorArgb & color )
{
	VertexBufferFormat format;
	format.setPosition();
	format.setColor0();
	DynamicVertexBuffer vertexBuffer(format);

	// ----------
	// Longitude lines for sphere
	{
		for (int j = 0; j < nLongitudes; j++)
		{
			float theta = (PI_TIMES_2 / float(nLongitudes)) * float(j);

			int tessPhi = tessTheta / 2;

			vertexBuffer.lock( tessPhi + 1 );
			VertexBufferWriteIterator vbi = vertexBuffer.begin();

			for(int i = 0; i <= tessPhi; i++)
			{
				float phi = (PI / float(tessPhi)) * float(i);

				float x = radius * sin(phi) * cos(theta);
				float y = radius * cos(phi);
				float z = radius * sin(phi) * sin(theta);

				vbi.setPosition(center + Vector(x,y,z));
				vbi.setColor0(color);
				++vbi;
			}

			vertexBuffer.unlock();
			setVertexBuffer(vertexBuffer);
			drawLineStrip();

		}
	}

	// ----------
	// Latitude lines for sphere
	{
		for (int j = 1; j < nLatitudes + 1; j++)
		{
			float phi = (PI / float(nLatitudes + 1)) * float(j);

			vertexBuffer.lock( tessTheta + 1 );
			VertexBufferWriteIterator vbi = vertexBuffer.begin();

			for(int i = 0; i <= tessTheta; i++)
			{
				float theta = (PI_TIMES_2 / float(tessTheta)) * float(i);

				float x = radius * sin(phi) * cos(theta);
				float y = radius * cos(phi);
				float z = radius * sin(phi) * sin(theta);

				vbi.setPosition(center + Vector(x,y,z));
				vbi.setColor0(color);
				++vbi;
			}

			vertexBuffer.unlock();
			setVertexBuffer(vertexBuffer);
			drawLineStrip();
		}
	}
}

void Graphics::drawXZCircle(const Vector &center, const float radius, const int tessTheta, const VectorArgb & color )
{
	VertexBufferFormat format;
	format.setPosition();
	format.setColor0();
	DynamicVertexBuffer vertexBuffer(format);

	// ----------

	vertexBuffer.lock( tessTheta + 1 );
	VertexBufferWriteIterator vbi = vertexBuffer.begin();

	for(int i = 0; i <= tessTheta; i++)
	{
		float theta = (PI_TIMES_2 / float(tessTheta)) * float(i);

		float x = radius * cos(theta);
		float y = 0;
		float z = radius * sin(theta);

		vbi.setPosition(center + Vector(x,y,z));
		vbi.setColor0(color);
		++vbi;
	}

	vertexBuffer.unlock();
	setVertexBuffer(vertexBuffer);
	drawLineStrip();
}

void Graphics::drawOctahedron(const Vector &center, const float radius, const VectorArgb & color )
{
	VertexBufferFormat format;
	format.setPosition();
	format.setColor0();
	DynamicVertexBuffer vertexBuffer(format);

	// ----------

	Vector dX( radius, 0.0f, 0.0f );
	Vector dY( 0.0f, radius, 0.0f );
	Vector dZ( 0.0f, 0.0f, radius );

	vertexBuffer.lock( 24 );
	VertexBufferWriteIterator vbi = vertexBuffer.begin();

	// X-Y plane

	vbi.setPosition(center + dX);	vbi.setColor0(color);	++vbi;
	vbi.setPosition(center + dY);	vbi.setColor0(color);	++vbi;

	vbi.setPosition(center + dX);	vbi.setColor0(color);	++vbi;
	vbi.setPosition(center - dY);	vbi.setColor0(color);	++vbi;

	vbi.setPosition(center - dX);	vbi.setColor0(color);	++vbi;
	vbi.setPosition(center + dY);	vbi.setColor0(color);	++vbi;

	vbi.setPosition(center - dX);	vbi.setColor0(color);	++vbi;
	vbi.setPosition(center - dY);	vbi.setColor0(color);	++vbi;


	// X-Z plane

	vbi.setPosition(center + dX);	vbi.setColor0(color);	++vbi;
	vbi.setPosition(center + dZ);	vbi.setColor0(color);	++vbi;

	vbi.setPosition(center + dX);	vbi.setColor0(color);	++vbi;
	vbi.setPosition(center - dZ);	vbi.setColor0(color);	++vbi;

	vbi.setPosition(center - dX);	vbi.setColor0(color);	++vbi;
	vbi.setPosition(center + dZ);	vbi.setColor0(color);	++vbi;

	vbi.setPosition(center - dX);	vbi.setColor0(color);	++vbi;
	vbi.setPosition(center - dZ);	vbi.setColor0(color);	++vbi;


	// Y-Z plane

	vbi.setPosition(center + dY);	vbi.setColor0(color);	++vbi;
	vbi.setPosition(center + dZ);	vbi.setColor0(color);	++vbi;

	vbi.setPosition(center + dY);	vbi.setColor0(color);	++vbi;
	vbi.setPosition(center - dZ);	vbi.setColor0(color);	++vbi;

	vbi.setPosition(center - dY);	vbi.setColor0(color);	++vbi;
	vbi.setPosition(center + dZ);	vbi.setColor0(color);	++vbi;

	vbi.setPosition(center - dY);	vbi.setColor0(color);	++vbi;
	vbi.setPosition(center - dZ);	vbi.setColor0(color);	++vbi;

	vertexBuffer.unlock();
	setVertexBuffer(vertexBuffer);
	drawLineList();
}

void Graphics::drawCylinder(const Vector &base, const float radius, const float height, const int tessTheta, const int tessRho, const int tessZ, int nSpokes, const VectorArgb & color )
{
	VertexBufferFormat format;
	format.setPosition();
	format.setColor0();
	DynamicVertexBuffer vertexBuffer(format);

	// ----------
	// Vertical lines for cylinder

	vertexBuffer.lock( nSpokes * 2 );
	VertexBufferWriteIterator vbi = vertexBuffer.begin();

	for (int i = 0; i < nSpokes; i++)
	{
		float angle = (PI_TIMES_2 / float(nSpokes)) * float(i);

		float x = radius * cos(angle);
		float y = height;
		float z = radius * sin(angle);

		vbi.setPosition(base + Vector(x,0,z));
		vbi.setColor0(color);
		++vbi;

		vbi.setPosition(base + Vector(x,y,z));
		vbi.setColor0(color);
		++vbi;

	}

	vertexBuffer.unlock();
	setVertexBuffer(vertexBuffer);
	drawLineList();

	// ----------
	// Horizontal lines for the cylinder

	for(int j = 0; j <= tessZ; j++)
	{
		vertexBuffer.lock( tessTheta + 1 );
		VertexBufferWriteIterator vbi = vertexBuffer.begin();

		for (int i = 0; i <= tessTheta; i++)
		{
			float angle = (PI_TIMES_2 / float(tessTheta)) * float(i);

			float x = radius * cos(angle);
			float y = (height / float(tessZ)) * float(j);
			float z = radius * sin(angle);

			vbi.setPosition(base + Vector(x,y,z));
			vbi.setColor0(color);
			++vbi;
		}

		vertexBuffer.unlock();
		setVertexBuffer(vertexBuffer);
		drawLineStrip();
	}

	// ----------
	// Cap rays for the cylinder

	for(int k = 0; k < 2; k++)
	{
		float y = height * float(k);

		vertexBuffer.lock( nSpokes * 4 );
		VertexBufferWriteIterator vbi = vertexBuffer.begin();

		for (int i = 0; i < nSpokes; i++)
		{
			float angle = (PI_TIMES_2 / float(nSpokes)) * float(i);

			float x = radius * cos(angle);
			float z = radius * sin(angle);

			vbi.setPosition(base + Vector(0,0,0));
			vbi.setColor0(color);
			++vbi;

			vbi.setPosition(base + Vector(x,0,z));
			vbi.setColor0(color);
			++vbi;

			vbi.setPosition(base + Vector(0,y,0));
			vbi.setColor0(color);
			++vbi;

			vbi.setPosition(base + Vector(x,y,z));
			vbi.setColor0(color);
			++vbi;
		}

		vertexBuffer.unlock();
		setVertexBuffer(vertexBuffer);
		drawLineList();
	}

	// ----------
	// Cap rings for the cylinder
	{
		for(int k = 0; k < 2; k++)
		{
			float y = height * float(k);

			for(int j = 1; j < tessRho; j++)
			{
				float rho = (radius / float(tessRho)) * float(j);

				vertexBuffer.lock( tessTheta + 1 );
				VertexBufferWriteIterator vbi = vertexBuffer.begin();

				for (int i = 0; i <= tessTheta; i++)
				{
					float angle = (PI_TIMES_2 / float(tessTheta)) * float(i);

					float x = rho * cos(angle);
					float z = rho * sin(angle);

					vbi.setPosition(base + Vector(x,y,z));
					vbi.setColor0(color);
					++vbi;
				}

				vertexBuffer.unlock();
				setVertexBuffer(vertexBuffer);
				drawLineStrip();
			}
		}
	}
}

// ----------------------------------------------------------------------
/**
 * Render a camera frustum.
 *
 * @param frustumVertices The vertices return by Camera::getFrustumVertices.
 */

void Graphics::drawFrustum(const Vector *frustumVertices, const PackedArgb &color)
{
	VertexBufferFormat format;
	format.setPosition();
	format.setColor0();

	DynamicVertexBuffer vertexBuffer(format);
	vertexBuffer.lock(8);
		VertexBufferWriteIterator vbi = vertexBuffer.begin();
		for (int i = 0; i < 8; ++i, ++vbi)
		{
			vbi.setPosition(frustumVertices[i]);
			vbi.setColor0(color);
		}
	vertexBuffer.unlock();

	DynamicIndexBuffer indexBuffer;
	indexBuffer.lock(24);
		indexBuffer.begin()[ 0] = 0;
		indexBuffer.begin()[ 1] = 1;
		indexBuffer.begin()[ 2] = 1;
		indexBuffer.begin()[ 3] = 2;
		indexBuffer.begin()[ 4] = 2;
		indexBuffer.begin()[ 5] = 3;
		indexBuffer.begin()[ 6] = 3;
		indexBuffer.begin()[ 7] = 0;

		indexBuffer.begin()[ 8] = 4;
		indexBuffer.begin()[ 9] = 5;
		indexBuffer.begin()[10] = 5;
		indexBuffer.begin()[11] = 6;
		indexBuffer.begin()[12] = 6;
		indexBuffer.begin()[13] = 7;
		indexBuffer.begin()[14] = 7;
		indexBuffer.begin()[15] = 4;

		indexBuffer.begin()[16] = 0;
		indexBuffer.begin()[17] = 4;
		indexBuffer.begin()[18] = 1;
		indexBuffer.begin()[19] = 5;
		indexBuffer.begin()[20] = 2;
		indexBuffer.begin()[21] = 6;
		indexBuffer.begin()[22] = 3;
		indexBuffer.begin()[23] = 7;
	indexBuffer.unlock();

	Graphics::setVertexBuffer(vertexBuffer);
	Graphics::setIndexBuffer(indexBuffer);
	Graphics::drawIndexedLineList();
}

// ----------------------------------------------------------------------
/**
 * Render an outline box.
 *
 * This routine will render an outlined box given the left-rear-bottom corner,
 * height, width, and depth.  The points of the box will be computed in
 * object space, and thus will be modified by the object-to-world transform
 * and scale.
 *
 * @param leftRearBottomCorner the rear-left-bottom corner of the bos
 * @param height height of the box
 * @param width width of the box
 * @param depth depth of the box
 * @param diffuse color of the box
 */

void Graphics::drawBox(const Vector &leftRearBottomCorner, float height, float width, float depth, const VectorArgb &diffuse)
{
	VertexBufferFormat format;
	format.setPosition();
	format.setColor0();
	DynamicVertexBuffer vertexBuffer(format);

	//   6---7
	//  /|  /|
	// 2-+-3 |
	// | 4-+-5  <----back (larger z)
	// |/  |/
	// 0---1  <----rear (smaller z)

	//leftRearBottomCorner represents point 0 in the picture

	vertexBuffer.lock(8);

		VertexBufferWriteIterator v = vertexBuffer.begin();

		//rear points
		v.setPosition(leftRearBottomCorner.x, leftRearBottomCorner.y, leftRearBottomCorner.z);
		v.setColor0(diffuse);
		++v;

		v.setPosition(leftRearBottomCorner.x + width, leftRearBottomCorner.y, leftRearBottomCorner.z);
		v.setColor0(diffuse);
		++v;

		v.setPosition(leftRearBottomCorner.x, leftRearBottomCorner.y + height, leftRearBottomCorner.z);
		v.setColor0(diffuse);
		++v;

		v.setPosition(leftRearBottomCorner.x + width, leftRearBottomCorner.y + height, leftRearBottomCorner.z);
		v.setColor0(diffuse);
		++v;

		//back points
		v.setPosition(leftRearBottomCorner.x, leftRearBottomCorner.y, leftRearBottomCorner.z + depth);
		v.setColor0(diffuse);
		++v;

		v.setPosition(leftRearBottomCorner.x + width, leftRearBottomCorner.y, leftRearBottomCorner.z + depth);
		v.setColor0(diffuse);
		++v;

		v.setPosition(leftRearBottomCorner.x, leftRearBottomCorner.y + height, leftRearBottomCorner.z + depth);
		v.setColor0(diffuse);
		++v;

		v.setPosition(leftRearBottomCorner.x + width, leftRearBottomCorner.y + height, leftRearBottomCorner.z + depth);
		v.setColor0(diffuse);

	vertexBuffer.unlock();

	const Index indices[24] =
	{
		0, 1,
		0, 2,
		0, 4,
		1, 3,
		1, 5,
		2, 3,
		2, 6,
		4, 5,
		4, 6,
		7, 3,
		7, 5,
		7, 6
	};
	DynamicIndexBuffer indexBuffer;
	indexBuffer.lock (24);
		memcpy(indexBuffer.begin(), indices, sizeof(indices));
	indexBuffer.unlock ();

	setVertexBuffer(vertexBuffer);
	setIndexBuffer(indexBuffer);
	drawIndexedLineList();
}

// ----------------------------------------------------------------------

void Graphics::drawBox(const Vector &extentMin, const Vector& extentMax, const VectorArgb &diffuse)
{
	VertexBufferFormat format;
	format.setPosition();
	format.setColor0();
	DynamicVertexBuffer vertexBuffer(format);

	// 0---1     .
	// |\  |\    .
	// | 4-+-5   .
	// 3-+-2 |   .
	//  \|  \|   .
	//   7---6   .

	float left   = extentMin.x;
	float right  = extentMax.x;
	float top    = extentMax.y;
	float bottom = extentMin.y;
	float front  = extentMax.z;
	float back   = extentMin.z;

	vertexBuffer.lock(8);

		VertexBufferWriteIterator v = vertexBuffer.begin();

		v.setPosition(left,  top,    front);
		v.setColor0(diffuse);
		++v;

		v.setPosition(right, top,    front);
		v.setColor0(diffuse);
		++v;

		v.setPosition(right, bottom, front);
		v.setColor0(diffuse);
		++v;

		v.setPosition(left,  bottom, front);
		v.setColor0(diffuse);
		++v;

		v.setPosition(left,  top,    back);
		v.setColor0(diffuse);
		++v;

		v.setPosition(right, top,    back);
		v.setColor0(diffuse);
		++v;

		v.setPosition(right, bottom, back);
		v.setColor0(diffuse);
		++v;

		v.setPosition(left,  bottom, back);
		v.setColor0(diffuse);

	vertexBuffer.unlock();

	const Index indices[] =
	{
		0, 1,
		1, 2,
		2, 3,
		3, 0,

		4, 5,
		5, 6,
		6, 7,
		7, 4,

		0, 4,
		1, 5,
		2, 6,
		3, 7
	};
	DynamicIndexBuffer indexBuffer;
	indexBuffer.lock (24);
		memcpy(indexBuffer.begin(), indices, sizeof(indices));
	indexBuffer.unlock ();

	setVertexBuffer(vertexBuffer);
	setIndexBuffer(indexBuffer);
	drawIndexedLineList();
}

// ----------------------------------------------------------------------
/**
 * Render the normals of a VertexBuffer.
 *
 * This routine will render the normal vectors of a VertexBuffer.  It will
 * Fatal in debug builds if VBFF_normals is not set for the VertexBuffer.
 * It will render each normal as a green line with a red tip (to show the
 * normal's direction).
 *
 * @param vertexBuffer the vertex array whose normals we want to draw
 * @param scale length of the normal (defaults to 1)
 * @param argbLine color of line (defaults to VectorArgb::solidGreen)
 * @param argbEndpoint color of point on tip (defaults to VectorArgb::solidRed)
 */

void Graphics::drawVertexNormals(VertexBufferReadIterator first, const VertexBufferReadIterator &last, float scale, const VectorArgb &argbLine, const VectorArgb &argbEndpoint)
{
	Vector pos;
	Vector normal;

	for ( ; first != last;  ++first)
	{
		pos    = first.getPosition();
		normal = first.getNormal();

	  //draw the line
		Graphics::drawLine (pos, pos+(scale*normal), argbLine);

	//draw a point on the tip to show direction
		Graphics::drawPoint(pos+(scale*normal), argbEndpoint);
	}
}

// ----------------------------------------------------------------------

void Graphics::drawVertexMatrixFrames(VertexBufferReadIterator first, const VertexBufferReadIterator &last, float scale)
{
	const int useTextureCoordinateSet = first.getNumberOfTextureCoordinateSets() - 1;
	if (useTextureCoordinateSet < 0 || first.getTextureCoordinateSetDimension(useTextureCoordinateSet) != 4)
		return;

	Vector pos, i, j, k;
	float flip;
	for ( ; first != last;  ++first)
	{
		pos = first.getPosition();

		first.getTextureCoordinates(useTextureCoordinateSet, i.x, i.y, i.z, flip);
		k = first.getNormal();
		j = k.cross(i) * flip;

		Graphics::drawLine (pos, pos+(i*scale), VectorArgb::solidRed);
		Graphics::drawLine (pos, pos+(j*scale), VectorArgb::solidGreen);
		Graphics::drawLine (pos, pos+(k*scale), (flip >= 0) ? VectorArgb::solidBlue : VectorArgb::solidCyan);
	}
}

// ----------------------------------------------------------------------

void Graphics::drawExtent (const Extent* extent, const VectorArgb& color)
{
	NOT_NULL (extent);

	//-- the engine doesn't know anything about the graphics layer, so I
	//		can't have Extent::render exist to know how to render
	//		sub-classed extents. this is the best thing I could think of

	const BoxExtent* boxExtent = dynamic_cast<const BoxExtent*> (extent);
	if (boxExtent)
	{
		drawBox (boxExtent->getMin (), boxExtent->getMax (), color);
	}
	else
	{
		//-- draw sphere extents
		drawCircle (extent->getSphere().getCenter (), Vector::unitX, extent->getSphere().getRadius (), 20, color);
		drawCircle (extent->getSphere().getCenter (), Vector::unitY, extent->getSphere().getRadius (), 20, color);
		drawCircle (extent->getSphere().getCenter (), Vector::unitZ, extent->getSphere().getRadius (), 20, color);
	}
}

// ----------------------------------------------------------------------

void Graphics::setVertexBuffer(const HardwareVertexBuffer &vertexBuffer)
{
	NOT_NULL(ms_api);
	NOT_NULL(ms_api->setVertexBuffer);
	ms_api->setVertexBuffer(vertexBuffer);
}

// ----------------------------------------------------------------------

void Graphics::setVertexBuffer(VertexBufferVector const & vertexBufferVector)
{
	NOT_NULL(ms_api);
	NOT_NULL(ms_api->setVertexBufferVector);
	ms_api->setVertexBufferVector(vertexBufferVector);
}

// ----------------------------------------------------------------------

void Graphics::setIndexBuffer(const HardwareIndexBuffer &indexBuffer)
{
#ifdef _DEBUG
	if (indexBuffer.getType() == HardwareIndexBuffer::T_dynamic)
	{
		const DynamicIndexBuffer *dynamicIndexBuffer = safe_cast<const DynamicIndexBuffer *>(&indexBuffer);
		DEBUG_FATAL(dynamicIndexBuffer->getDynamicId() == 0, ("Dynamic IB never locked, cannot set"));
		DEBUG_FATAL(dynamicIndexBuffer->getDynamicId() != DynamicIndexBuffer::getDynamicGlobalId(), ("Setting a dynamic IB after another one has been locked"));
	}
#endif

	ms_api->setIndexBuffer(indexBuffer);
}

// ----------------------------------------------------------------------

void Graphics::drawPointList()
{
	predrawCheck();
	if (ms_shaderValidated)
		ms_api->drawPointList();
}

// ----------------------------------------------------------------------

void Graphics::drawLineList()
{
	predrawCheck();
	if (ms_shaderValidated)
		ms_api->drawLineList();
}

// ----------------------------------------------------------------------

void Graphics::drawLineStrip()
{
	predrawCheck();
	if (ms_shaderValidated)
		ms_api->drawLineStrip();
}

// ----------------------------------------------------------------------

void Graphics::drawTriangleList()
{
	predrawCheck();
	if (ms_shaderValidated)
		ms_api->drawTriangleList();
}

// ----------------------------------------------------------------------

void Graphics::drawTriangleStrip()
{
	predrawCheck();
	if (ms_shaderValidated)
		ms_api->drawTriangleStrip();
}

// ----------------------------------------------------------------------

void Graphics::drawTriangleFan()
{
	predrawCheck();
	if (ms_shaderValidated)
		ms_api->drawTriangleFan();
}

// ----------------------------------------------------------------------
/**
 * Draws a list of quads.
 *
 * This routine may be silently converted to a indexed triangle list, in
 * which case it will change the currently set index buffer.
 */

void Graphics::drawQuadList()
{
	predrawCheck();
	if (ms_shaderValidated)
		ms_api->drawQuadList();
}

// ----------------------------------------------------------------------

void Graphics::drawIndexedPointList()
{
	predrawCheck();
	if (ms_shaderValidated)
		ms_api->drawIndexedPointList();
}

// ----------------------------------------------------------------------

void Graphics::drawIndexedLineList()
{
	predrawCheck();
	if (ms_shaderValidated)
		ms_api->drawIndexedLineList();
}

// ----------------------------------------------------------------------

void Graphics::drawIndexedLineStrip()
{
	predrawCheck();
	if (ms_shaderValidated)
		ms_api->drawIndexedLineStrip();
}

// ----------------------------------------------------------------------

void Graphics::drawIndexedTriangleList()
{
	predrawCheck();
	if (ms_shaderValidated)
		ms_api->drawIndexedTriangleList();
}

// ----------------------------------------------------------------------

void Graphics::drawIndexedTriangleStrip()
{
	predrawCheck();
	if (ms_shaderValidated)
		ms_api->drawIndexedTriangleStrip();
}

// ----------------------------------------------------------------------

void Graphics::drawIndexedTriangleFan()
{
	predrawCheck();
	if (ms_shaderValidated)
		ms_api->drawIndexedTriangleFan();
}

// ----------------------------------------------------------------------
/**
 * Draw a point list.
 *
 * Draw the specified number of primitives starting from the specified vertex.
 *
 * @param startVertex     The first vertex to render with.
 * @param primitiveCount  The number of primitives to draw.
 */

void Graphics::drawPointList(int startVertex, int primitiveCount)
{
	predrawCheck();
	if (ms_shaderValidated)
		ms_api->drawPartialPointList(startVertex, primitiveCount);
}

// ----------------------------------------------------------------------
/**
 * Draw a line list.
 *
 * Draw the specified number of primitives starting from the specified vertex.
 *
 * @param startVertex     The first vertex to render with.
 * @param primitiveCount  The number of primitives to draw.
 */

void Graphics::drawLineList(int startVertex, int primitiveCount)
{
	predrawCheck();
	if (ms_shaderValidated)
		ms_api->drawPartialLineList(startVertex, primitiveCount);
}

// ----------------------------------------------------------------------
/**
 * Draw a line strip.
 *
 * Draw the specified number of primitives starting from the specified vertex.
 *
 * @param startVertex     The first vertex to render with.
 * @param primitiveCount  The number of primitives to draw.
 */

void Graphics::drawLineStrip(int startVertex, int primitiveCount)
{
	predrawCheck();
	if (ms_shaderValidated)
		ms_api->drawPartialLineStrip(startVertex, primitiveCount);
}

// ----------------------------------------------------------------------
/**
 * Draw a triangle list.
 *
 * Draw the specified number of primitives starting from the specified vertex.
 *
 * @param startVertex     The first vertex to render with.
 * @param primitiveCount  The number of primitives to draw.
 */

void Graphics::drawTriangleList(int startVertex, int primitiveCount)
{
	predrawCheck();
	if (ms_shaderValidated)
		ms_api->drawPartialTriangleList(startVertex, primitiveCount);
}

// ----------------------------------------------------------------------
/**
 * Draw a triangle strip.
 *
 * Draw the specified number of primitives starting from the specified vertex.
 *
 * @param startVertex     The first vertex to render with.
 * @param primitiveCount  The number of primitives to draw.
 */

void Graphics::drawTriangleStrip(int startVertex, int primitiveCount)
{
	predrawCheck();
	if (ms_shaderValidated)
		ms_api->drawPartialTriangleStrip(startVertex, primitiveCount);
}

// ----------------------------------------------------------------------
/**
 * Draw a triangle Fan.
 *
 * Draw the specified number of primitives starting from the specified vertex.
 *
 * @param startVertex     The first vertex to render with.
 * @param primitiveCount  The number of primitives to draw.
 */

void Graphics::drawTriangleFan(int startVertex, int primitiveCount)
{
	predrawCheck();
	if (ms_shaderValidated)
		ms_api->drawPartialTriangleFan(startVertex, primitiveCount);
}

// ----------------------------------------------------------------------
/**
 * Draw an indexed point list.
 *
 * baseIndex lets you slide the indices around within a VB without touching all the indices.  This
 * value is effectively added to all vertex indices before they are used.
 *
 * minimumVertexIndex and numberOfVertices specifies the span of vertices in the vertex
 * buffer that may be used while drawing.  The actual usable vertices from the vertex buffer
 * are from [baseIndex + minimumVertexIndex] to [baseIndex + minimumVertexIndex + numberOfVertices - 1] inclusive.
 *
 * @param baseIndex           The value to add to all indices before selecting the vertex.
 * @param minimumVertexIndex  The minimum index specified in the index list.
 * @param numberOfVertices    The range of vertices that may be referenced.
 * @param startIndex          The first index in the index buffer to render with.
 * @param primitiveCount      The number of primitives to draw.
 */

void Graphics::drawIndexedPointList(int baseIndex, int minimumVertexIndex, int numberOfVertices, int startIndex, int primitiveCount)
{
	predrawCheck();
	if (ms_shaderValidated)
		ms_api->drawPartialIndexedPointList(baseIndex, minimumVertexIndex, numberOfVertices, startIndex, primitiveCount);
}

// ----------------------------------------------------------------------
/**
 * Draw an indexed line list.
 *
 * baseIndex lets you slide the indices around within a VB without touching all the indices.  This
 * value is effectively added to all vertex indices before they are used.
 *
 * minimumVertexIndex and numberOfVertices specifies the span of vertices in the vertex
 * buffer that may be used while drawing.  The actual usable vertices from the vertex buffer
 * are from [baseIndex + minimumVertexIndex] to [baseIndex + minimumVertexIndex + numberOfVertices - 1] inclusive.
 *
 * @param baseIndex           The value to add to all indices before selecting the vertex.
 * @param minimumVertexIndex  The minimum index specified in the index list.
 * @param numberOfVertices    The range of vertices that may be referenced.
 * @param startIndex          The first index in the index buffer to render with.
 * @param primitiveCount      The number of primitives to draw.
 */

void Graphics::drawIndexedLineList(int baseIndex, int minimumVertexIndex, int numberOfVertices, int startIndex, int primitiveCount)
{
	predrawCheck();
	if (ms_shaderValidated)
		ms_api->drawPartialIndexedLineList(baseIndex, minimumVertexIndex, numberOfVertices, startIndex, primitiveCount);
}

// ----------------------------------------------------------------------
/**
 * Draw an indexed line strip.
 *
 * baseIndex lets you slide the indices around within a VB without touching all the indices.  This
 * value is effectively added to all vertex indices before they are used.
 *
 * minimumVertexIndex and numberOfVertices specifies the span of vertices in the vertex
 * buffer that may be used while drawing.  The actual usable vertices from the vertex buffer
 * are from [baseIndex + minimumVertexIndex] to [baseIndex + minimumVertexIndex + numberOfVertices - 1] inclusive.
 *
 * @param baseIndex           The value to add to all indices before selecting the vertex.
 * @param minimumVertexIndex  The minimum index specified in the index list.
 * @param numberOfVertices    The range of vertices that may be referenced.
 * @param startIndex          The first index in the index buffer to render with.
 * @param primitiveCount      The number of primitives to draw.
 */

void Graphics::drawIndexedLineStrip(int baseIndex, int minimumVertexIndex, int numberOfVertices, int startIndex, int primitiveCount)
{
	predrawCheck();
	if (ms_shaderValidated)
		ms_api->drawPartialIndexedLineStrip(baseIndex, minimumVertexIndex, numberOfVertices, startIndex, primitiveCount);
}

// ----------------------------------------------------------------------
/**
 * Draw an indexed triangle list.
 *
 * baseIndex lets you slide the indices around within a VB without touching all the indices.  This
 * value is effectively added to all vertex indices before they are used.
 *
 * minimumVertexIndex and numberOfVertices specifies the span of vertices in the vertex
 * buffer that may be used while drawing.  The actual usable vertices from the vertex buffer
 * are from [baseIndex + minimumVertexIndex] to [baseIndex + minimumVertexIndex + numberOfVertices - 1] inclusive.
 *
 * @param baseIndex           The value to add to all indices before selecting the vertex.
 * @param minimumVertexIndex  The minimum index specified in the index list.
 * @param numberOfVertices    The range of vertices that may be referenced.
 * @param startIndex          The first index in the index buffer to render with.
 * @param primitiveCount      The number of primitives to draw.
 */

void Graphics::drawIndexedTriangleList(int baseIndex, int minimumVertexIndex, int numberOfVertices, int startIndex, int primitiveCount)
{
	predrawCheck();
	if (ms_shaderValidated)
		ms_api->drawPartialIndexedTriangleList(baseIndex, minimumVertexIndex, numberOfVertices, startIndex, primitiveCount);
}

// ----------------------------------------------------------------------
/**
 * Draw an indexed triangle list.
 *
 * baseIndex lets you slide the indices around within a VB without touching all the indices.  This
 * value is effectively added to all vertex indices before they are used.
 *
 * minimumVertexIndex and numberOfVertices specifies the span of vertices in the vertex
 * buffer that may be used while drawing.  The actual usable vertices from the vertex buffer
 * are from [baseIndex + minimumVertexIndex] to [baseIndex + minimumVertexIndex + numberOfVertices - 1] inclusive.
 *
 * @param baseIndex           The value to add to all indices before selecting the vertex.
 * @param minimumVertexIndex  The minimum index specified in the index list.
 * @param numberOfVertices    The range of vertices that may be referenced.
 * @param startIndex          The first index in the index buffer to render with.
 * @param primitiveCount      The number of primitives to draw.
 */

void Graphics::drawIndexedTriangleStrip(int baseIndex, int minimumVertexIndex, int numberOfVertices, int startIndex, int primitiveCount)
{
	predrawCheck();
	if (ms_shaderValidated)
		ms_api->drawPartialIndexedTriangleStrip(baseIndex, minimumVertexIndex, numberOfVertices, startIndex, primitiveCount);
}

// ----------------------------------------------------------------------
/**
 * Draw an indexed triangle list.
 *
 * baseIndex lets you slide the indices around within a VB without touching all the indices.  This
 * value is effectively added to all vertex indices before they are used.
 *
 * minimumVertexIndex and numberOfVertices specifies the span of vertices in the vertex
 * buffer that may be used while drawing.  The actual usable vertices from the vertex buffer
 * are from [baseIndex + minimumVertexIndex] to [baseIndex + minimumVertexIndex + numberOfVertices - 1] inclusive.
 *
 * @param baseIndex           The value to add to all indices before selecting the vertex.
 * @param minimumVertexIndex  The minimum index specified in the index fan.
 * @param numberOfVertices    The range of vertices that may be referenced.
 * @param startIndex          The first index in the index buffer to render with.
 * @param primitiveCount      The number of primitives to draw.
 */

void Graphics::drawIndexedTriangleFan(int baseIndex, int minimumVertexIndex, int numberOfVertices, int startIndex, int primitiveCount)
{
	predrawCheck();
	if (ms_shaderValidated)
		ms_api->drawPartialIndexedTriangleFan(baseIndex, minimumVertexIndex, numberOfVertices, startIndex, primitiveCount);
}

// ----------------------------------------------------------------------
/**
 * Optimize an index buffer
 *
 * @param indices             The array of indexed triangles to optimize (in place)
 * @param numIndices          The number of indices in the array
*/
void Graphics::optimizeIndexBuffer(Index *indices, int numIndices)
{
	ms_api->optimizeIndexBuffer(indices, numIndices);
}

// ----------------------------------------------------------------------
/**
 * Return the maximum number of vertex buffer streams that may be set
 * at one time with Graphics::setVertexBuffer().
 *
 * @return  the maximum number of vertex buffers that can be set simultaneously with Graphics::setVertexBuffer()
 */

int Graphics::getMaximumVertexBufferStreamCount()
{
	return ms_api->getMaximumVertexBufferStreamCount();
}

// ----------------------------------------------------------------------

void Graphics::setBloomEnabled(bool enabled)
{
	ms_api->setBloomEnabled(enabled);
}

// ----------------------------------------------------------------------

void Graphics::pixSetMarker(WCHAR const * markerName)
{
#if PRODUCTION == 0
	ms_api->pixSetMarker(markerName);
#else
	UNREF(markerName);
#endif
}

// ----------------------------------------------------------------------

void Graphics::pixBeginEvent(WCHAR const * eventName)
{
#if PRODUCTION == 0
	ms_api->pixBeginEvent(eventName);
#else
	UNREF(eventName);
#endif
}

// ----------------------------------------------------------------------

void Graphics::pixEndEvent(WCHAR const * eventName)
{
#if PRODUCTION == 0
	ms_api->pixEndEvent(eventName);
#else
	UNREF(eventName);
#endif
}

// ----------------------------------------------------------------------

bool Graphics::writeImage(char const * file, int const width, int const height, int const pitch, int const * pixelsARGB, bool const alphaExtend, Gl_imageFormat const imageFormat, Rectangle2d const * subRect)
{
#if PRODUCTION == 0
	return ms_api->writeImage(file, width, height, pitch, pixelsARGB, alphaExtend, imageFormat, subRect);
#else
	UNREF(file);
	UNREF(width);
	UNREF(height);
	UNREF(pitch);
	UNREF(alphaExtend);
	UNREF(imageFormat);
	UNREF(pixelsARGB);
	UNREF(subRect);
	return true;
#endif
}

// ----------------------------------------------------------------------

bool Graphics::supportsAntialias()
{
	return ms_api->supportsAntialias();
}

// ----------------------------------------------------------------------

void Graphics::setAntialiasEnabled(bool antialiasEnable)
{
	ms_api->setAntialiasEnabled(antialiasEnable);
}

// ----------------------------------------------------------------------

#if PRODUCTION == 0
bool Graphics::createVideoBuffers(int width, int height)
{
	return ms_api->createVideoBuffers(width, height);
}
#endif // PRODUCTION

// ----------------------------------------------------------------------

#if PRODUCTION == 0
void Graphics::fillVideoBuffers()
{
	ms_api->fillVideoBuffers();
}
#endif // PRODUCTION

// ----------------------------------------------------------------------

#if PRODUCTION == 0
bool Graphics::getVideoBufferData(void *buffer, size_t bufferSize)
{
	return ms_api->getVideoBufferData(buffer, bufferSize);
}
#endif // PRODUCTION

// ----------------------------------------------------------------------

#if PRODUCTION == 0
void Graphics::releaseVideoBuffers()
{
	ms_api->releaseVideoBuffers();
}
#endif // PRODUCTION

// ----------------------------------------------------------------------
