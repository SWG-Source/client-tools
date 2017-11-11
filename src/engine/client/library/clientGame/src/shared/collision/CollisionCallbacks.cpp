// ======================================================================
//
// CollisionCallbacks.cpp
// tford
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/CollisionCallbacks.h"

#include "sharedCollision/SetupSharedCollision.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/ClientEffect.h"
#include "clientGame/ClientEffectTemplate.h"
#include "clientGame/ClientEffectTemplateList.h"
#include "clientGame/ClientObject.h"
#include "clientGame/Game.h"
#include "clientGame/ShipController.h"
#include "clientGame/ShipObject.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedGame/CollisionCallbackManager.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedNetworkMessages/MessageQueueUpdateShipOnCollision.h"
#include "sharedObject/CellProperty.h"

// ======================================================================

namespace CollisionCallbacksNamespace
{
	ClientEffectTemplate const * ms_shipShipClientEffectTemplate = 0;
	ClientEffectTemplate const * ms_shipObjectClientEffectTemplate = 0;

	bool ms_isPlayingClientEffect = false;
	bool ms_ignoreCollision;

	void remove();

	bool canTestCollisionDetectionOnObjectThisFrame(Object * const object);
	int convertObjectToIndex(Object * const object);
	void onNoHit(Object * const object);
	bool onHitDoVisualsOnly(Object * const object, Object * const wasHitByThisObject);
	bool onHitDoCollisionWith(Object * const object, Object * const wasHitByThisObject);
	bool onHitDoCollisionWithPOBShipElseDoVisualsOnly(Object * const object, Object * const wasHitByThisObject);
	bool onDoCollisionWithTerrain(Object * const object);
}

using namespace CollisionCallbacksNamespace;

// ======================================================================

void CollisionCallbacks::install()
{
	InstallTimer const installTimer("CollisionCallbacks::install");

	CollisionCallbackManager::install();

	CollisionCallbackManager::registerCanTestCollisionDetectionOnObjectThisFrame(CollisionCallbacksNamespace::canTestCollisionDetectionOnObjectThisFrame);
	CollisionCallbackManager::registerConvertObjectToIndexFunction(CollisionCallbacksNamespace::convertObjectToIndex);

	int const shipFighter = static_cast<int>(SharedObjectTemplate::GOT_ship_fighter);
	int const shipCapital = static_cast<int>(SharedObjectTemplate::GOT_ship_capital);
	int const shipStation = static_cast<int>(SharedObjectTemplate::GOT_ship_station);
	int const shipTransport = static_cast<int>(SharedObjectTemplate::GOT_ship_transport);
	int const asteroid = static_cast<int>(SharedObjectTemplate::GOT_misc_asteroid);
	int const miningAsteroidStatic = static_cast<int>(SharedObjectTemplate::GOT_ship_mining_asteroid_static);
	int const miningAsteroidDynamic = static_cast<int>(SharedObjectTemplate::GOT_ship_mining_asteroid_dynamic);

	CollisionCallbackManager::registerNoHitFunction(CollisionCallbacksNamespace::onNoHit, shipFighter);

	CollisionCallbackManager::registerOnHitFunction(CollisionCallbacksNamespace::onHitDoCollisionWithPOBShipElseDoVisualsOnly, shipFighter, shipFighter);
	CollisionCallbackManager::registerOnHitFunction(CollisionCallbacksNamespace::onHitDoCollisionWith, shipFighter, shipCapital);
	CollisionCallbackManager::registerOnHitFunction(CollisionCallbacksNamespace::onHitDoCollisionWith, shipFighter, shipStation);
	CollisionCallbackManager::registerOnHitFunction(CollisionCallbacksNamespace::onHitDoCollisionWith, shipFighter, shipTransport);
	CollisionCallbackManager::registerOnHitFunction(CollisionCallbacksNamespace::onHitDoCollisionWith, shipFighter, asteroid);
	CollisionCallbackManager::registerOnHitFunction(CollisionCallbacksNamespace::onHitDoCollisionWith, shipFighter, miningAsteroidStatic);
	CollisionCallbackManager::registerOnHitFunction(CollisionCallbacksNamespace::onHitDoCollisionWith, shipFighter, miningAsteroidDynamic);

//CollisionCallbackManager::registerDoCollisionWithTerrainFunction(CollisionCallbacksNamespace::onDoCollisionWithTerrain);

	{
		CrcLowerString const name("clienteffect/space_collision.cef");
		ms_shipShipClientEffectTemplate = ClientEffectTemplateList::fetch(name);
	}

	{
		CrcLowerString const name("clienteffect/space_collision.cef");
		ms_shipObjectClientEffectTemplate = ClientEffectTemplateList::fetch(name);
	}

	DebugFlags::registerFlag(ms_ignoreCollision, "ClientGame/CollisionCallbacks", "ignoreCollision");

	ExitChain::add(CollisionCallbacksNamespace::remove, "CollisionCallbacks");
}

