// ======================================================================
//
// InputMap.h
//
// Portions copyright 1999 Bootprint Entertainment
// Portions copyright 2001-2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INPUT_MAP_H
#define INPUT_MAP_H

// ======================================================================

struct IoEvent;
class  Iff;
class  MessageQueue;

#include <string>

// ======================================================================

// Map input events to messages.

class InputMap
{
public:

	enum
	{
		MAX_JOYSTICKS          = 1,

		MAX_KEYS               = 255,
		MAX_MOUSE_BUTTONS      = 8,
		MAX_JOYSTICK_BUTTONS   = 32,
		MAX_JOYSTICK_AXIS      = 6,
		MAX_JOYSTICK_SLIDERS   = 2,
		MAX_JOYSTICK_POV_HATS  = 4,
		MAX_SHIFT_BITS         = 32
	};

	class Command;

	//-----------------------------------------------------------------
	/**
	* The type of input with which a generic value identifier is associated.
	*/
	enum InputType
	{
		IT_None,
		IT_Key,
		IT_MouseButton,
		IT_JoyButton,
		IT_JoyAxis,
		IT_JoySlider,
		IT_JoyPovHat
	};

	//-----------------------------------------------------------------
	/**
	* class that indicates which button or control is bound to a given command, adn the shift state for it
	*/

	struct BindInfo
	{
		uint32    shiftState;
		InputType type;
		int32     value;
		BindInfo ();
		BindInfo (uint32 theShiftStaate, InputType theType, int32 theValue);
	};

	//-----------------------------------------------------------------

	struct CommandBindInfoSet
	{
		const Command * cmd;
		BindInfo      * binds;
		uint32          numBinds;

		~CommandBindInfoSet ();
		CommandBindInfoSet ();

	private:
		CommandBindInfoSet & operator= (const CommandBindInfoSet &);
		CommandBindInfoSet (const CommandBindInfoSet &);
	};

	//-----------------------------------------------------------------

	typedef stdvector<const Command *>::fwd CommandVector;
	typedef stdvector<std::string>::fwd     StringVector;
	
	struct Shifts;

private:

	struct SparseMap;

	struct Map
	{
		struct Button
		{
			const Command * cmd;
			uint32          shiftBits;
		};

		struct Axis
		{
			const Command * cmd;
		};

		struct Slider
		{
			const Command * cmd;
		};

		struct PovHat
		{
			const Command * cmd;
		};

		struct Joystick
		{
			Button  joystickButton[MAX_JOYSTICK_BUTTONS];
			Axis    joystickAxis[MAX_JOYSTICK_AXIS];
			Slider  joystickSlider[MAX_JOYSTICK_SLIDERS];
			PovHat  joystickPovHat[MAX_JOYSTICK_POV_HATS];
		};

		Button   key[MAX_KEYS];
		Button   mouseButton[MAX_MOUSE_BUTTONS];
		Joystick joystick[MAX_JOYSTICKS];
		Map();
	};

private:

	SparseMap      *m_firstSparseMap;
	SparseMap      *m_lastSparseMap;
	SparseMap      *m_defaultSparseMap;

	MessageQueue   *m_messageQueue;

	int             m_keyboardNumber;
	int             m_mouseNumber;
	int             m_joystickNumber[MAX_JOYSTICKS];

	Shifts *        m_shifts;
	Map             m_currentMap;

	struct Repeat;
	Repeat *        m_repeat;

	uint32          m_shiftState;
	int             m_shiftStateBitCount[MAX_SHIFT_BITS];

	CommandVector * m_cmds;

	bool            m_ownsCmds;

	std::string     m_baseFilename;
	std::string     m_customFilename;
	std::string     m_commandFilename;

	StringVector *  m_commandCategories;

private:

	bool            load(Iff & iff, bool allowFail);
	void            load_0006(Iff & iff);

	void            processButtonPress(const Map::Button &mapButton, InputType type, int value);
	void            processButtonRelease(InputType type, int value);

	void            processJoystickPovHat(int joystickIndex, int hatNumber, real direction);

	void            addShiftBits(uint bits);
	void            removeShiftBits(uint bits);
	void            activateNewShiftState(void);
	void            expandSparseMap (const SparseMap &sparseMap);

