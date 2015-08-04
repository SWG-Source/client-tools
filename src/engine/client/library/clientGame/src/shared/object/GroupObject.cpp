// ======================================================================
//
// GroupObject.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/GroupObject.h"

#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientGame/ShipObject.h"
#include "clientUserInterface/CuiStringIdsGroup.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "sharedGame/SharedGroupObjectTemplate.h"
#include "sharedFoundation/Crc.h"
#include "sharedMessageDispatch/Transceiver.h"

// ======================================================================

namespace
{
	namespace Transceivers
	{
		MessageDispatch::Transceiver<const GroupObject::Messages::MembersChanged::Payload &,                   GroupObject::Messages::MembersChanged>             membersChanged;
		MessageDispatch::Transceiver<const GroupObject::Messages::MemberAdded::Payload &,                      GroupObject::Messages::MemberAdded>                memberAdded;
		MessageDispatch::Transceiver<const GroupObject::Messages::MemberRemoved::Payload &,                    GroupObject::Messages::MemberRemoved>              memberRemoved;
		MessageDispatch::Transceiver<const GroupObject::Messages::FormationNameCrcChanged::Payload &,          GroupObject::Messages::FormationNameCrcChanged>    formationNameCrcChanged;
		MessageDispatch::Transceiver<const GroupObject::Messages::MemberShipsChanged::Payload &, GroupObject::Messages::MemberShipsChanged> memberShipsChanged;
		MessageDispatch::Transceiver<const GroupObject::Messages::MemberShipAdded::Payload &, GroupObject::Messages::MemberShipAdded> memberShipAdded;
		MessageDispatch::Transceiver<const GroupObject::Messages::MemberShipRemoved::Payload &, GroupObject::Messages::MemberShipRemoved> memberShipRemoved;
		MessageDispatch::Transceiver<const GroupObject::Messages::LootMasterChanged::Payload &, GroupObject::Messages::LootMasterChanged> lootMasterChanged;
		MessageDispatch::Transceiver<const GroupObject::Messages::LootRuleChanged::Payload &, GroupObject::Messages::LootRuleChanged> lootRuleChanged;
		MessageDispatch::Transceiver<const GroupObject::Messages::GroupPickupTimerChanged::Payload &, GroupObject::Messages::GroupPickupTimerChanged> groupPickupTimerChanged; 
	}
}

//----------------------------------------------------------------------

void GroupObject::Callbacks::FormationNameCrcChanged::modified(GroupObject &target, const uint32 &, const uint32 &, bool )
{
	Transceivers::formationNameCrcChanged.emitMessage(target);
}

//----------------------------------------------------------------------

void GroupObject::Callbacks::LootMasterChanged::modified(GroupObject & target, const NetworkId &, const NetworkId &, bool)
{
	Transceivers::lootMasterChanged.emitMessage(target);
}

//----------------------------------------------------------------------

void GroupObject::Callbacks::LootRuleChanged::modified(GroupObject &target, const uint32 &, const uint32 &, bool )
{
	Transceivers::lootRuleChanged.emitMessage(target);
}

//----------------------------------------------------------------------

void GroupObject::Callbacks::GroupPickupTimerServerEpochChanged::modified(GroupObject &target, const std::pair<int32, int32> & /*oldValue*/, const std::pair<int32, int32> &newValue, bool /*local*/)
{
	if ((newValue.first > 0) && (newValue.second > 0))
		target.m_groupPickupTimerClientEpoch = std::make_pair(static_cast<time_t>(newValue.first + GameNetwork::getServerEpochTimeDifference()), static_cast<time_t>(newValue.second + GameNetwork::getServerEpochTimeDifference()));
	else
		target.m_groupPickupTimerClientEpoch = std::make_pair(0, 0);

	Transceivers::groupPickupTimerChanged.emitMessage(target);
}

// ======================================================================

