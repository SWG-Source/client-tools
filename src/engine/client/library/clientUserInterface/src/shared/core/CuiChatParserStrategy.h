// ======================================================================
//
// CuiChatParserStrategy.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CuiChatParserStrategy_H
#define INCLUDED_CuiChatParserStrategy_H

// ======================================================================

#include "clientUserInterface/CuiConsoleHelper.h"

class CommandParserHistory;
class CommandParser;
class CuiSocialsParser;
class CuiCommandTableParser;

namespace MessageDispatch
{
	class Callback;
}

//----------------------------------------------------------------------

/**
* The CuiChatParserStrategy takes an input string and attempts to interpret it as
* chat.  An input string beginning with '/' is interpreted as a command for the command parsers.
*/

class CuiChatParserStrategy : public CuiConsoleHelperParserStrategy
{
public:
	explicit               CuiChatParserStrategy (CommandParser * parser, bool globalCommandParser);
	                      ~CuiChatParserStrategy ();
	bool                   parse            (const Unicode::String & str, Unicode::String & result) const;
	bool                   tabCompleteToken (const Unicode::String & str, size_t tokenPos, stdvector<Unicode::String>::fwd & results, size_t & token_start, size_t & token_end) const;
	CommandParserHistory * getHistory () const;

	void                   setChatRoomId  (uint32 id);
	void                   setUseChatRoom (bool b);

	void                   onCommandParserRequest (std::pair<const std::string, bool> & payload);

private:

	CommandParser *                m_parser;
	CuiSocialsParser *             m_socialsParser;
	CuiCommandTableParser *        m_commandTableParser;

	uint32                         m_chatRoomId;
	bool                           m_useChatRoom;

	MessageDispatch::Callback *    m_callback;

private:
	CuiChatParserStrategy ();
	CuiChatParserStrategy (const CuiChatParserStrategy & rhs);
	CuiChatParserStrategy & operator= (const CuiChatParserStrategy & rhs);
};

// ======================================================================

#endif
