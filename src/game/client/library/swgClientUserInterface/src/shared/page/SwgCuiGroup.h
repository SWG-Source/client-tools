//======================================================================
//
// SwgCuiGroup.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiGroup_H
#define INCLUDED_SwgCuiGroup_H

//======================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"
#include "swgClientUserInterface/SwgCuiLockableMediator.h"

//----------------------------------------------------------------------

class UIPage;
class UIScrollbar;
class UIVolumePage;
class SwgCuiBuffDisplay;
class SwgCuiMfdStatus;
class UIButton;
class UIComposite;
class UIText;
class GroupObject;
class CreatureObject;
class TangibleObject;
class UIImage;
class UIButton;

namespace MessageDispatch
{
	class Callback;
};

//----------------------------------------------------------------------

class SwgCuiGroup :
public SwgCuiLockableMediator
{
public:

	typedef std::pair<NetworkId, std::string> GroupMember;

	static Unicode::String buildNamePrefix(GroupObject const * const group, NetworkId const & memberToTest);

	explicit            SwgCuiGroup        (UIPage & page);
	void                update             (float deltaTimeSecs);
	void                updateGroupMembers ();

	void                onMembersChanged   (const GroupObject & group);
	void                onGroupChanged     (const CreatureObject & creature);

	void                onMemberAdded      (const std::pair<GroupObject *, const GroupMember *> & payload);
	void                onMemberRemoved    (const std::pair<GroupObject *, const GroupMember *> & payload);

	bool                OnMessage            (UIWidget *Context, const UIMessage & msg);
	void                OnPopupMenuSelection (UIWidget * context);

	void                onLookAtTargetChanged (const CreatureObject & payload);

	virtual void        OnWidgetRectChanging (UIWidget * context, UIRect & targetRect);
	
	void                onLootChanged(const GroupObject & group);

	void				onGroupPickupPointTimerChanged(GroupObject const & group);

protected:
	void                performActivate   ();
	void                performDeactivate ();

	void                clear             ();
private:

	void                updateSelection ();

	void addMember(NetworkId const & member, std::string const & memberName, Unicode::String const & prefixString);
	void clearMembers();
	void activateMFDs(bool activate);
	void resizeGroupWindow(int numElements);
	void generateMemberPopup(UIPopupMenu * pop);

						~SwgCuiGroup ();

	                    SwgCuiGroup ();
	                    SwgCuiGroup (const SwgCuiGroup &);
	SwgCuiGroup &       operator= (const SwgCuiGroup &);

	UIPage *            m_timerPage;
	UIPage *            m_timerBar;
	UIText *			m_timerText;

	UISmartPointer<UIPage> m_sample;

	typedef stdmap<NetworkId /*player*/, UISmartPointer<SwgCuiMfdStatus> >::fwd MfdStatusVector;
	MfdStatusVector *   m_mfds;

	MessageDispatch::Callback * m_callback;

	bool                m_mouseIsDown;
	NetworkId           m_popupMemberId;

	bool                m_heightManuallySet;

	int m_sceneType; // Game::SceneType
};

//======================================================================

#endif
