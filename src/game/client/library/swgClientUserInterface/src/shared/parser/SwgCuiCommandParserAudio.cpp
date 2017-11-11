// ======================================================================
//
// SwgCuiCommandParserAudio.cpp
// Copyright Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiCommandParserAudio.h"

#include "clientAudio/Audio.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/NetworkId.h"
#include <cstdio>
#include <vector>

// ======================================================================

namespace SwgCuiCommandParserAudioNamespace
{
	namespace CommandNames
	{
		const char * const s_enabled               = "disable";
		const char * const s_masterVolume          = "masterVolume";
		const char * const s_soundEffectVolume     = "soundEffectVolume";
		const char * const s_backGroundMusicVolume = "backGroundMusicVolume";
		const char * const s_playerMusicVolume     = "playerMusicVolume";
		const char * const s_userInterfaceVolume   = "userInterfaceVolume";
		const char * const s_maxSamples            = "maxSamples";
		const char * const s_maxCachedSampleSize   = "maxCachedSampleSize";
		const char * const s_maxCacheSize          = "maxCacheSize";
		const char * const s_stopAllSounds         = "stopAllSounds";
#ifdef _DEBUG	
		const char * const s_debug                 = "debug";
		const char * const s_playBufferedAudio     = "playBufferedAudio";
#endif // _DEBUG
	}

	const CommandParser::CmdInfo cmds[] =
	{
		{CommandNames::s_enabled,               0, "[0 | 1]", "Toggle audio"},
		{CommandNames::s_masterVolume,          0, "[0...1]", "Master audio volume"},
		{CommandNames::s_soundEffectVolume,     0, "[0...1]", "Sound effects volume"},
		{CommandNames::s_backGroundMusicVolume, 0, "[0...1]", "Background music volume"},
		{CommandNames::s_playerMusicVolume,     0, "[0...1]", "Player music volume"},
		{CommandNames::s_userInterfaceVolume,   0, "[0...1]", "User interface volume"},
		{CommandNames::s_maxSamples,            0, "[16...64]", "Max simultaneous samples"},
		{CommandNames::s_maxCachedSampleSize,   0, "[64...128]", "Max size (KB) of a cached sample"},
		{CommandNames::s_maxCacheSize,          0, "[4...16]", "Max cache size (MB) for samples"},
		{CommandNames::s_stopAllSounds,         0, "", "Stops all playing sounds"},
#ifdef _DEBUG	
		{CommandNames::s_debug,                 0, "[0 | 1]", "Toggle audio debugging"},
		{CommandNames::s_playBufferedAudio,     2, "<sound|music> <filename>", "Read <filename> into a memory buffer and use the buffered <sound|music> sample to play it."},
#endif // _DEBUG
		{"", 0, "", ""} // this must be last
	};

#ifdef _DEBUG
	static char * s_bufferSound = 0;
	static char * s_bufferMusic = 0;
#endif
}

using namespace SwgCuiCommandParserAudioNamespace;

//-----------------------------------------------------------------

SwgCuiCommandParserAudio::SwgCuiCommandParserAudio()
 : CommandParser ("audio", 0, "...", "audio commands", 0)
{
	createDelegateCommands(SwgCuiCommandParserAudioNamespace::cmds);
}

//-----------------------------------------------------------------

SwgCuiCommandParserAudio::~SwgCuiCommandParserAudio()
{
#ifdef _DEBUG
	if (s_bufferSound)
	{
		delete [] s_bufferSound;
		s_bufferSound = 0;
	}

	if (s_bufferMusic)
	{
		delete [] s_bufferMusic;
		s_bufferMusic = 0;
	}
#endif
}

//-----------------------------------------------------------------

