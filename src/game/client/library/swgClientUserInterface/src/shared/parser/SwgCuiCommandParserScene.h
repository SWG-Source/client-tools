// ======================================================================
//
// SwgCuiCommandParserScene.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiCommandParserScene_H
#define INCLUDED_SwgCuiCommandParserScene_H

#include "sharedCommandParser/CommandParser.h"

// ======================================================================
class ShipDamageMessage;

/**
* Commands that are related to Scenes
*/

class SwgCuiCommandParserScene : public CommandParser
{
public:
	struct Messages
	{
		struct ShipDamage 
		{
			typedef ShipDamageMessage Payload;
		};
	};

	                          SwgCuiCommandParserScene ();
	virtual bool              performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node);
	
private:
	                          SwgCuiCommandParserScene (const SwgCuiCommandParserScene & rhs);
	SwgCuiCommandParserScene &  operator= (const SwgCuiCommandParserScene & rhs);
};

// ======================================================================

#endif
