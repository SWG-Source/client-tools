//======================================================================
//
// CuiInstantMessageManager.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiInstantMessageManager_H
#define INCLUDED_CuiInstantMessageManager_H

//======================================================================

#include <string>

#include "sharedNetworkMessages/ChatAvatarId.h"

//----------------------------------------------------------------------

class CuiInstantMessageManagerElement
{
public:
	ChatAvatarId     avatarId;
	Unicode::String  message;
};

//----------------------------------------------------------------------

class CuiInstantMessageManager
{
public:

	typedef CuiInstantMessageManagerElement MessageElement;

	struct Messages
	{
		struct MessageReceived
		{
			typedef MessageElement Payload;
		};

		struct MessageFailed
		{
			typedef Unicode::String Payload;
		};
	};

	static void   install ();
	static void   remove  ();

	static bool   sendMessageToCharacter (const std::string & targetName, const Unicode::String & message, const Unicode::String & outOfBand, bool isReply = false);
	static void   sendMessageToCharacter (const ChatAvatarId & avatarId,  const Unicode::String & message, const Unicode::String & outOfBand, bool isReply = false);

	static void   receiveMessage         (const ChatAvatarId & sender, const Unicode::String & message);

	static bool   tell                   (const Unicode::String & str, Unicode::String & result);
	static bool   reply                  (const Unicode::String & str, Unicode::String & result);
	static bool   retell                 (const Unicode::String & str, Unicode::String & result);
	static bool   targetedTell           (const Unicode::String & str, Unicode::String & result);

	static ChatAvatarId const & getLastChatId();
 
public:

	static const Unicode::String       ms_cmdSend;
	static const Unicode::String       ms_cmdTell;
	static const Unicode::String       ms_cmdRetell;
	static const Unicode::String       ms_cmdReply;
	static const Unicode::String       ms_cmdTargetedTell;
};

//======================================================================

#endif
