// ======================================================================
//
// CuiChatParserStrategy.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiChatParserStrategy.h"

#include "clientGame/Game.h"
#include "clientUserInterface/CuiChatParser.h"
#include "clientUserInterface/CuiChatRoomManager.h"
#include "clientUserInterface/CuiCommandTableParser.h"
#include "clientUserInterface/CuiInstantMessageManager.h"
#include "clientUserInterface/CuiMessageQueueManager.h"
#include "clientUserInterface/CuiSocialsParser.h"
#include "sharedCommandParser/CommandParser.h"
#include "sharedCommandParser/CommandParserHistory.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedMessageDispatch/Transceiver.h"

#include "utf8.h"

#include <vector>

//lint -esym(534, basic_string<unsigned short,_STL::char_traits<unsigned short>,_STL::allocator<unsigned short>>::erase)
//lint -esym(534, std::basic_string<unsigned short,_STL::char_traits<unsigned short>,_STL::allocator<unsigned short>>::erase)
//lint -esym(534, _STL::basic_string<unsigned short,_STL::char_traits<unsigned short>,_STL::allocator<unsigned short>>::erase)
//lint -esym(534, basic_string::erase)
//lint -esym(534, std::basic_string::erase)
//lint -esym(534, _STL::basic_string::erase)

// ======================================================================

/**
* The CuiChatParserStrategy takes ownership of the parser object
* @param parser the strategy owns the parser and deletes it in the dtor
*/

CuiChatParserStrategy::CuiChatParserStrategy (CommandParser * parser, bool globalCommandParser) : 
CuiConsoleHelperParserStrategy (),  //lint !e1769 no ctor
m_parser                       (NON_NULL (parser)),
m_socialsParser                (NON_NULL (new CuiSocialsParser)),
m_commandTableParser           (NON_NULL (new CuiCommandTableParser)),
m_chatRoomId                   (0),
m_useChatRoom                  (false),
m_callback                     (globalCommandParser ? new MessageDispatch::Callback : 0)
{
	MEM_OWN(m_parser);

//	if (m_callback)
//		m_callback->connect (*this, &CuiChatParserStrategy::onCommandParserRequest, static_cast<CuiMessageQueueManager::Messages::CommandParserRequest *>(0));
}

//-----------------------------------------------------------------

CuiChatParserStrategy::~CuiChatParserStrategy ()
{
	delete m_commandTableParser;
	m_commandTableParser = 0;

	delete m_socialsParser;
	m_socialsParser = 0;

	delete m_parser;
	m_parser = 0;

	delete m_callback;
	m_callback = 0;
}

//----------------------------------------------------------------------

void CuiChatParserStrategy::onCommandParserRequest (CuiMessageQueueManager::Messages::CommandParserRequest::Payload & payload)
{
	Unicode::String result;

	if (parse (Unicode::utf8ToWide (payload.first), result))
		payload.second = true;
	else
		WARNING (true, ("CuiChatParserStrategy::onCommandParserRequest failed for '%s'. Result: '%s'", payload.first.c_str (), Unicode::wideToNarrow (result).c_str ()));

	if (!result.empty ())
		CuiChatRoomManager::sendPrelocalizedChat (result);
}  //lint !e1762 //const

//-----------------------------------------------------------------
/**
* @param str should come in pre-trimmed from the consolehelper
*/ 

bool CuiChatParserStrategy::parse (const Unicode::String & str, Unicode::String & result) const
{
	if (str.empty ())
	{
		result += Unicode::narrowToWide ("Empty command string.");
		return false;
	}

	if (CuiSocialsParser::preparseEmoticons (str, result))
		return true;

	//-- no handler means we pass it on to the chat parser
	if (str [0] == CuiChatParser::getCmdChar ())
	{
		const Unicode::String strippedString (str.substr (1));
		
		// The various command parsers will try to "auto-complete" a command.  So it is
		// possible for one parser to finish successfully on a partial match while
		// the command would be an exact match in another parser.
		//
		// Since an exact match should always "win", we will try to locate which
		// parser from each command table will do the the actual parsing.  If
		// a found parser corresponds to the exact command, we will try to use
		// the corresponding command table to do the actual parsing.

		// First, grab the first token since that corresponds to the command we are looking for
		size_t          endPos = 0;
		Unicode::String commandToken;
		if (Unicode::getFirstToken (strippedString, 0, endPos, commandToken))
		{
			std::vector<Unicode::String> tokenVector;
			tokenVector.push_back (commandToken);

			// Now run through the command tables to find the parser that will parse the string
			CommandParser * commandParserList[] = { m_commandTableParser, m_parser, m_socialsParser };
			for (unsigned i = 0; i < (sizeof(commandParserList) / sizeof(commandParserList[0])); ++i)
			{
				CommandParser * parser = commandParserList[i]->findParser (tokenVector, 0);
				if (parser)
				{
					// See if the parser is an exact match to the command
					// (as opposed to corresponding to "an auto-complete")
					if (Unicode::caseInsensitiveCompare (commandToken, parser->getCmd()))
					{
						// We are only done if the command table can fully parse the command
						if (commandParserList[i]->parse (NetworkId::cms_invalid, strippedString, result) != CommandParser::ERR_CMD_NOT_FOUND)
						{
							return true;
						}

						result.erase();
					}
				}
			}

			// If we get here, we didn't find an exact parser match for the command, so it is business as usual...
		}

		if (m_commandTableParser->parse (NetworkId::cms_invalid, strippedString, result) != CommandParser::ERR_CMD_NOT_FOUND)
			return true;

		result.erase ();

		if (m_parser->parse (NetworkId::cms_invalid, strippedString, result) != CommandParser::ERR_CMD_NOT_FOUND)
			return true;

		result.erase ();

		if (m_socialsParser->parse (NetworkId::cms_invalid, strippedString, result) != CommandParser::ERR_CMD_NOT_FOUND)
			return true;
		
		result.erase ();

		if (CuiChatRoomManager::parse (strippedString, result) != CommandParser::ERR_NONE)
			return true;
	}
	
	result.erase ();

	IGNORE_RETURN (CuiChatParser::parse (str, result, m_chatRoomId, m_useChatRoom));
	return true;
}

//-----------------------------------------------------------------

bool CuiChatParserStrategy::tabCompleteToken (const Unicode::String & str, size_t tokenPos, std::vector<Unicode::String> & results, size_t & token_start, size_t & token_end) const
{
	if (str.empty ())
		return false;

	//-- no handler means we pass it on to the chat parser
	if (str [0] == CuiChatParser::getCmdChar ())
	{		
		const Unicode::String shortenedString (str.substr (1));
		const bool commandTableRetval = m_commandTableParser->tabCompleteToken (shortenedString, tokenPos + 1, results, token_start, token_end);
		const bool defaultRetval      = m_parser->tabCompleteToken             (shortenedString, tokenPos + 1, results, token_start, token_end);
		const bool socialRetval       = m_socialsParser->tabCompleteToken      (shortenedString, tokenPos + 1, results, token_start, token_end);

		if (!socialRetval && !defaultRetval && !commandTableRetval)
		{
			results.clear ();
			//-- attempt to tabcomplete in chat parser
			return false;
		} 
		++token_start;
		++token_end;
	}


	return true;
}

//----------------------------------------------------------------------

void CuiChatParserStrategy::setChatRoomId (uint32 id)
{
	m_chatRoomId = id;
}

//----------------------------------------------------------------------

void CuiChatParserStrategy::setUseChatRoom (bool b)
{
	m_useChatRoom = b;
}

// ======================================================================
