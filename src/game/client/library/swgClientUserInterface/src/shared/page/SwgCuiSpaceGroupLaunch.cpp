//===================================================================
//
// SwgCuiSpaceGroupLaunch.cpp
// copyright 2004, sony online entertainment
// tford
//
//===================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiSpaceGroupLaunch.h"

#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GroupObject.h"
#include "clientGame/GroupManager.h"
#include "clientGame/PlayerCreatureController.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/Watcher.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/Container.h"
#include "sharedObject/NetworkIdManager.h"
#include "UIBaseObject.h"
#include "UIButton.h"
#include "UIComposite.h"
#include "UIImage.h"
#include "UIText.h"
#include <list>
#include <map>
#include <set>

//===================================================================

namespace SwgCuiSpaceGroupLaunchNamespace
{
	namespace DataProperties
	{
		UILowerString const HasMemberData = UILowerString("HasMemberData");
		UILowerString const NetworkID = UILowerString("NetworkID");

		UILowerString const ImageState = UILowerString("ImageState");
		UILowerString const ButtonState = UILowerString("ButtonState");

		UILowerString const InviteButton = UILowerString("InviteButton");
		UILowerString const KickButton = UILowerString("KickButton");
		UILowerString const ReinviteButton = UILowerString("ReinviteButton");
		UILowerString const WaitingButton = UILowerString("WaitingButton");
	}

	enum ImageState
	{
		IS_none,
		IS_notInvited,
		IS_accepted,
		IS_kicked,
		IS_declinedImage
	};

	enum ButtonState
	{
		BS_none,
		BS_invite,
		BS_kick,
		BS_reinvite,
		BS_waiting
	};

	float const cs_updateAfterElapsedTimeSeconds = 0.5f;
	int const cs_maximumNumberOfPlayersToInvite = 30;

	SwgCuiSpaceGroupLaunch::NetworkIdSet s_acceptedMembers;
	bool s_acceptedMembersWasReset = false;

	void setBaseData(UIPage * page, NetworkId const & id, Unicode::String const & name);
	void updateStatus(UIPage * page);

	bool getHasMemberData(UIBaseObject const * object);

	NetworkId getNetworkId(UIBaseObject const * object);
	void setNetworkId(UIBaseObject * object, NetworkId const & id);

	ImageState getImageState(UIBaseObject const * object);
	void setImageState(UIBaseObject * object, ImageState state);

	ButtonState getButtonState(UIBaseObject const * object);
	void setButtonState(UIBaseObject * object, ButtonState state);

	bool getIsInviteButton(UIButton const * object);
	bool getIsKickButton(UIButton const * object);
	bool getIsReinviteButton(UIButton const * object);
	bool getIsWaitingButton(UIButton const * object);

	UIButton * getInviteButton(UIPage const * page);
	UIButton * getKickButton(UIPage const * page);
	UIButton * getReInviteButton(UIPage const *  page);

	void findPotentialPassengersInSameCell(SwgCuiSpaceGroupLaunch::CreatureWatcherSet & membersInSameCell, ShipObject const * proposedShip);
}

//----------------------------------------------------------------------

void SwgCuiSpaceGroupLaunchNamespace::setBaseData(UIPage * const page, NetworkId const & id, Unicode::String const & name)
{
	NOT_NULL(page);

	UIText * const nameText = safe_cast<UIText *>(page->GetChild("name"));
	if (nameText != 0)
	{
		nameText->SetText(name);
	}

	setNetworkId(page, id);

	UIButton * const inviteButton = safe_cast<UIButton *>(page->GetChild("btnInvite"));
	UIButton * const kickButton = safe_cast<UIButton *>(page->GetChild("btnKick"));
	UIButton * const reInviteButton = safe_cast<UIButton *>(page->GetChild("btnReInvite"));
	UIButton * const waitingButton = safe_cast<UIButton *>(page->GetChild("btnWaiting"));

	if (inviteButton != 0)
	{
		inviteButton->SetPropertyBoolean(DataProperties::InviteButton, true);
	}

	if (kickButton != 0)
	{
		kickButton->SetPropertyBoolean(DataProperties::KickButton, true);
	}

	if (reInviteButton != 0)
	{
		reInviteButton->SetPropertyBoolean(DataProperties::ReinviteButton, true);
	}

	if (waitingButton != 0)
	{
		waitingButton->SetPropertyBoolean(DataProperties::WaitingButton, true);
	}

	setImageState(page, IS_notInvited);
	setButtonState(page, BS_invite);
}

