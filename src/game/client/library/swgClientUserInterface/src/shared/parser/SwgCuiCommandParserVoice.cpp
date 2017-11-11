//======================================================================
//
// SwgCuiCommandParserVoice.cpp
// copyright (c) 2008 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiCommandParserVoice.h"

#include "UIUtils.h"
#include "clientUserInterface/CuiVoiceChatManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "sharedFoundation/NetworkId.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"

#include "StringId.h"

#include <vector>
#include <sstream>

//======================================================================


namespace SwgCuiCommandParserVoiceNamespace
{
	static StringId const mustBeSubscribed("ui_voice", "must_subscribe");


	namespace Commands
	{
#define MAKE_COMMAND(a) const char * const a = #a

		MAKE_COMMAND (showVoiceUsers);
		MAKE_COMMAND (hideVoiceUsers);
		MAKE_COMMAND (showBar);
		MAKE_COMMAND (hideBar);
		MAKE_COMMAND (pushtotalkDown);
		MAKE_COMMAND (pushtotalkUp);
		MAKE_COMMAND (setSelectedChannel);
		MAKE_COMMAND (info);
		MAKE_COMMAND (invite);
		MAKE_COMMAND (kick);
		MAKE_COMMAND (join);

#ifdef _DEBUG
		MAKE_COMMAND (leave);
		MAKE_COMMAND (registerChannel);
		MAKE_COMMAND (unregisterChannel);
		MAKE_COMMAND (setListening);
		MAKE_COMMAND (setSpeaking);
		//MAKE_COMMAND (toggleMicMute);
		MAKE_COMMAND (setdebuglevel);
		MAKE_COMMAND (mute);
		MAKE_COMMAND (unmute);
		MAKE_COMMAND (muteMic);
		MAKE_COMMAND (unmuteMic);
#endif

#undef MAKE_COMMAND
	}

	const CommandParser::CmdInfo cmds[] =
	{
		{ Commands::showVoiceUsers,       0, "", "Show the voice users window"},
		{ Commands::hideVoiceUsers,       0, "", "Hide the voice users window"},
		{ Commands::showBar,              0, "", "Show the voice bar"},
		{ Commands::hideBar,              0, "", "Hide the voice bar"},
		{ Commands::pushtotalkDown,       0, "", "Start transmitting in push to talk mode"},
		{ Commands::pushtotalkUp,         0, "", "Stop transmitting in push to talk mode"},
		{ Commands::setSelectedChannel,   1, "<roomName>", "Set which channel you are connected to"},
		{ Commands::info,                 0, "", "Dump current voice chat info"},
		{ Commands::invite,               1, "<player name>", "Invite another player to your private voice chat room"},
		{ Commands::kick,                 1, "<player name>", "Kick a player from your private voice chat room"},
		{ Commands::join,                 1, "<roomName>", "Join a public channel"},

#ifdef _DEBUG
		{ Commands::leave,                1, "<roomName>", "Leave a channel"},
		{ Commands::registerChannel,      2, "<roomName> <uri> [password]", "Enter channel info"},
		{ Commands::unregisterChannel,    1, "<roomName>", "Removes a channel info entry"},		
		{ Commands::setListening,         1, "<roomName> [isListening]", "Starts or stops listening to a channel"},
		{ Commands::setSpeaking,          1, "<roomName>", "Sets the current speaking to channel"},
		//{ Commands::toggleMicMute,        0, "", "Toggle the local microphone mute state"},
		{ Commands::setdebuglevel,        1, "<level>", "Set the debug output level (higher=more)"},
		{ Commands::mute,                 0, "", "Mute voice chat"},
		{ Commands::unmute,               0, "", "Unmute voice chat"},
		{ Commands::muteMic,              0, "", "Mute the microphone"},
		{ Commands::unmuteMic,            0, "", "Unmute the microphone"},
#endif
		{ "",                             0, "",                                   ""}, // this must be last
	};

