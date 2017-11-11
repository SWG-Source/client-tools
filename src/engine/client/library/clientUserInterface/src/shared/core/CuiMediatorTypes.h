// ======================================================================
//
// CuiMediatorTypes.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CuiMediatorTypes_H
#define INCLUDED_CuiMediatorTypes_H

// ======================================================================
#define MAKE_MEDIATOR_TYPE(a) Type const a = #a ;

namespace CuiMediatorTypes
{
	typedef const char * Type;

	MAKE_MEDIATOR_TYPE (Backdrop);
	MAKE_MEDIATOR_TYPE (BugSubmission);
	MAKE_MEDIATOR_TYPE (ColorPicker);
	MAKE_MEDIATOR_TYPE (Console);
	MAKE_MEDIATOR_TYPE (DeleteSkillConfirmation);
	MAKE_MEDIATOR_TYPE (HueObject);
	MAKE_MEDIATOR_TYPE (InputBox);
	MAKE_MEDIATOR_TYPE (Keypad);
	MAKE_MEDIATOR_TYPE (Notepad);
	MAKE_MEDIATOR_TYPE (PopupHelp);
	MAKE_MEDIATOR_TYPE (ResourceSplitter);
	MAKE_MEDIATOR_TYPE (SetName);
	MAKE_MEDIATOR_TYPE (Splash);
	MAKE_MEDIATOR_TYPE (Transfer);
	MAKE_MEDIATOR_TYPE (Transition);
	MAKE_MEDIATOR_TYPE (VehicleProto);
	MAKE_MEDIATOR_TYPE (IMEInput);
};

#undef MAKE_MEDIATOR_TYPE

// ======================================================================

#endif
