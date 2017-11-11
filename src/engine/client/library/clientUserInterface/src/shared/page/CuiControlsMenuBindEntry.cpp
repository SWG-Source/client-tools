// ======================================================================
//
// CuiControlsMenuBindEntry.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiControlsMenuBindEntry.h"

#include "LocalizedString.h"
#include "LocalizedStringTable.h"
#include "UIButton.h"
#include "UICursor.h"
#include "UIData.h"
#include "UIManager.h"
#include "UIPage.h"
#include "UIText.h"
#include "UnicodeUtils.h"
#include "clientUserInterface/CuiInputNames.h"
#include "clientUserInterface/CuiIoWin.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiStringIdsOptions.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "sharedInputMap/InputMap.h"
#include "sharedInputMap/InputMap_Command.h"
#include "sharedIoWin/IoWin.h"
#include "sharedIoWin/IoWinManager.h"
#include "sharedMessageDispatch/Message.h"

// ======================================================================

const char * const CuiControlsMenuBindEntry::Messages::UPDATE_BINDING = "CuiControlsMenuBindEntry::Messages::UPDATE_BINDING";


namespace CuiControlsMenuBindEntryNamespace
{
	const std::string s_pointerToggleCommand = "CMD_uiPointerToggle";
}

using namespace CuiControlsMenuBindEntryNamespace;

//-----------------------------------------------------------------

struct CuiControlsMenuBindEntry::BindWin : public IoWin
{
public:
	//lint -e754
	explicit BindWin(const char *debugName, CuiControlsMenuBindEntry * bindEntry);
	
	virtual IoResult processEvent(IoEvent *event);
	
	CuiControlsMenuBindEntry * m_bindEntry;
	
	bool                        m_die;
	bool                        m_restart;
	bool m_pause;
	
private:
	
	BindWin ();
	BindWin & operator=(const BindWin &); //lint !e754 //local struct member not ref
	BindWin (const BindWin &);            //lint !e754 //local struct member not ref
};

//-----------------------------------------------------------------

CuiControlsMenuBindEntry::BindWin::BindWin(const char *debugName, CuiControlsMenuBindEntry * bindEntry) :
IoWin              (debugName),
m_bindEntry        (bindEntry),
m_die (false),
m_restart (false),
m_pause(false)
{
}

//-----------------------------------------------------------------

IoResult CuiControlsMenuBindEntry::BindWin::processEvent (IoEvent *event)
{
	if (m_die)
		return IOR_PassKillMe;

	if (m_pause)
		return IOR_Pass;
	
	//-- pass these through, we don't care about them
	
	if (event->type == IOET_MouseMove   ||
		event->type == IOET_WindowOpen  ||
		event->type == IOET_WindowClose ||
		event->type == IOET_WindowKill  ||			
		event->type == IOET_Prepare     ||
		event->type == IOET_Update      ||
		event->type == IOET_SetSystemMouseCursorPosition)
		return IOR_Pass;

	if (event->type == IOET_Character)
		return IOR_Block;

	const uint32 shiftState = m_bindEntry->m_inputMap->getShiftState ();

	//-----------------------------------------------------------------

	if (event->type == IOET_KeyDown && m_bindEntry->m_bindInfo.type != InputMap::IT_None && shiftState == 0)
	{	
		//-- this should get picked up by the cancel button
		
		if (event->arg2 == CuiIoWin::getEscapeKeyCode ())
		{
			return IOR_Pass;
		}

		//-- this should get picked up by the ok button

		if (event->arg2 == CuiIoWin::getEnterKeyCode ())
		{
			return IOR_Pass;
		}
	}

	//-----------------------------------------------------------------
	//-- keep track of what modifiers are down, always
	//-- the messagequeue should always be null for this input map

	m_bindEntry->m_inputMap->processEvent (event);
	
	//-----------------------------------------------------------------
	//-- the bind entry menu wants input for certain things always, like the
	//-- cancel/ok buttons
	
	if (NON_NULL (UIManager::gUIManager ().GetRootPage ())->GetWidgetWithMouseLock ())
		return IOR_Pass;

	UIPoint cursorPt;
	CuiManager::getIoWin ().getCursorLocation (cursorPt);
	
	if (m_bindEntry->wantsInputAt (cursorPt))
		return IOR_Pass;
	
	//-----------------------------------------------------------------
	//-- if a button or key goes up, then go ahead mark this as being ready for a reset
	//-- this is to handle buttons which are modifiers but used in binds

	if (event->type == IOET_KeyUp || event->type == IOET_MouseButtonUp || event->type == IOET_JoystickButtonUp)
		return IOR_Block;
	
	//-- joymove threshold is pretty liberal here for ease of use
	static const real JOYMOVE_THRESHOLD = CONST_REAL (0.2);

	//-- ignore joystick centering msgs
	if (event->type == IOET_JoystickMove)
	{
		if (fabs (event->arg3) < JOYMOVE_THRESHOLD)
			return IOR_Block;
	}

	//-- ignore povhat centering msgs
	if (event->type == IOET_JoystickPovHat && event->arg3 < CONST_REAL (0.0))
		return IOR_Block;

	m_bindEntry->m_bindInfo = m_bindEntry->m_inputMap->getBindInfo ();

	if (InputMap::IT_None == m_bindEntry->m_bindInfo.type)
	{
		//-- joystick axes & sliders need special handling
		if (IOET_JoystickMove == event->type)
		{
			m_bindEntry->m_bindInfo.type = InputMap::IT_JoyAxis;
			m_bindEntry->m_bindInfo.value = event->arg2;
		}
		else if (IOET_JoystickSlider == event->type)
		{
			m_bindEntry->m_bindInfo.type = InputMap::IT_JoySlider;
			m_bindEntry->m_bindInfo.value = event->arg2;
		}
	}

	m_bindEntry->updateBindResultText();

	if (event->type == IOET_InputReset)
		return IOR_Pass;
	else
		return IOR_Block;
} //lint !e818

