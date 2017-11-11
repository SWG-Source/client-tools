// ======================================================================
//
// SetupClientGraphics.cpp
// copyright 1998 Bootprint Entertainment
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/SetupClientGraphics.h"

#include "clientGraphics/Camera.h"
#include "clientGraphics/ClientDebugShapeRenderer.h"
#include "clientGraphics/ConfigClientGraphics.h"
#include "clientGraphics/CustomizableShaderTemplate.h"
#include "clientGraphics/DebugPrimitive.h"
#include "clientGraphics/GraphicsDebugFlags.h"
#include "clientGraphics/DynamicVertexBuffer.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/GraphicsOptionTags.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/ScreenShotHelper.h"
#include "clientGraphics/ShaderCapability.h"
#include "clientGraphics/ShaderEffectList.h"
#include "clientGraphics/ShaderImplementation.h"
#include "clientGraphics/ShaderImplementationList.h"
#include "clientGraphics/ShaderPrimitiveSet.h"
#include "clientGraphics/ShaderPrimitiveSetTemplate.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/StaticVertexBuffer.h"
#include "clientGraphics/SwitchShaderTemplate.h"
#include "clientGraphics/SwitchTextureShaderTemplate.h"
#include "clientGraphics/SystemVertexBuffer.h"
#include "clientGraphics/Texture.h"
#include "clientGraphics/TextureList.h"
#include "sharedFoundation/ConfigSharedFoundation.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedObject/CellProperty.h"

// ======================================================================

const Tag TAG_ENVM = TAG(E,N,V,M);
const Tag TAG__SKY = TAG(_,S,K,Y);

// ======================================================================

static void TextureFetch(Texture const *texture)
{
	texture->fetch();
}

// ----------------------------------------------------------------------

static void TextureRelease(Texture const *texture)
{
	texture->release();
}

// ======================================================================
// Install the engine
//
// Remarks:
//
//   The settings in the Data structure will determine which subsystems
//   get initialized.

