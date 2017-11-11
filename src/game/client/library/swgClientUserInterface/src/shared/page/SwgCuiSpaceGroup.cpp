//======================================================================
//
// SwgCuiSpaceGroup.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiSpaceGroup.h"

#include "clientGame/ClientCommandQueue.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GroupObject.h"
#include "clientGame/ShipObject.h"
#include "clientGraphics/Camera.h"
#include "clientUserInterface/CuiMenuInfoTypes.h"
#include "clientUserInterface/CuiStringIdsGroup.h"
#include "sharedFoundation/Crc.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/NetworkIdManager.h"
#include "UIButton.h"
#include "UIComposite.h"
#include "UIImage.h"
#include "UIManager.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UIPopupMenu.h"
#include "UIPopupMenustyle.h"
#include "UIText.h"

#include <algorithm>
#include <list>
#include <map>

//======================================================================

namespace SwgCuiSpaceGroupNamespace
{
	namespace DataProperties
	{
		UILowerString const LocalizedDisplayName = UILowerString("LocalizedDisplayName");
		UILowerString const ShipNetworkID = UILowerString("ShipNetworkID");
		UILowerString const IsLeader = UILowerString("IsLeader");
		UILowerString const IsPlayerShip = UILowerString("IsPlayerShip");
		UILowerString const NumberOfPassengers = UILowerString("NumberOfPassengers");
	}

	struct GroupData
	{
		GroupData();

		NetworkId m_shipNetworkid;
		Unicode::String m_localizedName;
		float m_distanceFromPlayer;
		int m_numberOfPassengers;
		float m_shipShield;
		float m_shipArmor;
		bool m_isOutOfRange;
		bool m_isLeader;
		bool m_isPlayerShip;
	};

	namespace PopupIds
	{
		const std::string group_kick = "group_kick";
		const std::string group_disband = "group_disband";
		const std::string group_leave = "group_leave";
		const std::string group_masterlooter = "group_master_looter";
		const std::string group_makeleader   = "group_make_leader";
		const std::string group_loot	= "group_loot";
	}

	float distanceFromPlayer(Object const * object);
	void setHealth(UIPage * const uiPlayerPage, float const shield, float const armor);
	void populateMemberData(UIPage * uiPlayerPage, GroupData const & data);
	void updateMemberLocationArrow(UIPage * uiPlayerPage, Object const * ship);
	void updateInformation(UIPage * const uiPlayerPage);
	NetworkId getShipNetworkId(UIPage const * uiPlayerPage);
	Unicode::String getLocalizedDisplayName(UIPage const * uiPlayerPage);
	bool getIsLeader(UIPage const * uiPlayerPage);
	bool getIsPlayerShip(UIPage const * uiPlayerPage);
	int getNumberOfPassengers(UIPage const * uiPlayerPage);
}

//======================================================================

SwgCuiSpaceGroupNamespace::GroupData::GroupData()
: m_shipNetworkid()
, m_localizedName()
, m_distanceFromPlayer(0.0f)
, m_numberOfPassengers(1)
, m_shipShield(0.0f)
, m_shipArmor(0.0f)
, m_isOutOfRange(false)
, m_isLeader(false)
, m_isPlayerShip(false)
{
}

//-------------------------------------------------------------------

float SwgCuiSpaceGroupNamespace::distanceFromPlayer(Object const * const object)
{
	const Object * const player = Game::getPlayer();

	if ((player != 0) && (object != 0))
	{
		return object->getPosition_w().magnitudeBetween(player->getPosition_w());
	}
	return 0.0f;
}

//-------------------------------------------------------------------