//-----------------------------------------------------------------

CuiControlsMenuBindEntry::CuiControlsMenuBindEntry (UIPage & thePage) :
CuiMediator               ("CuiControlsMenuBindEntry", thePage),
UIEventCallback           (), //lint !e1769
MessageDispatch::Receiver (),
MessageDispatch::Emitter  (),
m_cancelButton            (0),
m_okButton                (0),
m_bindText                (0),
m_bindResultText          (0),
m_command                 (0),
m_inputMap                (0),
m_bindInfo                (),
m_bindWin                 (0),
m_messageBox              (0),
m_modalWarning            (false),
m_overwriteWarning        (false),
m_inAddBindingConfirm     (false),
m_oldDiscardIdenticalJoystickMotionEvents(false)
{
	getCodeDataObject (TUIButton,  m_okButton,       "OkButton");
	getCodeDataObject (TUIButton,  m_cancelButton,   "CancelButton");
	getCodeDataObject (TUIText,    m_bindText,       "bindText");
	getCodeDataObject (TUIText,    m_bindResultText, "bindResultText");

	m_bindText->SetPreLocalized (true);

	Zero (m_bindInfo);

	registerMediatorObject (*m_cancelButton, true);
	registerMediatorObject (*m_okButton,     true);
	registerMediatorObject (getPage (),      true);
}

//-----------------------------------------------------------------

CuiControlsMenuBindEntry::~CuiControlsMenuBindEntry ()
{
	m_cancelButton   = 0;
	m_okButton       = 0;
	m_bindText       = 0;
	m_bindResultText = 0;
	m_command        = 0;
	delete m_bindWin;
	m_bindWin        = 0;

	m_inputMap       = 0;
	m_messageBox     = 0;
}

//-----------------------------------------------------------------

void CuiControlsMenuBindEntry::performActivate ()
{
	setPointerInputActive  (true);
	setKeyboardInputActive (true);
	setInputToggleActive   (false);
	
	Zero (m_bindInfo);

	UIRect rect;
	m_bindResultText->GetWorldRect (rect);

	CuiManager::getIoWin ().warpCursor ((rect.left + rect.right) / 2, (rect.top + rect.bottom) / 2);

	m_oldDiscardIdenticalJoystickMotionEvents = IoWinManager::isDiscardIdenticalJoystickMotionEvents();
	IoWinManager::setDiscardIdenticalJoystickMotionEvents(true);

	m_bindWin = new BindWin ("BindWin", this);
	m_bindWin->open ();

	m_okButton->SetFocus ();
}

