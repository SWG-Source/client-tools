//======================================================================
//
// SwgCuiGroup.cpp
// copyright(c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiGroup.h"

#include "clientGame/ClientCommandQueue.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GroupObject.h"
#include "clientUserInterface/CuiRadialMenuManager.h"
#include "clientUserInterface/CuiSettings.h"
#include "clientUserInterface/CuiStringIdsGroup.h"
#include "clientUserInterface/CuiWorkspace.h"
#include "sharedFoundation/CalendarTime.h"
#include "sharedFoundation/Crc.h"
#include "clientUserInterface/CuiMenuInfoTypes.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/NetworkIdManager.h"
#include "swgClientUserInterface/SwgCuiBuffDisplay.h"
#include "swgClientUserInterface/SwgCuiStatusFactory.h"

#include "UIButton.h"
#include "UIComposite.h"
#include "UIData.h"
#include "UIImage.h"
#include "UIManager.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UIPopupMenu.h"
#include "UIPopupMenustyle.h"
#include "UIText.h"
#include "UIUtils.h"

#include <algorithm>

//======================================================================

namespace SwgCuiGroupNamespace
{
	namespace Properties
	{
		const UILowerString MemberId("MemberId");
	}

	const std::string IconFocusName = "IconFocus";
	
	namespace PopupIds
	{
		const std::string group_kick = "group_kick";
		const std::string group_disband = "group_disband";
		const std::string group_leave = "group_leave";
		const std::string group_masterlooter = "group_master_looter";
		const std::string group_makeleader   = "group_make_leader";
		const std::string group_tell   = "group_tell";
	}

	char const * const cs_lootStrings[GroupObject::LR_numberOfLootRules] =
							{
								"FF",
								"ML",
								"LO",
								"RN"
							};

	Unicode::String getLootString(GroupObject::LootRule const lootRule);
	
	namespace Settings
	{
		const std::string buffsVisible("buffsVisible");
	}
	
	UIScalar const s_maxGroupSize = 8;
	UIScalar const s_minGroupSize = 1;

	Unicode::String s_timerString;
	bool s_groupPickupTimerActive = false;
}

//======================================================================

Unicode::String SwgCuiGroupNamespace::getLootString(GroupObject::LootRule const lootRule)
{
	if (lootRule < GroupObject::LR_numberOfLootRules)
	{
		return Unicode::narrowToWide(cs_lootStrings[lootRule]);
	}

	return Unicode::String();
}

//======================================================================

using namespace SwgCuiGroupNamespace;

//======================================================================

SwgCuiGroup::SwgCuiGroup(UIPage & page) :
SwgCuiLockableMediator("SwgCuiGroup", page),
m_sample(0),
m_mfds(new MfdStatusVector),
m_callback(new MessageDispatch::Callback),
m_mouseIsDown(false),
m_popupMemberId(),
m_heightManuallySet(false),
m_sceneType(static_cast<int>(Game::getHudSceneType())),
m_timerPage(NULL),
m_timerText(NULL),
m_timerBar(NULL)
{
	{
		UIPage * sample = 0;
		getCodeDataObject(TUIPage, sample, "sample");
		m_sample = UI_ASOBJECT(UIPage, sample->DuplicateObject());
		m_sample->SetVisible(false);
		m_sample->Link();

		getPage().RemoveChild(sample);
	}
	
	m_callback->connect(*this, &SwgCuiGroup::onMembersChanged, static_cast<GroupObject::Messages::MembersChanged*>(0));
	m_callback->connect(*this, &SwgCuiGroup::onMemberAdded, static_cast<GroupObject::Messages::MemberAdded*>(0));
	m_callback->connect(*this, &SwgCuiGroup::onMemberRemoved, static_cast<GroupObject::Messages::MemberRemoved*>(0));
	m_callback->connect(*this, &SwgCuiGroup::onGroupChanged , static_cast<CreatureObject::Messages::GroupChanged*>(0));
	m_callback->connect(*this, &SwgCuiGroup::onLootChanged, static_cast<GroupObject::Messages::LootMasterChanged*>(0));
	m_callback->connect(*this, &SwgCuiGroup::onLootChanged, static_cast<GroupObject::Messages::LootRuleChanged*>(0));
	m_callback->connect(*this, &SwgCuiGroup::onGroupPickupPointTimerChanged, static_cast<GroupObject::Messages::GroupPickupTimerChanged*>(0));

	registerMediatorObject(getPage(), true);
	
	getCodeDataObject(TUIPage, m_timerPage, "timerpage");
	getCodeDataObject(TUIPage, m_timerBar, "timerbar");
	getCodeDataObject(TUIText, m_timerText, "timertext");

	m_timerPage->SetVisible(false);

	s_timerString = StringId("ui", "group_pickup_timer").localize();
}

