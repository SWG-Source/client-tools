//======================================================================
//
// CuiLoadingManager.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiLoadingManager_H
#define INCLUDED_CuiLoadingManager_H

//======================================================================

class StringId;

class CuiLoadingManager
{

public:

	struct Messages
	{
		static const char * const FullscreenLoadingEnabled;
		static const char * const FullscreenLoadingDisabled;
		static const char * const FullscreenBackButtonEnabled;
	};

	static void                    setFullscreenLoadingEnabled  (bool b);
	static void                    setFullscreenLoadingPercent  (int percent);
	static void                    setFullscreenLoadingString   (const Unicode::String & str);
	static void                    setFullscreenBackButtonEnabled (bool b);
	static void                    setServerObjectsReceived(bool b);
	static void                    setPlayerReady(bool b);
	static void                    setFileCachingComplete(bool b);
	static void                    setWorldLoaded(bool b);
	static void                    setTerrainGenerated(bool b);

	static bool                    getFullscreenLoadingEnabled ();
	static int                     getFullscreenLoadingPercent ();
	static const Unicode::String & getFullscreenLoadingString  ();
	static bool                    getServerObjectsReceived();
	static bool                    getFileCachingComplete();
	static bool                    getWorldLoaded();
	static bool                    getTerrainGenerated();
	static bool                    getPlayerReady();
	static bool                    isInitialLoadingScreen();
	static void                    setInitialLoadingScreenShown();
	static bool                    isLoadingScreenVisible();
	static void                    setLoadingScreenVisible(bool visible);
	static bool                    firstLoadingScreenHasBeenShown();
	static void                    setFirstLoadingScreenHasBeenShown();

	static void                    setPlanetName(const std::string& planetName);
	static const std::string&      getPlanetName();
	static bool                    getIsGeneralTip();
	static void                    setPlanetTerrainFilename(const std::string& terrainFilename);

	static bool                    getLoadingData(StringId& /*OUT*/ loadingText, StringId& /*OUT*/ pictureTitle, std::string& /*OUT*/ loadingPicture);

	static char const * const      getLoadingTypeMediator();

private:
	static std::string             ms_currentPlanet;
	static bool                    ms_isGeneralTip;
	static bool                    ms_previousDetailAppearanceFadeInEnabled;
};

//======================================================================

#endif
