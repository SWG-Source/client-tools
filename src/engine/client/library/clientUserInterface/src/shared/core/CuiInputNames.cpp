// ======================================================================
//
// CuiInputNames.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiInputNames.h"

#include "clientDirectInput/DirectInput.h"
#include "LocalizationManager.h"
#include "LocalizedStringTable.h"
#include "StringId.h"
#include "UnicodeUtils.h"
#include "sharedFile/Iff.h"
#include "sharedInputMap/InputMap_Shifts.h"

#include <map>

// ======================================================================

namespace
{
	//-----------------------------------------------------------------
	typedef std::map<int32, Unicode::NarrowString> NameMap_t;
	struct InputNameCollection;

	//-----------------------------------------------------------------

	LocalizedStringTable * s_stringTable;
	bool                   s_installed;

	typedef std::map<uint32, Unicode::String> ShiftNameMap_t;
	ShiftNameMap_t *       s_shiftNameMap;

	//-- the standard input names for the various input codes
	InputNameCollection *  s_inputNames;
	//-- the special input names for input codes which are equivalent to each other (e.g. LCONTROL RCONTROL)
	InputNameCollection *  s_equivalentInputNames;

	//-----------------------------------------------------------------

	bool getInputName (int32 code, const NameMap_t & nameMap, Unicode::String & name)
	{
		NameMap_t::const_iterator find_it = nameMap.find (code);
		
		if (find_it == nameMap.end ())
			return false;
		
		const Unicode::NarrowString & canonicalName = (*find_it).second;
		
		if (s_stringTable)
		{
			const LocalizedString *	locstr = s_stringTable->getLocalizedString (canonicalName);
			
			if (locstr == 0)
			{
				name = Unicode::narrowToWide (canonicalName);
			}
			else			
				name = locstr->getString ();
		}
		else
		{
			name = Unicode::narrowToWide (canonicalName);
		}
		
		return true;
	}

	//-----------------------------------------------------------------
	struct InputNameCollection
	{
		NameMap_t              m_keyNameMap;
		NameMap_t              m_mouseButtonNameMap;
		NameMap_t              m_joyButtonNameMap;
		NameMap_t              m_joyAxisNameMap;
		NameMap_t              m_joySliderNameMap;
		NameMap_t              m_joyHatNameMap;
		
		//-----------------------------------------------------------------
		
		static inline void loadCode_0000 (Iff & iff, NameMap_t & nameMap)
		{
			iff.enterChunk (TAG (C,O,D,E));
			{
				const int32 val = iff.read_int32 ();
				char *     name = iff.read_string ();
				
				IGNORE_RETURN (nameMap.insert (std::make_pair (val, Unicode::NarrowString (name))));
				
				delete[] name;
				name = 0;
			}
			iff.exitChunk ();
		}
		//-----------------------------------------------------------------
		
		static void loadNameMap_0000 (Iff & iff,  NameMap_t & nameMap)
		{
			while (iff.getNumberOfBlocksLeft ())
			{
				loadCode_0000 (iff, nameMap);
			}
		}

		//----------------------------------------------------------------------

		bool getInputValueString (const InputMap::BindInfo & binfo, Unicode::String & str) const;
				
		//-----------------------------------------------------------------
		
		void loadInputNames_0000 (Iff & iff)
		{
			while (iff.getNumberOfBlocksLeft ())
			{
				if (iff.enterForm (TAG (K,E,Y,S), true))
				{
					loadNameMap_0000 (iff, m_keyNameMap);
					iff.exitForm ();
				}
				else if (iff.enterForm (TAG (M,B,T,S), true))
				{
					loadNameMap_0000 (iff, m_mouseButtonNameMap);
					iff.exitForm ();
				}
				else if (iff.enterForm (TAG (J,O,Y,B), true))
				{
					loadNameMap_0000 (iff, m_joyButtonNameMap);
					iff.exitForm ();
				}		
				else if (iff.enterForm (TAG (J,O,Y,A), true))
				{
					loadNameMap_0000 (iff, m_joyAxisNameMap);
					iff.exitForm ();
				}		
				else if (iff.enterForm (TAG (J,O,Y,S), true))
				{
					loadNameMap_0000 (iff, m_joySliderNameMap);
					iff.exitForm ();
				}		
				else if (iff.enterForm (TAG (J,O,Y,H), true))
				{
					loadNameMap_0000 (iff, m_joyHatNameMap);
					iff.exitForm ();
				}
#if _DEBUG
				else
				{
					char buffer[512];
					iff.formatLocation(buffer, sizeof(buffer));
					DEBUG_FATAL(true, ("unhandled data block in %s", buffer));	
				}
#endif
			}
		}

		//-----------------------------------------------------------------
		
	};
	
	//----------------------------------------------------------------------

