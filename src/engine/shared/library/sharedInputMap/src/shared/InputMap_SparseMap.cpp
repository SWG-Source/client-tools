// ======================================================================
//
// InputMap_SpareMap.cpp
// Portions copyright 1999 Bootprint Entertainment
// Portions copyright 2001-2004 Sony Online Entertainment
//
// ======================================================================

#include "sharedInputMap/FirstSharedInputMap.h"
#include "sharedInputMap/InputMap_SparseMap.h"

#include "sharedInputMap/InputMap_Command.h"
#include "sharedFile/Iff.h"
#include <vector>

//-----------------------------------------------------------------

namespace
{
	const Tag TAG_POV  = TAG3 (P,O,V);
	const Tag TAG_MOSB = TAG (M,O,S,B);
	const Tag TAG_JOYB = TAG (J,O,Y,B);
	const Tag TAG_KEYS = TAG (K,E,Y,S);
	const Tag TAG_JOYX = TAG (J,O,Y,X);
	const Tag TAG_JOYS = TAG (J,O,Y,S);

	//-- todo: remove this by making all the sparsemap structs the same type

	template<typename T> inline int loadShiftStuff (const InputMap & imap, Iff & iff, const uint32 shiftState, const uint32 type, uint32 & num, T *& buttons)
	{
		UNREF (type);
		UNREF(shiftState);

		int failCount = 0;

		num = static_cast<uint32>(iff.getNumberOfBlocksLeft ());
		if (num)
		{
			buttons = NON_NULL (new T [num]);
			
			for (uint32 i = 0; i < num; ++i)
			{
				iff.enterChunk (TAG_DATA);
				{
					T & btn                      = buttons [i];
					btn.id                       = iff.read_int32();

					std::string command  = iff.read_stdstring ();		
					if (command == "CMD_toggleSafety")  // This command was renamed to reflect its current purpose.
						command = "CMD_toggleAutoAim";

					btn.cmd                      = imap.findCommandByName (command.c_str (), true);

					if (!command.empty () && !btn.cmd)
					{
						++failCount;
						WARNING (!command.empty () && !btn.cmd, ("InputMap::SparseMap attempted to load binding [%d] of type [%d] for command [%s] in shiftState [%d] but command not found.", i, type, command.c_str (), shiftState));
					}

					WARNING (btn.cmd && (btn.cmd->types & type) == 0, ("Attempt to assign command %s to key %d in shiftstate %d, allowed bits: %d\n", command.c_str (), btn.id, shiftState, btn.cmd->types));	
				}
				iff.exitChunk (TAG_DATA);
			}
		}

		return failCount;
	}

	//-- todo: remove this by making all the sparsemap structs the same type

	template<typename T> inline void writeShiftStuff (Iff & iff, const uint32 num, const T * buttons)
	{
		if (buttons)
		{		
			for (uint32 i = 0; i < num; ++i)
			{
				iff.insertChunk (TAG_DATA);
				{
					iff.insertChunkData   (buttons [i].id);
					iff.insertChunkString (buttons [i].cmd ? buttons [i].cmd->name.c_str () : "");
				}
				iff.exitChunk (TAG_DATA);
			}
		}
	}

	//-----------------------------------------------------------------

	/**
	* a method to insert an entry into a SparseMap array, to match the specified BindInfo
	*
	*/

	template<typename T> inline void insertInfo (const InputMap::BindInfo & binfo, const InputMap::Command * cmd, T *& array, uint32 & num)
	{
		//-----------------------------------------------------------------
		//-- simply replace if it is already in this sparsemap
		
		if (array)
		{
			for (uint32 i = 0; i < num; ++i)
			{
				if (array [i].id == binfo.value)
				{
					array [i].cmd = cmd;
					return;
				}
			}
		}
		
		T * newBtns = NON_NULL (new T [num + 1]);
		
		if (array)
		{
			for (uint32 i = 0; i < num; ++i)
			{
				newBtns [i] = array [i];
			}
		}
		
		newBtns [num].cmd    = cmd;
		newBtns [num].id     = binfo.value;
		
		delete[] array;
		array = newBtns;
		
		++num;
	}
}

//-----------------------------------------------------------------

InputMap::SparseMap::Joystick::Joystick(void)
: numberOfJoystickButtons(0),
	joystickButton(NULL),
	numberOfJoystickAxis(0),
	joystickAxis(NULL),
	numberOfJoystickSliders(0),
	joystickSlider(NULL),
	numberOfJoystickPovHats(0),
	joystickPovHat(NULL)
{
}

// ----------------------------------------------------------------------