// ----------------------------------------------------------------------

void CollisionCallbacks::setIgnoreCollision(bool const value)
{
	ms_ignoreCollision = value;
}

// ======================================================================

void CollisionCallbacksNamespace::remove()
{
	if (ms_shipShipClientEffectTemplate)
	{
		ms_shipShipClientEffectTemplate->release();
		ms_shipShipClientEffectTemplate = 0;
	}
	if (ms_shipObjectClientEffectTemplate)
	{
		ms_shipObjectClientEffectTemplate->release();
		ms_shipObjectClientEffectTemplate = 0;
	}

	DebugFlags::unregisterFlag(ms_ignoreCollision);
}

// ----------------------------------------------------------------------

bool CollisionCallbacksNamespace::canTestCollisionDetectionOnObjectThisFrame(Object * const object)
{
	return (object == Game::getPlayerPilotedShip());
}

// ----------------------------------------------------------------------

int CollisionCallbacksNamespace::convertObjectToIndex(Object * const object)
{
	FATAL(!object, ("CollisionCallbacksNamespace::convertObjectToIndex: object == NULL."));

	ClientObject const * clientObject = object->asClientObject();
	if (clientObject)
		return clientObject->getGameObjectType();

	return SharedObjectTemplate::GOT_none;
}

// ----------------------------------------------------------------------

void CollisionCallbacksNamespace::onNoHit(Object * const object)
{
	if (ms_ignoreCollision)
	{
		return;
	}

	ShipObject * shipObject = safe_cast<ShipObject *>(object);
	DEBUG_FATAL (!shipObject, ("CollisionCallbacksNamespace::onNoHit: shipObject == NULL"));

	if (Game::getConstPlayerPilotedShip() == shipObject)
	{
		ms_isPlayingClientEffect = false;
	}
}

// ----------------------------------------------------------------------

bool CollisionCallbacksNamespace::onHitDoVisualsOnly(Object * const object, Object * const wasHitByThisObject)
{
	if (ms_ignoreCollision)
	{
		return false;
	}

	ShipObject * shipObject = safe_cast<ShipObject *>(object);
	ShipObject * wasHitByThisShipObject = safe_cast<ShipObject *>(wasHitByThisObject);

	DEBUG_FATAL (!shipObject, ("CollisionCallbacksNamespace::onHitDoVisualsOnly: shipObject == NULL"));
	DEBUG_FATAL (!wasHitByThisShipObject, ("CollisionCallbacksNamespace::onHitDoVisualsOnly: wasHitByThisShipObject == NULL"));

	if ((Game::getConstPlayerPilotedShip() != shipObject) || (ms_isPlayingClientEffect))
	{
		return true;
	}

	Vector const shipPosition_w(shipObject->getPosition_w());
	Vector const wasHitByThisShipPosition_w(wasHitByThisShipObject->getPosition_w());
	Vector const effectPosition((shipPosition_w + wasHitByThisShipPosition_w) * 0.5f);

	if (ms_shipShipClientEffectTemplate)
	{
		ClientEffect * const clientEffect = ms_shipShipClientEffectTemplate->createClientEffect(CellProperty::getWorldCellProperty(), effectPosition, Vector::unitY);
		clientEffect->execute();
		delete clientEffect;
	}
	ms_isPlayingClientEffect = true;

	return true;
}