	bool InputNameCollection::getInputValueString (const InputMap::BindInfo & binfo, Unicode::String & str) const
	{
		bool found = false;
		
		if (binfo.type == InputMap::IT_Key)
		{
			if (binfo.value <= 255)
			{
				std::string keyName;
				
				if (DirectInput::getScanCodeKeyName(static_cast<uint8>(binfo.value), keyName))
				{
					found = true;
					str = Unicode::narrowToWide(keyName);
				}
			}
			
			if (!found)
			{
				found = getInputName (binfo.value, m_keyNameMap, str);
			}
		}
		else if (binfo.type == InputMap::IT_MouseButton)
		{
			found = getInputName (binfo.value, m_mouseButtonNameMap, str);
		}
		else if (binfo.type == InputMap::IT_JoyButton)
		{
			found = getInputName (binfo.value, m_joyButtonNameMap, str);
		}
		else if (binfo.type == InputMap::IT_JoyAxis)
		{
			found = getInputName (binfo.value, m_joyAxisNameMap, str);
		}
		else if (binfo.type == InputMap::IT_JoySlider)
		{
			found = getInputName(binfo.value, m_joySliderNameMap, str);
		}
		else if (binfo.type == InputMap::IT_JoyPovHat)
		{
			found = getInputName (binfo.value, m_joyHatNameMap, str);
		}
		else if (binfo.type == InputMap::IT_None)
		{
			found = true;
			str.clear ();
		}
		else
		{
			DEBUG_WARNING (true, ("CuiInputNames getInputValueString() found unhandled input type on binding type=[%d].", static_cast<int>(binfo.type)));
		}
		
		return found;
	}
	
	//-----------------------------------------------------------------
	
	void loadInputNames (const char * filename)
	{		
		s_inputNames           = new InputNameCollection;
		s_equivalentInputNames = new InputNameCollection;

		Iff iff (filename);
		
		iff.enterForm(TAG (I,N,M,S));
		
		switch(iff.getCurrentName())
		{
		case TAG_0000:

			//-----------------------------------------------------------------
			//-- load primary code names
			iff.enterForm(TAG_0000);
			{
				s_inputNames->loadInputNames_0000(iff);
			}
			iff.exitForm(TAG_0000);
			
			//-----------------------------------------------------------------
			//-- load names of equivalent keys

			iff.enterForm(TAG (E,Q,I,V));
			{
				s_equivalentInputNames->loadInputNames_0000 (iff);
			}
			iff.exitForm(TAG (E,Q,I,V));
			
			break;
			
		default:
			{
				char buffer[512];
				iff.formatLocation(buffer, sizeof(buffer));
				DEBUG_FATAL(true, ("unknown version number in %s", buffer));
			}
			break;
		}

		iff.exitForm();
	}

	//-----------------------------------------------------------------
}

//-----------------------------------------------------------------

void CuiInputNames::install ()
{
	DEBUG_FATAL (s_installed, ("already installed.\n"));
	s_stringTable = LocalizationManager::getManager ().fetchStringTable ("inputnames");
	loadInputNames ("input/inputnames.iff");
	s_shiftNameMap = new ShiftNameMap_t;
	s_installed = true;
}

//-----------------------------------------------------------------

void CuiInputNames::remove  ()
{
	DEBUG_FATAL (!s_installed, ("not installed.\n"));

	delete s_shiftNameMap;
	s_shiftNameMap = 0;

	delete s_equivalentInputNames;
	s_equivalentInputNames = 0;

	delete s_inputNames;
	s_inputNames = 0;

	if (s_stringTable)
	{
		LocalizationManager::getManager ().releaseStringTable (s_stringTable);
		s_stringTable = 0;
	}

	s_installed = false;
}

//-----------------------------------------------------------------
void CuiInputNames::setInputShifts (const InputMap::Shifts & shifts)
{
	uint32 i;
	Unicode::String name;
	s_shiftNameMap->clear ();	

	//-----------------------------------------------------------------
	//-- key shift states

	for (i = 0; i < shifts.numberOfKeys; ++i)
	{
		if (!getInputName (shifts.key [i].button, s_inputNames->m_keyNameMap, name))
			name = Unicode::narrowToWide ("UNKNOWN");

		const std::pair<ShiftNameMap_t::iterator, bool> retval = s_shiftNameMap->insert (std::make_pair (shifts.key [i].bits, name));

		//-- shift state is already in map, look for an equivalent name
		if (!retval.second)
		{
			if (getInputName (shifts.key [i].button, s_equivalentInputNames->m_keyNameMap, name))
			{
				s_shiftNameMap->erase (retval.first);
				IGNORE_RETURN (s_shiftNameMap->insert (std::make_pair (shifts.key [i].bits, name)));
			}
		}
	}

	//-----------------------------------------------------------------
	//-- mouse button shift states

	for (i = 0; i < shifts.numberOfMouseButtons; ++i)
	{
		if (!getInputName (shifts.mouseButton [i].button, s_inputNames->m_mouseButtonNameMap, name))
			name = Unicode::narrowToWide ("UNKNOWN");
		
		const std::pair<ShiftNameMap_t::iterator, bool> retval = s_shiftNameMap->insert (std::make_pair (shifts.mouseButton [i].bits, name));
		
		//-- shift state is already in map, look for an equivalent name
		if (!retval.second)
		{
			if (getInputName (shifts.key [i].button, s_equivalentInputNames->m_keyNameMap, name))
			{
				s_shiftNameMap->erase (retval.first);
				IGNORE_RETURN (s_shiftNameMap->insert (std::make_pair (shifts.key [i].bits, name)));
			}
		}
	}
	
	//-----------------------------------------------------------------
	//-- joystick button shift states

	for (int j = 0; j < InputMap::MAX_JOYSTICKS; ++j)
	{
		const InputMap::Shifts::Joystick & joy = shifts.joystick [j];
		
		for (i = 0; i < joy.numberOfJoystickButtons; ++i)
		{
			if (!getInputName (joy.joystickButton [i].button, s_inputNames->m_keyNameMap, name))
				name = Unicode::narrowToWide ("UNKNOWN");
			
			const std::pair<ShiftNameMap_t::iterator, bool> retval = s_shiftNameMap->insert (std::make_pair (joy.joystickButton [i].bits, name));
			
			//-- shift state is already in map, look for an equivalent name
			if (!retval.second)
			{
				if (getInputName (shifts.key [i].button, s_equivalentInputNames->m_keyNameMap, name))
				{
					s_shiftNameMap->erase (retval.first);
					IGNORE_RETURN (s_shiftNameMap->insert (std::make_pair (shifts.key [i].bits, name)));
				}
			}
		}
	}
}

