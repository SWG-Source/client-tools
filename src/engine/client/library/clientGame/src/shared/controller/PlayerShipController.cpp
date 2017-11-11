// ======================================================================
//
// PlayerShipController.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/PlayerShipController.h"

#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientShipObjectInterface.h"
#include "clientGame/ClientShipTargeting.h"
#include "clientGame/ClientStringIds.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GameMusicManager.h"
#include "clientGame/GameNetwork.h"
#include "clientGame/GroundScene.h"
#include "clientGame/GroupObject.h"
#include "clientGame/ShipObject.h"
#include "clientGraphics/Camera.h"
#include "clientGraphics/DebugPrimitive.h"
#include "clientGraphics/Graphics.h"
#include "clientUserInterface/CuiDamageManager.h"
#include "clientUserInterface/CuiIoWin.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "sharedCollision/BoxExtent.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/CollisionWorld.h"
#include "sharedCollision/SpaceAvoidanceManager.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedGame/CollisionCallbackManager.h"
#include "sharedGame/SharedShipObjectTemplate.h"
#include "sharedGame/ShipChassisSlotType.h"
#include "sharedGame/ShipDynamicsModel.h"
#include "sharedGame/SpaceStringIds.h"
#include "sharedMathArchive/TransformArchive.h"
#include "sharedMathArchive/VectorArchive.h"
#include "sharedNetworkMessages/MessageQueueDataTransform.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedNetworkMessages/MessageQueueTeleportAck.h"
#include "sharedNetworkMessages/MessageQueueUpdateShipOnCollision.h"
#include "sharedNetworkMessages/ShipDamageMessage.h"
#include "sharedNetworkMessages/ShipUpdateTransformCollisionMessage.h"
#include "sharedNetworkMessages/ShipUpdateTransformMessage.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedTerrain/TerrainObject.h"
#include "swgSharedUtility/States.h"

// ======================================================================
// PlayerShipControllerNamespace
// ======================================================================

//lint -e641

namespace PlayerShipControllerNamespace
{
	float const c_defaultReliableTransformUpdateTime = 4.f;
	float const c_defaultTransformUpdateTime = 0.2f;


	bool ms_debugReport;
	bool ms_invertPitch;
	bool ms_invertRoll = true;
	bool ms_renderShipDynamicsModel = false;
	bool ms_renderServerShipDynamicsModel = false;
	bool ms_renderShipAppearanceExtent = false;
	bool ms_renderShipTangibleExtent = false;
	bool ms_virtualJoystickEnabled = true;

	float ms_deadZone = 4.f;

	void remove();

	float const c_throttleAcceleration = 0.015f;
	float const c_throttleAccelerationBase = 0.002f;
	float const c_throttleAccelerationMax = 0.7f;
	float const c_throttleAccelerationDelay = 0.2f;

	float const c_autopilotTolerance = 0.05f; // miss angle in radians that counts as being "on target"
	float const c_autopilotBadlyOffTargetTolerance = PI_OVER_4; // miss angle that counts as being "badly off target".  (Used to decide how to set the throttle.)
	float const c_autopilotReachedTargetTolerance = 5.0f; // how close to the target location counts as being there
	float const c_autopilotTurnSlowDistance = 50.0f; // when within this distance, turn slower to avoid circling the target
	float const c_autopilotDeadZone = 0.25f; // how large a control deflection is needed to cancel autopilot

	float const c_shipFollowDistance = 50.f;
	float const c_shipFollowTolerance = 10.f;

	float computeRequiredAcceleration(float const currentPosition, float const targetPosition, float const currentSpeed, float const acceleration, float const maxSpeed);

	float convertValueWithDeadZone(float value, float joystickSensitivityPower, float joystickDeadZone)
	{
		joystickDeadZone = clamp(0.0f, joystickDeadZone, 0.9f);
		float const oo_joystickDeadZoneRange = RECIP(1.0f - joystickDeadZone);

		if (value > 0.0f)
		{
			value = std::max(0.0f, (value - joystickDeadZone)) * oo_joystickDeadZoneRange;
			if (1.0f != joystickSensitivityPower)
				value = pow(value, joystickSensitivityPower);
		}
		else
		{
			value = std::min(0.0f, (value + joystickDeadZone)) * oo_joystickDeadZoneRange;
			if (1.0f != joystickSensitivityPower)
				value = -pow(-value, joystickSensitivityPower);
		}

		return value;
	}
}

using namespace PlayerShipControllerNamespace;

// ======================================================================
// STATIC PUBLIC PlayerShipController
// ======================================================================

void PlayerShipController::install()
{
	InstallTimer const installTimer("PlayerShipController::install");

	DebugFlags::registerFlag(ms_debugReport, "ClientGame/PlayerShipController", "debugReport");
	DebugFlags::registerFlag(ms_invertPitch, "ClientGame/PlayerShipController", "invertPitch");
	DebugFlags::registerFlag(ms_invertRoll, "ClientGame/PlayerShipController", "invertRoll");
	DebugFlags::registerFlag(ms_renderShipDynamicsModel, "ClientGame/PlayerShipController", "renderShipDynamicsModel");
	DebugFlags::registerFlag(ms_renderServerShipDynamicsModel, "ClientGame/PlayerShipController", "renderServerShipDynamicsModel");
	DebugFlags::registerFlag(ms_renderShipAppearanceExtent, "ClientGame/PlayerShipController", "renderShipAppearanceExtent");
	DebugFlags::registerFlag(ms_renderShipTangibleExtent, "ClientGame/PlayerShipController", "renderShipTangibleExtent");
	DebugFlags::registerFlag(ms_virtualJoystickEnabled, "ClientGame/PlayerShipController", "virtualJoystickEnabled");

	ms_deadZone = ConfigFile::getKeyFloat("ClientGame/PlayerShipController", "deadZone", ms_deadZone);

	ExitChain::add(PlayerShipControllerNamespace::remove, "PlayerCreatureController::remove");
}

// ======================================================================
// STATIC PRIVATE PlayerShipController
// ======================================================================

void PlayerShipControllerNamespace::remove()
{
	DebugFlags::unregisterFlag(ms_debugReport);
	DebugFlags::unregisterFlag(ms_invertPitch);
	DebugFlags::unregisterFlag(ms_invertRoll);
	DebugFlags::unregisterFlag(ms_renderShipDynamicsModel);
	DebugFlags::unregisterFlag(ms_renderServerShipDynamicsModel);
	DebugFlags::unregisterFlag(ms_renderShipAppearanceExtent);
	DebugFlags::unregisterFlag(ms_renderShipTangibleExtent);
	DebugFlags::unregisterFlag(ms_virtualJoystickEnabled);
}

// ======================================================================
// PUBLIC PlayerShipController
// ======================================================================

