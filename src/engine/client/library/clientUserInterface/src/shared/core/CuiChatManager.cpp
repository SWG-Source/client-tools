// ======================================================================
//
// CuiChatManager.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiChatManager.h"

#include "clientGame/ConfigClientGame.h"
#include "clientGame/Game.h"
#include "clientUserInterface/CuiStringVariablesData.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "clientUserInterface/ConfigClientUserInterface.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedNetworkMessages/ChatAvatarId.h"
#include "sharedUtility/Callback.h"
#include "sharedUtility/LocalMachineOptionManager.h"
#include "StringId.h"
#include "UIManager.h"

//----------------------------------------------------------------------

namespace
{
	Callback *        ms_chatFontSizeCallback;
	Callback *        ms_chatBoxKeyClickCallback;
	bool              ms_chatBoxTimestamp;

	int               ms_chatStyleDefault = CuiChatManager::CS_Diku;

	bool s_installed = false;
}

//----------------------------------------------------------------------

int                         CuiChatManager::ms_chatStyle                      = CuiChatManager::CS_Prose;
int                         CuiChatManager::ms_chatWindowFontSizeDefaultIndex = 0;
bool                        CuiChatManager::ms_chatBoxKeyClick                = false;

//----------------------------------------------------------------------

void CuiChatManager::install ()
{
	DEBUG_FATAL (s_installed, ("already installed"));
	ms_chatFontSizeCallback = new Callback;
	ms_chatFontSizeCallback->fetch ();
	ms_chatBoxKeyClickCallback = new Callback;
	ms_chatBoxKeyClickCallback->fetch ();

	bool const useJapanese = UIManager::gUIManager().isLocaleJapanese();
	if(useJapanese)
		ms_chatStyleDefault = CuiChatManager::CS_Brief;

	ms_chatWindowFontSizeDefaultIndex = ConfigClientUserInterface::getChatWindowFontSizeDefaultIndex ();
	ms_chatBoxKeyClick                = ConfigClientUserInterface::getChatBoxKeyClick ();
	ms_chatStyle                      = ms_chatStyleDefault;
	ms_chatBoxTimestamp               = ConfigClientUserInterface::getChatTimestampEnabled ();

	const char * const section = "CuiChatManager";
	LocalMachineOptionManager::registerOption (ms_chatWindowFontSizeDefaultIndex, section, "chatWindowFontSizeDefaultIndex");
	LocalMachineOptionManager::registerOption (ms_chatBoxKeyClick,                section, "chatBoxKeyClick");
	LocalMachineOptionManager::registerOption (ms_chatBoxTimestamp,               section, "chatBoxTimestamp");
	LocalMachineOptionManager::registerOption (ms_chatStyle,                      section, "chatStyle");
	
	s_installed = true;
}

//----------------------------------------------------------------------

void CuiChatManager::remove ()
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	ms_chatFontSizeCallback->release ();
	ms_chatFontSizeCallback = 0;
	ms_chatBoxKeyClickCallback->release ();
	ms_chatBoxKeyClickCallback = 0;

	s_installed = false;
}

//----------------------------------------------------------------------

void CuiChatManager::setChatStyle (ChatStyle cs)
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	ms_chatStyle = cs;
}

//----------------------------------------------------------------------

void CuiChatManager::getShortName            (const ChatAvatarId & avatarId, Unicode::String & str)
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	const ChatAvatarId  & selfId = CuiChatManager::getSelfAvatarId ();

	std::string tmp;

	const Unicode::String & namePart = StringId::decodeString (Unicode::narrowToWide (avatarId.name));

	if (avatarId.gameCode.empty () || !_stricmp (selfId.gameCode.c_str (), avatarId.gameCode.c_str ()))
	{
		if (avatarId.cluster.empty () || !_stricmp (selfId.cluster.c_str (), avatarId.cluster.c_str ()))
		{
			//-- just the name part
		}
		else
		{
			tmp = avatarId.cluster + '.';
		}
	}
	else	
		tmp = avatarId.gameCode + '.' + avatarId.cluster + '.';

	str = Unicode::narrowToWide (tmp) + namePart;
}

//----------------------------------------------------------------------

const Unicode::String CuiChatManager::getShortName            (const ChatAvatarId & avatarId)
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	Unicode::String str;
	getShortName (avatarId, str);
	return str;
}

//----------------------------------------------------------------------

const ChatAvatarId CuiChatManager::getSelfAvatarId         ()
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	std::string     loginId;
	std::string     myClusterName;
	NetworkId       id;
	Unicode::String playerName;

	if (Game::getPlayerPath (loginId, myClusterName, playerName, id))
	{
		std::string name = Unicode::wideToNarrow (playerName);
		if(name.find_first_of(" ") != name.npos)  //lint !e737
			name = name.substr(0, name.find_first_of(" "));

		return ChatAvatarId (ConfigClientGame::getGameChatCode (), myClusterName, name);
	}

	return ChatAvatarId ();
}

//----------------------------------------------------------------------

const bool CuiChatManager::constructChatAvatarId   (const std::string & name, ChatAvatarId & chatAvatar)
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	chatAvatar = ChatAvatarId (name);

	if (chatAvatar.name.empty ())
		return false;

	const ChatAvatarId  & selfId = CuiChatManager::getSelfAvatarId ();

	if (chatAvatar.gameCode.empty ())
		chatAvatar.gameCode = selfId.gameCode;

	if (chatAvatar.cluster.empty ())
		chatAvatar.cluster = selfId.cluster;

	return true;
}

