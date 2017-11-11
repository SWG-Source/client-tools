// ======================================================================
//
// SwgCuiCommandParserCustomization.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_SwgCuiCommandParserCustomization_H
#define INCLUDED_SwgCuiCommandParserCustomization_H

// ======================================================================

#include "sharedCommandParser/CommandParser.h"

// ======================================================================

class SwgCuiCommandParserCustomization: public CommandParser
{
public:

	SwgCuiCommandParserCustomization();
	virtual ~SwgCuiCommandParserCustomization();

	virtual bool  performParsing(NetworkId const &userId, StringVector_t const &argv, String_t const &originalCommand, String_t &result, CommandParser const *node);

private:

	// Disabled.
	SwgCuiCommandParserCustomization(SwgCuiCommandParserCustomization const&);
	SwgCuiCommandParserCustomization &operator =(SwgCuiCommandParserCustomization const&);

};

// ======================================================================

#endif
