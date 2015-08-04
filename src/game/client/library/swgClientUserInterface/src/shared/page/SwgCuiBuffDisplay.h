//======================================================================
//
// SwgCuiBuffDisplay.h
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiBuffDisplay_H
#define INCLUDED_SwgCuiBuffDisplay_H

//======================================================================

#include "sharedMessageDispatch/Receiver.h"
#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"
#include "sharedObject/CachedNetworkId.h"
#include "swgSharedUtility/Attributes.def"

//----------------------------------------------------------------------

class ClientMfdStatusUpdateMessage;
class ClientObject;
class CreatureObject;
class StringId;
class TangibleObject;
class Transform;
class UIImage;
class UIVolumePage;

namespace MessageDispatch
{
	class Callback;
}

//----------------------------------------------------------------------

class SwgCuiBuffDisplay :
public CuiMediator,
public UIEventCallback
{
public:
	SwgCuiBuffDisplay(UIPage & page);

	virtual void setTarget(CreatureObject * creature);
	virtual void setTarget(const NetworkId & id);
	virtual const CachedNetworkId & getTarget() const;

	virtual void update(float deltaTimeSecs);


protected:
	virtual ~SwgCuiBuffDisplay();

protected:
	
	void performActivate();
	void performDeactivate();

	CachedNetworkId m_objectId;
	MessageDispatch::Callback * m_callback;
	UIVolumePage * m_volume;
	UIImageStyle * m_blank;

	float m_internalTimer;

	UIEffector *m_effectorBlink;

private:
	SwgCuiBuffDisplay();
	SwgCuiBuffDisplay(const SwgCuiBuffDisplay &);
	SwgCuiBuffDisplay & operator=(const SwgCuiBuffDisplay &);
};

//----------------------------------------------------------------------

inline CachedNetworkId const & SwgCuiBuffDisplay::getTarget() const
{
	return m_objectId;
}

//======================================================================

#endif