//----------------------------------------------------------------------

const Unicode::String CuiChatManager::prosify (const ChatAvatarId & actor, const Unicode::String & otherText, const StringId & stringId)
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	Unicode::String shortName;
	CuiChatManager::getShortName (actor, shortName);

	Unicode::String localText = otherText;
	if (ms_chatStyle == CS_Prose)
	{
		shortName [0] = towupper (shortName [0]);
		localText [0] = towupper (localText [0]);
	}

	Unicode::String msg;
	CuiStringVariablesManager::process (stringId, shortName, Unicode::emptyString, localText, msg);
	return msg;
}

//----------------------------------------------------------------------

const Unicode::String CuiChatManager::prosify (const ChatAvatarId & actor, const StringId & stringId)
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	return prosify (actor, ChatAvatarId (), ChatAvatarId (), stringId);
}

//----------------------------------------------------------------------

const Unicode::String CuiChatManager::prosify (const ChatAvatarId & actor, const ChatAvatarId & target, const StringId & stringId)
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	return prosify (actor, target, ChatAvatarId (), stringId);
}

//----------------------------------------------------------------------

const Unicode::String CuiChatManager::prosify (const ChatAvatarId & actor, const ChatAvatarId & target, const ChatAvatarId & other, const StringId & stringId)
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	Unicode::String shortName;
	CuiChatManager::getShortName (actor, shortName);
	Unicode::String targetShortName;
	if (!target.name.empty ())
		CuiChatManager::getShortName (target, targetShortName);
	Unicode::String otherShortName;
	if (!other.name.empty ())
		CuiChatManager::getShortName (other, otherShortName);
	
	CuiStringVariablesData csvd;
	csvd.sourceName = shortName;
	csvd.targetName = targetShortName;
	csvd.otherName  = otherShortName;
	
	Unicode::String sentMsg;
	CuiStringVariablesManager::process (stringId, csvd, sentMsg);
	
	return sentMsg;
}

//----------------------------------------------------------------------

int CuiChatManager::getChatWindowFontSizeDefaultIndex ()
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	return ms_chatWindowFontSizeDefaultIndex;
}

//----------------------------------------------------------------------

int CuiChatManager::getChatWindowFontSizeDefaultSize ()
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	const ConfigClientUserInterface::IntVector & iv = ConfigClientUserInterface::getChatWindowFontSizes ();

	if (iv.empty () || ms_chatWindowFontSizeDefaultIndex < 0 || ms_chatWindowFontSizeDefaultIndex >= static_cast<int>(iv.size ()))
		return 0;

	return iv [ms_chatWindowFontSizeDefaultIndex];
}

//----------------------------------------------------------------------

void CuiChatManager::setChatWindowFontSizeDefaultIndex (int index)
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	index = std::min (index, ConfigClientUserInterface::getChatWindowFontSizesCount () - 1);
	index = std::max (0, index);
	ms_chatWindowFontSizeDefaultIndex = index;
	ms_chatFontSizeCallback->performCallback ();
}

//----------------------------------------------------------------------

bool CuiChatManager::getChatBoxKeyClick ()
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	return ms_chatBoxKeyClick;
}

//----------------------------------------------------------------------

void CuiChatManager::setChatBoxKeyClick (bool b)
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	ms_chatBoxKeyClick = b;
	ms_chatBoxKeyClickCallback->performCallback ();
}

//----------------------------------------------------------------------

Callback & CuiChatManager::getChatFontSizeCallback ()
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	return *ms_chatFontSizeCallback;
}

//----------------------------------------------------------------------

Callback & CuiChatManager::getChatBoxKeyClickCallback ()
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	return *ms_chatBoxKeyClickCallback;
}

//----------------------------------------------------------------------

bool CuiChatManager::isChatBrief                       ()
{
	return ms_chatStyle == CS_Brief;
}

//----------------------------------------------------------------------

bool CuiChatManager::isChatDiku                        ()
{
	return ms_chatStyle == CS_Diku;
}

//----------------------------------------------------------------------

bool CuiChatManager::isChatProse                       ()
{
	return ms_chatStyle == CS_Prose;
}

//----------------------------------------------------------------------

void CuiChatManager::setChatBrief                      (bool b)
{
	if (b)
		setChatStyle (CS_Brief);
}

//----------------------------------------------------------------------

void CuiChatManager::setChatDiku                       (bool b)
{
	if (b)
		setChatStyle (CS_Diku);
}

//----------------------------------------------------------------------

void CuiChatManager::setChatProse                      (bool b)
{
	if (b)
		setChatStyle (CS_Prose);
}

//----------------------------------------------------------------------

bool CuiChatManager::isChatBriefDefault                ()
{
	return ms_chatStyleDefault == CS_Brief;
}

//----------------------------------------------------------------------

bool CuiChatManager::isChatDikuDefault                 ()
{
	return ms_chatStyleDefault == CS_Diku;
}

//----------------------------------------------------------------------

bool CuiChatManager::isChatProseDefault                ()
{
	return ms_chatStyleDefault == CS_Prose;
}

//----------------------------------------------------------------------

bool CuiChatManager::getChatBoxTimestamp    ()
{
	return ms_chatBoxTimestamp;
}

//----------------------------------------------------------------------

void CuiChatManager::setChatBoxTimestamp    (bool b)
{
	ms_chatBoxTimestamp = b;
}

// ======================================================================
