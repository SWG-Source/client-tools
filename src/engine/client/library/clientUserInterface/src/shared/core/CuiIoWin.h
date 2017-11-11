//--
//-- CuiIoWin.h
//-- jwatson 2001-02-11
//-- copyright 2001 Sony Online Entertainment
//--


#ifndef CuiIoWin_H
#define CuiIoWin_H

#include "sharedIoWin/IoWin.h"
#include "UITypes.h"

class Callback;
class InputMap;
class MessageQueue;
class MouseCursor;
class Timer;
struct UIMessage;

namespace MessageDispatch
{
	class Emitter;
}

//-----------------------------------------------------------------

/**
* CuiIoWin is the IoWin responsible for getting all IoEvents for the UI
* and drawing the UI.  As a result, it should always be at the top of
* the IoWin stack.  This placement is enforced via the CuiManager
* 
* @author jwatson
* @date 2001-02-11
* @see nothing
*/

//-----------------------------------------------------------------

class CuiIoWin : public IoWin
{
public:

	struct Messages
	{
		static const char * const MOUSE_HIT_EDGE;
		static const char * const POINTER_INPUT_TOGGLED;
		static const char * const KEYBOARD_INPUT_TOGGLED;
		static const char * const CONTROL_KEY_DOWN;
		static const char * const CONTROL_KEY_UP;
	};

	enum MouseLookState
	{
		MouseLookState_Disabled,
		MouseLookState_Camera,
		MouseLookState_Avatar,
		MouseLookState_Move
	};

	void                  draw                    () const;
	IoResult              processEvent            (IoEvent * event);
	                      CuiIoWin                ();
	                      ~CuiIoWin               ();
	void                  setPointerInputActive   (const bool b);
	void                  setKeyboardInputActive  (const bool b);
	void                  setInputToggleActive    (const bool b);
	bool                  getPointerInputActive   () const;
	bool                  getKeyboardInputActive  () const;
	bool                  getInputToggleActive    () const;

	void                  getCursorLocation       (UIPoint & pt) const;
	void                  warpCursor              (int x, int y);

	static int            getEscapeKeyCode        ();
	static int            getEnterKeyCode         ();

	static bool           isAlphaNumeric          (int code);
	static bool           isChatFodder            (int code);

	static int            getDeadZoneSize         ();
	static void           setDeadZoneSize         (int xy);
	static int getDeadZoneSizeUsable();

	void                  flashDeadZoneInertia    () const;

	bool                  parseMessage            (const int message, const real value, UIMessage & msg);

	void                  discardCurrentMessage   ();

	bool                  getProcessChatInput     () const;
	void                  setProcessChatInput     (bool b);

	MessageDispatch::Emitter & getEmitter         () const;

	bool                  getRadialMenuActiveHack () const;
	void                  setRadialMenuActiveHack (bool b);

	bool                  getPointerToggledOn     () const;

	void                  requestPointer          (bool b);
	void                  requestKeyboard         (bool b);

	/**
	* Don't use this right now.
	*/
	void                  setPointerToggledOn     (bool b);

	const UIRect &        getDeadZone             () const;
	void                  setDeadZone             (const UISize & size);
	void                  resetDeadZone           ();

	void				  getScreenCenter         (UIPoint & screenCenter) const;
	void                  setScreenCenter         (const UIPoint & size, bool keepMouseOffset);
	void                  resetScreenCenter       ();
	void getScreenCenterDefault(UIPoint & screenCenter) const;
	void getScreenCenterOffset(UIPoint & screenCenter) const;

	typedef void (*ModeCallback) (void* context);
	void          setModeCallback (ModeCallback modeCallback, void* context);

	Callback &            getCallbackConsole         ();

	static float          getDoubleClickTimeoutSecs  ();

	void                  resetInputMaps             ();
	static MouseLookState getMouseLookState          ();
	static void           setMouseLookState          ( MouseLookState state );
	static MouseLookState getLastMouseLookState      ();

	void                  updateMouseLookState       ( int eventType, const UIMessage &msg );

