//--
//-- CuiIoWin.cpp
//-- jwatson 2001-02-11
//-- copyright 2001 Sony Online Entertainment
//--
#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiIoWin.h"

#include "UIManager.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "clientGame/AwayFromKeyBoardManager.h"
#include "clientGame/Bloom.h"
#include "clientGame/Game.h"
#include "clientGame/GroundScene.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/PostProcessingEffectsManager.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/SwgVideoCapture.h"
#include "clientObject/MouseCursor.h"
#include "clientUserInterface/ConfigClientUserInterface.h"
#include "clientUserInterface/CuiActionManager.h"
#include "clientUserInterface/CuiActions.h"
#include "clientUserInterface/CuiConversationManager.h"
#include "clientUserInterface/CuiInputMessage.def"
#include "clientUserInterface/CuiLoadingManager.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMediator.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiMediatorTypes.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiTextManager.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/MessageQueue.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/Timer.h"
#include "sharedInputMap/InputMap.h"
#include "sharedInputMap/InputMap_Command.h"
#include "sharedMessageDispatch/Emitter.h"
#include "sharedMessageDispatch/Message.h"
#include "sharedUtility/Callback.h"
#include "sharedUtility/LocalMachineOptionManager.h"

#include <dinput.h>
#include <map>

//-----------------------------------------------------------------

namespace CuiIoWinNamespace
{
	UIPoint s_lastMousePoint;
	
	int getFirstKeycodeForCommand (InputMap const & inputMap, const std::string & command, const char * name)
	{
		NOT_NULL (name);
		
		int retval = 0;
		
		InputMap::CommandBindInfoSet * cmdBindInfoSets;
		const InputMap::Command * const cmd = NON_NULL (inputMap.findCommandByName (command.c_str ()));
		
		if (cmd && inputMap.getCommandBindings (cmdBindInfoSets, cmd) && cmdBindInfoSets)
		{
			const InputMap::BindInfo & bi = cmdBindInfoSets [0].binds [0];
			retval = bi.value;
			delete[] cmdBindInfoSets;
		}
		else
			DEBUG_FATAL (true, ("no ui cmd bind info sets for %s.\n", name));

		return retval;
	}

	int        ms_reticleDeadZoneSize = 101;
	int        ms_reticleDeadZoneSizeUsable = ms_reticleDeadZoneSize;
	int        ms_minimumSpaceDeadZone = 101;

	float      ms_flashDeadZoneTime   = 2.0f;
	bool       s_installed            = false;
	VectorArgb ms_flashDeadZoneColor;

	bool       s_discardNextKeyDown = false;

	inline bool isNavigationCode (int code)
	{
		return 
			code == DIK_HOME  ||
			code == DIK_UP    ||
			code == DIK_PRIOR ||
			code == DIK_LEFT  ||
			code == DIK_RIGHT ||
			code == DIK_END   ||
			code == DIK_DOWN  ||
			code == DIK_NEXT;
	}

	bool s_debugShiftStates = false;

	float s_lastClickTimes [3] = { 0.0f, 0.0f, 0.0f };
	UIPoint m_lastClickPoint;
	const UIPoint m_doubleClickMoveThreshold = UIPoint (4L, 4L);
}
using namespace CuiIoWinNamespace;

//-----------------------------------------------------------------

int CuiIoWin::ms_escapeKeyCode;
int CuiIoWin::ms_enterKeyCode;
const char * const CuiIoWin::Messages::MOUSE_HIT_EDGE         = "CuiIoWin::Messages::MOUSE_HIT_EDGE";
const char * const CuiIoWin::Messages::POINTER_INPUT_TOGGLED  = "CuiIoWin::Messages::POINTER_INPUT_TOGGLED";
const char * const CuiIoWin::Messages::KEYBOARD_INPUT_TOGGLED = "CuiIoWin::Messages::KEYBOARD_INPUT_TOGGLED";
const char * const CuiIoWin::Messages::CONTROL_KEY_DOWN = "CuiIoWin::Messages::CONTROL_KEY_DOWN";
const char * const CuiIoWin::Messages::CONTROL_KEY_UP = "CuiIoWin::Messages::CONTROL_KEY_UP";

float CuiIoWin::ms_doubleClickTimeoutSecs = 0.30f;

CuiIoWin::MouseLookState CuiIoWin::ms_mouseLookState     = MouseLookState_Disabled;
CuiIoWin::MouseLookState CuiIoWin::ms_lastMouseLookState = MouseLookState_Disabled;

//-----------------------------------------------------------------

CuiIoWin::CuiIoWin () :
IoWin                     ("CuiIoWin"),
m_pointerInputActive      (false),
m_keyboardInputActive     (false),
m_inputToggleActive       (false),
m_mouseCursor             (new MouseCursor(0, MouseCursor::S_system)),
m_messageQueue            (new MessageQueue (10)),
m_inputMap                (0),
m_timer                   (new Timer),
m_discardCurrentMessage   (false),
m_keyRepeatTimer          (new KeyRepeatTimer),
m_processChatInput        (false),
m_pointerToggledCount     (0),
m_keyboardRequestCount    (0),
m_radialMenuActiveHack    (false),
m_emitter                 (new MessageDispatch::Emitter),
m_deadZone(),
m_screenCenter(),
m_modeCallback            (0),
m_context                 (0),
m_currentMode             (false),
m_callbackConsole         (new Callback),
m_previousShiftState(0),
m_ignoreNextNumPadValue         (false)
{

	if (!s_installed)
	{
		int dummy = 0;
		int xyDeadzoneSize = 0;
		ms_reticleDeadZoneSize = 0;
		ConfigClientUserInterface::getReticleDeadZoneSize (xyDeadzoneSize, dummy);
		setDeadZoneSize(xyDeadzoneSize);
		const char * const section = "ClientUserInterface";

		LocalMachineOptionManager::registerOption (ms_reticleDeadZoneSize, section, "reticleDeadZoneSize", 1);
		DebugFlags::registerFlag (s_debugShiftStates, "ClientUserInterface", "debugShiftStates");

		s_installed = true;
	}

	m_inputMap = new InputMap ("input/ui_inputmap.iff", 0, m_messageQueue);

	ms_enterKeyCode  = getFirstKeycodeForCommand (*m_inputMap, "CuiM_Enter",  "ENTER");
	ms_escapeKeyCode = getFirstKeycodeForCommand (*m_inputMap, "CuiM_Escape", "ESCAPE");

	const UIPage * const page = NON_NULL (UIManager::gUIManager ().GetRootPage ());
	warpCursor (page->GetWidth () / 2, page->GetHeight () / 2);

	resetDeadZone ();
	resetScreenCenter ();

	m_callbackConsole->fetch ();
}

//-----------------------------------------------------------------