void SwgCuiSpaceGroupNamespace::setHealth(UIPage * const uiPlayerPage, float const shield, float const armor)
{
	NOT_NULL(uiPlayerPage);

	UIPage * const healthPage = safe_cast<UIPage *>(uiPlayerPage->GetChild("health"));
	UIPage * const shieldPage = (healthPage != 0) ? safe_cast<UIPage *>(healthPage->GetChild("shield")) : 0;
	UIPage * const spacerPage = (healthPage != 0) ? safe_cast<UIPage *>(healthPage->GetChild("spacer")) : 0;
	UIPage * const armorPage = (healthPage != 0) ? safe_cast<UIPage *>(healthPage->GetChild("armor")) : 0;

	if (healthPage && shieldPage && armorPage && spacerPage)
	{
		int const totalWidth = healthPage->GetWidth() - spacerPage->GetWidth();
		int const shieldWidth = static_cast<int>(shield * 0.5f * static_cast<float>(totalWidth));
		UISize size = shieldPage->GetSize();
		size.x=shieldWidth;
		shieldPage->SetSize(size);
		shieldPage->SetMaximumSize(size);
		shieldPage->SetMinimumSize(size);

		int const armorWidth = static_cast<int>(armor * 0.5f * static_cast<float>(totalWidth));
		size = armorPage->GetSize();
		size.x=armorWidth;
		armorPage->SetSize(size);
		armorPage->SetMaximumSize(size);
		armorPage->SetMinimumSize(size);

		healthPage->Pack();
	}
} //lint !e818 // uiPlayerPage parameter is logically nonconst

//-------------------------------------------------------------------

void SwgCuiSpaceGroupNamespace::populateMemberData(UIPage * const uiPlayerPage, GroupData const & data)
{
	NOT_NULL(uiPlayerPage);

	uiPlayerPage->SetPropertyNarrow(DataProperties::ShipNetworkID, data.m_shipNetworkid.getValueString());
	uiPlayerPage->SetProperty(DataProperties::LocalizedDisplayName, data.m_localizedName);
	uiPlayerPage->SetPropertyBoolean(DataProperties::IsLeader, data.m_isLeader);
	uiPlayerPage->SetPropertyBoolean(DataProperties::IsPlayerShip, data.m_isPlayerShip);
	uiPlayerPage->SetPropertyInteger(DataProperties::NumberOfPassengers, data.m_numberOfPassengers);

	Unicode::String const & localizedName = data.m_localizedName;
	Unicode::String finalText;
	UIPage * const arrowPage = safe_cast<UIPage *>(uiPlayerPage->GetChild("nameArrow"));
	UIText * const uiText = safe_cast<UIText *>(arrowPage->GetChild("name"));
	if (uiText != 0)
	{
		if (data.m_isLeader)
		{
			finalText = Unicode::narrowToWide("\\#00ff44 (L) \\#.");
		}

		finalText += localizedName;

		if (data.m_numberOfPassengers > 1)
		{
			char buffer[16];
			snprintf(buffer, sizeof(buffer) - 1, "%d", data.m_numberOfPassengers);
			finalText += Unicode::narrowToWide(" (") + Unicode::narrowToWide(buffer) + Unicode::narrowToWide(")");
		}

		if (data.m_isOutOfRange)
		{
			finalText += CuiStringIdsGroup::out_of_range_suffix.localize();
		}
		else if (!data.m_isPlayerShip)
		{
			char buffer[128];
			snprintf(buffer, sizeof(buffer) - 1, " \\#ffffff(%.0fm)\\#.", data.m_distanceFromPlayer);
			finalText += Unicode::narrowToWide(buffer);
		}

		uiText->SetText(finalText);
	}

	setHealth(uiPlayerPage, data.m_shipShield, data.m_shipArmor);
} //lint !e818 // uiPlayerPage parameter is logically nonconst

//-------------------------------------------------------------------

