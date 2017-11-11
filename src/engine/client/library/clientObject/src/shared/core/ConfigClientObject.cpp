//===================================================================
//
// ConfigClientObject.cpp
// copyright 2000, verant interactive
//
//===================================================================

#include "clientObject/FirstClientObject.h"
#include "clientObject/ConfigClientObject.h"

#include "sharedFoundation/ConfigFile.h"

//===================================================================

namespace ConfigClientObjectNamespace
{
	bool        ms_forceLowDetailLevels;
	bool        ms_forceHighDetailLevels;
	float       ms_detailLevelBias;
	float       ms_detailLevelStretch;
	float       ms_detailOverlapFraction;
	float       ms_detailOverlapCap;
	bool        ms_preloadDetailLevels;
	bool        ms_detailAppearancesWithoutSprites;
	float       ms_interiorShadowAlpha;
	const char* ms_screenShader;
	bool        ms_disableMeshTestShapes;
}
using namespace ConfigClientObjectNamespace;

//===================================================================

float ConfigClientObject::getDetailLevelStretch ()
{
	return ms_detailLevelStretch;
}

// ------------------------------------------------------------------

float ConfigClientObject::getDetailOverlapFraction ()
{
	return ms_detailOverlapFraction;
}

// ------------------------------------------------------------------

float ConfigClientObject::getDetailOverlapCap ()
{
	return ms_detailOverlapCap;
}

//-------------------------------------------------------------------

bool ConfigClientObject::getPreloadDetailLevels ()
{
	return ms_preloadDetailLevels;
}

//-------------------------------------------------------------------

bool ConfigClientObject::getDetailAppearancesWithoutSprites()
{
	return ms_detailAppearancesWithoutSprites;
}

//-------------------------------------------------------------------

float ConfigClientObject::getInteriorShadowAlpha ()
{
	return ms_interiorShadowAlpha;
}

//-------------------------------------------------------------------

const char* ConfigClientObject::getScreenShader ()
{
	return ms_screenShader;
}

//----------------------------------------------------------------------

float ConfigClientObject::getDetailLevelBias                 ()
{
	return ms_detailLevelBias;
}

//----------------------------------------------------------------------

bool ConfigClientObject::getDisableMeshTestShapes ()
{
	return ms_disableMeshTestShapes;
}

//===================================================================

#define KEY_BOOL(a,b)    (ms_ ## a = ConfigFile::getKeyBool("ClientObject", #a, b))
#define KEY_FLOAT(a,b)   (ms_ ## a = ConfigFile::getKeyFloat("ClientObject", #a, b))
//#define KEY_INT(a,b)     (ms_ ## a = ConfigFile::getKeyInt("ClientObject", #a, b))
#define KEY_STRING(a,b)  (ms_ ## a = ConfigFile::getKeyString("ClientObject", #a, b))

//===================================================================

void ConfigClientObject::install()
{
	KEY_BOOL   (forceLowDetailLevels,            false);
	KEY_BOOL   (forceHighDetailLevels,           false);
	KEY_FLOAT  (detailLevelBias,                 1.f);
	KEY_FLOAT  (detailLevelStretch,              10.f);
	KEY_FLOAT  (detailOverlapFraction,           0.10f);
	KEY_FLOAT  (detailOverlapCap,                25.0f);
	KEY_BOOL   (preloadDetailLevels,             false);
	KEY_BOOL   (detailAppearancesWithoutSprites, false);
	KEY_FLOAT  (interiorShadowAlpha,             0.1f);
	KEY_STRING (screenShader,                    0);
	KEY_BOOL   (disableMeshTestShapes,           false);
}

//===================================================================