	bool processShowActiveSpeakers(const NetworkId & id, const CommandParser::StringVector_t & argv, const Unicode::String & originalCommand, Unicode::String & result, const CommandParser * node);
	bool processHideActiveSpeakers(const NetworkId & id, const CommandParser::StringVector_t & argv, const Unicode::String & originalCommand, Unicode::String & result, const CommandParser * node);
	bool processInfo(const NetworkId & id, const CommandParser::StringVector_t & argv, const Unicode::String & originalCommand, Unicode::String & result, const CommandParser * node);
	bool processShowFlybar(const NetworkId & id, const CommandParser::StringVector_t & argv, const Unicode::String & originalCommand, Unicode::String & result, const CommandParser * node);
	bool processHideFlybar(const NetworkId & id, const CommandParser::StringVector_t & argv, const Unicode::String & originalCommand, Unicode::String & result, const CommandParser * node);
	bool processPushToTalkUp(const NetworkId & id, const CommandParser::StringVector_t & argv, const Unicode::String & originalCommand, Unicode::String & result, const CommandParser * node);
	bool processPushToTalkDown(const NetworkId & id, const CommandParser::StringVector_t & argv, const Unicode::String & originalCommand, Unicode::String & result, const CommandParser * node);
	bool processSetSelectedChannel(const NetworkId & id, const CommandParser::StringVector_t & argv, const Unicode::String & originalCommand, Unicode::String & result, const CommandParser * node);
	bool processInvite(const NetworkId & id, const CommandParser::StringVector_t & argv, const Unicode::String & originalCommand, Unicode::String & result, const CommandParser * node);
	bool processKick(const NetworkId & id, const CommandParser::StringVector_t & argv, const Unicode::String & originalCommand, Unicode::String & result, const CommandParser * node);
	bool processJoin(const NetworkId & id, const CommandParser::StringVector_t & argv, const Unicode::String & originalCommand, Unicode::String & result, const CommandParser * node);

//These functions are not registered in release
	bool processLeave(const NetworkId & id, const CommandParser::StringVector_t & argv, const Unicode::String & originalCommand, Unicode::String & result, const CommandParser * node);
	bool processRegisterChannel(const NetworkId & id, const CommandParser::StringVector_t & argv, const Unicode::String & originalCommand, Unicode::String & result, const CommandParser * node);
	bool processUnregisterChannel(const NetworkId & id, const CommandParser::StringVector_t & argv, const Unicode::String & originalCommand, Unicode::String & result, const CommandParser * node);
	bool processSetListening(const NetworkId & id, const CommandParser::StringVector_t & argv, const Unicode::String & originalCommand, Unicode::String & result, const CommandParser * node);
	bool processSetSpeaking(const NetworkId & id, const CommandParser::StringVector_t & argv, const Unicode::String & originalCommand, Unicode::String & result, const CommandParser * node);
	//bool processToggleMicMute(const NetworkId & id, const CommandParser::StringVector_t & argv, const Unicode::String & originalCommand, Unicode::String & result, const CommandParser * node);
	bool processSetDebugLevel(const NetworkId & id, const CommandParser::StringVector_t & argv, const Unicode::String & originalCommand, Unicode::String & result, const CommandParser * node);
	bool processMute(const NetworkId & id, const CommandParser::StringVector_t & argv, const Unicode::String & originalCommand, Unicode::String & result, const CommandParser * node);
	bool processUnmute(const NetworkId & id, const CommandParser::StringVector_t & argv, const Unicode::String & originalCommand, Unicode::String & result, const CommandParser * node);
	bool processMuteMic(const NetworkId & id, const CommandParser::StringVector_t & argv, const Unicode::String & originalCommand, Unicode::String & result, const CommandParser * node);
	bool processUnmuteMic(const NetworkId & id, const CommandParser::StringVector_t & argv, const Unicode::String & originalCommand, Unicode::String & result, const CommandParser * node);

}