//----------------------------------------------------------------------

SwgCuiGroup::~SwgCuiGroup()
{
	clear();

	m_callback->disconnect(*this, &SwgCuiGroup::onMembersChanged, static_cast<GroupObject::Messages::MembersChanged*>(0));
	m_callback->disconnect(*this, &SwgCuiGroup::onMemberAdded, static_cast<GroupObject::Messages::MemberAdded*>(0));
	m_callback->disconnect(*this, &SwgCuiGroup::onMemberRemoved, static_cast<GroupObject::Messages::MemberRemoved*>(0));
	m_callback->disconnect(*this, &SwgCuiGroup::onGroupChanged , static_cast<CreatureObject::Messages::GroupChanged*>(0));
	m_callback->disconnect(*this, &SwgCuiGroup::onGroupPickupPointTimerChanged, static_cast<GroupObject::Messages::GroupPickupTimerChanged*>(0));

	m_sample = 0;
	m_timerPage = 0;
	m_timerBar = 0;
	m_timerText = 0;

	delete m_mfds;
	m_mfds = 0;

	delete m_callback;
	m_callback = 0;
}

//----------------------------------------------------------------------

Unicode::String SwgCuiGroup::buildNamePrefix(GroupObject const * const group, NetworkId const & memberToTest)
{
	NOT_NULL(group);

	Unicode::String text;

	if (group == 0)
	{
		return text;
	}

	GroupObject::LootRule const lootRule = static_cast<GroupObject::LootRule>(group->getLootRule());
	bool const isLeader =(group->getLeader() == memberToTest);
	bool const isLootMaster =(group->getLootMaster() == memberToTest);
	bool const hasText =(isLeader || isLootMaster);

	if (hasText)
	{
		text += Unicode::narrowToWide("\\#00ff44 (");
	}

	if (isLeader)
	{
		text += Unicode::narrowToWide("L");

		if (isLootMaster)
		{
			text += Unicode::narrowToWide(":");
		}
	}

	if (isLootMaster)
	{
		text += getLootString(lootRule);
	}
	
	if (hasText)
	{
		text += Unicode::narrowToWide(") \\#.");
	}

	return text;
}

//----------------------------------------------------------------------

void SwgCuiGroup::performActivate()
{
	m_callback->connect(*this, &SwgCuiGroup::onLookAtTargetChanged, static_cast<CreatureObject::Messages::LookAtTargetChanged*>(0));
	updateGroupMembers();
	setIsUpdating(true);
}

//----------------------------------------------------------------------

void SwgCuiGroup::performDeactivate()
{
	setIsUpdating(false);
	m_callback->disconnect(*this, &SwgCuiGroup::onLookAtTargetChanged, static_cast<CreatureObject::Messages::LookAtTargetChanged*>(0));
}

//----------------------------------------------------------------------

