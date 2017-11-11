//======================================================================
//
// SwgCuiSpaceGroup.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiSpaceGroup_H
#define INCLUDED_SwgCuiSpaceGroup_H

//======================================================================


#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"
#include "swgClientUserInterface/SwgCuiLockableMediator.h"

//----------------------------------------------------------------------

class UIPage;
class UIScrollbar;
class UIVolumePage;
class SwgCuiMfdStatus;
class UIButton;
class UIComposite;
class GroupObject;
class GroupMember;
class GroupShipFormationMember;
class CreatureObject;
class TangibleObject;
class UIImage;

namespace MessageDispatch
{
	class Callback;
};

//----------------------------------------------------------------------

class SwgCuiSpaceGroup
: public SwgCuiLockableMediator
{
public:

	typedef std::pair<NetworkId, std::string> GroupMember;

	explicit SwgCuiSpaceGroup(UIPage & page);

	void onMembersChanged(GroupObject const & group);
	void onGroupChanged(CreatureObject const & creature);
	void onMemberAdded(std::pair<GroupObject *, const GroupMember *> const & payload);
	void onMemberRemoved(std::pair<GroupObject *, const GroupMember *> const & payload);
	void onMemberShipsChanged(GroupObject const & group);
	void onMemberShipAdded(std::pair<GroupObject *, const GroupShipFormationMember *> const & payload);
	void onMemberShipRemoved(std::pair<GroupObject *, const GroupShipFormationMember *> const & payload);
	bool OnMessage (UIWidget *context, const UIMessage & msg);
	void OnPopupMenuSelection (UIWidget * context);
	virtual void update(float deltaTimeSecs);
	
protected:
	virtual void performActivate();
	virtual void performDeactivate();
	virtual void OnButtonPressed(UIWidget * context);
	void populateData();

private:
	~SwgCuiSpaceGroup();
	SwgCuiSpaceGroup();
	SwgCuiSpaceGroup(SwgCuiSpaceGroup const &);
	SwgCuiSpaceGroup & operator=(SwgCuiSpaceGroup const &);

private:
	MessageDispatch::Callback * m_callback;
	UIComposite * m_members;
	UIPage * m_sample;
	UIButton * m_collapse;
	UIButton * m_expand;
	NetworkId m_popupSelection;

	int m_numberOfMembers;
	bool m_collapsed;
	bool m_dirty;
};

//======================================================================

#endif
