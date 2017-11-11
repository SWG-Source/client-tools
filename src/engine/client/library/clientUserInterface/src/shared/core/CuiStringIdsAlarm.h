//======================================================================
//
// CuiStringIdsAlarm.h
// Copyright Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiStringIdsAlarm_H
#define INCLUDED_CuiStringIdsAlarm_H

#include "StringId.h"

//======================================================================

#define MAKE_STRING_ID(a, b) const StringId b(#a,#b);

//----------------------------------------------------------------------
namespace CuiStringIdsAlarm
{
	MAKE_STRING_ID(ui_alarm, alarm_add_at_invalid_hour);
	MAKE_STRING_ID(ui_alarm, alarm_add_at_invalid_minute);
	MAKE_STRING_ID(ui_alarm, alarm_am);
	MAKE_STRING_ID(ui_alarm, alarm_at_invalid);
	MAKE_STRING_ID(ui_alarm, alarm_expired);
	MAKE_STRING_ID(ui_alarm, alarm_id_expires);
	MAKE_STRING_ID(ui_alarm, alarm_id_invalid);
	MAKE_STRING_ID(ui_alarm, alarm_in_invalid);
	MAKE_STRING_ID(ui_alarm, alarm_maximum_reached);
	MAKE_STRING_ID(ui_alarm, alarm_midnight);
	MAKE_STRING_ID(ui_alarm, alarm_none);
	MAKE_STRING_ID(ui_alarm, alarm_noon);
	MAKE_STRING_ID(ui_alarm, alarm_number);
	MAKE_STRING_ID(ui_alarm, alarm_pm);
	MAKE_STRING_ID(ui_alarm, alarm_recurring);
	MAKE_STRING_ID(ui_alarm, alarm_remaining);
	MAKE_STRING_ID(ui_alarm, alarm_remove);
	MAKE_STRING_ID(ui_alarm, alarm_title);
};

#undef MAKE_STRING_ID

//======================================================================

#endif // INCLUDED_CuiStringIdsAlarm_H
