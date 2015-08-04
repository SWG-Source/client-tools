// ======================================================================
//
// InputMap.cpp
//
// Portions copyright 1999 Bootprint Entertainment
// Portions copyright 2001-2004 Sony Online Entertainment
//
// ======================================================================

#include "sharedInputMap/FirstSharedInputMap.h"
#include "sharedInputMap/InputMap.h"

#include "sharedInputMap/InputMap_SparseMap.h"
#include "sharedInputMap/InputMap_Shifts.h"
#include "sharedInputMap/InputMap_Command.h"

#include "sharedFile/Iff.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/MessageQueue.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedIoWin/IoWin.def"
#include <algorithm>
#include <list>
#include <string>
#include <vector>

// ======================================================================

namespace
{
	
	const Tag TAG_MAP  = TAG3 (M,A,P);	
	const Tag TAG_IMAP = TAG (I,M,A,P);
	const Tag TAG_SHFT = TAG (S,H,F,T);
	const Tag TAG_CMDS = TAG (C,M,D,S);
	
	/**
	* Functor for sorting command list
	*/

	struct CommandComparator
	{
		inline bool operator () (const InputMap::Command * a, const InputMap::Command * b) const
		{
			NOT_NULL (a);
			NOT_NULL (b);
			return a->name < b->name;
		}
	};

	//-- maybe make this user configurable one day
	const float s_repeatDelayTime = 0.5f;
}

//-----------------------------------------------------------------
/**
* The class which encapsulates the repeating state of various input devices.
* This is a private implementation which supports mouse/keyboard/joystick buttons
* as well as joystick povhats.
*/

struct InputMap::Repeat
{
	enum State
	{
		Up          = 0,
		JustPressed = 1,
		Repeating   = 2
	};
	
	struct Button
	{
		State               state;
		const Command *     cmd;
		uint                shiftBits;
		int                 value;
		InputType           type;
		float               repeatStartCountdown;
		
		bool operator==(const Button & rhs) const
		{
			return rhs.type == type && rhs.value == value;
		}
	};
	
	struct PovHat
	{
		State           state;
		const Command * cmd;
		real            direction;
		float           repeatStartCountdown;
	};
	struct Joystick
	{
		PovHat  joystickPovHat[MAX_JOYSTICK_POV_HATS];
	};
	
	Joystick  joystick[MAX_JOYSTICKS];

	/**
	* The container is a list because the sequence order is the information we need to preserve
	*/

	typedef std::list <Button> ButtonContainer_t;
	ButtonContainer_t buttons;

	Repeat ();
};

//-----------------------------------------------------------------

InputMap::Repeat::Repeat () :
buttons ()
{
	memset (joystick, 0, sizeof (Joystick) * MAX_JOYSTICKS);
}

//----------------------------------------------------------------------

InputMap::Map::Map()
{
	Zero(key);
	Zero(mouseButton);
	Zero(joystick);
}

//----------------------------------------------------------------------

InputMap::InputMap  () :
m_firstSparseMap    (NULL),
m_lastSparseMap     (NULL),
m_defaultSparseMap  (NULL),
m_messageQueue      (0),
m_keyboardNumber    (0),
m_mouseNumber       (0),
m_shifts            (new Shifts),
m_currentMap        (),
m_repeat            (new Repeat),
m_shiftState        (0),
m_cmds              (new CommandVector),
m_ownsCmds          (true),
m_baseFilename      (),
m_customFilename    (),
m_commandFilename   (),
m_commandCategories (new StringVector)
{	
	memset(m_shiftStateBitCount, 0, sizeof(m_shiftStateBitCount));
	Zero(m_joystickNumber);
	m_joystickNumber[0] = ConfigFile::getKeyInt("ClientDirectInput", "useJoystick", 0, 0);
}

// ======================================================================
// Construct a new InputMap

InputMap::InputMap  (const char * fileName, const char * customFileName, MessageQueue *newMessageQueue) :
m_firstSparseMap    (NULL),
m_lastSparseMap     (NULL),
m_defaultSparseMap  (NULL),
m_messageQueue      (newMessageQueue),
m_keyboardNumber    (0),
m_mouseNumber       (0),
m_shifts            (new Shifts),
m_currentMap        (),
m_repeat            (new Repeat),
m_shiftState        (0),
m_cmds              (new CommandVector),
m_ownsCmds          (true),
m_baseFilename      (NON_NULL (fileName)),
m_customFilename    (customFileName ? customFileName : ""),
m_commandFilename   (),
m_commandCategories (new StringVector)
{
	DEBUG_REPORT_LOG(fileName, ("Loading input map [%s]\n", fileName));
	DEBUG_REPORT_LOG(customFileName, ("Loading custom input map [%s]\n", customFileName));

	memset(m_shiftStateBitCount, 0, sizeof(m_shiftStateBitCount));

	for (int j = 0; j < MAX_JOYSTICKS; ++j)
	{
		if (j == 0)
			m_joystickNumber[j] = 0;
		else
			m_joystickNumber[j] = -1;
	}

	Iff iff;

	bool loaded = false;

	if (Iff::isValid(m_customFilename.c_str()) && iff.open (m_customFilename.c_str (), true))
	{
		if (load (iff, true))
			loaded = true;
		else
			WARNING (true, ("Failed to load from [%s], trying base.", m_customFilename.c_str () ));
	}

	if (!loaded)
	{
		if (iff.open (m_baseFilename.c_str ()))
			IGNORE_RETURN(load (iff, false));
	}

	activateNewShiftState();

	m_joystickNumber[0] = ConfigFile::getKeyInt("ClientDirectInput", "useJoystick", 0, 0);
}

// ----------------------------------------------------------------------
/**
 * Destroy an InputMap.
 * 
 * The MessageQueue will not be destructed nor deleted.
 */

InputMap::~InputMap(void)
{
	destruct ();
	m_messageQueue = NULL;
	m_shifts = NULL; //lint !e423 //memleak.. not, deleted in destruct()
	m_repeat = NULL; //lint !e423 //memleak.. not, deleted in destruct()
	m_cmds = NULL; //lint !e423 //memleak.. not, deleted in destruct()
	m_commandCategories = NULL; //lint !e423 //memleak.. not, deleted in destruct()
}

//----------------------------------------------------------------------

