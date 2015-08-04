// ======================================================================
//
// ShipController.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ShipController.h"

#include "clientGame/Game.h"
#include "clientGame/NebulaManagerClient.h"
#include "clientGame/ShipHitEffectsManagerClient.h"
#include "clientGame/ShipObject.h"
#include "clientGraphics/Camera.h"
#include "clientGraphics/DebugPrimitive.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedGame/ShipDynamicsModel.h"
#include "sharedNetworkMessages/EnvironmentalHitData.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedNetworkMessages/MessageQueueNetworkId.h"
#include "sharedNetworkMessages/MessageQueueShipHit.h"
#include "sharedNetworkMessages/MessageQueueUpdateShipOnCollision.h"
#include "sharedNetworkMessages/NebulaLightningHitData.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/NetworkIdManager.h"


// ======================================================================
// ShipControllerNamespace
// ======================================================================

namespace ShipControllerNamespace
{
	bool        ms_drawTurretTargetLine                        = false;

}
using namespace ShipControllerNamespace;

// ======================================================================
// STATIC PUBLIC ShipController
// ======================================================================

void ShipController::install ()
{
	InstallTimer const installTimer("ShipController::install");

	DebugFlags::registerFlag (ms_drawTurretTargetLine, "ClientGame/ShipController", "drawTurretTargetLines");

	ExitChain::add(&remove, "ShipController::remove");
}

// ======================================================================
// STATIC PRIVATE ShipController
// ======================================================================

void ShipController::remove ()
{
	DebugFlags::unregisterFlag (ms_drawTurretTargetLine);
}

// ======================================================================
// PUBLIC ShipController
// ======================================================================

ShipController::ShipController(ShipObject * owner) :
	ClientController(owner),
	m_shipDynamicsModel(new ShipDynamicsModel)
{
}

// ----------------------------------------------------------------------

ShipController::~ShipController()
{
	delete m_shipDynamicsModel;
}

// ----------------------------------------------------------------------

//----------------------------------------------------------------------

ShipController * ShipController::asShipController()
{
	return this;
}

//----------------------------------------------------------------------

ShipController const * ShipController::asShipController() const
{
	return this;
}

//----------------------------------------------------------------------

void ShipController::endBaselines()
{
	Object const * const owner = getOwner();
	if (owner)
	{
		m_shipDynamicsModel->setTransform(owner->getTransform_o2p());
		m_shipDynamicsModel->makeStationary();
	}

	ClientController::endBaselines();
}

// ----------------------------------------------------------------------

float ShipController::getCurrentSpeed() const
{
	return m_shipDynamicsModel ? m_shipDynamicsModel->getVelocity().magnitude() : 0.f;
}

// ----------------------------------------------------------------------

void ShipController::receiveTransform(ShipUpdateTransformMessage const & /*shipUpdateTransformMessage*/)
{
}

//----------------------------------------------------------------------

void ShipController::setShipDynamicsModelTransform(Transform const & transform)
{
	if (m_shipDynamicsModel != 0)
	{
		m_shipDynamicsModel->setTransform(transform);
	}
}

//----------------------------------------------------------------------

void ShipController::handleMessage(int message, float value, const MessageQueue::Data* data, uint32 flags)
{
	switch(message)
	{
	case CM_lightningHitShip:
		{
			typedef MessageQueueGenericValueType<NebulaLightningHitData> MessageType;
			MessageType const * const messageData = dynamic_cast<MessageType const *>(data);
			if (messageData != NULL)
				NebulaManagerClient::handleServerHit(*safe_cast<ClientObject *>(getOwner()), messageData->getValue());
		}
		break;

	case CM_environmentHitShip:
		{
			typedef MessageQueueGenericValueType<EnvironmentalHitData> MessageType;
			MessageType const * const messageData = dynamic_cast<MessageType const *>(data);
			if (messageData != NULL)
				NebulaManagerClient::handleServerEnvironmentalDamage(*safe_cast<ClientObject *>(getOwner()), messageData->getValue());
		}
		break;

	case CM_spaceShipHit:
		{
			MessageQueueShipHit const * const messageData = dynamic_cast<MessageQueueShipHit const *>(data);
			if (messageData != NULL)
				ShipHitEffectsManagerClient::handleShipHit(*NON_NULL(getShipOwner()), *messageData);
		}
		break;

	case CM_shipStopFiring:
		{
			ShipObject * const ship = NON_NULL(getShipOwner());

			typedef MessageQueueGenericValueType<int> MessageType;			
			MessageType const * const msg = dynamic_cast<MessageType const *>(data);

			if (msg)
			{
				int const weaponIndex = msg->getValue();
				ship->stopFiringWeapon(weaponIndex, false);
			}
		}
		break;


	default:
		ClientController::handleMessage(message, value, data, flags);
		break;
	}
}

//----------------------------------------------------------------------

void ShipController::setLookAtTarget (const NetworkId & id)
{
	MessageQueueNetworkId * const msg = new MessageQueueNetworkId (id);
	appendMessage (static_cast<int>(CM_clientLookAtTarget), 0.0f, msg,
		GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_SERVER);
} //lint !e429 // msg has not been freed or returned // it's okay, message queue owns.

//----------------------------------------------------------------------

void ShipController::setLookAtTargetSlot (ShipChassisSlotType::Type slot)
{
	MessageQueueGenericValueType<int> * const msg = new MessageQueueGenericValueType<int>(slot);
	appendMessage (static_cast<int>(CM_clientLookAtTargetComponent), 0.0f, msg,
		GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_SERVER);
} //lint !e429 // msg has not been freed or returned // it's okay, message queue owns.

//----------------------------------------------------------------------

void ShipController::respondToCollision(Vector const & deltaToMove_p, Vector const & newReflection_p, Vector const & /*normalOfSurface_p*/)
{
	// this is identical to the server's ShipController::respondToCollision.
	// the difference it that the server also must call the virtual method
	// ShipController::experiencedCollision at the end of execution
	NOT_NULL(m_shipDynamicsModel);
	Transform transform_p(m_shipDynamicsModel->getTransform());
	transform_p.move_p(deltaToMove_p);

	m_shipDynamicsModel->setTransform(transform_p);
	m_shipDynamicsModel->setVelocity(newReflection_p * m_shipDynamicsModel->getVelocity().magnitude());

	ShipObject * const owner = getShipOwner();
	NOT_NULL(owner);
	owner->setTransform_o2p(transform_p);
}

// ----------------------------------------------------------------------

Vector const & ShipController::getVelocity_p() const
{
	return m_shipDynamicsModel ? m_shipDynamicsModel->getVelocity() : Vector::zero;
}

//----------------------------------------------------------------------

Transform const & ShipController::getServerPredictedTransform_o2w() const
{
	return m_shipDynamicsModel ? m_shipDynamicsModel->getTransform() : Transform::identity;
}

// ----------------------------------------------------------------------

PlayerShipController * ShipController::asPlayerShipController()
{
	return 0;
}

// ----------------------------------------------------------------------

PlayerShipController const * ShipController::asPlayerShipController() const
{
	return 0;
}

// ======================================================================
// PROTECTED ShipController
// ======================================================================

ShipObject * ShipController::getShipOwner()
{
	Object * const object = getOwner();
	ClientObject * const clientObject = object ? object->asClientObject() : 0;
	return clientObject ? clientObject->asShipObject() : 0;
}

// ----------------------------------------------------------------------

ShipObject const * ShipController::getShipOwner() const
{
	return const_cast<ShipController *>(this)->getShipOwner();
}

// ======================================================================