//----------------------------------------------------------------------

void SwgCuiSpaceGroupLaunchNamespace::updateStatus(UIPage * const page)
{
	NOT_NULL(page);

	UIImage * const notInvitedImage = safe_cast<UIImage *>(page->GetChild("notInvited"));
	UIImage * const acceptedImage = safe_cast<UIImage *>(page->GetChild("accepted"));
	UIImage * const kickedImage = safe_cast<UIImage *>(page->GetChild("kicked"));
	UIImage * const declinedImage = safe_cast<UIImage *>(page->GetChild("declined"));

	UIImage * imageToEnable = 0;

	switch(getImageState(page))
	{
		case IS_notInvited:
			imageToEnable = notInvitedImage;
			break;
		case IS_accepted:
			imageToEnable = acceptedImage;
			break;
		case IS_kicked:
			imageToEnable = kickedImage;
			break;
		case IS_declinedImage:
			imageToEnable = declinedImage;
			break;
		default:
			break;
	}

	if (notInvitedImage != 0)
	{
		notInvitedImage->SetVisible(false);
	}

	if (acceptedImage != 0)
	{
		acceptedImage->SetVisible(false);
	}

	if (kickedImage != 0)
	{
		kickedImage->SetVisible(false);
	}

	if (declinedImage != 0)
	{
		declinedImage->SetVisible(false);
	}

	if (imageToEnable != 0)
	{
		imageToEnable->SetVisible(true);
	}

	UIButton * const inviteButton = safe_cast<UIButton *>(page->GetChild("btnInvite"));
	UIButton * const kickButton = safe_cast<UIButton *>(page->GetChild("btnKick"));
	UIButton * const reInviteButton = safe_cast<UIButton *>(page->GetChild("btnReInvite"));
	UIButton * const waitingButton = safe_cast<UIButton *>(page->GetChild("btnWaiting"));
	UIButton * buttonToEnable = 0;

	switch(getButtonState(page))
	{
		case BS_invite:
			buttonToEnable = inviteButton;
			break;
		case BS_kick:
			buttonToEnable = kickButton;
			break;
		case BS_reinvite:
			buttonToEnable = reInviteButton;
			break;
		case BS_waiting:
			buttonToEnable = waitingButton;
			break;
		default:
			break;
	}

	if (inviteButton != 0)
	{
		inviteButton->SetVisible(false);
	}

	if (kickButton != 0)
	{
		kickButton->SetVisible(false);
	}

	if (reInviteButton != 0)
	{
		reInviteButton->SetVisible(false);
	}

	if (waitingButton != 0)
	{
		waitingButton->SetVisible(false);
	}

	if (buttonToEnable != 0)
	{
		buttonToEnable->SetVisible(true);
	}
}

//----------------------------------------------------------------------

bool SwgCuiSpaceGroupLaunchNamespace::getHasMemberData(UIBaseObject const * const object)
{
	NOT_NULL(object);
	bool hasMemberData = false;
	object->GetPropertyBoolean(DataProperties::HasMemberData, hasMemberData);
	return hasMemberData;
}

//----------------------------------------------------------------------

NetworkId SwgCuiSpaceGroupLaunchNamespace::getNetworkId(UIBaseObject const * const object)
{
	NOT_NULL(object);
	DEBUG_FATAL(!getHasMemberData(object), ("SwgCuiSpaceGroupLaunchNamespace::getNetworkId"));

	std::string networkID;
	object->GetPropertyNarrow(DataProperties::NetworkID, networkID);
	return NetworkId(networkID);
}

//----------------------------------------------------------------------

void SwgCuiSpaceGroupLaunchNamespace::setNetworkId(UIBaseObject * const object, NetworkId const & id)
{
	object->SetPropertyBoolean(DataProperties::HasMemberData, true);
	object->SetPropertyNarrow(DataProperties::NetworkID, id.getValueString());

	typedef std::list<UIBaseObject *> UIObjectList;

	UIObjectList children;

	object->GetChildren(children);

	UIObjectList::const_iterator ii = children.begin();
	UIObjectList::const_iterator iiEnd = children.end();

	for (; ii != iiEnd; ++ii)
	{
		UIBaseObject * const child = *ii;
		setNetworkId(child, id);
	}
}

