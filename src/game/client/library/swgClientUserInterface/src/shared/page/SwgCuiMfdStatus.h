//======================================================================
//
// SwgCuiMfdStatus.h
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiMfdStatus_H
#define INCLUDED_SwgCuiMfdStatus_H

//======================================================================

#include "sharedMessageDispatch/Receiver.h"
#include "swgClientUserInterface/SwgCuiLockableMediator.h"
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

namespace MessageDispatch
{
	class Callback;
}

//----------------------------------------------------------------------

class SwgCuiMfdStatus :
public SwgCuiLockableMediator
{
public:
	SwgCuiMfdStatus(char const * const mediatorName, UIPage & page);

	virtual void setTarget(TangibleObject * tangible) = 0;
	virtual void setTarget(const NetworkId & id) = 0;
	virtual const CachedNetworkId & getTarget() const;

	virtual void setTargetNamePrefix(const Unicode::String & prefix) = 0;
	virtual void setTargetName(const Unicode::String & name) = 0;
	virtual void setObjectName(const Unicode::String & name);
	virtual void updateTargetName (const ClientObject & obj) = 0;

	virtual void setShowRange(bool b) = 0;
	virtual void update(float deltaTimeSecs) = 0;
	virtual void update(ClientMfdStatusUpdateMessage const &);

	virtual void setDisplayStates(bool stats, bool states, bool posture) = 0;

protected:
	virtual ~SwgCuiMfdStatus();

protected:
	CachedNetworkId m_objectId;
	MessageDispatch::Callback * m_callback;
	UIImage * m_directionArrow;

private:
	SwgCuiMfdStatus();
	SwgCuiMfdStatus(const SwgCuiMfdStatus &);
	SwgCuiMfdStatus & operator=(const SwgCuiMfdStatus &);
};

//----------------------------------------------------------------------

inline CachedNetworkId const & SwgCuiMfdStatus::getTarget() const
{
	return m_objectId;
}

//======================================================================

#endif
