//======================================================================
//
// CuiStringIdsCharacterSheet.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiStringIdsCharacterSheet_H
#define INCLUDED_CuiStringIdsCharacterSheet_H

#include "StringId.h"

//======================================================================

#define MAKE_STRING_ID(a, b) const StringId b(#a,#b);

//----------------------------------------------------------------------

namespace CuiStringIdsCharacterSheet
{
	MAKE_STRING_ID(ui_charsheet,      homeless);
	MAKE_STRING_ID(ui_charsheet,      married);
	MAKE_STRING_ID(ui_charsheet,      unmarried);
	MAKE_STRING_ID(ui_charsheet,      unknown);
	MAKE_STRING_ID(ui_charsheet,      faction_neutral);
	MAKE_STRING_ID(ui_charsheet,      faction_neutral_imperialhelpercovert);
	MAKE_STRING_ID(ui_charsheet,      faction_neutral_imperialhelperdeclared);
	MAKE_STRING_ID(ui_charsheet,      faction_neutral_rebelhelpercovert);
	MAKE_STRING_ID(ui_charsheet,      faction_neutral_rebelhelperdeclared);
	MAKE_STRING_ID(ui_charsheet,      faction_rebelcovert);
	MAKE_STRING_ID(ui_charsheet,      faction_rebeldeclared);
	MAKE_STRING_ID(ui_charsheet,      faction_rebelonleave);
	MAKE_STRING_ID(ui_charsheet,      faction_imperialcovert);
	MAKE_STRING_ID(ui_charsheet,      faction_imperialdeclared);
	MAKE_STRING_ID(ui_charsheet,      faction_imperialonleave);
	MAKE_STRING_ID(ui_charsheet,      statmig_usealltpoints);
	MAKE_STRING_ID(ui_charsheet,      badges_earned);
	MAKE_STRING_ID(ui_charsheet,      badges_unearned);
	MAKE_STRING_ID(ui_charsheet,      badges_none);
	MAKE_STRING_ID(ui_charsheet,      badges_all);
};

#undef MAKE_STRING_ID

//======================================================================

#endif