//-----------------------------------------------------------------

void CuiInputNames::getInputValueString (const InputMap::BindInfo & binfo, Unicode::String & name)
{	
	if (!s_equivalentInputNames->getInputValueString (binfo, name) && !s_inputNames->getInputValueString (binfo, name))
		name = Unicode::narrowToWide ("UNKNOWN");
}

//-----------------------------------------------------------------

void CuiInputNames::appendInputString (const InputMap::BindInfo & binfo, Unicode::String & bindStr)
{	
		
	//-----------------------------------------------------------------
	//-- iterate through the possible shift states, largest first, to see if any are contained
	//-- in this bind's shift state
	//-- if so, mask the iterator's shift state from the bind's shift state, and continue
	//-- this catches a shift state that represents a combination of several other states first
	
	uint32 shiftBits = binfo.shiftState;
	
	int numShiftKeys = 0;
	
	const ShiftNameMap_t::reverse_iterator rend = s_shiftNameMap->rend ();
	
	for (ShiftNameMap_t::reverse_iterator it = s_shiftNameMap->rbegin (); it != rend; ++it) //lint !e55 !e81 //bad type
	{
		const uint32 mask = (*it).first;
		
		if ((mask & shiftBits) == mask)
		{
			if (numShiftKeys)
				IGNORE_RETURN (bindStr.append (1, '+'));
			
			bindStr += (*it).second;
			
			shiftBits &= ~mask;
			++numShiftKeys;
		}
	}
	
	if (numShiftKeys)
		IGNORE_RETURN (bindStr.append (1, '+'));

	Unicode::String name;
	getInputValueString (binfo, name);

	bindStr += name;

}
//-----------------------------------------------------------------

void CuiInputNames::appendInputString (const InputMap::CommandBindInfoSet & cbis, Unicode::String & bindStr)
{	
	for (uint32 j = 0; j < cbis.numBinds; ++j)
	{
		if (j != 0)
		{
			IGNORE_RETURN (bindStr.append (1, ','));
			IGNORE_RETURN (bindStr.append (1, ' '));
		}

		const InputMap::BindInfo & binfo = cbis.binds [j];
		
		appendInputString (binfo, bindStr);
	}
}

//----------------------------------------------------------------------

bool CuiInputNames::getInputValueString (const InputMap & imap, const std::string & commandName, Unicode::String & bindStr)
{
	const InputMap::Command * const cmd = imap.findCommandByName (commandName.c_str (), true);
	
	if (!cmd)
		return false;
	
	InputMap::CommandBindInfoSet * cmdsBindInfoSets = 0;
	const InputMap::Shifts & shifts = imap.getShifts ();
	CuiInputNames::setInputShifts (shifts);
	
	const uint32 numCmds = imap.getCommandBindings (cmdsBindInfoSets, cmd);
	
	if (numCmds)
	{
		const InputMap::CommandBindInfoSet & cbis = cmdsBindInfoSets [0];
		
		if (cbis.numBinds)
		{
			CuiInputNames::appendInputString (cbis, bindStr);
		}
	}

	delete [] cmdsBindInfoSets;
	cmdsBindInfoSets = 0;

	return true;
}

//----------------------------------------------------------------------

const Unicode::String CuiInputNames::getLocalizedCommandName (const std::string & commandName)
{
	static StringId sid ("cmd_ui_n", "");
	sid.setText (Unicode::toLower (commandName));
	return sid.localize ();
}

//----------------------------------------------------------------------

const Unicode::String CuiInputNames::getLocalizedCommandDesc (const std::string & commandName)
{
	static StringId sid ("cmd_ui_d", "");
	sid.setText (Unicode::toLower (commandName));
	return sid.localize ();
}

// ======================================================================
