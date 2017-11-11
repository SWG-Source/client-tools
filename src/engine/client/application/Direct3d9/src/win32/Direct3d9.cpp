// ======================================================================
//
// Direct3d9.cpp
// Copyright 1998 Bootprint Entertainment
// Copyright 2000-2001 Sony Online Entertainment
// All Rights Reserved.
//
// @todo make verify work for VSPS version
//
// ======================================================================

#include "FirstDirect3d9.h"
#include "Direct3d9.h"

#include "ConfigDirect3d9.h"
#include "Direct3d9_DynamicIndexBufferData.h"
#include "Direct3d9_DynamicVertexBufferData.h"
#include "Direct3d9_LightManager.h"
#include "Direct3d9_Metrics.h"
#include "Direct3d9_PixelShaderConstantRegisters.h"
#include "Direct3d9_PixelShaderProgramData.h"
#include "Direct3d9_RenderTarget.h"
#include "Direct3d9_ShaderImplementationData.h"
#include "Direct3d9_StateCache.h"
#include "Direct3d9_StaticIndexBufferData.h"
#include "Direct3d9_StaticShaderData.h"
#include "Direct3d9_StaticVertexBufferData.h"
#include "Direct3d9_TextureData.h"
#include "Direct3d9_VertexBufferVectorData.h"
#include "Direct3d9_VertexDeclarationMap.h"
#include "Direct3d9_VertexShaderConstantRegisters.h"
#include "Direct3d9_VertexShaderData.h"
#include "PaddedVector.h"
#include "SetupDll.h"
#include "WriteTGA.h"

#include "clientGraphics/Camera.h"
#include "clientGraphics/ConfigClientGraphics.h"
#include "clientGraphics/Gl_dll.def"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/Material.h"
#include "clientGraphics/ShaderCapability.h"
#include "clientGraphics/VertexBuffer.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/DebugKey.h"
#include "sharedDebug/PerformanceTimer.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/ConfigSharedFoundation.h"
#include "sharedFoundation/CrashReportInformation.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/Production.h"
#include "sharedMath/Rectangle2d.h"
#include "sharedMath/Transform.h"
#include "sharedMath/Vector.h"
#include "sharedMath/VectorRgba.h"

#include <ddraw.h>
#include <d3dx9.h>
#include <stdio.h>

#pragma warning (disable: 4201)
#include <mmsystem.h>
#pragma warning (default: 4201)

extern "C"
{
#include "jpeglib.h"


#if _MSC_VER < 1300
// from WINUSER.H
typedef struct tagMONITORINFO
{
	DWORD   cbSize;
	RECT    rcMonitor;
	RECT    rcWork;
	DWORD   dwFlags;
} MONITORINFO, *LPMONITORINFO;
#endif

WINUSERAPI BOOL WINAPI GetMonitorInfoA(HMONITOR hMonitor, LPMONITORINFO lpmi);
WINUSERAPI BOOL WINAPI GetMonitorInfoW(HMONITOR hMonitor, LPMONITORINFO lpmi);
#ifdef UNICODE
#define GetMonitorInfo  GetMonitorInfoW
#else
#define GetMonitorInfo  GetMonitorInfoA
#endif // !UNICODE

}

// ======================================================================

#if !defined(FFP) && !defined(VSPS)
#error must define FFP, VSPS, or both
#endif

// ======================================================================

namespace Direct3d9Namespace
{
	// ----------------------------------------------------------------------
	// types

	typedef void (*CallbackFunction)();
	typedef std::vector<CallbackFunction> CallbackFunctions;

	// ----------------------------------------------------------------------
	// functions

	bool                               verify();
	void                               remove();
	void                               displayModeChanged();
	bool                               checkDisplayMode();
	void                               lostDevice();
	void                               restoreDevice();
	bool                               isGdiVisible();
	bool                               wasDeviceReset();

	void                               flushResources(bool fullReset);

	bool                               requiresVertexAndPixelShaders();

	void                               updateWindowSettings();

	bool                               supportsScissorRect();
	bool                               supportsHardwareMouseCursor();

#ifdef _DEBUG
	bool                               getShowMipmapLevels();
	void                               clearStaticShader();
	void                               getRenderedVerticesPointsLinesTrianglesCalls(int &vertices, int &points, int &lines, int &triangles, int &calls);
#endif

	char const *                       getFormatName(D3DFORMAT format);
	D3DFORMAT                          convertToAdapterFormat(D3DFORMAT backBufferFormat);
	D3DFORMAT const *                  getMatchingColorAlphaFormats(int color, int alpha);
	D3DFORMAT const *                  getMatchingDepthStencilFormats(int z, int stencil);
	int                                getDepthBufferBitDepth(D3DFORMAT depthStencilFormat);
	int                                getStencilBufferBitDepth(D3DFORMAT depthStencilFormat);

	bool                               supportsMipmappedCubeMaps();
	int                                getMaximumVertexBufferStreamCount();
	void                               getOtherAdapterRects(std::vector<RECT> &otherAdapterRects);

#ifdef _DEBUG
	void                               showMipmapLevels(bool enabled);
	bool                               getShowMipmapLevels();
	void                               setBadVertexBufferVertexShaderCombination(bool *flag, const char *appearanceName);
#endif
	void                               setBadVertexShaderStaticShader(const StaticShader *shader);

	void                               resize(int newWidth, int newHeight);
	void                               setWindowedMode(bool windowed, bool force);
	void                               setWindowedMode(bool windowed);
	void                               setWindow(HWND window, int width, int height);
	void                               removeWindow(HWND window);
	void                               setPresentParameters();

	void                               setBrightnessContrastGamma(float brightness, float contrast, float gamma);

	void                               setFillMode(GlFillMode newFillMode);
	void                               setCullMode(GlCullMode newCullMode);

	void                               setPointSize(float size);
	void                               setPointSizeMin( float min );
	void                               setPointSizeMax( float max );
	void                               setPointScaleEnable( bool bEnable );
	void                               setPointScaleFactor( float A, float B, float C);
	void                               setPointSpriteEnable( bool bEnable );

	void                               clearViewport(bool clearColor, uint32 colorValue, bool clearDepth, float depthValue, bool clearStencil, uint32 stencilValue);

	bool                               screenShot(GlScreenShotFormat screenShotFormat, int quality, const char *fileName);

	void                               update(float elapsedTime);
	void                               beginScene();
	void                               endScene();
	bool                               getBackBuffer();
	bool                               lockBackBuffer(Gl_pixelRect &o_pixels, const RECT *i_lockRect);
	bool                               unlockBackBuffer();
	void                               releaseBackBuffer();
	bool                               present(bool windowed, HWND window, int width, int height);
	bool                               present();
	bool                               present(HWND window, int width, int height);
	void                               setRenderTarget(Texture *texture, CubeFace cubeFace, int mipmapLevel);
	bool                               copyRenderTargetToNonRenderTargetTexture();

	bool                               setMouseCursor(Texture const & mouseCursorTexture, int hotSpotX, int hotSpotY);
	bool                               showMouseCursor(bool cursorVisible);

	void                               setViewport(int x0, int y0, int x1, int y1, float minZ, float maxZ);
	void                               setScissorRect(bool enabled, int x, int y, int width, int height);
	void                               setWorldToCameraTransform(const Transform &transform, const Vector &cameraPosition);
	void                               setProjectionMatrix(const GlMatrix4x4 &projectionMatrix);
	void                               setFog(bool enabled, float density, const PackedArgb &color);
	void                               setObjectToWorldTransformAndScale(const Transform &transform, const Vector &scale);
	void                               setGlobalTexture(Tag tag, const Texture &texture);
	void                               releaseAllGlobalTextures();
	void                               setTextureTransform(int stage, bool enabled, int dimension, bool projected, const float *transform);
	void                               setVertexShaderUserConstants(int index, float c0, float c1, float c2, float c3);
	void                               setPixelShaderUserConstants(VectorRgba const * constants, int count);

	ShaderImplementationGraphicsData * createShaderImplementationGraphicsData(const ShaderImplementation &shaderImplementation);
	StaticShaderGraphicsData *         createStaticShaderGraphicsData(const StaticShader &shader);

	ShaderImplementationPassVertexShaderGraphicsData       * createVertexShaderData(ShaderImplementationPassVertexShader const &vertexShader);
	ShaderImplementationPassPixelShaderProgramGraphicsData * createPixelShaderProgramData(ShaderImplementationPassPixelShaderProgram const &pixelShaderProgram);

	void                               setAlphaFadeOpacity(bool enabled, float opacity);
	void                               noSetAlphaFadeOpacity(bool enabled, float opacity);

//	VectorRgba const &                 getAlphaFadeAndBloomSettings();

	void                               setLights(const stdvector<const Light*>::fwd &lightList);

	StaticVertexBufferGraphicsData *   createVertexBufferData(const StaticVertexBuffer &vertexBuffer);
	DynamicVertexBufferGraphicsData *  createVertexBufferData(const DynamicVertexBuffer &vertexBuffer);
	VertexBufferVectorGraphicsData *   createVertexBufferVectorData(VertexBufferVector const & vertexBufferVector);

	StaticIndexBufferGraphicsData *    createIndexBufferData(const StaticIndexBuffer &indexBuffer);
	DynamicIndexBufferGraphicsData *   createIndexBufferData();
	void                               setDynamicIndexBufferSize(int numberOfIndices);

	void                               getOneToOneUVMapping(int textureWidth, int textureHeight, float &u0, float &v0, float &u1, float &v1);
	TextureGraphicsData *              createTextureData(const Texture &texture, const TextureFormat *runtimeFormats, int numberOfRuntimeFormats);

#ifdef _DEBUG
	bool                               shouldDraw(bool indexed, int primitiveCount);
#endif

	void                               drawPointList();
	void                               drawLineList();
	void                               drawLineStrip();
	void                               drawTriangleList();
	void                               drawTriangleStrip();
	void                               drawTriangleFan();
	void                               drawQuadList();

	void                               drawIndexedPointList();
	void                               drawIndexedLineList();
	void                               drawIndexedLineStrip();
	void                               drawIndexedTriangleList();
	void                               drawIndexedTriangleStrip();
	void                               drawIndexedTriangleFan();

	void                               drawPointList(int startVertex, int primitiveCount);
	void                               drawLineList(int startVertex, int primitiveCount);
	void                               drawLineStrip(int startVertex, int primitiveCount);
	void                               drawTriangleList(int startVertex, int primitiveCount);
	void                               drawTriangleStrip(int startVertex, int primitiveCount);
	void                               drawTriangleFan(int startVertex, int primitiveCount);

	void                               drawIndexedPointList(int baseInex, int minimumVertexIndex, int numberOfVertices, int startIndex, int primitiveCount);
	void                               drawIndexedLineList(int baseIndex, int minimumVertexIndex, int numberOfVertices, int startIndex, int primitiveCount);
	void                               drawIndexedLineStrip(int baseIndex, int minimumVertexIndex, int numberOfVertices, int startIndex, int primitiveCount);
	void                               drawIndexedTriangleList(int baseIndex, int minimumVertexIndex, int numberOfVertices, int startIndex, int primitiveCount);
	void                               drawIndexedTriangleStrip(int baseIndex, int minimumVertexIndex, int numberOfVertices, int startIndex, int primitiveCount);
	void                               drawIndexedTriangleFan(int baseIndex, int minimumVertexIndex, int numberOfVertices, int startIndex, int primitiveCount);

	void                               resizeQuadListIndexBuffer(int numberOfQuads);

	void                               addDeviceLostCallback(CallbackFunction callbackFunction);
	void                               removeDeviceLostCallback(CallbackFunction callbackFunction);
	void                               addDeviceRestoredCallback(CallbackFunction callbackFunction);
	void                               removeDeviceRestoredCallback(CallbackFunction callbackFunction);

	void                               optimizeIndexBuffer(WORD *indices, int numIndices);

	void                               setBloomEnabled(bool enabled);

	void                               pixSetMarker(WCHAR const * markerName);
	void                               pixBeginEvent(WCHAR const * eventName);
	void                               pixEndEvent(WCHAR const * eventName);
	bool                               writeImage(char const * file, int const width, int const height, int const pitch, int const * pixelsARGB, bool const alphaExtend, Gl_imageFormat const imageFormat, Rectangle2d const * subRect);

	void                               _queryVideoMemory();

	// video capture routines

#if PRODUCTION == 0
	bool                               createVideoBuffers(int width, int height);
	void                               fillVideoBuffers();
	bool                               getVideoBufferData(void * buffer, size_t bufferSize);
	void                               releaseVideoBuffers();
#endif  // PRODUCTION

	// ----------------------------------------------------------------------
	// variables

	bool                                       ms_installed;
	bool                                       ms_windowed;
	bool                                       ms_engineOwnsWindow;
	bool                                       ms_borderlessWindow;
	int                                        ms_windowX;
	int                                        ms_windowY;
	void                                       (*ms_windowedModeChanged)(bool windowed);
	CallbackFunctions                          ms_deviceLostCallbacks;
	CallbackFunctions                          ms_deviceRestoredCallbacks;
	bool                                       ms_displayModeChanged;
	bool                                       ms_deviceReset;
#ifdef _DEBUG
#if defined(FFP) && defined(VSPS)
	bool                                       ms_noRenderVertexShaders;
	bool                                       ms_noRenderFixedFunctionPipeline;
#endif
	bool                                       ms_showMipmapLevelsEnabled;
	bool                                       ms_disableAll;
	bool                                       ms_disablePass0Plus;
	bool                                       ms_disablePass1Plus;
	bool                                       ms_disablePass2Plus;
	bool                                       ms_disablePass3Plus;
	bool                                       ms_minimumDrawPrimitives2;
	bool                                       ms_minimumDrawPrimitives4;
	bool                                       ms_minimumDrawPrimitives8;
	bool                                       ms_minimumDrawPrimitives16;
	bool                                       ms_minimumDrawPrimitives32;
	bool                                       ms_minimumDrawPrimitives64;
	bool                                       ms_minimumDrawPrimitives128;
	bool                                       ms_minimumDrawPrimitives256;
	bool                                       ms_minimumDrawPrimitives512;
	bool                                       ms_noClearFrameBuffer;
	bool                                       ms_noClearDepthBuffer;
	bool                                       ms_noClearStencilBuffer;
	bool                                       ms_noScissor;
	bool                                       ms_noIndexedPrimitives;
	bool                                       ms_noNonIndexedPrimitives;
	bool                                       ms_disableAlphaFadeOpacity;
#endif
	int                                        ms_frameNumber;
	float                                      ms_currentTime;
	HWND                                       ms_window;

	Gl_api                                     ms_glApi;
#if defined(_DEBUG) && defined(VSPS)
	bool                                      *ms_badVertexBufferVertexShaderCombination;
	const char                                *ms_appearanceName;
#endif

	IDirect3D9                                *ms_direct3d;
	IDirect3DDevice9                          *ms_device;
	D3DPRESENT_PARAMETERS                      ms_presentParameters;
	D3DCAPS9                                   ms_deviceCaps;
	D3DDEVTYPE                                 ms_deviceType;
	D3DDISPLAYMODE                             ms_displayMode;
	D3DFORMAT                                  ms_adapterFormat;
	UINT                                       ms_adapter;
	D3DFORMAT                                  ms_backBufferFormat;
	D3DFORMAT                                  ms_depthStencilFormat;
	D3DADAPTER_IDENTIFIER9                     ms_adapterIdentifier;
	IDirect3DSurface9                         *ms_backBuffer;
	bool                                       ms_backBufferLocked;
	bool                                       ms_hasDepthBuffer;
	bool                                       ms_hasStencilBuffer;
	bool                                       ms_supportsStreamOffsets;
	bool                                       ms_supportsDynamicTextures;
	bool                                       ms_supportsMultiSample;
	DWORD                                      ms_multiSampleQualityLevels;

	bool                                       ms_alphaBlendEnable;
	uint8                                      ms_alphaTestReferenceValue;
	uint8                                      ms_colorWriteEnable;

#ifdef VSPS
	bool                                       ms_alphaFadeOpacityEnabled;
	bool                                       ms_alphaFadeOpacityDirty;
	VectorRgba                                 ms_alphaFadeOpacity;
#endif

#ifdef _DEBUG
	StaticShader const *                       ms_currentShader;
	int                                        ms_currentPassIndex;
	StaticVertexBuffer const *                 ms_currentStaticVertexBuffer;
#endif
	BYTE ms_colorCorrectionTable[256];
	bool applyGammaCorrectionToXRGBSurface( IDirect3DSurface9 *surface );

	int                                        ms_width;
	int                                        ms_height;

	int                                        ms_viewportX;
	int                                        ms_viewportY;
	int                                        ms_viewportWidth;
	int                                        ms_viewportHeight;
	float                                      ms_viewportMinimumZ;
	float                                      ms_viewportMaximumZ;

	int                                        ms_sliceNumberOfVertices;
	int                                        ms_sliceFirstVertex;

	IDirect3DIndexBuffer9 *                    ms_savedIndexBuffer;
	int                                        ms_sliceNumberOfIndices;
	int                                        ms_sliceFirstIndex;

	int                                        ms_lastVertexBufferCount;

	DWORD                                      ms_fogColor;
	DWORD                                      ms_fogModeValue;

	bool                                       ms_antialiasEnabled = false;

#ifdef FFP
	const D3DTEXTURETRANSFORMFLAGS             cms_textureTransformFlagsLookup[4] =
	{
		D3DTTFF_COUNT1,
		D3DTTFF_COUNT2,
		D3DTTFF_COUNT3,
		D3DTTFF_COUNT4
	};

	const D3DTRANSFORMSTATETYPE                cms_textureTransformLookup[8] =
	{
		D3DTS_TEXTURE0,
		D3DTS_TEXTURE1,
		D3DTS_TEXTURE2,
		D3DTS_TEXTURE3,
		D3DTS_TEXTURE4,
		D3DTS_TEXTURE5,
		D3DTS_TEXTURE6,
		D3DTS_TEXTURE7,
	};
#endif


	bool                                ms_transformDirty;
#if defined(FFP) && defined(VSPS)
	bool                                ms_usingVertexShader;
#endif

	D3DXMATRIX                          ms_cachedObjectToWorldMatrix;
	D3DXMATRIX                          ms_cachedWorldToCameraMatrix;
	D3DXMATRIX                          ms_cachedProjectionMatrix;
	D3DXMATRIX                          ms_cachedWorldToProjectionMatrix;

#if DEBUG_LEVEL == DEBUG_LEVEL_DEBUG
	int                                 ms_drawCall = 0;
	bool                                ms_allowDrawCallSelection;
	int                                 ms_allowDrawCallBegin = 0;
	int                                 ms_allowDrawCallEnd = 0x7fffffff;
	int                                *ms_allowDrawCallPointer = &ms_allowDrawCallBegin;
#endif

	const StaticShader                 *ms_badVertexShaderStaticShader;

	void                               *ms_temporaryBuffer;
	int                                 ms_temporaryBufferSize;

#ifdef _DEBUG

	struct WarningRecord
	{
		StaticShader const * m_staticShader;
		VertexBuffer const * m_vertexBuffer;

		WarningRecord(StaticShader const * staticShader, VertexBuffer const *vertexBuffer);
		bool operator < (WarningRecord const &rhs);
	};

	WarningRecord::WarningRecord(StaticShader const * staticShader, VertexBuffer const *vertexBuffer)
	:
		m_staticShader(staticShader),
		m_vertexBuffer(vertexBuffer)
	{
	}

	bool WarningRecord::operator < (WarningRecord const &rhs)
	{
		return (m_staticShader < rhs.m_staticShader || (m_staticShader == rhs.m_staticShader && m_vertexBuffer < rhs.m_vertexBuffer));
	}

#endif

	int   ms_shaderCapability;
	int   ms_videoMemoryInMegabytes = 32;

	int                ms_quadListIndexBufferNumberOfQuads;
	StaticIndexBuffer *ms_quadListIndexBuffer;

	const D3DCUBEMAP_FACES ms_cubeFaceLookup[] =
	{
		D3DCUBEMAP_FACE_POSITIVE_X,  // CF_positiveX
		D3DCUBEMAP_FACE_NEGATIVE_X,  // CF_negativeX
		D3DCUBEMAP_FACE_POSITIVE_Y,  // CF_positiveY
		D3DCUBEMAP_FACE_NEGATIVE_Y,  // CF_negativeY
		D3DCUBEMAP_FACE_POSITIVE_Z,  // CF_positiveZ
		D3DCUBEMAP_FACE_NEGATIVE_Z   // CF_negativeZ
	};

	PerformanceTimer *ms_performanceTimer;

	int nextPowerOfTwo(int x);

	// video capture variables
	IDirect3DSurface9 *ms_videoSurface;             // Surface to StretchRect the backbuffer to
	IDirect3DSurface9 *ms_videoOffScreenSurface;    // Surface to GetRenderData to
}
using namespace Direct3d9Namespace;

// ======================================================================

extern "C" __declspec(dllexport) Gl_api const * GetApi();

// ======================================================================

Gl_api const * GetApi()
{
	ms_glApi.verify  = verify;
	ms_glApi.install = Direct3d9::install;
	return &ms_glApi;
}

