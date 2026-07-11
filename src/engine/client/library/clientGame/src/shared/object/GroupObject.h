// ======================================================================
//
// GroupObject.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_INCLUDED_GroupObject_H
#define	_INCLUDED_GroupObject_H

// ======================================================================

#include "Archive/AutoDeltaVector.h"
#include "clientGame/UniverseObject.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkIdArchive.h"
#include "sharedMathArchive/VectorArchive.h"

// ======================================================================

class SharedGroupObjectTemplate;

// ======================================================================

class GroupObject: public UniverseObject
{
public:

	typedef std::pair<NetworkId, std::string> GroupMember;
	typedef stdvector<GroupMember>::fwd GroupMemberVector;
	typedef std::pair<NetworkId /*ship*/, int /*formation slot*/> GroupShipFormationMember;

	enum LootRule
	{
		LR_freeForAll = 0,
		LR_masterLooter,
		LR_lottery,
		LR_random,
		LR_numberOfLootRules
	};

	struct Messages
	{
		struct MembersChanged
		{
			typedef GroupObject Payload;
		};

		struct MemberAdded
		{
			typedef std::pair<GroupObject *, const GroupMember *> Payload;
		};

		struct MemberRemoved
		{
			typedef std::pair<GroupObject *, const GroupMember *> Payload;
		};

		struct MemberShipsChanged
		{
			typedef GroupObject Payload;
		};

		struct MemberShipAdded
		{
			typedef std::pair<GroupObject *, const GroupShipFormationMember *> Payload;
		};

		struct MemberShipRemoved
		{
			typedef std::pair<GroupObject *, const GroupShipFormationMember *> Payload;
		};

		struct NameChanged
		{
			typedef GroupObject Payload;
		};

		struct FormationNameCrcChanged
		{
			typedef GroupObject Payload;
		};

		struct LootMasterChanged
		{
			typedef GroupObject Payload;
		};

		struct LootRuleChanged
		{
			typedef GroupObject Payload;
		};

		struct GroupPickupTimerChanged 
		{
			typedef GroupObject Payload; 
		};
	};

	struct Callbacks
	{
		struct FormationNameCrcChanged
		{
			void modified(GroupObject &target, const uint32 &oldValue, const uint32 &newValue, bool local);
		};

		struct LootMasterChanged
		{
			void modified(GroupObject & target, const NetworkId & old, const NetworkId & value, bool local);
		};

		struct LootRuleChanged
		{
			void modified(GroupObject & target, const uint32 &oldValue, const uint32 &newValue, bool local);
		};

		struct GroupPickupTimerServerEpochChanged
		{
			void modified(GroupObject &target, const std::pair<int32, int32> &oldValue, const std::pair<int32, int32> &newValue, bool local);
		};
	};

	/**
	 * Structure to store the results of getMembersInShipsList
	 */
	struct ShipMemberData
	{
		NetworkId m_ship;
		int m_playerCount;
		bool m_leader;
	};

	explicit GroupObject(SharedGroupObjectTemplate const *sharedGroupObjectTemplate);
	~GroupObject();

	const GroupMemberVector &   getGroupMembers () const;
	const std::string &         getGroupName () const;
	uint32                      getFormationNameCrc () const;

	const GroupMember *         findMember (const NetworkId & id, bool & isLeader) const;
	const GroupMember *         findMember (const std::string & name, bool & isLeader) const;

	const GroupMember *         findMemberByIndex (int index, const NetworkId & ignoreId) const;
	NetworkId                   findShipByIndex (int index) const;
	int                         getIndexForMember (NetworkId const & id) const;

	void                        membersOnChanged   ();
	void                        membersOnErase     (const unsigned int n, const GroupMember & member);
	void                        membersOnInsert    (const unsigned int n, const GroupMember & member);

	void memberShipsOnChanged();
	void memberShipsOnErase(const unsigned int n, const GroupShipFormationMember & member);
	void memberShipsOnInsert(const unsigned int n, const GroupShipFormationMember & member);

	int16                       getGroupLevel      () const;

	const NetworkId &           getLeader          () const;

	NetworkId const & getShipFromMember(NetworkId const & id) const;
	int getShipFormationSlotFromMember(NetworkId const & id) const;
	GroupMember const * findMemberFromFormationSlot(int formationSlot) const;
	std::vector<ShipMemberData> getMembersInShipsList() const;

	bool isGroupMember(NetworkId const & networkId) const;

	NetworkId const & getLootMaster() const;
	int getLootRule() const;

	std::pair<time_t, time_t> const & getGroupPickupTimer() const;
	unsigned int getSecondsLeftOnGroupPickup() const;
	unsigned int getGroupPickupDurationSeconds() const;

	std::string const & getGroupPickupLocationPlanet() const;

private:
	GroupObject();
	GroupObject &operator= (GroupObject const &rhs);
	GroupObject(GroupObject const &source);

private:
	Archive::AutoDeltaVariable<std::string> m_groupName;
	Archive::AutoDeltaVector<GroupMember, GroupObject> m_groupMembers;
	Archive::AutoDeltaVector<GroupShipFormationMember, GroupObject> m_groupShipFormationMembers;
	Archive::AutoDeltaVariable<int16> m_groupLevel;
	Archive::AutoDeltaVariableCallback<uint32, Callbacks::FormationNameCrcChanged, GroupObject> m_formationNameCrc;
	Archive::AutoDeltaVariableCallback<NetworkId, Callbacks::LootMasterChanged, GroupObject> m_lootMaster;
	Archive::AutoDeltaVariableCallback<uint32, Callbacks::LootRuleChanged, GroupObject> m_lootRule;

	// <start time_t, end time_t> (may not be 0, but if the end time is < the current time, it means no group pickup is currently active)
	// these times are in the server Epoch; they are convert to the client Epoch and stored in m_groupPickupTimerClientEpoch
	Archive::AutoDeltaVariableCallback<std::pair<int32, int32>, Callbacks::GroupPickupTimerServerEpochChanged, GroupObject> m_groupPickupTimerServerEpoch;
	std::pair<time_t, time_t> m_groupPickupTimerClientEpoch;

	Archive::AutoDeltaVariable<std::pair<std::string, Vector> > m_groupPickupLocation; // planet/zone name, and (x,y,z) world location
};

//----------------------------------------------------------------------

inline const GroupObject::GroupMemberVector & GroupObject::getGroupMembers () const
{
	return m_groupMembers.get ();
}

//----------------------------------------------------------------------

inline const std::string & GroupObject::getGroupName () const
{
	return m_groupName.get ();
}

//----------------------------------------------------------------------

inline int16 GroupObject::getGroupLevel () const
{
	return m_groupLevel.get ();
}

//----------------------------------------------------------------------

inline uint32 GroupObject::getFormationNameCrc() const
{
	return m_formationNameCrc.get ();
}

// ----------------------------------------------------------------------

inline std::pair<time_t, time_t> const & GroupObject::getGroupPickupTimer() const
{
	return m_groupPickupTimerClientEpoch;
}

// ----------------------------------------------------------------------

inline std::string const & GroupObject::getGroupPickupLocationPlanet() const
{
	return m_groupPickupLocation.get().first;
}

// ======================================================================

#endif	// _INCLUDED_GroupObject_H