PlayerShipController::PlayerShipController(ShipObject * const newOwner) :
	ShipController(newOwner),
	m_serverShipDynamicsModel(new ShipDynamicsModel),
	m_sendTransformTimer(c_defaultTransformUpdateTime),
	m_sendTransformThisFrame(false),
	m_sendReliableTransformTimer(c_defaultReliableTransformUpdateTime),
	m_sendReliableTransformThisFrame(false),
	m_previousTransform_p(),
	m_throttlePosition(0.0f),
	m_throttleAcceleration(0.0f),
	m_throttleRepeatDelay(0.0f),
	m_virtualJoystickActive(ms_virtualJoystickEnabled),
	m_shipToFollow(),	
	m_serverToClientLastSyncStamp(0),
	m_lockInputState(false),
	m_allowTransformsWhileLocked(false),
	m_sentFinalTransform(false),
	m_autopilotEngaged(false),
	m_autopilotMode(AM_rollLevel),
	m_autopilotTargetHeading(Vector::zero),
	m_autopilotTargetLocation(Vector::zero),
	m_autopilotFollowLocation(Vector::zero),
	m_autoLevelTimer(10.0f),
	m_inFormation(false),
	m_lastJoystickThrottlePosition(-1.0f),
	m_shipThatLastShotPlayer(),
	m_swapRollYawAxes(false),
	m_nearZoneEdge(false),
	m_throttleDeltaSliderPosition(0.0f),
	m_throttleDeltaAxisPosition(0.0f),
	m_sentAutoPilotEngagedMessage(false),
	m_boosterWasActive(false)
{
}

// ----------------------------------------------------------------------

PlayerShipController::~PlayerShipController()
{
	delete m_serverShipDynamicsModel;
}

// ----------------------------------------------------------------------

void PlayerShipController::conclude()
{
	//-- Send movement data to server
	ShipObject const * const owner = getShipOwner();
	if (owner && owner->isInitialized() && owner->getNetworkId() != NetworkId::cms_invalid)
	{
		if (m_sendReliableTransformThisFrame || m_sendTransformThisFrame)
		{
			Transform const transform_p = owner->getTransform_o2p();
			if (m_sendReliableTransformThisFrame || transform_p != m_previousTransform_p)
			{
				m_previousTransform_p = transform_p;
				sendTransform(m_sendReliableTransformThisFrame);
			}
		}
	}

	//-- Chain up to parent
	ShipController::conclude();
}

// ----------------------------------------------------------------------

void PlayerShipController::receiveTransform(ShipUpdateTransformCollisionMessage const & shipUpdateTransformMessage)
{
	ShipObject * const owner = getShipOwner();
	NOT_NULL(owner);

	Vector const clientLocation = m_shipDynamicsModel->getTransform().getPosition_p();
	Vector const serverLocation = shipUpdateTransformMessage.getTransform().getPosition_p();
	float const appearanceRadiusSqr = sqr(owner->getAppearanceSphereRadius());

	m_serverShipDynamicsModel->setTransform(shipUpdateTransformMessage.getTransform());
	m_serverShipDynamicsModel->setVelocity(shipUpdateTransformMessage.getVelocity());

	// if the client is farther away than its appearance radius then we snap it back
	// to the server's location.  Otherwise we can assume that the client and the
	// server are roughly the same and the the client has already handled the collision

	if (appearanceRadiusSqr < (clientLocation - serverLocation).magnitudeSquared())
	{
		m_shipDynamicsModel->setTransform(shipUpdateTransformMessage.getTransform());
		m_shipDynamicsModel->setVelocity(shipUpdateTransformMessage.getVelocity());

		owner->setTransform_o2p(m_shipDynamicsModel->getTransform());

		// here the server is authoritative so we need to store position
		CollisionProperty * const collision = owner->getCollisionProperty();
		NOT_NULL(collision);
		collision->storePosition();
	}
}

// ----------------------------------------------------------------------

void PlayerShipController::receiveTransform(ShipUpdateTransformMessage const & shipUpdateTransformMessage)
{
	if (getShipOwner()->hasCondition(TangibleObject::C_docking))
	{
		uint32 const syncStampLong = shipUpdateTransformMessage.getSyncStampLong();
		if (   m_serverToClientLastSyncStamp == 0
		    || GameNetwork::syncStampLongDeltaTime(m_serverToClientLastSyncStamp, syncStampLong) > 0)
		{
			m_serverToClientLastSyncStamp = syncStampLong;
			m_serverShipDynamicsModel->setTransform(shipUpdateTransformMessage.getTransform());
			m_serverShipDynamicsModel->setVelocity(shipUpdateTransformMessage.getVelocity());
		}
	}
}

// ----------------------------------------------------------------------

PlayerShipController * PlayerShipController::asPlayerShipController()
{
	return this;
}

// ----------------------------------------------------------------------

PlayerShipController const * PlayerShipController::asPlayerShipController() const
{
	return this;
}

// ======================================================================
// PROTECTED PlayerShipController
// ======================================================================