void InputMap::destruct ()
{
	SparseMap *sparseMap, *nextMap;

	for (sparseMap = m_firstSparseMap; sparseMap; sparseMap = nextMap)
	{
		nextMap = sparseMap->next;
		delete sparseMap;
	}
	
	m_firstSparseMap = 0;
	m_lastSparseMap = 0;
	m_defaultSparseMap = 0;

	if (m_ownsCmds)
	{
		if (NULL != m_cmds)
		{
			std::for_each (m_cmds->begin (), m_cmds->end (), PointerDeleter ());
			delete m_cmds;
			m_cmds = NULL;
		}
	}

	m_messageQueue = 0;
	
	delete m_repeat; 
	m_repeat = 0;

	delete m_shifts;
	m_shifts = NULL;

	delete m_commandCategories;
	m_commandCategories = 0;
}

//----------------------------------------------------------------------

void InputMap::setBaseFileName (const char * const filename)
{
	NOT_NULL (filename);
	m_baseFilename = filename;
}


//-----------------------------------------------------------------
/**
* Reload an InputMap's data
* @param tryCwd if true, attempt to load the InputMap by filename from the CWD
*/

bool InputMap::reload (bool forceBase)
{
	DEBUG_FATAL (!m_ownsCmds, ("Cannot reload an inputmap that does not own its commands.\n"));

	//-----------------------------------------------------------------
	//-- store the filename information, as the dtor will delete it

	const std::string copy_filename        = m_customFilename;
	const std::string copy_baseFilename    = m_baseFilename;
	const std::string copy_commandFilename = m_commandFilename;

	MessageQueue * const mq  = m_messageQueue;

	destruct ();

	m_cmds            = new CommandVector; //lint !e423 //creation of memleak... not.  deleted in destruct()
	m_commandFilename = copy_commandFilename;
	m_customFilename  = copy_filename;
	m_baseFilename    = copy_baseFilename;
	m_messageQueue    = mq;

	m_repeat = new Repeat; //lint !e423 //creation of memleak... not.  deleted in destruct()
	m_shifts = new Shifts; //lint !e423 //creation of memleak... not.  deleted in destruct()

	m_commandCategories = new StringVector;; //lint !e423 //creation of memleak... not.  deleted in destruct()

	memset(m_shiftStateBitCount, 0, sizeof(m_shiftStateBitCount));

	Iff iff; 

	bool loaded = false;

	if (!forceBase && Iff::isValid (m_customFilename.c_str ()) && iff.open (m_customFilename.c_str (), true))
	{
		if (load (iff, true))
			loaded = true;
		else
			WARNING (true, ("Failed to reload from [%s], trying base.", m_customFilename.c_str () ));
	}

	if (!loaded)
	{
		if (!iff.open (m_baseFilename.c_str (), true))
		{
			WARNING     (true, ("Unable to open base inputmap from [%s]", m_baseFilename.c_str ()));
			DEBUG_FATAL (true, ("Unable to open base inputmap from [%s]", m_baseFilename.c_str ()));
			return false;  //lint !e527 //unreachable
		}

		loaded = load (iff, false);
	}
		
	if (loaded)
		handleInputReset ();

	return loaded;
}
//-----------------------------------------------------------------
/**
* Write the input map data to an Iff.
*/

bool InputMap::write (const char * filename, bool forceBase ) const
{
	Iff iff (1024, true, false);

	write (iff);

	std::string path;
	
	if (filename)
		path = filename;
	else if (forceBase)
		path = m_baseFilename;
	else
		path = m_customFilename;

	static const char path_seps [] = { '\\', '/', 0 };

	const size_t slashpos = path.find_last_of (path_seps);

	if (slashpos != static_cast<size_t>(std::string::npos))
		IGNORE_RETURN (Os::createDirectories (path.substr (0, slashpos).c_str ()));

	return iff.write (path.c_str (), true);
}

//-----------------------------------------------------------------
/**
* Write the input map data to an Iff.
*/

void InputMap::write (Iff & iff) const
{
	NOT_NULL (m_cmds);

	iff.insertForm (TAG_IMAP);
	{
		iff.insertForm (TAG_0006);
		{
			iff.insertForm (TAG_CMDS);
			{
				iff.insertChunk (TAG_NAME);
				{
					iff.insertChunkString (m_commandFilename.c_str ());
				}
				iff.exitChunk (TAG_NAME);

				Command::write_0006_all (iff, *m_cmds, true);
			}
			iff.exitForm (TAG_CMDS);

			iff.insertForm (TAG_SHFT);
			{
				NOT_NULL(m_shifts);
				m_shifts->write_0006 (iff);
			}
			iff.exitForm (TAG_SHFT);

			const SparseMap * sm = 0;

			for (sm = m_firstSparseMap; sm; sm = sm->next)
			{
				iff.insertForm (TAG_MAP);
				{
					sm->write_0006 (iff);
				}
				iff.exitForm (TAG_MAP);
			}
		}
		iff.exitForm (TAG_0006);
	}
	iff.exitForm (TAG_IMAP);
}

// ----------------------------------------------------------------------
/**
 * Load the input map data from the Iff.
 * 
 * @param iff  [In] The Iff to load the map from
*/

bool InputMap::load (Iff & iff, bool allowFail)
{
	UNREF(allowFail);

	bool ok = false;
	
	Zero(m_currentMap);
	
	iff.enterForm(TAG_IMAP);
	
	switch(iff.getCurrentName())
	{
	case TAG_0006:
		iff.enterForm(TAG_0006);
		{
			load_0006(iff);
			ok = true;
		}
		iff.exitForm(TAG_0006);
		break;

	case TAG_0005:
		WARNING (true, ("InputMap skipping old version 0005 [%s]", iff.getFileName ()));
		break;
		
	default:
		{
			char buffer[512];
			iff.formatLocation(buffer, sizeof(buffer));
			char tagbuf [8];
			ConvertTagToString (iff.getCurrentName (), tagbuf);

			WARNING     (true, ("InputMap::load unknown version number %s in %s, should be _0006", tagbuf, buffer));
			DEBUG_FATAL (!allowFail, ("InputMap::load unknown version number %s in %s, should be _0006", tagbuf, buffer));
		}
		break;
	}
	
	iff.exitForm(TAG_IMAP, true);
	return ok;
}

// ------------------------------------------------------------------
/**
 * Load the input map data from the Iff.
 * 
 * @param iff  [In] The Iff to load the map from
 */

