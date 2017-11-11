
//======================================================================
//
// CuiStringIdsGameLanguage.h
// Copyright Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiStringIdsGameLanguage_H
#define INCLUDED_CuiStringIdsGameLanguage_H

#include "StringId.h"

//======================================================================

#define MAKE_STRING_ID(a, b) const StringId b(#a,#b);

//----------------------------------------------------------------------
namespace CuiStringIdsGameLanguage
{
	MAKE_STRING_ID(game_language, comprehend);
	MAKE_STRING_ID(game_language, fluent_many);
	MAKE_STRING_ID(game_language, fluent_one);
	MAKE_STRING_ID(game_language, language_bothan);
	MAKE_STRING_ID(game_language, language_common);
	MAKE_STRING_ID(game_language, language_current);
	MAKE_STRING_ID(game_language, language_human);
	MAKE_STRING_ID(game_language, language_moncalamari);
	MAKE_STRING_ID(game_language, language_rodian);
	MAKE_STRING_ID(game_language, language_trandoshan);
	MAKE_STRING_ID(game_language, language_twilek);
	MAKE_STRING_ID(game_language, language_wookie);
	MAKE_STRING_ID(game_language, language_zabrak);
	MAKE_STRING_ID(game_language, language_ithorian);
	MAKE_STRING_ID(game_language, language_sullustan);
	MAKE_STRING_ID(game_language, lekku_no_comprehend);
	MAKE_STRING_ID(game_language, not_fluent_in_language);
	MAKE_STRING_ID(game_language, not_spoken_language);
	MAKE_STRING_ID(game_language, prose_speak);
	MAKE_STRING_ID(game_language, prose_comprehend);
	MAKE_STRING_ID(game_language, speak);
	MAKE_STRING_ID(game_language, spoken_languages);
	MAKE_STRING_ID(game_language, usage);
};

#undef MAKE_STRING_ID

//======================================================================

#endif // INCLUDED_CuiStringIdsGameLanguage_H
