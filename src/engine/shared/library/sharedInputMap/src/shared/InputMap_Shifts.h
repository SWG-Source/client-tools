// ======================================================================
//
// InputMap_Shifts.h
// jeff grills
//
// copyright 2001 Bootprint Entertainment
//
// ======================================================================

#ifndef INPUT_MAP_SHIFTS_H
#define INPUT_MAP_SHIFTS_H

// ======================================================================

#include "sharedInputMap/InputMap.h"

// ======================================================================

// Map input events to messages.

struct InputMap::Shifts
{
	struct Button
	{
		int             button;
		const Command * cmd;
		uint32          bits;
	};
	
	struct Joystick
	{
		uint32   numberOfJoystickButtons;
		Button  *joystickButton;
		
		Joystick(void);
		~Joystick(void);
	};
	
	uint32    numberOfKeys;
	Button   *key;
	
	uint32    numberOfMouseButtons;
	Button   *mouseButton;
	
	Joystick  joystick[MAX_JOYSTICKS];
	
	Shifts(void);
	~Shifts(void);
	
	void        load_0006          (const InputMap & imap, Iff & iff);
	void        write_0006         (Iff & iff) const;

	void        copy               (const Shifts & rhs);

	void        getCommandBindings (stdvector<BindInfo>::fwd & biv, const Command * cmd) const;

	void        removeBindings     (const Command & cmd);
};

//----------------------------------------------------------------------

#endif

//-----------------------------------------------------------------