bool SetupClientGraphics::install(const Data &data)
{
	// setup the Graphics configuration
	{
		ConfigClientGraphics::Defaults defaults;
		Zero(defaults);
		defaults.rasterMajor           = data.rasterMajor;
		defaults.screenWidth           = data.screenWidth;
		defaults.screenHeight          = data.screenHeight;
		defaults.windowed              = data.windowed;
		defaults.skipInitialClearViewport = data.skipInitialClearViewport;
		defaults.colorBufferBitDepth   = data.colorBufferBitDepth;
		defaults.alphaBufferBitDepth   = data.alphaBufferBitDepth;
		defaults.zBufferBitDepth       = data.zBufferBitDepth;
		defaults.stencilBufferBitDepth = data.stencilBufferBitDepth;
		ConfigClientGraphics::install(defaults);
	}

	if (data.use3dSystem)
	{
		Camera::install();
		GraphicsDebugFlags::install();
		GraphicsOptionTags::install();
		if (!Graphics::install())
			return false;

		if (Graphics::getShaderCapability() < ShaderCapability(1,1) || Graphics::getVideoMemoryInMegabytes() < 40 || MemoryManager::getLimit() < 260)
		{
			GraphicsOptionTags::set(TAG(D,O,T,3), false);
			REPORT_LOG(ConfigSharedFoundation::getVerboseHardwareLogging(), ("Disabling DOT3\n"));
		}

		if (Graphics::getShaderCapability() < ShaderCapability(2,0) || Graphics::getVideoMemoryInMegabytes() < 100 || MemoryManager::getLimit() < 260)
		{
			GraphicsOptionTags::set(TAG(P,O,S,T), false);
			REPORT_LOG(ConfigSharedFoundation::getVerboseHardwareLogging(), ("Disabling Post Processing effects\n"));
		}

		if (Graphics::getShaderCapability() < ShaderCapability(2,0) || Graphics::getVideoMemoryInMegabytes() < 100 || MemoryManager::getLimit() < 260)
		{
			GraphicsOptionTags::set(TAG(H,E,A,T), false);
			REPORT_LOG(ConfigSharedFoundation::getVerboseHardwareLogging(), ("Disabling Heat effects\n"));
		}

		Texture::install();

#ifdef _DEBUG
		Line3dDebugPrimitive::install ();
		Line2dDebugPrimitive::install ();
		FrameDebugPrimitive::install ();
		FrustumDebugPrimitive::install ();
		SphereDebugPrimitive::install ();
		SphereDebugPrimitive2::install ();
		CircleDebugPrimitive::install ();
		OctahedronDebugPrimitive::install ();
		CylinderDebugPrimitive::install ();
		BoxDebugPrimitive::install ();
		IndexedDebugPrimitive::install ();
		VertexNormalsDebugPrimitive::install ();
		VertexMatrixFramesDebugPrimitive::install ();
		ClientDebugShapeRenderer::install ();
#endif

		ScreenShotHelper::install();
		TextureList::install();

		Texture const *environmentTexture = TextureList::fetchDefaultEnvironmentTexture();
		Graphics::setGlobalTexture(TAG_ENVM, *environmentTexture);
		Graphics::setGlobalTexture(TAG__SKY, *environmentTexture);
		environmentTexture->release();

		// @todo this stuff shouldn't be in the engine.  it should be set up by the game.
		{
			const Tag TAG_ENV  = TAG3(E,N,V);   // environment (infinite distance)
			const Tag TAG_CELS = TAG(C,E,L,S);  // Celestial objects
			const Tag TAG_TERR = TAG(T,E,R,R);  // opaque terrain
			const Tag TAG_BLOB = TAG(B,L,O,B);  // blobby shadow support
			const Tag TAG_MAIN = TAG(M,A,I,N);  // opaque
			const Tag TAG_PREA = TAG(P,R,E,A);  // pre-alpha (stuff that is alpha, but layered on top of opaque, so it really doesn't need to be sorted)
			const Tag TAG_ALPH = TAG(A,L,P,H);  // alpha
			const Tag TAG_BLND = TAG(B,L,N,D);  // blend
			const Tag TAG_WBLW = TAG(W,B,L,W);  // below water
			const Tag TAG_WATR = TAG(W,A,T,R);  // water
			const Tag TAG_WABV = TAG(W,A,B,V);  // above water
			const Tag TAG_PSTA = TAG(P,S,T,A);  // post-alpha
			const Tag TAG_SHDW = TAG(S,H,D,W);  // shadow
			const Tag TAG_LAST = TAG(L,A,S,T);  // debug

			// @todo LAST phase should be added on a config-file flag.
			ShaderPrimitiveSorter::install(13);

			ShaderPrimitiveSorter::setPhase(TAG_ENV,  0);
			ShaderPrimitiveSorter::setPhase(TAG_CELS, 1);
			ShaderPrimitiveSorter::setPhase(TAG_TERR, 2);
			ShaderPrimitiveSorter::setPhase(TAG_MAIN, 3);
			ShaderPrimitiveSorter::setPhase(TAG_BLOB, 4);
			ShaderPrimitiveSorter::setPhase(TAG_PREA, 5);
			ShaderPrimitiveSorter::setPhase(TAG_ALPH, 6);  // we have shaders using both ALPH and BLND to mean this phase
			ShaderPrimitiveSorter::setPhase(TAG_BLND, 6);  // we have shaders using both ALPH and BLND to mean this phase
			ShaderPrimitiveSorter::setPhase(TAG_WBLW, 7);  // below water
			ShaderPrimitiveSorter::setPhase(TAG_WATR, 8);  // water
			ShaderPrimitiveSorter::setPhase(TAG_WABV, 9);  // above water
			ShaderPrimitiveSorter::setPhase(TAG_PSTA, 10);
			ShaderPrimitiveSorter::setPhase(TAG_SHDW, 11);
			ShaderPrimitiveSorter::setPhase(TAG_LAST, 12);

#ifdef _DEBUG
			ShaderPrimitiveSorter::setPhaseDebugName(0,  "phase00_environment");
			ShaderPrimitiveSorter::setPhaseDebugName(1,  "phase01_celestials");
			ShaderPrimitiveSorter::setPhaseDebugName(2,  "phase01_terrain");
			ShaderPrimitiveSorter::setPhaseDebugName(3,  "phase02_main");
			ShaderPrimitiveSorter::setPhaseDebugName(4,  "phase03_shadowBlob");
			ShaderPrimitiveSorter::setPhaseDebugName(5,  "phase04_preAlpha");
			ShaderPrimitiveSorter::setPhaseDebugName(6,  "phase05_alpha");
			ShaderPrimitiveSorter::setPhaseDebugName(7,  "phase06_belowWater");
			ShaderPrimitiveSorter::setPhaseDebugName(8,  "phase07_water");
			ShaderPrimitiveSorter::setPhaseDebugName(9,  "phase08_aboveWater");
			ShaderPrimitiveSorter::setPhaseDebugName(10,  "phase09_postAlpha");
			ShaderPrimitiveSorter::setPhaseDebugName(11, "phase10_shadows");
			ShaderPrimitiveSorter::setPhaseDebugName(12, "phase11_last");
#endif

			ShaderPrimitiveSorter::setPhaseSort(0, ShaderPrimitiveSorter::S_fifo);
			ShaderPrimitiveSorter::setPhaseSort(1, ShaderPrimitiveSorter::S_fifo);
			ShaderPrimitiveSorter::setPhaseSort(2, ShaderPrimitiveSorter::S_performance);
			ShaderPrimitiveSorter::setPhaseSort(3, ShaderPrimitiveSorter::S_performance);
			ShaderPrimitiveSorter::setPhaseSort(4, ShaderPrimitiveSorter::S_performance);
			ShaderPrimitiveSorter::setPhaseSort(5, ShaderPrimitiveSorter::S_performance);
			ShaderPrimitiveSorter::setPhaseSort(6, ShaderPrimitiveSorter::S_painters);
			ShaderPrimitiveSorter::setPhaseSort(7, ShaderPrimitiveSorter::S_painters);
			ShaderPrimitiveSorter::setPhaseSort(8, ShaderPrimitiveSorter::S_painters);
			ShaderPrimitiveSorter::setPhaseSort(9, ShaderPrimitiveSorter::S_painters);
			ShaderPrimitiveSorter::setPhaseSort(10, ShaderPrimitiveSorter::S_painters);
			ShaderPrimitiveSorter::setPhaseSort(11, ShaderPrimitiveSorter::S_fifo);
			ShaderPrimitiveSorter::setPhaseSort(12, ShaderPrimitiveSorter::S_fifo);

			ShaderPrimitiveSorter::setPhaseDrawTime(0, ShaderPrimitiveSorter::D_beforeNextPush);
			ShaderPrimitiveSorter::setPhaseDrawTime(1, ShaderPrimitiveSorter::D_beforeNextPush);
			ShaderPrimitiveSorter::setPhaseDrawTime(2, ShaderPrimitiveSorter::D_beforeNextPush);
			ShaderPrimitiveSorter::setPhaseDrawTime(3, ShaderPrimitiveSorter::D_beforeNextPush);
			ShaderPrimitiveSorter::setPhaseDrawTime(4, ShaderPrimitiveSorter::D_beforeNextPush);
			ShaderPrimitiveSorter::setPhaseDrawTime(5, ShaderPrimitiveSorter::D_beforeNextPush);
			ShaderPrimitiveSorter::setPhaseDrawTime(6, ShaderPrimitiveSorter::D_beforeCurrentPop);
			ShaderPrimitiveSorter::setPhaseDrawTime(7, ShaderPrimitiveSorter::D_beforeCurrentPop);
			ShaderPrimitiveSorter::setPhaseDrawTime(8, ShaderPrimitiveSorter::D_beforeCurrentPop);
			ShaderPrimitiveSorter::setPhaseDrawTime(9, ShaderPrimitiveSorter::D_beforeCurrentPop);
			ShaderPrimitiveSorter::setPhaseDrawTime(10, ShaderPrimitiveSorter::D_beforeCurrentPop);
			ShaderPrimitiveSorter::setPhaseDrawTime(11, ShaderPrimitiveSorter::D_beforeCurrentPop);
			ShaderPrimitiveSorter::setPhaseDrawTime(12, ShaderPrimitiveSorter::D_beforeCurrentPop);
		}

		ShaderImplementation::install();
		ShaderImplementationList::install();
		ShaderEffectList::install();
		ShaderTemplateList::install();
		StaticShader::install();
		StaticShaderTemplate::install(data.preloadVertexColorShaderTemplates);
		SwitchShaderTemplate::install ();
		SwitchTextureShaderTemplate::install ();
		ShaderPrimitiveSet::install();
		ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate::install();

		CustomizableShaderTemplate::install();

		Graphics::setBadVertexShaderStaticShader(ShaderTemplateList::getBadVertexShaderStaticShader());
		CellProperty::setTextureHookFunctions(TextureFetch, TextureRelease);
		TextureList::addCleanupCallbackFunction(CellProperty::releaseWorldCellPropertyEnvironmentTexture);

		RenderWorld::install();
	}

	StaticVertexBuffer::install();
	DynamicVertexBuffer::install();
	SystemVertexBuffer::install();

	return true;
}

