// ======================================================================
//
// SpacePreloadedAssetManager.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_SpacePreloadedAssetManager_H
#define INCLUDED_SpacePreloadedAssetManager_H

// ======================================================================

class SpacePreloadedAssetManager
{
public:

	static void install();

	static void load(char const * fileName);

	static void preloadSomeAssets();
	static int getLoadingPercent();
	static bool donePreloading();
};

// ======================================================================

#endif
