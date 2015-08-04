// ======================================================================
//
// ConfigClientGraphics.cpp
// Portions copyright 1999 Bootprint Entertainment
// Portions copyright 2001-2002 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/ConfigClientGraphics.h"

#include "sharedFoundation/ConfigFile.h"
#include "clientGraphics/Graphics.def"

// ======================================================================

namespace ConfigClientGraphicsNamespace
{
	int   ms_rasterMajor;

	int   ms_screenWidth;
	int   ms_screenHeight;

	bool  ms_windowed;
	bool  ms_skipInitialClearViewport;
	bool  ms_borderlessWindow;

	int   ms_colorBufferBitDepth;
	int   ms_alphaBufferBitDepth;
	int   ms_zBufferBitDepth;
	int   ms_stencilBufferBitDepth;

	bool  ms_validateShaderImplementations;
	bool  ms_disableMultiStreamVertexBuffers;
	bool  ms_screenShotBackBuffer;
	bool  ms_nPatchTest;
	bool  ms_disableOcclusionCulling;

	bool  ms_logBadCustomizationData;

	float ms_dpvsMinimumObjectWidth;
	float ms_dpvsMinimumObjectHeight;
	float ms_dpvsMinimumObjectOpacity;
	float ms_dpvsImageScale;

	bool  ms_useHardwareMouseCursor;
	bool  ms_hardwareMouseCursorUseOriginalAlpha;
	bool  ms_constrainMouseCursorToWindow;

	bool  ms_enableLightScaling;

	int   ms_discardHighestMipMapLevels;
	int   ms_discardHighestNormalMipMapLevels;

	bool  ms_loadAllAssetsRegardlessOfShaderCapability;

	bool  ms_loadGpa;
}

using namespace ConfigClientGraphicsNamespace;

// ======================================================================

#define KEY_INT(a,b)     (ms_ ## a = ConfigFile::getKeyInt("ClientGraphics", #a, b))
#define KEY_FLOAT(a,b)   (ms_ ## a = ConfigFile::getKeyFloat("ClientGraphics", #a, b))
#define KEY_BOOL(a,b)    (ms_ ## a = ConfigFile::getKeyBool("ClientGraphics", #a, b))

// @todo remove these when the options have been migrated
#define KEY_SF_INT(a,b)     (ms_ ## a = ConfigFile::getKeyInt("ClientGraphics", #a, ConfigFile::getKeyInt("SharedFoundation", #a, b)))
#define KEY_SF_BOOL(a,b)    (ms_ ## a = ConfigFile::getKeyBool("ClientGraphics", #a, ConfigFile::getKeyBool("SharedFoundation", #a, b)))

// ======================================================================
// Determine the configuration information
//
// Remarks:
//
//   This routine inspects the ConfigFile class to set some variables for rapid access
//   by the rest of the engine.

void ConfigClientGraphics::install (const Defaults &defaults)
{
	KEY_INT(rasterMajor,                          defaults.rasterMajor);

	KEY_SF_INT(screenWidth,                       defaults.screenWidth);
	KEY_SF_INT(screenHeight,                      defaults.screenHeight);

	KEY_SF_BOOL(windowed,                         defaults.windowed);
	KEY_BOOL(skipInitialClearViewport,            defaults.skipInitialClearViewport);
	KEY_BOOL(borderlessWindow,                    false);

	KEY_INT(colorBufferBitDepth,                  defaults.colorBufferBitDepth);
	KEY_INT(alphaBufferBitDepth,                  defaults.alphaBufferBitDepth);
	KEY_INT(zBufferBitDepth,                      defaults.zBufferBitDepth);
	KEY_INT(stencilBufferBitDepth,                defaults.stencilBufferBitDepth);

	KEY_BOOL(validateShaderImplementations,       true);
	KEY_BOOL(screenShotBackBuffer,                false);
	KEY_BOOL(disableMultiStreamVertexBuffers,     true);
	KEY_BOOL(nPatchTest,                          false);
	KEY_BOOL(disableOcclusionCulling,             false);

	KEY_BOOL(logBadCustomizationData,             false);

	KEY_FLOAT(dpvsMinimumObjectWidth,             8.0f);
	KEY_FLOAT(dpvsMinimumObjectHeight,            8.0f);
	KEY_FLOAT(dpvsMinimumObjectOpacity,           1.0f);
	KEY_FLOAT(dpvsImageScale,                     0.5f);

	KEY_BOOL(useHardwareMouseCursor,              true);
	KEY_BOOL(hardwareMouseCursorUseOriginalAlpha, false);
	KEY_BOOL(constrainMouseCursorToWindow,        true);

	KEY_BOOL(enableLightScaling,                  true);

	KEY_INT(discardHighestMipMapLevels,           0);
	KEY_INT(discardHighestNormalMipMapLevels,     0);

	KEY_BOOL(loadAllAssetsRegardlessOfShaderCapability, false);

	KEY_BOOL(loadGpa,                             false);
}

// ----------------------------------------------------------------------
/**
 * Return the initial raster major number for the game.
 * 
 * @return The initial value of the raster major number for the graphics subsystem
 */

