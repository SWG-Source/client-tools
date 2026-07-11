//======================================================================
//
// InputMap_Command.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "sharedInputMap/FirstSharedInputMap.h"
#include "sharedInputMap/InputMap_Command.h"

#include "sharedFile/Iff.h"
#include "sharedFoundation/MessageQueue.h"
#include "sharedFoundation/MessageQueueDataTemplate.h"
#include <vector>
#include <algorithm>
#include <map>

//======================================================================


namespace InputMap_CommandNamespace
{
	const Tag TAG_CMD  = TAG3 (C,M,D);
	const Tag TAG_CATE = TAG (C,A,T,E);
	const Tag TAG_CMDS = TAG (C,M,D,S);

	/**
	* Functor for sorting command list
	*/

	struct CommandComparator
	{
		inline bool operator () (const InputMap::Command * a, const InputMap::Command * b)
		{
			NOT_NULL (a);
			NOT_NULL (b);
			return a->name < b->name;
		}
	};
}

using namespace InputMap_CommandNamespace;

//----------------------------------------------------------------------


const uint32 InputMap::Command::T_BUTTON = 0x0001;
const uint32 InputMap::Command::T_POVHAT = 0x0002;
const uint32 InputMap::Command::T_AXIS   = 0x0004;
const uint32 InputMap::Command::T_SLIDER = 0x0008;
	
const uint32 InputMap::Command::T_PRESSABLE = T_BUTTON | T_POVHAT;
const uint32 InputMap::Command::T_ANY       = T_BUTTON | T_POVHAT | T_AXIS | T_SLIDER;

//----------------------------------------------------------------------

void InputMap::Command::EventData::load_0006 (Iff & iff)
{
	message    = iff.read_int32     ();
	value      = iff.read_float     ();
	str        = iff.read_stdstring ();
}

//----------------------------------------------------------------------

void InputMap::Command::EventData::write_0006 (Iff & iff) const
{
	iff.insertChunkData   (message);
	iff.insertChunkData   (value);
	iff.insertChunkString (str.c_str ());
}

//----------------------------------------------------------------------

void InputMap::Command::load_0006 (Iff & iff)
{
	iff.enterChunk(TAG_CMD);
	{	
		name             = iff.read_stdstring ();
		types            = iff.read_uint32    ();
		userDefined      = iff.read_uint8     () != 0;
		repeatStartDelay = iff.read_uint8     () != 0;

		pressEvent.load_0006   (iff);
		repeatEvent.load_0006  (iff);
		releaseEvent.load_0006 (iff);
		resetEvent.load_0006   (iff);
	}
	iff.exitChunk (TAG_CMD);
}

//-----------------------------------------------------------------

void InputMap::Command::write_0006 (Iff & iff) const
{
	iff.insertChunk(TAG_CMD);
	{	
		iff.insertChunkString (name.c_str ());
		iff.insertChunkData   (types);
		iff.insertChunkData   (userDefined);
		iff.insertChunkData   (repeatStartDelay);

		pressEvent.write_0006   (iff);
		repeatEvent.write_0006  (iff);
		releaseEvent.write_0006 (iff);
		resetEvent.write_0006   (iff);
	}
	iff.exitChunk (TAG_CMD);
}

//----------------------------------------------------------------------

void InputMap::Command::write_0006_all (Iff & iff, const CommandVector & cv, bool userDefinedOnly)
{
	typedef stdmap<std::string, CommandVector>::fwd CategoryMap;
	CategoryMap cats;
	{
		for (CommandVector::const_iterator it = cv.begin (); it != cv.end (); ++it)
		{
			const Command * const cmd = *it;
			
			if (userDefinedOnly && !cmd->userDefined)
				continue;
			
			cats [cmd->category].push_back (cmd);
		}
	}

	for (CategoryMap::const_iterator it = cats.begin (); it != cats.end (); ++it)
	{
		const std::string & category           = (*it).first;
		const CommandVector & categoryCommands = (*it).second;

		iff.insertForm (TAG_CATE);
		{
			iff.insertChunk (TAG_NAME);
			{
				iff.insertChunkString (category.c_str ());
			}
			iff.exitChunk (TAG_NAME);

			for (CommandVector::const_iterator cit = categoryCommands.begin (); cit != categoryCommands.end (); ++cit)
			{
				const Command * const cmd = *cit;
				cmd->write_0006 (iff);
			}
		}
		iff.exitForm (TAG_CATE);
	}
}
//----------------------------------------------------------------------