// ----------------------------------------------------------------------

bool CollisionCallbacksNamespace::onHitDoCollisionWith(Object * const object, Object * const wasHitByThisObject)
{
	if (ms_ignoreCollision)
	{
		return false;
	}

	DEBUG_FATAL (!object, ("CollisionCallbacksNamespace::onHitDoCollisionWith: Object == NULL"));
	DEBUG_FATAL (!wasHitByThisObject, ("CollisionCallbacksNamespace::onHitDoCollisionWith: wasHitByThisObject == NULL"));

	ShipObject * shipObject = safe_cast<ShipObject *>(object);
	DEBUG_FATAL (!shipObject, ("CollisionCallbacksNamespace::onHitDoCollisionWith: shipObject == NULL"));

	if (Game::getConstPlayerPilotedShip() != shipObject)
	{
		return false;
	}

	CollisionCallbackManager::Result result;
	if (CollisionCallbackManager::intersectAndReflect(object, wasHitByThisObject, result))
	{
		ShipController * const shipController = safe_cast<ShipController *>(shipObject->getController());
		NOT_NULL(shipController);

		shipController->respondToCollision(result.m_deltaToMoveBack_p, result.m_newReflection_p, result.m_normalOfSurface_p);

		if (!ms_isPlayingClientEffect && ms_shipObjectClientEffectTemplate)
		{
			ClientEffect * const clientEffect = ms_shipObjectClientEffectTemplate->createClientEffect(CellProperty::getWorldCellProperty(), result.m_pointOfCollision_p, Vector::unitY);
			clientEffect->execute();
			delete clientEffect;
		}
		ms_isPlayingClientEffect = true;
		return true;
	}
	return false;
}

// ----------------------------------------------------------------------

bool CollisionCallbacksNamespace::onHitDoCollisionWithPOBShipElseDoVisualsOnly(Object * const object, Object * const wasHitByThisObject)
{
	if (wasHitByThisObject->getPortalProperty() != 0)
	{
		return CollisionCallbacksNamespace::onHitDoCollisionWith(object, wasHitByThisObject);
	}

	return CollisionCallbacksNamespace::onHitDoVisualsOnly(object, wasHitByThisObject);
}

// ----------------------------------------------------------------------

bool CollisionCallbacksNamespace::onDoCollisionWithTerrain(Object * const object)
{
	if (ms_ignoreCollision)
	{
		return false;
	}

	DEBUG_FATAL (!object, ("CollisionCallbacksNamespace::onDoCollisionWithTerrain: Object == NULL"));

	ShipObject * shipObject = safe_cast<ShipObject *>(object);
	DEBUG_FATAL (!shipObject, ("CollisionCallbacksNamespace::onDoCollisionWithTerrain: shipObject == NULL"));

	if (Game::getConstPlayerPilotedShip() != shipObject)
	{
		return false;
	}

	CollisionCallbackManager::Result result;
	if (CollisionCallbackManager::intersectAndReflectWithTerrain(object, result))
	{
		ShipController * const shipController = safe_cast<ShipController *>(shipObject->getController());
		NOT_NULL(shipController);

		shipController->respondToCollision(result.m_deltaToMoveBack_p, result.m_newReflection_p, result.m_normalOfSurface_p);

		if (!ms_isPlayingClientEffect && ms_shipObjectClientEffectTemplate)
		{
			ClientEffect * const clientEffect = ms_shipObjectClientEffectTemplate->createClientEffect(CellProperty::getWorldCellProperty(), result.m_pointOfCollision_p, Vector::unitY);
			clientEffect->execute();
			delete clientEffect;
		}
		ms_isPlayingClientEffect = true;
		return true;
	}
	return false;
}

// ======================================================================

