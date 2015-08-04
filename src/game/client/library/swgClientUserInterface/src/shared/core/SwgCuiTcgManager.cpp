// ======================================================================
//
// SwgCuiTcgManager.cpp
// copyright (c) 2008 Sony Online Entertainment LLC
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiTcgManager.h"

#include "clientAudio/Audio.h"
#include "clientGame/ConfigClientGame.h"
#include "clientUserInterface/CuiMediator.h"
#include "clientUserInterface/CuiWorkspace.h"
#include "libEverQuestTCG/libEverQuestTCG.h"
#include "sharedFoundation/Os.h"
#include "swgClientUserInterface/SwgCuiTcgControl.h"
#include "swgClientUserInterface/SwgCuiTcgWindow.h"
#include "swgClientUserInterface/SwgCuiWebBrowserManager.h"

#include "UIImage.h"
#include "UIManager.h"
#include "UIPage.h"

#include <list>

// ----------------------------------------------------------------------

namespace SwgCuiTcgManagerNamespace
{
	bool s_installed;

	void __stdcall navigateProc(const char * url);
	void __stdcall navigateWithPostDataProc(const char * url, const char * postData);
	void __stdcall playSound(char *buffer, unsigned bufferLenInBytes, libEverQuestTCG::AudioFormatType type);
	void __stdcall playMusic(char *buffer, unsigned bufferLenInBytes, libEverQuestTCG::AudioFormatType type);
	void __stdcall setSoundVolume(float zeroToOne);
	void __stdcall setMusicVolume(float zeroToOne);
	void __stdcall stopAllSounds();
	void __stdcall setWindowState(int windowState);

	char const * getAudioFormatExtension(libEverQuestTCG::AudioFormatType type);
}

using namespace SwgCuiTcgManagerNamespace;

// ----------------------------------------------------------------------

void SwgCuiTcgManager::install()
{
	DEBUG_FATAL(s_installed, ("already installed\n"));
	s_installed = true;

	static std::string startupDirectory = Os::getProgramStartupDirectory();


#if 0 // _DEBUG
	startupDirectory += "\\TradingCardGameD";
#else
	startupDirectory += "\\" + ConfigClientGame::getTcgDirectory();
#endif

	libEverQuestTCG::RealmType const realmType = ConfigClientGame::getUseTcgRealmTypeStage() ? libEverQuestTCG::REALM_Stage : libEverQuestTCG::REALM_Live;

	libEverQuestTCG::init(startupDirectory.c_str(), libEverQuestTCG::HPT_StarWarsGalaxies, realmType);
	libEverQuestTCG::setDesktopWindow(Os::getWindow());

	libEverQuestTCG::setNavigateCallback(navigateProc);
	libEverQuestTCG::setNavigateWithPostDataCallback(navigateWithPostDataProc);
	libEverQuestTCG::setPlaySoundCallback(playSound);
	libEverQuestTCG::setPlayMusicCallback(playMusic);
	libEverQuestTCG::setSetSoundVolumeCallback(setSoundVolume);
	libEverQuestTCG::setSetMusicVolumeCallback(setMusicVolume);
	libEverQuestTCG::setStopAllSoundsCallback(stopAllSounds);
	libEverQuestTCG::setSetWindowStateCallback(setWindowState);
}

// ----------------------------------------------------------------------

void SwgCuiTcgManager::remove()
{
	DEBUG_FATAL(!s_installed, ("not installed\n"));
	s_installed = false;
	libEverQuestTCG::release();
}

// ----------------------------------------------------------------------

void SwgCuiTcgManager::launch()
{
	if (!libEverQuestTCG::isLaunched())
	{
		libEverQuestTCG::launch();
		libEverQuestTCG::update();
	}
}

// ----------------------------------------------------------------------

void SwgCuiTcgManager::update(float deltaTimeSecs)
{
	UNREF(deltaTimeSecs);
	
	// This is being moved to CuiManager.cpp. We need to update the TCG BEFORE we start resizing windows, otherwise
	// we can end up with a UI page that is 1 frame behind in terms of size. This will cause a crash when we do our
	// texture memcpy.
	//if (libEverQuestTCG::isLaunched())
	//	libEverQuestTCG::update();
}

// ----------------------------------------------------------------------