//-----------------------------------------------------------------

void CuiControlsMenuBindEntry::performDeactivate ()
{
	setPointerInputActive  (false);
	setKeyboardInputActive (false);
	setInputToggleActive   (true);

	//-- bindWin is deleted by the IoWin stack, unless this is nonnull
	if (m_bindWin)
	{
		m_bindWin->m_die = true;
		m_bindWin = NULL; //lint !e423 //not a memory leak
	}

	IoWinManager::setDiscardIdenticalJoystickMotionEvents(m_oldDiscardIdenticalJoystickMotionEvents);
}

//-----------------------------------------------------------------

void CuiControlsMenuBindEntry::OnButtonPressed           ( UIWidget *context )
{
	if (context == m_okButton)
	{
		m_modalWarning     = false;
		m_overwriteWarning = false;
		
		if (doAddBinding ())
		{	
			deactivate ();
		}
		else if (NULL != m_bindWin)
			m_bindWin->m_pause = true;
	}
	else if (context == m_cancelButton)
	{
		deactivate ();
	}
} //lint !e818

//-----------------------------------------------------------------

bool CuiControlsMenuBindEntry::OnMessage           ( UIWidget *context, const UIMessage & )
{

	if (context == &getPage ())
	{

	}

	return true;
} //lint !e818

//-----------------------------------------------------------------

void CuiControlsMenuBindEntry::setCommand (InputMap * imap, const InputMap::Command * cmd)
{
	m_inputMap = imap;
	m_command  = cmd;

	if (imap == 0 || cmd == 0)
		return;

	Zero (m_bindInfo);

	const Unicode::String & localizedCmdName = CuiInputNames::getLocalizedCommandName (m_command->name);
	Unicode::String bindText;

	CuiStringVariablesManager::process (CuiStringIds::bind_key_prose, localizedCmdName, Unicode::emptyString, Unicode::emptyString, bindText);

	m_bindText->SetLocalText (bindText);

	m_bindResultText->Clear ();
}

//-----------------------------------------------------------------

bool CuiControlsMenuBindEntry::wantsInputAt (const UIPoint & pt) const
{
	return (m_cancelButton->GetWorldRect ().ContainsPoint (pt) || m_okButton->GetWorldRect ().ContainsPoint (pt));
}

//-----------------------------------------------------------------

void CuiControlsMenuBindEntry::updateBindResultText ()
{
	Unicode::String str;
	CuiInputNames::appendInputString (m_bindInfo, str);

	m_bindResultText->SetLocalText (str);
	m_okButton->SetEnabled (!str.empty ());
}

//-----------------------------------------------------------------

