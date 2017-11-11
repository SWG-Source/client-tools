//======================================================================
//
// SwgCuiChatRooms.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiChatRooms.h"

#include "UIButton.h"
#include "UIData.h"
#include "UIDataSourceContainer.h"
#include "UIPage.h"
#include "UISliderbar.h"
#include "UIText.h"
#include "UITreeView.h"
#include "UIUtils.h"
#include "UnicodeUtils.h"
#include "clientGame/GameNetwork.h"
#include "clientUserInterface/CuiActionManager.h"
#include "clientUserInterface/CuiActions.h"
#include "clientUserInterface/CuiChatManager.h"
#include "clientUserInterface/CuiChatRoomDataNode.h"
#include "clientUserInterface/CuiChatRoomManager.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiStringIdsChatRoom.h"
#include "clientUserInterface/CuiWorkspace.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/ChatAvatarIdArchive.h"
#include "sharedNetworkMessages/ChatRemoveAvatarFromRoom.h"
#include "swgClientUserInterface/SwgCuiChatRoomsCreate.h"
#include <algorithm>
#include <list>

//======================================================================

namespace
{

	ChatAvatarId s_selfId;

	//----------------------------------------------------------------------

	void setupDataSourceObject (UIDataSourceBase & obj, const CuiChatRoomDataNode & roomNode, const Unicode::String & iconJoinPath)
	{
		obj.SetName (Unicode::toLower (roomNode.name).c_str ());

		Unicode::String str;
		
		//@todo localize
		str = Unicode::narrowToWide (roomNode.name);
		
		if (!roomNode.data.title.empty ())
		{
			IGNORE_RETURN (str.append (1, ' '));
			IGNORE_RETURN (str.append (1, '-'));
			IGNORE_RETURN (str.append (1, ' '));
			IGNORE_RETURN (str.append (roomNode.data.title));
		}
		
		IGNORE_RETURN (obj.SetProperty (UITreeView::DataProperties::LocalText, str));
		
		if (!roomNode.data.id)
			obj.SetPropertyInteger (UITreeView::DataProperties::ColorIndex, 2);
		else if (roomNode.data.owner == s_selfId || roomNode.data.creator == s_selfId)
			obj.SetPropertyInteger (UITreeView::DataProperties::ColorIndex, 1);
		else
			obj.RemoveProperty (UITreeView::DataProperties::ColorIndex);

		if (CuiChatRoomManager::isRoomEntered (roomNode.data.id))
		{
			IGNORE_RETURN (obj.SetProperty (UITreeView::DataProperties::Icon, iconJoinPath));
		}
		else
			obj.RemoveProperty (UITreeView::DataProperties::Icon);		
	}

	//----------------------------------------------------------------------

	UIBaseObject * findObjectByLowerName (const UIBaseObject::UIObjectList & olist, const std::string & lowerName)
	{
		for (UIBaseObject::UIObjectList::const_iterator it = olist.begin (); it != olist.end (); ++it)
		{
			UIBaseObject * const obj = *it;	
			
			if (obj->GetName () == lowerName)
			{
				return obj;
			}
		}

		return 0;
	}

	//----------------------------------------------------------------------

	void populateDataSource (UIDataSourceContainer & container, const CuiChatRoomDataNode & room, const Unicode::String & iconJoinPath)
	{
		setupDataSourceObject (container, room, iconJoinPath);
		
		UIBaseObject::UIObjectList containerChildrenCopy;
		container.GetChildren (containerChildrenCopy);

		{
			for (UIBaseObject::UIObjectList::iterator it = containerChildrenCopy.begin (); it != containerChildrenCopy.end (); ++it)
			{
				UIBaseObject * const obj = *it;
				obj->Attach (0);
			}
		}

		container.Clear ();

		{
			const CuiChatRoomDataNode::NodeVector & roomChildren = room.getChildren ();

			for (CuiChatRoomDataNode::NodeVector::const_iterator it = roomChildren.begin (); it != roomChildren.end (); ++it)
			{
				const CuiChatRoomDataNode * const childRoom = NON_NULL (*it);
				
				//-- ignore the system room
				if (!_stricmp (childRoom->name.c_str (), ChatRoomTypes::ROOM_SYSTEM.c_str ()))
					continue;

				//-- ignore the system room
				if (!_stricmp (childRoom->name.c_str (), ChatRoomTypes::ROOM_PLANET.c_str ()))
					continue;

				UIDataSourceContainer * subcontainer = dynamic_cast<UIDataSourceContainer *>(findObjectByLowerName (containerChildrenCopy, childRoom->getLowerName ()));
				
				if (!subcontainer)
				{
					subcontainer = new UIDataSourceContainer;
				}

				IGNORE_RETURN (container.AddChild (subcontainer));
				populateDataSource (*subcontainer, *childRoom, iconJoinPath);
			} //lint !e429 //subcontainer not aleak
		}

		std::for_each (containerChildrenCopy.begin (), containerChildrenCopy.end (), UIBaseObject::DetachFunctor (0));
	}
}

