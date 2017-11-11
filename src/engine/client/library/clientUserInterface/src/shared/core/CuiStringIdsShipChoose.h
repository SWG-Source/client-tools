//======================================================================
//
// CCuiStringIdsShipChoose.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CCuiStringIdsShipChoose_H
#define INCLUDED_CCuiStringIdsShipChoose_H

#include "StringId.h"

//======================================================================

#define MAKE_STRING_ID(a, b) const StringId b(#a,#b);

//----------------------------------------------------------------------

namespace CuiStringIdsShipChoose
{
	MAKE_STRING_ID(ui_shipchoose, condition_reactor_uninstalled);
	MAKE_STRING_ID(ui_shipchoose, condition_reactor_disabled);
	MAKE_STRING_ID(ui_shipchoose, condition_engine_uninstalled);
	MAKE_STRING_ID(ui_shipchoose, condition_engine_disabled);
	MAKE_STRING_ID(ui_shipchoose, condition_damaged);
	MAKE_STRING_ID(ui_shipchoose, condition_pristine);
	MAKE_STRING_ID(ui_shipchoose, here);
	MAKE_STRING_ID(ui_shipchoose, no_parking_data);
	MAKE_STRING_ID(ui_shipchoose, fetching);
	MAKE_STRING_ID(ui_shipchoose, not_parked_here);
	MAKE_STRING_ID(ui_shipchoose, select);
	MAKE_STRING_ID(ui_shipchoose, godparking);
	MAKE_STRING_ID(ui_shipchoose, dont_own_a_ship);
};

#undef MAKE_STRING_ID

//======================================================================

#endif