void InputMap::load_0006(Iff & iff)
{
	iff.enterForm (TAG_CMDS);
	{
		NOT_NULL(m_cmds);
		NOT_NULL(m_commandCategories);

		iff.enterChunk (TAG_NAME);
		{
			m_commandFilename = iff.read_stdstring ();
			if (!Command::load_0006_from_file (m_commandFilename, *m_cmds, *m_commandCategories))
				WARNING (true, ("Unable to load commands from file [%s]", m_commandFilename.c_str ()));
		}
		iff.exitChunk (TAG_NAME);

		Command::load_0006_all (iff, *m_cmds, *m_commandCategories);
	}
	iff.exitForm (TAG_CMDS);

	// check for shifting inputs
	if (iff.enterForm(TAG_SHFT, true))
	{
		NOT_NULL(m_shifts);
		m_shifts->load_0006 (*this, iff);
		iff.exitForm(TAG_SHFT);
	}

	// read in all the maps
	while (!iff.atEndOfForm())
	{
		iff.enterForm(TAG_MAP);
		{
			// create a new map and link it into the sparse map chain
			SparseMap * const newMap = new SparseMap;
			newMap->load_0006 (*this, iff);
			
			//-- the default sparse map is the one without a shift state
			if (newMap->shiftState == 0)
				m_defaultSparseMap = newMap;
			
			newMap->next = NULL;
			if (m_lastSparseMap)
				m_lastSparseMap->next = newMap;
			else
				m_firstSparseMap = newMap;
			m_lastSparseMap = newMap;
		}
		iff.exitForm (TAG_MAP);
	}
}
// ----------------------------------------------------------------------
/**
 * Add a new set of bits to the shift state.
 * 
 * This routine keeps track of how many times each bit has been added so
 * that when bits are removed, the shift state stays active until the
 * last bit of that entry is removed.
 * 
 * @param bits  [In] Bit mask to add to the shift state
 * @see InputMap::removeShiftBits()
 */

void InputMap::addShiftBits(uint bits)
{
	int i;

	for (i = 0; i < MAX_SHIFT_BITS; ++i, bits >>= 1)
		if (bits & 1)
			++m_shiftStateBitCount[i];

	activateNewShiftState();
}

// ----------------------------------------------------------------------
/**
 * Remove a set of bits from the shift state.
 * 
 * This routine keeps track of how many times each bit has been added so
 * that when bits are removed, the shift state stays active until the
 * last bit of that entry is removed.
 * 
 * @param bits  [In] Bit mask to add to the shift state
 * @see InputMap::addShiftBits()
 */

void InputMap::removeShiftBits(uint bits)
{
	int i;

	for (i = 0; i < MAX_SHIFT_BITS; ++i, bits >>= 1)
	{
		if (bits & 1)
		{
			--m_shiftStateBitCount[i];
			DEBUG_FATAL(m_shiftStateBitCount[i] < 0, ("InputMap::removeShiftBits underflow on bit %d", i));
		}
	}

	activateNewShiftState();
}

//-----------------------------------------------------------------
/**
* expand a sparse map into the current map, overwriting any data already there
*/

void InputMap::expandSparseMap (const SparseMap &sparseMap)
{

	uint32 i, j;

	for (i = 0; i < sparseMap.numberOfKeys; ++i)
	{
		const SparseMap::Button &sparseKey = sparseMap.key[i];
		Map::Button             &mapKey    = m_currentMap.key[sparseKey.id];
		mapKey.cmd                         = sparseKey.cmd;
	}
	
	// expand the sparse joystick button map into the currentMap
	for (i = 0; i < sparseMap.numberOfMouseButtons; ++i)
	{
		const SparseMap::Button &sparseMouseButton = sparseMap.mouseButton[i];
		Map::Button             &mapMouseButton    = m_currentMap.mouseButton[sparseMouseButton.id];
		mapMouseButton.cmd                         = sparseMouseButton.cmd;
	}
	
	for (j = 0; j < MAX_JOYSTICKS; ++j)
	{
		// expand the sparse joystick button map into the currentMap
		for (i = 0; i < sparseMap.joystick[j].numberOfJoystickButtons; ++i)
		{
			const SparseMap::Button &sparseJoystickButton = sparseMap.joystick[j].joystickButton[i];
			Map::Button             &mapJoystickButton    = m_currentMap.joystick[j].joystickButton[sparseJoystickButton.id];
			mapJoystickButton.cmd                         = sparseJoystickButton.cmd;
		}
		
		// expand the sparse joystick button map into the currentMap
		for (i = 0; i < sparseMap.joystick[j].numberOfJoystickAxis; ++i)
		{
			const SparseMap::Axis   &sparseJoystickAxis   = sparseMap.joystick[j].joystickAxis[i];
			Map::Axis               &mapJoystickAxis      = m_currentMap.joystick[j].joystickAxis[sparseJoystickAxis.id];
			mapJoystickAxis.cmd                           = sparseJoystickAxis.cmd;
		}
		
		// expand the sparse joystick slider map into the currentMap
		for (i = 0; i < sparseMap.joystick[j].numberOfJoystickSliders; ++i)
		{
			const SparseMap::Slider &sparseJoystickSlider = sparseMap.joystick[j].joystickSlider[i];
			Map::Slider             &mapJoystickSlider    = m_currentMap.joystick[j].joystickSlider[sparseJoystickSlider.id];
			mapJoystickSlider.cmd                         = sparseJoystickSlider.cmd;
		}

		// expand the sparse joystick pov hat map into the currentMap
		for (i = 0; i < sparseMap.joystick[j].numberOfJoystickPovHats; ++i)
		{
			const SparseMap::PovHat    &sparseJoystickPovHat  = sparseMap.joystick[j].joystickPovHat[i];
			Map::PovHat                &mapJoystickPovHat     = m_currentMap.joystick[j].joystickPovHat[sparseJoystickPovHat.id];

			mapJoystickPovHat.cmd                             = sparseJoystickPovHat.cmd;
		}
	}
}

// ----------------------------------------------------------------------
/**
 * Set a new shift state.
 * 
 * This routine will determine the appropriate sparse map and decompress
 * it into the current map.
 */

void InputMap::activateNewShiftState ()
{
	SparseMap *sparseMap;
	uint32     i, j;

	// recompute the shift state from the shift state bit counts
	m_shiftState = 0;
	for (i = 0; i < MAX_SHIFT_BITS; ++i)
		if (m_shiftStateBitCount[i])
			m_shiftState |= 1U << i;

	// wipe the current map
	Zero(m_currentMap);

	// search for the sparse map
	for (sparseMap = m_firstSparseMap; sparseMap && sparseMap->shiftState != m_shiftState; sparseMap = sparseMap->next)
		;

	// if the sparse map exists
	if (sparseMap)
	{
		// populate the map with the defaults from the unshifted keyboard
		if (m_defaultSparseMap && sparseMap->absorbDefaultShiftMaps)
			expandSparseMap (*m_defaultSparseMap);

		expandSparseMap (*sparseMap);
	}

	NOT_NULL(m_shifts);

	// overlay all the shifting keys onto the current map
	for (i = 0; i < m_shifts->numberOfKeys; ++i)
	{
		Map::Button &mapButton   = m_currentMap.key[m_shifts->key[i].button];
		mapButton.shiftBits      = m_shifts->key[i].bits;
	}

	// overlay all the shifting mouse buttons onto the current map
	for (i = 0; i < m_shifts->numberOfMouseButtons; ++i)
	{
		Map::Button &mapButton = m_currentMap.mouseButton[m_shifts->mouseButton[i].button];
		mapButton.shiftBits    = m_shifts->mouseButton[i].bits;
	}

	// overlay all the shifting joystick buttons onto the current map
	for (j = 0; j < MAX_JOYSTICKS; ++j)
	{
		for (i = 0; i < m_shifts->joystick[j].numberOfJoystickButtons; ++i)
		{
			const int    buttonNumber = m_shifts->joystick[j].joystickButton[i].button;
			Map::Button &mapButton    = m_currentMap.joystick[j].joystickButton[buttonNumber];
			mapButton.shiftBits       = m_shifts->joystick[j].joystickButton[i].bits;
		}
	}
}

