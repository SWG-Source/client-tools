// ======================================================================
//
// CuiChatFormatter.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CuiChatFormatter_H
#define INCLUDED_CuiChatFormatter_H


// ======================================================================

class ChatSendText;
class ClientObject;

class MessageQueueSpatialChat;

//-----------------------------------------------------------------
/**
* The CuiChatFormatter is a static utility class that knows how to convert
* a raw chat message into the appropriate formatted text to be displayed
* in the UI.
*
* If Jake says happily, 'Hello.  My Name is Jake.'
*
* Possible styles are:
*
* Default:  
*           Jake:  Hello. My Name Is Jake.
* Diku:
*           Jake says happily, 'Hello.  My Name is Jake.'
* Prose:
*           'Hello.', Jake says happily, 'My Name is Jake.'
*/

class CuiChatFormatter
{
public:

	static void                install ();
	static void                remove ();

	static void                formatDiku  (const MessageQueueSpatialChat & spatialChat, Unicode::String & str);
	static void                formatRaw   (const MessageQueueSpatialChat & spatialChat, Unicode::String & str);
	static void                formatProse (const MessageQueueSpatialChat & spatialChat, Unicode::String & str);
	static void                formatSpaceComm (const MessageQueueSpatialChat & spatialChat, Unicode::String & str);

private:
	                           CuiChatFormatter ();
	                           CuiChatFormatter (const CuiChatFormatter & rhs);
	CuiChatFormatter &         operator=    (const CuiChatFormatter & rhs);

	enum TextPosition
	{
		TP_First,
		TP_Last
	};
	static void                formatStandard  (const MessageQueueSpatialChat & spatialChat, Unicode::String & str, TextPosition position, bool randomMoodPhrase);
};

// ======================================================================

#endif
