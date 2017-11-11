//===================================================================
//
// SwgCuiWeaponGroupAssignment.h
// copyright 2004, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_SwgCuiWeaponGroupAssignment_H
#define INCLUDED_SwgCuiWeaponGroupAssignment_H

//===================================================================

#include "UIEventCallback.h"
#include "clientUserInterface/CuiMediator.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"

class ShipObject;
class UIList;
class UIText;

namespace MessageDispatch
{
	class Callback;
};

//===================================================================

class SwgCuiWeaponGroupAssignment : public UIEventCallback, public CuiMediator
{
public:
	explicit SwgCuiWeaponGroupAssignment(UIPage& page);

	void OnGenericSelectionChanged (UIWidget * context);
	bool OnMessage (UIWidget * context, UIMessage const & msg);
	void OnButtonPressed (UIWidget * context);
	virtual void performActivate ();
	virtual void performDeactivate ();

	void onWeaponGroupsResetForShip(NetworkId const & shipId);
	void updateWeaponDescriptions();

private:
	virtual ~SwgCuiWeaponGroupAssignment();

	ShipObject * getShipObject() const;

private:
	SwgCuiWeaponGroupAssignment();
	SwgCuiWeaponGroupAssignment(const SwgCuiWeaponGroupAssignment&);
	SwgCuiWeaponGroupAssignment& operator=(const SwgCuiWeaponGroupAssignment&);

private:
	void initializeUi();
	void initializeGroupListUi();
	void setSelectedWeaponGroup(int groupNumber);

private:
	UIList * m_weaponGroupList;
	UIList * m_allWeaponsList;
	UIList * m_weaponsInSelectedGroupList;
	UIButton * m_addButton;
	UIButton * m_removeButton;
	NetworkId m_shipId;
	int m_selectedGroup;
	MessageDispatch::Callback * m_callback;
	UIText * m_textAttribsTop;
	UIText * m_textAttribsBottom;
};

//===================================================================

#endif