CuiIoWin::~CuiIoWin ()
{
	m_callbackConsole->release ();
	delete m_mouseCursor;
	delete m_inputMap;
	delete m_messageQueue;
	delete m_timer;
	delete m_keyRepeatTimer;
	delete m_emitter;

	m_callbackConsole = 0;
	m_keyRepeatTimer  = 0;
	m_timer           = 0;
	m_mouseCursor     = 0;
	m_inputMap        = 0;
	m_messageQueue    = 0;
	m_emitter         = 0;
}

//-----------------------------------------------------------------

void CuiIoWin::draw () const
{
	PostProcessingEffectsManager::preSceneRender();
	Bloom::preSceneRender();

	IoWin::draw ();

#if PRODUCTION == 0
	VideoCapture::SingleUse::run();
#endif // PRODUCTION

	Bloom::postSceneRender();
	PostProcessingEffectsManager::postSceneRender();

	const GroundScene * const gs = dynamic_cast<const GroundScene *>(Game::getScene ());
	if (gs)
		gs->drawOverlays();

	{
		PROFILER_AUTO_BLOCK_DEFINE ("CuiManager::render (ui)");
		CuiManager::render ();
	}

	if (ms_flashDeadZoneTime > 0.0f)
	{
		if (Game::getScene () && !CuiLoadingManager::getFullscreenLoadingEnabled ())
		{
			ms_flashDeadZoneColor.a = std::min (1.0f, ms_flashDeadZoneTime);
			
			Graphics::setStaticShader (ShaderTemplateList::get2dVertexColorAStaticShader ());
			Graphics::drawRectangle (static_cast<int>(m_deadZone.left), static_cast<int>(m_deadZone.top), static_cast<int>(m_deadZone.right), static_cast<int>(m_deadZone.bottom), ms_flashDeadZoneColor);
		}
		else
			ms_flashDeadZoneTime = 0.0f;
	}
}

//-----------------------------------------------------------------

namespace
{
	//-- ui messages that always get processed by the UI, no matter the input state
	inline bool isUiUniversalInput (int message)
	{
		return 
			message == CuiM_Escape ||
			message == CuiM_PageUp ||
			message == CuiM_PageDown;
	}

	inline bool isChatInput (int message)
	{
		return isUiUniversalInput (message);
	}

	/*
	inline bool isChatDik (int code)
	{
		return (code == DIK_RETURN || code == DIK_BACK || code == DIK_DELETE);
	}
	*/
}

//----------------------------------------------------------------------

bool CuiIoWin::isAlphaNumeric (int code)
{
	return
		(code >= DIK_1 && code <= DIK_CAPITAL &&
		code != DIK_TAB &&
		code != DIK_GRAVE &&
		code != DIK_LSHIFT &&
		code != DIK_RSHIFT &&
		code != DIK_LCONTROL &&
		code != DIK_RCONTROL &&
		code != DIK_LMENU &&
		code != DIK_RMENU);
}

//-----------------------------------------------------------------

bool CuiIoWin::isChatFodder (int code)
{
	return isAlphaNumeric (code);
}

//-----------------------------------------------------------------