float PlayerShipController::realAlter(float const elapsedTime)
{
	NP_PROFILER_AUTO_BLOCK_DEFINE("PlayerShipController::realAlter");

	//-- Only continue if we're a ShipObject
	ShipObject * const owner = getShipOwner();
	if (!owner)
	{
		DEBUG_WARNING(true,("PlayerShipController::realAlter(): skipping rest of alter because owner is NULL, chaining up to parent class."));
		return ShipController::realAlter(elapsedTime);
	}

	//-- Fix lookAt target slot if it isn't valid
	{
		ShipChassisSlotType::Type targetSlot = owner->getPilotLookAtTargetSlot();
		if (targetSlot != ShipChassisSlotType::SCST_invalid)
		{
			CachedNetworkId const & target = owner->getPilotLookAtTarget();

			if (target.getObject() &&
					target.getObject()->asClientObject() &&
					target.getObject()->asClientObject()->asShipObject() &&
					target.getObject()->asClientObject()->asShipObject()->isComponentDemolished(targetSlot))
				owner->setPilotLookAtTargetSlot(target.getObject()->asClientObject()->asShipObject()->getNextValidTargetableSlot(targetSlot));
		}
	}

	//-- Process controller messages
	bool locked = false;
	bool cameraLocked = false;
	float yawPosition = 0.f;
	float joystickYawPosition = 0.0f;
	float pitchPosition = 0.f;
	float joystickPitchPosition = 0.0f;
	float rollPosition = 0.f;
	float joystickRollPosition = 0.f;

	bool firingAllWeapons = false;
	bool useThrottleAccelerationDecay = true;

	bool firingWeapons[ShipChassisSlotType::cms_numWeaponIndices];
	{
		for (int i = 0; i < ShipChassisSlotType::cms_numWeaponIndices; ++i)
			firingWeapons[i] = false;
	}

	m_throttleRepeatDelay -= elapsedTime;

	if (!ms_virtualJoystickEnabled)
		m_virtualJoystickActive = false;

	bool const virtualJoystickWasActive = m_virtualJoystickActive;
	static float const deactivateVirtualJoystickThreshold = 0.5f;

	float const joystickDeadZone = clamp (0.0f, CuiPreferences::getJoystickDeadZone(), 0.5f);
	float const joystickSensitivityRaw = clamp(0.1f, CuiPreferences::getJoystickSensitivity(), 10.0f);
	float const joystickSensitivityPower = RECIP(joystickSensitivityRaw);
	bool const invertJoystick = CuiPreferences::isJoystickInverted();

	bool throttleChanged = false;
	bool swapRollYawAxes = false;

	MessageQueue * const queue = Game::getGameMessageQueue();
	for (int i = 0; i < queue->getNumberOfMessages(); ++i)
	{
		int message;
		float value;
		MessageQueue::Data * data;
		queue->getMessage(i, &message, &value, &data);

		switch (message)
		{
		case CM_swapRollYawAxes:
			swapRollYawAxes = true;
			break;

		case CM_shipThrottleDeltaAxis:
			m_throttleDeltaAxisPosition = convertValueWithDeadZone(value, joystickSensitivityPower, joystickDeadZone);
			break;

		case CM_shipThrottleDeltaSlider:
			m_throttleDeltaSliderPosition = (value * 2.0f) - 1.0f;
			m_throttleDeltaSliderPosition = convertValueWithDeadZone(m_throttleDeltaSliderPosition, 1.0f, joystickDeadZone);
			break;

		case CM_shipThrottleChange:
			throttleChanged = true;

			if (m_throttleRepeatDelay <= 0.0f)
			{
				if (value < 0.0f)
					m_throttleAcceleration = clamp(-c_throttleAccelerationMax, m_throttleAcceleration - (c_throttleAcceleration * elapsedTime), -c_throttleAccelerationBase);
				else
					m_throttleAcceleration = clamp(c_throttleAccelerationBase, m_throttleAcceleration + (c_throttleAcceleration * elapsedTime), c_throttleAccelerationMax);
				setThrottlePosition(m_throttlePosition + m_throttleAcceleration, true);
				useThrottleAccelerationDecay = false;
			}
			break;
		case CM_setThrottlePosition:
			setThrottlePosition(value, true);
			break;

		case CM_shipThrottleSetSlider:
			setThrottlePositionFromJoystick(value, false);
			break;

		case CM_shipThrottleSetAxis:
			{
				//The throttle slider's range is [0,1], but the joystick axis's matching range is [1,-1]
				//(at least for the Saitek Cyborg Evo)
				float const ratio = (1.0f - value) / 2.0f;
				setThrottlePositionFromJoystick(ratio, false);
			}
			break;

		case CM_shipThrottleChangeStart:
			m_throttleRepeatDelay = c_throttleAccelerationDelay;
			m_throttleAcceleration = 0.0f;
			setThrottlePosition(m_throttlePosition + (value * 0.05f), true);
			useThrottleAccelerationDecay = false;
			break;

		case CM_shipFireWeaponAll:
			firingAllWeapons = true;
			break;

		case CM_shipFireWeapon0:
		case CM_shipFireWeapon1:
		case CM_shipFireWeapon2:
		case CM_shipFireWeapon3:
		case CM_shipFireWeapon4:
		case CM_shipFireWeapon5:
		case CM_shipFireWeapon6:
		case CM_shipFireWeapon7:
			//-- positive value means start firing
			if (value > 0.0f)
			{
				firingWeapons[message - CM_shipFireWeapon0] = true;
			}
			//-- negative value means stop firing
			else if (value < 0.0f)
			{
				owner->stopFiringWeapon(message - CM_shipFireWeapon0, true);
			}

			break;

		case CM_cameraYawMouse:
			yawPosition += value;
			break;

		case CM_cameraPitchMouse:
			pitchPosition += value;
			break;

		case CM_cameraLock:
			cameraLocked = true;
			break;

		case CM_yawButton:
			yawPosition += value;
			break;

		case CM_pitchButton:
			pitchPosition += value;
			break;

		case CM_rollButton:
			rollPosition += value;
			break;

		case CM_yaw:
			value = convertValueWithDeadZone(value, joystickSensitivityPower, joystickDeadZone);

			if (m_swapRollYawAxes)
				joystickRollPosition = value;
			else
				joystickYawPosition = value;

			if (!WithinEpsilonInclusive(0.0f, value, deactivateVirtualJoystickThreshold))
				m_virtualJoystickActive = false;
			break;

		case CM_pitch:
			value = convertValueWithDeadZone(value, joystickSensitivityPower, joystickDeadZone);

			if (!invertJoystick)
				value = -value;

			joystickPitchPosition = value;

			if (!WithinEpsilonInclusive(0.0f, value, deactivateVirtualJoystickThreshold))
				m_virtualJoystickActive = false;
			break;

		case CM_roll:
			value = convertValueWithDeadZone(value, joystickSensitivityPower, joystickDeadZone);

			if (m_swapRollYawAxes)
				joystickYawPosition = value;
			else
				joystickRollPosition = value;

			if (!WithinEpsilonInclusive(0.0f, value, deactivateVirtualJoystickThreshold))
				m_virtualJoystickActive = false;
			break;

		default:
			break;
		}
	}

	if (!WithinEpsilonExclusive(0.0f, m_throttleDeltaSliderPosition, 0.05f))
	{
		setThrottlePosition(m_throttlePosition + (m_throttleDeltaSliderPosition * elapsedTime), true);
	}
	else if (!WithinEpsilonExclusive(0.0f, m_throttleDeltaAxisPosition, 0.05f))
	{
		setThrottlePosition(m_throttlePosition + (m_throttleDeltaAxisPosition * elapsedTime), true);
	}

	m_swapRollYawAxes = swapRollYawAxes;

#ifdef ENABLE_FORMATIONS
	if(m_inFormation)
	{
		CreatureObject const * const player = owner->getPilot();
		if(player)
		{
			CachedNetworkId const id (player->getGroup ());
			GroupObject const * const group = safe_cast<const GroupObject *>(id.getObject());
			if(group)
			{
				if(group->getFormationNameCrc() != Crc::crcNull)
				{
					Vector pos;
					IGNORE_RETURN(PlayerFormationManager::getPositionOffset(group->getFormationNameCrc(), group->getShipFormationSlotFromMember(player->getNetworkId()), pos));
					NetworkId const & leaderCreatureId = group->getLeader();
					NetworkId const & leaderShipId = group->getShipFromMember(leaderCreatureId);
					Object const * const objectLeaderShip = NetworkIdManager::getObjectById(leaderShipId);
					ClientObject const * const clientLeaderShip = (objectLeaderShip != 0) ? objectLeaderShip->asClientObject() : 0;
					ShipObject const * const leaderShip = (clientLeaderShip != 0) ? clientLeaderShip->asShipObject() : 0;

					if(leaderShip)
					{
						// If you are in a formation and this is the leader ship, then update speed.
						bool canUseAvoidance = false;
						if (leaderShipId == m_shipToFollow)
						{
							canUseAvoidance = true;
							m_autoPilotMatchSpeed = leaderShip->getCurrentSpeed();
						}

						m_autopilotFollowLocation = leaderShip->getInterceptPosition(elapsedTime) + leaderShip->rotate_o2p(pos);
						Vector avoidancePosition_w;

						if (canUseAvoidance && SpaceAvoidanceManager::getAvoidancePosition(*owner, m_autopilotFollowLocation - owner->getPosition_w(), m_autopilotFollowLocation, avoidancePosition_w))
						{
							internalEngageAutopilotToLocation(avoidancePosition_w);
						}
						else
						{
							internalEngageAutopilotToLocation(m_autopilotFollowLocation);
						}
					}
				}
				else
					setInFormation(false);
			}
			else
				setInFormation(false);
		}
	}
#endif

	if (useThrottleAccelerationDecay)
	{
		if (m_throttleAcceleration > 0.0f)
		{
			m_throttleAcceleration -= elapsedTime;
			m_throttleAcceleration = clamp(0.0f, m_throttleAcceleration, c_throttleAccelerationMax);
		}
		else
		{
			m_throttleAcceleration += elapsedTime;
			m_throttleAcceleration = clamp(-c_throttleAccelerationMax, m_throttleAcceleration, 0.0f);
		}
	}

	if (cameraLocked)
		locked = true;

	if (CuiPreferences::getPilotMouseModeAutomatic() == CuiPreferences::PMM_cockpitCamera)
		locked = true;

	if (locked)
		m_virtualJoystickActive = false;

	if (cameraLocked)
	{
		joystickYawPosition = 0.0f;
		joystickPitchPosition = 0.0f;
		joystickRollPosition = 0.0f;
	}

	int const deadZoneSize = CuiIoWin::getDeadZoneSizeUsable();
	if (!locked && !CuiManager::getPointerInputActive() && deadZoneSize > 0)
	{
		UIPoint cursorLocation;
		CuiManager::getIoWin().getCursorLocation(cursorLocation);

		UIPoint screenCenter;
		CuiManager::getIoWin().getScreenCenter(screenCenter);

		if (virtualJoystickWasActive && !m_virtualJoystickActive)
			CuiManager::getIoWin().warpCursor(screenCenter.x, screenCenter.y);
		else if (ms_virtualJoystickEnabled && screenCenter != cursorLocation)
		{
			m_virtualJoystickActive = true;

			if (!virtualJoystickWasActive)
				CuiManager::getIoWin().warpCursor(screenCenter.x, screenCenter.y);
			else
			{
				float const virtualJoystickDeadZone = ms_deadZone / static_cast<float>(deadZoneSize);
				float const oo_halfDeadZoneSize = RECIP(static_cast<float>(std::max(1, deadZoneSize / 2))); //lint !e653 // possible loss of a fraction -- intentional

				float const x = static_cast<float>(cursorLocation.x - screenCenter.x);
				if (fabs(x) > ms_deadZone)
					yawPosition += convertValueWithDeadZone(x * oo_halfDeadZoneSize, 1.0f, virtualJoystickDeadZone);

				float const y = static_cast<float>(cursorLocation.y - screenCenter.y);
				if (fabs(y) > ms_deadZone)
					pitchPosition += convertValueWithDeadZone(y * oo_halfDeadZoneSize, 1.0f, virtualJoystickDeadZone);
			}
		}
	}

	//-- Clamp potential mods between -1 and 1

	yawPosition += joystickYawPosition;
	pitchPosition += joystickPitchPosition;
	rollPosition += joystickRollPosition;

	yawPosition = clamp(-1.f, yawPosition, 1.f);
	pitchPosition = clamp(-1.f, pitchPosition, 1.f);
	rollPosition = clamp(-1.f, rollPosition, 1.f);

	if (ms_invertPitch)
		pitchPosition = -pitchPosition;

	if (ms_invertRoll)
		rollPosition = -rollPosition;

	if ((abs(yawPosition) < c_autopilotDeadZone)
		&& (abs(pitchPosition) < c_autopilotDeadZone)
		&& (abs(rollPosition) < c_autopilotDeadZone)
		&& !throttleChanged
		&& (m_boosterWasActive || !owner->isBoosterActive())) //Turning on the booster cancels autopilot.  Having it on does not.
	{
		// regular autopilot requires the controls to be near neutral, but they don't have to be completely neutral:

		if (m_autopilotEngaged)
			doAutopilot(yawPosition, pitchPosition, rollPosition, m_throttlePosition);
		else
		{
			// auto-leveling is pickier:  timer must have elapsed, and controls must be completely neutral:

			if (CuiPreferences::getShipAutolevel() && !m_lockInputState && 
				(abs(yawPosition) < FLT_MIN) && (abs(pitchPosition) < FLT_MIN) && (abs(rollPosition) < FLT_MIN) &&
				m_autoLevelTimer.updateZero(elapsedTime))
			{
				internalEngageAutopilotRollLevel();
			}
		}
	}
	else
	{
		m_autoLevelTimer.reset();
		cancelAutopilot();
	}

	//-- Zero out any input if the input state is locked
	if (m_lockInputState && !m_autopilotEngaged)
	{
		yawPosition = 0.f;
		pitchPosition = 0.f;
		rollPosition = 0.f;
	}

	//-- Clear input state if pilot is frozen
	CreatureObject const * const playerCreature = owner->getPilot();
	if (!playerCreature || playerCreature->getState(States::Frozen))
	{
		yawPosition = 0.f;
		pitchPosition = 0.f;
		rollPosition = 0.f;
		setThrottlePosition(0.f, true);
	}

	//-- Update cockpit
	queue->appendMessage(static_cast<int>(CM_shipDynamicsYaw), yawPosition);
	queue->appendMessage(static_cast<int>(CM_shipDynamicsPitch), pitchPosition);
	queue->appendMessage(static_cast<int>(CM_shipDynamicsRoll), rollPosition);

	//-- Update model
	NOT_NULL(m_shipDynamicsModel);

	GroundScene const * const groundScene = dynamic_cast<GroundScene *>(Game::getScene());

	if (   (groundScene != NULL)
	    && (groundScene->getCurrentView() != GroundScene::CI_cockpit))
	{
		setThrottlePosition(0.f, false);
	}

	if (getShipOwner()->hasCondition(TangibleObject::C_docking))
	{
		Transform const & serverTransform = m_serverShipDynamicsModel->getTransform();
		Transform const & clientTransform = owner->getTransform_o2p();

		if (!clientTransform.approximates(serverTransform))
		{
			//determine new position
			Object const & cell = owner->getParentCell()->getOwner();
			Vector const & serverPosition_w = cell.rotateTranslate_o2w(serverTransform.getPosition_p());
			Vector const & clientPosition_w = owner->getPosition_w();
			Vector const & position_w = Vector::linearInterpolate(clientPosition_w, serverPosition_w, clamp(0.0f, 2.f * elapsedTime, 1.0f));

			//determine new rotation
			Transform const & startTransform = owner->getTransform_o2p();
			Quaternion const startOrientation(startTransform);
			Quaternion const endOrientation(serverTransform);
			Quaternion const & orientation = startOrientation.slerp(endOrientation, clamp(0.0f, 2.f * elapsedTime, 1.0f));

			Transform transform_p;
			orientation.getTransform(&transform_p);
			transform_p.reorthonormalize();
			transform_p.setPosition_p(position_w);

			//apply new rotation/position
			m_shipDynamicsModel->setTransform(transform_p);
			m_shipDynamicsModel->makeStationary();

			CollisionWorld::objectWarped(getShipOwner());
		}
	}
	else
	{
		m_shipDynamicsModel->model(elapsedTime, yawPosition, pitchPosition, rollPosition, getThrottlePosition(), ClientShipObjectInterface(owner));
		NOT_NULL(m_serverShipDynamicsModel);

		m_serverShipDynamicsModel->setTransform(m_shipDynamicsModel->getTransform());
		m_serverShipDynamicsModel->setVelocity(m_shipDynamicsModel->getVelocity());
		m_serverShipDynamicsModel->setYawRate(m_shipDynamicsModel->getYawRate());
		m_serverShipDynamicsModel->setPitchRate(m_shipDynamicsModel->getPitchRate());
		m_serverShipDynamicsModel->setRollRate(m_shipDynamicsModel->getRollRate());
		m_serverShipDynamicsModel->predict(elapsedTime, ClientShipObjectInterface(owner));
	}

	//-- Update ship
	owner->setTransform_o2p(m_shipDynamicsModel->getTransform());

	//-- Fire guns after we've changed to our final transform for this frame
	{
		for (int weapon = 0; weapon < ShipChassisSlotType::cms_numWeaponIndices; ++weapon)
		{
			if ((firingAllWeapons || firingWeapons[weapon]) && !owner->isTurret(weapon))
				IGNORE_RETURN(owner->fireWeapon(weapon));
		}
	}

	//-- Tick transform timers
	m_sendTransformThisFrame = m_sendTransformTimer.updateZero(elapsedTime);
	m_sendReliableTransformThisFrame = m_sendReliableTransformTimer.updateZero(elapsedTime);

	//-- Warn the player if we're getting close to the zone edge
	{
		Vector const & ownerPosition = owner->getPosition_w();
		float const mapSize = TerrainObject::getConstInstance()->getMapWidthInMeters() / 2;

		float const dangerZone = mapSize - 400.0f;
		float const resetZone = mapSize - 800.0f;
		if (!m_nearZoneEdge)
		{
			bool inDanger = abs(ownerPosition.x) > dangerZone ||
				abs(ownerPosition.y) > dangerZone ||
				abs(ownerPosition.z) > dangerZone;
			if (inDanger)
			{
				m_nearZoneEdge=true;
				CuiSystemMessageManager::sendFakeSystemMessage(ClientStringIds::near_zone_edge.localize());
			}
		}
		else
		{
			// already sent warning, check for getting far enough away to clear the warning
			bool outOfDanger = abs(ownerPosition.x) < resetZone &&
				abs(ownerPosition.y) < resetZone &&
				abs(ownerPosition.z) < resetZone;
			if (outOfDanger)
				m_nearZoneEdge=false;
		}
	}

#ifdef _DEBUG
	Camera const * const camera = Game::getCamera();
	if (camera)
	{
		//-- render client transform
		if (ms_renderShipDynamicsModel)
			camera->addDebugPrimitive(new FrameDebugPrimitive(FrameDebugPrimitive::S_none, m_shipDynamicsModel->getTransform(), 1.0f));

		//-- render client transform
		if (ms_renderServerShipDynamicsModel)
			camera->addDebugPrimitive(new FrameDebugPrimitive(FrameDebugPrimitive::S_none, m_serverShipDynamicsModel->getTransform(), 1.0f));

		if (ms_renderShipAppearanceExtent)
		{
			BoxExtent const * const boxExtent = dynamic_cast<BoxExtent const *>(owner->getAppearance() ? owner->getAppearance()->getExtent() : 0);
			if (boxExtent)
				camera->addDebugPrimitive(new BoxDebugPrimitive(BoxDebugPrimitive::S_z, owner->getTransform_o2w(), boxExtent->getBox(), PackedArgb::solidYellow));
		}

		if (ms_renderShipTangibleExtent)
		{
			AxialBox const tangibleExtent = owner->getTangibleExtent();
			camera->addDebugPrimitive(new BoxDebugPrimitive(BoxDebugPrimitive::S_z, owner->getTransform_o2w(), tangibleExtent, PackedArgb::solidWhite));
		}
	}

	if (ms_debugReport)
	{
		DEBUG_REPORT_PRINT(true, ("-- PlayerShipController\n"));
		DEBUG_REPORT_PRINT(true, ("     yawPosition=%1.2f\n", yawPosition));
		DEBUG_REPORT_PRINT(true, ("   pitchPosition=%1.2f\n", pitchPosition));
		DEBUG_REPORT_PRINT(true, ("    rollPosition=%1.2f\n", rollPosition));
		DEBUG_REPORT_PRINT(true, ("throttlePosition=%1.2f\n", m_throttlePosition));
	}
#endif

	m_boosterWasActive=owner->isBoosterActive();

	return ShipController::realAlter(elapsedTime);
}

