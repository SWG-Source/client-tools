// ======================================================================
//
// SwgCuiCommandParserMount.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_SwgCuiCommandParserMount_H
#define INCLUDED_SwgCuiCommandParserMount_H

// ======================================================================

#include "sharedCommandParser/CommandParser.h"

// ======================================================================

class SwgCuiCommandParserMount: public CommandParser
{
public:

	SwgCuiCommandParserMount();
	virtual ~SwgCuiCommandParserMount();

	virtual bool  performParsing(NetworkId const &userId, StringVector_t const &argv, String_t const &originalCommand, String_t &result, CommandParser const *node);

private:

	// Disabled.
	SwgCuiCommandParserMount(SwgCuiCommandParserMount const&);
	SwgCuiCommandParserMount &operator =(SwgCuiCommandParserMount const&);

};

// ======================================================================

#endif