// ----------------------------------------------------------------------

bool Direct3d9Namespace::verify()
{
	return true;
}

// ----------------------------------------------------------------------

IDirect3D9 *Direct3d9::getDirect3d()
{
	return ms_direct3d;
}

// ----------------------------------------------------------------------

IDirect3DDevice9 *Direct3d9::getDevice()
{
	return ms_device;
}

// ----------------------------------------------------------------------

float Direct3d9::getCurrentTime()
{
	return ms_currentTime;
}

// ----------------------------------------------------------------------

int Direct3d9::getFrameNumber()
{
	return ms_frameNumber;
}

// ----------------------------------------------------------------------

D3DDEVTYPE Direct3d9::getDeviceType()
{
	return ms_deviceType;
}

// ----------------------------------------------------------------------

D3DFORMAT Direct3d9::getAdapterFormat()
{
	return ms_adapterFormat;
}

// ----------------------------------------------------------------------

int Direct3d9::getShaderCapability()
{
	return ms_shaderCapability;
}

// ----------------------------------------------------------------------

int Direct3d9::getVideoMemoryInMegabytes()
{
	return ms_videoMemoryInMegabytes;
}

// ----------------------------------------------------------------------

bool Direct3d9Namespace::requiresVertexAndPixelShaders()
{
#ifdef FFP
	return false;
#else
	return true;
#endif
}

// ----------------------------------------------------------------------

DWORD Direct3d9::getFogColor()
{
	return ms_fogColor;
}

// ----------------------------------------------------------------------

D3DFORMAT Direct3d9::getDepthStencilFormat()
{
	return ms_depthStencilFormat;
}

// ----------------------------------------------------------------------

D3DCUBEMAP_FACES Direct3d9::getD3dCubeFace(CubeFace const cubeFace)
{
	DEBUG_FATAL(cubeFace == CF_none, ("cannot look up CF_none"));
	return ms_cubeFaceLookup[cubeFace];
}

// ----------------------------------------------------------------------

const char *Direct3d9Namespace::getFormatName(D3DFORMAT format)
{
#define CASE(a) case a: return #a
	switch (format)
	{
		CASE(D3DFMT_UNKNOWN);

		CASE(D3DFMT_R8G8B8);
		CASE(D3DFMT_A8R8G8B8);
		CASE(D3DFMT_X8R8G8B8);
		CASE(D3DFMT_R5G6B5);
		CASE(D3DFMT_X1R5G5B5);
		CASE(D3DFMT_A1R5G5B5);
		CASE(D3DFMT_A4R4G4B4);
		CASE(D3DFMT_R3G3B2);
		CASE(D3DFMT_A8);
		CASE(D3DFMT_A8R3G3B2);
		CASE(D3DFMT_X4R4G4B4);
		CASE(D3DFMT_A2B10G10R10);
		CASE(D3DFMT_A8B8G8R8);
		CASE(D3DFMT_X8B8G8R8);
		CASE(D3DFMT_G16R16);
		CASE(D3DFMT_A2R10G10B10);
		CASE(D3DFMT_A16B16G16R16);

		CASE(D3DFMT_A8P8);
		CASE(D3DFMT_P8);

		CASE(D3DFMT_L8);
		CASE(D3DFMT_A8L8);
		CASE(D3DFMT_A4L4);

		CASE(D3DFMT_V8U8);
		CASE(D3DFMT_L6V5U5);
		CASE(D3DFMT_X8L8V8U8);
		CASE(D3DFMT_Q8W8V8U8);
		CASE(D3DFMT_V16U16);
		CASE(D3DFMT_A2W10V10U10);

		CASE(D3DFMT_UYVY);
		CASE(D3DFMT_R8G8_B8G8);
		CASE(D3DFMT_YUY2);
		CASE(D3DFMT_G8R8_G8B8);
		CASE(D3DFMT_DXT1);
		CASE(D3DFMT_DXT2);
		CASE(D3DFMT_DXT3);
		CASE(D3DFMT_DXT4);
		CASE(D3DFMT_DXT5);

		CASE(D3DFMT_D16_LOCKABLE);
		CASE(D3DFMT_D32);
		CASE(D3DFMT_D15S1);
		CASE(D3DFMT_D24S8);
		CASE(D3DFMT_D24X8);
		CASE(D3DFMT_D24X4S4);
		CASE(D3DFMT_D16);

		CASE(D3DFMT_D32F_LOCKABLE);
		CASE(D3DFMT_D24FS8);

		CASE(D3DFMT_L16);

		CASE(D3DFMT_VERTEXDATA);
		CASE(D3DFMT_INDEX16);
		CASE(D3DFMT_INDEX32);

		CASE(D3DFMT_Q16W16V16U16);

		CASE(D3DFMT_MULTI2_ARGB8);

		CASE(D3DFMT_R16F);
		CASE(D3DFMT_G16R16F);
		CASE(D3DFMT_A16B16G16R16F);

		CASE(D3DFMT_R32F);
		CASE(D3DFMT_G32R32F);
		CASE(D3DFMT_A32B32G32R32F);

		CASE(D3DFMT_CxV8U8);
	}

#undef CASE

	return "bad format";
}

// ----------------------------------------------------------------------

D3DFORMAT Direct3d9Namespace::convertToAdapterFormat(D3DFORMAT backBufferFormat)
{
	switch (backBufferFormat)
	{
		case D3DFMT_R8G8B8:    return D3DFMT_R8G8B8;
		case D3DFMT_A8R8G8B8:  return D3DFMT_X8R8G8B8;
		case D3DFMT_X8R8G8B8:  return D3DFMT_X8R8G8B8;
		case D3DFMT_R5G6B5:    return D3DFMT_R5G6B5;
		case D3DFMT_X1R5G5B5:  return D3DFMT_X1R5G5B5;
		case D3DFMT_A1R5G5B5:  return D3DFMT_X1R5G5B5;
	}

	DEBUG_FATAL(true, ("Unknown back buffer format"));
	return D3DFMT_UNKNOWN;
}

// ----------------------------------------------------------------------

const D3DFORMAT *Direct3d9Namespace::getMatchingColorAlphaFormats(int color, int alpha)
{
	switch (color)
	{
		case -1:
			switch (alpha)
			{
				case -1:
					{
						static const D3DFORMAT bbf[] = { D3DFMT_A8R8G8B8, D3DFMT_R8G8B8, D3DFMT_X8R8G8B8, D3DFMT_A1R5G5B5, D3DFMT_R5G6B5, D3DFMT_X1R5G5B5, D3DFMT_UNKNOWN };
						return bbf;
					}
				case  0:
					{
						static const D3DFORMAT bbf[] = { D3DFMT_R8G8B8, D3DFMT_X8R8G8B8, D3DFMT_R5G6B5, D3DFMT_X1R5G5B5, D3DFMT_UNKNOWN };
						return bbf;
					}
				case  1:
					{
						static const D3DFORMAT bbf[] = { D3DFMT_A1R5G5B5, D3DFMT_UNKNOWN };
						return bbf;
					}
				case  8:
					{
						static const D3DFORMAT bbf[] = { D3DFMT_A8R8G8B8, D3DFMT_UNKNOWN };
						return bbf;
					}
			}
			break;

		case 15:
			switch (alpha)
			{
				case -1:
					{
						static const D3DFORMAT bbf[] = { D3DFMT_A1R5G5B5, D3DFMT_X1R5G5B5, D3DFMT_UNKNOWN };
						return bbf;
					}
				case  0:
					{
						static const D3DFORMAT bbf[] = { D3DFMT_X1R5G5B5, D3DFMT_UNKNOWN };
						return bbf;
					}
				case  1:
					{
						static const D3DFORMAT bbf[] = { D3DFMT_A1R5G5B5, D3DFMT_UNKNOWN };
						return bbf;
					}
			}
			break;

		case 16:
			switch (alpha)
			{
				case -1:
				case  0:
				{
					static const D3DFORMAT bbf[] = { D3DFMT_R5G6B5, D3DFMT_UNKNOWN };
					return bbf;
				}
			}
			break;

		case 24:
			switch (alpha)
			{
				case -1:
					{
						static const D3DFORMAT bbf[] = { D3DFMT_A8R8G8B8, D3DFMT_X8R8G8B8, D3DFMT_R8G8B8, D3DFMT_UNKNOWN };
						return bbf;
					}
				case 0:
					{
						static const D3DFORMAT bbf[] = { D3DFMT_X8R8G8B8, D3DFMT_R8G8B8, D3DFMT_UNKNOWN };
						return bbf;
					}
				case 8:
					{
						static const D3DFORMAT bbf[] = { D3DFMT_A8R8G8B8, D3DFMT_UNKNOWN };
						return bbf;
					}
			}
			break;
	}

	return NULL;
}

// ----------------------------------------------------------------------

int Direct3d9Namespace::getDepthBufferBitDepth(D3DFORMAT depthStencilFormat)
{
	switch (depthStencilFormat)
	{
		case D3DFMT_D24S8:    return 24;
		case D3DFMT_D24X4S4:  return 24;
		case D3DFMT_D32:      return 32;
		case D3DFMT_D24X8:    return 24;
		case D3DFMT_D16:      return 16;
		case D3DFMT_D15S1:    return 15;
		default:
			DEBUG_FATAL(true, ("Not a depth/stencil format"));
	}

	return 0;
}

// ----------------------------------------------------------------------

int Direct3d9Namespace::getStencilBufferBitDepth(D3DFORMAT depthStencilFormat)
{
	switch (depthStencilFormat)
	{
		case D3DFMT_D24S8:    return 8;
		case D3DFMT_D24X4S4:  return 4;
		case D3DFMT_D32:      return 0;
		case D3DFMT_D24X8:    return 0;
		case D3DFMT_D16:      return 0;
		case D3DFMT_D15S1:    return 1;
		default:
			DEBUG_FATAL(true, ("Not a depth/stencil format"));
	}

	return 0;
}

// ----------------------------------------------------------------------

const D3DFORMAT *Direct3d9Namespace::getMatchingDepthStencilFormats(int z, int stencil)
{
	switch (z)
	{
		case -1:
			switch (stencil)
			{
				case -1:
				{
					static const D3DFORMAT dsf[] = { D3DFMT_D24S8, D3DFMT_D24X4S4, D3DFMT_D32, D3DFMT_D24X8, D3DFMT_D16, D3DFMT_D15S1, D3DFMT_UNKNOWN };
					return dsf;
				}

				case  0:
				{
					static const D3DFORMAT dsf[] = {  D3DFMT_D32, D3DFMT_D24X8, D3DFMT_D16, D3DFMT_UNKNOWN };
					return dsf;
				}

				case  1:
				{
					static const D3DFORMAT dsf[] = { D3DFMT_D15S1, D3DFMT_UNKNOWN };
					return dsf;
				}

				case  4:
				{
					static const D3DFORMAT dsf[] = { D3DFMT_D24X4S4, D3DFMT_UNKNOWN };
					return dsf;
				}

				case  8:
				{
					static const D3DFORMAT dsf[] = { D3DFMT_D24S8, D3DFMT_UNKNOWN };
					return dsf;
				}
			}
			break;

		case 15:
			switch (stencil)
			{
				case -1:
				case  1:
					{
						static const D3DFORMAT dsf[] = { D3DFMT_D15S1, D3DFMT_UNKNOWN };
						return dsf;
					}
			}
			break;

		case 16:
			switch (stencil)
			{
				case -1:
				case  0:
					{
						static const D3DFORMAT dsf[] = { D3DFMT_D16, D3DFMT_UNKNOWN };
						return dsf;
					}
			}
			break;

		case 24:
			switch (stencil)
			{
				case -1:
					{
						static const D3DFORMAT dsf[] = { D3DFMT_D24S8, D3DFMT_D24X4S4, D3DFMT_D24X8, D3DFMT_UNKNOWN };
						return dsf;
					}
				case  0:
					{
						static const D3DFORMAT dsf[] = { D3DFMT_D24X8, D3DFMT_UNKNOWN };
						return dsf;
					}
				case 4:
					{
						static D3DFORMAT dsf[] = { D3DFMT_D24X4S4, D3DFMT_UNKNOWN };
						return dsf;
					}
				case 8:
					{
						static const D3DFORMAT dsf[] = { D3DFMT_D24S8, D3DFMT_UNKNOWN };
						return dsf;
					}
					break;
			}
			break;

		case 32:
			switch (stencil)
			{
				case -1:
				case 0:
				{
					static const D3DFORMAT dsf[] = { D3DFMT_D32, D3DFMT_UNKNOWN };
					return dsf;
				}
			}
			break;
	}

	return NULL;
}

// ----------------------------------------------------------------------

