// ======================================================================
//
// SwgCuiOptKeymap.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiOptKeymap.h"

#include "UIButton.h"
#include "UICheckbox.h"
#include "UIComboBox.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UITabbedPane.h"
#include "UITable.h"
#include "UITableModelDefault.h"
#include "UnicodeUtils.h"
#include "clientGame/ClientMacroManager.h"
#include "clientGame/FreeChaseCamera.h"
#include "clientGame/Game.h"
#include "clientGame/GroundScene.h"
#include "clientGame/InputScheme.h"
#include "clientUserInterface/CuiControlsMenuBindEntry.h"
#include "clientUserInterface/CuiInputNames.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiSettings.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiStringIdsOptions.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "clientUserInterface/CuiWorkspace.h"
#include "sharedFoundation/Production.h"
#include "sharedInputMap/InputMap.h"
#include "sharedInputMap/InputMap_Command.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedUtility/Callback.h"
#include "sharedUtility/CallbackReceiver.h"
#include "sharedUtility/CurrentUserOptionManager.h"
#include "sharedUtility/LocalMachineOptionManager.h"
#include <list>
#include <map>
#include <vector>

// ======================================================================

namespace SwgCuiOptKeymapNamespace
{
	namespace Categories
	{
		const std::string all      = "all";
		const std::string custom   = "custom";
		const std::string internal = "internal";
	}

	const std::string s_pointerToggleCommand = "CMD_uiPointerToggle";

	const std::string s_optStringFileName = "ui_opt";

}

using namespace SwgCuiOptKeymapNamespace;

//----------------------------------------------------------------------

class SwgCuiOptKeymap::InputSchemeCallbackReceiver : public CallbackReceiver 
{
public:
	void performCallback () 
	{
		NON_NULL(menu)->onInputSchemeReset ();
		NON_NULL(menu)->doApply ();
	}
	
	InputSchemeCallbackReceiver  () : 
	CallbackReceiver(),
	menu(NULL)
	{}

	~InputSchemeCallbackReceiver () 
	{ 
		menu = NULL;
	}
	
	SwgCuiOptKeymap * menu;  //lint !e1925 //public data member
};

//-----------------------------------------------------------------

