// ======================================================================
//
// SwgCuiCommandParserVideo.cpp
// Copyright Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiCommandParserVideo.h"

#include "clientGraphics/Video.h"
#include "clientGraphics/VideoList.h"
#include "clientGraphics/VideoPlaybackManager.h"
#include "clientGame/Game.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/NetworkId.h"
#include <cstdio>
#include <vector>

// ======================================================================

namespace SwgCuiCommandParserVideoNamespace
{
	namespace CommandNames
	{
		const char * const s_playVideo             = "play";
	}

	const CommandParser::CmdInfo cmds[] =
	{
		{CommandNames::s_playVideo,             0, "<video filename>", "Play a video."},
		{"", 0, "", ""} // this must be last
	};
}

//-----------------------------------------------------------------

SwgCuiCommandParserVideo::SwgCuiCommandParserVideo()
 : CommandParser ("video", 0, "...", "video commands", 0)
{
	createDelegateCommands(SwgCuiCommandParserVideoNamespace::cmds);
}
//-----------------------------------------------------------------

bool SwgCuiCommandParserVideo::performParsing(const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node)
{
	UNREF(node);
	UNREF(originalCommand);
	UNREF(userId);

	//-----------------------------------------------------------------

	if (isCommand(argv [0], SwgCuiCommandParserVideoNamespace::CommandNames::s_playVideo))
	{
		if (argv.size()==2)
		{
			const std::string fileName = Unicode::wideToNarrow (argv [1]);

			Game::playCutScene(fileName.c_str(), false);

			result += Unicode::narrowToWide ("Playing cut-scene.\n");
			
			return true;
		}
		else
		{
			result += Unicode::narrowToWide ("Error playing cut-scene.\n");
		}
	}
	
	//-----------------------------------------------------------------

	return false;
}

// ======================================================================