void SwgCuiGroup::update(float deltaTimeSecs)
{
	CuiMediator::update(deltaTimeSecs);

	bool const showPage = !m_mfds->empty();
	getPage().SetVisible(showPage);
	activateMFDs(showPage);

	if (s_groupPickupTimerActive)
	{
		time_t timerEnd = 0;
		time_t timerTotal = 0;

		CreatureObject const * const player = Game::getPlayerCreature();
		if (player) 
		{
			CachedNetworkId const id(player->getGroup());

			if (id.isValid())
			{
				GroupObject const * const group = safe_cast<GroupObject const * const>(id.getObject());

				if (group)
				{
					timerEnd = group->getSecondsLeftOnGroupPickup();
					timerTotal = group->getGroupPickupDurationSeconds();
				}
			}			
		}
		
		if (timerEnd > 0)
		{
			UIWidget const * const parent = m_timerBar->GetParentWidget();

			if (parent)
			{
				long const width = parent->GetWidth() * timerEnd / timerTotal;
				m_timerBar->SetWidth(width);
								
				std::string timeString = CalendarTime::convertSecondsToMS(timerEnd);
							
				m_timerText->SetLocalText(s_timerString + Unicode::narrowToWide(timeString.c_str()));
			}
		}
		else
		{
			s_groupPickupTimerActive = false;
			resizeGroupWindow(m_mfds->size());
		}
	}
	
	m_timerPage->SetVisible(s_groupPickupTimerActive);
}

//----------------------------------------------------------------------

void SwgCuiGroup::updateGroupMembers()
{
	clear();

	CreatureObject const * const player = Game::getPlayerCreature();
	if (player) 
	{
		CachedNetworkId const id(player->getGroup());
		GroupObject const * group = safe_cast<const GroupObject *>(id.getObject());

		if (group) 
		{
			GroupObject::GroupMemberVector const & gmv = group->getGroupMembers();

			// your player is not in the group window, so resize it to size() - 1 members.
			resizeGroupWindow(gmv.size() - 1);

			for (GroupObject::GroupMemberVector::const_iterator it = gmv.begin(); it != gmv.end(); ++it)
			{
				GroupObject::GroupMember const & member = *it;
				
				if (member.first != player->getNetworkId())
				{
					addMember(member.first, member.second, buildNamePrefix(group, member.first));
				}
			}
		}
	}

	updateSelection();
}

//----------------------------------------------------------------------

void SwgCuiGroup::clear()
{
	clearMembers();
	m_mouseIsDown = false;
	m_heightManuallySet = false;
}

//----------------------------------------------------------------------

void SwgCuiGroup::onMembersChanged(const GroupObject & group)
{
	const CreatureObject * const player = Game::getPlayerCreature();
	if (!player)
		return;

	if (group.getNetworkId() == player->getGroup())
	{
		updateGroupMembers();
	}
}

//----------------------------------------------------------------------

void SwgCuiGroup::onMemberAdded(const GroupObject::Messages::MemberRemoved::Payload &)
{
} //lint !e1762 could be const, but it changes the signature.

//----------------------------------------------------------------------

void SwgCuiGroup::onMemberRemoved(const GroupObject::Messages::MemberRemoved::Payload &)
{
} //lint !e1762 could be const, but it changes the signature.

//----------------------------------------------------------------------

void SwgCuiGroup::onGroupChanged(const CreatureObject & creature)
{
	if (dynamic_cast<CreatureObject *>(Game::getPlayer()) == &creature)
		updateGroupMembers();
}

//----------------------------------------------------------------------

bool SwgCuiGroup::OnMessage(UIWidget * context, const UIMessage & msg)
{
	if (context->GetParent() != &getPage())
		return true;

	if (msg.Type == UIMessage::LeftMouseDown)
	{
		m_mouseIsDown = true;
		context->SetActivated(true);
		return true;
	}
	else if (msg.Type == UIMessage::LeftMouseUp)
	{
		if (m_mouseIsDown)
		{
			m_mouseIsDown = false;
			context->SetActivated(false);

			if (context->HitTest(msg.MouseCoords))
			{
				std::string memberIdStr;
				if (context->GetPropertyNarrow(Properties::MemberId, memberIdStr))
				{
					const NetworkId id(memberIdStr);
					if (NetworkIdManager::getObjectById(id))
					{
						CreatureObject * const player = Game::getPlayerCreature();
						if (player)
						{
							player->setIntendedTarget(id);
							player->setLookAtTarget(id);
							return false;
						}
					}
				}
			}
		}
		return true;
	}
	else if (msg.Type == UIMessage::RightMouseUp)
	{
		m_popupMemberId = NetworkId::cms_invalid;

		if (context->GetParent() == &getPage())
		{
			UIPopupMenu * const pop = new UIPopupMenu(&getPage()); 
			pop->SetStyle(getPage().FindPopupStyle());

			std::string memberIdStr;
			if (context->GetPropertyNarrow(Properties::MemberId, memberIdStr))
			{
				m_popupMemberId = NetworkId (memberIdStr);
				generateMemberPopup(pop);
			
			} //lint !e429 custodial pointer not freed or returned. The Workspace owns it.
			
			appendPopupOptions (pop);

			pop->SetLocation(context->GetWorldLocation() + msg.MouseCoords);
			UIManager::gUIManager().PushContextWidget(*pop);
			pop->AddCallback(this);
			return false;
		}
	}
	
	return true;
}