void SwgCuiSpaceGroupNamespace::updateMemberLocationArrow(UIPage * const uiPlayerPage, Object const * const ship)
{
	NOT_NULL(uiPlayerPage);

	UIPage * const arrowPage = safe_cast<UIPage *>(uiPlayerPage->GetChild("nameArrow"));
	UIPage * const indicatorPage = (arrowPage != 0) ? safe_cast<UIPage *>(arrowPage->GetChild("indicator")) : 0;
	UIImage * const arrow = (indicatorPage != 0) ? safe_cast<UIImage *>(indicatorPage->GetChild("arrow")) : 0;
	UIImage * const circle = (indicatorPage != 0) ? safe_cast<UIImage *>(indicatorPage->GetChild("circle")) : 0;

	if ((arrow != 0) && (circle != 0))
	{
		ShipObject const * const playerShipObject = Game::getPlayerContainingShip();
		NetworkId const & playerShipId = (playerShipObject != 0) ? playerShipObject->getNetworkId() : NetworkId::cms_invalid;

		bool arrowVisible = (ship != 0) ? (playerShipId != ship->getNetworkId()) : false;

		if (arrowVisible)
		{
			Camera const * const camera = Game::getCamera();

			if ((camera != 0) && (ship != 0))
			{
				Vector const & cameraPos_w = camera->getPosition_w();
				Vector delta_vector = ship->getPosition_w() - cameraPos_w;
				Vector const delta_in_camera = camera->rotate_w2o(delta_vector);
				bool const normalized = delta_vector.normalize();
				float const theta = atan2(delta_in_camera.x, delta_in_camera.y);
				float const dotProduct = camera->getObjectFrameK_w().dot(delta_vector);
				if (!normalized || acos(dotProduct) < std::min(camera->getVerticalFieldOfView(), camera->getHorizontalFieldOfView()) / 2.f)
				{
					arrowVisible = false;
				}
				else
				{
					arrow->SetRotation(theta / PI_TIMES_2 - 0.25f);
				}
			}
		}
		arrow->SetVisible(arrowVisible);
		circle->SetVisible((ship) && (playerShipObject != ship) && (!arrowVisible));
	}
} //lint !e818 // uiPlayerPage parameter is logically nonconst

//-------------------------------------------------------------------

void SwgCuiSpaceGroupNamespace::updateInformation(UIPage * const uiPlayerPage)
{
	NOT_NULL(uiPlayerPage);

	NetworkId const shipId(getShipNetworkId(uiPlayerPage));

	if (shipId.isValid())
	{
		Object const * const object = NetworkIdManager::getObjectById(shipId);
		ClientObject const * const clientObject = (object != 0) ? object->asClientObject() : 0;
		ShipObject const * const shipObject = (clientObject != 0) ? clientObject->asShipObject() : 0;

		GroupData data;

		data.m_shipNetworkid = shipId;
		data.m_isLeader = getIsLeader(uiPlayerPage);
		data.m_isPlayerShip = getIsPlayerShip(uiPlayerPage);
		data.m_numberOfPassengers = getNumberOfPassengers(uiPlayerPage);

		if (shipObject != 0)
		{
			data.m_distanceFromPlayer = distanceFromPlayer(shipObject);
			data.m_localizedName = shipObject->getLocalizedName();
			data.m_shipShield = shipObject->getShieldHealth();
			data.m_shipArmor = shipObject->getArmorHealth();
		}
		else
		{
			data.m_localizedName = getLocalizedDisplayName(uiPlayerPage);
			data.m_isOutOfRange = true;
		}

		populateMemberData(uiPlayerPage, data);
		updateMemberLocationArrow(uiPlayerPage, shipObject);
	}
} //lint !e818 // uiPlayerPage parameter is logically nonconst

//-------------------------------------------------------------------

NetworkId SwgCuiSpaceGroupNamespace::getShipNetworkId(UIPage const * const uiPlayerPage)
{
	NOT_NULL(uiPlayerPage);
	std::string networkID;
	uiPlayerPage->GetPropertyNarrow(DataProperties::ShipNetworkID, networkID);
	return NetworkId(networkID);
}

//-------------------------------------------------------------------

Unicode::String SwgCuiSpaceGroupNamespace::getLocalizedDisplayName(UIPage const * uiPlayerPage)
{
	NOT_NULL(uiPlayerPage);
	Unicode::String localizedDisplayName;
	uiPlayerPage->GetProperty(DataProperties::LocalizedDisplayName, localizedDisplayName);
	return localizedDisplayName;
}