int ConfigClientGraphics::getRasterMajor()
{
	return ms_rasterMajor;
}

// ----------------------------------------------------------------------
/**
 * Return the initial screen width for the game.
 * 
 * @return The initial value of the screen width for the graphics subsystem
 */

int ConfigClientGraphics::getScreenWidth()
{
	return ms_screenWidth;
}

// ----------------------------------------------------------------------
/**
 * Return the initial screen height for the game.
 * 
 * @return The initial value of the screen height for the graphics subsystem
 */

int ConfigClientGraphics::getScreenHeight()
{
	return ms_screenHeight;
}

// ----------------------------------------------------------------------
/**
 * return the initial windowed state for the game.
 * 
 * @return The initial value of the windowed flag for the graphics subsystem
 */

bool ConfigClientGraphics::getWindowed()
{
	return ms_windowed;
}

//----------------------------------------------------------------------

bool ConfigClientGraphics::getBorderlessWindow()
{
	return ms_borderlessWindow;
}

// ----------------------------------------------------------------------
/**
 * Return the initial screen color depth (in bits) for the game.
 * 
 * @return The initial value of the color depth (in bits) for the graphics subsystem
 */

int ConfigClientGraphics::getColorBufferBitDepth()
{
	return ms_colorBufferBitDepth;
}

// ----------------------------------------------------------------------
/**
 * Return the initial screen alpha depth (in bits) for the game.
 * 
 * @return The initial value of the alpha depth (in bits) for the graphics subsystem
 */

int ConfigClientGraphics::getAlphaBufferBitDepth()
{
	return ms_alphaBufferBitDepth;
}

// ----------------------------------------------------------------------
/**
 * Return the initial z buffer depth (in bits) for the game.
 * 
 * @return The initial value of the z buffer depth (in bits) for the graphics subsystem
 */

int ConfigClientGraphics::getZBufferBitDepth()
{
	return ms_zBufferBitDepth;
}

// ----------------------------------------------------------------------
/**
 * Return the initial screen depth (in bits) for the game.
 * 
 * @return The initial value of the screen depth (in bits) for the graphics subsystem
 */

int ConfigClientGraphics::getStencilBufferBitDepth()
{
	return ms_stencilBufferBitDepth;
}

// ----------------------------------------------------------------------

bool ConfigClientGraphics::getValidateShaderImplementations()
{
	return ms_validateShaderImplementations;
}

// ----------------------------------------------------------------------

bool ConfigClientGraphics::getDisableMultiStreamVertexBuffers()
{
	return ms_disableMultiStreamVertexBuffers;
}

//----------------------------------------------------------------------

bool ConfigClientGraphics::getDisableOcclusionCulling()
{
	return ms_disableOcclusionCulling;
}

// ----------------------------------------------------------------------

bool ConfigClientGraphics::getLogBadCustomizationData()
{
	return ms_logBadCustomizationData;
}

// ----------------------------------------------------------------------

float ConfigClientGraphics::getDpvsMinimumObjectWidth()
{
	return ms_dpvsMinimumObjectWidth;
}

// ----------------------------------------------------------------------

float ConfigClientGraphics::getDpvsMinimumObjectHeight()
{
	return ms_dpvsMinimumObjectHeight;
}

// ----------------------------------------------------------------------

float ConfigClientGraphics::getDpvsMinimumObjectOpacity()
{
	return ms_dpvsMinimumObjectOpacity;
}

// ----------------------------------------------------------------------

float ConfigClientGraphics::getDpvsImageScale()
{
	return ms_dpvsImageScale;
}

// ----------------------------------------------------------------------

bool ConfigClientGraphics::getUseHardwareMouseCursor()
{
	return ms_useHardwareMouseCursor;
}

// ----------------------------------------------------------------------

bool ConfigClientGraphics::getHardwareMouseCursorUseOriginalAlpha()
{
	return ms_hardwareMouseCursorUseOriginalAlpha;
}

// ----------------------------------------------------------------------

bool ConfigClientGraphics::getConstrainMouseCursorToWindow()
{
	return ms_constrainMouseCursorToWindow;
}

// ----------------------------------------------------------------------

bool ConfigClientGraphics::getEnableLightScaling()
{
	return ms_enableLightScaling;
}

// ----------------------------------------------------------------------

int ConfigClientGraphics::getDiscardHighestMipMapLevels()
{
	return ms_discardHighestMipMapLevels;
}

// ----------------------------------------------------------------------

int ConfigClientGraphics::getDiscardHighestNormalMipMapLevels()
{
	return ms_discardHighestNormalMipMapLevels;
}

// ----------------------------------------------------------------------

bool ConfigClientGraphics::getLoadAllAssetsRegardlessOfShaderCapability()
{
	return ms_loadAllAssetsRegardlessOfShaderCapability;
}

// ----------------------------------------------------------------------

bool ConfigClientGraphics::getLoadGpa()
{
	return ms_loadGpa;
}

// ----------------------------------------------------------------------

bool ConfigClientGraphics::getSkipInitialClearViewport()
{
	return ms_skipInitialClearViewport;
}

// ======================================================================
