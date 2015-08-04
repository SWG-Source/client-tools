//=============================================================================
//
// SwgCuiSpaceFlyOutPage.h
// copyright (c) 2004 Sony Online Entertainment
//
//=============================================================================

#ifndef INCLUDED_SwgCuiSpaceFlyOutPage_H
#define INCLUDED_SwgCuiSpaceFlyOutPage_H

//=============================================================================


#include "clientGame/ShipWeaponGroupManager.h"
#include "sharedMessageDispatch/Receiver.h"
#include "swgClientUserInterface/SwgCuiLockableMediator.h"

//-----------------------------------------------------------------------------

class ShipDamageMessage;
class ShipObject;

//-----------------------------------------------------------------------------

namespace MessageDispatch
{
	class Callback;
}

//-----------------------------------------------------------------------------

class SwgCuiSpaceFlyOutPage :
public SwgCuiLockableMediator,
public MessageDispatch::Receiver
{
public:
	SwgCuiSpaceFlyOutPage(UIPage & page);
	
	virtual void performActivate();
	virtual void performDeactivate();
	virtual void update(float updateDeltaSeconds);

	void onShipDamaged(const ShipDamageMessage & shipDamage);
	void onWeaponGroupChanged(const std::pair<ShipWeaponGroupManager::GroupType, int> & weaponGroup);
	void onCargoChanged(ShipObject & ship);

	virtual void OnButtonPressed (UIWidget *context);

	void receiveMessage(const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);

protected:
	virtual ~SwgCuiSpaceFlyOutPage();

protected:
	MessageDispatch::Callback * m_callback;

private:
	SwgCuiSpaceFlyOutPage();
	SwgCuiSpaceFlyOutPage(const SwgCuiSpaceFlyOutPage &);
	SwgCuiSpaceFlyOutPage & operator=(const SwgCuiSpaceFlyOutPage &);

private:
};

//=============================================================================

#endif