bool CuiIoWin::parseMessage (const int message, const real value, UIMessage & msg)
{
	AwayFromKeyBoardManager::touch();

	//-----------------------------------------------------------------
	//-- todo: make UIMessage::KeyRepeat work right

	msg.Type = (value < 0.0f ? UIMessage::KeyDown : (value > 0.0f ? UIMessage::KeyUp : UIMessage::KeyRepeat));

	NOT_NULL (m_timer);
	NOT_NULL (m_keyRepeatTimer);

	if (msg.Type == UIMessage::KeyDown)
		(*m_keyRepeatTimer) [message] = m_timer->getExpirationOvershoot ();
	else if (msg.Type == UIMessage::KeyRepeat)
	{
		if (((*m_keyRepeatTimer) [message] + 0.5f) > m_timer->getExpirationOvershoot ())
			return false;
	}

	const float curTime = Game::getElapsedTime ();

	switch (message)
	{
	case CuiM_BackSpace:
		msg.Keystroke = UIMessage::BackSpace;
		break;
	case CuiM_Insert:
		msg.Keystroke = UIMessage::Insert;
		break;
	case CuiM_Delete:
		msg.Keystroke = UIMessage::Delete;
		break;
	case CuiM_LeftArrow:
		msg.Keystroke = UIMessage::LeftArrow;
		break;
	case CuiM_RightArrow:
		msg.Keystroke = UIMessage::RightArrow;
		break;
	case CuiM_UpArrow:
		msg.Keystroke = UIMessage::UpArrow;
		break;
	case CuiM_DownArrow:
		msg.Keystroke = UIMessage::DownArrow;
		break;
	case CuiM_Home:
		msg.Keystroke = UIMessage::Home;
		break;
	case CuiM_End:
		msg.Keystroke = UIMessage::End;
		break;
	case CuiM_PageUp:
		msg.Keystroke = UIMessage::PageUp;
		break;
	case CuiM_PageDown:
		msg.Keystroke = UIMessage::PageDown;
		break;
	case CuiM_Tab:
		msg.Keystroke = UIMessage::Tab;
		break;
	case CuiM_Space:
		msg.Keystroke = UIMessage::Space;
		break;
	case CuiM_Enter:
		msg.Keystroke = UIMessage::Enter;
		break;
	case CuiM_Escape:
		msg.Keystroke = UIMessage::Escape;
		break;
	case CuiM_Copy:
		msg.Keystroke = UIMessage::Copy;
		break;
	case CuiM_Cut:
		msg.Keystroke = UIMessage::Cut;
		break;
	case CuiM_Paste:
		msg.Keystroke = UIMessage::Paste;
		break;
	case CuiM_Undo:
		msg.Keystroke = UIMessage::Undo;
		break;
	case CuiM_Help:
		break;

	case CuiM_MouseLeft:
		if (msg.Type == UIMessage::KeyUp)
		{
			msg.Type = UIMessage::LeftMouseUp;
		}
		else if (msg.Type == UIMessage::KeyDown)
		{
			msg.Type = UIMessage::LeftMouseDown;

			//-- nonzero mousebutton up message Data member indicates that this is a doubleclick
			if ((curTime - s_lastClickTimes [0] <= ms_doubleClickTimeoutSecs))
			{
				const UIPoint deltaMousePoint (abs (m_lastClickPoint.x - msg.MouseCoords.x), abs (m_lastClickPoint.y - msg.MouseCoords.y));
				if (deltaMousePoint.x <= m_doubleClickMoveThreshold.x &&
					deltaMousePoint.y <= m_doubleClickMoveThreshold.y)
					msg.Data = 1;
			}

			s_lastClickTimes [0] = msg.Data ? 0 : curTime;
			m_lastClickPoint = msg.MouseCoords;
		}
		else
			msg.Type = UIMessage::KeyFirst;
		break;
	case CuiM_MouseMiddle:
		if (msg.Type == UIMessage::KeyUp)
		{
			msg.Type = UIMessage::MiddleMouseUp;
		}
		else if (msg.Type == UIMessage::KeyDown)
		{
			msg.Type = UIMessage::MiddleMouseDown;

			//-- nonzero mousebutton up message Data member indicates that this is a doubleclick
			if ((curTime - s_lastClickTimes [2]) <= ms_doubleClickTimeoutSecs)
			{
				const UIPoint deltaMousePoint (abs (m_lastClickPoint.x - msg.MouseCoords.x), abs (m_lastClickPoint.y - msg.MouseCoords.y));
				if (deltaMousePoint.x <= m_doubleClickMoveThreshold.x &&
					deltaMousePoint.y <= m_doubleClickMoveThreshold.y)
					msg.Data = 1;
			}

			s_lastClickTimes [2] = msg.Data ? 0 : curTime;
			m_lastClickPoint = msg.MouseCoords;
		}
		else
			return false;
		break;
	case CuiM_MouseRight:
		if (msg.Type == UIMessage::KeyUp)
		{
			msg.Type = UIMessage::RightMouseUp;
		}
		else if (msg.Type == UIMessage::KeyDown)
		{
			msg.Type = UIMessage::RightMouseDown;

			//-- nonzero mousebutton up message Data member indicates that this is a doubleclick
			if ((curTime - s_lastClickTimes [1]) <= ms_doubleClickTimeoutSecs)
			{
				const UIPoint deltaMousePoint (abs (m_lastClickPoint.x - msg.MouseCoords.x), abs (m_lastClickPoint.y - msg.MouseCoords.y));
				if (deltaMousePoint.x <= m_doubleClickMoveThreshold.x &&
					deltaMousePoint.y <= m_doubleClickMoveThreshold.y)
					msg.Data = 1;
			}

			s_lastClickTimes [1] = msg.Data ? 0 :curTime;
			m_lastClickPoint = msg.MouseCoords;
		}
		else
			return false;
		break;
	case CuiM_Mouse4:
	case CuiM_Mouse5:
	case CuiM_Mouse6:
	case CuiM_Mouse7:
	case CuiM_Mouse8:
	case CuiM_MouseWheelUp:
	case CuiM_MouseWheelDown:
		msg.Type = UIMessage::KeyFirst;
		break;
	case CuiM_Console:
		m_callbackConsole->performCallback ();
		return false;

	case CuiM_JoyAxisX:
		
		if (!m_pointerInputActive || !ConfigClientUserInterface::getJoystickMovesPointer ())
			return false;
		
		//-- todo: configurable joystick sensitivity		
		msg.MouseCoords.x += static_cast<long>(value * 10.0f);
		m_mouseCursor->gotoXY (msg.MouseCoords.x, msg.MouseCoords.y);
		msg.Type = UIMessage::MouseMove;
		
		break;
		
	case CuiM_JoyAxisY:
		
		if (!m_pointerInputActive || !ConfigClientUserInterface::getJoystickMovesPointer ())
			return false;
		
		//-- todo: configurable joystick sensitivity
		msg.MouseCoords.y += static_cast<long>(value * 10.0f);
		m_mouseCursor->gotoXY (msg.MouseCoords.x, msg.MouseCoords.y);
		msg.Type = UIMessage::MouseMove;

		break;

	default:
		return false;
	}
	
	if (!m_keyboardInputActive && msg.IsKeyMessage ())
	{
		//----------------------------------------------------------------------
		//-- arrow keys
		
		bool isTextMessage = msg.Type == UIMessage::Character;
		
		if (!isTextMessage)
		{
			if (msg.Type == UIMessage::KeyUp || msg.Type == UIMessage::KeyDown || msg.Type == UIMessage::KeyRepeat)
			{
				const uint16 k = msg.Keystroke;
				
				if (
					k == UIMessage::DownArrow ||
					k == UIMessage::UpArrow ||
					k == UIMessage::LeftArrow ||
					k == UIMessage::RightArrow ||
					k == UIMessage::Home ||
					k == UIMessage::End ||
					k == UIMessage::Enter ||
					k == UIMessage::Tab ||
					k == UIMessage::BackSpace ||
					k == UIMessage::Delete
					)
					isTextMessage = true;
			}
		}
		
		if (isTextMessage)
		{
			const UIWidget * const focused = UIManager::gUIManager ().GetFocusedLeafWidget ();
			if (focused && (focused->IsA (TUITextbox) || focused->IsA (TUIText) || focused->IsA (TUIComboBox) || focused->IsA (TUIList) || focused->IsA (TUITable) || focused->IsA(TUIWebBrowser)))
			{
				Unicode::String dummy;
				static const UILowerString prop_cuiIoWinNoArrow ("CuiIoWinNoArrow");
				if (CuiPreferences::getModalChat () || !focused->GetProperty (prop_cuiIoWinNoArrow, dummy))
					return true;
			}
		}

		if (isUiUniversalInput (message))
			return true;

		if (m_processChatInput && (message == CuiM_BackSpace || message == CuiM_Delete) && (!msg.Modifiers.isShiftDown () && !msg.Modifiers.isControlDown () && !msg.Modifiers.isAltDown ()))
			return true;

		if (!CuiPreferences::getModalChat () && (message == CuiM_Enter) && (!msg.Modifiers.isShiftDown () && !msg.Modifiers.isControlDown () && !msg.Modifiers.isAltDown ()))
			return true;

		//-- radial menus want character input
		if (m_radialMenuActiveHack)
			return true;

		if (!m_processChatInput || !isChatInput (message))
			return false;
	}

	if (!m_pointerInputActive && msg.IsMouseMessage () && !msg.IsMouseButtonMessage ())
		return false;

	return true;
}

//-----------------------------------------------------------------

