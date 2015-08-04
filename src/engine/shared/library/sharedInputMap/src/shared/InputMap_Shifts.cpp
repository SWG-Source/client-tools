// ======================================================================
//
// InputMap.cpp
// copyright 1999 Bootprint Entertainment
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedInputMap/FirstSharedInputMap.h"
#include "sharedInputMap/InputMap_Shifts.h"

#include "sharedInputMap/InputMap_Command.h"
#include "sharedFile/Iff.h"
#include <vector>

// ======================================================================

namespace
{
	const Tag TAG_KEY  = TAG3 (K,E,Y);
	const Tag TAG_MOSB = TAG (M,O,S,B);
	const Tag TAG_JOYB = TAG (J,O,Y,B);
		
	inline void load_buttons (const InputMap & imap, Iff & iff, uint32 & num, InputMap::Shifts::Button *& buttons)
	{
		num = static_cast<uint32>(iff.getNumberOfBlocksLeft ());
		if (num)
		{
			buttons = NON_NULL (new InputMap::Shifts::Button[num]);
			
			for (uint32 i = 0; i < num; ++i)
			{
				iff.enterChunk (TAG_DATA);
				{
					buttons[i].button  = iff.read_int32();
					buttons[i].bits    = iff.read_uint32();
					
					std::string name  = iff.read_stdstring ();
					if (name == "CMD_toggleSafety") // This command was renamed to reflect its current purpose.
						name = "CMD_toggleAutoAim";

					buttons[i].cmd            = imap.findCommandByName (name.c_str ());
				}
				iff.exitChunk (TAG_DATA);
			}
		}
	}
	
	//-----------------------------------------------------------------
	
	inline void write_buttons (Iff & iff, const uint32 num, const InputMap::Shifts::Button * buttons)
	{
		if (buttons)
		{		
			for (uint32 i = 0; i < num; ++i)
			{
				iff.insertChunk (TAG_DATA);
				{
					iff.insertChunkData   (buttons [i].button);
					iff.insertChunkData   (buttons [i].bits);
					iff.insertChunkString (buttons [i].cmd ? buttons [i].cmd->name.c_str () : "");
				}
				iff.exitChunk ();
			}
		}
	}
	
	//-----------------------------------------------------------------
	
	inline void getArrayCommandBindings (
		std::vector<InputMap::BindInfo> & biv,
		const InputMap::Command * cmd,
		const InputMap::InputType type,
		const uint32 num,
		const InputMap::Shifts::Button * buttons)
	{
		for (uint32 j = 0; j < num; ++j)
		{
			if (cmd == buttons [j].cmd)
			{
				const InputMap::BindInfo bi (0, type, buttons [j].button);
				biv.push_back (bi);
			}
		}
	}

	//----------------------------------------------------------------------

}

// ======================================================================

InputMap::Shifts::Joystick::Joystick(void)
: numberOfJoystickButtons(0),
	joystickButton(NULL)
{
}

// ----------------------------------------------------------------------

InputMap::Shifts::Joystick::~Joystick(void)
{
	delete [] joystickButton;
	joystickButton = 0;
}

// ======================================================================

InputMap::Shifts::Shifts(void)
: numberOfKeys(0),
	key(NULL),
	numberOfMouseButtons(0),
	mouseButton(NULL)
//lint -save -esym(1926,Shifts::joystick) // default constructor
{
//lint -restore
}

// ----------------------------------------------------------------------

InputMap::Shifts::~Shifts(void)
{
	delete [] key;
	key = 0;
	delete [] mouseButton;
	mouseButton = 0;
}

//-----------------------------------------------------------------

void InputMap::Shifts::load_0006 (const InputMap & imap, Iff & iff)
{
	if (iff.enterForm(TAG_KEY, true))
	{
		load_buttons (imap, iff, numberOfKeys, key);
		iff.exitForm (TAG_KEY);
	}
	
	// read in shifting mouse buttons
	if (iff.enterForm(TAG_MOSB, true))
	{
		load_buttons (imap, iff, numberOfMouseButtons, mouseButton);
		iff.exitForm (TAG_MOSB);
	}
	
	// read in shifting joystick buttons
	if (iff.enterForm(TAG_JOYB, true))
	{
		load_buttons (imap, iff, joystick[0].numberOfJoystickButtons, joystick[0].joystickButton);		
		iff.exitForm (TAG_JOYB);
	}
}