//----------------------------------------------------------------------

SwgCuiChatRooms::SwgCuiChatRooms (UIPage & page) :
CuiMediator       ("SwgCuiChatRooms", page),
UIEventCallback   (),
m_buttonJoin      (0),
m_buttonLeave     (0),
m_buttonRefresh   (0),
m_buttonDone      (0),
m_buttonCreate    (0),
m_buttonDelete    (0),
m_buttonWho       (0),
m_textPath        (0),
m_textTitle       (0),
m_textCreator     (0),
m_textOwner       (0),
m_textMembers     (0),
m_tree            (0),
m_slider          (0),
m_callback        (new MessageDispatch::Callback),
m_sliderDirection (false),
m_iconJoinPath    (),
m_roomIdToDestroy (0)
{
	getCodeDataObject (TUIButton, m_buttonJoin,    "buttonJoin");
	getCodeDataObject (TUIButton, m_buttonLeave,   "buttonLeave");
	getCodeDataObject (TUIButton, m_buttonRefresh, "buttonRefresh");
	getCodeDataObject (TUIButton, m_buttonDone,    "buttonDone");
	getCodeDataObject (TUIButton, m_buttonCreate,  "buttonCreate");
	getCodeDataObject (TUIButton, m_buttonDelete,  "buttonDelete");
	getCodeDataObject (TUIButton, m_buttonWho,     "buttonWho");

	getCodeDataObject (TUIText,      m_textPath,      "textPath");
	getCodeDataObject (TUIText,      m_textTitle,     "textTitle");
	getCodeDataObject (TUIText,      m_textCreator,   "textCreator");
	getCodeDataObject (TUIText,      m_textOwner,     "textOwner");
	getCodeDataObject (TUIText,      m_textMembers,   "textMembers");
	getCodeDataObject (TUITreeView,  m_tree,          "tree");
	getCodeDataObject (TUISliderbar, m_slider,        "slider");

	if (!getCodeDataString ("iconJoined", m_iconJoinPath))
		DEBUG_FATAL (true, ("No iconJoined icon property in SwgCuiChatRooms"));

	m_textCreator->SetPreLocalized (true);
	m_textOwner->SetPreLocalized   (true);
	m_textTitle->SetPreLocalized   (true);
	m_textPath->SetPreLocalized    (true);
	m_textMembers->SetPreLocalized (true);

	setState (MS_closeable);
	setState (MS_closeDeactivates);

	UIDataSourceContainer * const dsc = NON_NULL (m_tree->GetDataSourceContainer ());
	dsc->Clear ();

	registerMediatorObject (*m_buttonJoin,     true);
	registerMediatorObject (*m_buttonLeave,    true);
	registerMediatorObject (*m_buttonRefresh,  true);
	registerMediatorObject (*m_buttonDone,     true);
	registerMediatorObject (*m_buttonCreate,   true);
	registerMediatorObject (*m_buttonDelete,   true);
	registerMediatorObject (*m_tree,           true);
	registerMediatorObject (*m_buttonWho,      true);
}

//----------------------------------------------------------------------