//-------------------------------------------------------------------

bool SwgCuiSpaceGroupNamespace::getIsLeader(UIPage const * uiPlayerPage)
{
	NOT_NULL(uiPlayerPage);
	bool isLeader = false;
	uiPlayerPage->GetPropertyBoolean(DataProperties::IsLeader, isLeader);
	return isLeader;
}

//-------------------------------------------------------------------

bool SwgCuiSpaceGroupNamespace::getIsPlayerShip(UIPage const * uiPlayerPage)
{
	NOT_NULL(uiPlayerPage);
	bool isPlayerShip = false;
	uiPlayerPage->GetPropertyBoolean(DataProperties::IsPlayerShip, isPlayerShip);
	return isPlayerShip;
}

//-------------------------------------------------------------------

int SwgCuiSpaceGroupNamespace::getNumberOfPassengers(UIPage const * uiPlayerPage)
{
	NOT_NULL(uiPlayerPage);
	int numberOfPassengers = 0;
	uiPlayerPage->GetPropertyInteger(DataProperties::NumberOfPassengers, numberOfPassengers);
	return numberOfPassengers;
}

//======================================================================

using namespace SwgCuiSpaceGroupNamespace;

//======================================================================

SwgCuiSpaceGroup::SwgCuiSpaceGroup(UIPage & page)
: SwgCuiLockableMediator("SwgCuiSpaceGroupStatus", page)
, m_callback(new MessageDispatch::Callback)
, m_members(0)
, m_sample(0)
, m_collapse(0)
, m_expand(0)
, m_numberOfMembers(0)
, m_collapsed(0)
, m_dirty(true)
, m_popupSelection(NetworkId::cms_invalid)
{
	getCodeDataObject(TUIComposite, m_members, "members");
	getCodeDataObject(TUIPage, m_sample, "sample");
	getCodeDataObject(TUIButton, m_collapse, "buttoncollapse");
	getCodeDataObject(TUIButton, m_expand, "buttonexpand");

	m_sample->Link();

	registerMediatorObject(*m_collapse, true);
	registerMediatorObject(*m_expand, true);

	m_callback->connect(*this, &SwgCuiSpaceGroup::onMembersChanged, static_cast<GroupObject::Messages::MembersChanged*>(0));
	m_callback->connect(*this, &SwgCuiSpaceGroup::onMemberAdded, static_cast<GroupObject::Messages::MemberAdded*>(0));
	m_callback->connect(*this, &SwgCuiSpaceGroup::onMemberRemoved, static_cast<GroupObject::Messages::MemberRemoved*>(0));
	m_callback->connect(*this, &SwgCuiSpaceGroup::onGroupChanged, static_cast<CreatureObject::Messages::GroupChanged*>(0));
	m_callback->connect(*this, &SwgCuiSpaceGroup::onMemberShipsChanged, static_cast<GroupObject::Messages::MemberShipsChanged*>(0));
	m_callback->connect(*this, &SwgCuiSpaceGroup::onMemberShipAdded, static_cast<GroupObject::Messages::MemberShipAdded*>(0));
	m_callback->connect(*this, &SwgCuiSpaceGroup::onMemberShipRemoved, static_cast<GroupObject::Messages::MemberShipRemoved*>(0));

}

//-------------------------------------------------------------------

SwgCuiSpaceGroup::~SwgCuiSpaceGroup()
{
	m_callback->disconnect(*this, &SwgCuiSpaceGroup::onMembersChanged, static_cast<GroupObject::Messages::MembersChanged*>(0));
	m_callback->disconnect(*this, &SwgCuiSpaceGroup::onMemberAdded, static_cast<GroupObject::Messages::MemberAdded*>(0));
	m_callback->disconnect(*this, &SwgCuiSpaceGroup::onMemberRemoved, static_cast<GroupObject::Messages::MemberRemoved*>(0));
	m_callback->disconnect(*this, &SwgCuiSpaceGroup::onGroupChanged, static_cast<CreatureObject::Messages::GroupChanged*>(0));
	m_callback->disconnect(*this, &SwgCuiSpaceGroup::onMemberShipsChanged, static_cast<GroupObject::Messages::MemberShipsChanged*>(0));
	m_callback->disconnect(*this, &SwgCuiSpaceGroup::onMemberShipAdded, static_cast<GroupObject::Messages::MemberShipAdded*>(0));
	m_callback->disconnect(*this, &SwgCuiSpaceGroup::onMemberShipRemoved, static_cast<GroupObject::Messages::MemberShipRemoved*>(0));

	delete m_callback;
	m_callback = 0;

	m_members = 0;
	m_sample = 0;
	m_collapse = 0;
	m_expand = 0;
}

