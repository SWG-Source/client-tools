//======================================================================
//
// SwgCuiShipView.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiShipView_H
#define INCLUDED_SwgCuiShipView_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"
#include "clientGame/ShipObject.h"

//----------------------------------------------------------------------

class CuiWidget3dObjectListViewer;
class GroupMember;
class GroupObject;
class ShipObject;
class UIButton;
class UIText;

namespace MessageDispatch
{
	class Callback;
}

//----------------------------------------------------------------------

class SwgCuiShipView :
public CuiMediator,
public UIEventCallback
{
public:
	explicit SwgCuiShipView(UIPage & page);
	void setShip(ShipObject * ship);
	virtual void OnButtonPressed(UIWidget * context);
	void setTerminal(NetworkId const & nid);
	void onComponentsChanged (ShipObject::Messages::ComponentsChanged::Payload & ship);
	virtual void update (float deltaTimeSecs);

protected:
	virtual void performActivate();
	virtual void performDeactivate();

private:
	void clearUi();
	void populateUi();
	void setupViewer();
	void populateUiText();
	void populateGroupButton();

private:
	//disabled
	SwgCuiShipView(SwgCuiShipView const & rhs);
	SwgCuiShipView & operator= (SwgCuiShipView const & rhs);

private:
	virtual ~SwgCuiShipView();

private:
	MessageDispatch::Callback * m_callback;
	Watcher<ShipObject> m_ship;
	CuiWidget3dObjectListViewer * m_shipViewer;
	UIText * m_title;
	UIText * m_statsAttribs;
	UIText * m_statsValues;
	UIButton * m_manageButton;
	UIButton * m_groupButton;
	UIButton * m_launchButton;
	UIButton * m_travelButton;
};

//======================================================================

#endif