using namespace SwgCuiCommandParserVoiceNamespace;

//----------------------------------------------------------------------


SwgCuiCommandParserVoice::SwgCuiCommandParserVoice () :
CommandParser   ("voice", 0, "...", "Voice chat commands", 0)
{
	createDelegateCommands (cmds);

#define DECLARE_PROCESSOR(name,func)m_processorMap[Unicode::narrowToWide(Commands::##name)] = &func

	DECLARE_PROCESSOR(showVoiceUsers, processShowActiveSpeakers);
	DECLARE_PROCESSOR(hideVoiceUsers, processHideActiveSpeakers);
	DECLARE_PROCESSOR(showBar, processShowFlybar);
	DECLARE_PROCESSOR(hideBar, processHideFlybar);
	DECLARE_PROCESSOR(pushtotalkDown, processPushToTalkDown);
	DECLARE_PROCESSOR(pushtotalkUp, processPushToTalkUp);
	DECLARE_PROCESSOR(setSelectedChannel, processSetSelectedChannel);
	DECLARE_PROCESSOR(info, processInfo);
	DECLARE_PROCESSOR(invite, processInvite);
	DECLARE_PROCESSOR(kick, processKick);
	DECLARE_PROCESSOR(join, processJoin);

#ifdef _DEBUG
	DECLARE_PROCESSOR(leave, processLeave);
	DECLARE_PROCESSOR(registerChannel, processRegisterChannel);
	DECLARE_PROCESSOR(unregisterChannel, processUnregisterChannel);
	DECLARE_PROCESSOR(setSpeaking, processSetSpeaking);
	DECLARE_PROCESSOR(setListening, processSetListening);
	//DECLARE_PROCESSOR(toggleMicMute,processToggleMicMute);
	DECLARE_PROCESSOR(setdebuglevel, processSetDebugLevel);
	DECLARE_PROCESSOR(mute, processMute);
	DECLARE_PROCESSOR(unmute, processUnmute);
	DECLARE_PROCESSOR(muteMic, processMuteMic);
	DECLARE_PROCESSOR(unmuteMic, processUnmuteMic);
#endif

#undef DECLARE_PROCESSOR
}

//----------------------------------------------------------------------

bool SwgCuiCommandParserVoice::performParsing  (const NetworkId & id, const StringVector_t & argv, const Unicode::String & originalCommand, Unicode::String & result, const CommandParser * node)
{
	ProcessorMap::iterator i = m_processorMap.find(argv[0]);
	if(i != m_processorMap.end())
	{
		return (*(i->second))(id, argv, originalCommand, result, node);
	}

	return false;
}

//----------------------------------------------------------------------

bool SwgCuiCommandParserVoiceNamespace::processInfo(const NetworkId & , const CommandParser::StringVector_t &, const Unicode::String &, Unicode::String & result, const CommandParser *)
{
	std::string outString = "Dumping Voice Chat Info...\n";
	CuiVoiceChatManager::dumpDebugInfo(outString);
	result += Unicode::narrowToWide(outString);

	return true;
}

//----------------------------------------------------------------------

bool SwgCuiCommandParserVoiceNamespace::processRegisterChannel(const NetworkId &, const CommandParser::StringVector_t & argv, const Unicode::String &, Unicode::String &, const CommandParser *)
{
	CuiVoiceChatManager::VoiceChatChannelData data;

	data.name = Unicode::wideToNarrow(argv[1]);
	data.uri = Unicode::wideToNarrow(argv[2]);
	data.password = argv.size() > 3 ? Unicode::wideToNarrow(argv[3]) : "";

	CuiVoiceChatManager::getInstance().enterChannelData(data, true);

	return true;
}

//----------------------------------------------------------------------

