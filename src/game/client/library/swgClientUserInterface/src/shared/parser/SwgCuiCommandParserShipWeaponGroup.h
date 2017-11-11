// ======================================================================
//
// SwgCuiCommandParserShipWeaponGroup.h
// Copyright Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiCommandParserShipWeaponGroup_H
#define INCLUDED_SwgCuiCommandParserShipWeaponGroup_H

#include "sharedCommandParser/CommandParser.h"

// ======================================================================

class SwgCuiCommandParserShipWeaponGroup : public CommandParser
{
public:

	SwgCuiCommandParserShipWeaponGroup();
	~SwgCuiCommandParserShipWeaponGroup() {}

	virtual bool performParsing(const NetworkId &userId, const StringVector_t &argv, const String_t &originalCommand, String_t &result, const CommandParser *node);

private:

	SwgCuiCommandParserShipWeaponGroup(const SwgCuiCommandParserShipWeaponGroup &rhs);
	SwgCuiCommandParserShipWeaponGroup & operator=(const SwgCuiCommandParserShipWeaponGroup &rhs);
};

// ======================================================================

#endif // INCLUDED_SwgCuiCommandParserShipWeaponGroup_H
