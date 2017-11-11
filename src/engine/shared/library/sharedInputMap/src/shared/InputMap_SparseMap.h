// ======================================================================
//
// InputMap_SparseMap.h
// jeff grills
//
// copyright 2001 Bootprint Entertainment
//
// ======================================================================

#ifndef INPUT_MAP_SPARSE_MAP_H
#define INPUT_MAP_SPARSE_MAP_H

// ======================================================================

#include "sharedInputMap/InputMap.h"

struct IoEvent;
class  Iff;
class  MessageQueue;

// ======================================================================

// Map input events to messages.

struct InputMap::SparseMap
{
	
	struct Button
	{
		int             id;
		const Command * cmd;
	};
	
	struct Axis
	{
		int             id;
		const Command * cmd;
	};

	struct Slider
	{
		int             id;
		const Command * cmd;
	};
	
	struct PovHat
	{
		int             id;
		const Command * cmd;
	};
	
	struct Joystick
	{
		uint32           numberOfJoystickButtons;
		Button          *joystickButton;
		
		uint32           numberOfJoystickAxis;
		Axis            *joystickAxis;
		
		uint32           numberOfJoystickSliders;
		Slider          *joystickSlider;

		uint32           numberOfJoystickPovHats;
		PovHat          *joystickPovHat;
		
		Joystick(void);
		~Joystick(void);
	};
	
	SparseMap       *next;
	uint32           shiftState;
	bool             absorbDefaultShiftMaps;
	
	uint32           numberOfKeys;
	Button          *key;
	
	uint32           numberOfMouseButtons;
	Button          *mouseButton;
	
	Joystick         joystick[MAX_JOYSTICKS];
	
	SparseMap(void);
	~SparseMap(void);
	
	void            load_0006  (const InputMap & imap, Iff & iff);
	void            write_0006 (Iff & iff) const;
	
	bool            addBinding (const BindInfo & binfo, const Command * cmd);
	void            removeBindings (const Command * cmd);
	
	const Command * getCommandByBinding (const BindInfo & binfo) const;
	
	void            getCommandBindings (stdvector<BindInfo>::fwd & biv, const Command * cmd) const;

};

//-----------------------------------------------------------------

#endif

//-----------------------------------------------------------------
