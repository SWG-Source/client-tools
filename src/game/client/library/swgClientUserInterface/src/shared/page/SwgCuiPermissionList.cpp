// ======================================================================
//
// SwgCuiPermissionList.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiPermissionList.h"

#include "UIButton.h"
#include "UIComboBox.h"
#include "UIData.h"
#include "UIDatasource.h"
#include "UIList.h"
#include "UIPage.h"
#include "UIText.h"

#include "sharedObject/Object.h"

#include "clientGame/ClientCommandQueue.h"
#include "clientGame/Game.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiStringIds.h"

#include "swgSharedNetworkMessages/PermissionListCreateMessage.h"
#include <vector>

//-----------------------------------------------------------------------

namespace SwgCuiPermissionListNamespace
{

}

//-----------------------------------------------------------------------

SwgCuiPermissionList::SwgCuiPermissionList  (UIPage & page)
:
CuiMediator               ("SwgCuiPermissionList", page),
UIEventCallback           (),
UINotification            (),
MessageDispatch::Receiver (),
m_titleText               (0),
m_memberList              (0),
m_currentName             (),
m_listName                (),
m_addButton               (0),
m_removeButton            (0),
m_closeButton             (0),
m_addPage                 (0),
m_addOk                   (0),
m_addCancel               (0),
m_addCombo                (0),
m_nearbyPeople            ()
{
	setState    (MS_closeable);
	setState    (MS_closeDeactivates);
	removeState (MS_iconifiable);

	getCodeDataObject (TUIText,   m_titleText,      "titleText");
	getCodeDataObject (TUIList,   m_memberList,     "userList");
	getCodeDataObject (TUIButton, m_addButton,      "buttonAdd");
	getCodeDataObject (TUIButton, m_removeButton,   "buttonRemove");
	getCodeDataObject (TUIButton, m_closeButton,    "closebutton");

	getCodeDataObject (TUIPage,   m_addPage,        "addPg");
	getCodeDataObject (TUIButton, m_addOk,          "addOk");
	getCodeDataObject (TUIButton, m_addCancel,      "addCancel");
	getCodeDataObject (TUIComboBox, m_addCombo,     "addInput");

	UIDataSource* dataSource = m_memberList->GetDataSource ();
	dataSource->Clear();

	dataSource = m_addCombo->GetDataSource ();
	dataSource->Clear();

	m_titleText->SetLocalText(CuiStringIds::permissionlist_title.localize());
}

//-----------------------------------------------------------------------

SwgCuiPermissionList::~SwgCuiPermissionList ()
{
	m_memberList     = 0;
	m_addButton      = 0;
	m_removeButton   = 0;
	m_closeButton    = 0;
}

//-----------------------------------------------------------------------

void SwgCuiPermissionList::receiveMessage(const MessageDispatch::Emitter &, const MessageDispatch::MessageBase &)
{
}

//-----------------------------------------------------------------------

void SwgCuiPermissionList::performActivate ()
{
	CuiManager::requestPointer (true);
	m_addButton->AddCallback        (this);
	m_removeButton->AddCallback     (this);
	m_closeButton->AddCallback      (this);
	m_memberList->AddCallback       (this);
}

//-----------------------------------------------------------------------

void SwgCuiPermissionList::performDeactivate ()
{
	CuiManager::requestPointer    (false);
	m_addButton->RemoveCallback        (this);
	m_removeButton->RemoveCallback     (this);
	m_closeButton->RemoveCallback      (this);
	m_memberList->RemoveCallback       (this);
}

//-----------------------------------------------------------------------

bool SwgCuiPermissionList::close ()
{
	deactivate ();
	return true;
}

//-----------------------------------------------------------------------