//-------------------------------------------------------------------

void SwgCuiSpaceGroup::onMembersChanged(GroupObject::Messages::MembersChanged::Payload const & /*group*/)
{
	m_dirty = true;
}

//-------------------------------------------------------------------

void SwgCuiSpaceGroup::onGroupChanged(CreatureObject::Messages::GroupChanged::Payload const & /*creature*/)
{
	m_dirty = true;
}

//-------------------------------------------------------------------

void SwgCuiSpaceGroup::onMemberAdded(GroupObject::Messages::MemberAdded::Payload const & /*payload*/)
{
	m_dirty = true;
}

//-------------------------------------------------------------------

void SwgCuiSpaceGroup::onMemberRemoved(GroupObject::Messages::MemberRemoved::Payload const & /*payload*/)
{
	m_dirty = true;
}

//-------------------------------------------------------------------

void SwgCuiSpaceGroup::onMemberShipsChanged(GroupObject::Messages::MemberShipsChanged::Payload const & /*group*/)
{
	m_dirty = true;
}

//-------------------------------------------------------------------

void SwgCuiSpaceGroup::onMemberShipAdded(std::pair<GroupObject *, const GroupShipFormationMember *> const & /*payload*/)
{
	m_dirty = true;
}

//-------------------------------------------------------------------

void SwgCuiSpaceGroup::onMemberShipRemoved(std::pair<GroupObject *, const GroupShipFormationMember *> const & /*payload*/)
{
	m_dirty = true;
}

//-------------------------------------------------------------------

void SwgCuiSpaceGroup::performActivate()
{
	setIsUpdating(true);
	m_dirty = true;
}

//-------------------------------------------------------------------

void SwgCuiSpaceGroup::performDeactivate()
{
	setIsUpdating(false);
}

//-------------------------------------------------------------------

void SwgCuiSpaceGroup::OnButtonPressed(UIWidget * const context)
{
	if (context == m_collapse)
	{
		m_collapsed = true;
	}
	else if (context == m_expand)
	{
		m_collapsed = false;
	}

	m_dirty = true;
}

//-------------------------------------------------------------------

void SwgCuiSpaceGroup::update(float)
{
	if (m_dirty)
	{
		m_dirty = false;
		populateData();
	}

	bool hasMembers = false;

	if (m_members != 0)
	{
		UIBaseObject::UIObjectList children;

		m_members->GetChildren(children);

		UIBaseObject::UIObjectList::const_iterator ii = children.begin();
		UIBaseObject::UIObjectList::const_iterator iiEnd = children.end();

		for (; ii != iiEnd; ++ii)
		{
			UIPage * const uiPlayerPage = dynamic_cast<UIPage *>(*ii);
			if (uiPlayerPage != 0)
			{
				updateInformation(uiPlayerPage);
				hasMembers = true;
			}
		}
	}

	setEnabled(hasMembers);
}

//-------------------------------------------------------------------