//----------------------------------------------------------------------

void SwgCuiGroup::OnPopupMenuSelection(UIWidget * context)
{
	if (m_popupMemberId == NetworkId::cms_invalid)
		return;

	if (!context->IsA(TUIPopupMenu))
		return;

	UIPopupMenu * const pop = safe_cast<UIPopupMenu *>(context);

	NOT_NULL(pop);

	const std::string & selection = pop->GetSelectedName();
	
	if (selection == PopupIds::group_kick)
	{
		static const uint32 hash_group_kick = Crc::normalizeAndCalculate("dismissGroupMember");
		ClientCommandQueue::enqueueCommand(hash_group_kick, m_popupMemberId, Unicode::String());
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
		ClientCommandQueue::enqueueCommand(hash_groupMakeMasterLooter, m_popupMemberId, Unicode::String());
	}
	else if (selection == PopupIds::group_makeleader)
	{
		static uint32 const hash_groupMakeLeader = Crc::normalizeAndCalculate("makeLeader");
		ClientCommandQueue::enqueueCommand(hash_groupMakeLeader, m_popupMemberId, Unicode::String());

	}
	else if (selection == PopupIds::group_tell)
	{
		CreatureObject const * const player = Game::getPlayerCreature();
		if (player) 
		{
			CachedNetworkId const id(player->getGroup());
			GroupObject const * group = safe_cast<const GroupObject *>(id.getObject());

			if (group) 
			{
				GroupObject::GroupMemberVector const & gmv = group->getGroupMembers();

				for (GroupObject::GroupMemberVector::const_iterator it = gmv.begin(); it != gmv.end(); ++it)
				{
					GroupObject::GroupMember const & member = *it;

					if (member.first == m_popupMemberId)
					{
						char tellMember[64];
						int const spaceLoc = member.second.find(' ');

						// if there is a space, then there is a last name which needs to be formatted out
						if (spaceLoc > 0)
						{
							char firstName[32];
							memcpy (firstName, member.second.c_str(), spaceLoc);
							firstName[spaceLoc] = '\0';
							sprintf(tellMember, "/tell %s ",firstName);
						}
						else
							sprintf(tellMember, "/tell %s ", member.second);

						Game::startChatInput(Unicode::narrowToWide(tellMember));
					}
				}
			}
		}
	}
	else
	{
		SwgCuiLockableMediator::OnPopupMenuSelection(context);
	}

	m_popupMemberId = NetworkId::cms_invalid;
}

//----------------------------------------------------------------------

void SwgCuiGroup::onLookAtTargetChanged(const CreatureObject::Messages::LookAtTargetChanged::Payload & payload)
{
	if (&payload == static_cast<const Object*>(Game::getPlayer()))
	{
		updateSelection();
	}
}

//----------------------------------------------------------------------