void SwgCuiTcgManager::setLoginInfo(char const * const username, char const * const sessionId)
{
	char const * const u = username ? username : "";
	char const * const s = sessionId ? sessionId : "";

	libEverQuestTCG::setUserName(u);
	libEverQuestTCG::setSessionID(s);
}

// ======================================================================

void __stdcall SwgCuiTcgManagerNamespace::navigateProc(const char * url)
{
	if (url)
	{
		DEBUG_REPORT_LOG(true, ("SwgCuiTcgManagerNamespace::navigateProc() - %s\n", url));

		SwgCuiWebBrowserManager::createWebBrowserPage(false);
		SwgCuiWebBrowserManager::setURL(url, true);
	}
}

// ----------------------------------------------------------------------

void __stdcall SwgCuiTcgManagerNamespace::navigateWithPostDataProc(const char * url, const char * postData)
{
	if (url)
	{
		DEBUG_REPORT_LOG(true, ("SwgCuiTcgManagerNamespace::navigateWithPostDataProc() - %s\n", url));

		SwgCuiWebBrowserManager::createWebBrowserPage(false);

		if (postData)
			SwgCuiWebBrowserManager::setURL(url, true, postData, strlen(postData));
		else
			SwgCuiWebBrowserManager::setURL(url, true);
	}
}

// ----------------------------------------------------------------------

void __stdcall SwgCuiTcgManagerNamespace::playSound(char *buffer, unsigned bufferLenInBytes, libEverQuestTCG::AudioFormatType type)
{
	DEBUG_REPORT_LOG(true, ("SwgCuiTcgManagerNamespace::playSound() - %d, %d\n", bufferLenInBytes, type));

	Audio::playBufferedSound(buffer, bufferLenInBytes, getAudioFormatExtension(type));
}

// ----------------------------------------------------------------------

void __stdcall SwgCuiTcgManagerNamespace::playMusic(char *buffer, unsigned bufferLenInBytes, libEverQuestTCG::AudioFormatType type)
{
	DEBUG_REPORT_LOG(true, ("SwgCuiTcgManagerNamespace::playMusic() - %d, %d\n", bufferLenInBytes, type));

	Audio::playBufferedMusic(buffer, bufferLenInBytes, getAudioFormatExtension(type));
}


// ----------------------------------------------------------------------

void __stdcall SwgCuiTcgManagerNamespace::setSoundVolume(float zeroToOne)
{
	DEBUG_REPORT_LOG(true, ("SwgCuiTcgManagerNamespace::setSoundVolume() - %f\n", zeroToOne));

	Audio::setBufferedSoundVolume(zeroToOne);
}

// ----------------------------------------------------------------------

void __stdcall SwgCuiTcgManagerNamespace::setMusicVolume(float zeroToOne)
{
	DEBUG_REPORT_LOG(true, ("SwgCuiTcgManagerNamespace::setMusicVolume() - %f\n", zeroToOne));

	Audio::setBufferedMusicVolume(zeroToOne);
}

// ----------------------------------------------------------------------

void __stdcall SwgCuiTcgManagerNamespace::stopAllSounds()
{
	DEBUG_REPORT_LOG(true, ("SwgCuiTcgManagerNamespace::stopAllSounds()\n"));

	Audio::stopBufferedSound();
	Audio::stopBufferedMusic();
}

// ----------------------------------------------------------------------

void __stdcall SwgCuiTcgManagerNamespace::setWindowState(int windowState)
{
	UNREF(windowState);
	DEBUG_REPORT_LOG(true, ("SwgCuiTcgManagerNamespace::setWindowState() - %d\n", windowState));
}

// ----------------------------------------------------------------------

char const * SwgCuiTcgManagerNamespace::getAudioFormatExtension(libEverQuestTCG::AudioFormatType type)
{
	char const * extension = 0;

	switch (type)
	{
	case libEverQuestTCG::WAVAudioFormat:
		extension = ".wav";
		break;
	case libEverQuestTCG::MP3AudioFormat:
		extension = ".mp3";
		break;
	case libEverQuestTCG::OGGAudioFormat:
		extension = ".ogg";
		break;
	default :
		DEBUG_FATAL(true, ("Invalid audio format %d.", type));
		break;
	}

	return extension;
}

// ======================================================================