void SwgCuiSpaceGroup::populateData()
{
	if (m_sample != 0)
	{
		m_sample->SetVisible(false);

		typedef std::vector<GroupObject::ShipMemberData> MembersToAddType;
		MembersToAddType membersToAdd;

		CreatureObject const * const playerCreatureObject = Game::getPlayerCreature();
		if (playerCreatureObject != 0)
		{
			CachedNetworkId const id(playerCreatureObject->getGroup());
			GroupObject const * const group = safe_cast<const GroupObject *>(id.getObject());
			if(group != 0)
			{
				membersToAdd = group->getMembersInShipsList();
			}
		}
		
		typedef std::vector<UIPage *> PageList;
		PageList memberPages;

		{
			MembersToAddType::const_iterator ii = membersToAdd.begin();
			MembersToAddType::const_iterator iiEnd = membersToAdd.end();

			for (int i = 0; ii != iiEnd; ++ii, ++i)
			{
				if ((i > 0) && (m_collapsed))
				{
					continue;
				}
				
				GroupObject::ShipMemberData const & shipMemberData = *ii;

				GroupData data;
				data.m_shipNetworkid = shipMemberData.m_ship;
				data.m_numberOfPassengers = shipMemberData.m_playerCount;
				data.m_isLeader = shipMemberData.m_leader;
				data.m_isPlayerShip = (ii == membersToAdd.begin());

				Object const * const object = NetworkIdManager::getObjectById(shipMemberData.m_ship);
				ClientObject const * const clientObject = (object != 0) ? object->asClientObject() : 0;
				ShipObject const * const shipObject = (clientObject != 0) ? clientObject->asShipObject() : 0;
				
				if (shipObject != 0)
				{
					data.m_isOutOfRange = false;

					data.m_distanceFromPlayer = distanceFromPlayer(shipObject);
					data.m_localizedName = shipObject->getLocalizedName();									
					data.m_shipShield = shipObject->getShieldHealth();
					data.m_shipArmor= shipObject->getArmorHealth();					
				}
				else
				{
					data.m_isOutOfRange = true;
				}
		
				UIPage * const page = safe_cast<UIPage *>(m_sample->DuplicateObject());
				populateMemberData(page, data);
				memberPages.push_back(page);
			}
		}

		if (m_members != 0)
		{
			// clear the samples
			m_members->Clear();

			{
				// add the new pages
				PageList::const_iterator ii = memberPages.begin();
				PageList::const_iterator iiEnd = memberPages.end();

				for (; ii != iiEnd; ++ii)
				{
					UIPage * const page = *ii;
					page->SetVisible(true);
					page->SetContextCapable(true, true);
					page->AddCallback(this);
					page->SetEnabled(true);
					page->Link();
					m_members->AddChild(page);
				}
			}

			m_numberOfMembers = static_cast<int>(memberPages.size());

			UISize const & sizeIncrement = getPage().GetSizeIncrement();
			int pageHeight = (m_numberOfMembers + 1) * (sizeIncrement.y + m_sample->GetHeight());

			getPage().SetHeight(pageHeight);
			getPage().SetPackDirty(true);

			m_members->Link();

			m_collapse->SetVisible(!m_collapsed);
			m_expand->SetVisible(m_collapsed);
		}
	}
}