//-----------------------------------------------------------------

void InputMap::Shifts::write_0006 (Iff & iff) const
{

	// write shifting mouse buttons
	iff.insertForm (TAG_KEY, true);
	write_buttons  (iff, numberOfKeys, key);
	iff.exitForm   (TAG_KEY);
	
	// write shifting mouse buttons
	iff.insertForm (TAG_MOSB, true);	
	write_buttons  (iff, numberOfMouseButtons, mouseButton);
	iff.exitForm   (TAG_MOSB);
	
	// write shifting joystick buttons
	iff.insertForm (TAG_JOYB, true);
	write_buttons  (iff, joystick[0].numberOfJoystickButtons, joystick[0].joystickButton);		
	iff.exitForm   (TAG_JOYB);
}

//-----------------------------------------------------------------

void InputMap::Shifts::copy (const Shifts & rhs)
{
	//-----------------------------------------------------------------
	//-- make a deep copy here to prevent grief in the dtor
	
	*this = rhs;
	
	this->key = 0;
	this->mouseButton = 0;
	
	if (rhs.numberOfKeys)
	{
		key = NON_NULL (new Shifts::Button [rhs.numberOfKeys]);
		memcpy (key, rhs.key, rhs.numberOfKeys * sizeof (Shifts::Button));
	}
	
	if (rhs.numberOfMouseButtons)
	{
		mouseButton = NON_NULL (new Shifts::Button [rhs.numberOfMouseButtons]);
		memcpy (mouseButton, rhs.mouseButton, rhs.numberOfMouseButtons * sizeof (Shifts::Button));
	}
	
	for (uint32 j = 0; j < MAX_JOYSTICKS; ++j)
	{
		const Shifts::Joystick & joy = rhs.joystick [j];
		Shifts::Joystick & rbmjoy    = this->joystick [j];
		
		rbmjoy = joy;
		rbmjoy.joystickButton = 0;
		
		if (joy.numberOfJoystickButtons)
		{
			rbmjoy.joystickButton = NON_NULL (new Shifts::Button [joy.numberOfJoystickButtons]);
			memcpy (rbmjoy.joystickButton, joy.joystickButton, joy.numberOfJoystickButtons * sizeof (Shifts::Button));
		}
	}
}

//-----------------------------------------------------------------

void InputMap::Shifts::getCommandBindings (std::vector<BindInfo> & biv, const Command * cmd) const
{
	
	//-----------------------------------------------------------------
	//-- keys

	getArrayCommandBindings (biv, cmd, IT_Key, numberOfKeys, key);
	
	//-----------------------------------------------------------------
	//-- mouseButton shifts

	getArrayCommandBindings (biv, cmd, IT_MouseButton, numberOfMouseButtons, mouseButton);
		
	//-----------------------------------------------------------------
	//-- joybutton shifts
	
	for (uint32 j = 0; j < MAX_JOYSTICKS; ++j)
	{
		const Joystick & joy = joystick [j];

		getArrayCommandBindings (biv, cmd, IT_JoyButton, joy.numberOfJoystickButtons, joy.joystickButton);
	}
}

//----------------------------------------------------------------------

void InputMap::Shifts::removeBindings     (const Command & cmd)
{
	{
		for (uint32 i = 0; i < numberOfKeys; ++i)
		{
			Button & button = key [i];
			if (button.cmd == &cmd)
				button.cmd = 0;
		}
	}

	{
		for (uint32 i = 0; i < numberOfMouseButtons; ++i)
		{
			Button & button = mouseButton [i];
			if (button.cmd == &cmd)
				button.cmd = 0;
		}
	}
}

//-----------------------------------------------------------------