SwgCuiOptKeymap::SwgCuiOptKeymap (UIPage & page, Game::SceneType sceneType) :
SwgCuiOptBase ("SwgCuiOptChat", page),
m_buttonDelete                (0),
m_buttonRebind                (0),
m_entryPage                   (0),
m_bindEntry                   (0),
m_workingInputMap            (0),
m_originalInputMap			 (0),
m_tableBinds                  (0),
m_tableModelBinds             (0),
m_comboPreset                 (0),
m_tabBinds                    (0),
m_checkMouseMode              (0),
m_checkModalChat              (0),
m_messageBoxModalChatConfirm  (0),
m_checkChaseCam               (0),
m_checkTurnStrafes            (0),
m_checkCanFireSecondariesFromToolbar (0),
m_middleMouseDrivesMovement   (0),
m_mouseLeftAndRightDrivesMovement   (0),
m_scrollThroughDefaultActions (0),
m_callbackReceiver            (new InputSchemeCallbackReceiver),
m_lastSelectedCategory        (),
m_sceneType                   (sceneType)
{
	m_callbackReceiver->menu = this;

	getCodeDataObject (TUIButton,      m_buttonDelete,         "buttonDelete",         true);
	getCodeDataObject (TUIButton,      m_buttonRebind,         "buttonRebind",         true);
	getCodeDataObject (TUITable,       m_tableBinds,           "tableBinds",           true);
	getCodeDataObject (TUIComboBox,    m_comboPreset,          "comboPreset",          true);
	getCodeDataObject (TUITabbedPane,  m_tabBinds,             "tabBinds",             true);
	getCodeDataObject (TUICheckbox,    m_checkMouseMode,       "checkMouseMode",       true);
	getCodeDataObject (TUICheckbox,    m_checkChaseCam,        "checkChaseCam",        true);
	getCodeDataObject (TUICheckbox,    m_checkTurnStrafes,     "checkTurnStrafes",     true);
	getCodeDataObject (TUICheckbox,    m_checkCanFireSecondariesFromToolbar,"checkCanFireSecondariesFromToolbar", true);
	getCodeDataObject (TUICheckbox,    m_middleMouseDrivesMovement, "middleMouseDrivesMovement",                 true);
	getCodeDataObject (TUICheckbox,    m_mouseLeftAndRightDrivesMovement, "mouseLeftAndRightDrivesMovement",     true);
	getCodeDataObject (TUICheckbox,    m_scrollThroughDefaultActions, "scrollThroughDefaultActions",             true);
	getCodeDataObject (TUICheckbox,    m_checkModalChat,       "checkModalChat",       true);

	if (m_tableBinds)
		m_tableModelBinds = dynamic_cast<UITableModelDefault *>(m_tableBinds->GetTableModel ());
	if (m_tableModelBinds)
		m_tableModelBinds->ClearData ();

	getCodeDataObject (TUIPage,       m_entryPage,  "EntryPage", true);

	if (m_entryPage)
		m_entryPage->SetVisible (false);

	connectToMessage (CuiControlsMenuBindEntry::Messages::UPDATE_BINDING);

	if (m_buttonDelete)                          registerMediatorObject (*m_buttonDelete,           true);
	if (m_buttonRebind)                          registerMediatorObject (*m_buttonRebind,           true);
	if (m_tableBinds)                            registerMediatorObject (*m_tableBinds,             true);
	if (m_comboPreset)                           registerMediatorObject (*m_comboPreset,            true);
	if (m_tabBinds)                              registerMediatorObject (*m_tabBinds,               true);
	if (m_checkMouseMode)                        registerMediatorObject (*m_checkMouseMode,         true);
	if (m_checkChaseCam)                         registerMediatorObject (*m_checkChaseCam,          true);
	if (m_checkTurnStrafes)                      registerMediatorObject (*m_checkTurnStrafes,       true);
	if (m_checkCanFireSecondariesFromToolbar)    registerMediatorObject (*m_checkCanFireSecondariesFromToolbar, true);
	if (m_middleMouseDrivesMovement)             registerMediatorObject (*m_middleMouseDrivesMovement, true);
	if (m_mouseLeftAndRightDrivesMovement)       registerMediatorObject (*m_mouseLeftAndRightDrivesMovement, true);
	if (m_scrollThroughDefaultActions)           registerMediatorObject (*m_scrollThroughDefaultActions, true);
	if (m_checkModalChat)                        registerMediatorObject (*m_checkModalChat,         true);

	registerMediatorObject (getPage (),                true);

	m_callback->connect(*this, &SwgCuiOptKeymap::onSceneChange, static_cast<Game::Messages::SceneChanged*>(0));

	setState (MS_closeable);
}

//-----------------------------------------------------------------

SwgCuiOptKeymap::~SwgCuiOptKeymap ()
{	

	delete m_callbackReceiver;
	m_callbackReceiver = 0;

	m_entryPage    = 0;
	m_bindEntry    = 0;

	m_workingInputMap = 0;

	if (m_originalInputMap)
	{
		delete m_originalInputMap;
	}

	m_originalInputMap = 0;
	m_tableModelBinds = NULL;
	m_checkMouseMode = 0;
	m_checkChaseCam = 0;
	m_checkTurnStrafes = 0;
	m_checkCanFireSecondariesFromToolbar = 0;
	m_middleMouseDrivesMovement = 0;
	m_mouseLeftAndRightDrivesMovement = 0;
	m_scrollThroughDefaultActions = 0;
	m_messageBoxModalChatConfirm = NULL;
}

//-----------------------------------------------------------------

void SwgCuiOptKeymap::performActivate ()
{
	m_workingInputMap = Game::getGameInputMap ();

	if(!m_originalInputMap && m_workingInputMap)
		m_originalInputMap = NON_NULL (m_workingInputMap->getRebindingMap ());

	CuiManager::requestPointer (true);

	resetTabs    ();
	resetPresets ();
	resetChecks  ();
	updateData   ();

	if (m_tableModelBinds)
		m_tableModelBinds->sortOnColumn (0, UITableModel::SD_up);
	
	Callback * const callback = InputScheme::getResetCallback ();
	if (callback)
		callback->attachReceiver (*m_callbackReceiver);
}

//----------------------------------------------------------------------

