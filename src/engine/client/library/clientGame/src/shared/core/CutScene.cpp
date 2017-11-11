// ======================================================================
//
// CutScene.cpp
// Copyright 2005 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/CutScene.h"

#include "clientGame/Game.h"
#include "clientAudio/Audio.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/Video.h"
#include "clientGraphics/VideoList.h"
#include "clientGraphics/VideoPlaybackManager.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedUtility/DataTable.h"

#include <vector>
#include <algorithm>
#include <set>
#include <string>

#include <dinput.h>


// ======================================================================

namespace CutSceneNamespace
{
	static bool   s_installed;
	static bool   s_resolutionAdjusted;
	static bool   s_fullScreen;
	static bool   s_isPremier;
	static int    s_restoreWidth;
	static int    s_restoreHeight;
	static Video *s_video;
	static float  s_backgroundMusicVolume=1;

	// ======================================================================

	static const char *s_suffix = ".cut";

	// ======================================================================

	typedef std::pair<std::string, std::string> StringPair;

	struct StringPairFirstLT { 
		bool operator()(const StringPair &x1, const StringPair &x2) const {
			return x1.first.compare(x2.first)<0;
		}
	};

	static std::vector<StringPair> s_areaCutSceneLookup;

	// ======================================================================

	static void _loadAreaCutSceneLookups()
	{
		s_areaCutSceneLookup.clear();
		Iff iff;
		if (iff.open("datatables/cutscenes/cutscenes.iff", true))
		{
			DataTable areaCutScenesTable;
			areaCutScenesTable.load(iff);
			
			int const areaCount = areaCutScenesTable.getNumRows();
			for (int areaRow = 0; areaRow < areaCount; ++areaRow)
			{
				const std::string &areaName = areaCutScenesTable.getStringValue("areaName", areaRow);
				const std::string &cutSceneName = areaCutScenesTable.getStringValue("cutSceneName", areaRow);

				s_areaCutSceneLookup.emplace_back();
				StringPair &lookup = s_areaCutSceneLookup.back();
				lookup.first = areaName;
				lookup.second = cutSceneName;
			}

			std::sort(s_areaCutSceneLookup.begin(), s_areaCutSceneLookup.end(), StringPairFirstLT());

			/*
			std::vector<StringPair>::iterator si;
			for (si=s_areaCutSceneLookup.begin();si!=s_areaCutSceneLookup.end();++si)
			{
				REPORT_PRINT(true, ("%s -> %s\n", si->first.c_str(), si->second.c_str()));
			}
			*/
		}
	}

	// ======================================================================

	static void _loadSeenCutScenesFile(std::set<std::string> &o_scenes, const char *fileName)
	{
		FILE *file = fopen(fileName, "rb");
		if (file)
		{
			char line[_MAX_PATH];

			line[sizeof(line)-1]=0;
			while (fgets(line, sizeof(line)-1, file))
			{
				for (char *iter=line;*iter;iter++)
				{
					if (*iter=='\r' || *iter=='\n')
					{
						*iter=0;
						break;
					}
				}

				if (*line)
				{
					_strlwr(line);
					o_scenes.insert(std::string(line));
				}
			}

			fclose(file);
			file=0;
		}
	}

	// ======================================================================

	static void _saveSeenCutScenesFile(const char *fileName, const std::set<std::string> &scenes)
	{
		FILE *file = fopen(fileName, "wb");
		if (file)
		{
			const std::set<std::string>::const_iterator siend=scenes.end();
			for (std::set<std::string>::const_iterator si=scenes.begin();si!=siend;++si)
			{
				fprintf(file, "%s\r\n", si->c_str());
			}
			fclose(file);
			file=0;
		}
	}

	// ======================================================================

	static void remove()
	{
		if (!s_installed)
		{
			return;
		}

		s_areaCutSceneLookup.clear();
		s_installed=false;
	}

}
using namespace CutSceneNamespace;

// ======================================================================

bool CutScene::install()
{
	_loadAreaCutSceneLookups();
	ExitChain::add(CutSceneNamespace::remove, "CutScene::remove");
	s_installed=true;
	return true;
}

// ======================================================================