IoResult CuiIoWin::processEvent (IoEvent * event)
{
	NOT_NULL (m_mouseCursor);
	NOT_NULL (m_inputMap);
	NOT_NULL (m_messageQueue);

	m_mouseCursor->processEvent (event);

	bool retval = false;

	UIManager * const uiManager = &UIManager::gUIManager ();

	//-----------------------------------------------------------------
	//-- setup the UIMessage

	UIMessage msg;
	msg.Data = 0;

	msg.MouseCoords.x = m_mouseCursor->getX ();
	msg.MouseCoords.y = m_mouseCursor->getY ();

	//-----------------------------------------------------------------
	//-- process the event to obtain any new shifting information

	m_inputMap->processEvent (event);
	m_inputMap->beginFrame ();

	const uint32 shiftState = m_inputMap->getShiftState ();

	msg.Modifiers.LeftAlt     = msg.Modifiers.RightAlt     = (shiftState & CuiM_BITS_MENU)  != 0;
	msg.Modifiers.LeftControl = msg.Modifiers.RightControl = (shiftState & CuiM_BITS_CTRL)  != 0;
	msg.Modifiers.LeftShift   = msg.Modifiers.RightShift   = (shiftState & CuiM_BITS_SHIFT) != 0;

	msg.Modifiers.LeftMouseDown   = (shiftState & CuiM_BITS_MOUSE_L) != 0;
	msg.Modifiers.RightMouseDown  = (shiftState & CuiM_BITS_MOUSE_R) != 0;
	msg.Modifiers.MiddleMouseDown = (shiftState & CuiM_BITS_MOUSE_M) != 0;

	if (shiftState != m_previousShiftState)
	{
		if ((shiftState & CuiM_BITS_CTRL) != (m_previousShiftState & CuiM_BITS_CTRL))
		{
			m_emitter->emitMessage(MessageDispatch::MessageBase(((shiftState & CuiM_BITS_CTRL) != 0)
					? Messages::CONTROL_KEY_DOWN
					: Messages::CONTROL_KEY_UP));
		}
	}

	if (s_debugShiftStates && event->type == IOET_Update)
	{
		InputMap * const gameInputMap = Game::getGameInputMap ();
		
		if (gameInputMap)
		{
			const uint32 gameShiftState = gameInputMap->getShiftState ();
			
			UIMessage gameMsg;
			
			gameMsg.Modifiers.LeftAlt     = gameMsg.Modifiers.RightAlt     = (gameShiftState & CuiM_BITS_MENU)  != 0;
			gameMsg.Modifiers.LeftControl = gameMsg.Modifiers.RightControl = (gameShiftState & CuiM_BITS_CTRL)  != 0;
			gameMsg.Modifiers.LeftShift   = gameMsg.Modifiers.RightShift   = (gameShiftState & CuiM_BITS_SHIFT) != 0;
			
			gameMsg.Modifiers.LeftMouseDown   = (gameShiftState & CuiM_BITS_MOUSE_L) != 0;
			gameMsg.Modifiers.RightMouseDown  = (gameShiftState & CuiM_BITS_MOUSE_R) != 0;
			gameMsg.Modifiers.MiddleMouseDown = (gameShiftState & CuiM_BITS_MOUSE_M) != 0;
			
			char buf [256];
			const size_t buf_size = sizeof (buf);
			
			snprintf (buf, buf_size, 
				"%sALT %sCTL %sSHF \\#ffffff-- %sLMB %sMMB %sRMB\n"
				"%sALT %sCTL %sSHF \\#ffffff-- %sLMB %sMMB %sRMB",
				msg.Modifiers.isAltDown ()         ? "\\#ff0000" : "\\#aaaaaa",
				msg.Modifiers.isControlDown ()     ? "\\#ff0000" : "\\#aaaaaa",
				msg.Modifiers.isShiftDown ()       ? "\\#ff0000" : "\\#aaaaaa",
				msg.Modifiers.LeftMouseDown        ? "\\#ff0000" : "\\#aaaaaa",
				msg.Modifiers.MiddleMouseDown      ? "\\#ff0000" : "\\#aaaaaa",
				msg.Modifiers.RightMouseDown       ? "\\#ff0000" : "\\#aaaaaa",
				gameMsg.Modifiers.isAltDown ()     ? "\\#ff0000" : "\\#ffffff",
				gameMsg.Modifiers.isControlDown () ? "\\#ff0000" : "\\#ffffff",
				gameMsg.Modifiers.isShiftDown ()   ? "\\#ff0000" : "\\#ffffff",
				gameMsg.Modifiers.LeftMouseDown    ? "\\#ff0000" : "\\#ffffff",
				gameMsg.Modifiers.MiddleMouseDown  ? "\\#ff0000" : "\\#ffffff",
				gameMsg.Modifiers.RightMouseDown   ? "\\#ff0000" : "\\#ffffff");
			
			CuiTextManager::TextEnqueueInfo ti;
			ti.screenVect.x = 150.0f;
			ti.screenVect.y = 300.0f;
			
			CuiTextManager::enqueueText (Unicode::narrowToWide (buf), ti);
		}
	}

	bool textIsFocused       = false;
	bool navigationIsFocused = false;
	m_previousShiftState = shiftState;
	
	{
		const UIWidget * const focused = UIManager::gUIManager ().GetFocusedLeafWidget ();
		if (focused)
		{
			textIsFocused       = focused->IsA (TUITextbox) || focused->IsA (TUIText)|| focused->IsA(TUIWebBrowser);
			navigationIsFocused = textIsFocused || focused->IsA (TUIComboBox) || focused->IsA (TUIList) || focused->IsA (TUITable) ;
			
			Unicode::String dummy;
			static const UILowerString prop_cuiIoWinNoArrow ("CuiIoWinNoArrow");
			if (!CuiPreferences::getModalChat () && focused->GetProperty (prop_cuiIoWinNoArrow, dummy))
				textIsFocused = navigationIsFocused = false;
		}
	}
	
	//-----------------------------------------------------------------

	switch (event->type)
	{
	case IOET_MouseMove:
		{		
			UIPoint newCoords;
			//Add a movement multiplier
			newCoords.x = m_mouseCursor->getX ();
			newCoords.y = m_mouseCursor->getY ();
			UIPoint delta (newCoords - UIManager::gUIManager ().GetLastMouseCoord ());						
			if((delta.x != 0) && (delta.y != 0))
			{				
				delta.x = static_cast<long>(delta.x * GroundScene::getMouseSensitivity());
				delta.y = static_cast<long>(delta.y * GroundScene::getMouseSensitivity());
				newCoords.x = UIManager::gUIManager ().GetLastMouseCoord ().x + delta.x;
				newCoords.y = UIManager::gUIManager ().GetLastMouseCoord ().y + delta.y;
				msg.MouseCoords = newCoords;
				m_mouseCursor->gotoXY (msg.MouseCoords.x, msg.MouseCoords.y);					
			}			
		}

		AwayFromKeyBoardManager::touch();

		if (!Game::isHudSceneTypeSpace())
		{
			if ( getMouseLookState() != MouseLookState_Disabled )
			{
				if ( uiManager->GetTopContextWidget() )
				{
					setMouseLookState( MouseLookState_Disabled );
				}
				else
				{
					// If we're panning the camera around, not clicking, we wanna make sure this action is cancelled...
					UIPoint delta;
					delta.Set(m_mouseCursor->getX(), m_mouseCursor->getY());
					delta -= UIManager::gUIManager().GetLastMouseCoord();
					if (delta.Magnitude() > UIManager::gUIManager().GetDragThreshold())
						IGNORE_RETURN(CuiActionManager::performAction (CuiActions::setIntendedAndSummonRadialMenu, Unicode::narrowToWide ("cancel")));

					CuiManager::InputManager::setPointerMotionCapturedByUiX (false);
					CuiManager::InputManager::setPointerMotionCapturedByUiY (false);
					m_mouseCursor->gotoXY (s_lastMousePoint.x, s_lastMousePoint.y);
					return IOR_Pass;
				}
			}
		}

		if (!m_pointerInputActive && uiManager->IsContextRequestPending (false))
		{
			msg.MouseCoords.x = m_mouseCursor->getX ();
			msg.MouseCoords.y = m_mouseCursor->getY ();
			const UIPoint delta (UIManager::gUIManager ().GetLastMouseCoord ()  - msg.MouseCoords);

			if (delta.Magnitude () > UIManager::gUIManager ().GetDragThreshold ())
				uiManager->ForcePendingContextRequest (false);

			return IOR_Block;
		}

		//				if (m_pointerInputActive)
		else
		{
			//-- handle mouse wheel
			if (event->arg2 == 2)
			{
				// @todo this is a windows-specific constant relating to mousewheels
				static const float one_over_120 = RECIP (120.0f);
				msg.Type          = UIMessage::MouseWheel;
				msg.Data          = static_cast<short>(event->arg3 * one_over_120);
				msg.MouseCoords.x = m_mouseCursor->getX ();
				msg.MouseCoords.y = m_mouseCursor->getY ();
				if (uiManager->ProcessMessage (msg))
					return IOR_Block;
				else
					return IOR_Pass;
			}

			//-- normal movement
			else
			{

				const GroundScene * const gs = dynamic_cast<const GroundScene *>(Game::getScene ());
				const bool isFirstPerson = gs ? gs->isFirstPerson () : 0;

				UIRect deadZone;
				
				if (!m_pointerInputActive)
				{
					deadZone = m_deadZone;

					if (isFirstPerson)
					{
						deadZone.left = deadZone.right  = Graphics::getCurrentRenderTargetWidth () / 2L;
						deadZone.top  = deadZone.bottom = Graphics::getCurrentRenderTargetHeight () / 2L;
					}

					else 
					{
						if (GroundScene::getInvertMouseLook ())
						{
							if (event->arg2 == IOMT_TranslateY)
							{
								const long deltaY        = static_cast<int>(event->arg3);
								msg.MouseCoords.y        -= 2 * deltaY;
								m_mouseCursor->gotoXY (msg.MouseCoords.x, msg.MouseCoords.y);
							}
						}
					}
				}
				else
				{
					deadZone.bottom = Graphics::getCurrentRenderTargetHeight () - 2L;
					deadZone.right  = Graphics::getCurrentRenderTargetWidth  () - 2L;
				}

				deadZone.bottom = std::min(deadZone.bottom, Graphics::getCurrentRenderTargetHeight() - 2L);
				deadZone.right = std::min(deadZone.right, Graphics::getCurrentRenderTargetWidth() - 2L);
				deadZone.top = std::max(deadZone.top, 0L);
				deadZone.left = std::max(deadZone.left, 0L);

				msg.Type = UIMessage::MouseMove;

				if (gs && !m_pointerInputActive)
				{
					if (!isFirstPerson)
					{
						msg.MouseCoords.y = std::min (std::max (deadZone.top,  msg.MouseCoords.y), deadZone.bottom);
						msg.MouseCoords.x = std::min (std::max (deadZone.left, msg.MouseCoords.x), deadZone.right);
					}
					else
					{
						msg.MouseCoords.y = Graphics::getCurrentRenderTargetHeight () / 2L;
						msg.MouseCoords.x = Graphics::getCurrentRenderTargetWidth () / 2L;
					}
					
					m_mouseCursor->gotoXY (msg.MouseCoords.x, msg.MouseCoords.y);
				}
				
				IGNORE_RETURN (uiManager->ProcessMessage (msg));
				
				CuiManager::InputManager::setPointerMotionCapturedByUiX (true);
				CuiManager::InputManager::setPointerMotionCapturedByUiY (true);
				
				if ( gs && !CuiPreferences::getUseModelessInterface() )
				{
					if ( !CuiManager::getPointerInputActive () || (CuiPreferences::getPointerModeMouseCameraEnabled () && CuiMediator::getCountPointerInputActive () <= 0) )
					{
						/*
						const UIWidget * const wid = NON_NULL (UIManager::gUIManager ().GetRootPage ())->GetWidgetWithMouseLock ();

						if (!wid)
						*/
						{
							if (
								msg.MouseCoords.x <= deadZone.left  ||
								msg.MouseCoords.x >= deadZone.right)
								CuiManager::InputManager::setPointerMotionCapturedByUiX (false);
							
							if (
								msg.MouseCoords.y <= deadZone.top   ||
								msg.MouseCoords.y >= deadZone.bottom)
								CuiManager::InputManager::setPointerMotionCapturedByUiY (false);
						}
					}
				}

				s_lastMousePoint = msg.MouseCoords;

				if (!CuiManager::getPointerMotionCapturedByUiX () || !CuiManager::getPointerMotionCapturedByUiY ())
				{
					return IOR_Pass;
				}

				if (msg.MouseCoords.x <= 0 || (msg.MouseCoords.x >= Graphics::getCurrentRenderTargetWidth () - 2) ||
					msg.MouseCoords.y <= 0 || (msg.MouseCoords.y >= Graphics::getCurrentRenderTargetHeight () - 2))
				{
					m_emitter->emitMessage (MessageDispatch::MessageBase (Messages::MOUSE_HIT_EDGE));
				}

				return IOR_Block;
			}
		}

		return IOR_Pass;

		//-----------------------------------------------------------------

	case IOET_Character:
		{
			AwayFromKeyBoardManager::touch();

			if (CuiPreferences::getModalChat () && !m_keyboardInputActive && !textIsFocused && !m_radialMenuActiveHack)
				return IOR_Pass;

			if (!CuiPreferences::getModalChat ())
			{
				if (Os::isNumPadChar(static_cast<unsigned char>(event->arg2)))
				{
					m_ignoreNextNumPadValue = true;
					return IOR_Pass;
				}
				else if (Os::isNumPadValue(static_cast<unsigned char>(event->arg2)) && m_ignoreNextNumPadValue)
				{
					m_ignoreNextNumPadValue = false;
					return IOR_Pass;
				}
			}

			m_ignoreNextNumPadValue = false;

			if (m_radialMenuActiveHack && event->arg2 >= '0' && event->arg2 <= '9')
			{
				s_discardNextKeyDown = true;
			}

			//-- TODO: this should support more than just ASCII character input

			if (event->arg2 == '\b' || event->arg2 == '\t' || event->arg2 == '\n')
				return IOR_Block;

			//-- TODO: this is bogus
			if (event->arg2 == '~' || event->arg2 == '`')
				return IOR_Block;

			msg.Type        = UIMessage::Character;
			msg.Keystroke	= static_cast<unsigned short>(event->arg2);

			//-----------------------------------------------------------------
			//-- don't send control characters through this way

			if (msg.Keystroke >= ' ')
			{
				//if nothing accepted this character message, make certain that it goes to chat
				if(!uiManager->ProcessMessage (msg))
				{
					UIWidget* selected = UIManager::gUIManager().GetRootPage()->GetFocusedLeafWidget();
					//unselected the focused widgets so that chat gets the message
					if(!selected)
					{
						UIBaseObject* groundHud = UIManager::gUIManager().GetRootPage()->GetChild("GroundHud");
						UIManager::gUIManager().GetRootPage()->SelectChild(groundHud);
					}
					else
					{
						while(selected && selected != UIManager::gUIManager().GetRootPage())
						{
							selected->SelectChild(NULL);
							selected = dynamic_cast<UIWidget*>(selected->GetParent());
						}
					}
					//reprocess the message now that chat should catch it
					IGNORE_RETURN(uiManager->ProcessMessage (msg));
				}
			}
		}

		return IOR_Block;

	case IOET_Update:

		if (ms_flashDeadZoneTime > 0.0f)
			ms_flashDeadZoneTime -= event->arg3;

		NOT_NULL (m_timer);
		NOT_NULL (m_inputMap);

		IGNORE_RETURN (m_timer->updateNoReset (event->arg3));

		m_inputMap->handleRepeats (event->arg3);
		m_inputMap->beginFrame();

		//-- handle the mode callback
		if (m_modeCallback)
		{
			if (m_currentMode != getPointerToggledOn ())
			{
				m_modeCallback (m_context);

				m_modeCallback = 0;
				m_context = 0;
			}
		}

		break;

	case IOET_WindowOpen:
	case IOET_WindowClose:
	case IOET_WindowKill:
	case IOET_Prepare:
	case IOET_InputReset:
	case IOET_Count:
		return IOR_Pass;

	case IOET_KeyDown:
		AwayFromKeyBoardManager::touch();
		
		if (m_keyboardInputActive)
		{
			InputMap * const gameInputMap = Game::getGameInputMap ();
			if (gameInputMap)
			{
				gameInputMap->handleInputReset ();

				InputMap::BindInfo bi;
				bi.shiftState = shiftState;
				bi.type       = InputMap::IT_Key;
				bi.value      = event->arg2;

				const InputMap::Command * const cmd = gameInputMap->getCommandByBinding (bi);
				if (NULL != cmd)
				{
					//-- these 2 types of command should always work, regardless of input mode
					//-- fixme: this should eventually be data driven
					static std::string const consoleCommandName = "CMD_console";
					static std::string const pointerToggleCommandName = "CMD_uiPointerToggle";

					if (cmd->name == consoleCommandName || cmd->name == pointerToggleCommandName)
						IGNORE_RETURN(gameInputMap->executeCommandByName (cmd->name.c_str ()));
				}
			}
			retval = true;
		}

		if ((textIsFocused && isAlphaNumeric (event->arg2) && !msg.Modifiers.isControlDown ()) || (navigationIsFocused && isNavigationCode (event->arg2)) || s_discardNextKeyDown)
			retval = true;
		
		s_discardNextKeyDown = false;

		//-- no chatting when ctrl or menu are down
//		if (m_processChatInput && (m_inputMap->getShiftState () & (CuiM_BITS_CTRL | CuiM_BITS_MENU)) == 0)) && isChatFodder (event->arg2))
//			retval = true;
		
		break;
		
	case IOET_KeyUp:
		{
			m_ignoreNextNumPadValue = false;

			if (m_keyboardInputActive)
				retval = true;
			
			if ((textIsFocused && isAlphaNumeric (event->arg2) && !msg.Modifiers.isControlDown ()) || (navigationIsFocused && isNavigationCode (event->arg2)))
				retval = true;
		}
		break;

	case IOET_JoystickButtonDown:
	case IOET_JoystickButtonUp:
	case IOET_JoystickPovHat:
	case IOET_JoystickMove:
	case IOET_JoystickSlider:
		break;
	case IOET_MouseButtonUp:
	case IOET_MouseButtonDown:
		break;
	case IOET_SetSystemMouseCursorPosition:
		break;

	case IOET_IMEComposition:
		msg.Type = UIMessage::IMEComposition;
		retval = uiManager->ProcessMessage (msg) || retval;
		break;
	case IOET_IMEChangeCandidate:
		CuiMediatorFactory::activate (CuiMediatorTypes::IMEInput);

		msg.Type = UIMessage::IMEChangeCandidate;
		retval = uiManager->ProcessMessage (msg) || retval;
		break;
	case IOET_IMECloseCandidate:
		CuiMediatorFactory::deactivate (CuiMediatorTypes::IMEInput);

		msg.Type = UIMessage::IMECloseCandidate;
		retval = uiManager->ProcessMessage (msg) || retval;
		break;
	case IOET_IMEEndComposition:
		msg.Type = UIMessage::IMEEndComposition;
		retval = uiManager->ProcessMessage (msg) || retval;
		break;
	}

	bool shouldResetInput = false;

	if (const int num = m_messageQueue->getNumberOfMessages () > 0) //lint !e111 //assignment to const object
	{
		for (int i = 0; i < num; ++i)
		{
			int message;
			real value;

			m_messageQueue->getMessage (i, &message, &value);

			m_discardCurrentMessage = false;

			if (parseMessage (message, value, msg))
			{
				if (m_discardCurrentMessage)
				{
					m_discardCurrentMessage = false;
					continue;
				}

				if (msg.Type != UIMessage::KeyFirst)
				{
					//-- ignore alt-enter or alt-tab

					if ((msg.Type == UIMessage::KeyUp || msg.Type == UIMessage::KeyDown) &&
						(msg.Keystroke == UIMessage::Enter || msg.Keystroke == UIMessage::Tab) &&
						msg.Modifiers.isAltDown ())
					{
						retval = true;
						shouldResetInput = true;
						continue;
					}

					retval = uiManager->ProcessMessage (msg) || retval;

					//-----------------------------------------------------------------
					//-- send a doubleclick message if needed

					if ( ms_mouseLookState != MouseLookState_Avatar && msg.Data )
					{
						if ( msg.Type == UIMessage::LeftMouseDown
							|| msg.Type == UIMessage::RightMouseDown
							|| msg.Type == UIMessage::MiddleMouseDown)
						{

							if (msg.Type == UIMessage::LeftMouseDown)
							{
								msg.Type = UIMessage::LeftMouseDoubleClick;
							}
							else if (msg.Type == UIMessage::MiddleMouseDown)
							{
								msg.Type = UIMessage::MiddleMouseDoubleClick;
							}
							else if	( msg.Type == UIMessage::RightMouseDown )
							{
								msg.Type = UIMessage::RightMouseDoubleClick;
							}

							retval = uiManager->ProcessMessage (msg) || retval;
						}
					}

					if (!retval && msg.Type == UIMessage::KeyDown && msg.Keystroke == UIMessage::Escape)
					{
						retval = CuiConversationManager::stop () || retval;
					}

					if (msg.Type > UIMessage::IMEFirst && msg.Type < UIMessage::IMELast)
					{
						retval = uiManager->ProcessMessage(msg) || retval;
					}
				}
			}
		}
	}

	if (shouldResetInput)
	{
		resetInputMaps ();
	}

	updateMouseLookState( event->type, msg );

	if (event->type == IOET_Update)
	{
		return IOR_Pass;
	}

	if (event->type == IOET_MouseButtonUp)
	{
		// IOET_MouseButtonDown might have actually been generated in
		// another IoWin.  In that case, it's important that IOET_MouseButtonUp
		// be propagated to other IoWins that may have actually
		// generated the IOET_MouseButtonDown
		return IOR_Pass;
	}

	return retval ? IOR_Block : IOR_Pass;

} //lint !e818 //event point to const, moron lint