SwgCuiChatRooms::~SwgCuiChatRooms ()
{
	m_buttonJoin     = 0;
	m_buttonLeave    = 0;
	m_buttonRefresh  = 0;
	m_buttonDone     = 0;
	m_buttonCreate   = 0;
	m_buttonDelete   = 0;
	m_buttonWho      = 0;

	m_textPath      = 0;
	m_textTitle     = 0;
	m_textCreator   = 0;
	m_textOwner     = 0;
	m_textMembers   = 0;
	m_tree          = 0;
	m_slider        = 0;

	delete m_callback;
	m_callback      = 0;
}

//----------------------------------------------------------------------

void SwgCuiChatRooms::performActivate   ()
{
	CuiManager::requestPointer (true);

	updateTree    ();
	updateText    ();
	updateButtons ();

	m_buttonRefresh->SetVisible (true);

	m_callback->connect (*this, &SwgCuiChatRooms::onRoomsModified,       static_cast<CuiChatRoomManager::Messages::Modified*>   (0));

	refresh ();
}

//----------------------------------------------------------------------

void SwgCuiChatRooms::performDeactivate ()
{
	m_callback->disconnect (*this, &SwgCuiChatRooms::onRoomsModified,       static_cast<CuiChatRoomManager::Messages::Modified*>   (0));

	CuiManager::requestPointer (false);
}

//----------------------------------------------------------------------

void SwgCuiChatRooms::OnButtonPressed( UIWidget *context )
{
	if (context == m_buttonJoin)
	{
		join ();
	}
	else if (context == m_buttonDone)
	{
		if (getButtonClose ())
			getButtonClose ()->Press ();
	}
	else if (context == m_buttonLeave)
	{
		leave ();
	}
	else if (context == m_buttonCreate)
	{
		create ();
	}
	else if (context == m_buttonDelete)
	{
		destroy (0, false);
	}
	else if (context == m_buttonRefresh)
	{
		refresh ();
	}
	else if (context == m_buttonWho)
	{
		who ();
	}

} //lint !e818 //stfu noob

//----------------------------------------------------------------------

void SwgCuiChatRooms::OnGenericSelectionChanged (UIWidget * context)
{
	if (context == m_tree)
	{
		updateText ();
		updateButtons ();
	}
} //lint !e818 //stfu noob

//----------------------------------------------------------------------