// ----------------------------------------------------------------------

void InputMap::processButtonPress(const Map::Button &mapButton, InputType type, int value)
{
	NOT_NULL (m_repeat);

	Repeat::Button repeatButton = { Repeat::JustPressed, 0, 0, value, type, 0.0f };
	
	// send the message if appropriate
	if (m_messageQueue && mapButton.cmd)
		IGNORE_RETURN(mapButton.cmd->pressEvent.execute (*m_messageQueue, 0));

	//----------------------------------------------------------------------
	//-- see if the button is already down, and skip incrementing the repeat
	if (m_repeat->buttons.end () != std::find (m_repeat->buttons.begin (), m_repeat->buttons.end (), repeatButton))
		return;

	repeatButton.cmd                  = mapButton.cmd;
	repeatButton.repeatStartCountdown = (mapButton.cmd && mapButton.cmd->repeatStartDelay) ? s_repeatDelayTime : 0.0f;

	//-- handle shifting key

	if (mapButton.shiftBits)
	{
		repeatButton.shiftBits      = mapButton.shiftBits;
		addShiftBits(mapButton.shiftBits);
	}
	else
	{
		repeatButton.shiftBits      = 0;
	}

	m_repeat->buttons.remove (repeatButton);
	m_repeat->buttons.push_back (repeatButton);

}

// ----------------------------------------------------------------------

void InputMap::processButtonRelease (InputType type, int value)
{
	const Repeat::Button buttonFinder = { Repeat::JustPressed, 0, 0, value, type, 0.0f };

	NOT_NULL (m_repeat);
	Repeat::ButtonContainer_t::iterator fit = std::find (m_repeat->buttons.begin (), m_repeat->buttons.end (), buttonFinder);

	if (fit == m_repeat->buttons.end ()) //lint !e1702 //stl
	{
		return;
	}
	
	const Repeat::Button & repeatButton = *fit; //lint !e1702 //stl
	
	if (repeatButton.shiftBits)
	{
		// handle shifting key
		removeShiftBits(repeatButton.shiftBits);
	}
	
	// send the message if appropriate
	if (m_messageQueue && repeatButton.cmd)
		IGNORE_RETURN(repeatButton.cmd->releaseEvent.execute (*m_messageQueue, 0));

	IGNORE_RETURN (m_repeat->buttons.erase (fit));
}

// ----------------------------------------------------------------------

void InputMap::processJoystickPovHat(int joystickIndex, int hatNumber, real direction)
{
	DEBUG_FATAL(hatNumber < 0 || hatNumber >= MAX_JOYSTICK_POV_HATS, ("joystick pov out of range %d/%d", hatNumber, MAX_JOYSTICK_POV_HATS));

	NOT_NULL(m_repeat);

	Repeat::PovHat & repeatPovHat = m_repeat->joystick[joystickIndex].joystickPovHat[hatNumber];

	// check for the hat off center
	if (direction >= 0)
	{
		// make sure it's a new value
		if (repeatPovHat.state == Repeat::Up || direction != repeatPovHat.direction)  //lint !e777 // testing floats for equality
		{
			// update the current state
			const Map::PovHat  &mapPovHat     = m_currentMap.joystick[joystickIndex].joystickPovHat[hatNumber];
			repeatPovHat.cmd                  = mapPovHat.cmd;
			repeatPovHat.state                = Repeat::JustPressed;
			repeatPovHat.direction            = direction;
			repeatPovHat.repeatStartCountdown = (mapPovHat.cmd && mapPovHat.cmd->repeatStartDelay) ? s_repeatDelayTime : 0.0f;
	
			// send the message if one is set
			if (m_messageQueue && repeatPovHat.cmd)
				IGNORE_RETURN(repeatPovHat.cmd->pressEvent.execute (*m_messageQueue, 0));
		}
	}
	else
	{
		// make sure it's a new value
		if (repeatPovHat.state != Repeat::Up)
		{
			// send the message if one is set
			if (m_messageQueue && repeatPovHat.cmd)
				IGNORE_RETURN(repeatPovHat.cmd->releaseEvent.execute (*m_messageQueue, 0));

			// update the current state
			repeatPovHat.state = Repeat::Up;
		}
	}
}

// ----------------------------------------------------------------------

void InputMap::beginFrame(void)
{
	NOT_NULL(m_messageQueue);
	m_messageQueue->beginFrame();
}

// ----------------------------------------------------------------------
/**
 * Process an event.
 * 
 * This routine will use the specified event along with the map data and the
 * current state data to enqueue the appropriate messages to the MessageQueue.
 * 
 * @param event  [In] Input event to process
 */

