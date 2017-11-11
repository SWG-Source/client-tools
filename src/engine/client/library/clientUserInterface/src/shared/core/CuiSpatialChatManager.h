// ======================================================================
//
// CuiSpatialChatManager.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CuiSpatialChatManager_H
#define INCLUDED_CuiSpatialChatManager_H
// ======================================================================

#include "sharedMessageDispatch/Message.h"

class ChatSendText;
class LocalizedStringTable;
class MessageQueueSpatialChat;
class NetworkId;

//-----------------------------------------------------------------

/**
* The CuiSpatialChatManager handles the transmission and receipt of chat messages
* to and from the server.  
*
* During transmission of a chat message, the CuiSpatialChatManager enqueues the
* emote appropriate to the chat message, if any.
*
* During receipt of a chat message, the CuiSpatialChatManager takes the raw chat
* message and formats it into the appropriate style (prose, diku, IRC, etc...)
* The CuiSpatialChatManager triggers the creation or update of any graphical chat
* bubbles, if necessay.
* The CuiSpatialChatManager then dispatches the formatted text to the appropriate
* UI elements (console, chat box) 
* 
*/

class CuiSpatialChatManager
{
public:

	struct Messages
	{
		static const char * const CHAT_RECEIVED;
	};

	typedef MessageDispatch::Message<int>             ChatInputCharacterMsg;
	typedef MessageDispatch::Message<Unicode::String> ChatReceivedMsg;

	typedef stdvector<Unicode::String>::fwd           StringVector;



	static void                          install               ();
	static void                          remove                ();

	static void                          processMessage        (const MessageQueueSpatialChat & spatialChat, bool chatBoxOk = true, bool chatBubbleOk = true);
	static void                          sendMessage           (const NetworkId & targetId, uint32 chatType, uint32 mood, const Unicode::String & str, bool isPrivate, bool skipSource, uint32 const language);

	static Unicode::String               getChatVerbPhrase     (uint32 chatType);

	static uint32                        findChatTypeByLocalizedAbbrev (const Unicode::String & abbrev);

	static void                          getLocalizedChatTypeNames     (StringVector & sv);

private:
	                                     CuiSpatialChatManager        ();
	                                     CuiSpatialChatManager        (const CuiSpatialChatManager & rhs);
	CuiSpatialChatManager &                     operator=             (const CuiSpatialChatManager & rhs);
};

// ======================================================================

#endif