//----------------------------------------------------------------------

SwgCuiSpaceGroupLaunchNamespace::ImageState SwgCuiSpaceGroupLaunchNamespace::getImageState(UIBaseObject const * const object)
{
	NOT_NULL(object);
	DEBUG_FATAL(!getHasMemberData(object), ("SwgCuiSpaceGroupLaunchNamespace::getIsImageState"));

	int data = 0;
	object->GetPropertyInteger(DataProperties::ImageState, data);

	return static_cast<SwgCuiSpaceGroupLaunchNamespace::ImageState>(data);
}

//----------------------------------------------------------------------

void SwgCuiSpaceGroupLaunchNamespace::setImageState(UIBaseObject * const object, SwgCuiSpaceGroupLaunchNamespace::ImageState state)
{
	NOT_NULL(object);
	object->SetPropertyBoolean(DataProperties::HasMemberData, true);
	object->SetPropertyInteger(DataProperties::ImageState, static_cast<int>(state));
}

//----------------------------------------------------------------------

SwgCuiSpaceGroupLaunchNamespace::ButtonState SwgCuiSpaceGroupLaunchNamespace::getButtonState(UIBaseObject const * const object)
{
	NOT_NULL(object);
	DEBUG_FATAL(!getHasMemberData(object), ("SwgCuiSpaceGroupLaunchNamespace::getIsButtonState"));

	int data = 0;
	object->GetPropertyInteger(DataProperties::ButtonState, data);

	return static_cast<SwgCuiSpaceGroupLaunchNamespace::ButtonState>(data);
}

//----------------------------------------------------------------------

void SwgCuiSpaceGroupLaunchNamespace::setButtonState(UIBaseObject * const object, SwgCuiSpaceGroupLaunchNamespace::ButtonState state)
{
	NOT_NULL(object);
	object->SetPropertyBoolean(DataProperties::HasMemberData, true);
	object->SetPropertyInteger(DataProperties::ButtonState, static_cast<int>(state));
}

//----------------------------------------------------------------------

bool SwgCuiSpaceGroupLaunchNamespace::getIsInviteButton(UIButton const * const object)
{
	NOT_NULL(object);
	DEBUG_FATAL(!getHasMemberData(object), ("SwgCuiSpaceGroupLaunchNamespace::getIsInviteButton"));

	bool isInviteButton = false;
	object->GetPropertyBoolean(DataProperties::InviteButton, isInviteButton);
	return isInviteButton;
}

//----------------------------------------------------------------------

bool SwgCuiSpaceGroupLaunchNamespace::getIsKickButton(UIButton const * const object)
{
	NOT_NULL(object);
	DEBUG_FATAL(!getHasMemberData(object), ("SwgCuiSpaceGroupLaunchNamespace::getIsKickButton"));

	bool isKickButton = false;
	object->GetPropertyBoolean(DataProperties::KickButton, isKickButton);
	return isKickButton;
}

//----------------------------------------------------------------------

bool SwgCuiSpaceGroupLaunchNamespace::getIsReinviteButton(UIButton const * const object)
{
	NOT_NULL(object);
	DEBUG_FATAL(!getHasMemberData(object), ("SwgCuiSpaceGroupLaunchNamespace::getIsReinviteButton"));

	bool isReinviteButton = false;
	object->GetPropertyBoolean(DataProperties::ReinviteButton, isReinviteButton);
	return isReinviteButton;
}

//----------------------------------------------------------------------

bool SwgCuiSpaceGroupLaunchNamespace::getIsWaitingButton(UIButton const * const object)
{
	NOT_NULL(object);
	DEBUG_FATAL(!getHasMemberData(object), ("SwgCuiSpaceGroupLaunchNamespace::getIsWaitingButton"));

	bool isWaitingButton = false;
	object->GetPropertyBoolean(DataProperties::WaitingButton, isWaitingButton);
	return isWaitingButton;
}

//----------------------------------------------------------------------

UIButton * SwgCuiSpaceGroupLaunchNamespace::getInviteButton(UIPage const * const page)
{
	NOT_NULL(page);
	DEBUG_FATAL(!getHasMemberData(page), ("SwgCuiSpaceGroupLaunchNamespace::getHasAccepted"));

	return safe_cast<UIButton *>(page->GetChild("btnInvite"));
}