bool SwgCuiCommandParserVoiceNamespace::processUnregisterChannel(const NetworkId &, const CommandParser::StringVector_t & argv, const Unicode::String &, Unicode::String &, const CommandParser *)
{	
	CuiVoiceChatManager::getInstance().eraseChannelData(Unicode::wideToNarrow(argv[1]));
	return true;
}

//----------------------------------------------------------------------

bool SwgCuiCommandParserVoiceNamespace::processJoin(const NetworkId &, const CommandParser::StringVector_t & argv, const Unicode::String &, Unicode::String & result, const CommandParser *)
{
	CuiVoiceChatManager::requestJoinChannel(Unicode::wideToNarrow(argv[1]));

	result += Unicode::narrowToWide("Attempting to join channel ") + argv[1];
	return true;
}

//----------------------------------------------------------------------

bool SwgCuiCommandParserVoiceNamespace::processLeave(const NetworkId &, const CommandParser::StringVector_t & argv, const Unicode::String &, Unicode::String &, const CommandParser *)
{
	CuiVoiceChatManager::requestLeaveChannel(Unicode::wideToNarrow(argv[1]));
	return true;
}

//----------------------------------------------------------------------

bool SwgCuiCommandParserVoiceNamespace::processSetListening(const NetworkId &, const CommandParser::StringVector_t & argv, const Unicode::String &, Unicode::String &, const CommandParser *)
{
	bool listen = true;
	if(argv.size() > 2)
	{
		listen = argv[2][0] == '1';
	}

	CuiVoiceChatManager::setListeningToChannel(Unicode::wideToNarrow(argv[1]), listen);
	return true;
}

//----------------------------------------------------------------------

bool SwgCuiCommandParserVoiceNamespace::processSetSpeaking(const NetworkId &, const CommandParser::StringVector_t & argv, const Unicode::String &, Unicode::String &, const CommandParser *)
{
	CuiVoiceChatManager::setSpeakingToChannel(Unicode::wideToNarrow(argv[1]));
	return true;
}

//----------------------------------------------------------------------

bool SwgCuiCommandParserVoiceNamespace::processSetSelectedChannel(const NetworkId &, const CommandParser::StringVector_t & argv, const Unicode::String &, Unicode::String &, const CommandParser *)
{
	CuiVoiceChatManager::setChannelSimpleMode(Unicode::wideToNarrow(argv[1]));
	return true;
}

//----------------------------------------------------------------------

bool SwgCuiCommandParserVoiceNamespace::processShowActiveSpeakers(const NetworkId &, const CommandParser::StringVector_t &, const Unicode::String &, Unicode::String &, const CommandParser *)
{
	CuiMediatorFactory::activateInWorkspace(CuiMediatorTypes::WS_VoiceActiveSpeakers);
	return true;
}

//----------------------------------------------------------------------

bool SwgCuiCommandParserVoiceNamespace::processHideActiveSpeakers(const NetworkId &, const CommandParser::StringVector_t &, const Unicode::String &, Unicode::String &, const CommandParser *)
{
	CuiMediatorFactory::deactivateInWorkspace(CuiMediatorTypes::WS_VoiceActiveSpeakers);
	return true;
}

//----------------------------------------------------------------------

bool SwgCuiCommandParserVoiceNamespace::processShowFlybar(const NetworkId &, const CommandParser::StringVector_t &, const Unicode::String &, Unicode::String &, const CommandParser *)
{
	CuiVoiceChatManager::setShowFlybar(true);
	return true;
}

//----------------------------------------------------------------------

bool SwgCuiCommandParserVoiceNamespace::processHideFlybar(const NetworkId &, const CommandParser::StringVector_t &, const Unicode::String &, Unicode::String &, const CommandParser *)
{
	CuiVoiceChatManager::setShowFlybar(false);
	return true;
}

//----------------------------------------------------------------------

