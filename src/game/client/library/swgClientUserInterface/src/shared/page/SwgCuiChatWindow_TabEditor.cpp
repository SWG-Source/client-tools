//======================================================================
//
// SwgCuiChatWindow_TabEditor.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiChatWindow_TabEditor.h"
#include "swgClientUserInterface/SwgCuiChatWindow_Tab.h"

#include "UIButton.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIList.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UIText.h"
#include "UITextbox.h"
#include "clientUserInterface/CuiActionManager.h"
#include "clientUserInterface/CuiActions.h"
#include "clientUserInterface/CuiChatRoomDataNode.h"
#include "clientUserInterface/CuiChatRoomManager.h"
#include "clientUserInterface/CuiManager.h"
#include "sharedMessageDispatch/Transceiver.h"

//======================================================================

namespace
{
	namespace Transceivers
	{
		MessageDispatch::Transceiver<const SwgCuiChatWindow::TabEditor::Messages::ChangeCompleted::Payload &, SwgCuiChatWindow::TabEditor::Messages::ChangeCompleted > 
			s_changeComplete;
	}

	namespace DataProperties
	{
		const UILowerString ChannelType = UILowerString ("ChannelType");
		const UILowerString ChannelName = UILowerString ("ChannelName");
	}
	
	//----------------------------------------------------------------------

	bool getChannelIdFromList (UIList & list, SwgCuiChatWindow::ChannelId & id)
	{
		const long index_available = list.GetLastSelectedRow ();

		if (index_available < 0)
			return false;

		const UIData * const data  = list.GetDataAtRow (index_available);
		
		if (!data)
		{
			WARNING (true, ("Bad index"));
			return false;
		}
		
		int type = 0;
		if (!data->GetPropertyInteger (DataProperties::ChannelType, type))
		{
			WARNING (true, ("No type"));
			return false;
		}
		
		id.type = static_cast<SwgCuiChatWindow::ChannelType>(type);
		
		if (type == SwgCuiChatWindow::CT_chatRoom)
		{
			std::string name;
			
			if (!data->GetPropertyNarrow (DataProperties::ChannelName, name))
			{
				WARNING (true, ("Channel with no name"));
				return false;
			}
			
			id.setName (name);
		}
		else
			id.updateDefaultName ();

		return true;
	}

	//----------------------------------------------------------------------

	UIData * const createDataFromChannelId (const SwgCuiChatWindow::ChannelId & id)
	{
		UIData * const data = new UIData;
		data->SetProperty        (UIList::DataProperties::TEXT, id.getDisplayName ());
		data->SetPropertyNarrow  (DataProperties::ChannelName,  id.getName ());
		data->SetPropertyInteger (DataProperties::ChannelType,  static_cast<int>(id.type));

		return data;
	}
}

//----------------------------------------------------------------------

SwgCuiChatWindow::TabEditor::TabEditor            (UIPage & page) :
CuiMediator           ("SwgCuiChatWindow::TabEditor", page),
UIEventCallback       (),
m_tab                 (new Tab),
m_buttonOk            (0),
m_buttonCancel        (0),
m_buttonAdd           (0),
m_buttonRemove        (0),
m_buttonJoin          (0),
m_buttonDefault       (0),
m_listAvailable       (0),
m_listCurrent         (0),
m_textDefault         (0),
m_textboxSetName      (0),
m_ignoreListChange    (false)
{
	getCodeDataObject (TUIButton,  m_buttonOk,       "buttonOk");
	getCodeDataObject (TUIButton,  m_buttonCancel,   "buttonCancel");
	getCodeDataObject (TUIButton,  m_buttonAdd,      "buttonAdd");
	getCodeDataObject (TUIButton,  m_buttonRemove,   "buttonRemove");
	getCodeDataObject (TUIButton,  m_buttonJoin,     "buttonJoin");
	getCodeDataObject (TUIButton,  m_buttonDefault,  "buttonDefault");

	getCodeDataObject (TUIList,    m_listAvailable,  "listAvailable");
	getCodeDataObject (TUIList,    m_listCurrent,    "listCurrent");
	
	getCodeDataObject (TUIText,    m_textDefault,    "textDefault");

	getCodeDataObject (TUITextbox, m_textboxSetName, "textboxSetName");

	setState (MS_closeable);

	UIDataSource * const ds_available = NON_NULL (m_listAvailable->GetDataSource ());
	ds_available->Clear ();

	UIDataSource * const ds_current = NON_NULL (m_listCurrent->GetDataSource ());
	ds_current->Clear ();

	registerMediatorObject (*m_buttonOk, true);
	registerMediatorObject (*m_buttonCancel, true);
	registerMediatorObject (*m_buttonAdd, true);
	registerMediatorObject (*m_buttonRemove, true);
	registerMediatorObject (*m_buttonJoin, true);
	registerMediatorObject (*m_buttonDefault, true);
	registerMediatorObject (*m_listAvailable, true);
	registerMediatorObject (*m_listCurrent, true);

	m_textDefault->SetPreLocalized (true);
}

