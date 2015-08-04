//======================================================================
//
// SwgCuiCommandParserVoice.h
// copyright (c) 2008 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiCommandParserVoice_H
#define INCLUDED_SwgCuiCommandParserVoice_H

#include "sharedCommandParser/CommandParser.h"

#include <map>

class CommandParserHistory;

// ====================================================================

class SwgCuiCommandParserVoice : public CommandParser
{
public:

	explicit SwgCuiCommandParserVoice ();
	virtual bool performParsing(NetworkId const & userId,StringVector_t const & argv, Unicode::String const & originalCommand, Unicode::String & result, CommandParser const * node);

private:

	typedef bool(*ProcessorFunction)(const NetworkId &, const CommandParser::StringVector_t &, const Unicode::String &, Unicode::String &, const CommandParser *);
	typedef std::map<String_t,ProcessorFunction> ProcessorMap;

	ProcessorMap m_processorMap;

	SwgCuiCommandParserVoice (SwgCuiCommandParserVoice const & rhs);
	SwgCuiCommandParserVoice & operator=(SwgCuiCommandParserVoice const & rhs);
};


//======================================================================

#endif