InputMap::SparseMap::Joystick::~Joystick(void)
{
	delete [] joystickButton;
	joystickButton = 0;
	delete [] joystickAxis;
	joystickAxis = 0;
	delete [] joystickSlider;
	joystickSlider = 0;
	delete [] joystickPovHat;
	joystickPovHat = 0;
}

//-----------------------------------------------------------------

InputMap::SparseMap::SparseMap(void)
: next(NULL),
	shiftState(0),
	numberOfKeys(0),
	key(NULL),
	numberOfMouseButtons(0),
	mouseButton(NULL)
//lint -save -esym(1926,SparseMap::joystick) // default constructor
{
//lint -restore
}


// ----------------------------------------------------------------------

InputMap::SparseMap::~SparseMap(void)
{
	next = NULL;
	delete [] key;
	key = 0;
	delete [] mouseButton;
	mouseButton = 0;
}

//-----------------------------------------------------------------

void InputMap::SparseMap::load_0006 (const InputMap & imap, Iff & iff)
{
	
	// get the shifting state that this sparse map is valid for
	iff.enterChunk(TAG_INFO);
	{
		shiftState             = iff.read_uint32();
		absorbDefaultShiftMaps = iff.read_uint8 () != 0;
	}
	iff.exitChunk(TAG_INFO);
	
	// load the keyboard keys
	if (iff.enterForm(TAG_KEYS, true))
	{
		const int failCount = loadShiftStuff (imap, iff, shiftState, Command::T_BUTTON, numberOfKeys, key);
		WARNING (failCount > 0, ("InputMap::SparseMap failed to load %d key bindings for shift state %d", failCount, shiftState));
		iff.exitForm(TAG_KEYS);
	}
	
	if (iff.enterForm(TAG_MOSB, true))
	{
		const int failCount = loadShiftStuff (imap, iff, shiftState, Command::T_BUTTON, numberOfMouseButtons, mouseButton);
		WARNING (failCount > 0, ("InputMap::SparseMap failed to load %d mouse button bindings", failCount, shiftState));
		iff.exitForm(TAG_MOSB);
	}
	
	if (iff.enterForm(TAG_JOYB, true))
	{
		const int failCount = loadShiftStuff (imap, iff, shiftState, Command::T_BUTTON, joystick[0].numberOfJoystickButtons, joystick[0].joystickButton);
		WARNING (failCount > 0, ("InputMap::SparseMap failed to load %d joystick button bindings", failCount, shiftState));
		iff.exitForm(TAG_JOYB);
	}
	
	if (iff.enterForm(TAG_JOYX, true))
	{
		const int failCount = loadShiftStuff (imap, iff, shiftState, Command::T_AXIS, joystick[0].numberOfJoystickAxis, joystick[0].joystickAxis);	
		WARNING (failCount > 0, ("InputMap::SparseMap failed to load %d joystick axis bindings", failCount, shiftState));
		iff.exitForm(TAG_JOYX);
	}
	
	if (iff.enterForm(TAG_JOYS, true))
	{
		const int failCount = loadShiftStuff (imap, iff, shiftState, Command::T_SLIDER, joystick[0].numberOfJoystickSliders, joystick[0].joystickSlider);	
		WARNING (failCount > 0, ("InputMap::SparseMap failed to load %d joystick slider bindings", failCount, shiftState));
		iff.exitForm(TAG_JOYS);
	}

	if (iff.enterForm(TAG_POV, true))
	{
		const int failCount = loadShiftStuff (imap, iff, shiftState, Command::T_POVHAT, joystick[0].numberOfJoystickPovHats, joystick[0].joystickPovHat);	
		WARNING (failCount > 0, ("InputMap::SparseMap failed to load %d joystick pov bindings", failCount, shiftState));
		iff.exitForm(TAG_POV);
	}
}

//-----------------------------------------------------------------