void SwgCuiOptKeymap::revert()
{
	if (m_originalInputMap && m_workingInputMap)
	{
		m_workingInputMap->applyRebindingMapChanges (*m_originalInputMap);
		
		if (!m_workingInputMap->write (0, false))
		{
			// TODO: localize
			CuiMessageBox::createInfoBox (Unicode::narrowToWide ("Unable to save configuration.\n"));			
			return;
		}

		CuiSettings::save();
	}
	
	CuiPreferences::signalKeybindingsChanged ();
}

//-----------------------------------------------------------------

void SwgCuiOptKeymap::performDeactivate ()
{
	CuiManager::requestPointer (false);

	if (m_tableModelBinds)
		m_tableModelBinds->ClearData ();
	if (m_bindEntry)
	{
		m_bindEntry->deactivate ();
		m_bindEntry->release ();
		m_bindEntry = 0;
	}
	
	if (m_messageBoxModalChatConfirm)
 		m_messageBoxModalChatConfirm->closeMessageBox ();
	Callback * const callback = InputScheme::getResetCallback ();
	if (callback)
		callback->detachReceiver (*m_callbackReceiver);

	LocalMachineOptionManager::save ();
	CurrentUserOptionManager::save ();

}

//----------------------------------------------------------------------
   
bool SwgCuiOptKeymap::close ()
{
	deactivate ();
	if(m_originalInputMap)
	{
		delete m_originalInputMap;
		m_originalInputMap = 0;
	}
	return true;
}

//----------------------------------------------------------------------

void SwgCuiOptKeymap::onSceneChange(bool const &)
{
	revert();
	close();
}

//-----------------------------------------------------------------

void SwgCuiOptKeymap::OnButtonPressed           ( UIWidget *context )
{
	if (context == m_buttonResetDefaults)
	{	
		resetDefaults();
	}
	else if (context == m_buttonDelete)
	{
		if (!m_tableModelBinds || !m_tableBinds)
			return;
		const UIData * const data = m_tableModelBinds->GetCellDataVisual (m_tableBinds->GetLastSelectedRow (), 0);
		if (!data)
			return;

		const std::string & commandName = data->GetName ();
		const InputMap::Command * const cmd = NON_NULL(m_workingInputMap)->findCommandByName (commandName.c_str ());
		NOT_NULL (cmd);

		if (cmd->name == s_pointerToggleCommand)
		{
			const Unicode::String & localizedCommandName = CuiInputNames::getLocalizedCommandName (cmd->name);
			Unicode::String result;
			CuiStringVariablesManager::process (CuiStringIds::bind_delete_cannot_delete_prose, Unicode::emptyString, localizedCommandName, Unicode::emptyString, result);
			CuiMessageBox::createInfoBox (result);
			return;
		}
		NON_NULL(m_workingInputMap)->removeBindings (cmd);
		updateData ();
		IGNORE_RETURN(doApply ());
		CuiPreferences::signalKeybindingsChanged ();
		m_workingInputMap->handleInputReset();
	}
	else if (context == m_buttonRebind)
	{
		if (!m_tableModelBinds || !m_tableBinds || !m_entryPage)
			return;
		const UIData * const data = m_tableModelBinds->GetCellDataVisual (m_tableBinds->GetLastSelectedRow (), 0);
		if (!data)
			return;

		const std::string & commandName = data->GetName ();
		const InputMap::Command * const cmd = NON_NULL(m_workingInputMap)->findCommandByName (commandName.c_str ());
		NOT_NULL (cmd);

		//-- release-safe: if the selected row maps to no command, don't open the
		//-- bind dialog with a null command (would crash in doAddBinding)
		if (!cmd)
			return;

		if (!m_bindEntry)
		{
			m_bindEntry = new CuiControlsMenuBindEntry (*m_entryPage);
			m_bindEntry->fetch ();
		}

		NOT_NULL (m_bindEntry);
		m_bindEntry->activate ();
		m_bindEntry->setCommand (m_workingInputMap, cmd);

		if (getContainingWorkspace ())
			getContainingWorkspace ()->cancelFocusForMediator (*this);
	}
}

//-----------------------------------------------------------------

