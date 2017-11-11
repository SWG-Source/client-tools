//--
//-- CuiMediator.cpp
//-- jwatson 2001-02-11
//-- copyright 2001 Sony Online Entertainment
//--

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiMediator.h"

#include "clientGame/ClientObject.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/Game.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiMediatorTypes.h"
#include "clientUserInterface/CuiMediator_MediatorEventCallback.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiPopupHelp.h"
#include "clientUserInterface/CuiPopupHelpData.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiSettings.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "clientUserInterface/CuiWorkspace.h"
#include "clientUserInterface/CuiWorkspaceIcon.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/Profiler.h"
#include "sharedObject/CachedNetworkId.h"

#include "UIButton.h"
#include "UICheckbox.h"
#include "UIComposite.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIManager.h"
#include "UIPage.h"
#include "UIPopupMenu.h"
#include "UITextbox.h"
#include "UIUtils.h"

#include <typeinfo.h>
#include <vector>
#include <list>

//-----------------------------------------------------------------

namespace CuiMediatorNamespace
{
	typedef std::vector<CuiMediator *> MediatorVector;
	static MediatorVector s_mediators;

	static MediatorVector s_mediatorsToAdd;
	static MediatorVector s_mediatorsToAddNext;

	static bool                         s_debugMediatorCollection = false;
	static bool                         s_debugInit = false;
	static bool                         s_debugLogMediatorRefCount = false;

	real const maxSaneCoordinate        = 100000.0f;

	namespace Settings
	{
		static const std::string popupHelpEnabled = "popupHelpEnabled";
	}
	static const UILowerString prop_keyboardInputActive = UILowerString ("KeyboardInputActive");

	void install();
	void remove();

	void getAutoRegisterObjects(UIWidget & parent, UIBaseObject::UIObjectList & autoRegisterObjects)
	{
		UIBaseObject::UIObjectList children;
		parent.GetChildren(children);

		UIBaseObject::UIObjectList::iterator itEnd = children.end();
		for (UIBaseObject::UIObjectList::iterator it = children.begin(); it != itEnd; ++it) 
		{
			UIWidget * const widget = UI_ASOBJECT(UIWidget, *it);
			if (widget) 
			{
				if (widget->GetAutoRegister()) 
				{
					autoRegisterObjects.push_back(widget);
				}

				getAutoRegisterObjects(*widget, autoRegisterObjects);
			}
		}
	}
}
using namespace CuiMediatorNamespace;

//-----------------------------------------------------------------

const char * const CuiMediator::ButtonPaths::Close             = "ButtonClose";
const char * const CuiMediator::ButtonPaths::MaximizeRestore   = "ButtonMaximize";
const char * const CuiMediator::ButtonPaths::Minimize          = "ButtonMinimize";

int CuiMediator::ms_countPointer  = 0;
int CuiMediator::ms_countKeyboard = 0;
int CuiMediator::ms_countToggle   = 0;

//-----------------------------------------------------------------

void CuiMediatorNamespace::install()
{
}

//-----------------------------------------------------------------

void CuiMediatorNamespace::remove()
{
	s_mediatorsToAdd.insert (s_mediatorsToAdd.end (), s_mediatorsToAddNext.begin (), s_mediatorsToAddNext.end ());
	s_mediatorsToAddNext.clear ();
	for (MediatorVector::iterator it = s_mediatorsToAdd.begin (); it != s_mediatorsToAdd.end (); ++it)
	{
		CuiMediator * const mediator = NON_NULL (*it);
		s_mediators.push_back (mediator);
	}
	s_mediatorsToAdd.clear ();
}

//-----------------------------------------------------------------