//-----------------------------------------------------------------

void CuiIoWin::getCursorLocation (UIPoint & pt) const
{
	pt.x = m_mouseCursor->getX ();
	pt.y = m_mouseCursor->getY ();
}

//-----------------------------------------------------------------

void CuiIoWin::warpCursor (int x, int y)
{
	m_mouseCursor->gotoXY (x, y);

	UIMessage msg;
	msg.Type	      = UIMessage::MouseMove;
	Zero (msg.Modifiers);
	msg.MouseCoords.x = m_mouseCursor->getX ();
	msg.MouseCoords.y = m_mouseCursor->getY ();
	IGNORE_RETURN (UIManager::gUIManager ().ProcessMessage (msg));
}

//----------------------------------------------------------------------

void CuiIoWin::setPointerInputActive (const bool b)
{
	bool const overide = CuiPreferences::getUseModelessInterface() || b;

	if (m_pointerInputActive != overide && (overide  || m_pointerToggledCount <= 0)) //lint !e731 bool arg
	{
		m_pointerInputActive = overide;

		UIPoint pt (UIManager::gUIManager ().GetLastMouseCoord ());
		NON_NULL (UIManager::gUIManager ().GetRootPage ())->ReleaseMouseLock (pt);
		UIManager::gUIManager ().AbortDragAndDrop (0);

		if (Game::getScene ())
		{
			//check out our mouse state and update the mouse look state accordingly
			InputMap* inputMap = Game::getGameInputMap();
			if (inputMap)
			{
				uint32 shiftState = inputMap->getShiftState();
				MouseLookState mouseState = MouseLookState_Disabled;

				if (shiftState & CuiM_BITS_MOUSE_R)
				{
					mouseState = MouseLookState_Avatar;
				}

				setMouseLookState (mouseState);
			}

			if (!overide)
			{
				pt.y = std::min (std::max (m_deadZone.top,  pt.y), m_deadZone.bottom);
				pt.x = std::min (std::max (m_deadZone.left, pt.x), m_deadZone.right);

				warpCursor (pt.x, pt.y);
			}
			else
			{
			}
		}

		m_emitter->emitMessage (MessageDispatch::MessageBase (Messages::POINTER_INPUT_TOGGLED));
		IGNORE_RETURN(CuiActionManager::performAction(CuiActions::onPointerToggled, Unicode::String()));
	}
}

