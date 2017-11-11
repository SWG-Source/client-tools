//======================================================================
//
// CuiLoadingManager.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiLoadingManager.h"

#include "clientGame/Game.h"
#include "clientObject/DetailAppearance.h"
#include "clientObject/MeshAppearance.h"
#include "clientUserInterface/ConfigClientUserInterface.h"
#include "clientUserInterface/CuiSettings.h"
#include "sharedFoundation/Clock.h"
#include "sharedMessageDispatch/Emitter.h"
#include "sharedMessageDispatch/Message.h"
#include "sharedRandom/Random.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"

#include "Unicode.h"
#include "StringId.h"
#include <vector>
#include <algorithm>

//======================================================================

namespace CuiLoadingManagerNamespace
{
	MessageDispatch::Emitter s_emitter;
	bool            s_fullScreenLoadingEnabled = false;
	int             s_fullScreenLoadingPercent = 0;
	Unicode::String s_fullScreenLoadingString;
	bool            s_serverObjectsReceived;
	bool            s_playerReady;
	bool            s_fileCachingComplete;
	bool            s_worldLoaded;
	bool            s_terrainGenerated;
	//has ANY load screen ever been shown during this run of the client?
	bool            s_initialLoadingScreen = true;
	bool            s_loadingScreenVisible = false;
	//has the first loading screen of this CURRENT load sequence been shown yet?
	bool            s_firstLoadingScreenHasBeenShown = false;

	std::vector<std::string> s_planetNamesSpaceLoading;

	const std::string s_spaceloadingTable("datatables/loading/planets_space_loading.iff");

	const std::string s_loadingTablePrefix("datatables/loading/");
	const std::string s_loadingTablePostfix(".iff");
	const std::string s_generalTableName("general");
	const std::string s_tutorialSceneName("tutorial");
	const std::string s_simpleSceneName("simple");
	static const std::string s_hothSceneName("adventure2");
	
	bool s_installed = false;
	
	void install()
	{
		if (s_installed)
			return;
		
		s_installed = true;
		
		//make sure to close() this table before we leave the function
		DataTable* const table = DataTableManager::getTable(s_spaceloadingTable, true);
		if(!table)
		{
			DEBUG_WARNING(true, ("CuiLoadingManager::getLoadingData could not load %s", s_spaceloadingTable.c_str()));
			DataTableManager::close(s_spaceloadingTable);
			return;
		}
		
		const int numRows = table->getNumRows ();

		for (int i = 0; i < numRows; ++i)
		{
			std::string const & str = table->getStringValue(0, i);
			s_planetNamesSpaceLoading.push_back(str);
		}

		DataTableManager::close(s_spaceloadingTable);

		std::sort(s_planetNamesSpaceLoading.begin(), s_planetNamesSpaceLoading.end());
	}
}

using namespace CuiLoadingManagerNamespace;

//======================================================================

const char * const CuiLoadingManager::Messages::FullscreenLoadingEnabled  = "CuiLoadingManager::FullscreenLoadingEnabled";
const char * const CuiLoadingManager::Messages::FullscreenLoadingDisabled = "CuiLoadingManager::FullscreenLoadingDisabled";
const char * const CuiLoadingManager::Messages::FullscreenBackButtonEnabled = "CuiLoadingManager::FullscreenBackButtonEnabled";

std::string CuiLoadingManager::ms_currentPlanet;
bool        CuiLoadingManager::ms_isGeneralTip = false;
bool		CuiLoadingManager::ms_previousDetailAppearanceFadeInEnabled = false;

//----------------------------------------------------------------------