CuiMediator::CuiMediator (const char * const mediatorDebugName, UIPage & newPage) :
m_mediatorDebugName      (NON_NULL (mediatorDebugName)),
m_thePage                (newPage),
m_codeData               (0),
m_previousMediator       (0),
m_states                 (MS_maximizable | MS_movable | MS_resizable | MS_iconifiable | MS_settingsEnabled | MS_popupHelpOk),
m_icon                   (0),
m_lockedAspectRatio      (new UIFloatPoint (0.0f, 0.0f)),
m_restoreRect            (new UIRect ()),
m_containingWorkspace    (0),
m_buttonClose            (0),
m_buttonMaximizeRestore  (0),
m_buttonMinimize         (0),
m_eventCallback          (0),
m_refcount               (0),
m_startSize              (),
m_popupHelp              (0),
m_popupHelpData          (0),
m_checkboxPopupHelp      (0),
m_popupHelpIndex         (-1),
m_objectCallbackVector   (new ObjectCallbackVector),
m_widgetToFocus          (0),
m_associatedObjectId     (new CachedNetworkId),
m_associatedLocation     (Vector::maxXYZ), // "unset" value is Vector::maxXYZ
m_maxRangeFromObject     (-1.0f),
m_lastObjectCheckTime    (0.0f),
m_showFocusedGlowRect    (true)
{

	if (!s_debugInit)
	{
		s_debugInit = true;
		DebugFlags::registerFlag (s_debugMediatorCollection,       "ClientUserInterface", "mediatorCollection");
		DebugFlags::registerFlag (s_debugLogMediatorRefCount,      "ClientUserInterface", "logMediatorRefCount");
	}

	m_thePage.Attach (0);

	m_codeData = GET_UI_OBJ (m_thePage, UIData, "CodeData");

	m_thePage.SetVisible (false);
	
	//-- the following 3 buttons are optional

	if (m_codeData)
	{
		getCodeDataObject (TUIButton, m_buttonClose,           ButtonPaths::Close,           true);
		getCodeDataObject (TUIButton, m_buttonMaximizeRestore, ButtonPaths::MaximizeRestore, true);
		getCodeDataObject (TUIButton, m_buttonMinimize,        ButtonPaths::Minimize,        true);

		getCodeDataObject (TUICheckbox, m_checkboxPopupHelp,   "checkPopupHelp",             true);

		static const UILowerString prop_stickyVisible = UILowerString ("StickyVisible");
		static const UILowerString prop_startSize     = UILowerString ("StartSize");
		static const UILowerString prop_focusGlow     = UILowerString ("FocusGlow");

		getCodeDataObject (TUIWidget,    m_widgetToFocus,      "WidgetToFocus", true);

		if (m_widgetToFocus)
			m_widgetToFocus->Attach (0);

		bool b = false;
		if (m_codeData->GetPropertyBoolean (prop_stickyVisible, b))
			setOrRemoveState (MS_stickyVisible, b);

		if (m_codeData->GetPropertyBoolean (prop_focusGlow, b))
			setShowFocusedGlowRect(b);

		UISize startSize;
		if (m_codeData->GetPropertyPoint (prop_stickyVisible, startSize))
		{
			setOrRemoveState (MS_hasStartSize, true);
			m_startSize = startSize;
		}

		UIDataSource * popupHelpDataSource = 0;

		getCodeDataObject (TUIDataSource, popupHelpDataSource,        "PopupHelp",        true);

		if (popupHelpDataSource)
		{
			m_popupHelpData = new PopupHelpDataVector;
			const UIDataList & dataList = popupHelpDataSource->GetData ();

			for (UIDataList::const_iterator it = dataList.begin (); it != dataList.end (); ++it)
			{
				UIData * const data = *it;
				NOT_NULL (data);

				CuiPopupHelpData cphd (*data);
				m_popupHelpData->push_back (cphd);
			}
		}

		b = false;
		if (m_codeData->GetPropertyBoolean (prop_keyboardInputActive, b) && b)
		{
			setState(MS_getsKeyboardInput);
		}

	}
	
	if (m_buttonClose)
		m_buttonClose->Attach (0);
	if (m_buttonMaximizeRestore)
		m_buttonMaximizeRestore->Attach (0);
	if (m_buttonMinimize)
		m_buttonMinimize->Attach (0);
	if (m_checkboxPopupHelp)
	{
		m_checkboxPopupHelp->SetChecked (false, false);
		m_checkboxPopupHelp->Attach (0);
	}

	m_eventCallback  = new MediatorEventCallback (*this);

	s_mediatorsToAddNext.push_back (this);

	if (hasState (MS_hasStartSize))
		m_thePage.SetSize (m_startSize);

	m_defaultLocation = m_thePage.GetLocation();
	m_defaultSize = m_thePage.GetSize();
}

//-----------------------------------------------------------------

CuiMediator::~CuiMediator ()
{
	delete m_associatedObjectId;
	m_associatedObjectId = 0;

	if (m_widgetToFocus)
	{
		m_widgetToFocus->Detach (0);
		m_widgetToFocus = 0;
	}

	{
		const MediatorVector::iterator it = std::find (s_mediators.begin (), s_mediators.end (), this);
		if (it != s_mediators.end ())
			DEBUG_FATAL (true, ("mediator was still in the mediator list in dtor"));
	}
	DEBUG_FATAL (m_refcount != 0, ("bad refcount in CuiMediator dtor"));

	if (m_objectCallbackVector)
	{
		unregisterMediatorObjects();
		delete m_objectCallbackVector;
		m_objectCallbackVector = 0;
	}
	
	m_codeData = 0;
	setIcon (0);
	
	m_previousMediator = 0;
	m_icon = 0;
	m_containingWorkspace = 0;

	if (m_buttonClose)
	{
		m_buttonClose->RemoveCallback           (m_eventCallback);
		m_buttonClose->Detach (0);
	}
	if (m_buttonMaximizeRestore)
	{
		m_buttonMaximizeRestore->RemoveCallback (m_eventCallback);
		m_buttonMaximizeRestore->Detach (0);
	}
	if (m_buttonMinimize)
	{
		m_buttonMinimize->RemoveCallback        (m_eventCallback);
		m_buttonMinimize->Detach (0);
	}
	if (m_checkboxPopupHelp)
	{		
		m_checkboxPopupHelp->RemoveCallback      (m_eventCallback);
		m_checkboxPopupHelp->Detach (0);
	}

	m_buttonClose             = 0;
	m_buttonMaximizeRestore   = 0;
	m_buttonMinimize          = 0;
	m_checkboxPopupHelp       = 0;

	delete m_eventCallback;
	m_eventCallback = 0;

	m_thePage.Detach (0);

	delete m_lockedAspectRatio;
	m_lockedAspectRatio = 0;

	delete m_restoreRect;
	m_restoreRect = 0;

	delete m_popupHelpData;
	m_popupHelpData = 0;

	if (m_popupHelp)
	{
		m_popupHelp->release();
		m_popupHelp = 0;
	}
}

//-----------------------------------------------------------------