//======================================================================
bool SwgCuiSpaceGroup::OnMessage (UIWidget *context, const UIMessage & msg)
{
	if (msg.Type == UIMessage::RightMouseUp)
	{

		UIPopupMenu * const pop = new UIPopupMenu(&getPage()); 
		pop->SetStyle(getPage().FindPopupStyle());

		const CreatureObject * const player = Game::getPlayerCreature();
		if (player)
		{
			//-- must find player by name, to aquire the pilot's networkID, not his shipID
			std::string memberIdStr;
			context->GetPropertyNarrow(DataProperties::LocalizedDisplayName, memberIdStr);

			GroupObject const * const group = safe_cast<GroupObject const *>(player->getGroup().getObject());

			if (group)
			{
				bool isLeader = false;
				GroupObject::GroupMember const * const gm = group->findMember(memberIdStr, isLeader);

				if (gm)
				{
					m_popupSelection = gm->first;
					
					const NetworkId m_objectId(memberIdStr);
					const GroupObject::GroupMemberVector gmv = group->getGroupMembers();

					if (!gmv.empty())
					{
						//-- player is the leader
						if (gmv.front().first == player->getNetworkId())
						{
							if (group->getLootMaster() != m_popupSelection)
								pop->AddItem(PopupIds::group_masterlooter, Cui::MenuInfoTypes::getLocalizedLabel(Cui::MenuInfoTypes::GROUP_MASTER_LOOTER, 0));

							if	(m_popupSelection != player->getNetworkId())
							{
								pop->AddItem(PopupIds::group_makeleader, Cui::MenuInfoTypes::getLocalizedLabel(Cui::MenuInfoTypes::GROUP_MAKE_LEADER, 0));
								pop->AddItem(PopupIds::group_kick, Cui::MenuInfoTypes::getLocalizedLabel(Cui::MenuInfoTypes::GROUP_KICK, 0));
							}
							else
							{
								pop->AddItem(PopupIds::group_disband, Cui::MenuInfoTypes::getLocalizedLabel(Cui::MenuInfoTypes::GROUP_DISBAND, 0));
								pop->AddItem(PopupIds::group_loot, Cui::MenuInfoTypes::getLocalizedLabel(Cui::MenuInfoTypes::GROUP_LOOT, 0));
							}
						}
						//-- player is NOT the leader
						else
						{
							if	(m_popupSelection == player->getNetworkId())
								pop->AddItem(PopupIds::group_leave, Cui::MenuInfoTypes::getLocalizedLabel(Cui::MenuInfoTypes::GROUP_LEAVE, 0));
						}
					}
				}
			}
		}
	
		appendPopupOptions(pop);
 
		pop->SetLocation(context->GetWorldLocation() + msg.MouseCoords);
		UIManager::gUIManager().PushContextWidget(*pop);
		pop->AddCallback(this);
	}

	return true;
}

//======================================================================

void SwgCuiSpaceGroup::OnPopupMenuSelection (UIWidget * context)
{
	if (m_popupSelection == NetworkId::cms_invalid)
		return;

	if (!context->IsA(TUIPopupMenu))
		return;

	UIPopupMenu * const pop = safe_cast<UIPopupMenu *>(context);

	NOT_NULL(pop);

	const std::string & selection = pop->GetSelectedName();
	
	if (selection == PopupIds::group_kick)
	{
		static const uint32 hash_group_kick = Crc::normalizeAndCalculate("dismissGroupMember");
		ClientCommandQueue::enqueueCommand(hash_group_kick, m_popupSelection, Unicode::String());
	}
	else if (selection == PopupIds::group_disband)
	{
		static const uint32 hash_group_disband = Crc::normalizeAndCalculate("disband");
		ClientCommandQueue::enqueueCommand(hash_group_disband, NetworkId::cms_invalid, Unicode::String());
	}
	else if (selection == PopupIds::group_leave)
	{
		static const uint32 hash_group_leave = Crc::normalizeAndCalculate("leaveGroup");
		ClientCommandQueue::enqueueCommand(hash_group_leave, NetworkId::cms_invalid, Unicode::String());
	}
	else if (selection == PopupIds::group_masterlooter)
	{
		static uint32 const hash_groupMakeMasterLooter = Crc::normalizeAndCalculate("makeMasterLooter");
		ClientCommandQueue::enqueueCommand(hash_groupMakeMasterLooter, m_popupSelection, Unicode::String());
	}
	else if (selection == PopupIds::group_makeleader)
	{
		static uint32 const hash_groupMakeLeader = Crc::normalizeAndCalculate("makeLeader");
		ClientCommandQueue::enqueueCommand(hash_groupMakeLeader, m_popupSelection, Unicode::String());
	}
	else if (selection == PopupIds::group_loot)
	{
		static uint32 const hash_groupLoot = Crc::normalizeAndCalculate("groupLoot");
		ClientCommandQueue::enqueueCommand(hash_groupLoot, NetworkId::cms_invalid, Unicode::String());
	}
	else
	{
		SwgCuiLockableMediator::OnPopupMenuSelection(context);
	}
	
	m_popupSelection = NetworkId::cms_invalid;

}