//----------------------------------------------------------------------

UIButton * SwgCuiSpaceGroupLaunchNamespace::getKickButton(UIPage const * const page)
{
	NOT_NULL(page);
	DEBUG_FATAL(!getHasMemberData(page), ("SwgCuiSpaceGroupLaunchNamespace::getHasAccepted"));

	return safe_cast<UIButton *>(page->GetChild("btnKick"));
}

//----------------------------------------------------------------------

UIButton * SwgCuiSpaceGroupLaunchNamespace::getReInviteButton(UIPage const * const page)
{
	NOT_NULL(page);
	DEBUG_FATAL(!getHasMemberData(page), ("SwgCuiSpaceGroupLaunchNamespace::getHasAccepted"));

	return safe_cast<UIButton *>(page->GetChild("btnReInvite"));
}

//----------------------------------------------------------------------

void SwgCuiSpaceGroupLaunchNamespace::findPotentialPassengersInSameCell(SwgCuiSpaceGroupLaunch::CreatureWatcherSet & membersInSameCell, ShipObject const * const proposedShip)
{
	int numberOfPlayersInvited = 0;
	
	CreatureObject const * const playerCreatureObject = Game::getPlayerCreature();
	if (playerCreatureObject != 0)
	{
		CellProperty const * const playerCellProperty = playerCreatureObject->getParentCell();

		// favor the group first, in case there are 100 people in the cell
		CachedNetworkId const id(playerCreatureObject->getGroup());
		GroupObject const * const group = safe_cast<const GroupObject *>(id.getObject());
		if (group != 0)
		{
			GroupObject::GroupMemberVector const & members = group->getGroupMembers();

			GroupObject::GroupMemberVector::const_iterator ii = members.begin();
			GroupObject::GroupMemberVector::const_iterator iiEnd = members.end();

			for (; ii != iiEnd; ++ii)
			{
				NetworkId const & Id = ii->first;
				Object * const object = NetworkIdManager::getObjectById(Id);
				ClientObject * const clientObject = (object != 0) ? object->asClientObject() : 0;
				CreatureObject * const memberCreatureObject = (clientObject != 0) ? clientObject->asCreatureObject() : 0;

				if ((memberCreatureObject != 0) && (memberCreatureObject->isPlayer()) && (memberCreatureObject != playerCreatureObject))
				{
					CellProperty const * const memberCellProperty = (memberCreatureObject != 0) ? memberCreatureObject->getParentCell() : 0;

					if ((memberCellProperty != 0) && (playerCellProperty == memberCellProperty))
					{
						SwgCuiSpaceGroupLaunch::CreatureWatcher creatureWatcher(memberCreatureObject);
						membersInSameCell.insert(creatureWatcher);
						++numberOfPlayersInvited;
					}
				}
			}
		}

		if ((playerCellProperty != 0) && (proposedShip != 0) && (proposedShip->isPobShip()))
		{
			ContainerConstIterator ii = playerCellProperty->begin();
			ContainerConstIterator iiEnd = playerCellProperty->end();
			for (; ii != iiEnd; ++ii)
			{
				Object * const object = (*ii).getObject();
				ClientObject * const clientObject = (object != 0) ? object->asClientObject() : 0;
				TangibleObject * const tangibleObject = (clientObject != 0) ? clientObject->asTangibleObject() : 0;
				CreatureObject * const creatureObject = (tangibleObject != 0) ? tangibleObject->asCreatureObject() : 0;

				if ((creatureObject != 0) && (creatureObject->isPlayer()) && (creatureObject != playerCreatureObject))
				{
					if (numberOfPlayersInvited < cs_maximumNumberOfPlayersToInvite)
					{
						SwgCuiSpaceGroupLaunch::CreatureWatcher creatureWatcher(creatureObject);
						membersInSameCell.insert(creatureWatcher);
						++numberOfPlayersInvited;
					}
				}
			}
		}
	}
}

//======================================================================

using namespace SwgCuiSpaceGroupLaunchNamespace;

//======================================================================