	static bool           shouldHoldTarget           (const UIPoint & pt);

private:
	//-- explicitly disable copy constructor and assignment operators
	                      CuiIoWin  (const CuiIoWin &);
	CuiIoWin &            operator = (const CuiIoWin &);

private:

	bool                  m_pointerInputActive;
	bool                  m_keyboardInputActive;
	bool                  m_inputToggleActive;
	MouseCursor *         m_mouseCursor;
	MessageQueue *        m_messageQueue;
	InputMap    *         m_inputMap;
	Timer *               m_timer;
	bool                  m_discardCurrentMessage;
	bool				  m_ignoreNextNumPadValue;

	typedef stdmap<int, float>::fwd KeyRepeatTimer;
	KeyRepeatTimer *      m_keyRepeatTimer;

	static int            ms_escapeKeyCode;
	static int            ms_enterKeyCode;

	bool                  m_processChatInput;

	int                   m_pointerToggledCount;
	int                   m_keyboardRequestCount;

	bool                  m_radialMenuActiveHack;

	MessageDispatch::Emitter * m_emitter;

	UIRect                m_deadZone;

	UIPoint				  m_screenCenter;

	ModeCallback          m_modeCallback;
	void*                 m_context;
	bool                  m_currentMode;

	Callback *            m_callbackConsole;

	static float          ms_doubleClickTimeoutSecs;
	uint32 m_previousShiftState;

	static MouseLookState ms_mouseLookState;
	static MouseLookState ms_lastMouseLookState;
};

//-----------------------------------------------------------------

inline void CuiIoWin::setInputToggleActive (const bool b)
{
	m_inputToggleActive = b;
}

//-----------------------------------------------------------------

inline bool CuiIoWin::getPointerInputActive () const
{
	return m_pointerInputActive;
}

//-----------------------------------------------------------------

inline bool CuiIoWin::getKeyboardInputActive () const
{
	return m_keyboardInputActive;
}

//-----------------------------------------------------------------

inline bool CuiIoWin::getInputToggleActive () const
{
	return m_inputToggleActive;
}

//-----------------------------------------------------------------

inline int CuiIoWin::getEscapeKeyCode ()
{
	return ms_escapeKeyCode;
}

//-----------------------------------------------------------------

inline int CuiIoWin::getEnterKeyCode ()
{
	return ms_enterKeyCode;
}

//-----------------------------------------------------------------

inline bool  CuiIoWin::getProcessChatInput () const
{
	return m_processChatInput;
}

//-----------------------------------------------------------------

inline void  CuiIoWin::setProcessChatInput (bool b)
{
	m_processChatInput = b;
}

//-----------------------------------------------------------------

inline bool CuiIoWin::getPointerToggledOn () const
{
	return m_pointerToggledCount > 0;
}

//----------------------------------------------------------------------

inline MessageDispatch::Emitter & CuiIoWin::getEmitter () const
{
	NOT_NULL (m_emitter);
	return *m_emitter;
}

//-----------------------------------------------------------------

inline bool CuiIoWin::getRadialMenuActiveHack () const
{
	return m_radialMenuActiveHack;
}

//----------------------------------------------------------------------

inline void CuiIoWin::setRadialMenuActiveHack (bool b)
{
	m_radialMenuActiveHack = b;
}

//-----------------------------------------------------------------

inline const UIRect &        CuiIoWin::getDeadZone () const
{
	return m_deadZone;
}

//----------------------------------------------------------------------

inline float CuiIoWin::getDoubleClickTimeoutSecs ()
{
	return ms_doubleClickTimeoutSecs;
}

//----------------------------------------------------------------------

inline CuiIoWin::MouseLookState CuiIoWin::getMouseLookState()
{
	return ms_mouseLookState;
}

//----------------------------------------------------------------------

inline CuiIoWin::MouseLookState CuiIoWin::getLastMouseLookState()
{
	return ms_lastMouseLookState;
}
//----------------------------------------------------------------------

#endif

