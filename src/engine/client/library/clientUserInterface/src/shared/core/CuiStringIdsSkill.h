//======================================================================
//
// CuiStringIdsSkill.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiStringIdsSkill_H
#define INCLUDED_CuiStringIdsSkill_H

#include "StringId.h"

//======================================================================

#define MAKE_STRING_ID(a, b) const StringId b(#a,#b);

//----------------------------------------------------------------------

namespace CuiStringIdsSkill
{
	MAKE_STRING_ID(ui_skl,      confirm_surrender);
	MAKE_STRING_ID(ui_skl,      qualify);
	MAKE_STRING_ID(ui_skl,      command_acquired_prose);
	MAKE_STRING_ID(ui_skl,      skill_to_prefix);
	MAKE_STRING_ID(ui_skl,      surrender_prose);
	MAKE_STRING_ID(ui_skl,      acquire_skill_points_prose);
	MAKE_STRING_ID(ui_skl,      acquire_exp_prose);
	MAKE_STRING_ID(ui_skl,      exp_prose);
	MAKE_STRING_ID(ui_skl,      tooltip_skill_link_prose);
	MAKE_STRING_ID(ui_skl,      err_surrender_deps);
	MAKE_STRING_ID(ui_skl,      acquire_level_prose);
	MAKE_STRING_ID(ui_skl,      level_prose_1);
	MAKE_STRING_ID(ui_skl,      level_prose_2);

	/*
	* obsolete
	*/
	MAKE_STRING_ID(ui,      skill_exp_table_amount);
	MAKE_STRING_ID(ui,      skill_exp_table_amount_bar);
	MAKE_STRING_ID(ui,      skill_exp_table_type);   
	MAKE_STRING_ID(ui,      skill_prof_other);
};

#undef MAKE_STRING_ID

//======================================================================

#endif