// ----------------------------------------------------------------------

void PlayerShipController::handleNetUpdateTransform(MessageQueueDataTransform const &message)
{
	Object * const owner = getOwner();
	int const sequenceNumber = message.getSequenceNumber();

	if (owner && owner->isInitialized() && sequenceNumber >= 0)
	{
		if (owner->getAttachedTo())
			owner->setParentCell(CellProperty::getWorldCellProperty());

		CellProperty::setPortalTransitionsEnabled(false);
			owner->setTransform_o2p(message.getTransform());
		CellProperty::setPortalTransitionsEnabled(true);

		CollisionWorld::objectWarped(owner);

		m_shipDynamicsModel->setTransform(message.getTransform());
		m_shipDynamicsModel->makeStationary();

		m_serverShipDynamicsModel->setTransform(message.getTransform());
		m_serverShipDynamicsModel->makeStationary();
	}

	appendMessage(
		static_cast<int>(CM_teleportAck),
		0.f,
		new MessageQueueTeleportAck(sequenceNumber),
		GameControllerMessageFlags::SEND |
		GameControllerMessageFlags::RELIABLE |
		GameControllerMessageFlags::DEST_AUTH_SERVER);
}

// ----------------------------------------------------------------------

void PlayerShipController::handleNetUpdateTransformWithParent(MessageQueueDataTransformWithParent const & /*message*/)
{
	DEBUG_FATAL(true, ("PlayerShipController::handleNetUpdateTransformWithParent: not implemented (nor should it be)"));
}