void InputMap::Command::load_0006_all (Iff & iff, CommandVector & cv, StringVector & sv)
{
	while (iff.enterForm (TAG_CATE, true))
	{
		std::string category;

		iff.enterChunk (TAG_NAME);
		{
			category = iff.read_stdstring ();

			if (!category.empty ())
				if (std::find (sv.begin (), sv.end (), category) == sv.end ())
					sv.push_back (category);
		}
		iff.exitChunk (TAG_NAME);

		const uint32 numCmds = static_cast<uint32>(iff.getNumberOfBlocksLeft ());

		cv.reserve (numCmds);
		
		if (numCmds)
		{
			for (uint32 i = 0; i < numCmds; ++i)
			{
				Command * const cmd = new Command;
				cmd->load_0006    (iff);
				cmd->category       = category;

				if (category.empty ())
				{
					WARNING (true, ("InputMap::Command empty category for command [%s]", cmd->name.c_str ()));
					delete cmd;
				}
				else
					cv.push_back      (cmd);
			}
			
			std::sort (cv.begin (), cv.end (), CommandComparator ());
			
			std::string last_name;
			for (CommandVector::iterator it = cv.begin (); it != cv.end (); )
			{
				const Command * const cmd = *it;
				if (cmd->name == last_name)
				{
					WARNING (true, ("duplicate command [%s] found in inputmap.", cmd->name.c_str ()));
					delete cmd;
					it = cv.erase (it);
				}
				else
				{
					++it;
					last_name = cmd->name;
				}
			}
		}
		
		iff.exitForm (TAG_CATE);
	}
}

//----------------------------------------------------------------------

bool InputMap::Command::load_0006_from_file (const std::string & filename, CommandVector & cv, StringVector & sv)
{
	Iff iff;
	if (!iff.open (filename.c_str (), true))
	{
		WARNING (true, ("InputMap::Command::load_0006_from_file [%s] unable to open", filename.c_str ()));
		return false;
	}
	
	bool ok = false;
	iff.enterForm (TAG_CMDS);
	{
		switch(iff.getCurrentName())
		{
		case TAG_0006:
			iff.enterForm (TAG_0006);
			load_0006_all (iff, cv, sv);
			iff.exitForm  (TAG_0006);
			ok = true;
			break;
			
		default:
			{
				char buffer[512];
				iff.formatLocation(buffer, sizeof(buffer));
				DEBUG_FATAL(true, ("unknown version number in %s", buffer));
			}
			break;
		}
	}	
	iff.exitForm (TAG_CMDS);
	return ok;
}

//----------------------------------------------------------------------

void InputMap::Command::execute    (MessageQueue & mq, const float * overrideValuePress, const float * overrideValueRepeat, const float * overrideValueRelease) const
{
	pressEvent.execute   (mq, overrideValuePress);
	repeatEvent.execute  (mq, overrideValueRepeat);
	releaseEvent.execute (mq, overrideValueRelease);
}

//----------------------------------------------------------------------

bool InputMap::Command::EventData::execute (MessageQueue & mq, const float * overrideValue) const
{
	if (message)
	{
		const float theValue = overrideValue ? *overrideValue  : value;

		if (str.empty ())
			mq.appendMessage (message, theValue);
		else
		{
			typedef MessageQueueDataTemplate<std::string> MessageString;
			MessageString * const data = new MessageString (str);
			mq.appendMessage (message, theValue, data);

		}
		return true;
	}

	return false;
}

//----------------------------------------------------------------------

void    InputMap::Command::copy (const Command & rhs)
{
	name             = rhs.name;
	types            = rhs.types;
	pressEvent       = rhs.pressEvent;
	releaseEvent     = rhs.releaseEvent;
	repeatEvent      = rhs.repeatEvent;
	resetEvent       = rhs.resetEvent;
	userDefined      = rhs.userDefined;
	repeatStartDelay = rhs.repeatStartDelay;
	category         = rhs.category;
}

//======================================================================
