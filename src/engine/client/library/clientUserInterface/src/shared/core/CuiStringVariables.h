//======================================================================
//
// CuiStringVariables.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiStringVariables_H
#define INCLUDED_CuiStringVariables_H

//======================================================================

namespace CuiStringVariables
{
	/**
	* variables
	*
	* Full Names (first + last)
	*
	* %TU		user
	* %TT		target
	* %TO       other object
	*
	* %NU/NT/NO short name (first name only)
	* %UU/UT/UO unfiltered long name (no "corpse of")
	* %AU/AT/AO unfiltered short name (no "corpse of")
	* %SU/ST/SO	personal subjective pronoun:    he/she/it
	* %OU/OT/OO	personal objective pronoun:     him/her/it
	* %PU/PT/PO	possessive pronoun :            his/her/its
	* %FU/FT/FO	associated faction
	* %RU/RT/RO	associated species
	* %HU/HT/HO	honorific title (sir, madame, lady, etc)
	* %MU/MT/MO	military rank (if any) (private, commander, captain)
	* %KU/KT/KO	title (if any) (master weaponsmith, journeyman chef, medical assistant)
	* %DI       digit integer
	* %DF       digit float
	*
	*/

	//----------------------------------------------------------------------

	enum Variable
	{
		V_name,
		V_shortName,
		V_nameUnfiltered,  // Unfiltered only works on creatures... these strings won't have "a corpse "
		V_shortNameUnfiltered,
		V_personalSubjectivePronoun,
		V_personalObjectivePronoun,
		V_possessivePronoun,
		V_faction,
		V_species,
		V_honorific,
		V_militaryRank,
		V_title,
		V_digit,
		V_count,
		V_none,
	};
}

//======================================================================

#endif
