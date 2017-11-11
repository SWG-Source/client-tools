//======================================================================
//
// SwgCuiShipChoose.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiShipChoose_H
#define INCLUDED_SwgCuiShipChoose_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"
#include "clientGame/PlayerCreatureController.h"
#include "clientGame/ShipObject.h"

//----------------------------------------------------------------------

class ShipObject;
class UIButton;
class UIComposite;

namespace MessageDispatch
{
	class Callback;
}

//----------------------------------------------------------------------

class SwgCuiShipChoose :
public CuiMediator,
public UIEventCallback
{
public:
	explicit SwgCuiShipChoose(UIPage & page);
	virtual void OnButtonPressed(UIWidget * context);
	void setTerminal(NetworkId const & nid);
	void onShipParkingDataReceived(PlayerCreatureController::Messages::ShipParkingDataReceived::Payload const & payload);

protected:
	virtual void performActivate();
	virtual void performDeactivate();

private:
	void clearUI();
	void populateUI();
	void addShipCompPage(ShipObject & ship);

private:
	//disabled
	SwgCuiShipChoose(SwgCuiShipChoose const & rhs);
	SwgCuiShipChoose & operator= (SwgCuiShipChoose const & rhs);

private:
	virtual ~SwgCuiShipChoose();

private:
	MessageDispatch::Callback * m_callback;
	UIComposite * m_shipComp;
	UIComposite * m_sample;
	UIButton * m_closeButton;
	bool m_parkingDataReceived;
};

//======================================================================

#endif