void InputMap::SparseMap::write_0006 (Iff & iff) const
{
	// get the shifting state that this sparse map is valid for
	iff.insertChunk(TAG_INFO);
	{
		iff.insertChunkData (shiftState);
		iff.insertChunkData (static_cast<uint8>(absorbDefaultShiftMaps));

	}
	iff.exitChunk(TAG_INFO);
	
	if (numberOfKeys)
	{
		iff.insertForm(TAG_KEYS, true);
		{
			writeShiftStuff (iff, numberOfKeys, key);
		}
		iff.exitForm(TAG_KEYS);
	}
	
	if (numberOfMouseButtons)
	{
		iff.insertForm(TAG_MOSB, true);
		{
			writeShiftStuff (iff, numberOfMouseButtons, mouseButton);
		}
		iff.exitForm(TAG_MOSB);
	}
	if (joystick[0].numberOfJoystickButtons)
	{
		iff.insertForm(TAG_JOYB, true);
		{
			writeShiftStuff (iff, joystick[0].numberOfJoystickButtons, joystick[0].joystickButton);
		}
		iff.exitForm(TAG_JOYB);
	}
	if (joystick[0].numberOfJoystickAxis)
	{
		iff.insertForm(TAG_JOYX, true);
		{
			writeShiftStuff (iff, joystick[0].numberOfJoystickAxis, joystick[0].joystickAxis);	
		}
		iff.exitForm(TAG_JOYX);
	}
	if (joystick[0].numberOfJoystickSliders)
	{
		iff.insertForm(TAG_JOYS, true);
		{
			writeShiftStuff (iff, joystick[0].numberOfJoystickSliders, joystick[0].joystickSlider);
		}
		iff.exitForm(TAG_JOYS);
	}
	if (joystick[0].numberOfJoystickPovHats)
	{
		iff.insertForm(TAG_POV, true);
		{
			writeShiftStuff (iff, joystick[0].numberOfJoystickPovHats, joystick[0].joystickPovHat);	
		}
		iff.exitForm(TAG_POV);
	}
}

//-----------------------------------------------------------------
/**
* Remove all bindings for the given command
*/
void InputMap::SparseMap::removeBindings (const Command * cmd)
{
	uint32 i;
	
	if (key)
	{
		for (i = 0; i < numberOfKeys; ++i)
		{
			if (key [i].cmd == cmd)
				key [i].cmd = 0;
		}
	}
	
	if (mouseButton)
	{
		for (i = 0; i < numberOfMouseButtons; ++i)
		{
			if (mouseButton [i].cmd == cmd)
				mouseButton [i].cmd = 0;
		}
	}
	
	for (uint32 j = 0; j < MAX_JOYSTICKS; ++j)
	{
		Joystick & joy = joystick [j];
		
		for (i = 0; i < joy.numberOfJoystickButtons; ++i)
		{
			if (joy.joystickButton [i].cmd == cmd)
				joy.joystickButton [i].cmd = 0;
		}
		for (i = 0; i < joy.numberOfJoystickAxis; ++i)
		{
			if (joy.joystickAxis [i].cmd == cmd)
				joy.joystickAxis [i].cmd = 0;
		}
		for (i = 0; i < joy.numberOfJoystickSliders; ++i)
		{
			if (joy.joystickSlider [i].cmd == cmd)
				joy.joystickSlider [i].cmd = 0;
		}
		for (i = 0; i < joy.numberOfJoystickPovHats; ++i)
		{
			if (joy.joystickPovHat [i].cmd == cmd)
				joy.joystickPovHat [i].cmd = 0;
		}
	}
}

//-----------------------------------------------------------------

/**
* Bind an input state represented by a BindInfo, with the specified command.
*/
bool InputMap::SparseMap::addBinding (const BindInfo & binfo, const Command * cmd)
{
	switch (binfo.type)
	{	
	case IT_Key:
		insertInfo (binfo, cmd, key, numberOfKeys);
		break;
		
	case IT_MouseButton:
		insertInfo (binfo, cmd, mouseButton, numberOfMouseButtons);
		break;

	case IT_JoyButton:
		{
			for (uint32 i = 0; i < MAX_JOYSTICKS; ++i)
			{
				insertInfo (binfo, cmd, joystick [i].joystickButton, joystick [i].numberOfJoystickButtons);
			}
		}
		break;

	case IT_JoyPovHat:
		{
			for (uint32 i = 0; i < MAX_JOYSTICKS; ++i)
			{
				insertInfo (binfo, cmd, joystick [i].joystickPovHat, joystick [i].numberOfJoystickPovHats);
			}
		}

		break;

	case IT_JoyAxis:
		{
			for (uint32 i = 0; i < MAX_JOYSTICKS; ++i)
			{
				insertInfo (binfo, cmd, joystick [i].joystickAxis, joystick [i].numberOfJoystickAxis);
			}
		}
		break;	
		
	case IT_JoySlider:
		{
			for (uint32 i = 0; i < MAX_JOYSTICKS; ++i)
			{
				insertInfo (binfo, cmd, joystick [i].joystickSlider, joystick [i].numberOfJoystickSliders);
			}
		}
		break;	

	case IT_None:
	default:
		return false;
	}

	return true;
}

//-----------------------------------------------------------------