void SwgCuiPermissionList::OnButtonPressed(UIWidget *context)
{
	if (context == m_addButton)
	{
		//show the add page
		IGNORE_RETURN(m_addCombo->SetProperty(UILowerString("SelectedText"), Unicode::emptyString));
		m_addPage->SetProperty(UILowerString("visible"), Unicode::narrowToWide("true"));

		m_addOk->AddCallback(this);
		m_addCancel->AddCallback(this);
	}
	else if (context == m_removeButton)
	{
		//check for no selection
		if(m_currentName == Unicode::emptyString)
			return;

		//package and send the response
		Unicode::String params;
		params += m_currentName;
		params += Unicode::narrowToWide(" ");
		params += m_listName;
		params += Unicode::narrowToWide(" ");
		params += Unicode::narrowToWide("remove");
		ClientCommandQueue::enqueueCommand("permissionListModify", Game::getPlayer()->getNetworkId(), params);
		//close the main window
		IGNORE_RETURN(close());
	}
	else if (context == m_closeButton)
	{
		IGNORE_RETURN(close());
	}
	else if (context == m_addOk)
	{
		//close this window
		m_addPage->SetProperty(UILowerString("visible"), Unicode::narrowToWide("false"));
		m_addOk->RemoveCallback(this);
		m_addCancel->RemoveCallback(this);

		//get the name from the combo
		Unicode::String name;
		IGNORE_RETURN(m_addCombo->GetProperty(UILowerString("SelectedText"), name));

		//package and send the response
		Unicode::String params;
		params += name;
		params += Unicode::narrowToWide(" ");
		params += m_listName;
		params += Unicode::narrowToWide(" ");
		params += Unicode::narrowToWide("add");
		ClientCommandQueue::enqueueCommand("permissionListModify", Game::getPlayer()->getNetworkId(), params);
		
		//close this and the main window
		IGNORE_RETURN(close());
	}
	else if (context == m_addCancel)
	{
		//close just this window
		m_addPage->SetProperty(UILowerString("visible"), Unicode::narrowToWide("false"));
		m_addOk->RemoveCallback(this);
		m_addCancel->RemoveCallback(this);
	}
}

//-----------------------------------------------------------------------

void SwgCuiPermissionList::OnGenericSelectionChanged (UIWidget * context)
{
	if (context == m_memberList)
	{
		const UIData * const data = m_memberList->GetDataAtRow (m_memberList->GetLastSelectedRow ());
		if (data)
		{
			data->GetProperty (UIList::DataProperties::TEXT, m_currentName); //lint !e123 TEXT defined elsewhere as a macro
		}
	}
}

//-----------------------------------------------------------------------

void SwgCuiPermissionList::Notify(UINotificationServer *, UIBaseObject *, Code )
{
}

//-----------------------------------------------------------------------

SwgCuiPermissionList* SwgCuiPermissionList::createInto (UIPage & parent)
{
	UIPage * const dupe = NON_NULL (UIPage::DuplicateInto (parent, "/PDA.PermissionList"));
	return new SwgCuiPermissionList (*dupe);
}

//-----------------------------------------------------------------------

void SwgCuiPermissionList::setData (const PermissionListCreateMessage& msg)
{
	//add the current members to the list
	UIDataSource* dataSource = NON_NULL(m_memberList->GetDataSource ());
	dataSource->Clear();
	const std::vector<Unicode::String>& members = msg.getCurrentMembers();
	std::vector<Unicode::String>::const_iterator i;
	for(i = members.begin(); i != members.end(); ++i)
	{
		UIData* const d = new UIData;
		IGNORE_RETURN(d->SetProperty(UIList::DataProperties::TEXT, *i));//lint !e123 TEXT defined elsewhere as a macro
		IGNORE_RETURN(dataSource->AddChild(d));
	}

	//add the nearby people to the combo box
	dataSource = NON_NULL(m_addCombo->GetDataSource ());
	dataSource->Clear();
	m_nearbyPeople = msg.getNearbyPeople();
	for(i = m_nearbyPeople.begin(); i != m_nearbyPeople.end(); ++i)
	{
		UIData* const d = new UIData;
		IGNORE_RETURN(d->SetProperty(UIList::DataProperties::TEXT, *i));//lint !e123 TEXT defined elsewhere as a macro
		IGNORE_RETURN(dataSource->AddChild(d));
	}

	//get the name of the list that we're working with
	m_listName = msg.getListName();

	Unicode::String title = CuiStringIds::permissionlist_title.localize() + m_listName;

	m_titleText->SetLocalText(title);
}

// ======================================================================