void CuiMediator::activate (const char * previousMediator)
{
	if (m_containingWorkspace && !m_containingWorkspace->getEnabled ())
	{
		setWasVisible (true);
		return;
	}
	
	if (previousMediator)
		m_previousMediator = previousMediator;

	if (!isActive ())
	{
		setWasVisible (false);

		if (isIconified () && m_icon && m_icon->GetParent ())
		{
			m_icon->SetVisible (true);
		}
		else
		{
			setIconified (false);
			m_thePage.SetVisible (true);
			m_thePage.SetFocus ();
		}
		
		setState (MS_active);

		if (m_widgetToFocus)
		{
			m_widgetToFocus->SetFocus ();
			//if the selected widget is a textbox, autoselect the text
			UITextbox* const tb = dynamic_cast<UITextbox* const>(m_widgetToFocus);
			if(tb)
			{
				tb->SelectAll();
				tb->MoveCaratToEndOfLine();
			}
		}

		removeState (MS_closeNextFrame);

		performActivate();
		
		//-- performActivate can force a deactivate ()
		if (isActive ())
		{
			if (hasState(MS_getsKeyboardInput))
			{
				setKeyboardInputActive(true);
			}

			if (m_maxRangeFromObject > 0.0f)
			{
				if (m_associatedObjectId->isValid () || (m_associatedLocation.x < maxSaneCoordinate))
					setIsUpdating (true);
			}

			if (getSettingsEnabled () && !hasState (MS_settingsLoaded))
			{
				loadSettings ();
				setState (MS_settingsLoaded);
			}

			if (m_popupHelpData && hasState (MS_popupHelpOk) && m_popupHelpIndex < 0 && CuiPreferences::getShowPopupHelp ())
				startPopupHelp ();
			else if (m_popupHelp)
			{
				m_popupHelp->openNextFrame();
			}

			if (m_buttonClose)
				m_buttonClose->AddCallback           (m_eventCallback);
			if (m_buttonMaximizeRestore)
				m_buttonMaximizeRestore->AddCallback (m_eventCallback);
			if (m_buttonMinimize)
				m_buttonMinimize->AddCallback        (m_eventCallback);
			if (m_checkboxPopupHelp)
				m_checkboxPopupHelp->AddCallback     (m_eventCallback);

			UIEventCallback * const eventCallback = getCallbackObject();
			
			if (eventCallback)
			{
				for (ObjectCallbackVector::iterator it = m_objectCallbackVector->begin (); it != m_objectCallbackVector->end (); ++it)
				{
					
					const ObjectCallbackData & ocd = *it;
					const bool activeCallbacks = ocd.second;
					
					if (!activeCallbacks)
						continue;
					
					UIBaseObject * const obj   = NON_NULL (ocd.first);
					
					if (obj->IsA (TUIWidget))
					{
						UIWidget * const wid = safe_cast<UIWidget *>(obj);
						//Why check for the callback?  Because registerMediatorObject checks if the page is active, and if it is, it
						//adds in the callback for the widget.  But activate also adds in the callback.  Problem: registerMediatorObject
						//uses isActive to check, which is true when the page is being activated, although activate hasn't been called yet.
						//Result: double-add.
						if(!wid->HasCallback(eventCallback))
							wid->AddCallback (eventCallback);
					}
				}
			}

			if (m_containingWorkspace)
				m_containingWorkspace->updateGlow ();

			autoRegisterWidgets();
		}
	}
}

//-----------------------------------------------------------------

UIEventCallback* CuiMediator::getCallbackObject()
{
	UIEventCallback * const eventCallback = dynamic_cast<UIEventCallback *>(this); //lint !e740 //unusual indirect cast
	return eventCallback;
}

//-----------------------------------------------------------------

void CuiMediator::deactivate () 
{
	if (isActive ())
	{
		if (m_icon)
			m_icon->SetVisible (false);

		m_thePage.SetSelected (false);
		m_thePage.SetVisible (false);
		removeState (MS_active);

		UIPopupMenu * const associatedPopupMenu = UIManager::gUIManager().FindAssociatedPopupMenu(&m_thePage);
		if (associatedPopupMenu != 0)
		{
			UIManager::gUIManager().PopContextWidgets(associatedPopupMenu);
		}


		performDeactivate();

		//-- performDeactivate () can re-activate () the mediator
		if (!isActive ())
		{
			if (hasState(MS_getsKeyboardInput))
			{
				setKeyboardInputActive(false);
			}

			setWasVisible (false);

			if (getSettingsEnabled ())
			{
				CuiSettings::clearDataForOwner(m_mediatorDebugName);

				saveSettings ();

				if (m_popupHelpData || hasState (MS_settingsAutoSize) || hasState (MS_settingsAutoLoc))
					CuiSettings::setDirty (true);
			}

			if (m_popupHelp)
			{
//				if (m_containingWorkspace && m_containingWorkspace->hasMediator (*m_popupHelp))
//					m_containingWorkspace->removeMediator (*m_popupHelp);

				m_popupHelp->setForceClose (true);
				m_popupHelp->closeNextFrame ();
			}

			UIEventCallback * const eventCallback = getCallbackObject();
			
			if (eventCallback)
			{
				for (ObjectCallbackVector::iterator it = m_objectCallbackVector->begin (); it != m_objectCallbackVector->end (); ++it)
				{
					
					const ObjectCallbackData & ocd = *it;
					const bool activeCallbacks = ocd.second;
					
					if (!activeCallbacks)
						continue;
					
					UIBaseObject * const obj   = NON_NULL (ocd.first);
					
					if (obj->IsA (TUIWidget))
					{
						UIWidget * const wid = safe_cast<UIWidget *>(obj);
						wid->RemoveCallback (eventCallback);
					}
				}
			}
			
			if (m_buttonClose)
				m_buttonClose->RemoveCallback           (m_eventCallback);
			if (m_buttonMaximizeRestore)
				m_buttonMaximizeRestore->RemoveCallback (m_eventCallback);
			if (m_buttonMinimize)
				m_buttonMinimize->RemoveCallback        (m_eventCallback);
			if (m_checkboxPopupHelp)
				m_checkboxPopupHelp->RemoveCallback        (m_eventCallback);
			
			if (getInputToggleActive ())
				setInputToggleActive (false);
			
			if (getKeyboardInputActive ())
				setKeyboardInputActive (false);
			
			if (getPointerInputActive ())
				setPointerInputActive (false);

			if (m_containingWorkspace)
				m_containingWorkspace->updateGlow ();

			autoUnregisterWidgets();
		}
	}
}

//-----------------------------------------------------------------
/**
* Subclasses override this
*/

void	CuiMediator::performActivate ()
{
}

//-----------------------------------------------------------------
/**
* Subclasses override this
*/
void	CuiMediator::performDeactivate ()
{
}

//-----------------------------------------------------------------

void CuiMediator::setInputToggleActive (bool b)
{
	if (setOrRemoveState (MS_toggleActive, b))
	{
		if (b)
			++ms_countToggle;
		else
			--ms_countToggle;
		CuiManager::InputManager::setInputToggleActive (b);
	}
}

