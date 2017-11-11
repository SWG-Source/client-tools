//======================================================================
//
// CuiStringIdsResource.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiStringIdsResource_H
#define INCLUDED_CuiStringIdsResource_H

#include "StringId.h"

//======================================================================

#define MAKE_STRING_ID(a, b) const StringId b(#a,#b);

//----------------------------------------------------------------------

namespace CuiStringIdsResource
{
	MAKE_STRING_ID(ui,      res_confirm_dump_hopper);
	MAKE_STRING_ID(ui,      res_err_empty_hopper);
	MAKE_STRING_ID(ui,      res_extract_active_off);
	MAKE_STRING_ID(ui,      res_extract_active_on);
	MAKE_STRING_ID(ui,      res_resource_depleted);
	MAKE_STRING_ID(ui,      res_wait_empty_hopper);
	MAKE_STRING_ID(ui,      res_noresourceselected);
	MAKE_STRING_ID(ui,      res_pleaseselectresource);
	MAKE_STRING_ID(ui,      res_activatedwithoutresource);
	MAKE_STRING_ID(ui,      res_waiting);
	MAKE_STRING_ID(ui,      res_confirm_dumpall_hopper);

	MAKE_STRING_ID(ui,      survey_notool);
	MAKE_STRING_ID(ui,      survey_noresource);
	MAKE_STRING_ID(ui_roadmap, roadmap_not_valid);
};

#undef MAKE_STRING_ID

//======================================================================

#endif