//----------------------------------------------------------------------

bool PlayerShipController::isVirtualJoystickActive() const
{
	return m_virtualJoystickActive;
}

//----------------------------------------------------------------------

float PlayerShipController::getThrottlePosition() const
{
	ShipObject const * const shipOwner = getShipOwner();
	if (NULL != shipOwner && shipOwner->isBoosterActive())
		return 1.0f;

	return m_throttlePosition;
}

//----------------------------------------------------------------------

void PlayerShipController::setThrottlePosition(float throttlePosition, bool stopAutopilot)
{
	if (m_lockInputState)
	{
		return;
	}

	m_throttlePosition = clamp(0.0f, throttlePosition, 1.0f);

	// joystick doesn't always return a pure zero, so clamp it when it gets sufficiently small
	if (m_throttlePosition < 0.01f)
		m_throttlePosition = 0.0f;

	if (stopAutopilot)
		cancelAutopilot();
}

//----------------------------------------------------------------------

void PlayerShipController::setThrottlePositionFromJoystick(float joystickThrottlePosition, bool forceOverride)
{
	if (m_lockInputState)
	{
		return;
	}

	//-- only use joystick throttle input if it deviates more than epsilon from the last recorded joystick throttle position
	static float const epsilon = 0.01f;
	if (!forceOverride && WithinEpsilonExclusive(m_lastJoystickThrottlePosition, joystickThrottlePosition, epsilon))
	{
		return;
	}

	m_lastJoystickThrottlePosition = joystickThrottlePosition;
	setThrottlePosition(m_lastJoystickThrottlePosition, true);
}


