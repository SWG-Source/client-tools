//======================================================================
//
// Species.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_Species_H
#define INCLUDED_Species_H

#include "sharedGame/SharedCreatureObjectTemplate.h"

class StringId;

//======================================================================

class Species
{
public:

	static Unicode::String const & getLocalizedName (SharedCreatureObjectTemplate::Species const species);
	static const StringId &        getStringId      (SharedCreatureObjectTemplate::Species const species);
};

//======================================================================

#endif // INCLUDED_Species_H