void SwgCuiGroup::updateSelection()
{
	CreatureObject * const player = Game::getPlayerCreature();
	if (!player)
		return;

	const CachedNetworkId groupId(player->getGroup());
	const GroupObject * const group = safe_cast<const GroupObject *>(groupId.getObject());
	if (!group)
		return;

	CachedNetworkId lookAtTarget(player->getLookAtTarget());

	if (!lookAtTarget.getObject())
		lookAtTarget = NetworkId::cms_invalid;
	
	for (MfdStatusVector::const_iterator it = m_mfds->begin(); it != m_mfds->end(); ++it)
	{
		SwgCuiMfdStatus const * const mfd = NON_NULL(it->second.pointer());
		CachedNetworkId const & mfdId = mfd->getTarget();
		
		UIImage * image = safe_cast<UIImage *>(mfd->getPage().GetObjectFromPath(IconFocusName.c_str(), TUIImage));
		
		if (mfdId == NetworkId::cms_invalid || mfdId != lookAtTarget)
		{
			if (image)
				image->SetVisible(false);
		}
		else 
		{
			if (image)
			{
				image->SetVisible(true);
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiGroup::OnWidgetRectChanging(UIWidget * context, UIRect & targetRect)
{
	if (context == &getPage())
	{
		const UIRect & rect = context->GetRect();
		if (rect.top != targetRect.top || rect.bottom != targetRect.bottom)
			m_heightManuallySet = true;
	}
}

//----------------------------------------------------------------------

void SwgCuiGroup::onLootChanged(const GroupObject & group)
{
	CreatureObject const * const player = Game::getPlayerCreature();
	if (player != 0)
	{
		if (group.getNetworkId() == player->getGroup())
		{
			updateGroupMembers();
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiGroup::addMember(NetworkId const & member, std::string const & memberName, Unicode::String const & prefixString)
{
	MfdStatusVector::iterator const itMember = m_mfds->find(member);
	bool const hasMember = (itMember != m_mfds->end());

	UIPage * mfdStatusPage = NULL;
	SwgCuiMfdStatus * mfdStatus = NULL;

	if (hasMember) 
	{
		mfdStatus = itMember->second.pointer();
		mfdStatusPage = &(mfdStatus->getPage());
		mfdStatus->deactivate();
	}
	else
	{
		mfdStatusPage = UI_ASOBJECT(UIPage, m_sample->DuplicateObject());
		NOT_NULL(mfdStatusPage);

		getPage().AddChild(mfdStatusPage);
		getPage().MoveChild(mfdStatusPage, UIBaseObject::Bottom);
		
		
		mfdStatusPage->Link();
		
		


		UIScalar const windowHeight = getPage().GetSizeIncrement().y;
		mfdStatusPage->SetLocation(0, static_cast<UIScalar>(m_mfds->size()) * windowHeight);

		mfdStatus = SwgCuiStatusFactory::createStatusPage(static_cast<Game::SceneType>(m_sceneType), *mfdStatusPage, true);
		NOT_NULL(mfdStatus);
		mfdStatus->setShowRange(true);
		mfdStatus->setPageToLock(&getPage());

		mfdStatusPage->SetVisible(true);
		mfdStatusPage->SetEnabled(true);
		mfdStatusPage->SetAbsorbsInput(false);
		mfdStatusPage->SetUserMovable(false);
		mfdStatusPage->SetSelectable(false);
		mfdStatusPage->AddCallback(this);
		mfdStatusPage->SetContextCapable(true, true);
		mfdStatusPage->SetContextToParent(true);
		mfdStatusPage->SetPropertyNarrow(Properties::MemberId, member.getValueString());

		mfdStatus->setTarget(member);

		m_mfds->insert(std::make_pair(member, mfdStatus));
		
	}

	// Update text.
	mfdStatus->setTargetNamePrefix(prefixString);

	ClientObject const * const memberObject = safe_cast<ClientObject *>(NetworkIdManager::getObjectById(member));
	if (!memberObject)
	{
		const Unicode::String & s_out_of_range_suffix = CuiStringIdsGroup::out_of_range_suffix.localize();
		mfdStatus->setTargetName(StringId::decodeString(Unicode::narrowToWide(memberName)) + Unicode::narrowToWide("\\#aaaaaa") + s_out_of_range_suffix + Unicode::narrowToWide("\\#."));
		mfdStatus->setObjectName(Unicode::narrowToWide(memberName));
	}

	mfdStatus->activate();
}

//----------------------------------------------------------------------

void SwgCuiGroup::clearMembers()
{
	MfdStatusVector::iterator iter = m_mfds->begin();
	MfdStatusVector::iterator iterEnd = m_mfds->end();
	for (; iter != iterEnd; ++iter)
	{
		CuiMediator *mediator = iter->second;
		mediator->getPage().RemoveCallback(this);
		getPage().RemoveChild(&mediator->getPage());
	}

	m_mfds->clear();
	
	resizeGroupWindow(1);
	updateSelection();
}

//----------------------------------------------------------------------

void SwgCuiGroup::activateMFDs(bool active)
{
	MfdStatusVector::iterator iter = m_mfds->begin();
	MfdStatusVector::iterator iterEnd = m_mfds->end();
	for (; iter != iterEnd; ++iter)
	{
		active ? iter->second->activate() : iter->second->deactivate();
	}
}

//----------------------------------------------------------------------

void SwgCuiGroup::resizeGroupWindow(int numElements)
{
	// RLS TODO - Fix composite code so we can use a composite here.
	UIScalar const windowSize = clamp(s_minGroupSize, static_cast<UIScalar>(numElements), s_maxGroupSize);
	UIScalar const height = getPage().GetSizeIncrement().y * windowSize;

	UISize maxSize = getPage().GetMaximumSize();
	
	maxSize.y = height;
	maxSize.x = getPage().GetSize().x;

	if (s_groupPickupTimerActive)
	{
		UIPoint timerLoc = m_timerPage->GetLocation();
		timerLoc.y = maxSize.y;

		m_timerPage->SetLocation(timerLoc);

		maxSize.y += m_timerPage->GetSize().y;
	}

	getPage().SetMaximumSize(maxSize);
	getPage().SetSize(maxSize);
	getPage().SetPackDirty(true);
}

//----------------------------------------------------------------------

void SwgCuiGroup::onGroupPickupPointTimerChanged(GroupObject const & group)
{
	int left = group.getSecondsLeftOnGroupPickup();

	if (left > 0)
	{
		s_groupPickupTimerActive = true;
	}
	else
	{
		s_groupPickupTimerActive = false;
	}

	resizeGroupWindow(m_mfds->size());
}

//----------------------------------------------------------------------

void SwgCuiGroup::generateMemberPopup(UIPopupMenu * pop)
{
	if (m_mfds->empty())
		return;

	const CreatureObject * const player = Game::getPlayerCreature();
	if (!player)
		return;

	const GroupObject * const group = safe_cast<const GroupObject *>(player->getGroup().getObject());

	if (!group)
		return;

	const GroupObject::GroupMemberVector gmv = group->getGroupMembers();
	if (gmv.empty())
		return;
   
	pop->AddItem(PopupIds::group_tell, Cui::MenuInfoTypes::getLocalizedLabel(Cui::MenuInfoTypes::GROUP_TELL, 0));

	//-- player is the leader
	if (gmv.front().first == player->getNetworkId())
	{
		pop->AddItem(PopupIds::group_kick, Cui::MenuInfoTypes::getLocalizedLabel(Cui::MenuInfoTypes::GROUP_KICK, 0));

		if (group->getLootMaster() != m_popupMemberId)
			pop->AddItem(PopupIds::group_masterlooter, Cui::MenuInfoTypes::getLocalizedLabel(Cui::MenuInfoTypes::GROUP_MASTER_LOOTER, 0));

		pop->AddItem(PopupIds::group_makeleader, Cui::MenuInfoTypes::getLocalizedLabel(Cui::MenuInfoTypes::GROUP_MAKE_LEADER, 0));
	}
	//-- player is NOT the leader
	else
	{
		if (m_popupMemberId == player->getNetworkId())
			pop->AddItem(PopupIds::group_leave, Cui::MenuInfoTypes::getLocalizedLabel(Cui::MenuInfoTypes::GROUP_LEAVE, 0));
	}
}

//======================================================================