// ----------------------------------------------------------------------

void SetupClientGraphics::setupDefaultGameData(Data &data)
{
	Zero(data);

	data.use3dSystem                              = true;

	data.rasterMajor                              = 5;

	data.screenWidth                              = 1024;
	data.screenHeight                             = 768;

	data.windowed                                 = false;
	data.skipInitialClearViewport                 = false;

	data.colorBufferBitDepth                      = -1;
	data.alphaBufferBitDepth                      = -1;
	data.zBufferBitDepth                          = -1;
	data.stencilBufferBitDepth                    = -1;

	data.preloadVertexColorShaderTemplates        = true;
}

// ----------------------------------------------------------------------

void SetupClientGraphics::setupDefaultConsoleData(Data &data)
{
	Zero(data);

	data.use3dSystem                              = false;

	data.rasterMajor                              = 0;

	data.screenWidth                              = 1024;
	data.screenHeight                             = 768;

	data.windowed                                 = false;
	data.skipInitialClearViewport                 = false;

	data.colorBufferBitDepth                      = 0;
	data.alphaBufferBitDepth                      = 0;
	data.zBufferBitDepth                          = 0;
	data.stencilBufferBitDepth                    = 0;

	data.preloadVertexColorShaderTemplates        = false;
}

// ----------------------------------------------------------------------

void SetupClientGraphics::setupDefaultMFCData(Data &data)
{
	Zero(data);

	data.use3dSystem                              = true;

	data.rasterMajor                              = 5;

	data.screenWidth                              = GetSystemMetrics(SM_CXSCREEN);
	data.screenHeight                             = GetSystemMetrics(SM_CYSCREEN);

	data.windowed                                 = true;
	data.skipInitialClearViewport                 = true;

	data.colorBufferBitDepth                      = -1;
	data.alphaBufferBitDepth                      = -1;
	data.zBufferBitDepth                          = -1;
	data.stencilBufferBitDepth                    = -1;

	data.preloadVertexColorShaderTemplates        = true;
}

// ======================================================================