void InputMap::processEvent(const IoEvent *event)
{
	NOT_NULL(event);

	switch (event->type)
	{
	case IOET_KeyDown:
		{
			DEBUG_FATAL(event->arg2 < 0 || event->arg2 >= MAX_KEYS, ("key out of range %d/%d", event->arg2, MAX_KEYS));
			
			if (event->arg1 == m_keyboardNumber)
				processButtonPress(m_currentMap.key[event->arg2], IT_Key, event->arg2);			
		}
		break;
		
	case IOET_KeyUp:
		{
			DEBUG_FATAL(event->arg2 < 0 || event->arg2 >= MAX_KEYS, ("key out of range %d/%d", event->arg2, MAX_KEYS));
			
			if (event->arg1 == m_keyboardNumber)
				processButtonRelease(IT_Key, event->arg2);
			
		}
		break;
		
	case IOET_JoystickButtonDown:
		{
			DEBUG_FATAL(event->arg2 < 0 || event->arg2 >= MAX_JOYSTICK_BUTTONS, ("joystick button out of range %d/%d", event->arg2, MAX_JOYSTICK_BUTTONS));
			
			for (int j = 0; j < MAX_JOYSTICKS; ++j)
				if (event->arg1 == m_joystickNumber[j])
				{
					processButtonPress(m_currentMap.joystick[j].joystickButton[event->arg2], IT_JoyButton, event->arg2);
				}
		}
		break;
		
	case IOET_JoystickButtonUp:
		{
			DEBUG_FATAL(event->arg2 < 0 || event->arg2 >= MAX_JOYSTICK_BUTTONS, ("joystick button out of range %d/%d", event->arg2, MAX_JOYSTICK_BUTTONS));
			
			for (int j = 0; j < MAX_JOYSTICKS; ++j)
				if (event->arg1 == m_joystickNumber[j])
				{
					processButtonRelease(IT_JoyButton, event->arg2);	
				}
		}
		break;
		
	case IOET_MouseButtonDown:
		{
			DEBUG_FATAL(event->arg2 < 0 || event->arg2 >= MAX_MOUSE_BUTTONS, ("mouse button out of range %d/%d", event->arg2, MAX_MOUSE_BUTTONS));
			
			if (event->arg1 == m_mouseNumber)				
				processButtonPress(m_currentMap.mouseButton[event->arg2], IT_MouseButton, event->arg2);			
		}
		break;
		
	case IOET_MouseButtonUp:
		{
			DEBUG_FATAL(event->arg2 < 0 || event->arg2 >= MAX_MOUSE_BUTTONS, ("mouse button out of range %d/%d", event->arg2, MAX_MOUSE_BUTTONS));
			
			if (event->arg1 == m_mouseNumber)				
				processButtonRelease(IT_MouseButton, event->arg2);			
		}
		break;
		
	case IOET_JoystickMove:
		{
			DEBUG_FATAL(event->arg2 < 0 || event->arg2 >= MAX_JOYSTICK_AXIS, ("joystick axis out of range %d/%d", event->arg2 , MAX_JOYSTICK_AXIS));
			
			if (m_messageQueue)	
			{
				for (int j = 0; j < MAX_JOYSTICKS; ++j)
				{
					const Command * const cmd = m_currentMap.joystick[j].joystickAxis[event->arg2].cmd;
					if (event->arg1 == m_joystickNumber[j] && cmd && cmd->pressEvent.message)
						m_messageQueue->appendMessage(cmd->pressEvent.message, event->arg3 * cmd->pressEvent.value);
				}
			}
		}
		break;
		
	case IOET_JoystickSlider:
		{
			DEBUG_FATAL(event->arg2 < 0 || event->arg2 >= MAX_JOYSTICK_SLIDERS, ("joystick axis out of range %d/%d", event->arg2 , MAX_JOYSTICK_SLIDERS));
			
			if (m_messageQueue)
			{
				for (int j = 0; j < MAX_JOYSTICKS; ++j)
				{
					const Command * const cmd = m_currentMap.joystick[j].joystickSlider[event->arg2].cmd;
					if (event->arg1 == m_joystickNumber[j] && cmd && cmd->pressEvent.message)
						m_messageQueue->appendMessage(cmd->pressEvent.message, event->arg3 * cmd->pressEvent.value);
				}
			}
		}
		break;

	case IOET_JoystickPovHat:
		{
			for (int j = 0; j < MAX_JOYSTICKS; ++j)
				if (event->arg1 == m_joystickNumber[j])
					processJoystickPovHat(j, event->arg2, event->arg3);
		}
		break;

	case IOET_InputReset:
		handleInputReset();
		break;

	default:
			break;

	} //lint !e788 enum constant not used within defaulted switch
}

// ----------------------------------------------------------------------

void InputMap::handleInputReset(void)
{
	int i, j;

	NOT_NULL (m_repeat);

	while (!m_repeat->buttons.empty ())
	{
		Repeat::Button &button = m_repeat->buttons.front ();

		if (m_messageQueue && button.cmd)
			IGNORE_RETURN(button.cmd->resetEvent.execute (*m_messageQueue, 0));
		if (m_messageQueue && button.cmd)
			IGNORE_RETURN(button.cmd->releaseEvent.execute (*m_messageQueue, 0));

		IGNORE_RETURN (m_repeat->buttons.erase (m_repeat->buttons.begin ()));
	}

	// for all joystick axis, send the reset message
	for (j = 0; j < MAX_JOYSTICKS; ++j)
		for (i = 0; i < MAX_JOYSTICK_AXIS; ++i)
		{
			Map::Axis &axis = m_currentMap.joystick[j].joystickAxis[i];

			if (m_messageQueue && axis.cmd)
				IGNORE_RETURN(axis.cmd->resetEvent.execute (*m_messageQueue, 0));
			if (m_messageQueue && axis.cmd)
				IGNORE_RETURN(axis.cmd->releaseEvent.execute (*m_messageQueue, 0));
		}

	// for all joystick sliders, send the reset message
	for (j = 0; j < MAX_JOYSTICKS; ++j)
		for (i = 0; i < MAX_JOYSTICK_SLIDERS; ++i)
		{
			Map::Slider &slider= m_currentMap.joystick[j].joystickSlider[i];

			
			if (m_messageQueue && slider.cmd)
				IGNORE_RETURN(slider.cmd->resetEvent.execute (*m_messageQueue, 0));
			if (m_messageQueue && slider.cmd)
				IGNORE_RETURN(slider.cmd->releaseEvent.execute (*m_messageQueue, 0));
		}

	// for all joystick pov hats, send the reset message
	for (j = 0; j < MAX_JOYSTICKS; ++j)
		for (i = 0; i < MAX_JOYSTICK_POV_HATS; ++i)
		{
			Repeat::PovHat &hat = m_repeat->joystick[j].joystickPovHat[i];

			if (hat.state != Repeat::Up)
			{
				
				if (m_messageQueue && hat.cmd)
					IGNORE_RETURN(hat.cmd->resetEvent.execute (*m_messageQueue, 0));
				if (m_messageQueue && hat.cmd)
					IGNORE_RETURN(hat.cmd->releaseEvent.execute (*m_messageQueue, 0));

				hat.state = Repeat::Up;
			}
		}

	// reset the shift bits and invoke the new shift state
	memset(m_shiftStateBitCount, 0, sizeof(m_shiftStateBitCount));
	activateNewShiftState();
}

// ----------------------------------------------------------------------
/**
 * Handle adding repeat messages to the MessageQueue.
 * 
 * This routine will examine the state data and add any appropriate repeat
 * messages to the MessageQueue.
 */