//----------------------------------------------------------------------

void CuiMediator::incrementKeyboardInputActiveCount (int num)
{
	ms_countKeyboard += num;
	CuiManager::InputManager::setKeyboardInputActive (ms_countKeyboard > 0);
}

//----------------------------------------------------------------------

void CuiMediator::incrementPointerInputActiveCount  (int num)
{
	ms_countPointer += num;
	CuiManager::InputManager::setPointerInputActive (ms_countPointer > 0);
}

//-----------------------------------------------------------------

void CuiMediator::setKeyboardInputActive (bool b)
{
	if (setOrRemoveState (MS_keyboardActive, b))
	{
		if (b)
			incrementKeyboardInputActiveCount (1);
		else
			incrementKeyboardInputActiveCount (-1);
	}
}

//-----------------------------------------------------------------

void CuiMediator::setPointerInputActive (bool b)
{
	if (setOrRemoveState (MS_pointerActive, b))
	{
		if (b)
			++ms_countPointer;
		else
			--ms_countPointer;
		CuiManager::InputManager::setPointerInputActive (b);
	}
}

//-----------------------------------------------------------------

const char * CuiMediator::getPreviousMediator () const
{
	return m_previousMediator;
}

//-----------------------------------------------------------------

void CuiMediator::setPreviousMediator (const char * menu)
{
	m_previousMediator = menu;
}

//-----------------------------------------------------------------

void CuiMediator::setIcon (CuiWorkspaceIcon * icon)
{
	DEBUG_FATAL (&getPage () == icon, ("dont icon self\n"));

	if (icon)
	{
		icon->AddCallback (icon);
		icon->Attach (0);
	}

	if (m_icon)
	{
		m_icon->RemoveCallback (icon);
		m_icon->Detach (0);
	}

	m_icon = icon;
}

//----------------------------------------------------------------------

bool CuiMediator::getLockedAspectRatio (UIFloatPoint & ratio) const
{
	ratio = *m_lockedAspectRatio;
	return (ratio.x != 0.0f && ratio.y != 0.0f);
}

//----------------------------------------------------------------------

void CuiMediator::setLockedAspectRatio (const UIFloatPoint & ratio)
{
	*m_lockedAspectRatio = ratio;
}

//----------------------------------------------------------------------

bool CuiMediator::getIconLocation (UIPoint & loc) const
{
	if (m_icon)
	{
		loc = m_icon->GetLocation ();
		return true;
	}
	return false;
}

//-----------------------------------------------------------------

/**
* @return false if the workspace should go ahead and make the window large, or true if that was handled by the mediator
*/
bool CuiMediator::maximize ()
{
	setRestoreRect (m_thePage.GetRect ());
	setState       (MS_maximized);
	return false;
}

//-----------------------------------------------------------------
/**
* @return false if the workspace should go ahead and resize the window, or true if that was handled by the mediator
*/

bool CuiMediator::restore ()
{
	removeState (MS_maximized);
	return false;
}

//-----------------------------------------------------------------

/**
* @return false if the workspace should go ahead and delete the mediator, true if the closing was already fully handled by the mediator
*/
bool CuiMediator::close    ()
{
	removeState (MS_openNextFrame);

	if (isCloseDeactivates ())
	{
		deactivate ();
	}

	stopPopupHelp();

	return false;
}

//----------------------------------------------------------------------

void CuiMediator::update (float)
{
	if (!m_associatedObjectId->isValid () && (m_associatedLocation.x > maxSaneCoordinate))
		return;

	if (!m_containingWorkspace)
		return;

	const float currentTime = Game::getElapsedTime ();
	const float deltaTime = currentTime - m_lastObjectCheckTime;

	static const float OBJECT_CHECK_TIMEOUT = 1.0f;

	if (deltaTime < OBJECT_CHECK_TIMEOUT)
		return;

	m_lastObjectCheckTime = currentTime;

	const Object * const player = Game::getPlayer ();
	const ClientObject * obj = 0;

	if (player)
	{
		if (m_associatedObjectId->isValid ())
		{
			obj = safe_cast<ClientObject *>(m_associatedObjectId->getObject ());

			if (obj)
			{
				//-- infinite range
				if (m_maxRangeFromObject <= 0.0f)
				{
					return;
				}
				
				const Object * const containerParent = ContainerInterface::getFirstParentInWorld (*obj);
				
				const Vector & objPos_w    = containerParent->findPosition_w ();
				const Vector & playerPos_w = player->findPosition_w ();
				
				if ((objPos_w.magnitudeBetween (playerPos_w) - containerParent->getAppearanceSphereRadius ()) < m_maxRangeFromObject)
					return;
			}
		}
		else
		{
			//-- infinite range
			if (m_maxRangeFromObject <= 0.0f)
			{
				return;
			}

			const Vector & playerPos_w = player->findPosition_w ();

			if (m_associatedLocation.magnitudeBetween (playerPos_w) < m_maxRangeFromObject)
				return;
		}
	}

	static Unicode::String result;
	result.clear ();

	if (m_associatedObjectId->isValid ())
	{
		if (obj)
			CuiStringVariablesManager::process (CuiStringIds::sui_out_of_range_prose, Unicode::emptyString, obj->getLocalizedName (), Unicode::emptyString, result);
		else
			result = CuiStringIds::sui_obj_lost.localize ();
	}
	else
	{
		result = CuiStringIds::sui_out_of_location_range_prose.localize ();
	}

	CuiSystemMessageManager::sendFakeSystemMessage (result);

	//-- if we fell through, the objects are invalid or the distance is too great
	closeThroughWorkspace ();
}

//----------------------------------------------------------------------

void CuiMediator::setContainingWorkspace (CuiWorkspace * workspace)
{
	m_containingWorkspace = workspace;
}

//----------------------------------------------------------------------