void SwgCuiChatRooms::updateTree     ()
{
	UIDataSourceContainer * const dsc = m_tree->GetDataSourceContainer ();

	if (dsc)
	{
		s_selfId = CuiChatManager::getSelfAvatarId ();

		const CuiChatRoomDataNode * rooms = CuiChatRoomManager::findRoomNode (s_selfId.gameCode);

		if (rooms)
		{
			dsc->Attach (0);
			m_tree->SetDataSourceContainer (0, false);
			
			populateDataSource (*dsc, *rooms, m_iconJoinPath);
			
			m_tree->SetDataSourceContainer (dsc, true);
			dsc->Detach (0);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiChatRooms::updateText ()
{
	const CuiChatRoomDataNode * const roomNode = getSelectedChatRoom ();

	if (roomNode)
	{
		Unicode::String creatorName;
		CuiChatManager::getShortName (roomNode->data.creator, creatorName);
		Unicode::String ownerName;
		CuiChatManager::getShortName (roomNode->data.owner, ownerName);

		m_textCreator->SetLocalText (creatorName);

		if (ownerName == creatorName)
			m_textOwner->SetVisible (false);
		else
		{
			m_textOwner->SetLocalText   (ownerName);
			m_textOwner->SetVisible (true);
		}

		m_textPath->SetLocalText    (Unicode::narrowToWide (roomNode->data.path));

		if (roomNode->data.title.empty ())
			m_textTitle->SetVisible (false);
		else
		{
			m_textTitle->SetVisible (true);
			m_textTitle->SetLocalText   (roomNode->data.title);
		}

		m_textMembers->SetVisible (false);

		Unicode::String str;
		const CuiChatRoomDataNode::AvatarVector & roomMembers = roomNode->getMembers ();

		for (CuiChatRoomDataNode::AvatarVector::const_iterator it = roomMembers.begin (); it != roomMembers.end (); ++it)
		{
			const CuiChatAvatarId & avatarId = *it;

			Unicode::String memberName;
			CuiChatManager::getShortName (avatarId.chatId, memberName);
			str += memberName;
			str.append (1, '\n');
			m_textMembers->SetVisible (true);

		}

		m_textMembers->SetLocalText (str);

		UIPage * const parent = dynamic_cast<UIPage *>(m_textMembers->GetParent ());
		if (parent)
			parent->SetPackDirty (true);
	}
}

//----------------------------------------------------------------------

void SwgCuiChatRooms::updateButtons ()
{
	const CuiChatRoomDataNode * const roomNode = getSelectedChatRoom ();

	if (roomNode && roomNode->data.id)
	{
		const ChatAvatarId & selfId = CuiChatManager::getSelfAvatarId ();
		const bool playerIsOperator = (roomNode->data.owner == selfId) || (roomNode->data.creator == selfId) || roomNode->isModerator (selfId.getFullName ());
		const bool isRoomEntered    = CuiChatRoomManager::isRoomEntered (roomNode->data.id);

		m_buttonJoin->SetEnabled   (!isRoomEntered);
		m_buttonDelete->SetEnabled (playerIsOperator);
		m_buttonCreate->SetEnabled (true);
		m_buttonWho->SetEnabled    (isRoomEntered);

		// always enable the Leave button, just in case we are out of sync, so that
		// the player can "force" leave the room to get everything back in sync,
		// except for the guild and group chat room, which has the command
		// /groupTextChatRoomRejoin and /guildTextChatRoomRejoin and /cityTextChatRoomRejoin
		if ((roomNode->data.id == CuiChatRoomManager::getGuildRoomId()) || (roomNode->data.id == CuiChatRoomManager::getCityRoomId()) || (roomNode->data.id == CuiChatRoomManager::getGroupRoomId()))
		{
			m_buttonLeave->SetEnabled(false);

			// also disable Info and Join for the guild and group chat room
			m_buttonWho->SetEnabled(false);
			m_buttonJoin->SetEnabled(false);
		}
		else
		{
			m_buttonLeave->SetEnabled(true);
		}
	}
	else
	{
		m_buttonLeave->SetEnabled  (false);
		m_buttonJoin->SetEnabled   (false);
		m_buttonDelete->SetEnabled (false);
		m_buttonCreate->SetEnabled (false);
		m_buttonWho->SetEnabled    (false);
	}
}

//----------------------------------------------------------------------

void SwgCuiChatRooms::onRoomsModified (const uint32 & roomId)
{
	if (roomId == 0)
	{
		updateTree ();
		m_buttonRefresh->SetVisible (true);
	}
	else
	{
		const CuiChatRoomDataNode * const node = CuiChatRoomManager::findRoomNode (roomId);

		NOT_NULL (node);

		if (node)
			updateRoomNode (*node);
	}
}

//----------------------------------------------------------------------

void SwgCuiChatRooms::updateRoomNode (const CuiChatRoomDataNode & node)
{
	const CuiChatRoomDataNode * const selectedNode = getSelectedChatRoom ();
	
	if (selectedNode)
	{
		if (selectedNode->data.id == node.data.id)
		{
			updateText ();
		}
	}

	updateButtons ();


	UIDataSourceContainer * const dsc = m_tree->GetDataSourceContainer ();

	if (dsc)
	{
		UIDataSourceBase * const dsb = dynamic_cast<UIDataSourceBase *>(dsc->GetObjectFromPath (node.data.path.c_str (), TUIDataSourceBase));

		WARNING (!dsb, ("Received a roomnode update but room does not exist in the data tree."));

		if (dsb)
		{
			setupDataSourceObject (*dsb, node, m_iconJoinPath);
			//-- force a re-cache
			m_tree->SetDataSourceContainer (dsc);	
		}
	}
}

//----------------------------------------------------------------------

const CuiChatRoomDataNode * SwgCuiChatRooms::getSelectedChatRoom () const
{
	const UIDataSourceContainer * dsc = m_tree->GetDataSourceContainerAtRow (m_tree->GetLastSelectedRow ());

	std::string path;

	while (dsc && dsc != m_tree->GetDataSourceContainer ())
	{
		if (!path.empty ())
			path = dsc->GetName () + "." + path;
		else
			path = dsc->GetName ();

		dsc = dynamic_cast<const UIDataSourceContainer *>(dsc->GetParent ());
	}

	path = s_selfId.gameCode + "." + path;
	return CuiChatRoomManager::findRoomNode (path);
}

//----------------------------------------------------------------------

void SwgCuiChatRooms::join ()
{
	const CuiChatRoomDataNode * const roomNode = getSelectedChatRoom ();

	if (roomNode)
	{
		Unicode::String result;
		if (!CuiChatRoomManager::enterRoom (roomNode->data.id, roomNode, result))
		{
			CuiMessageBox::createInfoBox (result);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiChatRooms::refresh ()
{
	CuiChatRoomManager::requestRoomsRefresh ();
//	m_slider->SetVisible (true);
}

//----------------------------------------------------------------------

void SwgCuiChatRooms::leave ()
{
	const CuiChatRoomDataNode * const roomNode = getSelectedChatRoom ();

	if (roomNode)
	{
		// don't verify that the avatar is actually in the room, since we
		// may be out of sync and we just need to "force" leave the room
		// to get everything back in sync
		const ChatRemoveAvatarFromRoom msg (CuiChatManager::getSelfAvatarId(), roomNode->data.path);
		GameNetwork::send (msg, true);
	}
}

//----------------------------------------------------------------------

void SwgCuiChatRooms::create ()
{
	const CuiChatRoomDataNode * const roomNode = getSelectedChatRoom ();

	if (!roomNode || roomNode->data.id == 0)
		return;

	CuiWorkspace * const workspace = NON_NULL (getContainingWorkspace ());
	
	if (workspace)
	{
		SwgCuiChatRoomsCreate * mediator = safe_cast<SwgCuiChatRoomsCreate *>(workspace->findMediatorByType (typeid (SwgCuiChatRoomsCreate)));
		
		if (!mediator)
		{
			mediator = SwgCuiChatRoomsCreate::createInto (workspace->getPage ());
			workspace->addMediator      (*mediator);
		}

		mediator->activate ();
		
		workspace->focusMediator (*mediator, true);
		mediator->setRootPath    (roomNode->data.path);
	}
}

//----------------------------------------------------------------------

void SwgCuiChatRooms::destroy (uint32 roomId, bool confirmed)
{
	const CuiChatRoomDataNode * roomNode = 0;
	
	if (roomId)
		roomNode = CuiChatRoomManager::findRoomNode (roomId);
	else
		roomNode = getSelectedChatRoom ();

	if (roomNode)
	{
		if (!confirmed)
		{
			m_roomIdToDestroy = roomNode->data.id;
			CuiMessageBox * const box = CuiMessageBox::createYesNoBox (CuiStringIdsChatRoom::confirm_destroy_room.localize ());
			m_callback->connect (box->getTransceiverClosed (), *this, &SwgCuiChatRooms::onMessageBoxClosedConfirmDestroy);
			return;
		}
		
		Unicode::String result;
		if (!CuiChatRoomManager::destroyRoom (roomNode->data.id, result))
		{
			CuiMessageBox::createInfoBox (result);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiChatRooms::who ()
{
	const CuiChatRoomDataNode * const roomNode = getSelectedChatRoom ();

	if (roomNode)
	{
		Unicode::String param;
		UIUtils::FormatInteger (param, roomNode->data.id);
		CuiActionManager::performAction (CuiActions::chatRoomWho, param);
	}
}

//----------------------------------------------------------------------

void SwgCuiChatRooms::selectChatRoom            (uint32 roomId)
{
	UNREF (roomId);
}

//----------------------------------------------------------------------

void SwgCuiChatRooms::onMessageBoxClosedConfirmDestroy (const CuiMessageBox & box)
{
	if (box.completedAffirmative ())
	{
		destroy (m_roomIdToDestroy, true);
	}
}

//======================================================================