void CuiLoadingManager::setFullscreenLoadingEnabled  (bool b)
{
	//-- don't display long frame notifications during loading
	Clock::setLongFramesWarningAllowed(!b);

	if(s_fullScreenLoadingEnabled != b)
	{
		s_fullScreenLoadingEnabled = b;
		CuiLoadingManager::setFullscreenLoadingPercent (0);
		CuiLoadingManager::setFullscreenLoadingString  (Unicode::String ());
		if(b)
		{
			ms_previousDetailAppearanceFadeInEnabled = DetailAppearance::getFadeInEnabled();
			DetailAppearance::setFadeInEnabled(false);
		}
		else
		{
			DetailAppearance::setFadeInEnabled(ms_previousDetailAppearanceFadeInEnabled);
		}
		MeshAppearance::setFadeInEnabled(!b);
		
		s_firstLoadingScreenHasBeenShown = false;

		if (b)
			s_emitter.emitMessage (MessageDispatch::MessageBase (Messages::FullscreenLoadingEnabled));
		else
			s_emitter.emitMessage (MessageDispatch::MessageBase (Messages::FullscreenLoadingDisabled));
	}
}

//----------------------------------------------------------------------

void CuiLoadingManager::setFullscreenLoadingPercent (int percent)
{
	s_fullScreenLoadingPercent = percent;
}

//----------------------------------------------------------------------

bool CuiLoadingManager::getFullscreenLoadingEnabled ()
{
	return s_fullScreenLoadingEnabled;
}

//----------------------------------------------------------------------

int  CuiLoadingManager::getFullscreenLoadingPercent ()
{
	return s_fullScreenLoadingPercent;
}

//----------------------------------------------------------------------

void CuiLoadingManager::setFullscreenLoadingString   (const Unicode::String & str)
{
	s_fullScreenLoadingString = str;
}

//----------------------------------------------------------------------

const Unicode::String & CuiLoadingManager::getFullscreenLoadingString   ()
{
	return s_fullScreenLoadingString;
}

//----------------------------------------------------------------------

void CuiLoadingManager::setFullscreenBackButtonEnabled (bool b)
{
	s_emitter.emitMessage (MessageDispatch::Message<bool> (CuiLoadingManager::Messages::FullscreenBackButtonEnabled, b));
}

//----------------------------------------------------------------------

void CuiLoadingManager::setPlanetName(const std::string& planetName)
{
	//TODO straighten out yavin4's official name
	if(planetName == "yavin4")
		ms_currentPlanet = "yavin";
	else
		ms_currentPlanet = planetName;
}

//----------------------------------------------------------------------

const std::string& CuiLoadingManager::getPlanetName()
{
	return ms_currentPlanet;
}

//----------------------------------------------------------------------

bool CuiLoadingManager::getIsGeneralTip()
{
	return ms_isGeneralTip;
}

//----------------------------------------------------------------------

void CuiLoadingManager::setPlanetTerrainFilename(const std::string& terrainFilename)
{
	std::string::size_type startPos = terrainFilename.find_last_of("/");
	if(startPos == terrainFilename.npos)
	{
		startPos = terrainFilename.find("\\");
	}
	std::string::size_type endPos = terrainFilename.find(".trn", startPos);

	if(startPos == terrainFilename.npos || endPos == terrainFilename.npos)
	{
		DEBUG_WARNING(true, ("Terrain filename in unexpected format"));
		return;
	}
	setPlanetName(terrainFilename.substr(startPos+1, endPos-startPos-1));
}

//----------------------------------------------------------------------

