//===================================================================
//
// SwgCuiSpaceGroupLaunch.h
// copyright 2004, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_SwgCuiSpaceGroupLaunch_H
#define INCLUDED_SwgCuiSpaceGroupLaunch_H

//===================================================================

#include "UIEventCallback.h"
#include "clientUserInterface/CuiMediator.h"

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"
#include "clientGame/ShipObject.h"

//===================================================================

class CreatureObject;
class GroupObject;
class GroupMember;
class UIComposite;
class UIText;

namespace MessageDispatch
{
	class Callback;
};

//===================================================================

class SwgCuiSpaceGroupLaunch
: public UIEventCallback
, public CuiMediator
{
public:
	typedef stdset<NetworkId>::fwd NetworkIdSet;
	typedef Watcher<CreatureObject> CreatureWatcher;
	typedef stdset<CreatureWatcher>::fwd CreatureWatcherSet;

public:
	explicit SwgCuiSpaceGroupLaunch(UIPage & page);

	static NetworkIdSet const & getAcceptedMembers();
	static void resetAcceptedMembers();

	void setShip(ShipObject *);

	void onMembersChanged(GroupObject const & group);
	void onGroupChanged(CreatureObject const & creature);
	void onMemberAdded(std::pair<GroupObject *, const GroupMember *> const & payload);
	void onMemberRemoved(std::pair<GroupObject *, const GroupMember *> const & payload);
	void onReceivedInvitation(std::pair<NetworkId, bool /*accepted*/> const & answer);

protected:
	virtual void performActivate();
	virtual void performDeactivate();
	virtual void update(float deltaTimeSeconds);
	virtual void OnButtonPressed(UIWidget* context);

private:
	SwgCuiSpaceGroupLaunch();
	SwgCuiSpaceGroupLaunch(const SwgCuiSpaceGroupLaunch &);
	SwgCuiSpaceGroupLaunch & operator=(const SwgCuiSpaceGroupLaunch &);
	virtual ~SwgCuiSpaceGroupLaunch();

	void populate(CreatureWatcherSet const & membersInSameCell);
	void sendInviteToPlayer(NetworkId const & Id) const;
	void kickoutInvitedPlayer(NetworkId const & Id);
	void onDeclinedInvitation(NetworkId const & Id);
	void reset();

private:
	MessageDispatch::Callback * m_callback;

	Watcher<ShipObject> m_ship;

	UIButton * m_buttonCancel;
	UIButton * m_buttonOk;
	UIPage * m_sample;
	UIComposite * m_members;
	UIText * m_inviteRule;

	bool m_dirty;

	typedef stdmap<NetworkId, UIPage *>::fwd MemberPagesById;
	MemberPagesById m_memberPagesById;

	CreatureWatcherSet m_membersInSameCell;

	float m_elapsedTimeSeconds;
};

//===================================================================

#endif
