//===================================================================
//
// ConfigClientObject.h
// copyright 2000, verant interactive
//
//===================================================================

#ifndef INCLUDED_ConfigClientObject_H
#define INCLUDED_ConfigClientObject_H

//===================================================================

class ConfigClientObject
{
public:

	static void        install ();

	static float       getDetailLevelBias ();
	static float       getDetailLevelStretch ();
	static float       getDetailOverlapFraction ();
	static float       getDetailOverlapCap ();
	static bool        getPreloadDetailLevels ();
	static bool        getDetailAppearancesWithoutSprites ();
	static float       getInteriorShadowAlpha ();
	static const char* getScreenShader ();
	static bool        getDisableMeshTestShapes ();
};

//===================================================================

#endif