//----------------------------------------------------------------------

#ifdef ENABLE_FORMATIONS
void PlayerShipController::setInFormation(bool const inFormation)
{
	m_inFormation = inFormation;
	lockInputState(m_inFormation, true);
}
#endif

// ======================================================================
// PROTECTED PlayerShipController
// ======================================================================

void PlayerShipController::sendTransform(bool const reliable)
{
	if (getShipOwner()->hasCondition(TangibleObject::C_docking))
		return;

	if ((m_lockInputState) && (!m_allowTransformsWhileLocked))
	{
		if (m_sentFinalTransform)
			return;

		m_sentFinalTransform = true;
	}

	Object const * const owner = getOwner();
	NOT_NULL(owner);
	DEBUG_FATAL(owner->getAttachedTo(), ("PlayerShipController::sendTransform: movement within a cell not implemented"));

	if (GameNetwork::isConnectedToConnectionServer())
	{
		//-- Send transform update to server
		ShipUpdateTransformMessage const shipUpdateTransformMessage(
			0,
			m_shipDynamicsModel->getTransform(),
			m_lockInputState ? Vector::zero : m_shipDynamicsModel->getVelocity(),
			m_lockInputState ? 0.f : m_shipDynamicsModel->getYawRate(),
			m_lockInputState ? 0.f : m_shipDynamicsModel->getPitchRate(),
			m_lockInputState ? 0.f : m_shipDynamicsModel->getRollRate(),
			GameNetwork::getServerSyncStampLong());
		
		GameNetwork::send(shipUpdateTransformMessage, reliable);
	}
	
	//-- Update server model
	m_serverShipDynamicsModel->setTransform(m_shipDynamicsModel->getTransform());
	m_serverShipDynamicsModel->setVelocity(m_shipDynamicsModel->getVelocity());
}

// ----------------------------------------------------------------------

void PlayerShipController::handleMessage(int message, float value, const MessageQueue::Data* data, uint32 flags)
{
	switch(message)
	{
	case CM_shipDamageMessage:
		{
			Object const * const playerObject = Game::getPlayer();
			if (playerObject)
			{
				typedef MessageQueueGenericValueType<ShipDamageMessage> DamageMsg;
				DamageMsg const * const damageMessage = safe_cast<DamageMsg const * const>(data);
				if (damageMessage)
				{
					ShipDamageMessage const & shipDamage = damageMessage->getValue();

					if (shipDamage.getAttackerNetworkId() != NetworkId::cms_invalid)
					{
						CuiDamageManager::handleShipDamage(shipDamage);

						GameMusicManager::startCombatMusic(shipDamage.getAttackerNetworkId());
					}
				}
			}
		}
		break;
	case CM_addIgnoreIntersect:
		{
			const MessageQueueGenericValueType<NetworkId> * const msg = NON_NULL (safe_cast<const MessageQueueGenericValueType<NetworkId> *>(data));

			CollisionCallbackManager::addIgnoreIntersect(getOwner()->getNetworkId(), msg->getValue());

			DEBUG_REPORT_LOG(true, ("CM_addIgnoreIntersect object(%s) ignoreObject(%s)\n", getOwner()->getNetworkId().getValueString().c_str(), msg->getValue().getValueString().c_str()));
		}
		break;
	case CM_removeIgnoreIntersect:
		{
			const MessageQueueGenericValueType<NetworkId> * const msg = NON_NULL (safe_cast<const MessageQueueGenericValueType<NetworkId> *>(data));

			CollisionCallbackManager::removeIgnoreIntersect(getOwner()->getNetworkId(), msg->getValue());

			DEBUG_REPORT_LOG(true, ("CM_removeIgnoreIntersect object(%s) ignoreObject(%s)\n", getOwner()->getNetworkId().getValueString().c_str(), msg->getValue().getValueString().c_str()));
		}
		break;
	default:
		ShipController::handleMessage(message, value, data, flags);
		break;
	}

}

//----------------------------------------------------------------------

void PlayerShipController::lockInputState(bool lock, bool allowTransforms)
{
	m_lockInputState = lock;
	m_allowTransformsWhileLocked = allowTransforms;

	//-- Reset the need to send a zero velocity if we've unlocked the input state
	if (!m_lockInputState)
		m_sentFinalTransform = false;
}

//----------------------------------------------------------------------

bool PlayerShipController::isInputStateLocked() const
{
	return m_lockInputState;
}

//----------------------------------------------------------------------

/**
 * Roll the ship to be upright, but do not change pitch or yaw
 */
void PlayerShipController::internalEngageAutopilotRollLevel()
{
	internalEngageAutopilot(AM_rollLevel);
}

//----------------------------------------------------------------------

/**
 * Level the ship onto the xz plane
 */
void PlayerShipController::internalEngageAutopilotFullyLevel()
{
	ShipObject * const owner = getShipOwner();
	if (!owner)
		return;

	internalEngageAutopilot(AM_fullyLevel);
}

//----------------------------------------------------------------------

void PlayerShipController::internalEngageAutopilotToLocation(Vector const & target)
{
	ShipObject * const owner = getShipOwner();
	if (!owner)
		return;

	m_autopilotTargetLocation = target;
	internalEngageAutopilot(AM_toLocation);
}