bool SwgCuiCommandParserAudio::performParsing(const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node)
{
	UNREF(node);
	UNREF(originalCommand);
	UNREF(userId);

	//-----------------------------------------------------------------
	
	if (isCommand(argv[0], SwgCuiCommandParserAudioNamespace::CommandNames::s_enabled))
	{
		if (argv.size() > 1)
		{
			bool const enabled = (argv[1][0] == '0');

			Audio::setEnabled(enabled);
		}
	
		char text[256];
		_snprintf(text, sizeof(text), "Audio enabled: %s\n", Audio::isEnabled() ? "yes" : "no");
		result += Unicode::narrowToWide(text);
		return true;
	}

	//-----------------------------------------------------------------
	
	else if (isCommand(argv[0], SwgCuiCommandParserAudioNamespace::CommandNames::s_masterVolume))
	{
		if (argv.size() > 1)
		{
			float const volume = clamp(0.0f, static_cast<float>(atof(Unicode::wideToNarrow(argv[1]).c_str())), 1.0f);
			Audio::setMasterVolume(volume);
		}
	
		char text[256];
		_snprintf(text, sizeof(text), "Audio master volume: %.2f\n", Audio::getMasterVolume());
		result += Unicode::narrowToWide(text);
		return true;
	}
	
	//-----------------------------------------------------------------
	
	else if (isCommand(argv[0], SwgCuiCommandParserAudioNamespace::CommandNames::s_soundEffectVolume))
	{
		if (argv.size() > 1)
		{
			float const volume = clamp(0.0f, static_cast<float>(atof(Unicode::wideToNarrow(argv[1]).c_str())), 1.0f);
			Audio::setSoundEffectVolume(volume);
		}
	
		char text[256];
		_snprintf(text, sizeof(text), "Audio sound effect volume: %.2f\n", Audio::getSoundEffectVolume());
		result += Unicode::narrowToWide(text);
		return true;
	}

	//-----------------------------------------------------------------
	
	else if (isCommand(argv[0], SwgCuiCommandParserAudioNamespace::CommandNames::s_backGroundMusicVolume))
	{
		if (argv.size() > 1)
		{
			float const volume = clamp(0.0f, static_cast<float>(atof(Unicode::wideToNarrow(argv[1]).c_str())), 1.0f);
			Audio::setBackGroundMusicVolume(volume);
		}
	
		char text[256];
		_snprintf(text, sizeof(text), "Audio background music volume: %.2f\n", Audio::getBackGroundMusicVolume());
		result += Unicode::narrowToWide(text);
		return true;
	}

	//-----------------------------------------------------------------
	
	else if (isCommand(argv[0], SwgCuiCommandParserAudioNamespace::CommandNames::s_playerMusicVolume))
	{
		if (argv.size() > 1)
		{
			float const volume = clamp(0.0f, static_cast<float>(atof(Unicode::wideToNarrow(argv[1]).c_str())), 1.0f);
			Audio::setPlayerMusicVolume(volume);
		}
	
		char text[256];
		_snprintf(text, sizeof(text), "Audio player music volume: %.2f\n", Audio::getPlayerMusicVolume());
		result += Unicode::narrowToWide(text);
		return true;
	}

	//-----------------------------------------------------------------
	
	else if (isCommand(argv [0], SwgCuiCommandParserAudioNamespace::CommandNames::s_userInterfaceVolume))
	{
		if (argv.size() > 1)
		{
			float const volume = clamp(0.0f, static_cast<float>(atof(Unicode::wideToNarrow(argv[1]).c_str())), 1.0f);
			Audio::setUserInterfaceVolume(volume);
		}
	
		char text[256];
		_snprintf(text, sizeof(text), "Audio user interface volume: %.2f\n", Audio::getUserInterfaceVolume());
		result += Unicode::narrowToWide(text);
		return true;
	}

	//-----------------------------------------------------------------
	
	else if (isCommand(argv [0], SwgCuiCommandParserAudioNamespace::CommandNames::s_maxSamples))
	{
		if (argv.size() > 1)
		{
			int const maxSampleCount = atoi(Unicode::wideToNarrow(argv[1]).c_str());
			Audio::setRequestedMaxNumberOfSamples(maxSampleCount);
		}
	
		char text[256];
		_snprintf(text, sizeof(text), "Audio requested max number of samples: %d\n", Audio::getRequestedMaxNumberOfSamples());
		result += Unicode::narrowToWide(text);
		return true;
	}

	//-----------------------------------------------------------------
	
	else if (isCommand(argv [0], SwgCuiCommandParserAudioNamespace::CommandNames::s_maxCachedSampleSize))
	{
		if (argv.size() > 1)
		{
			int const maxCached2dSampleSize = clamp(64, atoi(Unicode::wideToNarrow(argv[1]).c_str()), 128) * 1024;
			Audio::setMaxCached2dSampleSize(maxCached2dSampleSize);
		}
	
		char text[256];
		_snprintf(text, sizeof(text), "Audio max cached sample size: %d KB\n", Audio::getMaxCached2dSampleSize() / 1024);
		result += Unicode::narrowToWide(text);
		return true;
	}

	//-----------------------------------------------------------------
	
	else if (isCommand(argv [0], SwgCuiCommandParserAudioNamespace::CommandNames::s_maxCacheSize))
	{
		if (argv.size() > 1)
		{
			//int const maxCacheSize = clamp(4, atoi(Unicode::wideToNarrow(argv[1]).c_str()), 16) * 1024 * 1024;
			//Audio::setMaxCacheSize(maxCacheSize);
		}
	
		//char text[256];
		//_snprintf(text, sizeof(text), "Audio max cache size: %d MB\n", Audio::getMaxCacheSize() / 1024 / 1024);
		//result += Unicode::narrowToWide(text);
		return true;
	}

	else if (isCommand(argv [0], SwgCuiCommandParserAudioNamespace::CommandNames::s_stopAllSounds))
	{
		float fadeOutTime = 0.0f;

		if (argv.size() > 1)
		{
			fadeOutTime = clamp(0.0f, static_cast<float>(atof(Unicode::wideToNarrow(argv[1]).c_str())), 16.0f);
		}

		result += Unicode::narrowToWide(FormattedString<1024>().sprintf("Audio::stopAllSounds(fadeOutTime = %.2f)", fadeOutTime));

		Audio::stopAllSounds(fadeOutTime);

		return true;
	}
	
	//-----------------------------------------------------------------

#ifdef _DEBUG	
	else if (isCommand(argv [0], SwgCuiCommandParserAudioNamespace::CommandNames::s_debug))
	{
		if (argv.size() > 1)
		{
			bool const enabled = (argv[1][0] == '1');

			Audio::setDebugEnabled(enabled);
		}
	
		char text[256];
		_snprintf(text, sizeof(text), "Audio debug %s\n", Audio::isDebugEnabled() ? "ON" : "OFF");
		result += Unicode::narrowToWide(text);
		return true;
	}

	else if (isCommand(argv [0], SwgCuiCommandParserAudioNamespace::CommandNames::s_playBufferedAudio))
	{
		int const bufferSize = 4*1024*1024;

		std::string filename = Unicode::wideToNarrow(argv[2]);

		FILE * audioFile = fopen(filename.c_str(), "rb");

		if (!audioFile)
		{
			result += Unicode::narrowToWide("Unable to open file\n");
			return true;
		}

		bool const playSound = argv[1][0] == 's' || argv[1][0] == 'S';
		std::string const extension = filename.substr(filename.length() - 4);
		char * & buffer = playSound ? s_bufferSound : s_bufferMusic;

		if (!buffer)
			buffer = new char[bufferSize];

		if (playSound)
			Audio::stopBufferedSound();
		else
			Audio::stopBufferedMusic();

		int readBytes = fread(buffer, sizeof(char), bufferSize, audioFile);

		fclose(audioFile);

		if (readBytes == bufferSize)
		{
			result += Unicode::narrowToWide("File too large\n");
			return true;
		}

		if (playSound)
			Audio::playBufferedSound(buffer, readBytes, extension.c_str());
		else
			Audio::playBufferedMusic(buffer, readBytes, extension.c_str());

		return true;
	}

#endif // _DEBUG

	return false;
}

// ======================================================================
