//======================================================================
//
// InputMap_Command.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_InputMap_Command_H
#define INCLUDED_InputMap_Command_H

//======================================================================

#include "sharedInputMap/InputMap.h"

//----------------------------------------------------------------------

//-----------------------------------------------------------------
//-- Command encapsulates a set of message/value pairs for various input events

class InputMap::Command
{
public:
	class EventData
	{
	public:
		int         message;
		float       value;
		std::string str;
		
		EventData () :
		message (0),
			value   (0.0f),
			str     ()
		{
		}
		
		EventData (int _message, float _value, const std::string & _str) :
		message (_message),
			value   (_value),
			str     (_str)
		{
		}
		
		EventData (int _message, float _value) :
		message (_message),
			value   (_value),
			str     ()
		{
		}
		
		void            load_0006  (Iff & iff);
		void            write_0006 (Iff & iff) const;
		
		bool            execute (MessageQueue & mq, const float * overrideValue) const;
		
	};
	
	static const uint32 T_BUTTON;
	static const uint32 T_POVHAT;
	static const uint32 T_AXIS;
	static const uint32 T_SLIDER;
				
	static const uint32 T_PRESSABLE;
	static const uint32 T_ANY;
		
	//-- the canonical name of the command, used for string table lookup, etc...
	std::string     name;
	std::string     category;
	
	//-- the types of input assignable to this command
	uint32          types;
	
	//-----------------------------------------------------------------
	//-- messages and values for various button-like states
	
	//-- press message used for
	//-- povhats: the direction message (change direction)
	//-- joyaxis: the frame-by-frame update
	//-- joyaxis: the value is the scaling factor applied to the joystick axis deflection
	//-- joyslider: the value is the scaling factor applied to the joystick slider deflection
	
	EventData       pressEvent;
	
	//-- release message used for
	//-- povhats: the centering message
	
	EventData       releaseEvent;
	
	//-- repeat message used for
	//-- povhats: repeating direction messages
	
	EventData       repeatEvent;
	
	//-- reset message is used for all input devices
	
	EventData       resetEvent;
	
	bool            userDefined;

	bool            repeatStartDelay;
	
	Command              () :
		name             (),
		types            (0),
		pressEvent       (),
		releaseEvent     (),
		repeatEvent      (),
		resetEvent       (),
		userDefined      (false),
		repeatStartDelay (false)
	{
	}
	
	void    copy (const Command & rhs);

	typedef stdvector<const Command *>::fwd CommandVector;
	typedef stdvector<std::string>::fwd     StringVector;

	void            load_0006           (Iff & iff);
	void            write_0006          (Iff & iff) const;
	static bool     load_0006_from_file (const std::string & filename, CommandVector & cv, StringVector & categories);
	static void     load_0006_all       (Iff & iff, CommandVector & cv, StringVector & categories);
	static void     write_0006_all      (Iff & iff, const CommandVector & cv, bool userDefinedOnly);

	void            execute             (MessageQueue & mq, const float * overrideValuePress, const float * overrideValueRepeat, const float * overrideValueRelease) const;
};
//======================================================================

#endif