bool Direct3d9::install(Gl_install *gl_install)
{
	DEBUG_FATAL(sizeof(PaddedVector) != sizeof(float) * 4, ("PaddedVector size bad"));
	NOT_NULL(gl_install);
	DEBUG_FATAL(ms_installed, ("already installed"));

	ConfigDirect3d9::install();

	ms_performanceTimer = new PerformanceTimer();

	ms_installed = true;
	ms_currentTime = 0.0f;
#ifdef VSPS
	ms_alphaFadeOpacityEnabled = false;
	ms_alphaFadeOpacityDirty = true;
	ms_alphaFadeOpacity.r = 0.0f;
	ms_alphaFadeOpacity.g = 0.0f;
	ms_alphaFadeOpacity.b = 0.0f;
	ms_alphaFadeOpacity.a = 0.0f;
#endif

	// store the screen dimensions
	ms_width               = gl_install->width;
	ms_height              = gl_install->height;
	ms_windowed            = gl_install->windowed;
	ms_window              = gl_install->window;
	ms_engineOwnsWindow    = gl_install->engineOwnsWindow;
	ms_borderlessWindow    = gl_install->borderlessWindow;
	ms_windowX             = gl_install->windowX;
	ms_windowY             = gl_install->windowY;
	ms_windowedModeChanged = gl_install->windowedModeChanged;

	// setup the api calls
	ms_glApi.remove                            = Direct3d9Namespace::remove;
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
	ms_glApi.setTexturesEnabled                = Direct3d9_StateCache::setTexturesEnabled;
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
	ms_glApi.presentToWindow                   = present;
	ms_glApi.setRenderTarget                   = setRenderTarget;
	ms_glApi.copyRenderTargetToNonRenderTargetTexture = copyRenderTargetToNonRenderTargetTexture;

	ms_glApi.screenShot                        = screenShot;

	ms_glApi.setBadVertexShaderStaticShader    = setBadVertexShaderStaticShader;
	ms_glApi.setStaticShader                   = setStaticShader;

	ms_glApi.createTextureData                 = createTextureData;
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

	ms_glApi.createShaderImplementationGraphicsData    = createShaderImplementationGraphicsData;
	ms_glApi.createStaticShaderGraphicsData            = createStaticShaderGraphicsData;
	ms_glApi.createVertexShaderData            = createVertexShaderData;
	ms_glApi.createPixelShaderProgramData      = createPixelShaderProgramData;

	ms_glApi.setAlphaFadeOpacity               = setAlphaFadeOpacity;

	ms_glApi.setLights                         = setLights;

	ms_glApi.createStaticVertexBufferData      = createVertexBufferData;
	ms_glApi.createDynamicVertexBufferData     = createVertexBufferData;
	ms_glApi.createVertexBufferVectorData      = createVertexBufferVectorData;
	ms_glApi.createStaticIndexBufferData       = createIndexBufferData;
	ms_glApi.createDynamicIndexBufferData      = createIndexBufferData;

	ms_glApi.getOneToOneUVMapping              = getOneToOneUVMapping;

	ms_glApi.setVertexBuffer                   = setVertexBuffer;
	ms_glApi.setVertexBufferVector             = setVertexBufferVector;
	ms_glApi.setIndexBuffer                    = setIndexBuffer;

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

	ms_glApi.drawPartialPointList              = drawPointList;
	ms_glApi.drawPartialLineList               = drawLineList;
	ms_glApi.drawPartialLineStrip              = drawLineStrip;
	ms_glApi.drawPartialTriangleList           = drawTriangleList;
	ms_glApi.drawPartialTriangleStrip          = drawTriangleStrip;
	ms_glApi.drawPartialTriangleFan            = drawTriangleFan;

	ms_glApi.drawPartialIndexedPointList       = drawIndexedPointList;
	ms_glApi.drawPartialIndexedLineList        = drawIndexedLineList;
	ms_glApi.drawPartialIndexedLineStrip       = drawIndexedLineStrip;
	ms_glApi.drawPartialIndexedTriangleList    = drawIndexedTriangleList;
	ms_glApi.drawPartialIndexedTriangleStrip   = drawIndexedTriangleStrip;
	ms_glApi.drawPartialIndexedTriangleFan     = drawIndexedTriangleFan;

	ms_glApi.getMaximumVertexBufferStreamCount = getMaximumVertexBufferStreamCount;

	ms_glApi.optimizeIndexBuffer			   = optimizeIndexBuffer;

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
#endif  // PRODUCTION

	// -----------------------------------
	// create Direct3d
	ms_direct3d = Direct3DCreate9(D3D_SDK_VERSION);
	FATAL(!ms_direct3d, ("Could not create direct3d"));
	// -----------------------------------

	const bool verboseHardwareLogging = ConfigSharedFoundation::getVerboseHardwareLogging();

	// ---------------------------------------------------------------------------------
	// Query for adapter video memory.
	_queryVideoMemory();

	REPORT_LOG(verboseHardwareLogging, ("Video memory = %i\n", ms_videoMemoryInMegabytes));
	CrashReportInformation::addStaticText("VideoMemory: %i\n", ms_videoMemoryInMegabytes);

#if PRODUCTION
	if (ms_videoMemoryInMegabytes < 32 && (ms_width > 1024 || ms_height > 768))
	{
		ms_width = gl_install->width = 1024;
		ms_height = gl_install->height = 768;
		REPORT_LOG(verboseHardwareLogging, ("Restricting screen resolution to %i x %i\n", ms_width, ms_height));
	}

	if (ms_videoMemoryInMegabytes < 64 && (ms_width > 1280 || ms_height > 1024))
	{
		ms_width = gl_install->width = 1280;
		ms_height = gl_install->height = 1024;
		REPORT_LOG(verboseHardwareLogging, ("Restricting screen resolution to %i x %i\n", ms_width, ms_height));
	}
#endif

	CrashReportInformation::addStaticText("GameResolution: %d %d\n", ms_width, ms_height);
	// ---------------------------------------------------------------------------------

	HRESULT hresult;

	// ---------------------------------------------------------------------------------
	// figure out which adapter to use
	{
		const int configAdapter = ConfigDirect3d9::getAdapter();
		ms_adapter = configAdapter < 0 ? D3DADAPTER_DEFAULT : configAdapter;
		int numberOfAdapters = ms_direct3d->GetAdapterCount();
		REPORT_LOG(verboseHardwareLogging, ("Using adapter %d of %d:\n", configAdapter, numberOfAdapters));
		CrashReportInformation::addStaticText("VideoAdapter: %d/%d\n", configAdapter, numberOfAdapters);

		// get the adapter identifier
		hresult = ms_direct3d->GetAdapterIdentifier(ms_adapter, 0, &ms_adapterIdentifier);
		FATAL_DX_HR("GetAdapterIdentifier failed %s", hresult);

		int const product    = HIWORD(ms_adapterIdentifier.DriverVersion.HighPart);
		int const version    = LOWORD(ms_adapterIdentifier.DriverVersion.HighPart);
		int const subVersion = HIWORD(ms_adapterIdentifier.DriverVersion.LowPart);
		int const build      = LOWORD(ms_adapterIdentifier.DriverVersion.LowPart);
		REPORT_LOG(verboseHardwareLogging, ("Adapter driver: %s\n", ms_adapterIdentifier.Driver));
		REPORT_LOG(verboseHardwareLogging, ("Adapter desc  : %s\n", ms_adapterIdentifier.Description));
		REPORT_LOG(verboseHardwareLogging, ("Adapter name  : %s\n", ms_adapterIdentifier.DeviceName));
		REPORT_LOG(verboseHardwareLogging, ("Adapter device: 0x%08x 0x%08x 0x%08x 0x%08x\n", ms_adapterIdentifier.VendorId, ms_adapterIdentifier.DeviceId, ms_adapterIdentifier.SubSysId, ms_adapterIdentifier.Revision));
		REPORT_LOG(verboseHardwareLogging, ("Adapter ver   : %d.%d.%04d.%04d\n", product, version, subVersion, build));
		CrashReportInformation::addStaticText("D3dDevice: 0x%04x 0x%04x\n", ms_adapterIdentifier.VendorId, ms_adapterIdentifier.DeviceId);
		CrashReportInformation::addStaticText("D3dDriver: %d.%d.%04d.%04d\n", product, version, subVersion, build);
	}
	// ---------------------------------------------------------------------------------

	// ---------------------------------------------------------------------------------
	// enumerate all the available display modes
	{
		D3DFORMAT enumerateFormats[] = { D3DFMT_A8R8G8B8, D3DFMT_X8R8G8B8,  D3DFMT_A2B10G10R10 };
		for (int f = 0; f < sizeof(enumerateFormats) / sizeof(enumerateFormats[0]); ++f)
		{
			D3DFORMAT format = enumerateFormats[f];
			int numberOfDisplayModes = ms_direct3d->GetAdapterModeCount(ms_adapter, format);
			DEBUG_FATAL(numberOfDisplayModes < 0, ("GetAdapterModeCount returned %d", numberOfDisplayModes));
			REPORT_LOG(verboseHardwareLogging, ("Available display modes:\n"));
			{
				for (int i = 0; i < numberOfDisplayModes; ++i)
				{
					hresult = ms_direct3d->EnumAdapterModes(ms_adapter, format, i, &ms_displayMode);
					FATAL_DX_HR("EnumAdapterModes failed %s", hresult);
					REPORT_LOG(verboseHardwareLogging, ("  %d=width %d=height %d=refresh %d=format\n", ms_displayMode.Width, ms_displayMode.Height, ms_displayMode.RefreshRate, static_cast<int>(ms_displayMode.Format)));
				}
			}
		}
	}
	// ---------------------------------------------------------------------------------

	// ---------------------------------------------------------------------------------
	// get the current display mode
	Zero(ms_displayMode);
	hresult = ms_direct3d->GetAdapterDisplayMode(ms_adapter, &ms_displayMode);
	FATAL_DX_HR("GetAdapterDisplayMode failed %s", hresult);
	ms_adapterFormat = ms_displayMode.Format;
	// ---------------------------------------------------------------------------------

	if (!checkDisplayMode())
	{
		ms_windowed = gl_install->windowed = false;
	}

	// ---------------------------------------------------------------------------------
#if PRODUCTION == 0
	if (ConfigDirect3d9::getUseReferenceRasterizer())
		ms_deviceType = D3DDEVTYPE_REF;
	else
#endif
		ms_deviceType = D3DDEVTYPE_HAL;
	// ---------------------------------------------------------------------------------

	const bool canLockBackBuffer    = !ConfigDirect3d9::getDoNotLockBackBuffer() && !ConfigDirect3d9::getAntiAlias();

	//const bool screenShotBackBuffer =  ConfigDirect3d9::getScreenShotBackBuffer();

	const DWORD lockableBackBuffer = canLockBackBuffer ? D3DPRESENTFLAG_LOCKABLE_BACKBUFFER : 0;

	// ---------------------------------------------------------------------------------
	// figure out color/alpha/depth/stencil buffer formats
	const D3DFORMAT *depthStencilFormats = getMatchingDepthStencilFormats(gl_install->zBufferBitDepth, gl_install->stencilBufferBitDepth);
	FATAL(!depthStencilFormats, ("invalid depth/stencil format specified"));
	const D3DFORMAT *backBufferFormats = getMatchingColorAlphaFormats(gl_install->colorBufferBitDepth, gl_install->alphaBufferBitDepth);
	FATAL(!backBufferFormats, ("invalid color/alpha format specified"));
	// ---------------------------------------------------------------------------------

	bool created = false;
	for (int h = 0; h < 2; ++h)
	{
		// figure out whether to create with HW T&L or SW T&L
		DWORD vertexProcessingMode = 0;
		if (ConfigDirect3d9::getVertexProcessingMode() == ConfigDirect3d9::VPM_default)
		{
			// try hardware first, then software
			if (h == 0)
				vertexProcessingMode = D3DCREATE_HARDWARE_VERTEXPROCESSING;
			else
				vertexProcessingMode = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
		}
		else
		{
			// no sense in doing the same thing twice
			if (h == 1)
				break;

			// choose hardware/software based on the switch
			if (ConfigDirect3d9::getVertexProcessingMode() == ConfigDirect3d9::VPM_hardware)
				vertexProcessingMode = D3DCREATE_HARDWARE_VERTEXPROCESSING;
			else
				if (ConfigDirect3d9::getVertexProcessingMode() == ConfigDirect3d9::VPM_software)
					vertexProcessingMode = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
				else
					FATAL(true, ("Invalid vertex processing mode"));
		}
		char const * const vertexProcessingModeText = (vertexProcessingMode == D3DCREATE_HARDWARE_VERTEXPROCESSING) ? "hardware" : "software";

		for (int i = 0; !created && backBufferFormats[i] != D3DFMT_UNKNOWN; ++i)
		{
			for (int j = 0; !created && depthStencilFormats[j] != D3DFMT_UNKNOWN; ++j)
			{
				if (!ms_windowed)
					ms_adapterFormat = convertToAdapterFormat(backBufferFormats[i]);
				ms_backBufferFormat = backBufferFormats[i];
				ms_depthStencilFormat = depthStencilFormats[j];

				// check adapter format, back buffer format, depth and stencil buffer compatibilities
				if (SUCCEEDED(ms_direct3d->CheckDeviceType(ms_adapter, ms_deviceType, ms_adapterFormat, ms_backBufferFormat, ms_windowed)) &&
						SUCCEEDED(ms_direct3d->CheckDeviceFormat(ms_adapter, ms_deviceType, ms_adapterFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, ms_depthStencilFormat)) &&
						SUCCEEDED(ms_direct3d->CheckDepthStencilMatch(ms_adapter, ms_deviceType, ms_adapterFormat, ms_backBufferFormat, ms_depthStencilFormat)))
				{
					hresult = ms_direct3d->GetDeviceCaps(ms_adapter, ms_deviceType, &ms_deviceCaps);
					FATAL_DX_HR("GetDeviceCaps failed %s", hresult);

					// try to create the d3d device
					Zero(ms_presentParameters);
					ms_presentParameters.BackBufferWidth        = ms_width;
					ms_presentParameters.BackBufferHeight       = ms_height;
					ms_presentParameters.BackBufferCount        = 1;
					ms_presentParameters.BackBufferFormat       = ms_backBufferFormat;
					ms_presentParameters.hDeviceWindow          = ms_window;
					ms_presentParameters.EnableAutoDepthStencil = TRUE;
					ms_presentParameters.AutoDepthStencilFormat = ms_depthStencilFormat;
					ms_presentParameters.Flags                  = lockableBackBuffer;

					/*  &&
					SUCCEEDED(ms_direct3d->CheckDeviceMultiSampleType( ms_adapter, 
					ms_deviceType , ms_depthStencilFormat, FALSE, 
					D3DMULTISAMPLE_NONMASKABLE, NULL ) )  &&
					SUCCEEDED(ms_direct3d->CheckDeviceMultiSampleType( ms_adapter, 
					ms_deviceType , ms_adapterFormat, FALSE, 
					D3DMULTISAMPLE_NONMASKABLE, NULL ) )*/

					// check for multisampling support
					DWORD qualityLevels;
					if( SUCCEEDED(ms_direct3d->CheckDeviceMultiSampleType( ms_adapter, 
						ms_deviceType , ms_backBufferFormat, FALSE, 
						D3DMULTISAMPLE_NONMASKABLE, &qualityLevels ) ))
					{
						ms_supportsMultiSample = true;
						ms_multiSampleQualityLevels = qualityLevels;
					}
					else
					{
						ms_supportsMultiSample = false;
					}

					setPresentParameters();

					// figure out what type of device to create
					if (vertexProcessingMode == D3DCREATE_HARDWARE_VERTEXPROCESSING && ConfigDirect3d9::getUsePureDevice())
						vertexProcessingMode |= D3DCREATE_PUREDEVICE;

					hresult = ms_direct3d->CreateDevice(ms_adapter, ms_deviceType, ms_window, vertexProcessingMode, &ms_presentParameters, &ms_device);

					if (SUCCEEDED(hresult))
					{
						IDirect3DSurface9 *depthStencilSurface = NULL;
						hresult = ms_device->GetDepthStencilSurface(&depthStencilSurface);
						FATAL_DX_HR("GetDepthStencilSurface failed %s", hresult);
						D3DSURFACE_DESC surfaceDesc;
						hresult = depthStencilSurface->GetDesc(&surfaceDesc);
						FATAL_DX_HR("GetDesc failed %s", hresult);
						depthStencilSurface->Release();

						if (surfaceDesc.Format == ms_depthStencilFormat)
						{
							created = true;
						}

						if (created && !gl_install->skipInitialClearViewport)
						{
							clearViewport(true, 0, false, 0, false, 0);
							present();
						}

						if (created)
						{
							REPORT_LOG(verboseHardwareLogging, ("Passed format %s %s %s %s\n", vertexProcessingModeText, getFormatName(ms_adapterFormat), getFormatName(ms_backBufferFormat), getFormatName(ms_depthStencilFormat)));
						}
						else
						{
							REPORT_LOG(verboseHardwareLogging, ("Failed wrong depth/stencil format %s %s %s %s\n", vertexProcessingModeText, getFormatName(ms_adapterFormat), getFormatName(ms_backBufferFormat), getFormatName(ms_depthStencilFormat)));
							ms_device->Release();
							ms_device = NULL;
						}
					}
					else
					{
						REPORT_LOG(verboseHardwareLogging, ("Failed create %s %s %s %s\n", vertexProcessingModeText, getFormatName(ms_adapterFormat), getFormatName(ms_backBufferFormat), getFormatName(ms_depthStencilFormat)));
					}
				}
				else
				{
					REPORT_LOG(verboseHardwareLogging, ("Failed checks %s %s %s %s\n", vertexProcessingModeText, getFormatName(ms_adapterFormat), getFormatName(ms_backBufferFormat), getFormatName(ms_depthStencilFormat)));
				}
			}
		}
	}

	if (!created)
	{
		char const *message = "Direct3D could not be correctly initialized.  Sometimes this will be fixed by rebooting your machine.\r\nIf it still occurs after a fresh reboot, you may need to reinstall DirectX.";
		MessageBox(NULL, message, NULL, MB_OK | MB_ICONSTOP);
		return false;
	}

	updateWindowSettings();

	ms_hasDepthBuffer = getDepthBufferBitDepth(ms_depthStencilFormat) != 0;
	ms_hasStencilBuffer = getStencilBufferBitDepth(ms_depthStencilFormat) != 0;

	hresult = ms_device->GetDeviceCaps(&ms_deviceCaps);
	FATAL_DX_HR("GetDeviceCaps failed %s", hresult);

	ms_supportsStreamOffsets   = (ms_deviceCaps.DevCaps2 & D3DDEVCAPS2_STREAMOFFSET) != 0;

	ms_supportsDynamicTextures = 
		   !ConfigDirect3d9::getDisableDynamicTextures() 
		&& (ms_deviceCaps.Caps2 & D3DCAPS2_DYNAMICTEXTURES) != 0
		;

	ms_deviceCaps.VertexShaderVersion &= 0xffff;
	ms_deviceCaps.PixelShaderVersion &= 0xffff;

	// limit the vertex and pixel shader version as requested.  this is unlike the override which lets you set it to anything.
	if (ms_deviceCaps.VertexShaderVersion > static_cast<DWORD>(ConfigDirect3d9::getMaxVertexShaderVersion()))
		ms_deviceCaps.VertexShaderVersion = static_cast<DWORD>(ConfigDirect3d9::getMaxVertexShaderVersion());
	if (ms_deviceCaps.PixelShaderVersion > static_cast<DWORD>(ConfigDirect3d9::getMaxPixelShaderVersion()))
		ms_deviceCaps.PixelShaderVersion = static_cast<DWORD>(ConfigDirect3d9::getMaxPixelShaderVersion());

	// figure out what shader capability this adapter has
	if (ConfigDirect3d9::getShaderCapabilityOverride())
		ms_shaderCapability = ConfigDirect3d9::getShaderCapabilityOverride();
	else
	{
#ifdef VSPS
		if (!ConfigDirect3d9::getDisableVertexAndPixelShaders() && ms_deviceCaps.VertexShaderVersion >= 0x0200 && ms_deviceCaps.PixelShaderVersion >= 0x0200)
			ms_shaderCapability = ShaderCapability(2,0);
		else
			if (!ConfigDirect3d9::getDisableVertexAndPixelShaders() && ms_deviceCaps.VertexShaderVersion >= 0x0101 && ms_deviceCaps.PixelShaderVersion >= 0x0104)
				ms_shaderCapability = ShaderCapability(1,4);
			else
				if (!ConfigDirect3d9::getDisableVertexAndPixelShaders() && ms_deviceCaps.VertexShaderVersion >= 0x0101 && ms_deviceCaps.PixelShaderVersion >= 0x0101)
					ms_shaderCapability = ShaderCapability(1,1);
				else
#endif
				{
#ifdef FFP
					if (ms_deviceCaps.MaxSimultaneousTextures >= 3)
						ms_shaderCapability = ShaderCapability(0,3);
					else
						if (ms_deviceCaps.MaxSimultaneousTextures >= 2)
							ms_shaderCapability = ShaderCapability(0,2);
						else
							ms_shaderCapability = ShaderCapability(0,0);
#else
					DEBUG_FATAL(true, ("VSPS-only graphics layer selected but no VSPS supported"));
#endif
				}

		if (ms_shaderCapability == ShaderCapability(2,0))
		{
#define IS_VERSION(a,b,c,d) (product == a && version == b && subVersion == c && build == d)
			int const product    = HIWORD(ms_adapterIdentifier.DriverVersion.HighPart);
			int const version    = LOWORD(ms_adapterIdentifier.DriverVersion.HighPart);
			int const subVersion = HIWORD(ms_adapterIdentifier.DriverVersion.LowPart);
			int const build      = LOWORD(ms_adapterIdentifier.DriverVersion.LowPart);

			if (IS_VERSION(6,14,10,6343) || IS_VERSION(6,14,1,6307) || IS_VERSION(4,14,1,9099) || IS_VERSION(4,14,1,9088))
			{
				REPORT_LOG(verboseHardwareLogging, ("Detected ATI catalyst 3.2 or 3.4 - falling back to shader capability 2.5\n"));
				CrashReportInformation::addStaticText("D3d9Hack1: enabling ATI catalyst 3.2/3.4  workaround\n");
				ms_shaderCapability = ShaderCapability(1,4);
			}
#undef IS_VERSION
			//check for Geforce FX series cards, which screwed up shader2.0 support.  See http://en.wikipedia.org/wiki/GeForce_FX
			if(ms_adapterIdentifier.VendorId == 0x10de) //NVidia
			{
				if(ms_adapterIdentifier.DeviceId  == 0x300 //various Geforce FX card versions
				 || ms_adapterIdentifier.DeviceId == 0x301
				 || ms_adapterIdentifier.DeviceId == 0x302
				 || ms_adapterIdentifier.DeviceId == 0x311
				 || ms_adapterIdentifier.DeviceId == 0x312
				 || ms_adapterIdentifier.DeviceId == 0x314
				 || ms_adapterIdentifier.DeviceId == 0x320
				 || ms_adapterIdentifier.DeviceId == 0x321
				 || ms_adapterIdentifier.DeviceId == 0x322
				 || ms_adapterIdentifier.DeviceId == 0x330
				 || ms_adapterIdentifier.DeviceId == 0x331
				 || ms_adapterIdentifier.DeviceId == 0x333
				 || ms_adapterIdentifier.DeviceId == 0x341
				)
				{
					REPORT_LOG(verboseHardwareLogging, ("Detected Geforce FX series card - falling back to shader capability 1.4\n"));
					CrashReportInformation::addStaticText("D3d9Hack1: enabling Geforce FX series card workaround\n");
					ms_shaderCapability = ShaderCapability(1,4);
				}
			}
		}
	}

	if (ms_shaderCapability < ShaderCapability(1,1))
		ms_glApi.setAlphaFadeOpacity = noSetAlphaFadeOpacity;

	REPORT_LOG(verboseHardwareLogging, ("Using graphics shader capability %d.%d\n", GetShaderCapabilityMajor(ms_shaderCapability), GetShaderCapabilityMinor(ms_shaderCapability)));
	CrashReportInformation::addStaticText("ShaderCapability: %d.%d\n", GetShaderCapabilityMajor(ms_shaderCapability), GetShaderCapabilityMinor(ms_shaderCapability));


	// install the other subsystems
	Direct3d9_VertexDeclarationMap::install();
#ifdef VSPS
	Direct3d9_VertexShaderData::install();
#endif
	Direct3d9_StaticShaderData::install();
	Direct3d9_ShaderImplementationData::install();
	Direct3d9_StateCache::install(getMaximumVertexBufferStreamCount());
	Direct3d9_VertexBufferDescriptorMap::install();
	Direct3d9_StaticVertexBufferData::install();
	Direct3d9_DynamicVertexBufferData::install();
	Direct3d9_StaticIndexBufferData::install();
	Direct3d9_DynamicIndexBufferData::install();
	Direct3d9_TextureData::install();
	Direct3d9_RenderTarget::install();
	Direct3d9_LightManager::install();
#ifdef _DEBUG
	Direct3d9_Metrics::install();
#endif

	setViewport(0, 0, ms_width, ms_height, 0.0, 1.0);

#if DEBUG_LEVEL == DEBUG_LEVEL_DEBUG
	DebugKey::registerFlag(ms_allowDrawCallSelection, "drawCallSelection");
#endif

#ifdef _DEBUG
#if defined(FFP) && defined(VSPS)
	DebugFlags::registerFlag(ms_noRenderVertexShaders,         "Direct3d9", "noRenderVertexShaders");
	DebugFlags::registerFlag(ms_noRenderFixedFunctionPipeline, "Direct3d9", "noRenderFixedFunctionPipeline");
#endif
	DebugFlags::registerFlag(ms_disableAll,                    "Direct3d9", "disableAll");
	DebugFlags::registerFlag(ms_disablePass0Plus,              "Direct3d9", "disablePass0+");
	DebugFlags::registerFlag(ms_disablePass1Plus,              "Direct3d9", "disablePass1+");
	DebugFlags::registerFlag(ms_disablePass2Plus,              "Direct3d9", "disablePass2+");
	DebugFlags::registerFlag(ms_disablePass3Plus,              "Direct3d9", "disablePass3+");
	DebugFlags::registerFlag(ms_minimumDrawPrimitives2,        "Direct3d9", "minimumDrawPrimitives002");
	DebugFlags::registerFlag(ms_minimumDrawPrimitives4,        "Direct3d9", "minimumDrawPrimitives004");
	DebugFlags::registerFlag(ms_minimumDrawPrimitives8,        "Direct3d9", "minimumDrawPrimitives008");
	DebugFlags::registerFlag(ms_minimumDrawPrimitives16,       "Direct3d9", "minimumDrawPrimitives016");
	DebugFlags::registerFlag(ms_minimumDrawPrimitives32,       "Direct3d9", "minimumDrawPrimitives032");
	DebugFlags::registerFlag(ms_minimumDrawPrimitives64,       "Direct3d9", "minimumDrawPrimitives064");
	DebugFlags::registerFlag(ms_minimumDrawPrimitives128,      "Direct3d9", "minimumDrawPrimitives128");
	DebugFlags::registerFlag(ms_minimumDrawPrimitives256,      "Direct3d9", "minimumDrawPrimitives256");
	DebugFlags::registerFlag(ms_minimumDrawPrimitives512,      "Direct3d9", "minimumDrawPrimitives512");
	DebugFlags::registerFlag(ms_noClearFrameBuffer,            "Direct3d9", "noClearFrameBuffer");
	DebugFlags::registerFlag(ms_noClearDepthBuffer,            "Direct3d9", "noClearDepthBuffer");
	DebugFlags::registerFlag(ms_noClearStencilBuffer,          "Direct3d9", "noClearStencilBuffer");
	DebugFlags::registerFlag(ms_noScissor,                     "Direct3d9", "noScissor");
	DebugFlags::registerFlag(ms_noIndexedPrimitives,           "Direct3d9", "noIndexedPrimitives");
	DebugFlags::registerFlag(ms_noNonIndexedPrimitives,        "Direct3d9", "noNonIndexedPrimitives");
	DebugFlags::registerFlag(ms_disableAlphaFadeOpacity,       "Direct3d9", "disableAlphaFadeOpacity");
#endif

#ifndef FFP
	// initialize the last row of the matrix when using VS only so we don't have to touch it again
	ms_cachedObjectToWorldMatrix._41 = 0.0f;
	ms_cachedObjectToWorldMatrix._42 = 0.0f;
	ms_cachedObjectToWorldMatrix._43 = 0.0f;
	ms_cachedObjectToWorldMatrix._44 = 1.0f;

	ms_cachedWorldToCameraMatrix._41 = 0.0f;
	ms_cachedWorldToCameraMatrix._42 = 0.0f;
	ms_cachedWorldToCameraMatrix._43 = 0.0f;
	ms_cachedWorldToCameraMatrix._44 = 1.0f;
#endif

	resizeQuadListIndexBuffer(4 * 1024);

	return true;
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::remove()
{
	DEBUG_FATAL(!ms_installed, ("not installed"));
	ms_installed = false;

	delete ms_performanceTimer;
	ms_performanceTimer = NULL;

	delete ms_temporaryBuffer;
	ms_temporaryBuffer = NULL;

	if (ms_quadListIndexBuffer)
	{
		delete ms_quadListIndexBuffer;
		ms_quadListIndexBuffer = NULL;
	}

	Direct3d9_RenderTarget::remove();
	Direct3d9_TextureData::remove();
	Direct3d9_DynamicIndexBufferData::remove();
	Direct3d9_StaticIndexBufferData::remove();
	Direct3d9_StaticVertexBufferData::remove();
	Direct3d9_DynamicVertexBufferData::remove();
	Direct3d9_VertexBufferDescriptorMap::remove();
#ifdef VSPS
	Direct3d9_VertexShaderData::remove();
#endif
	Direct3d9_VertexDeclarationMap::remove();
	Direct3d9_StateCache::remove();
#ifdef _DEBUG
	Direct3d9_Metrics::remove();
#endif

	releaseBackBuffer();

	if (ms_device)
	{
		IGNORE_RETURN(ms_device->Release());
		ms_device = NULL;
	}

	if (ms_direct3d)
	{
		IGNORE_RETURN(ms_direct3d->Release());
		ms_direct3d = NULL;
	}

	if (ms_engineOwnsWindow)
		SetWindowPos(ms_window, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOCOPYBITS | SWP_HIDEWINDOW);
}

// ----------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Create a DirectDraw interface and query for the video memory
void Direct3d9Namespace::_queryVideoMemory()
{
	IDirectDraw* directDraw = 0;
	if (SUCCEEDED(DirectDrawCreate (NULL, &directDraw, NULL)) && directDraw)
	{
		DDCAPS caps;
		memset (&caps, 0, sizeof (DDCAPS));
		caps.dwSize = sizeof (DDCAPS);
		if (SUCCEEDED(directDraw->GetCaps (&caps, 0)))
			ms_videoMemoryInMegabytes = caps.dwVidMemTotal / (1024 * 1024);

		directDraw->Release ();
		directDraw = 0;
	}
}
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------

void Direct3d9Namespace::displayModeChanged()
{
	ms_displayModeChanged = true;
}

// ----------------------------------------------------------------------

bool Direct3d9Namespace::checkDisplayMode()
{
	ms_displayModeChanged = false;
	if (ms_windowed && ms_engineOwnsWindow)
	{
		// get the current display mode
		Zero(ms_displayMode);
		HRESULT hresult = ms_direct3d->GetAdapterDisplayMode(ms_adapter, &ms_displayMode);
		FATAL_DX_HR("GetAdapterDisplayMode failed %s", hresult);
		ms_adapterFormat = ms_displayMode.Format;

		if (!ms_borderlessWindow && (static_cast<int>(ms_displayMode.Width) == ms_width || static_cast<int>(ms_displayMode.Height) == ms_height))
		{
			if (ms_adapterFormat != D3DFMT_A8R8G8B8 && ms_adapterFormat != D3DFMT_X8R8G8B8)
				Graphics::setLastError("engine", "desktop_same_not_32");
			else
				Graphics::setLastError("engine", "desktop_same");

			return false;
		}

		if (static_cast<int>(ms_displayMode.Width) < ms_width || static_cast<int>(ms_displayMode.Height) < ms_height)
		{
			if (ms_adapterFormat != D3DFMT_A8R8G8B8 && ms_adapterFormat != D3DFMT_X8R8G8B8)
				Graphics::setLastError("engine", "desktop_too_small_not_32");
			else
				Graphics::setLastError("engine", "desktop_too_small");

			return false;
		}

		if (ms_adapterFormat != D3DFMT_A8R8G8B8 && ms_adapterFormat != D3DFMT_X8R8G8B8)
		{
			Graphics::setLastError("engine", "desktop_not_32bit");
			return false;
		}
	}

	return true;
}

// ----------------------------------------------------------------------

int Direct3d9::getAdapter()
{
	return ms_adapter;
}

// ----------------------------------------------------------------------

bool Direct3d9Namespace::isGdiVisible()
{
	return ms_windowed;
}

// ----------------------------------------------------------------------

bool Direct3d9Namespace::wasDeviceReset()
{
	return ms_deviceReset;
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::flushResources(bool fullReset)
{
	if (fullReset)
	{
		setWindowedMode(ms_windowed, true);
	}
	else
	{
		IGNORE_RETURN(ms_device->EvictManagedResources());
	}
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::updateWindowSettings()
{
	if (ms_engineOwnsWindow)
	{
		DWORD const windowStyleWindowed    = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_BORDER | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
		DWORD const windowStyleFullscreen  = WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

		if (ms_windowed)
		{
			RECT rect;
			rect.left   = 0;
			rect.top    = 0;
			rect.right  = ms_width;
			rect.bottom = ms_height;

			DWORD const windowStyle = ms_borderlessWindow ? windowStyleFullscreen : windowStyleWindowed;
			SetWindowLong(ms_window, GWL_STYLE, windowStyle);

			// adjust it to include the windows crap around the client area
			const BOOL result1 = AdjustWindowRect(&rect, windowStyle, FALSE);
			DEBUG_FATAL(!result1, ("AdjustWindowRect failed"));
			UNREF(result1);

			// get the area of the monitor
			HMONITOR monitor = ms_direct3d->GetAdapterMonitor(ms_adapter);
			MONITORINFO monitorInfo;
			Zero(monitorInfo);
			monitorInfo.cbSize = sizeof(monitorInfo);
			GetMonitorInfo(monitor, &monitorInfo);

			if (ms_borderlessWindow)
			{
				ms_windowX = monitorInfo.rcMonitor.left;
				ms_windowY = monitorInfo.rcMonitor.top;
			}
			else
				if (ms_windowX == INT_MAX || ms_windowY == INT_MAX)
				{
					ms_windowX = monitorInfo.rcMonitor.left + (((monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left) - ms_width) / 2);
					ms_windowY = monitorInfo.rcMonitor.top  + (((monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top) - ms_height) / 2);
				}

			// reposition and resize the window
			const BOOL result2 = SetWindowPos(ms_window, HWND_NOTOPMOST, ms_windowX, ms_windowY, rect.right - rect.left, rect.bottom - rect.top, SWP_NOCOPYBITS |  SWP_SHOWWINDOW);
			FATAL(!result2, ("SetWindowPos failed"));
		}
		else
		{
			SetWindowLong(ms_window, GWL_STYLE, windowStyleFullscreen);

			HMONITOR monitor = ms_direct3d->GetAdapterMonitor(ms_adapter);
			MONITORINFO monitorInfo;
			Zero(monitorInfo);
			monitorInfo.cbSize = sizeof(monitorInfo);
			GetMonitorInfo(monitor, &monitorInfo);
			RECT const & r = monitorInfo.rcMonitor;

			const BOOL result2 = SetWindowPos(ms_window, HWND_TOPMOST, r.left, r.top, ms_width, ms_height, SWP_NOCOPYBITS |  SWP_SHOWWINDOW);
			FATAL(!result2, ("SetWindowPos failed"));
		}
	}
}

// ----------------------------------------------------------------------

bool Direct3d9::engineOwnsWindow()
{
	return ms_engineOwnsWindow;
}

// ----------------------------------------------------------------------

int Direct3d9::getMaxRenderTargetWidth()
{
	return ms_width;
}

// ----------------------------------------------------------------------

int Direct3d9::getMaxRenderTargetHeight()
{
	return ms_height;
}

// ----------------------------------------------------------------------

bool Direct3d9::supportsPixelShaders()
{
#if defined(FFP) && defined(VSPS)
	return ms_deviceCaps.PixelShaderVersion > 0;
#elif defined(FFP)
	return false;
#else
	return true;
#endif
}

// ----------------------------------------------------------------------

bool Direct3d9::supportsAntialias()
{
	return ConfigDirect3d9::getAntiAlias() && ms_supportsMultiSample;
}

// ----------------------------------------------------------------------

void Direct3d9::setAntialiasEnabled(bool enabled)
{
	if(supportsAntialias())
	{
		if(ms_antialiasEnabled != enabled)
		{
			ms_antialiasEnabled = enabled;
			setWindowedMode(ms_windowed, true);
		}
		ms_antialiasEnabled = enabled;
		Direct3d9_StateCache::setRenderState(D3DRS_MULTISAMPLEANTIALIAS, enabled);
	}
}

// ----------------------------------------------------------------------

bool Direct3d9::supportsVertexShaders()
{
#if defined(FFP) && defined(VSPS)
	return ms_deviceCaps.VertexShaderVersion > 0;
#elif defined(FFP)
	return false;
#else
	return true;
#endif
}

// ----------------------------------------------------------------------

bool Direct3d9Namespace::supportsScissorRect()
{
	return (ms_deviceCaps.RasterCaps & D3DPRASTERCAPS_SCISSORTEST) != 0;
}

// ----------------------------------------------------------------------

bool Direct3d9Namespace::supportsMipmappedCubeMaps()
{
	return (ms_deviceCaps.CubeTextureFilterCaps & (D3DPTFILTERCAPS_MIPFLINEAR | D3DPTFILTERCAPS_MIPFPOINT)) != 0;
}

// ----------------------------------------------------------------------

bool Direct3d9Namespace::supportsHardwareMouseCursor()
{
	return (ms_deviceCaps.CursorCaps & D3DCURSORCAPS_COLOR) != 0;
}

// ----------------------------------------------------------------------

bool Direct3d9::supportsTwoSidedStencil()
{
	return (ms_deviceCaps.StencilCaps & D3DSTENCILCAPS_TWOSIDED) != 0;
}

// ----------------------------------------------------------------------

bool Direct3d9::supportsStreamOffsets()
{
	return ms_supportsStreamOffsets;
}

// ----------------------------------------------------------------------

bool Direct3d9::supportsDynamicTextures()
{
	return ms_supportsDynamicTextures;
}

// ----------------------------------------------------------------------

DWORD Direct3d9::getMaxAnisotropy()
{
	return ms_deviceCaps.MaxAnisotropy;
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::addDeviceLostCallback(CallbackFunction callbackFunction)
{
	DEBUG_FATAL(std::find(ms_deviceLostCallbacks.begin(), ms_deviceLostCallbacks.end(), callbackFunction) != ms_deviceLostCallbacks.end(), ("Callback function already on list"));
	ms_deviceLostCallbacks.push_back(callbackFunction);
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::removeDeviceLostCallback(CallbackFunction callbackFunction)
{
	CallbackFunctions::iterator const i = std::find(ms_deviceLostCallbacks.begin(), ms_deviceLostCallbacks.end(), callbackFunction);
	DEBUG_FATAL(i == ms_deviceLostCallbacks.end(), ("Callback function not on list"));
	ms_deviceLostCallbacks.erase(i);
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::addDeviceRestoredCallback(CallbackFunction callbackFunction)
{
	DEBUG_FATAL(std::find(ms_deviceRestoredCallbacks.begin(), ms_deviceRestoredCallbacks.end(), callbackFunction) != ms_deviceRestoredCallbacks.end(), ("Callback function already on list"));
	ms_deviceRestoredCallbacks.push_back(callbackFunction);
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::removeDeviceRestoredCallback(CallbackFunction callbackFunction)
{
	CallbackFunctions::iterator const i = std::find(ms_deviceRestoredCallbacks.begin(), ms_deviceRestoredCallbacks.end(), callbackFunction);
	DEBUG_FATAL(i == ms_deviceRestoredCallbacks.end(), ("Callback function not on list"));
	ms_deviceRestoredCallbacks.erase(i);
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::lostDevice()
{
	CallbackFunctions::const_iterator const iEnd = ms_deviceLostCallbacks.end();
	for (CallbackFunctions::const_iterator i = ms_deviceLostCallbacks.begin(); i != iEnd; ++i)
		(*(*i))();

	Direct3d9_DynamicVertexBufferData::lostDevice();
	Direct3d9_DynamicIndexBufferData::lostDevice();
	Direct3d9_StateCache::lostDevice();
	Direct3d9_RenderTarget::lostDevice();
	Direct3d9_ShaderImplementationData::lostDevice();
	ms_transformDirty = true;

	releaseBackBuffer();
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::restoreDevice()
{
	Direct3d9_RenderTarget::restoreDevice();
	Direct3d9_DynamicVertexBufferData::restoreDevice();
	Direct3d9_DynamicIndexBufferData::restoreDevice();
	Direct3d9_StateCache::restoreDevice();
	setViewport(ms_viewportX, ms_viewportY, ms_viewportWidth, ms_viewportHeight, ms_viewportMinimumZ, ms_viewportMaximumZ);

	CallbackFunctions::const_iterator const iEnd = ms_deviceRestoredCallbacks.end();
	for (CallbackFunctions::const_iterator i = ms_deviceRestoredCallbacks.begin(); i != iEnd; ++i)
		(*(*i))();

	ms_deviceReset = true;
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::resize(int newWidth, int newHeight)
{
	lostDevice();

	// change the width
	ms_width = newWidth;
	ms_height = newHeight;
	ms_presentParameters.BackBufferWidth  = ms_width;
	ms_presentParameters.BackBufferHeight = ms_height;

	// try to restore the device
	const HRESULT hresult = ms_device->Reset(&ms_presentParameters);
	FATAL(FAILED(hresult), ("Reset failed in resize %d", HRESULT_CODE(hresult)));

	// recreate necessary resources after resetting the device
	restoreDevice();
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::setWindowedMode(bool windowed, bool force)
{
	if (ms_engineOwnsWindow && (force || ms_windowed != windowed))
	{
		if (ms_windowed)
		{
			RECT rect;
			GetWindowRect(ms_window, &rect);
			ms_windowX = rect.left;
			ms_windowY = rect.top;
		}

		// change the windowed mode
		bool old = ms_windowed;
		ms_windowed = windowed;
		setPresentParameters();
		lostDevice();
		HRESULT hresult = ms_device->Reset(&ms_presentParameters);
		if (FAILED(hresult))
		{
			// couldn't switch.  try to go back
			ms_windowed = old;
			setPresentParameters();
			HRESULT second = ms_device->Reset(&ms_presentParameters);

			FATAL(FAILED(second), ("Reset failed in setWindowedMode %d %d", HRESULT_CODE(hresult), HRESULT_CODE(second)));
		}

		updateWindowSettings();

		//  When going from full screen to windowed mode, a portion of the upper left corner of the screen
		//   with dimensions up to the size of the game's full screen mode dimensions does not always get
		//   updated.  This RedrawWindow call fixes the problem by making all desktop windows draw.
		if (ms_windowed)
			RedrawWindow( NULL, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN );

		// recreate necessary resources after resetting the device
		restoreDevice();
	}

	if (!checkDisplayMode())
		setWindowedMode(false);

	ms_windowedModeChanged(ms_windowed);
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::setWindowedMode(bool windowed)
{
	setWindowedMode(windowed, false);
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::setBrightnessContrastGamma(float brightness, float contrast, float gamma)
{
	D3DGAMMARAMP gammaRamp;
	int i = 0;;
	float f = 0.0f;
	float const step = 1.0 / 256.0f;
	float oneOverGamma = 1.0f / gamma;
	for ( ; i < 256; ++i, f += step)
	{
		float result = pow(0.5f + contrast * ((f * brightness) - 0.5f), oneOverGamma);
		WORD w = static_cast<WORD>(clamp(0, static_cast<int>(result * 65535.0f), 65535));
		BYTE b = static_cast<BYTE>(clamp(0, static_cast<int>(result * 255.0f), 255));
		gammaRamp.red[i]   = w;
		gammaRamp.green[i] = w;
		gammaRamp.blue[i]  = w;
		ms_colorCorrectionTable[i] = b;
	}

	ms_device->SetGammaRamp(0, D3DSGR_NO_CALIBRATION, &gammaRamp);
}

// ----------------------------------------------------------------------
/**
 * This sets the presentParameters MultiSampleType, SwapEffect, and Fullscreen_PresentationInterval.
 * Antialiasing in windowed mode is disabled.
 */

void Direct3d9Namespace::setPresentParameters()
{
	if (ms_windowed)
	{
		ms_presentParameters.Windowed = TRUE;
		ms_presentParameters.SwapEffect =  D3DSWAPEFFECT_COPY;
		ms_presentParameters.FullScreen_RefreshRateInHz = 0;
		ms_presentParameters.MultiSampleType = D3DMULTISAMPLE_NONE;
		ms_presentParameters.MultiSampleQuality = 0;
	}
	else
	{
		ms_presentParameters.Windowed = FALSE;
		ms_presentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
		const int refreshRate = ConfigDirect3d9::getFullscreenRefreshRate();
		ms_presentParameters.FullScreen_RefreshRateInHz = refreshRate > 0 ? refreshRate : D3DPRESENT_RATE_DEFAULT;
		ms_presentParameters.MultiSampleType = D3DMULTISAMPLE_NONE;
		ms_presentParameters.MultiSampleQuality = 0;
	}
	if(!ms_windowed && ms_supportsMultiSample && ConfigDirect3d9::getAntiAlias() && ms_antialiasEnabled)
	{
		ms_presentParameters.MultiSampleType = D3DMULTISAMPLE_NONMASKABLE;
		ms_presentParameters.MultiSampleQuality = ms_multiSampleQualityLevels - 1;
		if(ms_presentParameters.MultiSampleQuality > 2)
			ms_presentParameters.MultiSampleQuality = 2;
		ms_presentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	}

	if (ConfigDirect3d9::getAllowTearing())
		ms_presentParameters.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	else
		ms_presentParameters.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::setFillMode(GlFillMode newFillMode)
{
	switch (newFillMode)
	{
		case GFM_wire:
			Direct3d9_StateCache::setRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
			break;

		case GFM_solid:
			Direct3d9_StateCache::setRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
			break;

		default:
			DEBUG_FATAL(true, ("bad fill mode"));
	}
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::setCullMode(GlCullMode newCullMode)
{
	switch (newCullMode)
	{
		case GCM_none:
			Direct3d9_StateCache::setRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
			break;

		case GCM_clockwise:
			Direct3d9_StateCache::setRenderState(D3DRS_CULLMODE, D3DCULL_CW);
			break;

		case GCM_counterClockwise:
			Direct3d9_StateCache::setRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
			break;

		default:
			DEBUG_FATAL(true, ("bad cull mode"));
	}
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::setPointSize(float size)
{
	Direct3d9_StateCache::setRenderState(D3DRS_POINTSIZE,size);
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::setPointSizeMin( float min )
{
	Direct3d9_StateCache::setRenderState(D3DRS_POINTSIZE_MIN,min);
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::setPointSizeMax( float max )
{
	Direct3d9_StateCache::setRenderState(D3DRS_POINTSIZE_MAX,max);
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::setPointScaleEnable( bool bEnable )
{
	Direct3d9_StateCache::setRenderState(D3DRS_POINTSCALEENABLE,bEnable);
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::setPointScaleFactor( float A, float B, float C )
{
	Direct3d9_StateCache::setRenderState(D3DRS_POINTSCALE_A,A);
	Direct3d9_StateCache::setRenderState(D3DRS_POINTSCALE_B,B);
	Direct3d9_StateCache::setRenderState(D3DRS_POINTSCALE_C,C);
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::setPointSpriteEnable( bool bEnable )
{
	Direct3d9_StateCache::setRenderState(D3DRS_POINTSPRITEENABLE,bEnable);
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::clearViewport(bool clearColor, uint32 colorValue, bool clearDepth, float depthValue, bool clearStencil, uint32 stencilValue)
{
#ifdef _DEBUG
	if (ms_noClearFrameBuffer)
		clearColor = false;
	if (ms_noClearDepthBuffer)
		clearDepth = false;
	if (ms_noClearStencilBuffer)
		clearStencil = false;
#endif

	// d3d will fail the clear call if we try to clear a buffer that's not present
	if (!ms_hasDepthBuffer)
		clearDepth = false;
	if (!ms_hasStencilBuffer)
		clearStencil = false;

#ifdef _DEBUG
	static bool slowClearWarning;
	if (!slowClearWarning && ms_hasDepthBuffer && ms_hasStencilBuffer && ((clearDepth || clearStencil) && !(clearDepth && clearStencil)))
	{
		slowClearWarning = true;
		Warning("Clearing depth or stencil buffer but not both is inefficient");
	}
#endif

	const DWORD flags = (clearColor ? D3DCLEAR_TARGET : 0) | (clearDepth ? D3DCLEAR_ZBUFFER : 0) | (clearStencil ? D3DCLEAR_STENCIL : 0);
	const HRESULT hresult = ms_device->Clear(0, NULL, flags, colorValue, depthValue, stencilValue);
	FATAL(FAILED(hresult), ("Clear failed %d: %08x %08x %5.2f %08x", HRESULT_CODE(hresult), flags, colorValue, depthValue, stencilValue));
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::resizeQuadListIndexBuffer(int numberOfQuads)
{
	if (ms_quadListIndexBufferNumberOfQuads == 0)
	{
		ms_quadListIndexBufferNumberOfQuads = numberOfQuads;
	}
	else
	{
		delete ms_quadListIndexBuffer;
		ms_quadListIndexBuffer = NULL;

		while (ms_quadListIndexBufferNumberOfQuads < numberOfQuads)
			ms_quadListIndexBufferNumberOfQuads *= 2;
	}

	// create the new index buffer
	ms_quadListIndexBuffer = new StaticIndexBuffer(ms_quadListIndexBufferNumberOfQuads * 6);

	// fill the new index buffer
	ms_quadListIndexBuffer->lock();

		Index *index = ms_quadListIndexBuffer->begin();
		for (int i = 0, base = 0; i < ms_quadListIndexBufferNumberOfQuads; ++i, base += 4)
		{
			*index = static_cast<Index>(base + 0);
			++index;
			*index = static_cast<Index>(base + 1);
			++index;
			*index = static_cast<Index>(base + 2);
			++index;
			*index = static_cast<Index>(base + 0);
			++index;
			*index = static_cast<Index>(base + 2);
			++index;
			*index = static_cast<Index>(base + 3);
			++index;
		}

	ms_quadListIndexBuffer->unlock();
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::update(float elapsedTime)
{
	++ms_frameNumber;
	ms_currentTime += elapsedTime;

#ifdef _DEBUG
	Direct3d9_Metrics::reset();
#endif

#ifdef VSPS
	const float time[4] = { ms_currentTime, 0.f, 0.f, 0.f };
	Direct3d9_StateCache::setVertexShaderConstants(VSCR_currentTime, time, 1);
#endif

#if DEBUG_LEVEL == DEBUG_LEVEL_DEBUG

	if (ms_allowDrawCallSelection)
	{
		if (DebugKey::isPressed(1))
			ms_allowDrawCallPointer = &ms_allowDrawCallBegin;
		if (DebugKey::isPressed(2))
			ms_allowDrawCallPointer = &ms_allowDrawCallEnd;
		if (DebugKey::isPressed(3))
			*ms_allowDrawCallPointer -= 1;
		if (DebugKey::isPressed(4))
			*ms_allowDrawCallPointer += 1;

		if (DebugKey::isPressed(7))
		{
			ms_allowDrawCallBegin = 0;
			ms_allowDrawCallEnd   = 0x7fffffff;
		}
		if (DebugKey::isPressed(8))
			ms_allowDrawCallEnd = ms_allowDrawCallBegin;
		if (DebugKey::isPressed(9))
		{
			ms_allowDrawCallBegin -= 1;
			ms_allowDrawCallEnd   -= 1;
		}
		if (DebugKey::isPressed(0))
		{
			ms_allowDrawCallBegin += 1;
			ms_allowDrawCallEnd   += 1;
		}
		DEBUG_REPORT_PRINT(true, ("frame  %3d=last %3d=begin  %3d=end\n", ms_drawCall, ms_allowDrawCallBegin, ms_allowDrawCallEnd));
	}
	ms_drawCall = 0;
#endif
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::beginScene()
{
	if (ms_displayModeChanged && !checkDisplayMode())
		setWindowedMode(false);

	// begin the 3d scene
	const HRESULT hresult = ms_device->BeginScene();
	FATAL_DX_HR("BeginScene failed %s", hresult);

	Direct3d9_StaticShaderData::beginFrame();
	Direct3d9_DynamicVertexBufferData::beginFrame();
	Direct3d9_DynamicIndexBufferData::beginFrame();
	Direct3d9_LightManager::beginFrame();
	ms_savedIndexBuffer = NULL;
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::endScene()
{
	// end the 3d scene
	HRESULT hresult = ms_device->EndScene();
	FATAL_DX_HR("EndScene failed %s", hresult);
}

// ----------------------------------------------------------------------

bool Direct3d9Namespace::getBackBuffer()
{
	if (!ms_backBuffer)
	{
		HRESULT hresult = ms_device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &ms_backBuffer);
		if (FAILED(hresult) || !ms_backBuffer)
		{
			Graphics::setLastError("engine", "failure_to_get_backbuffer");
			return false;
		}
	}
	return true;
}

// ----------------------------------------------------------------------

bool Direct3d9Namespace::lockBackBuffer(Gl_pixelRect &o_pixels, const RECT *i_lockRect)
{
	HRESULT hresult;

	if (!ms_backBuffer && !getBackBuffer())
	{
		return false;
	}

	D3DLOCKED_RECT lockedRect;
	hresult = ms_backBuffer->LockRect(&lockedRect, i_lockRect, 0);
	if (FAILED(hresult))
	{
		Graphics::setLastError("engine", "failure_to_lock_backbuffer");
		return false;
	}
	ms_backBufferLocked=true;

	o_pixels.pixels = lockedRect.pBits;
	o_pixels.pitch = lockedRect.Pitch;

	D3DSURFACE_DESC desc;
	hresult = ms_backBuffer->GetDesc(&desc);

	//o_height = desc.Height;

	switch (desc.Format)
	{
	case D3DFMT_A2R10G10B10:
		o_pixels.colorBits=30;
		o_pixels.alphaBits=2;
		break;
	case D3DFMT_A8R8G8B8:
		o_pixels.colorBits=24;
		o_pixels.alphaBits=8;
		break;
	case D3DFMT_A1R5G5B5:
		o_pixels.colorBits=15;
		o_pixels.alphaBits=1;
		break;
	case D3DFMT_R5G6B5:
		o_pixels.colorBits=16;
		o_pixels.alphaBits=0;
		break;
	default:
		FATAL(true, ("Unsupported back buffer pixel format.\n"));
	}

	return true;
}

// ----------------------------------------------------------------------

bool Direct3d9Namespace::unlockBackBuffer()
{
	if (!ms_backBufferLocked || !ms_backBuffer)
	{
		return false;
	}

	ms_backBuffer->UnlockRect();
	ms_backBufferLocked=false;

	return true;
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::releaseBackBuffer()
{
	if (ms_backBuffer)
	{
		if (ms_backBufferLocked)
		{
			unlockBackBuffer();
		}

		IGNORE_RETURN(ms_backBuffer->Release());
		ms_backBuffer = NULL;
	}
}

// ----------------------------------------------------------------------

bool Direct3d9Namespace::present(bool windowed, HWND window, int width, int height)
{
	if (ms_displayModeChanged)
	{
		if (!checkDisplayMode())
			setWindowedMode(false);
		return false;
	}

	// display the new frame (but only do it when we're rendering to the frame buffer rather than a texture)
	HRESULT hresult;
	if (windowed)
	{
		RECT source;
		source.top = 0;
		source.left = 0;
		source.right = width;
		source.bottom = height;

		hresult = ms_device->Present(&source, NULL, window, NULL);
	}
	else
	{
		// flipping requires all the arguments are NULL
		hresult = ms_device->Present(NULL, NULL, NULL, NULL);
	}

	// check if the device was lost for any reason
	if (hresult == D3DERR_DEVICELOST || hresult == D3DERR_DRIVERINTERNALERROR)
	{
		char present[16];
		sprintf(present, "%d", HRESULT_CODE(hresult));

		// check if we can restore the device now
		hresult = ms_device->TestCooperativeLevel();
		if (SUCCEEDED(hresult) || hresult == D3DERR_DEVICENOTRESET)
		{
			char tcl[16];
			sprintf(tcl, "%d", HRESULT_CODE(hresult));

			DEBUG_REPORT_LOG(true, ("Device lost, restoring now\n"));

			lostDevice();

			// try to restore the device
			hresult = ms_device->Reset(&ms_presentParameters);

			// give a better message if we get back INVALIDCALL
			FATAL(hresult == D3DERR_INVALIDCALL, ("Reset failed %d - likely unreleased render target", HRESULT_CODE(hresult)));

			for (int i = 0; hresult == D3DERR_DEVICELOST && i < 60; ++i)
			{
				char present[16];
				sprintf(present, "%d", HRESULT_CODE(hresult));
				DEBUG_REPORT_LOG(true, ("Reset failed, trying repeatedly\n"));

				Sleep(500);
				hresult = ms_device->Reset(&ms_presentParameters);
			}

			FATAL_DX_HR("Reset failed after present %s", hresult);

			// recreate necessary resources after resetting the device
			restoreDevice();
			return false;
		}
		else
			if (hresult == D3DERR_DEVICELOST)
			{
				// device is lost, can't restore just yet.  waste some time.
				DEBUG_REPORT_LOG(true, ("Device lost, waiting to restore\n"));
				Sleep(50);
				return false;
			}
			else
			{
				FATAL_DX_HR("TestCooperativeLevel failed %s", hresult);
			}
	}
	else
	{
		FATAL_DX_HR("Present failed %s", hresult);
	}

	ms_deviceReset = false;

	return true;
}

// ----------------------------------------------------------------------

bool Direct3d9Namespace::present()
{
	return present(ms_windowed, ms_window, ms_width, ms_height);
}

// ----------------------------------------------------------------------

bool Direct3d9Namespace::present(HWND window, int width, int height)
{
	return present(true, window, width, height);
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::setRenderTarget(Texture *texture, CubeFace cubeFace, int mipmapLevel)
{
	if (texture)
		Direct3d9_RenderTarget::setRenderTarget(texture, cubeFace, mipmapLevel);
	else
		Direct3d9_RenderTarget::setRenderTargetToPrimary();
}

// ----------------------------------------------------------------------

bool Direct3d9Namespace::copyRenderTargetToNonRenderTargetTexture()
{
	return Direct3d9_RenderTarget::copyRenderTargetToNonRenderTargetTexture();
}

// ----------------------------------------------------------------------

bool Direct3d9Namespace::applyGammaCorrectionToXRGBSurface( IDirect3DSurface9 *surface )
{
	if (!surface)
		return false;

	D3DSURFACE_DESC desc;
	surface->GetDesc(&desc);

	// check the format
	if (desc.Format != D3DFMT_A8R8G8B8 && desc.Format != D3DFMT_X8R8G8B8)
		return false;

	D3DLOCKED_RECT lockedRect;

	// lock the surface
	HRESULT hresult = surface->LockRect(&lockedRect, NULL, 0);
	if (FAILED(hresult))
		return false;

	// color correct the bytes
	for( unsigned nLine = 0; nLine != desc.Height; nLine++ )
	{
		// get the start of the line
		PackedArgb * pBuffer = (PackedArgb *)((unsigned int)lockedRect.pBits + lockedRect.Pitch * nLine);

		// color correct the line
		PackedArgb * pBufferEol = pBuffer + desc.Width;
		for(;pBuffer != pBufferEol;pBuffer++)
		{
			// color
			pBuffer->setR( ms_colorCorrectionTable[ pBuffer->getR() ] );
			pBuffer->setG( ms_colorCorrectionTable[ pBuffer->getG() ] );
			pBuffer->setB( ms_colorCorrectionTable[ pBuffer->getB() ] );
		}
	}

	// unlock the surface
	hresult = surface->UnlockRect();

	return true;
}

// ----------------------------------------------------------------------

bool Direct3d9Namespace::screenShot(GlScreenShotFormat format, int quality, const char *fileName)
{
	HRESULT hresult;
	IDirect3DSurface9 *surface = NULL;
	POINT clientTopLeft = { 0, 0 };
	RECT monitorCoordinates = { 0, 0, 0, 0 };
	int offset = 0;

	if (ConfigDirect3d9::getScreenShotBackBuffer())
	{
		// I don't feel like handling pixel format conversions, and this is what the game is supposed to be running in anyway.
		if (ms_backBufferFormat != D3DFMT_A8R8G8B8 && ms_backBufferFormat != D3DFMT_X8R8G8B8)
		{
			Graphics::setLastError("engine", "screenshot_failed_wrong_format");
			return false;
		}

		// get access to the back buffer surface
		hresult = ms_device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &surface);

		// if unable to get access to the back buffer return false
		if (FAILED(hresult)||surface==NULL)
		{
			Graphics::setLastError("engine", "screenshot_failed_unknown");
			return false;
		}
	}
	else
	{
		// find out how big the scratch buffer should be
		D3DDISPLAYMODE displayMode;
		hresult = ms_device->GetDisplayMode(0, &displayMode);
		if (FAILED(hresult))
		{
			Graphics::setLastError("engine", "screenshot_failed_unknown");
			return false;
		}

		// find out where the monitor is anchored
		HMONITOR monitor = ms_direct3d->GetAdapterMonitor(ms_adapter);
		MONITORINFO monitorInfo;
		Zero(monitorInfo);
		monitorInfo.cbSize = sizeof(monitorInfo);
		GetMonitorInfo(monitor, &monitorInfo);
		monitorCoordinates = monitorInfo.rcMonitor;

		// get the window location relative to the monitor
		if (ms_windowed)
			ClientToScreen(ms_window, &clientTopLeft);

		// make sure the entire window is on the screen
		if (ms_windowed)
		{
			if (clientTopLeft.x < static_cast<int>(monitorCoordinates.left) || clientTopLeft.x+ms_width > static_cast<int>(monitorCoordinates.right) || clientTopLeft.y < static_cast<int>(monitorCoordinates.top) || clientTopLeft.y+ms_height > static_cast<int>(monitorCoordinates.bottom))
			{
				Graphics::setLastError("engine", "screenshot_failed_off_desktop");
				return false;
			}

			offset = ((clientTopLeft.x - monitorCoordinates.left) * 4) + ((clientTopLeft.y - monitorCoordinates.top) * displayMode.Width * 4);
		}

		// create a surface for the screen shot
		hresult = ms_device->CreateOffscreenPlainSurface(displayMode.Width, displayMode.Height, D3DFMT_A8R8G8B8, D3DPOOL_SCRATCH, &surface, NULL);
		if (FAILED(hresult) || surface == NULL)
		{
			Graphics::setLastError("engine", "screenshot_failed_unknown");
			return false;
		}

		// copy the front buffer to the surface
		hresult = ms_device->GetFrontBufferData(0, surface);

		// if unable to copy the data return false
		if (FAILED(hresult))
		{
			Graphics::setLastError("engine", "screenshot_failed_unknown");
			surface->Release();
			return false;
		}

		// if not windowed apply gamma correction to the ARGB copy of the front buffer
		if (!ms_windowed)
			applyGammaCorrectionToXRGBSurface( surface );
	}

	switch (format)
	{
		case GSSF_tga:
		{
			// lock the buffer
			D3DLOCKED_RECT lockedRect;
			hresult = surface->LockRect(&lockedRect, NULL, D3DLOCK_READONLY);
			if (FAILED(hresult))
			{
				Graphics::setLastError("engine", "screenshot_failed_unknown");
				surface->Release();
				return false;
			}

			// write it out as a TGA
			char buffer[Os::MAX_PATH_LENGTH];
			sprintf(buffer, "%s.tga", fileName);
			WriteTGA::write(buffer, ms_width, ms_height, reinterpret_cast<const byte *>(lockedRect.pBits) + offset, true, lockedRect.Pitch);

			// unlock the surface
			hresult = surface->UnlockRect();
			FATAL_DX_HR("Unlock failed %s", hresult);
			break;
		}

		case GSSF_bmp:
		{
			char buffer[Os::MAX_PATH_LENGTH];
			sprintf(buffer, "%s.bmp", fileName);

			RECT r;
			r.left   = (clientTopLeft.x - monitorCoordinates.left);
			r.top    = (clientTopLeft.y - monitorCoordinates.top);
			r.right  = r.left + ms_width;
			r.bottom = r.top + ms_height;

			hresult = D3DXSaveSurfaceToFile(buffer, D3DXIFF_BMP, surface, NULL, &r);
			if (FAILED(hresult))
			{
				Graphics::setLastError("engine", "screenshot_failed_write_problem");
				surface->Release();
				return false;
			}
			break;
		}

		case GSSF_jpg:
		{
			// get access to the pixels
			D3DLOCKED_RECT lockedRect;
			hresult = surface->LockRect(&lockedRect, NULL, D3DLOCK_READONLY);
			if (FAILED(hresult))
			{
				Graphics::setLastError("engine", "screenshot_failed_unknown");
				surface->Release();
				return false;
			}
			byte const *start = reinterpret_cast<byte const *>(lockedRect.pBits) + offset;

			// open the output file
			char buffer[Os::MAX_PATH_LENGTH];
			sprintf(buffer, "%s.jpg", fileName);
			FILE *outputFile = fopen(buffer, "wb");
			if (outputFile == NULL)
			{
				Graphics::setLastError("engine", "screenshot_failed_write_problem");
				return false;
			}

			// setup the jpeg compressions
			jpeg_compress_struct cinfo;
			jpeg_error_mgr jerr;
			cinfo.err = jpeg_std_error(&jerr);
			jpeg_create_compress(&cinfo);
			jpeg_stdio_dest(&cinfo, outputFile);
			cinfo.image_width = ms_width;
			cinfo.image_height = ms_height;
			cinfo.input_components = 3;
			cinfo.in_color_space = JCS_RGB;
			jpeg_set_defaults(&cinfo);
			if (quality > 0)
				jpeg_set_quality(&cinfo, clamp(0, quality, 100), TRUE);
			jpeg_start_compress(&cinfo, TRUE);

			// add the pixels to the jpeg image
			byte *scanLine = new byte[ms_width * 3];
			for (int y = 0; y < ms_height; ++y, start += lockedRect.Pitch)
			{
				uint32 const * source = reinterpret_cast<uint32 const *>(start);
				for (int x = 0, b = 0; x < ms_width; ++x)
				{
					uint32 pixel = source[x];
					scanLine[b++] = static_cast<byte>((pixel >> 16) & 0xff);
					scanLine[b++] = static_cast<byte>((pixel >>  8) & 0xff);
					scanLine[b++] = static_cast<byte>((pixel >>  0) & 0xff);
				}

				jpeg_write_scanlines(&cinfo, &scanLine, 1);
			}

			// done compressing the JPEG image, time to clean up
			delete [] scanLine;
			jpeg_finish_compress(&cinfo);
			jpeg_destroy_compress(&cinfo);

			fclose(outputFile);

			// unlock the surface
			hresult = surface->UnlockRect();
			FATAL_DX_HR("Unlock failed %s", hresult);

			break;
		}

		default:
		{
			Graphics::setLastError("engine", "screenshot_failed_unknown_format");
			surface->Release();
			return false;
		}
	}

	surface->Release();
	return true;
}

//-----------------------------------------------------------------------

TextureGraphicsData *Direct3d9Namespace::createTextureData(const Texture &texture, const TextureFormat *runtimeFormats, int numberOfRuntimeFormats)
{
	return new Direct3d9_TextureData(texture, runtimeFormats, numberOfRuntimeFormats);
}

// ----------------------------------------------------------------------

#ifdef _DEBUG
void Direct3d9Namespace::showMipmapLevels(bool enabled)
{
	ms_showMipmapLevelsEnabled = enabled;
}
#endif

// ----------------------------------------------------------------------

#ifdef _DEBUG
bool Direct3d9Namespace::getShowMipmapLevels()
{
	return ms_showMipmapLevelsEnabled;
}
#endif

// ----------------------------------------------------------------------

#ifdef _DEBUG

void Direct3d9Namespace::setBadVertexBufferVertexShaderCombination(bool *flag, const char *appearanceName)
{
#ifdef VSPS
	ms_badVertexBufferVertexShaderCombination = flag;
	ms_appearanceName = appearanceName;
#else
	UNREF(flag);
	UNREF(appearanceName);
#endif
}

#endif

// ----------------------------------------------------------------------

#ifdef _DEBUG
void Direct3d9Namespace::getRenderedVerticesPointsLinesTrianglesCalls(int &vertices, int &points, int &lines, int &triangles,int &calls)
{
	vertices  = Direct3d9_Metrics::vertices;
	points    = Direct3d9_Metrics::points;
	lines     = Direct3d9_Metrics::lines;
	triangles = Direct3d9_Metrics::triangles;
	calls     = Direct3d9_Metrics::drawPrimitiveCalls + Direct3d9_Metrics::drawIndexedPrimitiveCalls;
}
#endif

// ----------------------------------------------------------------------

void Direct3d9Namespace::setBadVertexShaderStaticShader(const StaticShader *shader)
{
	ms_badVertexShaderStaticShader = shader;
}

// ----------------------------------------------------------------------

void Direct3d9::setStaticShader(const StaticShader &shader, int pass)
{
#ifdef _DEBUG
	if (ms_disableAll)
		return;

	ms_currentPassIndex = clamp(0, pass, Direct3d9_Metrics::MAX_PASSES-1);

	if (ms_disablePass0Plus && pass >= 0)
		return;
	if (ms_disablePass1Plus && pass >= 1)
		return;
	if (ms_disablePass2Plus && pass >= 2)
		return;
	if (ms_disablePass3Plus && pass >= 3)
		return;

	ms_currentShader = &shader;
#endif

	const Direct3d9_StaticShaderData *d3dShader = static_cast<const Direct3d9_StaticShaderData *>(shader.m_graphicsData);
#if defined(FFP) && defined(VSPS)
	ms_usingVertexShader = d3dShader->apply(pass);
	Direct3d9_LightManager::setUsingVertexShaderProgram(ms_usingVertexShader);
#else
	IGNORE_RETURN(d3dShader->apply(pass));
#endif
	Direct3d9_LightManager::setObeysLightScale(shader.obeysLightScale());
}

// ----------------------------------------------------------------------

#ifdef _DEBUG
void Direct3d9::clearStaticShader()
{
	ms_currentShader = NULL;
}
#endif

// ----------------------------------------------------------------------

void Direct3d9::convertTransformToMatrix(const Transform &transform, D3DMATRIX & matrix)
{
#ifdef FFP
	matrix._11 = transform.matrix[0][0];
	matrix._12 = transform.matrix[1][0];
	matrix._13 = transform.matrix[2][0];
	matrix._14 = 0.0f;

	matrix._21 = transform.matrix[0][1];
	matrix._22 = transform.matrix[1][1];
	matrix._23 = transform.matrix[2][1];
	matrix._24 = 0.0f;

	matrix._31 = transform.matrix[0][2];
	matrix._32 = transform.matrix[1][2];
	matrix._33 = transform.matrix[2][2];
	matrix._34 = 0.0f;

	matrix._41 = transform.matrix[0][3];
	matrix._42 = transform.matrix[1][3];
	matrix._43 = transform.matrix[2][3];
	matrix._44 = 1.0f;
#else
	matrix._11 = transform.matrix[0][0];
	matrix._12 = transform.matrix[0][1];
	matrix._13 = transform.matrix[0][2];
	matrix._14 = transform.matrix[0][3];

	matrix._21 = transform.matrix[1][0];
	matrix._22 = transform.matrix[1][1];
	matrix._23 = transform.matrix[1][2];
	matrix._24 = transform.matrix[1][3];

	matrix._31 = transform.matrix[2][0];
	matrix._32 = transform.matrix[2][1];
	matrix._33 = transform.matrix[2][2];
	matrix._34 = transform.matrix[2][3];

	//matrix._41 = 0.0f;
	//matrix._42 = 0.0f;
	//matrix._43 = 0.0f;
	//matrix._44 = 1.0f;
#endif
}

// ----------------------------------------------------------------------

void Direct3d9::convertScaleAndTransformToMatrix(const Vector &scale, const Transform &transform, D3DMATRIX & matrix)
{
#ifdef FFP
	matrix._11 = transform.matrix[0][0] * scale.x;
	matrix._12 = transform.matrix[1][0] * scale.x;
	matrix._13 = transform.matrix[2][0] * scale.x;
	matrix._14 = 0.0f;

	matrix._21 = transform.matrix[0][1] * scale.y;
	matrix._22 = transform.matrix[1][1] * scale.y;
	matrix._23 = transform.matrix[2][1] * scale.y;
	matrix._24 = 0.0f;

	matrix._31 = transform.matrix[0][2] * scale.z;
	matrix._32 = transform.matrix[1][2] * scale.z;
	matrix._33 = transform.matrix[2][2] * scale.z;
	matrix._34 = 0.0f;

	matrix._41 = transform.matrix[0][3];
	matrix._42 = transform.matrix[1][3];
	matrix._43 = transform.matrix[2][3];
	matrix._44 = 1.0f;
#else
	matrix._11 = transform.matrix[0][0] * scale.x;
	matrix._12 = transform.matrix[0][1] * scale.x;
	matrix._13 = transform.matrix[0][2] * scale.x;
	matrix._14 = transform.matrix[0][3];

	matrix._21 = transform.matrix[1][0] * scale.y;
	matrix._22 = transform.matrix[1][1] * scale.y;
	matrix._23 = transform.matrix[1][2] * scale.y;
	matrix._24 = transform.matrix[1][3];

	matrix._31 = transform.matrix[2][0] * scale.z;
	matrix._32 = transform.matrix[2][1] * scale.z;
	matrix._33 = transform.matrix[2][2] * scale.z;
	matrix._34 = transform.matrix[2][3];

	//matrix._41 = 0.0f;
	//matrix._42 = 0.0f;
	//matrix._43 = 0.0f;
	//matrix._44 = 1.0f;
#endif
}

// ----------------------------------------------------------------------

bool Direct3d9Namespace::setMouseCursor(Texture const & mouseCursorTexture, int hotSpotX, int hotSpotY)
{
	if (GetForegroundWindow() == ms_window)
	{
		Direct3d9_TextureData const * textureData = safe_cast<Direct3d9_TextureData const *>(mouseCursorTexture.getGraphicsData());
		IDirect3DBaseTexture9 * baseTexture = textureData->getBaseTexture();

		IDirect3DSurface9 * surface = NULL;
		HRESULT hresult = static_cast<IDirect3DTexture9 *>(baseTexture)->GetSurfaceLevel(0, &surface);
		FATAL_DX_HR("Could not get top surface %s", hresult);

		hresult = ms_device->SetCursorProperties(hotSpotX, hotSpotY, surface);
		FATAL_DX_HR("Could not set cursor properties %s", hresult);
		surface->Release();

		return true;
	}

	return false;
}

// ----------------------------------------------------------------------

bool Direct3d9Namespace::showMouseCursor(bool enabled)
{
	if (GetForegroundWindow() == ms_window)
	{
		HRESULT hresult = ms_device->ShowCursor(enabled);
		FATAL_DX_HR("Could not show cursor %s", hresult);

		return true;
	}

	return false;
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::setViewport(int x, int y, int viewportWidth, int viewportHeight, float minZ, float maxZ)
{
	// cache these for device lost
	ms_viewportX            = x;
	ms_viewportY            = y;
	ms_viewportWidth        = viewportWidth;
	ms_viewportHeight       = viewportHeight;
	ms_viewportMinimumZ     = minZ;
	ms_viewportMaximumZ     = maxZ;

	// setup the viewport for this camera
	D3DVIEWPORT9  viewport;
	viewport.X      = x;
	viewport.Y      = y;
	viewport.Width  = viewportWidth;
	viewport.Height = viewportHeight;
	viewport.MinZ   = minZ;
	viewport.MaxZ   = maxZ;
	const HRESULT hresult = ms_device->SetViewport(&viewport);
	FATAL_DX_HR("SetViewport failed %s", hresult);

#ifdef VSPS
	// let the vertex shader know this information for 2d stuff
	float xOffset = ((x * 2.0f) / static_cast<float>(viewportWidth));
	float yOffset = ((y * 2.0f) / static_cast<float>(viewportHeight));
	float const viewportData[4] = { 2.0f / static_cast<float>(viewportWidth), -2.0f / static_cast<float>(viewportHeight), -1.0f - xOffset,  1.0f + yOffset };
	Direct3d9_StateCache::setVertexShaderConstants(VSCR_viewportData, viewportData, 1);
#endif

#ifdef _DEBUG
	Direct3d9_Metrics::setViewportCalls += 1;
#endif
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::setScissorRect(bool enabled, int x, int y, int width, int height)
{
#ifdef _DEBUG
	if (ms_noScissor)
		enabled = false;
#endif

	Direct3d9_StateCache::setRenderState(D3DRS_SCISSORTESTENABLE, enabled);
	if (enabled)
	{
		RECT const r = { x, y, x+width, y+height };
		HRESULT const hresult = ms_device->SetScissorRect(&r);
		FATAL_DX_HR("SetSissorRect failed %s", hresult);
	}
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::setWorldToCameraTransform(const Transform &transform, const Vector &cameraPosition)
{
#ifdef _DEBUG
	if (ms_disableAll)
		return;
#endif

	Direct3d9_LightManager::setCameraPosition(cameraPosition);

	Direct3d9::convertTransformToMatrix(transform, ms_cachedWorldToCameraMatrix);

#ifdef FFP
	// setup the world-to-camera transform
	const HRESULT hresult = ms_device->SetTransform(D3DTS_VIEW, &ms_cachedWorldToCameraMatrix);
	FATAL_DX_HR("SetTransform(view) failed %s", hresult);
#endif

#ifdef VSPS
	PaddedVector paddedPosition(cameraPosition);
	Direct3d9_StateCache::setVertexShaderConstants(VSCR_cameraPosition, &paddedPosition, 1);
	D3DXMatrixMultiply(&ms_cachedWorldToProjectionMatrix, &ms_cachedProjectionMatrix, &ms_cachedWorldToCameraMatrix);
#endif

#ifdef _DEBUG
	Direct3d9_Metrics::setTransformCalls += 1;
#endif
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::setProjectionMatrix(const GlMatrix4x4 &projectionMatrix)
{
#ifdef _DEBUG
	if (ms_disableAll)
		return;
#endif

	// setup the projection transform
#ifdef FFP
	ms_cachedProjectionMatrix._11 = projectionMatrix.matrix[0][0];
	ms_cachedProjectionMatrix._12 = projectionMatrix.matrix[1][0];
	ms_cachedProjectionMatrix._13 = projectionMatrix.matrix[2][0];
	ms_cachedProjectionMatrix._14 = projectionMatrix.matrix[3][0];

	ms_cachedProjectionMatrix._21 = projectionMatrix.matrix[0][1];
	ms_cachedProjectionMatrix._22 = projectionMatrix.matrix[1][1];
	ms_cachedProjectionMatrix._23 = projectionMatrix.matrix[2][1];
	ms_cachedProjectionMatrix._24 = projectionMatrix.matrix[3][1];

	ms_cachedProjectionMatrix._31 = projectionMatrix.matrix[0][2];
	ms_cachedProjectionMatrix._32 = projectionMatrix.matrix[1][2];
	ms_cachedProjectionMatrix._33 = projectionMatrix.matrix[2][2];
	ms_cachedProjectionMatrix._34 = projectionMatrix.matrix[3][2];

	ms_cachedProjectionMatrix._41 = projectionMatrix.matrix[0][3];
	ms_cachedProjectionMatrix._42 = projectionMatrix.matrix[1][3];
	ms_cachedProjectionMatrix._43 = projectionMatrix.matrix[2][3];
	ms_cachedProjectionMatrix._44 = projectionMatrix.matrix[3][3];
#else
	ms_cachedProjectionMatrix._11 = projectionMatrix.matrix[0][0];
	ms_cachedProjectionMatrix._12 = projectionMatrix.matrix[0][1];
	ms_cachedProjectionMatrix._13 = projectionMatrix.matrix[0][2];
	ms_cachedProjectionMatrix._14 = projectionMatrix.matrix[0][3];

	ms_cachedProjectionMatrix._21 = projectionMatrix.matrix[1][0];
	ms_cachedProjectionMatrix._22 = projectionMatrix.matrix[1][1];
	ms_cachedProjectionMatrix._23 = projectionMatrix.matrix[1][2];
	ms_cachedProjectionMatrix._24 = projectionMatrix.matrix[1][3];

	ms_cachedProjectionMatrix._31 = projectionMatrix.matrix[2][0];
	ms_cachedProjectionMatrix._32 = projectionMatrix.matrix[2][1];
	ms_cachedProjectionMatrix._33 = projectionMatrix.matrix[2][2];
	ms_cachedProjectionMatrix._34 = projectionMatrix.matrix[2][3];

	ms_cachedProjectionMatrix._41 = projectionMatrix.matrix[3][0];
	ms_cachedProjectionMatrix._42 = projectionMatrix.matrix[3][1];
	ms_cachedProjectionMatrix._43 = projectionMatrix.matrix[3][2];
	ms_cachedProjectionMatrix._44 = projectionMatrix.matrix[3][3];
#endif

#ifdef FFP
	const HRESULT hresult = ms_device->SetTransform(D3DTS_PROJECTION, &ms_cachedProjectionMatrix);
	FATAL_DX_HR("SetTransform(projection) failed %s", hresult);
#endif

#if defined(FFP) && defined(VSPS)
	D3DXMatrixMultiply(&ms_cachedWorldToProjectionMatrix, &ms_cachedWorldToCameraMatrix, &ms_cachedProjectionMatrix);
#elif defined(VSPS)
	D3DXMatrixMultiply(&ms_cachedWorldToProjectionMatrix, &ms_cachedProjectionMatrix, &ms_cachedWorldToCameraMatrix);
#endif

#ifdef _DEBUG
	Direct3d9_Metrics::setTransformCalls += 1;
#endif
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::setFog(bool enabled, float density, const PackedArgb &color)
{
#ifdef _DEBUG
	if (ms_disableAll)
		return;
#endif

	// fog enabled
	if (enabled)
	{
		ms_fogColor = color.getArgb();

		Direct3d9_StateCache::setRenderState(D3DRS_FOGENABLE, TRUE);
		Direct3d9_StateCache::setRenderState(D3DRS_FOGCOLOR, ms_fogColor);
		Direct3d9_StateCache::setRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_EXP2);

#ifdef FFP
		Direct3d9_StateCache::setRenderState(D3DRS_FOGDENSITY, density);
#endif

#ifdef VSPS
		const float fog[4] = { 0.0f, 0.0f, density, sqr(density) };
		Direct3d9_StateCache::setVertexShaderConstants(VSCR_fog, fog, 1);
#endif
	}
	else
	{
		Direct3d9_StateCache::setRenderState(D3DRS_FOGENABLE, FALSE);
	}
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::setObjectToWorldTransformAndScale(const Transform &transform, const Vector &scale)
{
#ifdef _DEBUG
	if (ms_disableAll)
		return;
#endif

	ms_transformDirty = true;
	Direct3d9::convertScaleAndTransformToMatrix(scale, transform, ms_cachedObjectToWorldMatrix);

#ifdef FFP
	const HRESULT hresult = ms_device->SetTransform(D3DTS_WORLD, &ms_cachedObjectToWorldMatrix);
	FATAL_DX_HR("SetTransform failed %s", hresult);
#endif

	Direct3d9_LightManager::setObjectToWorldTransform(transform);

#ifdef _DEBUG
	Direct3d9_Metrics::setTransformCalls += 1;
#endif
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::setGlobalTexture(Tag tag, const Texture &texture)
{
	Direct3d9_TextureData::setGlobalTexture(tag, texture);
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::releaseAllGlobalTextures()
{
	Direct3d9_TextureData::releaseAllGlobalTextures();
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::setTextureTransform(int stage, bool enabled, int dimension, bool projected, const float *transform)
{
	UNREF(stage);
	UNREF(enabled);
	UNREF(dimension);
	UNREF(projected);
	UNREF(transform);

#ifdef _DEBUG
	if (ms_disableAll)
		return;
#endif

#ifdef FFP

	DEBUG_FATAL(stage < 0 || stage > 7, ("invalid stage"));
	if (enabled)
	{
		NOT_NULL(transform);
		DEBUG_FATAL(dimension < 1 || dimension > 4, ("invalid dimension"));

		Direct3d9_StateCache::setTextureStageState(stage, D3DTSS_TEXTURETRANSFORMFLAGS, cms_textureTransformFlagsLookup[dimension-1] | (projected ? D3DTTFF_PROJECTED : 0));

		D3DMATRIX matrix;
		memcpy(&matrix, transform, sizeof(matrix));
		const HRESULT hresult = ms_device->SetTransform(cms_textureTransformLookup[stage], &matrix);
		FATAL_DX_HR("SetTransform failed %s", hresult);

#ifdef _DEBUG
		Direct3d9_Metrics::setTransformCalls += 1;
#endif
	}
	else
	{
		Direct3d9_StateCache::setTextureStageState(stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
	}
#endif

}

// ----------------------------------------------------------------------

void Direct3d9Namespace::setVertexShaderUserConstants(int index, float c0, float c1, float c2, float c3)
{
	UNREF(index);
	UNREF(c0);
	UNREF(c1);
	UNREF(c2);
	UNREF(c3);

#ifdef _DEBUG
	if (ms_disableAll)
		return;
#endif

#ifdef VSPS
	const float constants[4] = { c0, c1, c2, c3 };
	Direct3d9_StateCache::setVertexShaderConstants(VCSR_userConstant0 + index, constants, 1);
#endif
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::setPixelShaderUserConstants(VectorRgba const * constants, int count)
{
#ifdef _DEBUG
	if (ms_disableAll)
		return;
#endif

#ifdef VSPS
	Direct3d9_StateCache::setPixelShaderConstants(PSCR_userConstant, constants, count);
#else
	UNREF(constants);
	UNREF(count);
#endif
}

// ----------------------------------------------------------------------

ShaderImplementationGraphicsData *Direct3d9Namespace::createShaderImplementationGraphicsData(const ShaderImplementation &shaderImplementation)
{
	return new Direct3d9_ShaderImplementationData(shaderImplementation);
}

// ----------------------------------------------------------------------

StaticShaderGraphicsData *Direct3d9Namespace::createStaticShaderGraphicsData(const StaticShader &shader)
{
	return new Direct3d9_StaticShaderData(shader);
}

// ----------------------------------------------------------------------

ShaderImplementationPassVertexShaderGraphicsData *Direct3d9Namespace::createVertexShaderData(ShaderImplementationPassVertexShader const &vertexShader)
{
#ifdef VSPS
	return new Direct3d9_VertexShaderData(vertexShader);
#else
	UNREF(vertexShader);
	return NULL;
#endif
}

// ----------------------------------------------------------------------

ShaderImplementationPassPixelShaderProgramGraphicsData *Direct3d9Namespace::createPixelShaderProgramData(ShaderImplementationPassPixelShaderProgram const &pixelShaderProgram)
{
#ifdef VSPS
	return new Direct3d9_PixelShaderProgramData(pixelShaderProgram);
#else
	UNREF(pixelShaderProgram);
	return NULL;
#endif
}

// ----------------------------------------------------------------------

void Direct3d9::setAlphaBlendEnable(bool alphaBlendEnable)
{
	ms_alphaBlendEnable = alphaBlendEnable;
}

// ----------------------------------------------------------------------

void Direct3d9::setAlphaTestReferenceValue(uint8 alphaTestReferenceValue)
{
	ms_alphaTestReferenceValue = alphaTestReferenceValue;
}

// ----------------------------------------------------------------------

void Direct3d9::setColorWriteEnable(uint8 colorWriteEnable)
{
	ms_colorWriteEnable = colorWriteEnable;
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::setAlphaFadeOpacity(bool enabled, float opacity)
{
#ifdef VSPS

#ifdef _DEBUG
	if (ms_disableAlphaFadeOpacity)
	{
		ms_alphaFadeOpacityEnabled = false;
		ms_alphaFadeOpacity.r = 0.0f;
		ms_alphaFadeOpacity.a = 1.0f;
		ms_alphaFadeOpacityDirty = true;
	}
	else
#endif
	{
		if (ms_alphaFadeOpacityEnabled != enabled)
		{
			ms_alphaFadeOpacityEnabled = enabled;
			ms_alphaFadeOpacity.r = ms_alphaFadeOpacityEnabled ? 1.0f : 0.0f;
			ms_alphaFadeOpacityDirty = true;
		}

		if (opacity != ms_alphaFadeOpacity.a)
		{
			ms_alphaFadeOpacity.a = opacity;
			ms_alphaFadeOpacityDirty = true;
		}
	}

#else
	UNREF(opacity);
	UNREF(enabled);
#endif
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::noSetAlphaFadeOpacity(bool, float)
{
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::setLights(const stdvector<const Light*>::fwd &lightList)
{
#ifdef _DEBUG
	if (ms_disableAll)
		return;
#endif

	Direct3d9_LightManager::setLights(lightList);
}

// ----------------------------------------------------------------------

StaticVertexBufferGraphicsData *Direct3d9Namespace::createVertexBufferData(const StaticVertexBuffer &vertexBuffer)
{
	return new Direct3d9_StaticVertexBufferData(vertexBuffer);
}

// ----------------------------------------------------------------------

DynamicVertexBufferGraphicsData *Direct3d9Namespace::createVertexBufferData(const DynamicVertexBuffer &vertexBuffer)
{
	return new Direct3d9_DynamicVertexBufferData(vertexBuffer);
}

// ----------------------------------------------------------------------

VertexBufferVectorGraphicsData *Direct3d9Namespace::createVertexBufferVectorData(VertexBufferVector const & vertexBufferVector)
{
	return new Direct3d9_VertexBufferVectorData(vertexBufferVector);
}

// ----------------------------------------------------------------------

StaticIndexBufferGraphicsData *Direct3d9Namespace::createIndexBufferData(const StaticIndexBuffer &indexBuffer)
{
	return new Direct3d9_StaticIndexBufferData(indexBuffer);
}

// ----------------------------------------------------------------------

DynamicIndexBufferGraphicsData *Direct3d9Namespace::createIndexBufferData()
{
	return new Direct3d9_DynamicIndexBufferData();
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::setDynamicIndexBufferSize(int numberOfIndices)
{
	Direct3d9_DynamicIndexBufferData::setSize(numberOfIndices);
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::getOneToOneUVMapping(int textureWidth, int textureHeight, float &u0, float &v0, float &u1, float &v1)
{
	u0 = (0.0f + 0.5f) / static_cast<float>(textureWidth);
	v0 = (0.0f + 0.5f) / static_cast<float>(textureHeight);
	u1 = (static_cast<float>(textureWidth - 1) + 0.5f) / static_cast<float>(textureWidth);
	v1 = (static_cast<float>(textureHeight - 1) + 0.5f) / static_cast<float>(textureHeight);
}

// ----------------------------------------------------------------------

void Direct3d9::setVertexBuffer(const HardwareVertexBuffer & vertexBuffer)
{
#ifdef _DEBUG
	if (ms_disableAll)
		return;
	ms_currentStaticVertexBuffer = NULL;
#endif

	IDirect3DVertexBuffer9 *      vb                = 0;
	DWORD                         byteOffset        = 0;
	DWORD                         vertexSize        = 0;
	IDirect3DVertexDeclaration9 * vertexDeclaration = 0;

	switch (vertexBuffer.getType())
	{
		case HardwareVertexBuffer::T_static:
			{
				const StaticVertexBuffer *staticVertexBuffer = safe_cast<const StaticVertexBuffer *>(&vertexBuffer);
				const Direct3d9_StaticVertexBufferData *data = safe_cast<Direct3d9_StaticVertexBufferData *>(staticVertexBuffer->m_graphicsData);

#ifdef _DEBUG
				ms_currentStaticVertexBuffer = staticVertexBuffer;

				// increment the number of times this VB has been set
				++staticVertexBuffer->m_sets;

				if (data->firstTimeUsedThisFrame())
				{
					Direct3d9_Metrics::setVertexBufferUniqueVertexBuffers += 1;
					Direct3d9_Metrics::vertexBufferMemoryUsed += data->getMemorySize();
				}
#endif

				vb  = data->getVertexBuffer();
				vertexSize = data->getVertexSize();
				vertexDeclaration = data->getVertexDeclaration();

				// remember the number of vertices for the draw calls
				ms_sliceNumberOfVertices = staticVertexBuffer->getNumberOfVertices();
				ms_sliceFirstVertex      = 0;
			}
			break;

		case HardwareVertexBuffer::T_dynamic:
			{
				const DynamicVertexBuffer *dynamicVertexBuffer = safe_cast<const DynamicVertexBuffer *>(&vertexBuffer);
				const Direct3d9_DynamicVertexBufferData *data = safe_cast<Direct3d9_DynamicVertexBufferData *>(dynamicVertexBuffer->m_graphicsData);

				vb  = data->getVertexBuffer();
				vertexSize = data->getVertexSize();
				vertexDeclaration = data->getVertexDeclaration();

				// remember the number of vertices for the draw calls
				ms_sliceNumberOfVertices = data->getNumberOfVertices();
				ms_sliceFirstVertex      = data->getOffset();
			}
			break;

		default:
			DEBUG_FATAL(true, ("Unknown VB type"));
	}

	// set the VB declaration
	Direct3d9_StateCache::setVertexDeclaration(vertexDeclaration);

	// set the vertex buffer stream
	Direct3d9_StateCache::setStreamSource(0, vb, byteOffset, vertexSize);
	for (int i = 1; i < ms_lastVertexBufferCount; ++i)
		Direct3d9_StateCache::setStreamSource(i, 0, 0, 0);
	ms_lastVertexBufferCount = 1;

#ifdef _DEBUG
	Direct3d9_Metrics::setVertexBufferCalls += 1;
#endif
}

// ----------------------------------------------------------------------

void Direct3d9::setVertexBufferVector(const VertexBufferVector & vertexBufferVector)
{
#ifdef _DEBUG
	if (ms_disableAll)
		return;

	ms_currentStaticVertexBuffer = NULL;
#endif

	ms_sliceFirstVertex = 0;

	int stream = 0;
	bool staticBufferFound=false;
	bool dynamicBufferFound=false;
	VertexBufferVector::VertexBufferList const & vertexBufferList = *(vertexBufferVector.m_vertexBufferList);
	VertexBufferVector::VertexBufferList::const_iterator const iEnd = vertexBufferList.end();
	for (VertexBufferVector::VertexBufferList::const_iterator i = vertexBufferList.begin(); i != iEnd; ++i, ++stream)
	{
		IDirect3DVertexBuffer9  *vb         = 0;
		DWORD                    byteOffset = 0;
		DWORD                    vertexSize = 0;

		HardwareVertexBuffer const & vertexBuffer = **i;

		switch (vertexBuffer.getType())
		{
			case HardwareVertexBuffer::T_static:
				{
					staticBufferFound=true;

					const StaticVertexBuffer *staticVertexBuffer = safe_cast<const StaticVertexBuffer *>(&vertexBuffer);
					const Direct3d9_StaticVertexBufferData *data = safe_cast<Direct3d9_StaticVertexBufferData *>(staticVertexBuffer->m_graphicsData);

	#ifdef _DEBUG
					if (!ms_currentStaticVertexBuffer)
						ms_currentStaticVertexBuffer = staticVertexBuffer;

					// increment the number of times this VB has been set
					++staticVertexBuffer->m_sets;

					if (data->firstTimeUsedThisFrame())
					{
						Direct3d9_Metrics::setVertexBufferUniqueVertexBuffers += 1;
						Direct3d9_Metrics::vertexBufferMemoryUsed += data->getMemorySize();
					}

					if (dynamicBufferFound)
					{
						DEBUG_FATAL(ms_sliceNumberOfVertices!=staticVertexBuffer->getNumberOfVertices(),
							("Error - VertexBufferVector has mixed lengths"));

						DEBUG_FATAL(ms_sliceFirstVertex!=0,
							("Error - VertexBufferVector has mixed offsets"));
					}
	#endif

					vb  = data->getVertexBuffer();
					vertexSize = data->getVertexSize();

					// remember the number of vertices for the draw calls
					ms_sliceNumberOfVertices = staticVertexBuffer->getNumberOfVertices();
					ms_sliceFirstVertex      = 0;
				}
				break;

			case HardwareVertexBuffer::T_dynamic:
				{
					dynamicBufferFound=true;

					const DynamicVertexBuffer *dynamicVertexBuffer = safe_cast<const DynamicVertexBuffer *>(&vertexBuffer);
					const Direct3d9_DynamicVertexBufferData *data = safe_cast<Direct3d9_DynamicVertexBufferData *>(dynamicVertexBuffer->m_graphicsData);

					vb  = data->getVertexBuffer();
					vertexSize = data->getVertexSize();

	#ifdef _DEBUG
					if (staticBufferFound)
					{
						DEBUG_FATAL(ms_sliceNumberOfVertices!=data->getNumberOfVertices(),
							("Error - VertexBufferVector has mixed lengths"));

						if (!ms_supportsStreamOffsets)
						{
							DEBUG_FATAL(ms_sliceFirstVertex!=data->getOffset(),
								("Error - VertexBufferVector has mixed offsets"));
						}
					}
	#endif

					// remember the number of vertices for the draw calls
					ms_sliceNumberOfVertices = data->getNumberOfVertices();
					if (ms_supportsStreamOffsets)
					{
						byteOffset = data->getOffset() * vertexSize;
					}
					else
					{
						ms_sliceFirstVertex = data->getOffset();
					}
				}
				break;

			default:
				DEBUG_FATAL(true, ("Unknown VB type"));
		}

		Direct3d9_StateCache::setStreamSource(stream, vb, byteOffset, vertexSize);
	}

	Direct3d9_VertexBufferVectorData *data = safe_cast<Direct3d9_VertexBufferVectorData *>(vertexBufferVector.m_graphicsData);
	HRESULT result = ms_device->SetVertexDeclaration(data->getVertexDeclaration());
	FATAL_DX_HR("SetVertexDeclaration failed %s", result);

	// clear the remaining vertex buffer streams
	{
		for (int i = stream; i < ms_lastVertexBufferCount; ++i)
			Direct3d9_StateCache::setStreamSource(i, 0, 0, 0);
		ms_lastVertexBufferCount = stream;
	}

#ifdef _DEBUG
	Direct3d9_Metrics::setVertexBufferCalls += 1;
#endif
}

// ----------------------------------------------------------------------

void Direct3d9::setIndexBuffer(const HardwareIndexBuffer &indexBuffer)
{
#ifdef _DEBUG
	if (ms_disableAll)
		return;
#endif

	// get the index buffer
	if (indexBuffer.getType() == HardwareIndexBuffer::T_static)
	{
		const StaticIndexBuffer *sib = safe_cast<const StaticIndexBuffer *>(&indexBuffer);
		const Direct3d9_StaticIndexBufferData *ib = safe_cast<const Direct3d9_StaticIndexBufferData *>(sib->m_graphicsData);
		NOT_NULL(ib);

#ifdef _DEBUG
		if (ib->firstTimeUsedThisFrame())
		{
			Direct3d9_Metrics::setIndexBufferUniqueIndexBuffers += 1;
			Direct3d9_Metrics::indexBufferMemoryUsed += ib->getMemorySize();
		}
#endif

		// remember stuff about the indices
		ms_savedIndexBuffer     = ib->getIndexBuffer();
		ms_sliceNumberOfIndices = sib->getNumberOfIndices();
		ms_sliceFirstIndex      = 0;
	}
	else
	{
		const DynamicIndexBuffer *dib = safe_cast<const DynamicIndexBuffer *>(&indexBuffer);
		const Direct3d9_DynamicIndexBufferData *ib = safe_cast<const Direct3d9_DynamicIndexBufferData *>(dib->m_graphicsData);
		NOT_NULL(ib);

		// remember stuff about the indices
		ms_savedIndexBuffer     = Direct3d9_DynamicIndexBufferData::getIndexBuffer();
		ms_sliceNumberOfIndices = ib->getNumberOfIndices();
		ms_sliceFirstIndex      = ib->getOffset();
	}

	Direct3d9_StateCache::setIndexBuffer(ms_savedIndexBuffer);

#ifdef _DEBUG
	Direct3d9_Metrics::setIndexBufferCalls += 1;
#endif
}

// ----------------------------------------------------------------------

#ifdef _DEBUG

inline bool Direct3d9Namespace::shouldDraw(bool indexed, int primitiveCount)
{
	if (indexed && ms_noIndexedPrimitives)
		return false;

	if (!indexed && ms_noNonIndexedPrimitives)
		return false;

#if defined(FFP) && defined(VSPS)
	if (ms_usingVertexShader)
	{
		if (ms_noRenderVertexShaders)
			return false;
	}
	else
	{
		if (ms_noRenderFixedFunctionPipeline)
			return false;
	}
#endif

	if (ms_disableAll)
		return false;
	if (ms_disablePass0Plus && ms_currentPassIndex >= 0)
		return false;
	if (ms_disablePass1Plus && ms_currentPassIndex >= 1)
		return false;
	if (ms_disablePass2Plus && ms_currentPassIndex >= 2)
		return false;
	if (ms_disablePass3Plus && ms_currentPassIndex >= 3)
		return false;

	if (ms_minimumDrawPrimitives2 && primitiveCount < 2)
		return false;
	if (ms_minimumDrawPrimitives4 && primitiveCount < 4)
		return false;
	if (ms_minimumDrawPrimitives8 && primitiveCount < 8)
		return false;
	if (ms_minimumDrawPrimitives16 && primitiveCount < 16)
		return false;
	if (ms_minimumDrawPrimitives32 && primitiveCount < 32)
		return false;
	if (ms_minimumDrawPrimitives64 && primitiveCount < 64)
		return false;
	if (ms_minimumDrawPrimitives128 && primitiveCount < 128)
		return false;
	if (ms_minimumDrawPrimitives256 && primitiveCount < 256)
		return false;
	if (ms_minimumDrawPrimitives512 && primitiveCount < 512)
		return false;

	return true;
}

#endif

// ----------------------------------------------------------------------

inline bool Direct3d9::drawPrimitive()
{
	DEBUG_FATAL(!ms_currentShader, ("No shader set -- did you create a new shader and not another?"));

#if DEBUG_LEVEL == DEBUG_LEVEL_DEBUG
	if (ms_drawCall < ms_allowDrawCallBegin || ms_drawCall > ms_allowDrawCallEnd)
	{
		++ms_drawCall;
		return false;
	}
	++ms_drawCall;
#endif

	Direct3d9_LightManager::selectLights();

#if defined(FFP) && defined(VSPS)
	if (ms_usingVertexShader)
#endif
#ifdef VSPS
	{
#ifdef FFP
		Direct3d9_StateCache::resetTextureCoordinateIndices();
#endif

		// @todo work around bug in nvidia drivers when using vertex shaders with fog
		Direct3d9_StateCache::setRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_NONE);

		if (ms_transformDirty)
		{
			D3DXMATRIX matrices[2];

#ifdef FFP
			D3DXMatrixMultiplyTranspose(matrices+0, &ms_cachedObjectToWorldMatrix, &ms_cachedWorldToProjectionMatrix);
			D3DXMatrixTranspose(matrices+1, &ms_cachedObjectToWorldMatrix);
#else
			D3DXMatrixMultiply(matrices+0, &ms_cachedWorldToProjectionMatrix, &ms_cachedObjectToWorldMatrix);
			matrices[1] = ms_cachedObjectToWorldMatrix;
#endif

			Direct3d9_StateCache::setVertexShaderConstants(VSCR_objectWorldCameraProjectionMatrix, matrices, 8);
			ms_transformDirty = false;
		}
	}
#endif
#if defined(FFP) && defined(VSPS)
	else
	{
		// @todo work around bug in nvidia drivers when using vertex shaders with fog
		Direct3d9_StateCache::setRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_EXP2);
	}
#endif

#ifdef _DEBUG
	Direct3d9_Metrics::vertices += ms_sliceNumberOfVertices;
#endif

#ifdef VSPS
	if (ms_alphaFadeOpacityEnabled)
	{
		if (ms_alphaBlendEnable)
			Direct3d9_StateCache::setRenderState(D3DRS_COLORWRITEENABLE, ms_colorWriteEnable);
		else
			Direct3d9_StateCache::setRenderState(D3DRS_COLORWRITEENABLE, ms_colorWriteEnable & ~D3DCOLORWRITEENABLE_ALPHA);
		Direct3d9_StateCache::setRenderState(D3DRS_ALPHABLENDENABLE, true);
		Direct3d9_StateCache::setRenderState(D3DRS_ALPHAREF, static_cast<DWORD>(static_cast<float>(ms_alphaTestReferenceValue) * ms_alphaFadeOpacity.a));
	}
	else
#endif
	{
		Direct3d9_StateCache::setRenderState(D3DRS_COLORWRITEENABLE, ms_colorWriteEnable);
		Direct3d9_StateCache::setRenderState(D3DRS_ALPHABLENDENABLE, ms_alphaBlendEnable);
		Direct3d9_StateCache::setRenderState(D3DRS_ALPHAREF, ms_alphaTestReferenceValue);
	}

#ifdef VSPS
	if (ms_alphaFadeOpacityDirty)
	{
	// This is always set as part of the light structure in Direct3d9_LightManager.cpp
	//		Direct3d9_StateCache::setPixelShaderConstants(PSCR_alphaFadeOpacity, &ms_alphaFadeOpacity, 1);
	}
#endif

	return true;
}

// ----------------------------------------------------------------------

inline void Direct3d9::drawPrimitive(D3DPRIMITIVETYPE primitiveType, int startVertex, int primitiveCount)
{
#ifdef _DEBUG
	Direct3d9_Metrics::drawPrimitiveCalls += 1;
	Direct3d9_Metrics::perPassDrawPrimitiveCalls[ms_currentPassIndex] += 1;
#endif

	if (drawPrimitive())
	{
#if defined(_DEBUG) && defined(VSPS)
		if (ms_badVertexBufferVertexShaderCombination && *ms_badVertexBufferVertexShaderCombination)
			setStaticShader(*ms_badVertexShaderStaticShader, 0);
#endif

		// draw the primitives
		HRESULT hresult = ms_device->DrawPrimitive(primitiveType, ms_sliceFirstVertex + startVertex, primitiveCount);

#ifdef _DEBUG

#ifdef VSPS
		if (FAILED(hresult))
#ifdef FFP
			if (ms_usingVertexShader)
#endif
			{
				StaticShader const * badShader = ms_currentShader;

				// attempt to draw again with the debug shader
				setStaticShader(*ms_badVertexShaderStaticShader, 0);
				hresult = ms_device->DrawPrimitive(primitiveType, ms_sliceFirstVertex + startVertex, primitiveCount);

#ifdef _DEBUG
				if (!FAILED(hresult))
				{
					if (ms_badVertexBufferVertexShaderCombination)
						*ms_badVertexBufferVertexShaderCombination = true;
					DEBUG_WARNING(true, ("Missing vertex components in '%s' on shader '%s'", ms_appearanceName ? ms_appearanceName : "unknown", badShader->getShaderTemplate().getName().getString()));
				}
#endif
			}
#endif

		FATAL_DX_HR("DrawPrimitive failed %s", hresult);
#else
		UNREF(hresult);
#endif
	}
}

// ----------------------------------------------------------------------

inline void Direct3d9::drawIndexedPrimitive(D3DPRIMITIVETYPE primitiveType, int baseIndex, int minimumVertexIndex, int numberOfVertices, int startIndex, int primitiveCount, int numberOfIndices)
{
#ifdef _DEBUG
	Direct3d9_Metrics::indices += numberOfIndices;
	Direct3d9_Metrics::drawIndexedPrimitiveCalls += 1;
	Direct3d9_Metrics::perPassDrawIndexedPrimitiveCalls[ms_currentPassIndex] += 1;
#else
	UNREF(numberOfIndices);
#endif

	if (drawPrimitive())
	{
		DEBUG_FATAL(!ms_savedIndexBuffer, ("No saved index buffer"));

#if defined(_DEBUG) && defined(VSPS)
		if (ms_badVertexBufferVertexShaderCombination && *ms_badVertexBufferVertexShaderCombination)
			setStaticShader(*ms_badVertexShaderStaticShader, 0);
#endif

		// draw the primitives
		HRESULT hresult = ms_device->DrawIndexedPrimitive(primitiveType, ms_sliceFirstVertex + baseIndex, minimumVertexIndex, numberOfVertices, ms_sliceFirstIndex + startIndex, primitiveCount);

#ifdef _DEBUG
#ifdef VSPS
		if (FAILED(hresult))
#ifdef FFP
			if (ms_usingVertexShader)
#endif
			{
				StaticShader const * badShader = ms_currentShader;

				// attempt to draw again with the debug shader
				setStaticShader(*ms_badVertexShaderStaticShader, 0);
				hresult = ms_device->DrawIndexedPrimitive(primitiveType, ms_sliceFirstVertex + baseIndex, minimumVertexIndex, numberOfVertices, ms_sliceFirstIndex + startIndex, primitiveCount);

				if (!FAILED(hresult))
				{
					if (ms_badVertexBufferVertexShaderCombination)
						*ms_badVertexBufferVertexShaderCombination = true;
					DEBUG_WARNING(true, ("Missing vertex components in '%s' on shader '%s'", ms_appearanceName ? ms_appearanceName : "unknown", badShader->getShaderTemplate().getName().getString()));
				}
			}
#endif

		FATAL_DX_HR("DrawPrimitive failed %s", hresult);
#else
		UNREF(hresult);
#endif
	}
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::drawPointList()
{
#ifdef _DEBUG
	if (!shouldDraw(false, ms_sliceNumberOfVertices))
		return;

	Direct3d9_Metrics::points += ms_sliceNumberOfVertices;
#endif

	Direct3d9::drawPrimitive(D3DPT_POINTLIST, 0, ms_sliceNumberOfVertices);
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::drawLineList()
{
#ifdef _DEBUG
	if (!shouldDraw(false, ms_sliceNumberOfVertices /2))
		return;

	Direct3d9_Metrics::lines += (ms_sliceNumberOfVertices / 2);
#endif

	Direct3d9::drawPrimitive(D3DPT_LINELIST, 0, ms_sliceNumberOfVertices / 2);
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::drawLineStrip()
{
#ifdef _DEBUG
	if (!shouldDraw(false, ms_sliceNumberOfVertices - 1))
		return;

	Direct3d9_Metrics::lines += (ms_sliceNumberOfVertices - 1);
#endif

	Direct3d9::drawPrimitive(D3DPT_LINESTRIP, 0, ms_sliceNumberOfVertices - 1);
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::drawTriangleList()
{
#ifdef _DEBUG
	if (!shouldDraw(false, ms_sliceNumberOfVertices / 3))
		return;

	Direct3d9_Metrics::triangles += (ms_sliceNumberOfVertices / 3);
	Direct3d9_Metrics::perPassTriangles[ms_currentPassIndex] += (ms_sliceNumberOfVertices / 3);
#endif

	Direct3d9::drawPrimitive(D3DPT_TRIANGLELIST, 0, ms_sliceNumberOfVertices / 3);
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::drawTriangleStrip()
{
#ifdef _DEBUG
	if (!shouldDraw(false, ms_sliceNumberOfVertices - 2))
		return;

	Direct3d9_Metrics::triangles += (ms_sliceNumberOfVertices - 2);
	Direct3d9_Metrics::perPassTriangles[ms_currentPassIndex] += (ms_sliceNumberOfVertices - 2);
#endif

	Direct3d9::drawPrimitive(D3DPT_TRIANGLESTRIP, 0, ms_sliceNumberOfVertices - 2);
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::drawTriangleFan()
{
#ifdef _DEBUG
	if (!shouldDraw(false, ms_sliceNumberOfVertices - 2))
		return;

	Direct3d9_Metrics::triangles += (ms_sliceNumberOfVertices - 2);
	Direct3d9_Metrics::perPassTriangles[ms_currentPassIndex] += (ms_sliceNumberOfVertices - 2);
#endif

	Direct3d9::drawPrimitive(D3DPT_TRIANGLEFAN, 0, ms_sliceNumberOfVertices - 2);
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::drawQuadList()
{
	int const numberOfQuads = ms_sliceNumberOfVertices / 4;
	int const numberOfTriangles = numberOfQuads * 2;
#ifdef _DEBUG
	if (!shouldDraw(true, numberOfTriangles))
		return;

	Direct3d9_Metrics::triangles += numberOfTriangles;
	Direct3d9_Metrics::perPassTriangles[ms_currentPassIndex] += numberOfTriangles;
#endif

	// make sure the index buffer is large enough, and set it
	if (numberOfQuads > ms_quadListIndexBufferNumberOfQuads)
		resizeQuadListIndexBuffer(numberOfQuads);
	Direct3d9::setIndexBuffer(*ms_quadListIndexBuffer);

	Direct3d9::drawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, ms_sliceNumberOfVertices, 0, numberOfTriangles, numberOfTriangles * 3);
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::drawIndexedPointList()
{
#ifdef _DEBUG
	if (!shouldDraw(true, ms_sliceNumberOfIndices))
		return;

	Direct3d9_Metrics::points += ms_sliceNumberOfIndices;
#endif

	Direct3d9::drawIndexedPrimitive(D3DPT_POINTLIST, 0, 0, ms_sliceNumberOfVertices, 0, ms_sliceNumberOfIndices, ms_sliceNumberOfIndices);
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::drawIndexedLineList()
{
#ifdef _DEBUG
	if (!shouldDraw(true, ms_sliceNumberOfIndices / 2))
		return;

	Direct3d9_Metrics::lines += (ms_sliceNumberOfIndices / 2);
#endif

	Direct3d9::drawIndexedPrimitive(D3DPT_LINELIST, 0, 0, ms_sliceNumberOfVertices, 0, ms_sliceNumberOfIndices / 2, ms_sliceNumberOfIndices);
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::drawIndexedLineStrip()
{
#ifdef _DEBUG
	if (!shouldDraw(true, ms_sliceNumberOfIndices - 1))
		return;

	Direct3d9_Metrics::lines += (ms_sliceNumberOfIndices - 1);
#endif

	Direct3d9::drawIndexedPrimitive(D3DPT_LINESTRIP, 0, 0, ms_sliceNumberOfVertices, 0, ms_sliceNumberOfIndices - 1, ms_sliceNumberOfIndices);
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::drawIndexedTriangleList()
{
#ifdef _DEBUG
	if (!shouldDraw(true, ms_sliceNumberOfIndices / 3))
		return;

	Direct3d9_Metrics::triangles += (ms_sliceNumberOfIndices / 3);
	Direct3d9_Metrics::perPassTriangles[ms_currentPassIndex] += (ms_sliceNumberOfIndices / 3);
#endif
	Direct3d9::drawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, ms_sliceNumberOfVertices, 0, ms_sliceNumberOfIndices / 3, ms_sliceNumberOfIndices);
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::drawIndexedTriangleStrip()
{
#ifdef _DEBUG
	if (!shouldDraw(true, ms_sliceNumberOfIndices - 2))
		return;

	Direct3d9_Metrics::triangles += (ms_sliceNumberOfIndices - 2);
	Direct3d9_Metrics::perPassTriangles[ms_currentPassIndex] += (ms_sliceNumberOfIndices - 2);
#endif
	Direct3d9::drawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, 0, ms_sliceNumberOfVertices, 0, ms_sliceNumberOfIndices - 2, ms_sliceNumberOfIndices);
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::drawIndexedTriangleFan()
{
#ifdef _DEBUG
	if (!shouldDraw(true, ms_sliceNumberOfIndices - 2))
		return;

	Direct3d9_Metrics::triangles += (ms_sliceNumberOfIndices - 2);
	Direct3d9_Metrics::perPassTriangles[ms_currentPassIndex] += (ms_sliceNumberOfIndices - 2);
#endif
	Direct3d9::drawIndexedPrimitive(D3DPT_TRIANGLEFAN, 0, 0, ms_sliceNumberOfVertices, 0, ms_sliceNumberOfIndices - 2, ms_sliceNumberOfIndices);
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::drawPointList(int startVertex, int primitiveCount)
{
#ifdef _DEBUG
	if (!shouldDraw(false, primitiveCount))
		return;

	Direct3d9_Metrics::points += primitiveCount;
#endif
	Direct3d9::drawPrimitive(D3DPT_POINTLIST, startVertex, primitiveCount);
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::drawLineList(int startVertex, int primitiveCount)
{
#ifdef _DEBUG
	if (!shouldDraw(false, primitiveCount))
		return;

	Direct3d9_Metrics::lines += primitiveCount;
#endif
	Direct3d9::drawPrimitive(D3DPT_LINELIST, startVertex, primitiveCount);
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::drawLineStrip(int startVertex, int primitiveCount)
{
#ifdef _DEBUG
	if (!shouldDraw(false, primitiveCount))
		return;

	Direct3d9_Metrics::lines += primitiveCount;
#endif
	Direct3d9::drawPrimitive(D3DPT_LINESTRIP, startVertex, primitiveCount);
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::drawTriangleList(int startVertex, int primitiveCount)
{
#ifdef _DEBUG
	if (!shouldDraw(false, primitiveCount))
		return;

	Direct3d9_Metrics::triangles += primitiveCount;
	Direct3d9_Metrics::perPassTriangles[ms_currentPassIndex] += primitiveCount;
#endif
	Direct3d9::drawPrimitive(D3DPT_TRIANGLELIST, startVertex, primitiveCount);
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::drawTriangleStrip(int startVertex, int primitiveCount)
{
#ifdef _DEBUG
	if (!shouldDraw(false, primitiveCount))
		return;

	Direct3d9_Metrics::triangles += primitiveCount;
	Direct3d9_Metrics::perPassTriangles[ms_currentPassIndex] += primitiveCount;
#endif
	Direct3d9::drawPrimitive(D3DPT_TRIANGLESTRIP, startVertex, primitiveCount);
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::drawTriangleFan(int startVertex, int primitiveCount)
{
#ifdef _DEBUG
	if (!shouldDraw(false, primitiveCount))
		return;

	Direct3d9_Metrics::triangles += primitiveCount;
	Direct3d9_Metrics::perPassTriangles[ms_currentPassIndex] += primitiveCount;
#endif
	Direct3d9::drawPrimitive(D3DPT_TRIANGLEFAN, startVertex, primitiveCount);
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::drawIndexedPointList(int baseIndex, int minimumVertexIndex, int numberOfVertices, int startIndex, int primitiveCount)
{
#ifdef _DEBUG
	if (!shouldDraw(true, primitiveCount))
		return;

	Direct3d9_Metrics::points += primitiveCount;
#endif
	Direct3d9::drawIndexedPrimitive(D3DPT_POINTLIST, baseIndex, minimumVertexIndex, numberOfVertices, startIndex, primitiveCount, primitiveCount);
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::drawIndexedLineList(int baseIndex, int minimumVertexIndex, int numberOfVertices, int startIndex, int primitiveCount)
{
#ifdef _DEBUG
	if (!shouldDraw(true, primitiveCount))
		return;

	Direct3d9_Metrics::lines += primitiveCount;
#endif
	Direct3d9::drawIndexedPrimitive(D3DPT_LINELIST, baseIndex, minimumVertexIndex, numberOfVertices, startIndex, primitiveCount, primitiveCount * 2);
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::drawIndexedLineStrip(int baseIndex, int minimumVertexIndex, int numberOfVertices, int startIndex, int primitiveCount)
{
#ifdef _DEBUG
	if (!shouldDraw(true, primitiveCount))
		return;

	Direct3d9_Metrics::lines += primitiveCount;
#endif
	Direct3d9::drawIndexedPrimitive(D3DPT_LINESTRIP, baseIndex, minimumVertexIndex, numberOfVertices, startIndex, primitiveCount, primitiveCount+1);
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::drawIndexedTriangleList(int baseIndex, int minimumVertexIndex, int numberOfVertices, int startIndex, int primitiveCount)
{
#ifdef _DEBUG
	if (!shouldDraw(true, primitiveCount))
		return;

	Direct3d9_Metrics::triangles += primitiveCount;
	Direct3d9_Metrics::perPassTriangles[ms_currentPassIndex] += primitiveCount;
#endif
	Direct3d9::drawIndexedPrimitive(D3DPT_TRIANGLELIST, baseIndex, minimumVertexIndex, numberOfVertices, startIndex, primitiveCount, primitiveCount * 3);
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::drawIndexedTriangleStrip(int baseIndex, int minimumVertexIndex, int numberOfVertices, int startIndex, int primitiveCount)
{
#ifdef _DEBUG
	if (!shouldDraw(true, primitiveCount))
		return;

	Direct3d9_Metrics::triangles += primitiveCount;
	Direct3d9_Metrics::perPassTriangles[ms_currentPassIndex] += primitiveCount;
#endif
	Direct3d9::drawIndexedPrimitive(D3DPT_TRIANGLESTRIP, baseIndex, minimumVertexIndex, numberOfVertices, startIndex, primitiveCount, primitiveCount + 2);
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::drawIndexedTriangleFan(int baseIndex, int minimumVertexIndex, int numberOfVertices, int startIndex, int primitiveCount)
{
#ifdef _DEBUG
	if (!shouldDraw(true, primitiveCount))
		return;

	Direct3d9_Metrics::triangles += primitiveCount;
	Direct3d9_Metrics::perPassTriangles[ms_currentPassIndex] += primitiveCount;
#endif
	Direct3d9::drawIndexedPrimitive(D3DPT_TRIANGLEFAN, baseIndex, minimumVertexIndex, numberOfVertices, startIndex, primitiveCount, primitiveCount + 2);
}

// ----------------------------------------------------------------------

int Direct3d9Namespace::getMaximumVertexBufferStreamCount()
{
	if (ms_deviceCaps.MaxStreams < 1)
		return 1;

	return static_cast<int>(ms_deviceCaps.MaxStreams);
}

// ----------------------------------------------------------------------

void *Direct3d9::getTemporaryBuffer(int size)
{
	if (ms_temporaryBufferSize < size)
	{
		ms_temporaryBufferSize = size;
		delete ms_temporaryBuffer;
		ms_temporaryBuffer = operator new(ms_temporaryBufferSize);
	}

	return ms_temporaryBuffer;
}

#ifdef VSPS
VectorRgba const & Direct3d9::getAlphaFadeAndBloomSettings()
{
	return ms_alphaFadeOpacity;
}
#endif

// ----------------------------------------------------------------------

void Direct3d9Namespace::getOtherAdapterRects(std::vector<RECT> &otherAdapterRects)
{
	otherAdapterRects.clear();

	uint const numberOfAdapters = ms_direct3d->GetAdapterCount();
	for (uint i = 0; i < numberOfAdapters; ++i)
		if (i != ms_adapter)
		{
			// get the area of the monitor
			HMONITOR monitor = ms_direct3d->GetAdapterMonitor(i);
			MONITORINFO monitorInfo;
			Zero(monitorInfo);
			monitorInfo.cbSize = sizeof(monitorInfo);
			GetMonitorInfo(monitor, &monitorInfo);

			// put it on the list
			otherAdapterRects.push_back(monitorInfo.rcMonitor);
		}
}

// ----------------------------------------------------------------------

void Direct3d9::startPerformanceTimer()
{
	if (ms_performanceTimer)
		ms_performanceTimer->start();
}

// ----------------------------------------------------------------------

float Direct3d9::stopPerformanceTimer()
{
	if (!ms_performanceTimer)
			return 0.0f;

	ms_performanceTimer->stop();
	return ms_performanceTimer->getElapsedTime();
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::optimizeIndexBuffer(WORD *indices, int numIndices)
{
	ID3DXMesh* pD3DXMesh;
	HRESULT hRslt;
	WORD* indexData = NULL;

	if(numIndices == 0 || !indices)
		return;

	DEBUG_FATAL(numIndices % 3 != 0, ("Fatal: can't optimize a buffer that doesn't contain triangle face data"));
	hRslt = D3DXCreateMeshFVF(numIndices / 3,
							  numIndices,
							  D3DXMESH_SYSTEMMEM,
							  D3DFVF_XYZ,
							  ms_device,
							  &pD3DXMesh);

	if(hRslt != D3D_OK)
	{
		WARNING_DEBUG_FATAL(true, ("Could not optimize index buffer"));
		return;
	}

	hRslt = pD3DXMesh->LockIndexBuffer(0, (void**)&indexData);

	if(hRslt != D3D_OK)
	{
		WARNING_DEBUG_FATAL(true, ("Could not optimize index buffer"));
		return;
	}

	memcpy(indexData, indices, sizeof(WORD) * numIndices);

	pD3DXMesh->UnlockIndexBuffer();

	if(hRslt != D3D_OK)
	{
		WARNING_DEBUG_FATAL(true, ("Could not optimize index buffer"));
		return;
	}

	DWORD* adjacencyTable = new DWORD[numIndices];
	pD3DXMesh->GenerateAdjacency(0.0f, adjacencyTable);
	for(int j = 0; j < numIndices; j++)
	{
		adjacencyTable[j] = 0xFFFFFFFF;
	}

	pD3DXMesh->OptimizeInplace(D3DXMESHOPT_IGNOREVERTS | D3DXMESHOPT_VERTEXCACHE,
		adjacencyTable, NULL, NULL, NULL);

	hRslt = pD3DXMesh->LockIndexBuffer(0, (void**)&indexData);

	if(hRslt != D3D_OK)
	{
		WARNING_DEBUG_FATAL(true, ("Could not optimize index buffer"));
		return;
	}

	memcpy(indices, indexData, sizeof(WORD) * numIndices);
	pD3DXMesh->UnlockIndexBuffer();
	delete [] adjacencyTable;
	pD3DXMesh->Release();
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::setBloomEnabled(bool enabled)
{
#ifdef VSPS
	ms_alphaFadeOpacity.g = enabled ? 1.0f : 0.0f;
	ms_alphaFadeOpacityDirty = true;
#else
	UNREF(enabled);
#endif
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::pixSetMarker(WCHAR const * markerName)
{
#if (D3D_SDK_VERSION & 0x7fffffff) >= 32
	D3DPERF_SetMarker(0xffff00ff, markerName);
#else
	UNREF(markerName);
#endif
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::pixBeginEvent(WCHAR const * eventName)
{
#if (D3D_SDK_VERSION & 0x7fffffff) >= 32
	D3DPERF_BeginEvent(0xffffffff, eventName);
#else
	UNREF(eventName);
#endif
}

// ----------------------------------------------------------------------

void Direct3d9Namespace::pixEndEvent(WCHAR const *)
{
#if (D3D_SDK_VERSION & 0x7fffffff) >= 32
	D3DPERF_EndEvent();
#endif
}

// ----------------------------------------------------------------------

int Direct3d9Namespace::nextPowerOfTwo(int x)
{
	x--;
	x |= (x >> 1);
	x |= (x >> 2);
	x |= (x >> 4);
	x |= (x >> 8);
	x |= (x >> 16);
	x++;
	return x;
}

// ----------------------------------------------------------------------

bool Direct3d9Namespace::writeImage(char const * file, int const width, int const height, int const pitch, int const * pixelsARGB, bool const alphaExtend, Gl_imageFormat const imageFormat, Rectangle2d const * subRect)
{
	FATAL(file == NULL, ("Invalid file pointer."));
	FATAL(width <= 0 || width > 2048, ("Invalid width %d.", width));
	FATAL(height <= 0  || height > 2048, ("Invalid height %d.", height));
	FATAL(pitch < width, ("Invalid pitch %d.", pitch));
	FATAL(pixelsARGB == NULL, ("Invalid source pixel pointer."));

	if (subRect) 
	{
		FATAL(subRect->getWidth() <= 0, ("Invalid sub-rect width %d.", subRect->getWidth()));
		FATAL(subRect->getHeight() <= 0, ("Invalid sub-rect height %d.", subRect->getHeight()));
	}

	IDirect3DTexture9 * texturePointer = NULL;

	int const textureWidth = static_cast<int>(subRect ? subRect->getWidth() : width);
	int const textureHeight = static_cast<int>(subRect ? subRect->getHeight() : height);

	int const textureWidth2 = nextPowerOfTwo(textureWidth);
	int const textureHeight2 = nextPowerOfTwo(textureHeight);
	
	if (D3D_OK == Direct3d9::getDevice()->CreateTexture(textureWidth2, textureHeight2, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &texturePointer, 0))
	{
		D3DLOCKED_RECT lockedRect;
		HRESULT hresult(0);
	
		hresult = texturePointer->LockRect(0, &lockedRect, NULL, D3DLOCK_DISCARD);
		FATAL_DX_HR("LockRect failed %s", hresult);

		int * lockedPixels = reinterpret_cast<int *>(lockedRect.pBits);
		int const sourcePitch = pitch / sizeof(int);
		int const destPitch = lockedRect.Pitch  / sizeof(int);

		int const subLeft = subRect ? static_cast<int>(subRect->x0) : 0;
		int const subTop = subRect ? static_cast<int>(subRect->y0) : 0;
		
		for (int yp = 0; yp < textureHeight; ++yp) 
		{
			int const sourcePixelOffset = subLeft + ((yp + subTop) * sourcePitch);
			int const destPixelOffset = yp * destPitch;
			
			if (alphaExtend) 
			{
				for (int xp = 0; xp < textureWidth; ++xp) 
				{
					unsigned int pixel = static_cast<unsigned int>(pixelsARGB[xp + sourcePixelOffset]);
					unsigned char const alpha = static_cast<unsigned char>(pixel >> 24);
					lockedPixels[xp + destPixelOffset] = D3DCOLOR_ARGB(0x00, alpha, alpha, alpha);
				}
			}
			else
			{
				memcpy(lockedPixels + destPixelOffset, pixelsARGB + sourcePixelOffset, textureWidth * sizeof(int));
			}
		}

		texturePointer->UnlockRect(0);

		D3DXSaveTextureToFile(file, static_cast<D3DXIMAGE_FILEFORMAT>(imageFormat), texturePointer, NULL);

		texturePointer->Release();

		return true;
	}

	return false;
}

// ----------------------------------------------------------------------

#if PRODUCTION == 0
bool Direct3d9Namespace::createVideoBuffers(int width, int height)
{
	if (ms_videoSurface || ms_videoOffScreenSurface)
	{
		Graphics::setLastError("engine", "video_buffers_already_created");
		return false;
	}
	if (!getBackBuffer() || ms_backBufferLocked)
	{
		return false;
	}
	// create a surface for the video frame
	HRESULT hresult = ms_device->CreateRenderTarget(width, height, D3DFMT_X8R8G8B8, D3DMULTISAMPLE_NONE, 0, TRUE, &ms_videoSurface, NULL);
	if (FAILED(hresult) || ms_videoSurface == NULL)
	{
		Graphics::setLastError("engine", "create_video_buffer_failed");
		return false;
	}
	hresult = ms_device->CreateOffscreenPlainSurface(width, height, D3DFMT_X8R8G8B8, D3DPOOL_SYSTEMMEM, &ms_videoOffScreenSurface, NULL);
	if (FAILED(hresult) || ms_videoOffScreenSurface == NULL)
	{
		Graphics::setLastError("engine", "create_video_buffer_failed");
		return false;
	}
	return true;
}
#endif // PRODUCTION

// ----------------------------------------------------------------------

#if PRODUCTION == 0
void Direct3d9Namespace::fillVideoBuffers()
{
	if ((!ms_videoSurface) || (!ms_videoOffScreenSurface) || (!ms_backBuffer) || ms_backBufferLocked)
	{
		return;
	}
	if (FAILED(ms_device->StretchRect(ms_backBuffer, NULL, ms_videoSurface, NULL, D3DTEXF_LINEAR)))
	{
		return;
	}
	if (FAILED(ms_device->GetRenderTargetData(ms_videoSurface, ms_videoOffScreenSurface)))
	{
		return;
	}
}
#endif // PRODUCTION

// ----------------------------------------------------------------------

#if PRODUCTION == 0
bool Direct3d9Namespace::getVideoBufferData(void *buffer, size_t bufferSize)
{
	if (!ms_videoOffScreenSurface)
	{
		return false;
	}
	D3DLOCKED_RECT d3dLockedRect;
	if (FAILED(ms_videoOffScreenSurface->LockRect(&d3dLockedRect, NULL, D3DLOCK_NOSYSLOCK | D3DLOCK_READONLY | D3DLOCK_NO_DIRTY_UPDATE)))
	{
		return false;
	}
	memcpy(buffer, d3dLockedRect.pBits, bufferSize);
	ms_videoOffScreenSurface->UnlockRect();
	return true;
}
#endif // PRODUCTION

// ----------------------------------------------------------------------

#if PRODUCTION == 0
void Direct3d9Namespace::releaseVideoBuffers()
{
	if (ms_videoSurface)
	{
		ms_videoSurface->Release();
		ms_videoSurface = NULL;
	}
	if (ms_videoOffScreenSurface)
	{
		ms_videoOffScreenSurface->Release();
		ms_videoOffScreenSurface = NULL;
	}
}
#endif // PRODUCTION

// ======================================================================