SwgCuiSpaceGroupLaunch::SwgCuiSpaceGroupLaunch(UIPage & page)
: UIEventCallback()
, CuiMediator("SwgCuiSpaceGroupLaunch", page)
, m_callback(new MessageDispatch::Callback)
, m_ship(0)
, m_buttonCancel(0)
, m_buttonOk(0)
, m_sample(0)
, m_members(0)
, m_inviteRule(0)
, m_dirty(true)
, m_memberPagesById()
, m_elapsedTimeSeconds(0.0f)
{
	getCodeDataObject(TUIButton, m_buttonCancel, "buttonCancel");
	getCodeDataObject(TUIButton, m_buttonOk, "buttonOk");
	getCodeDataObject(TUIPage, m_sample, "sample");
	getCodeDataObject(TUIComposite, m_members, "members");
	getCodeDataObject(TUIText, m_inviteRule, "inviteRule");

	registerMediatorObject(*m_buttonCancel, true);
	registerMediatorObject(*m_buttonOk, true);

	setState(MS_closeable);
	setState(MS_closeDeactivates);
}

//----------------------------------------------------------------------

SwgCuiSpaceGroupLaunch::~SwgCuiSpaceGroupLaunch()
{
	delete m_callback;
	m_callback = 0;

	m_ship = 0;

	m_buttonCancel = 0;
	m_buttonOk = 0;
	m_sample = 0;
	m_members = 0;
}

//----------------------------------------------------------------------

SwgCuiSpaceGroupLaunch::NetworkIdSet const & SwgCuiSpaceGroupLaunch::getAcceptedMembers()
{
	return s_acceptedMembers;
}

//----------------------------------------------------------------------

void SwgCuiSpaceGroupLaunch::resetAcceptedMembers()
{
	s_acceptedMembers.clear();
	s_acceptedMembersWasReset = true;
}

//----------------------------------------------------------------------