//-----------------------------------------------------------------

void CuiIoWin::setKeyboardInputActive (const bool b)
{
	if (m_keyboardInputActive != b) //lint !e731 bool arg
	{
		m_keyboardInputActive = b;
		m_emitter->emitMessage (MessageDispatch::MessageBase (Messages::KEYBOARD_INPUT_TOGGLED));
	}
}

//-----------------------------------------------------------------

/**
* MessageDispatch Receivers that handle messages as a result of and during the IoEvent processing
* of the CuiIoWin may call discardCurrentMessage () to indicate that the message should be discarded.
*/
void   CuiIoWin::discardCurrentMessage ()
{
	m_discardCurrentMessage = true;
}

//-----------------------------------------------------------------

/**
*
* This toggles the CuiIoWin's pointer state.
*
*/

void CuiIoWin::setPointerToggledOn (bool b)
{
	bool wasZero = m_pointerToggledCount == 0;

	if (b)
		++m_pointerToggledCount;
	else
		m_pointerToggledCount = 0;

	CuiManager::InputManager::setPointerInputActive  (b || CuiPreferences::getMouseModeDefault ());

	if (wasZero && m_pointerToggledCount > 1)
		--m_pointerToggledCount;
}

//----------------------------------------------------------------------

void  CuiIoWin::requestKeyboard (bool b)
{
	if (b)
	{
		m_keyboardRequestCount = std::max (1, m_keyboardRequestCount + 1);
		setKeyboardInputActive (true);
	}
	else if (--m_keyboardRequestCount <= 0)
		setKeyboardInputActive (false);
}