bool CuiControlsMenuBindEntry::doAddBinding ()
{
	
	NOT_NULL (m_inputMap);
	NOT_NULL (m_command);

	//-----------------------------------------------------------------
	//-- check appropriate type
	{
		uint32 type = 0;

		if (InputMap::IT_Key == m_bindInfo.type || InputMap::IT_MouseButton == m_bindInfo.type || InputMap::IT_JoyButton == m_bindInfo.type)
			type = InputMap::Command::T_BUTTON;
		else if (InputMap::IT_JoyPovHat == m_bindInfo.type)
			type = InputMap::Command::T_POVHAT;
		else if (InputMap::IT_JoyAxis == m_bindInfo.type)
			type = InputMap::Command::T_AXIS;
		else if (InputMap::IT_JoySlider == m_bindInfo.type)
			type = InputMap::Command::T_SLIDER;

		if ((m_command->types & type) == 0)
		{
			DEBUG_FATAL(NULL != m_messageBox, ("too many messageboxes"));
			m_messageBox = CuiMessageBox::createInfoBox(CuiStringIdsOptions::invalid_command_type.localize());
			m_messageBox->connectToMessages(*this);
			return false;
		}
	}
	

	//-----------------------------------------------------------------
	//-- warn if we are binding a chat key in non-modal chat mode

	if (!m_modalWarning)
	{
		m_modalWarning = true;
		
		if (!CuiPreferences::getModalChat () &&
			m_bindInfo.type == InputMap::IT_Key &&
			m_bindInfo.shiftState == 0 &&
			CuiIoWin::isChatFodder (m_bindInfo.value))
		{
			DEBUG_FATAL (m_messageBox != 0, ("too many messageboxes.\n"));
			m_inAddBindingConfirm = true;
			m_messageBox = CuiMessageBox::createYesNoBox (CuiStringIdsOptions::confirm_keymap_chat_clash.localize ());
			m_messageBox->connectToMessages (*this);
			return true;
		}
	}
	
	//-----------------------------------------------------------------
	//-- warn if we are overwriting a key binding

	if (!m_overwriteWarning)
	{
		NOT_NULL (m_inputMap);
		const InputMap::Command * const cmd = m_inputMap->getCommandByBinding (m_bindInfo);		
		
		if (cmd && cmd != m_command)
		{
			const Unicode::String & localizedCommandName = CuiInputNames::getLocalizedCommandName (cmd->name);
			Unicode::String result;

			if (cmd->name == s_pointerToggleCommand)
			{
				CuiStringVariablesManager::process (CuiStringIds::bind_conflict_cannot_overwrite_prose, m_bindResultText->GetLocalText (), localizedCommandName, Unicode::emptyString, result);
				CuiMessageBox::createInfoBox (result);
				return false;
			}
	
			m_overwriteWarning = true;

			m_inAddBindingConfirm = true;
			CuiStringVariablesManager::process (CuiStringIds::bind_conflict_prose, m_bindResultText->GetLocalText (), localizedCommandName, Unicode::emptyString, result);
			m_messageBox = CuiMessageBox::createYesNoBox (result);
			m_messageBox->connectToMessages (*this);
			
			return true;
		}
	}

	//-----------------------------------------------------------------
	//-- make sure we don't exceed 2 binds per command
	
	InputMap::CommandBindInfoSet * cmdBindInfoSets = 0;
	const uint32 retval = m_inputMap->getCommandBindings (cmdBindInfoSets, m_command);
	UNREF (retval);
	DEBUG_FATAL (retval != 1, ("unable to find command binding set.\n"));
	
	if (cmdBindInfoSets)
	{
		InputMap::CommandBindInfoSet & cbis = cmdBindInfoSets [0];
		
		if (cbis.numBinds > 1)
		{
			m_inputMap->removeBindings (m_command);
			if (!m_inputMap->addBinding (cbis.binds [0], m_command))
			{
				WARNING(true, ("CuiControlsMenuBindEntry failed to bind command for rebind"));
			}
		}

		delete[] cmdBindInfoSets;
		cmdBindInfoSets = 0;
	}
	
	//-----------------------------------------------------------------
	//-- finally add the binding
	
	if (!m_inputMap->addBinding (m_bindInfo, m_command))
	{
		WARNING(true, ("CuiControlsMenuBindEntry failed to bind command"));
	}

	emitMessage (MessageDispatch::MessageBase (CuiControlsMenuBindEntry::Messages::UPDATE_BINDING));

	return true;
}

//-----------------------------------------------------------------------

void CuiControlsMenuBindEntry::receiveMessage(const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message)
{
	UNREF(source);

	//----------------------------------------------------------------------
	
	const CuiMessageBox::BoxMessage * const abm = dynamic_cast<const CuiMessageBox::BoxMessage *>(&message);
	
	if (abm)
	{
		NOT_NULL (m_messageBox);

		const CuiMessageBox & box = *NON_NULL (abm->getMessageBox ());

		if (&box != m_messageBox)
			return;

		//----------------------------------------------------------------------
		//-- go ahead and disconnect, since we might deactivate and potentially delete this page
		m_messageBox->disconnectFromMessages (*this);
		m_messageBox = 0;

		if (message.isType (CuiMessageBox::Messages::COMPLETED))
		{
			if (NULL != m_bindWin && m_bindWin->m_pause)
				m_bindWin->m_pause = false;

			if (m_inAddBindingConfirm)
			{
				m_inAddBindingConfirm = false;
				
				if (box.completedAffirmative ())
				{
					if (doAddBinding ())
						deactivate ();
				}
			}
		}
	}
}

// ======================================================================
