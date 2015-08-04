// ======================================================================
//
// GroupManager.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef _GroupManager_H_
#define _GroupManager_H_

// ======================================================================

class CreatureObject;
class NetworkId;

// ======================================================================

class GroupManager
{
public:

	typedef stdset<CreatureObject *>::fwd CreatureSet;

	static void handleGroupChange        (CreatureObject & creature, NetworkId const &oldGroup, NetworkId const &newGroup);
	static void getGroupMembers          (CreatureObject & creature, CreatureSet &members);
	static void handleGroupInviterChange (CreatureObject & creature);
	static void handleInviteResponse     (bool acceptedInvite);
	static void handleInviteToLaunchIntoSpace(NetworkId const & inviterId);

	// these reference count the number of space launching UIs that are open.
	// you are only allowed to accept invitations when none are open
	static void openedLaunchIntoSpaceUI();
	static void closedLaunchIntoSpaceUI();
};

// ======================================================================

#endif // _GroupManager_H_

