//===================================================================
//
// SwgCuiCommandParserCombat.h
// copyright (c) 2002 Sony Online Entertainment
//
//===================================================================

#ifndef INCLUDED_SwgCuiCommandParserCombat_H
#define INCLUDED_SwgCuiCommandParserCombat_H

//===================================================================

#include "sharedCommandParser/CommandParser.h"

//===================================================================

class SwgCuiCommandParserCombat : public CommandParser
{
public:

	SwgCuiCommandParserCombat ();
	virtual bool performParsing (const NetworkId& userId, const StringVector_t& argv, const Unicode::String& originalCommand, Unicode::String& result, const CommandParser* node);

private:

	SwgCuiCommandParserCombat (const SwgCuiCommandParserCombat& rhs);
	SwgCuiCommandParserCombat& operator= (const SwgCuiCommandParserCombat& rhs);
};

//===================================================================

#endif