//----------------------------------------------------------------------

void PlayerShipController::internalEngageAutopilotToDirection(Vector const & direction)
{
	ShipObject * const owner = getShipOwner();
	if (!owner)
		return;

	m_autopilotTargetHeading = direction;
	internalEngageAutopilot(AM_toHeading);
}

//----------------------------------------------------------------------

void PlayerShipController::internalEngageAutopilot(AutopilotMode const newMode)
{
	if ((!m_autopilotEngaged) && (newMode != AM_rollLevel))
	{
		CuiSystemMessageManager::sendFakeSystemMessage(SpaceStringIds::autopilot_engaged.localize());
		m_sentAutoPilotEngagedMessage = true;
	}

	m_autopilotEngaged = true;
	m_autopilotMode = newMode;

	if (!CuiManager::getPointerToggledOn())
	{
		UIPoint screenCenter;
		CuiManager::getIoWin().getScreenCenter(screenCenter);
		CuiManager::getIoWin().warpCursor(screenCenter.x, screenCenter.y);
	}
}

//----------------------------------------------------------------------

/**
 * Handle the ship's autopilot.
 *
 * m_autopilotMode determines what mode of autopilot the ship is using.  It
 * has these possible values:
 *
 * AM_rollLevel   Roll to point the wings at the horizon, but do not pitch or yaw.
 * AM_toHeading   Pitch and yaw to reach the specified heading, while keeping the ship upright
 * AM_toLocation  Fly to the specified location, then cut the throttle and level the ship
 */
void PlayerShipController::doAutopilot(float & yawPosition, float & pitchPosition, float & rollPosition, float & throttlePosition)
{
	DEBUG_FATAL(!m_autopilotEngaged, ("Programmer bug:  called PlayerShipController::doAutopilot when m_autopilotEngaged was false.\n"));

	ShipObject const * const owner = getShipOwner();
	if (!owner)
		return;

	Vector avoidancePosition_w;
	// Goal distance doesn't really matter for what we're using this test for, so it's arbitrarily set to where we'll be in 10 seconds:
	if (m_autopilotMode != AM_rollLevel && m_autopilotMode != AM_follow && 
		SpaceAvoidanceManager::getAvoidancePosition(*owner, m_shipDynamicsModel->getVelocity(), owner->getPosition_w() + m_shipDynamicsModel->getVelocity() * 10.0f, avoidancePosition_w, true))
	{
		cancelAutopilot();
		throttlePosition = 0;			
		CuiSystemMessageManager::sendFakeSystemMessage(ClientStringIds::autopilot_obstacle.localize());
		return;					
	}

	switch(m_autopilotMode)
	{
		case AM_rollLevel:
		{
			bool const isLevel = doAutopilotRollLevel(rollPosition);
			if (isLevel)
				cancelAutopilot();
			break;
		}

		case AM_fullyLevel:
		{
			m_autopilotTargetHeading = owner->getTransform_o2w().getLocalFrameK_p();
			m_autopilotTargetHeading.y=0;
			if (!m_autopilotTargetHeading.normalize())
				m_autopilotTargetHeading = Vector::unitZ; // ship is probably pointing straight up or down, so +Z is as good a direction to turn to as any

			IGNORE_RETURN(doAutopilotRollLevel(rollPosition));
			float const result = doAutopilotTurnToHeading(pitchPosition, yawPosition);
			if (result < c_autopilotTolerance)
				internalEngageAutopilotRollLevel();
			break;
		}

		case AM_toHeading:
		{
			IGNORE_RETURN(doAutopilotRollLevel(rollPosition));
			float const result = doAutopilotTurnToHeading(pitchPosition, yawPosition);
			if (result < c_autopilotTolerance)
				internalEngageAutopilotRollLevel();
			break;
		}

		case AM_toLocation:
		{
			Vector const & shipPosition = owner->getPosition_w();
			m_autopilotTargetHeading =  m_autopilotTargetLocation - shipPosition;

			float const result = doAutopilotTurnToHeading(pitchPosition, yawPosition);
			if (result < c_autopilotTolerance) // only roll level when mostly on target, to avoid "drunken autopilot"
				IGNORE_RETURN(doAutopilotRollLevel(rollPosition));

			// Adjust throttle.
			float const distanceToTarget = m_autopilotTargetHeading.approximateMagnitude();

			if (distanceToTarget < c_autopilotReachedTargetTolerance)
			{
				throttlePosition = 0.0f;
				internalEngageAutopilotRollLevel();
			}
			else
				throttlePosition = std::max(0.0f, computeRequiredAcceleration(0.0f, distanceToTarget, m_shipDynamicsModel->getVelocity().approximateMagnitude(),
					getShipOwner()->getShipActualDecelerationRate(), getShipOwner()->getShipActualSpeedMaximum()));

			// reduce throttle if not pointing the right direction
			if (result > c_autopilotTolerance)
			{
				float const multiplier = distanceToTarget < c_autopilotTurnSlowDistance ? 1.0f : 0.5f;

				if (result > c_autopilotBadlyOffTargetTolerance)
					throttlePosition -= throttlePosition * multiplier;
				else
					throttlePosition = throttlePosition * (1.0f - (multiplier * (result / c_autopilotBadlyOffTargetTolerance)));
			}
			break;
		}

		case AM_follow:
		{
			ShipObject const * const shipToFollow = getShipToFollow();
			if (shipToFollow != NULL)
			{
				float const maxSpeed = owner->getShipActualSpeedMaximum();
				if (maxSpeed > 0.0f)
					setThrottlePosition(shipToFollow->getCurrentSpeed() / maxSpeed, false);

				float distanceToTarget = (shipToFollow->getPosition_p()-owner->getPosition_p()).magnitude();
				if (shipToFollow->getAppearance())
					distanceToTarget -= shipToFollow->getAppearance()->getSphere().getRadius();
				if (owner->getAppearance())
					distanceToTarget -= owner->getAppearance()->getSphere().getRadius();

				if (distanceToTarget > c_shipFollowDistance+c_shipFollowTolerance)
					setThrottlePosition(1.f, false);
				else if (distanceToTarget < c_shipFollowDistance-c_shipFollowTolerance)
					setThrottlePosition(0.f, false);

				m_autopilotTargetHeading = shipToFollow->getPosition_p()-owner->getPosition_p();
				IGNORE_RETURN(doAutopilotTurnToHeading(pitchPosition, yawPosition));
			}
			else
				cancelAutopilot();
		}
	}
}

//----------------------------------------------------------------------

/**
 * Autopilot helper function.  Rolls the ship upright (but does not pitch or yaw).
 *
 * @return true if the ship is upright
 */
