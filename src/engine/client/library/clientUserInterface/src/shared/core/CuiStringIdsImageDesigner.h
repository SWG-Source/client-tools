//======================================================================
//
// CuiStringIdsImageDesigner.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiStringIdsImageDesigner_H
#define INCLUDED_CuiStringIdsImageDesigner_H

#include "StringId.h"

//======================================================================

#define MAKE_STRING_ID(a, b) const StringId b(#a,#b);

//----------------------------------------------------------------------

namespace CuiStringIdsImageDesigner
{
	MAKE_STRING_ID(ui_imagedesigner, designer_cancelled_session);
	MAKE_STRING_ID(ui_imagedesigner, recipient_cancelled_session);
	MAKE_STRING_ID(ui_imagedesigner, noTarget);
	MAKE_STRING_ID(ui_imagedesigner, noSkill);
	MAKE_STRING_ID(ui_imagedesigner, ready);
	MAKE_STRING_ID(ui_imagedesigner, recipient_caption);
	MAKE_STRING_ID(ui_imagedesigner, canceled_session_by_entering_combat);
	MAKE_STRING_ID(ui_imagedesigner, canceled_session_by_leaving_tent);
	MAKE_STRING_ID(ui_imagedesigner, return_to_tent_warning_prose);
};

#undef MAKE_STRING_ID

//======================================================================

#endif