void InputMap::handleRepeats (float elapsedTime)
{
	NOT_NULL (m_repeat);
	// -qq- it would be nice if we didn't have to iterate through all these tables
	// -qq- don't really like the cut & paste here

	const Repeat::ButtonContainer_t::iterator end = m_repeat->buttons.end ();

	for (Repeat::ButtonContainer_t::iterator it = m_repeat->buttons.begin (); it != end; ++it) //lint !e1702 //stl
	{
		Repeat::Button &button = *it; //lint !e1702 //stl

		if (button.state == Repeat::JustPressed)
		{
			button.repeatStartCountdown -= elapsedTime;

			if (button.repeatStartCountdown <= 0.0f)
				button.state = Repeat::Repeating;
		}
		else 
		{
			if (m_messageQueue && button.cmd && button.state == Repeat::Repeating)
			{
				IGNORE_RETURN(button.cmd->repeatEvent.execute (*m_messageQueue, 0));
			}
		}
	}

	// handle repeats for the joystick pov hats
	for (int j = 0; j < MAX_JOYSTICKS; ++j)
	{
		for (int i = 0; i < MAX_JOYSTICK_POV_HATS; ++i)
		{
			Repeat::PovHat &hat = m_repeat->joystick[j].joystickPovHat[i];

			if (hat.state == Repeat::JustPressed)
			{
				hat.repeatStartCountdown -= elapsedTime;
				if (hat.repeatStartCountdown <= 0.0f)
					hat.state = Repeat::Repeating;
			}
			else
			{
				if (m_messageQueue && hat.state == Repeat::Repeating && hat.cmd)
					IGNORE_RETURN(hat.cmd->repeatEvent.execute(*m_messageQueue, &hat.direction));
			}
		}
	}
}

//-----------------------------------------------------------------

InputMap::CommandBindInfoSet::~CommandBindInfoSet ()
{
	delete[] binds;
	binds = 0;

	cmd = 0;
}

//-----------------------------------------------------------------

InputMap::CommandBindInfoSet::CommandBindInfoSet () :
cmd (0),
binds (0),
numBinds (0)
{
}

//-----------------------------------------------------------------
/**
* Remove all bindings for the given command
*/
void InputMap::removeBindings (const Command * cmd)
{	
	for (SparseMap * sm = m_firstSparseMap; sm; sm = sm->next)
	{
		sm->removeBindings (cmd);
	}		
}

//-----------------------------------------------------------------

/**
* Get the bindinfos for a given command, or all commands.
* @param cmd if null, then the bindings for all commands are returned
* @param cmdBindInfoSets [output] if non-null after return, client must delete[] it when finished
* @return the number of command bindings returned in the cmdBindInfoSets pointer reference
*
*/

uint32 InputMap::getCommandBindings (CommandBindInfoSet *& cmdBindInfoSets, const Command * cmd) const
{
	NOT_NULL (m_cmds);

	//-----------------------------------------------------------------
	//-- first determine the number of commands to process
	uint32 set_size = 0;

	if (cmd == 0)
		set_size = m_cmds->size ();
	else
	{
		for (CommandVector::iterator it = m_cmds->begin (); it != m_cmds->end (); ++it)
		{
			const Command * const theCmd = *it;
			if (cmd == theCmd)
				++set_size;
		}
	}

	if (set_size == 0)
	{
		cmdBindInfoSets	= 0;
		return 0;
	}

	cmdBindInfoSets = new CommandBindInfoSet [set_size];

	//-----------------------------------------------------------------

	uint32 array_index = 0;
	for (CommandVector::iterator it = m_cmds->begin (); it != m_cmds->end (); ++it)
	{
		const Command * const theCmd = *it;

		if (cmd && cmd != theCmd)
			continue;

		DEBUG_FATAL (array_index >= set_size, ("array index out of bounds.\n"));

		CommandBindInfoSet & cbis = cmdBindInfoSets [array_index++];

		cbis.cmd = theCmd;

		NOT_NULL (cbis.cmd);

		std::vector<BindInfo> biv;

		uint32 j;

		//-----------------------------------------------------------------
		//-- first search the shifts for bound keys

		NOT_NULL(m_shifts);
		m_shifts->getCommandBindings (biv, cbis.cmd);

		//-----------------------------------------------------------------
		//-- keyboard shifts


		for (SparseMap * sm = m_firstSparseMap; sm; sm = sm->next)
		{
			sm->getCommandBindings (biv, cbis.cmd);
		} 

		//-----------------------------------------------------------------
		// end 	for (SparseMap * sm = firstSparseMap; sm; sm = sm->next)

		if (biv.empty ())
		{
			cbis.binds    = 0;
			cbis.numBinds = 0;
		}
		else
		{
			cbis.numBinds = biv.size ();
			cbis.binds = new BindInfo [cbis.numBinds];

			const std::vector<BindInfo>::const_iterator end = biv.end ();

			j = 0;

			for (std::vector<BindInfo>::const_iterator bit = biv.begin (); bit != end; ++bit, ++j)
			{ 
				cbis.binds [j] = *bit;
			}
		}
	}

	return set_size;
}
//-----------------------------------------------------------------
/**
* Bind an input state represented by a BindInfo, with the specified command.
*/
bool InputMap::addBinding (const BindInfo & binfo, const Command * cmd)
{
	SparseMap * sm = m_firstSparseMap;

	for (; sm && sm->shiftState != binfo.shiftState; sm = sm->next)
		;

	//-----------------------------------------------------------------
	//-- create the sparseMap if needed

	if (sm == 0)
	{
		sm = new SparseMap;

		Zero (*sm);

		sm->shiftState = binfo.shiftState;

		if (m_lastSparseMap)
			m_lastSparseMap->next = sm;
		else
			m_firstSparseMap = sm;

		m_lastSparseMap       = sm;
	}
	
	bool retval = sm->addBinding(binfo, cmd);
	activateNewShiftState ();

	return retval;
}

//-----------------------------------------------------------------
/**
* Given an input state represented by a BindInfo, find the unique command
* which is bound to this state. 
* @return null if no command is bound to this state
*/

const InputMap::Command * InputMap::getCommandByBinding (const BindInfo & binfo) const
{
	for (const SparseMap * sm = m_firstSparseMap; sm; sm = sm->next)
	{
		if (sm->shiftState == binfo.shiftState)
			return sm->getCommandByBinding (binfo);
	}

	if (binfo.shiftState != 0)
	{
		//-- no matching shiftstate was found.  This means that the requested shiftState does not exist in this
		//-- inputmap.  Therefore, just use the default zero shiftstate
		BindInfo unshiftedBinfo(binfo);
		unshiftedBinfo.shiftState = 0;
		return getCommandByBinding(unshiftedBinfo);
	}

	return NULL;
}