bool PlayerShipController::doAutopilotRollLevel(float & rollPosition)
{
	Vector const & y_o = NON_NULL(getShipOwner())->rotate_w2o(Vector::unitY);
	Vector const & target_o = y_o.cross(Vector::unitZ);	 // target vector is perpendicular to global "up" and local "forward"
	if (target_o.magnitudeSquared() > sqr(Vector::NORMALIZE_THRESHOLD))
	{
		// goal is to get the local "x" vector (representing the direction the right wing points) to the target vector
		float rollAngle = -atan2(target_o.y,target_o.x);

		rollPosition = computeRequiredAcceleration(rollAngle, 0.0f, m_shipDynamicsModel->getRollRate(),
			getShipOwner()->getShipActualRollAccelerationRate(), getShipOwner()->getShipActualRollRateMaximum());

		return abs(rollAngle) < c_autopilotTolerance;
	}
	else
		return true; // pointing straight up or down counts as being roll-level
}

//----------------------------------------------------------------------

/**
 * Autopilot helper function.  Given a current location & speed, compute how much speed to request
 * to reach a desired position.  (This can also be used for angles)
 * @return -1 to 1, indicating the % of maximum speed to request.  It is assumed the object will accelerate
 * to reach that speed while staying within the specified acceleration limit.
 */
float PlayerShipControllerNamespace::computeRequiredAcceleration(float const currentPosition, float const targetPosition, float const currentSpeed, float const acceleration, float const maxSpeed)
{
	// figure out the where we'd be if we tried to stop moving right now
	float const timeToStop = abs(currentSpeed / acceleration);
	float const stopPosition = currentPosition + ((currentSpeed>0) ? 1.0f:-1.0f) * 0.5f * acceleration * sqr(timeToStop);

	// if needed, accelerate to move the stop point towards the target point
	float desiredSpeed = 0.0f;
	if (stopPosition < targetPosition && currentPosition < targetPosition)
		desiredSpeed = maxSpeed;
	else if (stopPosition > targetPosition && currentPosition > targetPosition)
		desiredSpeed = -maxSpeed;

	// This is not a continuous system.  If we request full speed, we may miss the target point during the gap
	// between frames.  So, limit the requested speed to no more than what would reach the target within 0.1 seconds
	// (We assume we'll either get another frame within 0.1 seconds, or the frame rate is bad enough it won't look smooth
	// no matter what we do.)
	float const speedLimit = (targetPosition-currentPosition) / (0.1f);
	if (abs(desiredSpeed) > abs(speedLimit))
		desiredSpeed = speedLimit;

	return clamp (-1.0f, desiredSpeed / maxSpeed, 1.0f);
}

//----------------------------------------------------------------------

/**
 * Autopilot helper function.  Turns to the specified heading.
 *
 * @return (very) approximate error between the ship's heading and the specified heading, in radians
 */

float PlayerShipController::doAutopilotTurnToHeading(float & pitchPosition, float & yawPosition)
{
	Vector const & targetHeading_o = NON_NULL(getShipOwner())->rotate_w2o(m_autopilotTargetHeading);

	real const theta = targetHeading_o.theta();
	real const phi = targetHeading_o.phi();

	pitchPosition = computeRequiredAcceleration(-phi, 0.0f, m_shipDynamicsModel->getPitchRate(),
		getShipOwner()->getShipActualPitchAccelerationRate(), getShipOwner()->getShipActualPitchRateMaximum());

	yawPosition = computeRequiredAcceleration(-theta, 0.0f, m_shipDynamicsModel->getYawRate(),
		getShipOwner()->getShipActualYawAccelerationRate(), getShipOwner()->getShipActualYawRateMaximum());

	return abs(phi) + abs(theta);
}

//----------------------------------------------------------------------

void PlayerShipController::internalEngageAutopilotFollow(ShipObject const & target)
{	
	if (target.isEnemy())
		return;
	
	m_shipToFollow = CachedNetworkId(target);
	internalEngageAutopilot(AM_follow);
}

//----------------------------------------------------------------------

ShipObject const * PlayerShipController::getShipToFollow() const
{
	ClientObject const * const clientObject = safe_cast<ClientObject const *>(m_shipToFollow.getObject());
	ShipObject const * const shipObject = (clientObject != NULL) ? clientObject->asShipObject() : NULL;

	// check for a ship that can't be followed.  (This is done repeatedly because enemy state can change.)
	if (shipObject && shipObject->isEnemy())
		return NULL;

	return shipObject;
}

//----------------------------------------------------------------------

bool PlayerShipController::isFollowing() const
{
	return m_autopilotMode == AM_follow;
}

//----------------------------------------------------------------------

bool PlayerShipController::isAutoPilotEngaged() const
{
	return m_autopilotEngaged;
}

//----------------------------------------------------------------------

void PlayerShipController::cancelAutopilot()
{
	if ((m_autopilotEngaged) && (m_sentAutoPilotEngagedMessage))
	{
		CuiSystemMessageManager::sendFakeSystemMessage(SpaceStringIds::autopilot_disengaged.localize());
	}

	m_sentAutoPilotEngagedMessage = false;
	m_autopilotEngaged = false;
}


//----------------------------------------------------------------------
// Public versions of the autopilot engage functions, cancel following & other
// autopilot settings, then engage the autopilot.

void PlayerShipController::engageAutopilotRollLevel()
{
	cancelAutopilot();
	internalEngageAutopilotRollLevel();
}

//----------------------------------------------------------------------

void PlayerShipController::engageAutopilotFullyLevel()
{
	cancelAutopilot();
	internalEngageAutopilotFullyLevel();
}

//----------------------------------------------------------------------

void PlayerShipController::engageAutopilotToLocation(Vector const & target)
{
	cancelAutopilot();
	turnOffBooster();
	internalEngageAutopilotToLocation(target);
}

//----------------------------------------------------------------------

void PlayerShipController::engageAutopilotToDirection(Vector const & direction)
{
	cancelAutopilot();
	internalEngageAutopilotToDirection(direction);
}

//----------------------------------------------------------------------

void PlayerShipController::engageAutopilotFollow(ShipObject const & target)
{
	cancelAutopilot();
	turnOffBooster();
	internalEngageAutopilotFollow(target);
}

//----------------------------------------------------------------------

void PlayerShipController::matchSpeed(ShipObject const & target)
{
	ShipObject * const owner = getShipOwner();
	if (!owner)
		return;

	float const maxSpeed = owner->getShipActualSpeedMaximum();
	if (maxSpeed > 0.0f)
		setThrottlePosition(target.getCurrentSpeed() / maxSpeed, false);
}

//----------------------------------------------------------------------

void PlayerShipController::turnOffBooster() const
{
	static uint32 const hash_boosterOff = Crc::normalizeAndCalculate("boosterOff");

	ShipObject const * const owner = getShipOwner();
	if (!owner)
		return;

	if (owner->isBoosterActive())
		IGNORE_RETURN(ClientCommandQueue::enqueueCommand(hash_boosterOff, NetworkId::cms_invalid, Unicode::emptyString));
}

// ======================================================================