GroupObject::GroupObject(SharedGroupObjectTemplate const *t) :
	UniverseObject(t),
	m_groupName(),
	m_groupMembers(),
	m_groupShipFormationMembers(),
	m_groupLevel(0),
	m_formationNameCrc(Crc::crcNull),
	m_lootMaster(),
	m_lootRule(0),
	m_groupPickupTimerServerEpoch(std::make_pair(0, 0)),
	m_groupPickupTimerClientEpoch(std::make_pair(0, 0)),
	m_groupPickupLocation(std::make_pair("", Vector()))
{
	addSharedVariable_np(m_groupMembers);
	addSharedVariable_np(m_groupShipFormationMembers);
	addSharedVariable_np(m_groupName);
	addSharedVariable_np(m_groupLevel);
	addSharedVariable_np(m_formationNameCrc);
	addSharedVariable_np(m_lootMaster);
	addSharedVariable_np(m_lootRule);
	addSharedVariable_np(m_groupPickupTimerServerEpoch);
	addSharedVariable_np(m_groupPickupLocation);

	m_formationNameCrc.setSourceObject(this);
	m_lootMaster.setSourceObject(this);
	m_lootRule.setSourceObject(this);
	m_groupPickupTimerServerEpoch.setSourceObject(this);

	m_groupMembers.setOnChanged(this, &GroupObject::membersOnChanged);
	m_groupMembers.setOnInsert(this, &GroupObject::membersOnInsert);
	m_groupMembers.setOnErase(this, &GroupObject::membersOnErase);

	m_groupShipFormationMembers.setOnChanged(this, &GroupObject::memberShipsOnChanged);
	m_groupShipFormationMembers.setOnInsert(this, &GroupObject::memberShipsOnInsert);
	m_groupShipFormationMembers.setOnErase(this, &GroupObject::memberShipsOnErase);
}

// ----------------------------------------------------------------------

GroupObject::~GroupObject()
{
}

//----------------------------------------------------------------------

void GroupObject::membersOnChanged   ()
{
	Transceivers::membersChanged.emitMessage (*this);
}

//----------------------------------------------------------------------

void GroupObject::membersOnInsert   (const size_t, const GroupMember & member)
{
	Transceivers::memberAdded.emitMessage (Messages::MemberAdded::Payload (this, &member));

	CreatureObject const * const player = Game::getPlayerCreature();
	if (player && player->getGroup() == getNetworkId() && player->getNetworkId() != member.first)
	{
		Unicode::String result;
		CuiStringVariablesManager::process(CuiStringIdsGroup::other_joined_prose, StringId::decodeString(Unicode::narrowToWide(member.second)), Unicode::emptyString, Unicode::emptyString, result);
		CuiSystemMessageManager::sendFakeSystemMessage(result);
	}
}

//----------------------------------------------------------------------

void GroupObject::membersOnErase  (const size_t, const GroupMember & member)
{
	Transceivers::memberRemoved.emitMessage (Messages::MemberRemoved::Payload (this, &member));

	CreatureObject const * const player = Game::getPlayerCreature();
	if (player && player->getGroup() == getNetworkId() && player->getNetworkId() != member.first)
	{
		Unicode::String result;
		CuiStringVariablesManager::process(CuiStringIdsGroup::other_left_prose, StringId::decodeString(Unicode::narrowToWide(member.second)), Unicode::emptyString, Unicode::emptyString, result);
		CuiSystemMessageManager::sendFakeSystemMessage(result);
	}
}

//----------------------------------------------------------------------

void GroupObject::memberShipsOnChanged()
{
	Transceivers::memberShipsChanged.emitMessage(*this);
}

//----------------------------------------------------------------------

void GroupObject::memberShipsOnInsert(const size_t, const GroupShipFormationMember & member)
{
	Transceivers::memberShipAdded.emitMessage (Messages::MemberShipAdded::Payload (this, &member));
}

//----------------------------------------------------------------------