//-----------------------------------------------------------------
/**
* Obtain a BindInfo object that represents the current input state.
* This is done by scanning the list of current repeats.  If multiple
* non-modifiers are down, then the most recently pressed one is used
* as the BindInfo value.  If no non-modifiers are down, then the most
* recently pressed modifier is used as the BindInfo value.  A BindInfo
* type of IT_None indicates no appropriate bind representation exists.
*
*/

InputMap::BindInfo InputMap::getBindInfo () const
{
	BindInfo bi;

	NOT_NULL (m_repeat);

	const Repeat::ButtonContainer_t::reverse_iterator rend = m_repeat->buttons.rend ();

	const Repeat::Button * btn = 0;

	for (Repeat::ButtonContainer_t::reverse_iterator rit = m_repeat->buttons.rbegin (); rit != rend; ++rit)  //lint !e55 !e81 //stl
	{
		if ((*rit).shiftBits) //lint !e1702 //stl
		{
			if (btn == 0)
			{
				btn  = &(*rit);  //lint !e1702 //stl
			}
		}
		else
		{
			bi.shiftState = m_shiftState;
			bi.type       = (*rit).type;//lint !e1702 //stl
			bi.value      = (*rit).value;//lint !e1702 //stl
			return bi;
		}
	}

	if (btn)
	{
		bi.shiftState = m_shiftState & ~(btn->shiftBits);
		bi.value      = btn->value;
		bi.type       = btn->type;
	}

	//-----------------------------------------------------------------
	//-- no buttons available, check for povhats
	//-- this check is not done in pressed-order, first in array first serve

	if (bi.type == IT_None)
	{
		for (uint32 j = 0; j < MAX_JOYSTICKS && bi.type == IT_None; ++j)
		{
			const Repeat::Joystick & joy = m_repeat->joystick [j];

			for (uint32 i = 0; i < MAX_JOYSTICK_POV_HATS; ++i)
			{
				if (joy.joystickPovHat [i].state != Repeat::Up)
				{
					bi.value = static_cast<int32>(i);
					bi.type  = IT_JoyPovHat;
					break;
				}
			}
		}
	}

	return bi;
}
//-----------------------------------------------------------------
/**
* Replace the sparseMaps in this with a copy of the sparseMaps in rhs.
* The cmds pointers of both InputMaps must point to the same data.
*
* This method is used privately in conjunction with cancel/apply capable rebinding menus
* The interface to the menu clients should be
*
* @see getRebindingMap ()
* @see applyRebindingMapChanges ()
*/

void InputMap::applyRebindingMapChanges (const InputMap & rhs)
{
	DEBUG_FATAL (m_cmds != rhs.m_cmds, ("Incompatible inputmaps.\n"));

	while (m_firstSparseMap)
	{
		SparseMap * tmp = m_firstSparseMap;
		m_firstSparseMap = tmp->next;
		delete tmp;
	}

	m_firstSparseMap   = 0;
	m_lastSparseMap    = 0;
	m_defaultSparseMap = 0;

	for (SparseMap * sm = rhs.m_firstSparseMap; sm; sm = sm->next)
	{

		SparseMap * nsm = new SparseMap;
		*nsm = *sm;
		nsm->next = 0;

		nsm->key = new SparseMap::Button [nsm->numberOfKeys];
		memcpy (nsm->key, sm->key, sm->numberOfKeys * sizeof (SparseMap::Button));

		nsm->mouseButton = new SparseMap::Button [nsm->numberOfMouseButtons];
		memcpy (nsm->mouseButton, sm->mouseButton, sm->numberOfMouseButtons * sizeof (SparseMap::Button));
		
		for (uint32 j = 0; j < MAX_JOYSTICKS; ++j)
		{			
			const SparseMap::Joystick & joy = sm->joystick [j];
			SparseMap::Joystick & njoy      = nsm->joystick [j];

			njoy = joy;
			
			njoy.joystickButton = new SparseMap::Button [njoy.numberOfJoystickButtons];
			memcpy (njoy.joystickButton, joy.joystickButton, joy.numberOfJoystickButtons * sizeof (SparseMap::Button));
			
			njoy.joystickAxis = new SparseMap::Axis [njoy.numberOfJoystickAxis];
			memcpy (njoy.joystickAxis, joy.joystickAxis, joy.numberOfJoystickAxis * sizeof (SparseMap::Axis));

			njoy.joystickSlider = new SparseMap::Slider [njoy.numberOfJoystickSliders];
			memcpy (njoy.joystickSlider, joy.joystickSlider, joy.numberOfJoystickSliders * sizeof (SparseMap::Slider));
			
			njoy.joystickPovHat = new SparseMap::PovHat [njoy.numberOfJoystickPovHats];
			memcpy (njoy.joystickPovHat, joy.joystickPovHat, joy.numberOfJoystickPovHats * sizeof (SparseMap::PovHat));
		}
		
		if (sm->shiftState == 0)
			m_defaultSparseMap = nsm;

		if (m_lastSparseMap)
		{
			m_lastSparseMap->next = nsm;
			m_lastSparseMap = nsm;
		}
		else
		{
			m_firstSparseMap = nsm;
			m_lastSparseMap = nsm;
		}
	}

	activateNewShiftState();

}

//-----------------------------------------------------------------
/**
* Acquire a map suitable for doing rebindings.  The cmds pointer is shared with this map.
* The returned map has its own copy of the sparseMap.  Once the rebindingMap is massaged
* into the state you want, use applyRebindingMapChanges () to copy the sparseMap back into
* the parent map.
* 
*/
InputMap * InputMap::getRebindingMap () const
{
	InputMap * const rbm = new InputMap;

	delete rbm->m_cmds;
	rbm->m_cmds     = m_cmds;

	rbm->m_keyboardNumber = m_keyboardNumber;
	rbm->m_mouseNumber    = m_mouseNumber;

	rbm->m_commandFilename    = m_commandFilename;
	rbm->m_customFilename     = m_customFilename;
	rbm->m_baseFilename       = m_baseFilename;

	NOT_NULL(rbm->m_commandCategories);
	NOT_NULL(m_commandCategories);
	*rbm->m_commandCategories = *m_commandCategories;

	uint32 j;
	for (j = 0; j < MAX_JOYSTICKS; ++j)
		rbm->m_joystickNumber [j] = m_joystickNumber [j];

	//-- we must _not_ delete the cmds
	rbm->m_ownsCmds = false;

	NOT_NULL(rbm->m_shifts);
	NOT_NULL(m_shifts);
	rbm->m_shifts->copy (*m_shifts);

	//-----------------------------------------------------------------
	//-- copy the sparsemaps

	rbm->applyRebindingMapChanges (*this);
	return rbm;
}

//----------------------------------------------------------------------