//----------------------------------------------------------------------

SwgCuiChatWindow::TabEditor::~TabEditor            ()
{
	m_buttonOk           = 0;
	m_buttonCancel       = 0;
	m_buttonAdd          = 0;
	m_buttonRemove       = 0;
	m_buttonJoin         = 0;
	m_buttonDefault      = 0;
	m_listAvailable      = 0;
	m_listCurrent        = 0;
	m_textDefault        = 0;
	m_textboxSetName     = 0;

	delete m_tab;
	m_tab = 0;
}

//----------------------------------------------------------------------

void        SwgCuiChatWindow::TabEditor::performActivate      ()
{
	CuiManager::requestPointer (true);
	m_textboxSetName->SetFocus ();
}

//----------------------------------------------------------------------

void        SwgCuiChatWindow::TabEditor::performDeactivate    ()
{

	m_tab->setTabId (-1);

	CuiManager::requestPointer (false);
}


//----------------------------------------------------------------------

void SwgCuiChatWindow::TabEditor::OnGenericSelectionChanged (UIWidget * context)
{
	if (m_ignoreListChange)
		return;

	if (context == m_listCurrent)
	{
		m_ignoreListChange = true;
		m_listAvailable->SelectRow (-1);
		updateFromListState ();
		m_ignoreListChange = false;
	}
	else if (context == m_listAvailable)
	{
		m_ignoreListChange = true;
		m_listCurrent->SelectRow (-1);
		updateFromListState ();
		m_ignoreListChange = false;
	}
}

//----------------------------------------------------------------------

bool SwgCuiChatWindow::TabEditor::OnMessage (UIWidget *context, const UIMessage & msg)
{
	if (msg.Type == UIMessage::LeftMouseDoubleClick)
	{
		if (context == m_listCurrent)
		{
			removeChannel ();
			return false;
		}
		else if (context == m_listAvailable)
		{
			addChannel ();
			return false;
		}
	}

	return true;
}

//----------------------------------------------------------------------

void  SwgCuiChatWindow::TabEditor::OnButtonPressed      (UIWidget *context )
{
	if (m_buttonOk == context)
	{
		const Unicode::String & newUserAssignedName = m_textboxSetName->GetLocalText ();

		if (newUserAssignedName != m_tab->getName ())
			m_tab->setUserAssignedName (newUserAssignedName);

		Transceivers::s_changeComplete.emitMessage (*m_tab);
		//-- update actual tab
		closeThroughWorkspace ();
	}
	else if (m_buttonCancel == context)
	{
		closeThroughWorkspace ();
	}
	else if (m_buttonAdd == context)
	{
		addChannel ();
	}	
	else if (m_buttonRemove == context)
	{
		removeChannel ();
	}	
	else if (m_buttonJoin == context)
	{
		joinChannel ();
	}	
	else if (m_buttonDefault == context)
	{
		setDefaultChannel ();
	}	

}

//----------------------------------------------------------------------