bool CuiMediator::getCodeDataString (const char * name, UIString & value) const
{
	if (!m_codeData)
		return false;

	return m_codeData->GetProperty (UILowerString (name), value);
}

//----------------------------------------------------------------------

void CuiMediator::setIconified (bool b)
{
	setOrRemoveState (MS_iconified, b);
}

//----------------------------------------------------------------------

bool CuiMediator::test (std::string & result)
{
	if (m_containingWorkspace)
	{
	
	}

	UNREF (result);
	return false;
}

//-----------------------------------------------------------------

bool CuiMediator::setOrRemoveState (MediatorStates s, bool b)
{
	if (b)
		return setState (s);
	else
		return removeState (s);
}

//-----------------------------------------------------------------

void CuiMediator::setEnabled   (bool b)
{
	setOrRemoveState (MS_enabled, b);
	getPage ().SetEnabled (b);

	if (m_buttonClose)
		m_buttonClose->SetEnabled           (b);
	if (m_buttonMaximizeRestore)
		m_buttonMaximizeRestore->SetEnabled (b);
	if (m_buttonMinimize)
		m_buttonMinimize->SetEnabled        (b);
}

//----------------------------------------------------------------------

void CuiMediator::setStickyVisible       (bool b)
{
	setOrRemoveState (MS_stickyVisible, b);
}

//----------------------------------------------------------------------

void CuiMediator::setWasVisible          (bool b)
{
	setOrRemoveState (MS_wasVisible, b);
}

//----------------------------------------------------------------------

void CuiMediator::setIsUpdating (bool b)
{
	setOrRemoveState (MS_updating, b);
}

//----------------------------------------------------------------------

void CuiMediator::setSettingsEnabled     (bool b)
{
	setOrRemoveState (MS_settingsEnabled, b);
}

//-----------------------------------------------------------------

void CuiMediator::fetch ()
{
	++m_refcount;

	DEBUG_REPORT_LOG(s_debugLogMediatorRefCount, ("CuiMediator::fetch() name [%s] ptr [0x%08x] m_refCount [%d]\n", getMediatorDebugName ().c_str (), this, m_refcount));
}

//-----------------------------------------------------------------

void CuiMediator::release ()
{
	--m_refcount;

	DEBUG_REPORT_LOG(s_debugLogMediatorRefCount, ("CuiMediator::release() name [%s] ptr [0x%08x] m_refCount [%d]\n", getMediatorDebugName ().c_str (), this, m_refcount));
	DEBUG_FATAL (m_refcount < 0, ("bad CuiMediator refcount on release()"));
}

//-----------------------------------------------------------------

void CuiMediator::garbageCollect (bool force)
{
	for (MediatorVector::iterator it = s_mediators.begin (); it != s_mediators.end ();)
	{
		CuiMediator * const mediator = NON_NULL (*it);

		if (mediator->getRefCount () == 0 && (force || !mediator->isOpenNextFrame ()))
		{
			REPORT_LOG_PRINT (s_debugMediatorCollection, 
				(" :: CuiMediator::garbageCollect () mediator on [%s] [%s]\n", mediator->getMediatorDebugName ().c_str (), mediator->getPage ().GetFullPath ().c_str ()));

			mediator->deactivate ();
			it = s_mediators.erase (it);
			delete mediator;
		}
		else
			++it;
	}

	if (UIManager::isUIReady()) 
	{
		UIManager::gUIManager().garbageCollect();
	}
}

//-----------------------------------------------------------------

int CuiMediator::getRemainingMediatorCount ()
{
	return static_cast<int>(s_mediators.size ());
}

//----------------------------------------------------------------------

void CuiMediator::updateAll (float deltaTimeSecs)
{
#if _DEBUG
	//-- the profiler does not store a copy of the profile node name passed in
	//-- it stores a pointer.  Therefore we need a separate buffer for each node
	//-- we wish to create during the mediator update

	static const size_t numProfilerBufs = 128;
	static char         buf [numProfilerBufs][128];
	static const size_t buf_size = sizeof (buf [0]);
	int                 warnedProfilerBufsCount = 0;


	std::set <std::string> ns;

#endif
	{
		int i = 0;
		for (MediatorVector::iterator it = s_mediators.begin (); it != s_mediators.end (); ++it)
		{
			CuiMediator * const mediator = NON_NULL (*it);
			
			if (mediator->getRefCount () > 0)
			{
				if (mediator->isCloseNextFrame ())
				{
					mediator->removeState (MS_closeNextFrame);
					mediator->closeThroughWorkspace ();
					continue;
				}

				if (mediator->isOpenNextFrame ())
				{
					mediator->removeState (MS_openNextFrame);
					mediator->open ();
					continue;
				}

				if (mediator->m_popupHelp)
					mediator->updatePopupHelp (deltaTimeSecs);

				if (mediator->isUpdating ())
				{
#if _DEBUG
					if (i < numProfilerBufs)
					{
						snprintf (buf [i], buf_size, "%-30s 0x%08x", mediator->getMediatorDebugName ().c_str (), reinterpret_cast<int>(mediator));
						//PROFILER_START (buf [i]);
					}
#endif
					//-- actually do the update
					mediator->update (deltaTimeSecs);

#if _DEBUG
					if (i < numProfilerBufs)
						//PROFILER_STOP  (buf[i]);
#endif
					++i;
				}
			}
		}

		DEBUG_WARNING (i >= numProfilerBufs && i > warnedProfilerBufsCount, ("CuiMediator ran out of profiler name buffers, needed %d have %d", i, numProfilerBufs));
	}
	
	s_mediatorsToAdd.insert (s_mediatorsToAdd.end (), s_mediatorsToAddNext.begin (), s_mediatorsToAddNext.end ());
	s_mediatorsToAddNext.clear ();

	for (MediatorVector::iterator it = s_mediatorsToAdd.begin (); it != s_mediatorsToAdd.end (); ++it)
	{
		CuiMediator * const mediator = NON_NULL (*it);

		s_mediators.push_back (mediator);
		if(mediator->getRefCount () == 0 && mediator->isOpenNextFrame ())
		{
			mediator->removeState (MS_openNextFrame);
			mediator->open ();
		}
	}

	s_mediatorsToAdd.clear ();

	CuiMediatorFactory::clearActivatedThisFrame();
}