/**
* Find a command by name, and run it.
* Performs a slow, linear search through the commands.
*/

bool InputMap::executeCommandByName (const char * name, const float * overrideValuePress, const float * overrideValueRepeat, const float * overrideValueRelease)
{
	NOT_NULL (name);
	NOT_NULL (m_messageQueue);

	const Command * const cmd = findCommandByName (name, true);

	if (cmd)
	{
		cmd->execute (*m_messageQueue, overrideValuePress, overrideValueRepeat, overrideValueRelease);
		return true;
	}

	return false;
}

//----------------------------------------------------------------------

const InputMap::Command * InputMap::findCommandByString (const char * name, bool optional) const
{
	NOT_NULL (name);
	NOT_NULL(m_cmds);

	for (CommandVector::iterator it = m_cmds->begin (); it != m_cmds->end (); ++it)
	{
		const Command * const cmd = NON_NULL (*it);
		if (!_stricmp (name, cmd->pressEvent.str.c_str ()))
			return cmd;
	}

	UNREF(optional);
	DEBUG_FATAL (!optional, ("Unable to find command by name %s\n", name));
	return 0;
}

//----------------------------------------------------------------------

const InputMap::Command * InputMap::findCommandByName (const char * name, bool optional) const
{
	NOT_NULL (name);

	//-- empty string is ok
	if (!*name)
		return 0;

	NOT_NULL(m_cmds);
	for (CommandVector::iterator it = m_cmds->begin (); it != m_cmds->end (); ++it)
	{
		const Command * const cmd = NON_NULL (*it);
		if (!_stricmp (name, cmd->name.c_str ()))
			return cmd;
	}

	UNREF(optional);
	DEBUG_FATAL (!optional, ("Unable to find command by name %s\n", name));
	return 0;
}

//----------------------------------------------------------------------

uint32 InputMap::getNumCommands () const
{
	return NON_NULL(m_cmds)->size ();
}

//----------------------------------------------------------------------

/** Set the keyboard that this InputMap watches
*
* Remarks:
*
*   Pass in -1 to ignore all keyboard input
*/

void InputMap::setKeyboard (int newKeyboardNumber)
{
	DEBUG_FATAL(newKeyboardNumber < -1, ("bad keyboard number %d", newKeyboardNumber));
	m_keyboardNumber = newKeyboardNumber;
}

// ----------------------------------------------------------------------
/**
 * Set the mouse that this InputMap watches.
 * 
 * Pass in -1 to ignore all mouse input
 * 
 * @param newMouseNumber  New mouse device to watch
 */

void InputMap::setMouse (int newMouseNumber)
{
	DEBUG_FATAL(newMouseNumber < -1, ("bad mouse number %d", newMouseNumber));
	m_mouseNumber = newMouseNumber;
}

// ----------------------------------------------------------------------
/**
 * Set the joystick that this InputMap joystick slot watches.
 * 
 * Pass in -1 to newJoystickNumber ignore this joystick slot
 * 
 * @param joystickIndex  Joystick entry to change
 * @param newJoystickNumber  New joystick device to watch
 */

void InputMap::setJoystick (int joystickIndex, int newJoystickNumber)
{
	DEBUG_FATAL(joystickIndex < 0 || joystickIndex >= MAX_JOYSTICKS, ("bad joystick index %d", joystickIndex));
	DEBUG_FATAL(newJoystickNumber < -1, ("bad joystick number %d", newJoystickNumber));
	m_joystickNumber[joystickIndex] = newJoystickNumber;
}

//----------------------------------------------------------------------

const InputMap::StringVector & InputMap::getCommandCategories  () const
{
	return *NON_NULL(m_commandCategories);
}

//----------------------------------------------------------------------

void InputMap::getCommandsByCategory (const std::string & category, CommandVector & cv)
{
	NOT_NULL(m_cmds);
	for (CommandVector::const_iterator it = m_cmds->begin (); it != m_cmds->end (); ++it)
	{
		const Command * const cmd = *it;
		if (cmd->category == category)
			cv.push_back (cmd);
	}
}

//----------------------------------------------------------------------

bool InputMap::addCustomCommand      (const std::string & name, int message, const std::string & str, bool canModify)
{
	Command cmd;
	cmd.name = name;
	cmd.pressEvent.message = message;
	cmd.pressEvent.str     = str;
	cmd.types              = Command::T_ANY;
	cmd.category           = "custom";
	cmd.userDefined        = true;

	return addCustomCommand (cmd, canModify);
}

//----------------------------------------------------------------------

bool InputMap::addCustomCommand        (const Command & cmd, bool canModify)
{
	DEBUG_FATAL (!m_ownsCmds, ("InputMap::addCustomCommand won't work if it doesn't own the commands."));

	Command * const existing = const_cast<Command *>(findCommandByName (cmd.name.c_str (), true));
	if (existing)
	{
		if (!canModify)
		{
			DEBUG_FATAL (true, ("InputMap::addCustomCommand can't add existing command [%s]", cmd.name.c_str ()));			
			return false; //lint !e527 //unreachable
		}

		if (!existing->userDefined)
		{
			DEBUG_FATAL (true, ("InputMap::addCustomCommand can't modify non-userDefined command [%s]", cmd.name.c_str ()));
			return false; //lint !e527 //unreachable
		}

		existing->copy (cmd);
		existing->userDefined = true;
	}
	else
	{
		Command * const newCmd = new Command;
		newCmd->copy (cmd);
		newCmd->userDefined = true;
		NOT_NULL(m_cmds);
		m_cmds->push_back (newCmd);
		std::sort (m_cmds->begin (), m_cmds->end (), CommandComparator ());
	}

	return true;
}

//----------------------------------------------------------------------

bool InputMap::removeCustomCommand     (const std::string & name)
{
	DEBUG_FATAL (!m_ownsCmds, ("InputMap::removeCustomCommand won't work if it doesn't own the commands."));

	const Command * const existing = findCommandByName (name.c_str (), true);
	if (!existing)
	{
		WARNING (true, ("InputMap::removeCustomCommand can't remove existing command [%s]", name.c_str ()));
		return false;
	}

	if (!existing->userDefined)
	{
		DEBUG_FATAL (true, ("InputMap::removeCustomCommand can't remove non-userDefined command [%s]", name.c_str ()));
		return false; //lint !e527 //unreachable
	}

	removeBindings (existing);
	NOT_NULL(m_shifts);
	m_shifts->removeBindings (*existing);

	NOT_NULL(m_cmds);
	IGNORE_RETURN(m_cmds->erase (std::remove (m_cmds->begin (), m_cmds->end (), existing)));
	delete const_cast<Command *>(existing);
	return true;
}

// ======================================================================