void SwgCuiSpaceGroupLaunch::setShip(ShipObject * const ship)
{
	m_ship = ship;

	if ((m_ship != 0) && (m_ship->isPobShip()))
	{
		if (m_inviteRule != 0)
		{
			Unicode::String const rule(StringId("ui_group_launch", "invite_rule_pob").localize());
			m_inviteRule->SetLocalText(rule);
			m_inviteRule->SetPreLocalized(true);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiSpaceGroupLaunch::onMembersChanged(GroupObject::Messages::MembersChanged::Payload const & /*group*/)
{
	m_dirty = true;
}

//-------------------------------------------------------------------

void SwgCuiSpaceGroupLaunch::onGroupChanged(CreatureObject::Messages::GroupChanged::Payload const & /*creature*/)
{
	m_dirty = true;
}

//-------------------------------------------------------------------

void SwgCuiSpaceGroupLaunch::onMemberAdded(std::pair<GroupObject *, const GroupMember *> const & /*payload*/)
{
	m_dirty = true;
}

//-------------------------------------------------------------------

void SwgCuiSpaceGroupLaunch::onMemberRemoved(std::pair<GroupObject *, const GroupMember *> const & /*payload*/)
{
	m_dirty = true;
}

//-------------------------------------------------------------------

void SwgCuiSpaceGroupLaunch::performActivate()
{
	CuiMediator::performActivate();

	m_callback->connect(*this, &SwgCuiSpaceGroupLaunch::onMembersChanged, static_cast<GroupObject::Messages::MembersChanged*>(0));
	m_callback->connect(*this, &SwgCuiSpaceGroupLaunch::onMemberAdded, static_cast<GroupObject::Messages::MemberAdded*>(0));
	m_callback->connect(*this, &SwgCuiSpaceGroupLaunch::onMemberRemoved, static_cast<GroupObject::Messages::MemberRemoved*>(0));
	m_callback->connect(*this, &SwgCuiSpaceGroupLaunch::onGroupChanged, static_cast<CreatureObject::Messages::GroupChanged*>(0));
	m_callback->connect(*this, &SwgCuiSpaceGroupLaunch::onReceivedInvitation, static_cast<PlayerCreatureController::Messages::GroupMemberInvitationToLaunchReceived *>(0));

	setIsUpdating(true);
	m_dirty = true;

	GroupManager::openedLaunchIntoSpaceUI();
}

//-------------------------------------------------------------------

void SwgCuiSpaceGroupLaunch::performDeactivate()
{
	CuiMediator::performDeactivate();

	m_callback->disconnect(*this, &SwgCuiSpaceGroupLaunch::onMembersChanged, static_cast<GroupObject::Messages::MembersChanged*>(0));
	m_callback->disconnect(*this, &SwgCuiSpaceGroupLaunch::onMemberAdded, static_cast<GroupObject::Messages::MemberAdded*>(0));
	m_callback->disconnect(*this, &SwgCuiSpaceGroupLaunch::onMemberRemoved, static_cast<GroupObject::Messages::MemberRemoved*>(0));
	m_callback->disconnect(*this, &SwgCuiSpaceGroupLaunch::onGroupChanged, static_cast<CreatureObject::Messages::GroupChanged*>(0));
	m_callback->disconnect(*this, &SwgCuiSpaceGroupLaunch::onReceivedInvitation, static_cast<PlayerCreatureController::Messages::GroupMemberInvitationToLaunchReceived *>(0));

	setIsUpdating(false);

	GroupManager::closedLaunchIntoSpaceUI();
}

//-------------------------------------------------------------------

void SwgCuiSpaceGroupLaunch::update(float deltaTimeSeconds)
{
	m_elapsedTimeSeconds += deltaTimeSeconds;

	if ((m_elapsedTimeSeconds > cs_updateAfterElapsedTimeSeconds) || (m_dirty))
	{
		m_elapsedTimeSeconds = 0.0f;

		CreatureWatcherSet inSameCell;
		findPotentialPassengersInSameCell(inSameCell, m_ship);

		if (inSameCell != m_membersInSameCell)
		{
			m_membersInSameCell = inSameCell;
			m_dirty = true;
		}
	}

	if (m_dirty)
	{
		populate(m_membersInSameCell);
		m_dirty = false;
	}
}

//-------------------------------------------------------------------

void SwgCuiSpaceGroupLaunch::OnButtonPressed(UIWidget * const context)
{
	if (context == m_buttonOk)
	{
		closeThroughWorkspace();
	}
	else if (context == m_buttonCancel)
	{
		reset();
		closeThroughWorkspace();
	}
	else if (getHasMemberData(context))
	{
		UIButton * const button = dynamic_cast<UIButton *>(context);
		if (button != 0)
		{
			NetworkId const id(getNetworkId(button));

			if (getIsInviteButton(button))
			{
				sendInviteToPlayer(id);
			}
			else if (getIsKickButton(button))
			{
				kickoutInvitedPlayer(id);
			}
			else if (getIsReinviteButton(button))
			{
				sendInviteToPlayer(id);
			}

			UIPage * parent = dynamic_cast<UIPage *>(button->GetParentWidget());
			if (parent != 0)
			{
				updateStatus(parent);
			}
		}
	}
}

//-------------------------------------------------------------------

void SwgCuiSpaceGroupLaunch::populate(CreatureWatcherSet const & membersInSameCell)
{
	if (m_sample != 0)
	{
		m_sample->SetVisible(false);

		if (s_acceptedMembersWasReset)
		{
			s_acceptedMembersWasReset = false;
			m_memberPagesById.clear();
		}

		typedef std::vector<UIPage *> PageList;
		PageList pagesToAdd;

		{
			CreatureWatcherSet::const_iterator ii = membersInSameCell.begin();
			CreatureWatcherSet::const_iterator iiEnd = membersInSameCell.end();

			for (; ii != iiEnd; ++ii)
			{
				CreatureObject const * const creatureObject = *ii;
				if (creatureObject != 0)
				{
					UIPage * const page = safe_cast<UIPage *>(m_sample->DuplicateObject());

					NetworkId const & Id = creatureObject->getNetworkId();
					Unicode::String const & name = creatureObject->getLocalizedName();

					setBaseData(page, Id, name);

					MemberPagesById::const_iterator jj = m_memberPagesById.find(Id);
					if (jj != m_memberPagesById.end())
					{
						UIPage const * const previousPage = jj->second;

						if ((previousPage != 0) && (Id == getNetworkId(previousPage)))
						{
							setImageState(page, getImageState(previousPage));
							setButtonState(page, getButtonState(previousPage));
							updateStatus(page);
						}
					}

					pagesToAdd.push_back(page);
				}
			}
		}

		if (m_members != 0)
		{
			m_members->Clear();
			m_memberPagesById.clear();

			PageList::const_iterator ii = pagesToAdd.begin();
			PageList::const_iterator iiEnd = pagesToAdd.end();

			for (; ii != iiEnd; ++ii)
			{
				UIPage * const page = *ii;

				if (page != 0)
				{
					page->SetVisible(true);

					m_members->AddChild(page);

					m_memberPagesById[getNetworkId(page)] = page;

					registerMediatorObject(*getInviteButton(page), true);
					registerMediatorObject(*getKickButton(page), true);
					registerMediatorObject(*getReInviteButton(page), true);
				}
			}
			m_members->Link();
		}
	}
}

//-------------------------------------------------------------------

void SwgCuiSpaceGroupLaunch::sendInviteToPlayer(NetworkId const & Id) const
{
	MemberPagesById::const_iterator ii = m_memberPagesById.find(Id);
	if (ii != m_memberPagesById.end())
	{
		UIPage * const page = ii->second;
		DEBUG_FATAL(!getHasMemberData(page), ("SwgCuiSpaceGroupLaunch::sendInviteToPlayer"));
		setButtonState(page, BS_waiting);
		updateStatus(page);
	}

	Object * const player = Game::getPlayer();
	Controller * const playerController = (player != 0) ? player->getController() : 0;

	if (playerController != 0)
	{
		MessageQueueGenericValueType<NetworkId> * data = new MessageQueueGenericValueType<NetworkId>(Id);
		playerController->appendMessage(CM_inviteOtherGroupMembersToLaunchIntoSpace,
										0.0f,
										data,
										GameControllerMessageFlags::SEND |
										GameControllerMessageFlags::RELIABLE |
										GameControllerMessageFlags::DEST_AUTH_SERVER);
	} //lint !e429 // data owned by controller
}

//-------------------------------------------------------------------

void SwgCuiSpaceGroupLaunch::onReceivedInvitation(std::pair<NetworkId, bool /*accepted*/> const & answer)
{
	NetworkId const & Id = answer.first;
	bool accepted = answer.second;

	if (accepted)
	{
		IGNORE_RETURN(s_acceptedMembers.insert(Id));

		MemberPagesById::const_iterator ii = m_memberPagesById.find(Id);
		if (ii != m_memberPagesById.end())
		{
			UIPage * const page = ii->second;
			DEBUG_FATAL(!getHasMemberData(page), ("SwgCuiSpaceGroupLaunch::onAcceptedInvitation"));
			setImageState(page, IS_accepted);
			setButtonState(page, BS_kick);
			updateStatus(page);
		}
	}
	else
	{
		onDeclinedInvitation(Id);
	}
}

//-------------------------------------------------------------------

void SwgCuiSpaceGroupLaunch::onDeclinedInvitation(NetworkId const & Id)
{
	{
		NetworkIdSet::iterator ii = s_acceptedMembers.find(Id);
		if (ii != s_acceptedMembers.end())
		{
			s_acceptedMembers.erase(ii);
		}
	}

	MemberPagesById::const_iterator ii = m_memberPagesById.find(Id);
	if (ii != m_memberPagesById.end())
	{
		UIPage * const page = ii->second;
		DEBUG_FATAL(!getHasMemberData(page), ("SwgCuiSpaceGroupLaunch::onDeclinedInvitation"));
		setImageState(page, IS_declinedImage);
		setButtonState(page, BS_reinvite);
		updateStatus(page);
	}
}

//-------------------------------------------------------------------

void SwgCuiSpaceGroupLaunch::kickoutInvitedPlayer(NetworkId const & Id)
{
	{
		NetworkIdSet::iterator ii = s_acceptedMembers.find(Id);
		if (ii != s_acceptedMembers.end())
		{
			s_acceptedMembers.erase(ii);
		}
	}

	MemberPagesById::const_iterator ii = m_memberPagesById.find(Id);
	if (ii != m_memberPagesById.end())
	{
		UIPage * const page = ii->second;
		DEBUG_FATAL(!getHasMemberData(page), ("SwgCuiSpaceGroupLaunch::onDeclinedInvitation"));
		setImageState(page, IS_kicked);
		setButtonState(page, BS_reinvite);
		updateStatus(page);
	}
}

//-------------------------------------------------------------------

void SwgCuiSpaceGroupLaunch::reset()
{
	resetAcceptedMembers();
	m_memberPagesById.clear();
}

//======================================================================