SwgCuiChatWindow::TabEditor *  SwgCuiChatWindow::TabEditor::createInto                (UIPage & parent)
{
	UIPage * const dupe = NON_NULL (UIPage::DuplicateInto (parent, "/pda.ChatTabEditor"));
	dupe->Center ();
	return new TabEditor (*dupe);
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::TabEditor::updateFromListState       ()
{
	const long index_available = m_listAvailable->GetLastSelectedRow ();
	const long index_current   = m_listCurrent->GetLastSelectedRow   ();

	m_buttonAdd->SetEnabled     (index_available >= 0);
	m_buttonRemove->SetEnabled  (index_current   >= 0);

	m_buttonDefault->SetEnabled (true);

	/*
	ChannelId id;
	if (getChannelIdFromList (*m_listCurrent, id))
	{
		m_buttonDefault->SetEnabled (id != m_tab->getDefaultChannel ());
	}
	*/

	if (m_tab->channelsEmpty ())
	{
		m_textDefault->Clear ();
	}
	else
	{
		m_textDefault->SetLocalText (m_tab->getDefaultChannel ().getDisplayName ());
	}
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::TabEditor::setTab                    (const Tab & tab)
{
	m_tab->copy (tab, false);
	m_tab->setTabId (tab.getTabId ());

	reset ();
	updateFromListState ();
}

//----------------------------------------------------------------------

int SwgCuiChatWindow::TabEditor::getTabId                  ()
{
	return m_tab->getTabId ();
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::TabEditor::reset                ()
{
	m_ignoreListChange = true;

	UIDataSource * const ds_available = NON_NULL (m_listAvailable->GetDataSource ());
	ds_available->Clear ();
	
	UIDataSource * const ds_current = NON_NULL (m_listCurrent->GetDataSource ());
	ds_current->Clear ();
	
	Tab::ChannelVector cv;
	m_tab->getChannels (cv);
	
	{
		for (Tab::ChannelVector::const_iterator it = cv.begin (); it != cv.end (); ++it)
		{
			const ChannelId & id = *it;
			
			UIData * const data = createDataFromChannelId (id);
			ds_current->AddChild (data);
		}
	}
	
	//-- fill out the set first so that the channels are sorted properly
	typedef std::set<ChannelId> ChannelSet;
	ChannelSet cs;	
	
	{
		for (int i = static_cast<int>(CT_chatRoom) + 1; i < static_cast<int>(CT_numTypes); ++i)
		{
			const ChannelId id (static_cast<ChannelType>(i));
			if (!m_tab->hasChannel (id))
			{
				cs.insert (id);
			}
		}
	}
	
	{
		for (ChannelSet::const_iterator it = cs.begin (); it != cs.end (); ++it)
		{			
			const ChannelId & id = *it;
			UIData * const data = createDataFromChannelId (id);
			ds_available->AddChild (data);
		}		
	}

	CuiChatRoomManager::ChatRoomsEnteredVector rv;
	CuiChatRoomManager::getEnteredRooms (rv);
	
	for (CuiChatRoomManager::ChatRoomsEnteredVector::const_iterator it = rv.begin (); it != rv.end (); ++it)
	{
		const uint32 roomId = *it;
		const CuiChatRoomDataNode * const roomNode = CuiChatRoomManager::findRoomNode (roomId);
		
		if (roomNode)
		{
			const std::string & fullPath = roomNode->getFullPath ();

			const ChannelId chatRoomChannel (CT_chatRoom, fullPath);
			
			if (!m_tab->hasChannel (chatRoomChannel))
			{
				UIData * const data = createDataFromChannelId (chatRoomChannel);
				ds_available->AddChild (data);
			}
		}
	}

	m_ignoreListChange = false;

	m_textboxSetName->SetLocalText (m_tab->getName ());
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::TabEditor::addChannel ()
{
	ChannelId id;
	if (getChannelIdFromList (*m_listAvailable, id))
	{
		const long selectedIndex = m_listAvailable->GetLastSelectedRow ();
		m_tab->addChannel (id);	
		reset ();
		m_listAvailable->SelectRow (selectedIndex);
		updateFromListState ();
	}
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::TabEditor::removeChannel ()
{
	ChannelId id;
	if (getChannelIdFromList (*m_listCurrent, id))
	{
		const long selectedIndex = m_listCurrent->GetLastSelectedRow ();
		m_tab->removeChannel (id);	
		reset ();
		m_listCurrent->SelectRow (selectedIndex);
		updateFromListState ();
	}
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::TabEditor::joinChannel ()
{
	CuiActionManager::performAction (CuiActions::chatRoomBrowser, Unicode::emptyString);
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::TabEditor::setDefaultChannel ()
{
	ChannelId id;
	if (getChannelIdFromList (*m_listCurrent, id))
	{
		m_tab->setDefaultChannel (id);
		m_textboxSetName->SetLocalText (m_tab->getName ());		
		updateFromListState      ();
	}
}

//----------------------------------------------------------------------

bool SwgCuiChatWindow::TabEditor::handleChatRoomJoin        (const CuiChatRoomDataNode & roomNode)
{
	const std::string & fullPath = roomNode.getFullPath ();
	m_tab->addChannel (ChannelId (CT_chatRoom, fullPath));
	reset ();
	return true;
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::TabEditor::handleChatRoomLeave       (const CuiChatRoomDataNode & roomNode)
{
	const std::string & fullPath = roomNode.getFullPath ();
	m_tab->removeChannel (ChannelId (CT_chatRoom, fullPath));
	reset ();
}

//======================================================================