void GroupObject::memberShipsOnErase(const size_t, const GroupShipFormationMember & member)
{
	Transceivers::memberShipRemoved.emitMessage (Messages::MemberShipRemoved::Payload (this, &member));
}

//----------------------------------------------------------------------

const GroupObject::GroupMember * GroupObject::findMember (const NetworkId & id,           bool & isLeader) const
{
	int index = 0;
	const GroupMemberVector & members = m_groupMembers.get ();
	for (GroupMemberVector::const_iterator it = members.begin (); it != members.end (); ++it, ++index)
	{
		const GroupMember & mem = *it;
		if (mem.first == id)
		{
			isLeader = (index == 0);
			return &mem;
		}
	}

	return 0;
}

//----------------------------------------------------------------------

const GroupObject::GroupMember * GroupObject::findMember (const std::string & name, bool & isLeader) const
{
	std::string shortName;
	size_t dummySize = 0;
	if (!Unicode::getFirstToken (name, 0, dummySize, shortName))
		return 0;

	const Unicode::String & wideName = Unicode::narrowToWide (name);

	const size_t nameLen = shortName.size ();

	int index = 0;
	const GroupMemberVector & members = m_groupMembers.get ();
	for (GroupMemberVector::const_iterator it = members.begin (); it != members.end (); ++it, ++index)
	{
		const GroupMember & mem = *it;
		const std::string & groupMemberName = mem.second;

		if (!_strnicmp (shortName.c_str (), groupMemberName.c_str (), nameLen) ||
			Unicode::caseInsensitiveCompare (StringId::decodeString (Unicode::narrowToWide (groupMemberName)), wideName))
		{
			isLeader = (index == 0);
			return &mem;
		}
	}

	return 0;
}

//----------------------------------------------------------------------

const GroupObject::GroupMember * GroupObject::findMemberByIndex (int indexToFind, const NetworkId & ignoreId) const
{
	int index = 0;
	const GroupMemberVector & members = m_groupMembers.get ();
	for (GroupMemberVector::const_iterator it = members.begin (); it != members.end (); ++it)
	{
		const GroupMember & mem = *it;
		if (mem.first == ignoreId)
			continue;

		if (index == indexToFind)
			return &mem;

		++index;
	}

	return 0;
}

//----------------------------------------------------------------------

NetworkId GroupObject::findShipByIndex (int index) const
{
	std::vector<GroupObject::ShipMemberData> members = GroupObject::getMembersInShipsList();
	if ((index < 0) || (index >= static_cast<int>(members.size())))
		return NetworkId::cms_invalid;
	else
		return members[static_cast<size_t>(index)].m_ship;
}

//----------------------------------------------------------------------

int GroupObject::getIndexForMember (NetworkId const & id) const
{
	const GroupMemberVector & members = m_groupMembers.get ();
	int index = 0;
	for (GroupMemberVector::const_iterator it = members.begin (); it != members.end (); ++it, ++index)
	{
		if(it->first == id)
			return index;
	}
	return -1;
}

//----------------------------------------------------------------------

const NetworkId & GroupObject::getLeader          () const
{
	const GroupMemberVector & members = m_groupMembers.get ();
	if (!members.empty ())
		return members.front ().first;
	return NetworkId::cms_invalid;
}

//----------------------------------------------------------------------

NetworkId const & GroupObject::getShipFromMember(NetworkId const & id) const
{
	int const index = getIndexForMember(id);

	if ((index >= 0) && (index < static_cast<int>(m_groupShipFormationMembers.size())))
	{
		return m_groupShipFormationMembers[static_cast<unsigned int>(index)].first;
	}
	return NetworkId::cms_invalid;
}

//----------------------------------------------------------------------

int GroupObject::getShipFormationSlotFromMember(NetworkId const & id) const
{
	int const index = getIndexForMember(id);

	if ((index >= 0) && (index < static_cast<int>(m_groupShipFormationMembers.size())))
	{
		return m_groupShipFormationMembers[static_cast<unsigned int>(index)].second;
	}
	return -1;
}