//----------------------------------------------------------------------

void CuiMediator::closeThroughWorkspace()
{
	if (m_containingWorkspace)	
		m_containingWorkspace->close (*this);
	else
	{
		close();
		removeState(MS_openNextFrame);
	}
}

//----------------------------------------------------------------------

void CuiMediator::iconifyThroughWorkspace  ()
{
	if (m_containingWorkspace)	
		m_containingWorkspace->iconify (*this);
}

//----------------------------------------------------------------------

void CuiMediator::maximizeThroughWorkspace  ()
{
	if (isMaximized ())
	{
		if (m_containingWorkspace)	
			m_containingWorkspace->restore (*this);
		else
		{
			if (!restore ())
			{
				const UIRect & rect = getRestoreRect ();
				getPage ().SetRect (rect);
			}
		}
	}
	else
	{
		if (m_containingWorkspace)	
			m_containingWorkspace->maximize (*this);
		else
		{
			UIPage * const parent = dynamic_cast<UIPage *>(getPage ().GetParent ());
			if (parent)
			{
				if (!maximize ())
				{
					const UISize & size = parent->GetSize ();
					getPage ().SetSize (size);
					getPage ().SetLocation (UIPoint::zero);
				}
			}
		}
	}
}

//----------------------------------------------------------------------

void CuiMediator::setRestoreRect (const UIRect & rect)
{ 
	*m_restoreRect = rect;
}

//----------------------------------------------------------------------

void CuiMediator::registerMediatorObject (UIBaseObject & obj, bool activeCallbacks)
{
	obj.Attach (0);
	m_objectCallbackVector->push_back (ObjectCallbackData (&obj, activeCallbacks));
	if(isActive())
	{
		if(obj.IsA(TUIWidget))
		{
			UIWidget * const widget = safe_cast<UIWidget *>(&obj);
			if(widget)
			{
				UIEventCallback * const callback = dynamic_cast<UIEventCallback *>(this);
				if(callback)
					widget->AddCallback(callback);
			}
		}
	}
}

//----------------------------------------------------------------------

void CuiMediator::unregisterMediatorObject(UIBaseObject & obj)
{
	for (ObjectCallbackVector::iterator it = m_objectCallbackVector->begin (); it != m_objectCallbackVector->end (); ++it)
	{
		const ObjectCallbackData & ocd = *it;
		UIBaseObject * const ocdObj   = ocd.first;
		if(ocdObj == &obj)
		{
			m_objectCallbackVector->erase(it);
			obj.Detach(0);
			return;
		}
	}
	return;
}

//----------------------------------------------------------------------

bool CuiMediator::isRegisteredMediatorObject (UIBaseObject & obj)
{
	for (ObjectCallbackVector::iterator it = m_objectCallbackVector->begin (); it != m_objectCallbackVector->end (); ++it)
	{
		const ObjectCallbackData & ocd = *it;
		UIBaseObject * const ocdObj   = ocd.first;
		if(ocdObj == &obj)
			return true;
	}
	return false;
}

//----------------------------------------------------------------------

void CuiMediator::unregisterMediatorObjects()
{
	if (m_objectCallbackVector && !m_objectCallbackVector->empty())
	{
		std::vector<UIBaseObject *> objects;

		{
			objects.reserve(m_objectCallbackVector->size());
			for (ObjectCallbackVector::iterator it = m_objectCallbackVector->begin (); it != m_objectCallbackVector->end (); ++it)
			{
				const ObjectCallbackData & ocd = *it;
				UIBaseObject * const obj   = ocd.first;
				objects.push_back(obj);
			}
			m_objectCallbackVector->clear();
		}

		std::vector<UIBaseObject *>::iterator oi;
		for (oi=objects.begin();oi!=objects.end();++oi)
		{
			UIBaseObject * const obj = *oi;
			obj->Detach(0);
		}
	}
}

//----------------------------------------------------------------------

void CuiMediator::debugPrintMediators       (std::string & str)
{
	char buf [1024];
	const size_t bufsize = sizeof (buf);

	{
		for (MediatorVector::iterator it = s_mediators.begin (); it != s_mediators.end (); ++it)
		{
			CuiMediator * const mediator = NON_NULL (*it);
			snprintf (buf, bufsize, "          %40s ref=%2d active=%d\n", mediator->m_mediatorDebugName.c_str (), mediator->m_refcount, mediator->isActive ());
			str += buf;
		}
	}
	
	{
		for (MediatorVector::iterator it = s_mediatorsToAdd.begin (); it != s_mediatorsToAdd.end (); ++it)
		{
			CuiMediator * const mediator = NON_NULL (*it);
			snprintf (buf, bufsize, "(add)     %40s ref=%2d active=%d\n", mediator->m_mediatorDebugName.c_str (), mediator->m_refcount, mediator->isActive ());
			str += buf;
		}
	}
	
	for (MediatorVector::iterator it = s_mediatorsToAddNext.begin (); it != s_mediatorsToAddNext.end (); ++it)
	{
		CuiMediator * const mediator = NON_NULL (*it);
		snprintf (buf, bufsize, "(addNext) %40s ref=%2d active=%d\n", mediator->m_mediatorDebugName.c_str (), mediator->m_refcount, mediator->isActive ());
		str += buf;
	}

}

//----------------------------------------------------------------------

void CuiMediator::saveSettings () const
{
	if (m_popupHelpData)
		CuiSettings::saveBoolean (m_mediatorDebugName, Settings::popupHelpEnabled, hasState (MS_popupHelpOk));

	if (hasState (MS_settingsAutoSize))
	{
		const UISize & size = getPage ().GetSize ();
		CuiSettings::saveSize     (m_mediatorDebugName, size);
	}

	if (hasState (MS_settingsAutoLoc))
	{
		const UIPoint & loc = getPage ().GetLocation ();
		CuiSettings::saveLocation (m_mediatorDebugName, loc);
	}
}

