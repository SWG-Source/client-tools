//===================================================================
//
// SwgCuiSpaceAssignPlayerFormation.h
// copyright 2004, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_SwgCuiSpaceAssignPlayerFormation_H
#define INCLUDED_SwgCuiSpaceAssignPlayerFormation_H

//===================================================================

#include "UIEventCallback.h"
#include "clientUserInterface/CuiMediator.h"

#include <vector>

//===================================================================

class CreatureObject;
class GroupObject;
class GroupMember;
class GroupShipFormationMember;
class UIButton;
class UIText;
class UIVolumePage;

namespace MessageDispatch
{
	class Callback;
};

//===================================================================

class SwgCuiSpaceAssignPlayerFormation
: public UIEventCallback
, public CuiMediator
{
public:

	explicit SwgCuiSpaceAssignPlayerFormation(UIPage& page);
	virtual void OnButtonPressed(UIWidget* context);
	virtual bool OnMessage(UIWidget *context, UIMessage const & msg);

	void onMembersChanged(const GroupObject & group);
	void onGroupChanged(const CreatureObject & creature);
	void onMemberAdded(const std::pair<GroupObject *, const GroupMember *> & payload);
	void onMemberRemoved(const std::pair<GroupObject *, const GroupMember *> & payload);

	void onMemberShipsChanged(const GroupObject & group);
	void onMemberShipAdded(const std::pair<GroupObject *, const GroupShipFormationMember *> & payload);
	void onMemberShipRemoved(const std::pair<GroupObject *, const GroupShipFormationMember *> & payload);

protected:

	virtual void performActivate();
	virtual void performDeactivate();

private:

	SwgCuiSpaceAssignPlayerFormation();
	SwgCuiSpaceAssignPlayerFormation(const SwgCuiSpaceAssignPlayerFormation &);
	SwgCuiSpaceAssignPlayerFormation & operator=(const SwgCuiSpaceAssignPlayerFormation &);
	virtual ~SwgCuiSpaceAssignPlayerFormation();

	void previousFormation();
	void nextFormation();
	void associatePlayerWithShip(UIWidget * const playerWidget, UIWidget * const shipWidget) const;

	void populateFormationNames();
	void populateGroupMembersAndShips();
	void updateShipLayoutForCurrentFormation();

	void updateServer() const;
private:
	MessageDispatch::Callback * m_callback;

	UIButton * m_done;
	UIVolumePage * m_volumeMembers;
	UIVolumePage * m_volumeFormations;
	UIPage * m_formgraphShips;
	UIText * m_formationNameText;
	UIButton * m_formationButtonNameSample;
	UIPage * m_playerSample;
	UIPage * m_shipSample;

	typedef std::vector<UIPage *> PageList;
	PageList m_playerData;
	PageList m_playerShips;

	typedef stdvector<std::string>::fwd  FormationNameList;
	FormationNameList m_formationNames;
	int m_currentFormation;
	float m_elapsedTime;
};

//===================================================================

#endif
