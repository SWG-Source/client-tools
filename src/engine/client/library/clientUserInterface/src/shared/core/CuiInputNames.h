// ======================================================================
//
// CuiInputNames.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CuiInputNames_H
#define INCLUDED_CuiInputNames_H

// ======================================================================

#include "sharedInputMap/InputMap.h"

class CuiInputNames
{
public:

	static void                    install ();
	static void                    remove  ();

	static void                    getInputValueString     (const InputMap::BindInfo & binfo, Unicode::String & bindStr);
	static void                    appendInputString       (const InputMap::BindInfo & binfo, Unicode::String & bindStr);
	static void                    appendInputString       (const InputMap::CommandBindInfoSet & cbis, Unicode::String & bindStr);
	static void                    setInputShifts          (const InputMap::Shifts & shifts);
	static bool                    getInputValueString     (const InputMap & imap, const std::string & commandName, Unicode::String & bindStr);
	static const Unicode::String   getLocalizedCommandName (const std::string & commandName);
	static const Unicode::String   getLocalizedCommandDesc (const std::string & commandName);

private:
	                               CuiInputNames ();
	                               CuiInputNames (const CuiInputNames & rhs);
	CuiInputNames &               operator=    (const CuiInputNames & rhs);

};

// ======================================================================

#endif