//----------------------------------------------------------------------

GroupObject::GroupMember const * GroupObject::findMemberFromFormationSlot(int formationSlot) const
{
	unsigned int const numberOfGroupShipFormationMembers = m_groupShipFormationMembers.size();
	for (unsigned int i = 0; i < numberOfGroupShipFormationMembers; ++i)
	{
		if (m_groupShipFormationMembers[i].second == formationSlot)
		{
			if (i < m_groupMembers.size())
			{
				return &m_groupMembers[i];
			}
		}
	}
	return 0;
}

//----------------------------------------------------------------------

bool GroupObject::isGroupMember(NetworkId const & networkId) const
{
	bool dummy;
	return findMember(networkId, dummy) != 0;
}

//----------------------------------------------------------------------

/**
 * Get a list of all the ships containing group members, along with a count of 
 * the number of players in the ship.
 */
std::vector<GroupObject::ShipMemberData> GroupObject::getMembersInShipsList() const
{
	typedef std::map<NetworkId, ShipMemberData> ResultsType;
	ResultsType results;

	ShipObject const * const playerContainingShip = Game::getPlayerContainingShip();
	NetworkId const playerContainingShipId = (playerContainingShip != 0) ? playerContainingShip->getNetworkId() : NetworkId::cms_invalid;

	// Iterate through the list of members, make a distinct list of the ships
	{
		GroupMemberVector const & groupMembers = getGroupMembers ();
		for (GroupMemberVector::const_iterator i=groupMembers.begin(); i!=groupMembers.end(); ++i)
		{
			NetworkId const & shipId = getShipFromMember(i->first);
			if (shipId != NetworkId::cms_invalid)
			{
				ResultsType::iterator shipData = results.find(shipId);
				if (shipData == results.end())
				{
					ShipMemberData newShip;
					newShip.m_ship = shipId;
					newShip.m_playerCount = 0;
					newShip.m_leader = false;
					shipData = results.insert(std::make_pair(shipId, newShip)).first;
				}

				++(shipData->second.m_playerCount);
				if (i->first == getLeader())
					shipData->second.m_leader = true;
			}
		}
	}

	// Return the results in order, but move the player's ship to the front of the list
	{
		std::vector<ShipMemberData> sortedResults;
		sortedResults.reserve(results.size());

		for (ResultsType::const_iterator iter = results.begin(); iter != results.end(); ++iter)
		{
			if (iter->first == playerContainingShipId)
				IGNORE_RETURN(sortedResults.insert(sortedResults.begin(), iter->second));
			else
				sortedResults.push_back(iter->second);
		}

		return sortedResults;
	}
}

//----------------------------------------------------------------------

NetworkId const & GroupObject::getLootMaster() const
{
	return m_lootMaster.get();
}

//----------------------------------------------------------------------

int GroupObject::getLootRule() const
{
	return m_lootRule.get();
}

// ----------------------------------------------------------------------

unsigned int GroupObject::getSecondsLeftOnGroupPickup() const
{
	if ((m_groupPickupTimerClientEpoch.first > 0) && (m_groupPickupTimerClientEpoch.second > 0))
	{
		time_t const timeNow = ::time(NULL);
		if (m_groupPickupTimerClientEpoch.second > timeNow)
			return (m_groupPickupTimerClientEpoch.second - timeNow);
	}

	return 0;
}

// ----------------------------------------------------------------------

unsigned int GroupObject::getGroupPickupDurationSeconds() const
{
	if ((m_groupPickupTimerClientEpoch.first > 0) && (m_groupPickupTimerClientEpoch.second > 0) && (m_groupPickupTimerClientEpoch.second > m_groupPickupTimerClientEpoch.first))
		return (m_groupPickupTimerClientEpoch.second - m_groupPickupTimerClientEpoch.first);

	return 0;
}

// ======================================================================