//----------------------------------------------------------------------

void CuiMediator::loadSettings ()
{
	if (m_popupHelpData)
	{
		bool popupHelpOk = hasState (MS_popupHelpOk);
		CuiSettings::loadBoolean (m_mediatorDebugName, Settings::popupHelpEnabled, popupHelpOk);
		setOrRemoveState (MS_popupHelpOk, popupHelpOk);
	}

	loadSizeLocation (hasState (MS_settingsAutoSize), hasState (MS_settingsAutoLoc));
}

//----------------------------------------------------------------------

void CuiMediator::loadSizeLocation (bool doSize, bool doLocation)
{
	if (!doSize && !doLocation)
		return;
	
	UISize size  = getPage ().GetSize ();
	UIPoint loc  = getPage ().GetLocation ();
	
	if (doSize)
		CuiSettings::loadSize (m_mediatorDebugName, size);
	
	if (doLocation)
		CuiSettings::loadLocation (m_mediatorDebugName, loc);
	
	UIRect rect (loc, size);	
	
//	if (m_containingWorkspace)
//		m_containingWorkspace->autoPositionMovingRect (&getPage (), rect, true);
//	else
		getPage ().SetRect (rect);
}

//----------------------------------------------------------------------

void CuiMediator::setSettingsAutoSizeLocation (bool size, bool location)
{
	setOrRemoveState (MS_settingsAutoSize, size);
	setOrRemoveState (MS_settingsAutoLoc,  location);
}

//----------------------------------------------------------------------

inline bool CuiMediator::getStartSize (UISize & size)
{
	if (hasState (MS_hasStartSize))
	{
		size = m_startSize;
		return true;
	}

	return false;
}

//----------------------------------------------------------------------

void CuiMediator::setPointerToggledOn    (bool b)
{
	CuiManager::setPointerToggledOn (b);
}

//----------------------------------------------------------------------

void CuiMediator::onPopupHelpClose(const CuiPopupHelp & popupHelp, bool okNext)
{
	if (!isActive())
		return;

	if (m_popupHelp == &popupHelp)
	{
		m_popupHelp->release ();
		m_popupHelp = 0;
		
		removeState(MS_popupHelpOk);

		if (okNext)
		{
			++m_popupHelpIndex;

			if (m_popupHelpIndex >= 0 && m_popupHelpData != NULL && m_popupHelpIndex < static_cast<int>(m_popupHelpData->size ()))
			{
				setState(MS_popupHelpOk);

				const CuiPopupHelpData & cphd = (*m_popupHelpData) [m_popupHelpIndex];
				UIPage * const page = dynamic_cast<UIPage *>(getPage ().GetParentWidget ());
				NOT_NULL (page);
				
				m_popupHelp = safe_cast<CuiPopupHelp *>(CuiMediatorFactory::activateInWorkspace(CuiMediatorTypes::PopupHelp, true, false));
				m_popupHelp->fetch();
				m_popupHelp->setData(*this, cphd);
			}
		}

		if (m_checkboxPopupHelp)
			m_checkboxPopupHelp->SetChecked(hasState(MS_popupHelpOk), false);

		saveSettings();
	}
}

//----------------------------------------------------------------------

void CuiMediator::startPopupHelp()
{
	stopPopupHelp();

	if (!getContainingWorkspace ())
		return;

	m_popupHelpIndex = 0;

	if (m_popupHelpData && !m_popupHelpData->empty())
	{
		setState (MS_popupHelpOk);

		if (m_checkboxPopupHelp)
			m_checkboxPopupHelp->SetChecked(true, false);

		const CuiPopupHelpData & cphd = (*m_popupHelpData) [m_popupHelpIndex];
		UIPage * const page = dynamic_cast<UIPage *>(getPage ().GetParentWidget ());
		NOT_NULL (page);

		m_popupHelp = safe_cast<CuiPopupHelp *>(CuiMediatorFactory::activateInWorkspace(CuiMediatorTypes::PopupHelp, true, false));
		m_popupHelp->fetch ();
		m_popupHelp->setData(*this, cphd);
	}
}

//----------------------------------------------------------------------

void CuiMediator::stopPopupHelp()
{
	removeState (MS_popupHelpOk);
	
	if (m_checkboxPopupHelp)
		m_checkboxPopupHelp->SetChecked(false, false);

	if (m_popupHelp)
	{
		//-- this should trigger a call to CuiMediator::onPopupHelpClose () and release the popup ptr
		m_popupHelp->closeThroughWorkspace();

		//-- just in case
		if (m_popupHelp)
		{
			m_popupHelp->release();
			m_popupHelp = 0;
		}

		m_lastRect = getPage ().GetRect ();
	}

	if (getSettingsEnabled())
	{
		saveSettings ();
	}
}

//----------------------------------------------------------------------

void CuiMediator::updatePopupHelp (float )
{
	if (m_popupHelp)
	{
		const UIRect & rect = getPage ().GetRect ();
		if (rect != m_lastRect)
		{
			m_popupHelp->autoPosition ();
			m_lastRect = rect;
		}
	}
}

//----------------------------------------------------------------------

void CuiMediator::raisePopupHelp ()
{
	if (m_popupHelp)
	{
		UIPage * const parent = dynamic_cast<UIPage *>(m_popupHelp->getPage ().GetParent ());
		if (parent)
			parent->MoveChild (&m_popupHelp->getPage (), UIBaseObject::Top);
	}
}

//----------------------------------------------------------------------

UIBaseObject * CuiMediator::getCodeDataObject (const UIData * theData, UITypeID id, const char * name, bool optional) const
{
	return getCodeDataObject(NULL, theData, id, name, optional);
}