/**
* Given an input state represented by a BindInfo, find the unique command
* which is bound to this state, in this SparseMap. 
*
* @return null if no command is bound to this state
*/
const InputMap::Command * InputMap::SparseMap::getCommandByBinding (const BindInfo & binfo) const
{
	uint32 i;
	uint32 j;
	
	switch (binfo.type)
	{	
	case IT_Key:

		if (numberOfKeys == 0)
			break;

		NOT_NULL (key);

		for (i = 0; i < numberOfKeys; ++i)
		{
			if (key [i].id == binfo.value)
			{
				return key [i].cmd;
			}
		}
		
		break;
		
	case IT_MouseButton:
		
		if (numberOfMouseButtons == 0)
			break;

		NOT_NULL (mouseButton);

		for (i = 0; i < numberOfMouseButtons; ++i)
		{
			if (mouseButton [i].id == binfo.value)
			{
				return mouseButton [i].cmd;
			}
		}

		break;
		
	case IT_JoyButton:
		
		for (j = 0; j< MAX_JOYSTICKS; ++j)
		{
			const Joystick & joy = joystick [j];
					
			if (joy.numberOfJoystickButtons == 0)
				break;
			
			NOT_NULL (key);

			for (i = 0; i < joy.numberOfJoystickButtons; ++i)
			{
				if (joy.joystickButton [i].id == binfo.value)
				{
					return joy.joystickButton [i].cmd;
				}
			}
		}
		
		break;

	case IT_JoyAxis:
		
		for (j = 0; j< MAX_JOYSTICKS; ++j)
		{
			const Joystick & joy = joystick [j];
					
			if (joy.numberOfJoystickAxis == 0)
				break;
			
			NOT_NULL (key);

			for (i = 0; i < joy.numberOfJoystickAxis; ++i)
			{
				if (joy.joystickAxis [i].id == binfo.value)
				{
					return joy.joystickAxis [i].cmd;
				}
			}
		}
		
		break;

	case IT_JoySlider:
		
		for (j = 0; j< MAX_JOYSTICKS; ++j)
		{
			const Joystick & joy = joystick [j];
					
			if (joy.numberOfJoystickSliders == 0)
				break;
			
			NOT_NULL (key);

			for (i = 0; i < joy.numberOfJoystickSliders; ++i)
			{
				if (joy.joystickSlider [i].id == binfo.value)
				{
					return joy.joystickSlider [i].cmd;
				}
			}
		}
		
		break;

	case IT_JoyPovHat:
		
		for (j = 0; j< MAX_JOYSTICKS; ++j)
		{
			const Joystick & joy = joystick [j];
			
			if (joy.numberOfJoystickPovHats == 0)
				break;
			
			NOT_NULL (key);

			for (i = 0; i < joy.numberOfJoystickPovHats; ++i)
			{
				if (joy.joystickPovHat [i].id == binfo.value)
				{
					return joy.joystickPovHat [i].cmd;
				}
			}
		}
		
		break;
	case IT_None:
	default:
		break;
	}
	
	return 0;
}

//-----------------------------------------------------------------

void InputMap::SparseMap::getCommandBindings (std::vector<BindInfo> & biv, const Command * cmd) const
{
	//-- look for bound keyboard keys
	
	uint32 j;
	
	if (key)
	{
		for (j = 0; j < numberOfKeys; ++j)
		{
			if (key [j].cmd == cmd)
				biv.push_back (BindInfo (shiftState, IT_Key, key [j].id));
		}
	}
	
	//-----------------------------------------------------------------
	//-- look for bound mouse buttons
	
	if (mouseButton)
	{
		for (j = 0; j < numberOfMouseButtons; ++j)
		{
			if (mouseButton [j].cmd == cmd)
				biv.push_back (BindInfo (shiftState, IT_MouseButton, mouseButton [j].id));
		}
	}

	//-----------------------------------------------------------------
	//-- look for bound joystick keys
	
	for (j = 0; j < MAX_JOYSTICKS; ++j)
	{
		const SparseMap::Joystick & joy = joystick [j];
		
		uint32 k;

		for (k = 0; k < joy.numberOfJoystickButtons; ++k)
		{
			if (joy.joystickButton [k].cmd == cmd)
				biv.push_back (BindInfo (shiftState, IT_JoyButton, joy.joystickButton [k].id));
		}
		
		for (k = 0; k < joy.numberOfJoystickAxis; ++k)
		{
			if (joy.joystickAxis [k].cmd== cmd)
				biv.push_back (BindInfo (shiftState, IT_JoyAxis, joy.joystickAxis [k].id));
		}

		for (k = 0; k < joy.numberOfJoystickSliders; ++k)
		{
			if (joy.joystickSlider [k].cmd== cmd)
				biv.push_back (BindInfo (shiftState, IT_JoySlider, joy.joystickSlider [k].id));
		}
		
		for (k = 0; k < joy.numberOfJoystickPovHats; ++k)
		{
			if (joy.joystickPovHat [k].cmd == cmd)
				biv.push_back (BindInfo (shiftState, IT_JoyPovHat, joy.joystickPovHat [k].id));
		}
	}
}

// ======================================================================