//----------------------------------------------------------------------

void  CuiIoWin::requestPointer (bool b)
{
	if (b)
		++m_pointerToggledCount;
	else if (--m_pointerToggledCount <= 0)
		m_pointerToggledCount = 0;

	CuiManager::InputManager::setPointerInputActive  (b || CuiPreferences::getMouseModeDefault ());
}

// ----------------------------------------------------------------------

void CuiIoWin::resetDeadZone ()
{
	if (Game::isHudSceneTypeSpace())
		ms_reticleDeadZoneSizeUsable = std::max(ms_minimumSpaceDeadZone, ms_reticleDeadZoneSize);
	else
		ms_reticleDeadZoneSizeUsable = ms_reticleDeadZoneSize;
	
	const UISize deadZoneSize (ms_reticleDeadZoneSizeUsable, ms_reticleDeadZoneSizeUsable);
	setDeadZone (deadZoneSize);
}

//-----------------------------------------------------------------

/**
* @todo this must respond to screen resolution changes
*/

void CuiIoWin::setDeadZone (const UISize & size)
{
	UIPoint center;

	getScreenCenter(center);

	m_deadZone.Set (center - size / 2, size);
}

//-----------------------------------------------------------------

void CuiIoWin::setModeCallback (ModeCallback modeCallback, void* const context)
{
	m_modeCallback = modeCallback;
	m_context = context;
	m_currentMode = getPointerToggledOn ();
}

//----------------------------------------------------------------------

Callback & CuiIoWin::getCallbackConsole      ()
{
	return *m_callbackConsole;
}