bool CutScene::start(const char *i_videoName, bool isPremier)
{
	if (s_video)
	{
		return false;
	}

	s_video = VideoList::fetch(i_videoName);

	if (!s_video)
	{
		return false;
	}

	s_fullScreen=false;

	if (VideoPlaybackManager::insert(*s_video, true, false))
	{
		int screenWidth = Graphics::getCurrentRenderTargetWidth();
		int screenHeight = Graphics::getCurrentRenderTargetHeight();

		if (s_video->canStretchBlt())
		{
			VideoPlaybackManager::setPosition(*s_video, 0, 0, screenWidth, screenHeight);
			s_fullScreen=true;
		}
		else
		{
			const int videoWidth = s_video->getWidth();
			const int videoHeight = s_video->getHeight();

			s_restoreWidth = Graphics::getFrameBufferMaxWidth();
			s_restoreHeight = Graphics::getFrameBufferMaxHeight();

			if (  videoWidth  <= 640 
				&& videoHeight <= 480
				&& s_restoreWidth  > 640
				)
			{
				s_resolutionAdjusted=true;
				Graphics::resize(640, 480);
				screenWidth  = Graphics::getCurrentRenderTargetWidth();
				screenHeight = Graphics::getCurrentRenderTargetHeight();
				s_fullScreen = (screenWidth==videoWidth && screenHeight==videoHeight);
			}

			const int screenX = (screenWidth-videoWidth)/2;
			const int screenY = (screenHeight-videoHeight)/2;
			VideoPlaybackManager::setPosition(*s_video, screenX, screenY);
		}

		int videoVolume = 0;
		if (Audio::isEnabled())
		{
			float masterVolume = Audio::getMasterVolume();
			float effectsVolume = Audio::getSoundEffectVolume();
			videoVolume = int(effectsVolume*masterVolume*32768.0f);
		}
		s_video->setVolume(0, videoVolume);

		Audio::silenceAllNonBackgroundMusic();

		s_backgroundMusicVolume = Audio::getBackGroundMusicVolume();
		Audio::setBackGroundMusicVolume(0);

		s_isPremier = isPremier;

		return true;
	}
	else
	{
		s_video->release();
		s_video=0;
		return false;
	}
}

// ======================================================================

void CutScene::stop()
{
	if (s_video)
	{
		VideoPlaybackManager::remove(*s_video);
		s_video->release();
		s_video=0;

		s_isPremier = false;

		Audio::unSilenceAllNonBackgroundMusic();
		Audio::setBackGroundMusicVolume(s_backgroundMusicVolume);
	}

	if (s_resolutionAdjusted)
	{
		Graphics::resize(s_restoreWidth, s_restoreHeight);
		s_resolutionAdjusted=false;
	}
}

// ======================================================================

bool CutScene::isRunning()
{
	if (s_video)
	{
		return VideoPlaybackManager::has(*s_video);
	}
	else
	{
		return false;
	}
}

// ======================================================================

bool CutScene::isRunningPremier()
{
	return s_isPremier && isRunning();
}

// ======================================================================

bool CutScene::isFullScreen()
{
	return s_fullScreen;
}

// ======================================================================

void CutScene::update()
{
	if (!s_video)
	{
		return;
	}

	if (s_video->isFinished())
	{
		stop();
		Game::endCutScene();
	}
}

// ======================================================================

const char *CutScene::lookupCutScene(const char *i_areaName)
{
	StringPair lookup;
	lookup.first=i_areaName;
	
	std::vector<StringPair>::iterator si = std::lower_bound(
		s_areaCutSceneLookup.begin(), 
		s_areaCutSceneLookup.end(), 
		lookup,
		StringPairFirstLT()
		);
	
	if (si!=s_areaCutSceneLookup.end() && lookup.first==si->first)
	{
		return si->second.c_str();
	}
	else
	{
		return 0;
	}
}

// ======================================================================

bool CutScene::getCutSceneSeen(const char *i_videoName, const char *i_playerFileBaseName)
{
	if (  ! i_videoName 
		|| !*i_videoName 
		|| ! i_playerFileBaseName 
		|| !*i_playerFileBaseName
		)
	{
		return false;
	}

	// ------------------------------------------------------------

	char fileName[_MAX_PATH];
	strcpy(fileName, i_playerFileBaseName);
	strcat(fileName, s_suffix);

	// ------------------------------------------------------------

	std::string videoName;
	for (const char *si=i_videoName;*si;++si)
	{
		videoName+=char(tolower(*si));
	}

	// ------------------------------------------------------------

	std::set<std::string> scenes;
	_loadSeenCutScenesFile(scenes, fileName);

	// ------------------------------------------------------------

	return scenes.find(videoName)!=scenes.end();
}

// ======================================================================

void CutScene::setCutSceneSeen(const char *i_videoName, const char *i_playerFileBaseName, bool hasSeenIt)
{
	if (  ! i_videoName 
		|| !*i_videoName 
		|| ! i_playerFileBaseName 
		|| !*i_playerFileBaseName
		)
	{
		return;
	}

	// ------------------------------------------------------------

	char fileName[_MAX_PATH];
	strcpy(fileName, i_playerFileBaseName);
	strcat(fileName, s_suffix);

	// ------------------------------------------------------------

	std::string videoName;
	for (const char *si=i_videoName;*si;++si)
	{
		videoName+=char(tolower(*si));
	}

	// ------------------------------------------------------------

	std::set<std::string> scenes;
	_loadSeenCutScenesFile(scenes, fileName);

	// ------------------------------------------------------------

	if (hasSeenIt)
	{
		scenes.insert(videoName);
	}
	else
	{
		scenes.erase(videoName);
	}

	// ------------------------------------------------------------

	_saveSeenCutScenesFile(fileName, scenes);
}

// ======================================================================

void CutScene::clearAllSeen(const char *i_playerFileBaseName)
{
	if (  ! i_playerFileBaseName 
		|| !*i_playerFileBaseName
		)
	{
		return;
	}

	// ------------------------------------------------------------

	char fileName[_MAX_PATH];
	strcpy(fileName, i_playerFileBaseName);
	strcat(fileName, s_suffix);

	// ------------------------------------------------------------

	_unlink(fileName);
}

// ======================================================================
