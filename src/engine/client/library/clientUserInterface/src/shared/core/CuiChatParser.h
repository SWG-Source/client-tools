// ======================================================================
//
// CuiChatParser.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CuiChatParser_H
#define INCLUDED_CuiChatParser_H

// ======================================================================

class Object;
class LocalizedStringTable;

/**
* The CuiChatParser interprets an input string, determining mood, spatial chat type,
* and target and sends a formatted chat messages to the CuiChatManager if applicable.
* 
*/

class CuiChatParser
{
public:

	static void           install ();
	static void           remove  ();

	static bool           parse (const Unicode::String & cmd, Unicode::String & result, uint32 chatRoomId, bool useChatRoom);

	static Unicode::unicode_char_t      getCmdChar ();

	static uint32                       getEmoteChatType ();

private:
	                      CuiChatParser ();
	                      CuiChatParser (const CuiChatParser & rhs);
	CuiChatParser &       operator=    (const CuiChatParser & rhs);

	static Unicode::unicode_char_t  ms_cmdChar;

	static uint32                   ms_emoteChatType;
};

//-----------------------------------------------------------------

inline Unicode::unicode_char_t CuiChatParser::getCmdChar ()
{
	return ms_cmdChar;
}

//----------------------------------------------------------------------

inline uint32 CuiChatParser::getEmoteChatType ()
{
	return ms_emoteChatType;
}

// ======================================================================

#endif