//----------------------------------------------------------------------

void CuiIoWin::resetInputMaps             ()
{
	m_inputMap->handleInputReset ();
	InputMap * const gameInputMap = Game::getGameInputMap ();
	if (gameInputMap)
		gameInputMap->handleInputReset ();

	UIManager::gUIManager().ClearMouseDownControl();
}

//----------------------------------------------------------------------

int CuiIoWin::getDeadZoneSize()
{
	return ms_reticleDeadZoneSize;
}

//----------------------------------------------------------------------

int CuiIoWin::getDeadZoneSizeUsable()
{
	return ms_reticleDeadZoneSizeUsable;
}

//----------------------------------------------------------------------

void CuiIoWin::setDeadZoneSize         (int xy)
{
	if (ms_reticleDeadZoneSize != xy)
	{
		ms_reticleDeadZoneSize = xy;

		//-- even sizes are not allowed
		if ((ms_reticleDeadZoneSize % 2) != 0)
		{
			ms_reticleDeadZoneSize += 1;
		}

		ms_flashDeadZoneTime   = 2.0f;
		ms_flashDeadZoneColor  = VectorArgb::solidGreen;

		if (Game::isHudSceneTypeSpace())
			ms_reticleDeadZoneSizeUsable = std::max(ms_minimumSpaceDeadZone, ms_reticleDeadZoneSize);
		else
			ms_reticleDeadZoneSizeUsable = ms_reticleDeadZoneSize;

		if (CuiManager::getInstalled ())
			CuiManager::getIoWin ().resetDeadZone ();
	}
}

//----------------------------------------------------------------------

void CuiIoWin::flashDeadZoneInertia () const
{
	ms_flashDeadZoneTime   = 2.0f;
	ms_flashDeadZoneColor  = VectorArgb::solidRed;
}

//----------------------------------------------------------------------

void CuiIoWin::getScreenCenter (UIPoint & screenCenter) const
{
	if (m_pointerInputActive || !Game::isViewFirstPerson())
	{
		if (!Game::isHudSceneTypeSpace())
		{
			screenCenter.Set (Graphics::getCurrentRenderTargetWidth() / 2, Graphics::getCurrentRenderTargetHeight() / 2);
			return;
		}
	}

	screenCenter = m_screenCenter;
}

//----------------------------------------------------------------------

void CuiIoWin::setScreenCenter (UIPoint const & screenCenter, bool keepMouseOffset)
{
	if (m_screenCenter != screenCenter)
	{
		UIScalar size = m_deadZone.Width() / 2;
		m_deadZone.Set (screenCenter.x - size, screenCenter.y - size, screenCenter.x + size, screenCenter.y + size);

		if (!m_pointerInputActive && Game::isHudSceneTypeSpace())
		{
			if (keepMouseOffset)
			{
				UIPoint mouseRelativeOffset (m_mouseCursor->getX () - m_screenCenter.x, m_mouseCursor->getY () - m_screenCenter.y);
				warpCursor (screenCenter.x + mouseRelativeOffset.x, screenCenter.y + mouseRelativeOffset.y);
			}
			else
			{
				warpCursor (screenCenter.x, screenCenter.y);
			}
		}

		m_screenCenter = screenCenter;
	}
}

//----------------------------------------------------------------------

void CuiIoWin::resetScreenCenter ()
{
	UIPoint screenCenter (Graphics::getCurrentRenderTargetWidth() / 2, Graphics::getCurrentRenderTargetHeight() / 2);
	setScreenCenter (screenCenter, false);
}

//----------------------------------------------------------------------

void CuiIoWin::getScreenCenterDefault(UIPoint & screenCenter) const
{
	screenCenter.Set(Graphics::getCurrentRenderTargetWidth() / 2, Graphics::getCurrentRenderTargetHeight() / 2);
}

//----------------------------------------------------------------------

void CuiIoWin::getScreenCenterOffset(UIPoint & screenCenterOffset) const
{
	UIPoint screenCenter;
	getScreenCenter(screenCenter);

	UIPoint screenDefaultCenter;
	getScreenCenterDefault(screenDefaultCenter);

	screenCenterOffset = screenCenter - screenDefaultCenter;
}


//----------------------------------------------------------------------

void CuiIoWin::setMouseLookState( MouseLookState state )
{
	UIManager::gUIManager().DrawCursor( state == MouseLookState_Disabled );
	ms_lastMouseLookState = ms_mouseLookState;
	ms_mouseLookState = state;
}

//----------------------------------------------------------------------

void CuiIoWin::updateMouseLookState( int eventType, const UIMessage &msg )
{
	if (getPointerInputActive())
	{
		if ( eventType == IOET_MouseButtonDown || eventType == IOET_MouseButtonUp )
		{
			const UIBaseObject * const mouseDownControl = UIManager::gUIManager().GetMouseDownControl();

			bool stateChange = ( eventType == IOET_MouseButtonUp ) && ( getMouseLookState() != MouseLookState_Disabled );

			stateChange = stateChange || ( mouseDownControl && ( mouseDownControl->GetName() == "GroundHUD" ) );
				
			MouseLookState newState = MouseLookState_Disabled;

			if ( stateChange  )
			{
				if ( CuiPreferences::getUseSwgMouseMap() )
				{
					if ( msg.Modifiers.LeftMouseDown && msg.Modifiers.RightMouseDown )
					{
						newState = MouseLookState_Avatar;
					}
					else if ( msg.Modifiers.LeftMouseDown )
					{
						newState = MouseLookState_Camera;
					}
					else if ( msg.Modifiers.RightMouseDown )
					{
						newState = MouseLookState_Move;
					}
				}
				else
				{
					if (msg.Modifiers.RightMouseDown)
					{
						newState = MouseLookState_Avatar;
					}
				}
			}

			setMouseLookState( newState );

 		}

		if ( getMouseLookState() == MouseLookState_Move )
		{
			InputMap *inputMap = Game::getGameInputMap();

			if ( inputMap )
			{
				inputMap->executeCommandByName( "CMD_walk" );
			}
		}
	}
}

//----------------------------------------------------------------------

bool CuiIoWin::shouldHoldTarget(const UIPoint & pt)
{
	// tells us whether or not we should get a new target
	// we don't want to get a target if it has been < ms_doubleClickTimeoutSecs since our last click
	// and we have not moved > m_doubleClickMoveThreshold
	// this helps to make double clicking small / moving targets much easier
	if (abs(m_lastClickPoint.x - pt.x) <= m_doubleClickMoveThreshold.x &&
		abs(m_lastClickPoint.y - pt.y) <= m_doubleClickMoveThreshold.y &&
		Game::getElapsedTime() - s_lastClickTimes [0] <= ms_doubleClickTimeoutSecs)
		return true;

	return false;
}

//======================================================================