bool CuiLoadingManager::getLoadingData(StringId& /*OUT*/ loadingText, StringId& /*OUT*/ pictureTitle, std::string& /*OUT*/ loadingPicture)
{
	loadingText.clear();
	loadingPicture.clear();

	ms_isGeneralTip = false;

	std::string tableName = ms_currentPlanet;
	int result = Random::random(0, 2);
	//use general tips only on the ground

	//-- @todo data drive this
	if(!Game::isSpaceSceneName(ms_currentPlanet) && ms_currentPlanet != "mustafar" && ms_currentPlanet != s_hothSceneName)
	{
		// 1 in 3 times, show general info instead of planet specifc info
		if(ms_currentPlanet.empty() || result == 0 || ms_currentPlanet == s_tutorialSceneName || ms_currentPlanet == s_simpleSceneName)
		{
			tableName = s_generalTableName;
			ms_isGeneralTip = true;
		}
		else
		{
			tableName = ms_currentPlanet;
		}
	}
	std::string dataTableName = s_loadingTablePrefix + tableName + s_loadingTablePostfix;

	//make sure to close() this table before we leave the function
	DataTable* table = DataTableManager::getTable(dataTableName, true);
	if(!table)
	{
		DEBUG_WARNING(true, ("CuiLoadingManager::getLoadingData could not load table for planet, cannot continue"));
		DataTableManager::close(dataTableName);
		return false;
	}

	const int numRows = table->getNumRows ();
	if(numRows == 0)
	{
		DEBUG_WARNING(true, ("CuiLoadingManager::getLoadingData Table has no rows"));
		DataTableManager::close(dataTableName);
		return false;
	}

	//pick a random row
	int rand = Random::random(0, numRows-1);

	if(rand < 0 || rand >= numRows)
	{
		DEBUG_WARNING(true, ("CuiLoadingManager::getLoadingData row read from settings is out of bounds"));
		rand = 0;
	}

	const std::string & textString    = table->getStringValue (0, rand);
	const std::string & pictureString = table->getStringValue (1, rand);
	const std::string & titleString   = table->getStringValue (2, rand);

	loadingText    = StringId(textString);
	loadingPicture = pictureString;
	pictureTitle   = StringId(titleString);

	DataTableManager::close(dataTableName);
	return true;
}

//----------------------------------------------------------------------

void CuiLoadingManager::setServerObjectsReceived(bool b)
{
	s_serverObjectsReceived = b;
}

//----------------------------------------------------------------------

void CuiLoadingManager::setPlayerReady(bool b)
{
	s_playerReady = b;
}

//----------------------------------------------------------------------

void CuiLoadingManager::setFileCachingComplete(bool b)
{
	s_fileCachingComplete = b;
}

//----------------------------------------------------------------------

void CuiLoadingManager::setWorldLoaded(bool b)
{
	s_worldLoaded = b;
}

//----------------------------------------------------------------------

void CuiLoadingManager::setTerrainGenerated(bool b)
{
	s_terrainGenerated = b;
}

//----------------------------------------------------------------------

bool CuiLoadingManager::getServerObjectsReceived()
{
	return s_serverObjectsReceived;
}

//----------------------------------------------------------------------

bool CuiLoadingManager::getPlayerReady()
{
	return s_playerReady;
}

//----------------------------------------------------------------------

bool CuiLoadingManager::getFileCachingComplete()
{
	return s_fileCachingComplete;
}

//----------------------------------------------------------------------

bool CuiLoadingManager::getWorldLoaded()
{
	return s_worldLoaded;
}

//----------------------------------------------------------------------

bool CuiLoadingManager::getTerrainGenerated()
{
	return s_terrainGenerated;
}

//----------------------------------------------------------------------

bool CuiLoadingManager::isInitialLoadingScreen()
{
	return s_initialLoadingScreen;
}

//----------------------------------------------------------------------

void CuiLoadingManager::setInitialLoadingScreenShown()
{
	s_initialLoadingScreen = false;
}

//----------------------------------------------------------------------

bool CuiLoadingManager::isLoadingScreenVisible()
{
	return s_loadingScreenVisible;
}

//----------------------------------------------------------------------

void CuiLoadingManager::setLoadingScreenVisible(bool const visible)
{
	s_loadingScreenVisible = visible;
}

//----------------------------------------------------------------------

bool CuiLoadingManager::firstLoadingScreenHasBeenShown()
{
	return s_firstLoadingScreenHasBeenShown;
}

//----------------------------------------------------------------------

void CuiLoadingManager::setFirstLoadingScreenHasBeenShown()
{
	s_firstLoadingScreenHasBeenShown = true;
}

//----------------------------------------------------------------------

char const * const CuiLoadingManager::getLoadingTypeMediator()
{
	if (Game::isSpaceSceneName(ms_currentPlanet))
		return "LoadingSpace";

	//@todo: data drive
	if (ms_currentPlanet == "mustafar")
		return "LoadingMustafar";
	else if (ms_currentPlanet == s_hothSceneName)
		return "LoadingHoth";

	return "LoadingGround";

}

//======================================================================