bool SwgCuiCommandParserVoiceNamespace::processMute(const NetworkId &, const CommandParser::StringVector_t &, const Unicode::String &, Unicode::String &, const CommandParser *)
{
	CuiVoiceChatManager::setLocalSpeakerMute(true);
	return true;
}

//----------------------------------------------------------------------

bool SwgCuiCommandParserVoiceNamespace::processUnmute(const NetworkId &, const CommandParser::StringVector_t &, const Unicode::String &, Unicode::String &, const CommandParser *)
{
	CuiVoiceChatManager::setLocalSpeakerMute(false);
	return true;
}

//----------------------------------------------------------------------

bool SwgCuiCommandParserVoiceNamespace::processMuteMic(const NetworkId &, const CommandParser::StringVector_t &, const Unicode::String &, Unicode::String &, const CommandParser *)
{
	CuiVoiceChatManager::setLocalMicMute(true);
	return true;
}

//----------------------------------------------------------------------

bool SwgCuiCommandParserVoiceNamespace::processUnmuteMic(const NetworkId &, const CommandParser::StringVector_t &, const Unicode::String &, Unicode::String &, const CommandParser *)
{
	CuiVoiceChatManager::setLocalMicMute(false);
	return true;
}

////----------------------------------------------------------------------
//
//bool SwgCuiCommandParserVoiceNamespace::processToggleMicMute(const NetworkId &, const CommandParser::StringVector_t &, const Unicode::String &, Unicode::String &, const CommandParser *)
//{
//	CuiVoiceChatManager::setLocalMicMute(CuiVoiceChatManager::getLocalMicMute());
//	return true;
//}

//----------------------------------------------------------------------

bool SwgCuiCommandParserVoiceNamespace::processPushToTalkUp(const NetworkId &, const CommandParser::StringVector_t &, const Unicode::String &, Unicode::String &, const CommandParser *)
{
	CuiVoiceChatManager::pushToTalkKeyPressed(false);
	return true;
}

//----------------------------------------------------------------------

bool SwgCuiCommandParserVoiceNamespace::processPushToTalkDown(const NetworkId &, const CommandParser::StringVector_t &, const Unicode::String &, Unicode::String &, const CommandParser *)
{
	CuiVoiceChatManager::pushToTalkKeyPressed(true);
	return true;
}

//----------------------------------------------------------------------

bool SwgCuiCommandParserVoiceNamespace::processSetDebugLevel(const NetworkId &, const CommandParser::StringVector_t & argv, const Unicode::String &, Unicode::String & result, const CommandParser *)
{
	int level(strtoul(Unicode::wideToNarrow (argv[1]).c_str (), NULL, 10));

	CuiVoiceChatManager::setDebugOutputLevel(static_cast<CuiVoiceChatManager::DebugOutputLevel>(level));

	std::stringstream ss;
	ss << "Voice chat debug level set to " << level;

	result += Unicode::narrowToWide(ss.str());

	return true;
}

//----------------------------------------------------------------------

bool SwgCuiCommandParserVoiceNamespace::processInvite(const NetworkId &, const CommandParser::StringVector_t & argv, const Unicode::String &, Unicode::String & result, const CommandParser *)
{
	if(CuiVoiceChatManager::getVoiceChatDisabledByServer())
	{
		result += mustBeSubscribed.localize();
		return false;
	}

	std::string const who = Unicode::wideToNarrow(argv[1]);
	CuiVoiceChatManager::channelInvite(NetworkId(), who, CuiVoiceChatManager::getMyPrivateChannelName());
	return true;
}

//----------------------------------------------------------------------

bool SwgCuiCommandParserVoiceNamespace::processKick(const NetworkId &, const CommandParser::StringVector_t & argv, const Unicode::String &, Unicode::String &, const CommandParser *)
{
	std::string const who = Unicode::wideToNarrow(argv[1]);
	CuiVoiceChatManager::channelKick(NetworkId(), who, CuiVoiceChatManager::getMyPrivateChannelName());
	return true;
}

//======================================================================
