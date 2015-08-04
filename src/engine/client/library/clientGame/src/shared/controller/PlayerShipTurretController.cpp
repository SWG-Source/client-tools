// ======================================================================
//
// PlayerShipTurretController.cpp
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/PlayerShipTurretController.h"

#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/ShipObject.h"
#include "clientGame/ShipStation.h"
#include "clientUserInterface/CuiIoWin.h"
#include "clientUserInterface/CuiPreferences.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/Object.h"

// ======================================================================

namespace PlayerShipTurretControllerNamespace
{
	bool ms_invertPitch;

	void remove();
}

using namespace PlayerShipTurretControllerNamespace;

// ======================================================================

void PlayerShipTurretController::install()
{
	InstallTimer const installTimer("PlayerShipTurretController::install");

	DebugFlags::registerFlag(ms_invertPitch, "ClientGame/PlayerShipTurretController", "invertPitch");

	ExitChain::add(PlayerShipTurretControllerNamespace::remove, "PlayerShipTurretController::remove");
}

// ----------------------------------------------------------------------

void PlayerShipTurretControllerNamespace::remove()
{
	DebugFlags::unregisterFlag(ms_invertPitch);
}

// ----------------------------------------------------------------------

PlayerShipTurretController::PlayerShipTurretController(Object *owner) :
	ClientController(owner),
	m_yaw(0.f),
	m_pitch(0.f)
{
}

// ----------------------------------------------------------------------

PlayerShipTurretController::~PlayerShipTurretController()
{
}

// ----------------------------------------------------------------------

float PlayerShipTurretController::realAlter(float elapsedTime)
{
	float yawMod = 0.f;
	float pitchMod = 0.f;
	bool firing = false;

	CreatureObject * const playerCreature = NON_NULL(Game::getPlayerCreature());

	int const weaponIndex = ShipStation::getWeaponIndexForGunnerStation(playerCreature->getShipStation());

	ShipObject * const ship = NON_NULL(ShipObject::getContainingShip(*playerCreature));

	MessageQueue * queue = Game::getGameMessageQueue();
	for (int i = 0; i < queue->getNumberOfMessages(); ++i)
	{
		int message;
		float value;
		MessageQueue::Data *data;
		queue->getMessage(i, &message, &value, &data);

		switch (message)
		{
		case CM_shipFireWeaponAll:
		case CM_shipFireWeapon0:
		case CM_shipFireWeapon1:
		case CM_shipFireWeapon2:
		case CM_shipFireWeapon3:
		case CM_shipFireWeapon4:
		case CM_shipFireWeapon5:
		case CM_shipFireWeapon6:
		case CM_shipFireWeapon7:
			if (value > 0.0f)
			{
				firing = true;
			}
			else if (value < 0.0f)
			{
				ship->stopFiringWeapon(weaponIndex, true);
			}

			break;

		case CM_cameraYawMouse:
			yawMod += value;
			break;

		case CM_cameraPitchMouse:
			pitchMod += value;
			break;

		case CM_yawButton:
		case CM_yaw:
			yawMod += value * elapsedTime;
			break;

		case CM_pitchButton:
		case CM_pitch:
			if (CuiPreferences::isJoystickInverted())
				pitchMod += value * elapsedTime;
			else
				pitchMod -= value * elapsedTime;
			break;

		default:
			break;
		}
	}

	if (ms_invertPitch)
		pitchMod = -pitchMod;

	m_yaw += yawMod;
	m_pitch += pitchMod;

	// orient the turret.  m_yaw and m_pitch may be modified to match turret constraints.
	ship->orientShipTurret(weaponIndex, m_yaw, m_pitch, ms_invertPitch);

	//-- Fire guns after we've changed to our final transform for this frame
	if (firing && weaponIndex != -1)
		IGNORE_RETURN(ship->fireWeapon(weaponIndex));

	return ClientController::realAlter(elapsedTime);
}

// ======================================================================
