//======================================================================
//
// CCuiStringIdsShipView.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CCuiStringIdsShipView_H
#define INCLUDED_CCuiStringIdsShipView_H

#include "StringId.h"

//======================================================================

#define MAKE_STRING_ID(a, b) const StringId b(#a,#b);

//----------------------------------------------------------------------

namespace CuiStringIdsShipView
{
	MAKE_STRING_ID(ui_shipview, condition_reactor_uninstalled);
	MAKE_STRING_ID(ui_shipview, condition_reactor_disabled);
	MAKE_STRING_ID(ui_shipview, condition_engine_uninstalled);
	MAKE_STRING_ID(ui_shipview, condition_engine_disabled);
	MAKE_STRING_ID(ui_shipview, condition_damaged);
	MAKE_STRING_ID(ui_shipview, na);
};

#undef MAKE_STRING_ID

//======================================================================

#endif