bool SwgCuiOptKeymap::OnMessage ( UIWidget *context, const UIMessage & msg )
{
	if (context == m_tableBinds)
	{
		if (msg.Type == UIMessage::KeyDown)
		{
			if (msg.Keystroke == UIMessage::Delete)
			{
				if (m_buttonDelete)
					m_buttonDelete->Press ();
				return false;
			}
		}
		else if (msg.Type == UIMessage::LeftMouseDoubleClick)
		{
			getPage ().ReleaseMouseLock (UIPoint::zero);
			if (m_buttonRebind)
				m_buttonRebind->Press ();
			return false;
		}
		else if (msg.Type == UIMessage::Character && msg.Keystroke == ' ')
		{
			getPage ().ReleaseMouseLock (UIPoint::zero);
			if (m_buttonRebind)
				m_buttonRebind->Press ();
			return false;
		}
	}

	return true;
}

//----------------------------------------------------------------------

void SwgCuiOptKeymap::OnGenericSelectionChanged     (UIWidget * context)
{
	if (context == m_tableBinds && m_tableBinds)
	{
		if (m_buttonDelete)
			m_buttonDelete->SetEnabled (m_tableBinds->GetLastSelectedRow () >= 0);
		if (m_buttonRebind)
			m_buttonRebind->SetEnabled (m_tableBinds->GetLastSelectedRow () >= 0);
	}
	else if (context == m_comboPreset && m_comboPreset)
	{
		const int selectedIndex = m_comboPreset->GetSelectedIndex ();
		
		if (selectedIndex >= 0)
		{
			const UIData * const data = m_comboPreset->GetDataAtIndex (selectedIndex);
			if (data)
			{
				const std::string & schemeName = data->GetName ();
				IGNORE_RETURN(InputScheme::resetFromType (schemeName, false));
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiOptKeymap::OnTabbedPaneChanged    (UIWidget * context)
{
	if (context == m_tabBinds)
	{
		updateData ();
		if (m_tableModelBinds)
			m_tableModelBinds->sortOnColumn (0, UITableModel::SD_up);
	}
}

//----------------------------------------------------------------------

void SwgCuiOptKeymap::OnCheckboxSet                 (UIWidget * context)
{
	NOT_NULL (context);
	handleCheck (*safe_cast<UICheckbox *>(context));
}

//----------------------------------------------------------------------

void SwgCuiOptKeymap::OnCheckboxUnset               (UIWidget * context)
{
	NOT_NULL (context);
	handleCheck (*safe_cast<UICheckbox *>(context));
}

//----------------------------------------------------------------------

void SwgCuiOptKeymap::handleCheck  (UICheckbox & box)
{
	if (m_checkMouseMode == &box)
	{
		CuiPreferences::setMouseModeDefault (box.IsChecked ());
	}
	else if (m_checkModalChat == &box)
  	{
 		if (m_messageBoxModalChatConfirm)
 			m_messageBoxModalChatConfirm->closeMessageBox ();
 
 		const bool b = CuiPreferences::getModalChat ();
 		box.SetChecked (b, false);
 
 		if (b)
		{
 			m_messageBoxModalChatConfirm = CuiMessageBox::createYesNoBox (CuiStringIdsOptions::confirm_modeless_chat.localize ());
 			m_callback->connect (m_messageBoxModalChatConfirm->getTransceiverClosed (), *this, &SwgCuiOptKeymap::onModalChatConfirmClosed);
		}
		else
		{			
			CuiPreferences::setModalChat (!CuiPreferences::getModalChat ());
			m_checkModalChat->SetChecked (CuiPreferences::getModalChat (), false);
		}
 	}
	else if (m_checkTurnStrafes == &box)
	{
		CuiPreferences::setTurnStrafesDuringMouseModeToggle (box.IsChecked ());
	}
	else if (m_checkCanFireSecondariesFromToolbar == &box)
	{
		CuiPreferences::setCanFireSecondariesFromToolbar (box.IsChecked ());
	}
	else if (m_middleMouseDrivesMovement == &box)
	{
		CuiPreferences::setMiddleMouseDrivesMovementToggle(box.IsChecked());
	}
	else if (m_mouseLeftAndRightDrivesMovement == &box)
	{
		CuiPreferences::setMouseLeftAndRightDrivesMovementToggle(box.IsChecked());
	}
	else if (m_scrollThroughDefaultActions == &box)
	{
		CuiPreferences::setScrollThroughDefaultActions(box.IsChecked());
	}
	else if (m_checkChaseCam == &box)
	{
		FreeChaseCamera::setCameraMode (box.IsChecked () ? FreeChaseCamera::CM_chase : FreeChaseCamera::CM_free);
	}
}

//-----------------------------------------------------------------

void SwgCuiOptKeymap::updateData ()
{
	if (!m_tableModelBinds || !m_tableBinds)
		return;

	if (m_buttonDelete) m_buttonDelete->SetEnabled (false);
	if (m_buttonRebind) m_buttonRebind->SetEnabled (false);

	std::string lastSelectedCommand;
	UIPoint lastScrolledToPoint;

	{
		const UIData * const data = m_tableModelBinds->GetCellDataVisual (m_tableBinds->GetLastSelectedRow (), 0);
		if (data)
		{
			lastSelectedCommand = data->GetName ();
		}

		lastScrolledToPoint = m_tableBinds->GetScrollLocation ();

	}

	m_tableModelBinds->ClearData ();

	if (m_workingInputMap == 0)
		return;

	m_tableModelBinds->Attach (0);
	m_tableBinds->SetTableModel (0);

	// Fall back to "all" category when there's no tabBinds widget (NGE-retail
	// UI doesn't ship a keymap page so we built one without category tabs).
	std::string category = Categories::all;
	if (m_tabBinds)
	{
		const UIData * const activeTabData = m_tabBinds->GetTabData(m_tabBinds->GetActiveTab ());
		if (activeTabData)
			category = activeTabData->GetName ();
	}
	m_lastSelectedCategory = category;

	const bool all    = category == Categories::all;
	const bool custom = !all && category == Categories::custom;

	InputMap::CommandBindInfoSet * cmdsBindInfoSets;
	const InputMap::Shifts & shifts = m_workingInputMap->getShifts ();
	CuiInputNames::setInputShifts (shifts);

	const uint32 numCmds           = m_workingInputMap->getCommandBindings (cmdsBindInfoSets);

	if (cmdsBindInfoSets == 0)
		return;

	for (uint32 i = 0; i < numCmds; ++i)
	{
		const InputMap::CommandBindInfoSet & cbis = cmdsBindInfoSets [i];
		NOT_NULL (cbis.cmd);
		
		if (!all)
		{
			//-- for now, put all user defined commands in "custom"
			if (!(custom && cbis.cmd->userDefined) && category != cbis.cmd->category)
				continue;
		}
		else
		{
			if (cbis.cmd->category == Categories::internal ||
				cbis.cmd->category == Categories::custom)
				continue;
		}
		// don't show voice commands
		if (cbis.cmd->category == "voice") {
			continue;
		}
		
		Unicode::String localizedCommandName;
		Unicode::String localizedCommandDesc;

		if (custom)
		{
			const ClientMacroManager::Data * const data = ClientMacroManager::findMacroData (cbis.cmd->name);
			if (data)
			{
				localizedCommandName =  Unicode::narrowToWide (data->userDefinedName);
				localizedCommandDesc =  Unicode::narrowToWide (data->commandString);
			}
			else
			{
				localizedCommandName =  Unicode::narrowToWide ("ERROR LOADING MACRO");
			}
		}
		else
		{
			localizedCommandName =  CuiInputNames::getLocalizedCommandName (cbis.cmd->name);
			localizedCommandDesc =  CuiInputNames::getLocalizedCommandDesc (cbis.cmd->name);
		}

		UIData * const cellData = m_tableModelBinds->AppendCell (0, cbis.cmd->name.c_str (), localizedCommandName);
		NOT_NULL (cellData);
		IGNORE_RETURN(cellData->SetProperty (UIWidget::PropertyName::LocalTooltip, localizedCommandDesc));
		
		Unicode::String bindStr;
		
		if (cbis.numBinds != 0)
			CuiInputNames::appendInputString (cbis, bindStr);
		
		UIData * const bindCellData = m_tableModelBinds->AppendCell (1, cbis.cmd->name.c_str (), bindStr);
		NOT_NULL (bindCellData);
		IGNORE_RETURN(bindCellData->SetProperty (UIWidget::PropertyName::LocalTooltip, bindStr));
	}

	delete [] cmdsBindInfoSets;
	cmdsBindInfoSets = 0;

	m_tableBinds->SetTableModel (m_tableModelBinds);
	m_tableModelBinds->Detach (0);
	
	//-- select the previously highlighed command, if possible
	
	{
		bool found = false;
		
		if (!lastSelectedCommand.empty ())
		{
			const int rowCount = m_tableModelBinds->GetRowCount ();
			
			for (int row = 0; row < rowCount; ++row)
			{
				found = true;
				
				UIData * const data = m_tableModelBinds->GetCellDataVisual (row, 0);
				NOT_NULL (data);
				if (data->GetName () == lastSelectedCommand)
				{
					m_tableBinds->SelectRow(row);
					found = true;
					m_tableBinds->ScrollToPoint (lastScrolledToPoint);
					break;
				}
			}
		}
		
		if (!found)
		{
			m_tableBinds->SelectRow (-1);
		}
		
		if (lastSelectedCommand.empty ())
			m_tableBinds->ScrollToPoint (UIPoint::zero);
		else
			m_tableBinds->ScrollToPoint (lastScrolledToPoint);
		
	}

	if (m_buttonDelete) m_buttonDelete->SetEnabled   (m_tableBinds->GetLastSelectedRow () >= 0);
	if (m_buttonRebind) m_buttonRebind->SetEnabled   (m_tableBinds->GetLastSelectedRow () >= 0);
}

//-----------------------------------------------------------------

/**
* Attempts to apply changes and save to file.  Opens a dialog box on failure.
* @return false on failure
*/

bool SwgCuiOptKeymap::doApply ()
{

	if (m_workingInputMap)
	{
		if (!m_workingInputMap->write (0, false))
		{
			// TODO: localize
			CuiMessageBox::createInfoBox (Unicode::narrowToWide ("Unable to save configuration.\n"));			
			return false;
		}

		CuiSettings::save();
	}

	return true;
}

//-----------------------------------------------------------------

void SwgCuiOptKeymap::receiveMessage(const MessageDispatch::Emitter & , const MessageDispatch::MessageBase & message)
{
	if (message.isType (CuiControlsMenuBindEntry::Messages::UPDATE_BINDING) &&
		Game::getHudSceneType() == m_sceneType)
	{
		updateData ();
		IGNORE_RETURN(doApply ());
		CuiPreferences::signalKeybindingsChanged ();
	}
}

//----------------------------------------------------------------------

void SwgCuiOptKeymap::resetTabs ()
{
	if (!m_tabBinds)
		return;

	m_tabBinds->Clear ();

	if (!m_workingInputMap)
		return;

	int selectedIndex = 0;
	const InputMap::StringVector & sv = m_workingInputMap->getCommandCategories ();

	int index = 0;
	for (InputMap::StringVector::const_iterator it = sv.begin (); it != sv.end (); ++it)
	{
		const std::string & category = *it;

#if PRODUCTION == 1
		if (category == Categories::internal)
			continue;
#endif
		// do not show anyone the "voice" keybind tab of options
		// this should technically be removed from the input/inputmap_*.iff files, but they are mIFF source not datatables,
		// and they were not included in the leak, so they need a bit of reverse engineering, so this will do for now
		if (category == "voice") {
			continue;
		}

		std::string categoryIdName = "keybind_category_";
		categoryIdName += category;
		StringId categoryId(s_optStringFileName, categoryIdName);
		const Unicode::String & localizedCategory = categoryId.localize();
		UIData * const data  = m_tabBinds->AppendTab (localizedCategory, 0);
		NOT_NULL (data);
		data->SetProperty(UITabbedPane::DataProperties::Text, localizedCategory);
		data->SetName (category);

		if (category == m_lastSelectedCategory)
			selectedIndex = index;

		++index;
	}

	m_tabBinds->SetActiveTab (-1);
	m_tabBinds->SetActiveTab (selectedIndex);
}

//----------------------------------------------------------------------

void SwgCuiOptKeymap::resetPresets ()
{
	if (!m_comboPreset)
		return;

	InputScheme::StringVector sv;
	InputScheme::getTypes (sv);

	UIDataSource * const dataSource = m_comboPreset->GetDataSource();
	if (!dataSource)
		return;

	dataSource->Clear();

	std::string const & lastInputScheme = InputScheme::getLastInputSchemeType ();

	int selectComboIndex = 0;
	for (InputScheme::StringVector::const_iterator it = sv.begin (); it != sv.end (); ++it)
	{
		const std::string & schemeName = *it;

		if (!dataSource->GetChild (schemeName.c_str ()))
		{
			const Unicode::String & localizedSchemeName = InputScheme::localizeTypeName (schemeName);
			m_comboPreset->AddItem (localizedSchemeName, schemeName);
		}

		if (lastInputScheme == schemeName)
		{
			//-- find position of child
			UIDataList const & dataList = dataSource->GetData ();
			int index = 0;
			for (UIDataList::const_iterator dit = dataList.begin (); dit != dataList.end (); ++dit, ++index)
			{
				UIData const * const data = *dit;
				if (data && data->GetName () == schemeName)
				{
					selectComboIndex = index;
					break;
				}
			}
		}
	}

	if (m_comboPreset)
		m_comboPreset->SetSelectedIndex (selectComboIndex);
}

//----------------------------------------------------------------------

void SwgCuiOptKeymap::resetChecks ()
{
	bool const enabled = !CuiPreferences::getUseModelessInterface();

	if (m_checkMouseMode)                       { m_checkMouseMode->SetChecked(CuiPreferences::getMouseModeDefault (), false); m_checkMouseMode->SetEnabled(enabled); }
	if (m_checkModalChat)                       { m_checkModalChat->SetChecked(CuiPreferences::getModalChat (), false); m_checkModalChat->SetEnabled(enabled); }
	if (m_checkChaseCam)                        { m_checkChaseCam->SetChecked(FreeChaseCamera::getCameraMode () == FreeChaseCamera::CM_chase, false); m_checkChaseCam->SetEnabled(enabled); }
	if (m_checkTurnStrafes)                     { m_checkTurnStrafes->SetChecked(CuiPreferences::getTurnStrafesDuringMouseModeToggle (), false); m_checkTurnStrafes->SetEnabled(enabled); }
	if (m_checkCanFireSecondariesFromToolbar)   { m_checkCanFireSecondariesFromToolbar->SetChecked(CuiPreferences::getCanFireSecondariesFromToolbar (), false); m_checkCanFireSecondariesFromToolbar->SetEnabled(enabled); }
	if (m_middleMouseDrivesMovement)            { m_middleMouseDrivesMovement->SetChecked(CuiPreferences::getMiddleMouseDrivesMovementToggle(), false); m_middleMouseDrivesMovement->SetEnabled(enabled); }
	if (m_mouseLeftAndRightDrivesMovement)      { m_mouseLeftAndRightDrivesMovement->SetChecked(CuiPreferences::getMouseLeftAndRightDrivesMovementToggle(), false); m_mouseLeftAndRightDrivesMovement->SetEnabled(enabled); }
	if (m_scrollThroughDefaultActions)          { m_scrollThroughDefaultActions->SetChecked(CuiPreferences::getScrollThroughDefaultActions(), false); m_scrollThroughDefaultActions->SetEnabled(enabled); }
}

//----------------------------------------------------------------------

void SwgCuiOptKeymap::onModalChatConfirmClosed (const CuiMessageBox & box)
{
	if (&box == m_messageBoxModalChatConfirm)
	{
		m_messageBoxModalChatConfirm = 0;
		
		if (box.completedAffirmative ())
		{
			CuiPreferences::setModalChat (!CuiPreferences::getModalChat ());
			if (m_checkModalChat)
				m_checkModalChat->SetChecked (CuiPreferences::getModalChat (), false);
		}
	}
}

//----------------------------------------------------------------------
 
void SwgCuiOptKeymap::onInputSchemeReset ()
{
	if (m_bindEntry)
	{
		m_bindEntry->deactivate ();
		m_bindEntry->release ();
		m_bindEntry = 0;
	}

	if (m_originalInputMap)
	{
		delete m_originalInputMap;
		m_originalInputMap = 0;
		if (m_workingInputMap)
		{
			m_originalInputMap = NON_NULL (m_workingInputMap->getRebindingMap ());
		}
	}

	resetChecks ();
	updateData  ();
}

//----------------------------------------------------------------------

void SwgCuiOptKeymap::resetDefaults()
{
	if (!m_comboPreset)
		return;
	const int selectedIndex = m_comboPreset->GetSelectedIndex ();
	
	if (selectedIndex >= 0)
	{
		const UIData * const data = m_comboPreset->GetDataAtIndex (selectedIndex);
		if (data)
		{
			const std::string & schemeName = data->GetName ();
			if (!InputScheme::resetFromType (schemeName, false))
			{
				CuiMessageBox::createInfoBox (Unicode::narrowToWide ("Unable to load defaults.\n"));
				return;
			}
			//-- callback onInputSchemeReset() handles repopulating this UI
		}
	}
}

// ======================================================================