	void            setFilename (const char * fileName, const char * prependPath);

private:

	InputMap ();
	// disable these routines
	InputMap &operator =(const InputMap &);
	InputMap(const InputMap &);

public:

	                      InputMap (const char *fileName, const char * prependPath, MessageQueue *newMessageQueue);
	                     ~InputMap();

	void                  destruct ();

	bool                  write (const char * filename, bool forceBase) const;
	void                  write (Iff & iff) const;

	void                  setBaseFileName (const char * const filename);
	bool                  reload          (bool forceBase = true);

	void                  setKeyboard(int newKeyboardNumber);
	void                  setMouse(int newMouseNumber);
	void                  setJoystick(int joystickIndex, int newJoystickNumber);

	void                  handleInputReset(void);

	const MessageQueue *  getMessageQueue(void) const;
	MessageQueue       *  getMessageQueue(void);
	void                  setMessageQueue(MessageQueue *newMessageQueue);

	void                  beginFrame     ();
	void                  processEvent   (const IoEvent *event);
	void                  handleRepeats  (float elapsedTime);

	uint32                getNumCommands () const;
	const CommandVector & getCommands    () const;

	uint32                getCommandBindings (CommandBindInfoSet *& cmdBindInfoSets, const Command * cmd = 0) const;

	void                  removeBindings (const Command * cmd);

	const Shifts &        getShifts () const;

	bool                  addBinding (const BindInfo & binfo, const Command * cmd);

	const Command *       getCommandByBinding (const BindInfo & binfo) const;

	BindInfo              getBindInfo () const;

	InputMap *            getRebindingMap () const;
	void                  applyRebindingMapChanges (const InputMap & rhs);

	uint32                getShiftState         () const;

	const Command *       findCommandByName     (const char * name, bool optional = false) const;
	const Command *       findCommandByString   (const char * name, bool optional = false) const;

	bool                  executeCommandByName  (const char * name, const float * overrideValuePress = 0, const float * overrideValueRepeat = 0, const float * overrideValueRelease = 0);

	const StringVector &  getCommandCategories  () const;
	void                  getCommandsByCategory (const std::string & category, CommandVector & cv);

	bool                  addCustomCommand      (const std::string & name, int message, const std::string & str, bool canModify);
	bool                  addCustomCommand      (const Command & cmd, bool canModify);
	bool                  removeCustomCommand   (const std::string & name);
};

// ----------------------------------------------------------------------
/**
 * Get the MessageQueue used by this InputMap.
 * 
 * @return The MessageQueue that this InputMap writes into
 */

inline const MessageQueue *InputMap::getMessageQueue(void) const
{
	return m_messageQueue;
}

// ----------------------------------------------------------------------
/**
 * Get the MessageQueue used by this InputMap.
 * 
 * @return The MessageQueue that this InputMap writes into
 */

inline MessageQueue *InputMap::getMessageQueue(void)
{
	return m_messageQueue;
}

// ----------------------------------------------------------------------
/**
 * Change the MessageQueue that this InputMap write into.
 * 
 * The old MessageQueue will not be deallocated.
 */

inline void InputMap::setMessageQueue(MessageQueue *newMessageQueue)
{
	m_messageQueue = newMessageQueue;
}

//-----------------------------------------------------------------

inline const InputMap::CommandVector & InputMap::getCommands () const
{
	return *NON_NULL (m_cmds);
}

//-----------------------------------------------------------------

inline const InputMap::Shifts & InputMap::getShifts () const
{
	NOT_NULL (m_shifts);
	return *m_shifts;
}

//-----------------------------------------------------------------

inline uint32 InputMap::getShiftState () const
{
	return m_shiftState;
}

//----------------------------------------------------------------------

inline InputMap::BindInfo::BindInfo() : 
shiftState (0),
type (IT_None),
value (0)
{
}

//----------------------------------------------------------------------

inline InputMap::BindInfo::BindInfo (uint32 theShiftState, InputType theType, int32 theValue) :
shiftState (theShiftState),
type (theType),
value (theValue)
{
}

// ======================================================================

#endif