//----------------------------------------------------------------------

UIBaseObject * CuiMediator::getCodeDataObject (UIPage *rootPage, const UIData * theData, UITypeID id, const char * name, bool optional) const
{
	if (!theData)
	{
		FATAL (!optional, ("CuiMediator no code data for [%s]", m_thePage.GetFullPath ().c_str ()));
		return 0;
	}

	NOT_NULL (name);
	
	if(!rootPage)
		rootPage = &m_thePage;

	UIBaseObject * result = 0;

	UINarrowString path;
	
	if (theData->GetPropertyNarrow (UILowerString (name), path))
	{
		result = rootPage->GetObjectFromPath (path.c_str ());
		
		if (!result)
		{
			//-- a bad path should just warning if optional
			FATAL   (!optional, ("Unable to find CodeData object '%s' [%s] from [%s] for [%s]\n", name, path.c_str (), rootPage->GetFullPath ().c_str (), m_mediatorDebugName.c_str ()));
			WARNING (optional,  ("Unable to find CodeData object '%s' [%s] from [%s] for [%s]",   name, path.c_str (), rootPage->GetFullPath ().c_str (), m_mediatorDebugName.c_str ()));
			
			return 0;
		}
		
		if (result && !result->IsA (id))
		{
			FATAL (true,  ("CodeData object request type mismatch from '%s'. Requested '%s', type %d [%s], found type %s\n",
				rootPage->GetFullPath ().c_str (), name, id, path.c_str (), result->GetTypeName ()));
			result = 0;
		}
	}
	else
	{
		result = rootPage->GetChild (name);
		FATAL   (!result && !optional, ("Unable to find CodeData property '%s' from [%s] for [%s]\n", name, m_thePage.GetFullPath ().c_str (), m_mediatorDebugName.c_str ()));
	}


	return result;
}

//----------------------------------------------------------------------

void CuiMediator::setMediatorDebugName   (const std::string & mediatorDebugName)
{
	m_mediatorDebugName = mediatorDebugName;
}

//----------------------------------------------------------------------

void CuiMediator::closeNextFrame         ()
{
	removeState (MS_openNextFrame);
	setState (MS_closeNextFrame);
	deactivate ();
}

//----------------------------------------------------------------------

void CuiMediator::open ()
{
	CuiWorkspace * workspace = m_containingWorkspace;
	if (!workspace)
		workspace = CuiWorkspace::getGameWorkspace ();

	if (workspace)
	{
		if (!workspace->hasMediator (*this))
			workspace->addMediator (*this);

		workspace->focusMediator (*this, true);
	}

	activate ();

	CuiMessageBox::ensureFocus  ();
}

//----------------------------------------------------------------------

void CuiMediator::openNextFrame ()
{
	removeState (MS_closeNextFrame);
	setState (MS_openNextFrame);
}

//----------------------------------------------------------------------

void CuiMediator::setAssociatedObjectId (const NetworkId & id)
{
	*m_associatedObjectId = id;
	if (isActive ())
	{
		if (m_maxRangeFromObject > 0.0f)
		{
			if (m_associatedObjectId->isValid () || (m_associatedLocation.x < maxSaneCoordinate))
				setIsUpdating (true);
		}
	}
}

//----------------------------------------------------------------------

void CuiMediator::setAssociatedLocation (const Vector & location)
{
	m_associatedLocation = location;
	if (isActive ())
	{
		if (m_maxRangeFromObject > 0.0f)
		{
			if (m_associatedObjectId->isValid () || (m_associatedLocation.x < maxSaneCoordinate))
				setIsUpdating (true);
		}
	}
}

//----------------------------------------------------------------------

void CuiMediator::setMaxRangeFromObject (float maxRange)
{
	m_maxRangeFromObject = maxRange;
	if (isActive ())
	{
		if (m_maxRangeFromObject > 0.0f)
		{
			if (m_associatedObjectId->isValid () || (m_associatedLocation.x < maxSaneCoordinate))
				setIsUpdating (true);
		}
	}
}

//----------------------------------------------------------------------

void CuiMediator::handleMediatorPropertiesChanged ()
{

}

//----------------------------------------------------------------------

void CuiMediator::resetToDefaultSizeAndLocation     ()
{
	UIRect rect (m_defaultLocation, m_defaultSize);
	getPage ().SetRect (rect);
}

//----------------------------------------------------------------------

const UIComposite & CuiMediator::getComposite () const
{
	FATAL(!m_thePage.IsA(TUIComposite), ("[%s] is not a composite!", m_thePage.GetName().c_str()));

	return *safe_cast<UIComposite *>(&m_thePage);
}

//----------------------------------------------------------------------

UIComposite & CuiMediator::getComposite ()
{
	FATAL(!m_thePage.IsA(TUIComposite), ("[%s] is not a composite!", m_thePage.GetName().c_str()));

	return *safe_cast<UIComposite *>(&m_thePage);
}

//----------------------------------------------------------------------

void CuiMediator::autoRegisterWidgets()
{
	UIBaseObject::UIObjectList widgets;
	getAutoRegisterObjects(getPage(), widgets);

	UIBaseObject::UIObjectList::iterator const itEnd = widgets.end();
	for (UIBaseObject::UIObjectList::iterator it = widgets.begin(); it != itEnd; ++it)
	{
		UIBaseObject * const obj = *it;
		registerMediatorObject(*obj, true);
	}
}

//----------------------------------------------------------------------

void CuiMediator::autoUnregisterWidgets()
{
	UIBaseObject::UIObjectList widgets;
	getAutoRegisterObjects(getPage(), widgets);

	UIBaseObject::UIObjectList::iterator const itEnd = widgets.end();
	for (UIBaseObject::UIObjectList::iterator it = widgets.begin(); it != itEnd; ++it)
	{
		UIBaseObject * const obj = *it;
		unregisterMediatorObject(*obj);
	}
}

//======================================================================
