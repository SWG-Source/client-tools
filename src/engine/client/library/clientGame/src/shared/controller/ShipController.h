// ======================================================================
//
// ShipController.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ShipController_H
#define INCLUDED_ShipController_H

// ======================================================================

#include "clientGame/ClientController.h"
#include "sharedGame/ShipChassisSlotType.h"

class PlayerShipController;
class ShipDynamicsModel;
class ShipObject;
class ShipUpdateTransformMessage;
class Transform;

// ======================================================================

class ShipController : public ClientController
{
public:

	static void install ();

public:

	explicit ShipController(ShipObject * newOwner);
	virtual ~ShipController();

	virtual ShipController * asShipController();
	virtual ShipController const * asShipController() const;

	virtual void endBaselines();
	virtual float getCurrentSpeed() const;
	virtual void receiveTransform(ShipUpdateTransformMessage const & shipUpdateTransformMessage) = 0;
	void setShipDynamicsModelTransform(Transform const & transform);
	virtual void handleMessage(int message, float value, const MessageQueue::Data* data, uint32 flags);

	virtual PlayerShipController * asPlayerShipController();
	virtual PlayerShipController const * asPlayerShipController() const;

	void setLookAtTarget (NetworkId const & id);
	void setLookAtTargetSlot (ShipChassisSlotType::Type slot);

	void respondToCollision(Vector const & deltaToMove_p, Vector const & newReflection_p, Vector const & normalOfSurface_p);

	Vector const & getVelocity_p() const;

	Transform const & getServerPredictedTransform_o2w() const;

protected:

	ShipObject * getShipOwner();
	ShipObject const * getShipOwner() const;

protected:

	ShipDynamicsModel * const m_shipDynamicsModel;

private:
	
	static void remove ();

private:

	ShipController();
	ShipController(ShipController const &);
	ShipController & operator=(ShipController const &);
};

// ======================================================================

#endif
