//===================================================================
//
// RemoteCreatureController.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/RemoteCreatureController.h"

#include "clientGame/ConfigClientGame.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGraphics/Graphics.h"
#include "clientGame/GroundScene.h"
#include "clientGame/ShipController.h"
#include "clientGame/ShipObject.h"
#include "clientGame/ShipStation.h"
#include "clientGraphics/DebugPrimitive.h"
#include "clientObject/GameCamera.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "sharedCollision/CollisionWorld.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedMath/Vector2d.h"
#include "sharedNetworkMessages/MessageQueueDataTransform.h"
#include "sharedNetworkMessages/MessageQueueDataTransformWithParent.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/NetworkIdManager.h"
#include "swgSharedUtility/Postures.def"
#include "swgSharedUtility/States.def"

//===================================================================

namespace RemoteCreatureControllerNamespace
{
	float const cms_stopThreshold                        = 0.25f;
	float const cms_stuckWarpTimeThreshold               = 0.5f;
//	float const cms_runningAnimationThreshold            = 0.95f;

	float const ms_minimumCreatureRotation               = 1.0f / 4096.0f;

	//The amount that a corpse's position can be different than the server's
	//before we insist on warping the corpse to the correct spot.
	float const ms_allowDeadSliding                      = 10.0f * 10.0f;

	//
	// the animationSpeedFilterFactor is inverse of the the amount of time
	// it would take for the animation speed to equal the server speed,
	// so, if the factor is 4.0, then it would take 0.25 seconds for
	// the speeds to match up.
	//
	float const cms_animationSpeedFilterFactor           = 100.0f;
	
	// The decay factor for when the creature should be stopped at its
	// destination already. The closer this is to 1.0 the more running
	// in place the creature will do. The closer to 0.0 the faster they
	// will snap to standing
	float const cms_stopSpeedDecayFactor                 = 0.75f;

	bool        ms_disableDeadReckoning                  = false;
	bool        ms_logBadServerTransforms                = false;
	bool        ms_logServerMovementData                 = false;
	bool        ms_renderClientTransform                 = false;
	bool        ms_renderServerTransform                 = false;
	bool        ms_renderLineFromClientToServerTransform = false;
	bool        ms_renderClientPath                      = false;
	bool        ms_renderServerPath                      = false;
	bool        ms_disablePathUpdate                     = false;
	bool        ms_disableAcceleration                   = false;

#ifdef _DEBUG
	bool        ms_reportTransforms                      = false;
	bool        ms_printDeadReckoningUsage               = false;
#endif

	uint        ms_clientPathSize                        = 100;
	uint        ms_serverPathSize                        = 50;


	const Vector getPosition_w (const Object* const cellObject, const Transform& transform_p)
	{
		return cellObject ? cellObject->rotateTranslate_o2w (transform_p.getPosition_p ()) : Vector::zero;
	}

	const Vector getLocalFrameK_w (const Object* const cellObject, const Transform& transform_p)
	{
		return cellObject ? cellObject->rotate_o2w (transform_p.getLocalFrameK_p ()) : Vector::zero;
	}

	// A constant from 0 to 1 specifying how much to believe the server's speed versus the client's
	float const ANIM_SPEED_SERVER_WEIGHT_FACTOR_PLAYER	= 0.8f;
	float const ANIM_SPEED_SERVER_STOP_FACTOR_PLAYER    = 0.3f;
	
	float const ANIM_SPEED_SERVER_WEIGHT_FACTOR_NPC  	= 0.98f;
	float const ANIM_SPEED_SERVER_STOP_FACTOR_NPC       = 0.3f;

}

using namespace RemoteCreatureControllerNamespace;

//===================================================================
// STATIC PUBLIC RemoteCreatureController
//===================================================================

void RemoteCreatureController::install ()
{
	InstallTimer const installTimer("RemoteCreatureController::install");

	DebugFlags::registerFlag (ms_disableDeadReckoning,     "ClientGame/RemoteCreatureController", "disableDeadReckoning");
	DebugFlags::registerFlag (ms_logBadServerTransforms,   "ClientGame/RemoteCreatureController", "logBadServerTransforms");
	DebugFlags::registerFlag (ms_logServerMovementData,    "ClientGame/RemoteCreatureController", "logServerMovementData");
	DebugFlags::registerFlag (ms_renderClientTransform,    "ClientGame/RemoteCreatureController", "renderClientTransform");
	DebugFlags::registerFlag (ms_renderServerTransform,    "ClientGame/RemoteCreatureController", "renderServerTransform");
	DebugFlags::registerFlag (ms_renderLineFromClientToServerTransform, "ClientGame/RemoteCreatureController", "renderLineFromClientToServerTransform");
	DebugFlags::registerFlag (ms_renderClientPath,         "ClientGame/RemoteCreatureController", "renderClientPath");
	DebugFlags::registerFlag (ms_renderServerPath,         "ClientGame/RemoteCreatureController", "renderServerPath");
	DebugFlags::registerFlag (ms_disablePathUpdate,        "ClientGame/RemoteCreatureController", "disablePathUpdate");

#ifdef _DEBUG
	DebugFlags::registerFlag (ms_reportTransforms,         "ClientGame/RemoteCreatureController", "reportTransforms");
	DebugFlags::registerFlag (ms_printDeadReckoningUsage,  "ClientGame/RemoteCreatureController", "printDeadReckoningUsage");
#endif

	ExitChain::add (remove, "RemoteCreatureController::remove");
}

//===================================================================
// STATIC PRIVATE RemoteCreatureController
//===================================================================

void RemoteCreatureController::remove ()
{
	DebugFlags::unregisterFlag (ms_disableDeadReckoning);
	DebugFlags::unregisterFlag (ms_logBadServerTransforms);
	DebugFlags::unregisterFlag (ms_logServerMovementData);
	DebugFlags::unregisterFlag (ms_renderClientTransform);
	DebugFlags::unregisterFlag (ms_renderServerTransform);
	DebugFlags::unregisterFlag (ms_renderLineFromClientToServerTransform);
	DebugFlags::unregisterFlag (ms_renderClientPath);
	DebugFlags::unregisterFlag (ms_renderServerPath);
	DebugFlags::unregisterFlag (ms_disablePathUpdate);
#ifdef _DEBUG
	DebugFlags::unregisterFlag (ms_reportTransforms);
	DebugFlags::unregisterFlag (ms_printDeadReckoningUsage);
#endif
}

//===================================================================
// PUBLIC RemoteCreatureController
//===================================================================

RemoteCreatureController::RemoteCreatureController (CreatureObject* const newOwner) :
	CreatureController (newOwner),
	m_serverSequenceNumber (0),
	m_serverCellObject (&CellProperty::getWorldCellProperty ()->getOwner ()),
	m_serverTransform_p (),
	m_serverSpeed (0.f),
	m_currentSpeed (0.f),
	m_currentAnimationSpeed (0.f),
	m_pauseDeadReckoning (false),
	m_pauseDeadReckoningTimer (0.f),
	m_lastPosition_w (),
	m_distanceToMoveLastFrame (0.f),
	m_stuckWarpTimer (0.0f),
	m_desiredSpeed (0.0f),
	m_serverLookAtYaw(0.0f)

#ifdef _DEBUG
	, //for previous line
        m_clientPath (new VectorList),
	m_serverPath (new VectorList)
#endif
{
}

//-------------------------------------------------------------------

RemoteCreatureController::~RemoteCreatureController ()
{
#ifdef _DEBUG
	delete m_clientPath;
	m_clientPath = 0;

	delete m_serverPath;
	m_serverPath = 0;
#endif
}

//-------------------------------------------------------------------

void RemoteCreatureController::updateDeadReckoningModel (const CellProperty* const cell, const Transform& transform_p, const float speed)
{
	CreatureObject *creatureObject = safe_cast<CreatureObject *>(getOwner());
	updateDeadReckoningModel(cell, transform_p, speed, creatureObject->getLookAtYaw());
}

//-------------------------------------------------------------------

void RemoteCreatureController::updateDeadReckoningModel (const CellProperty* const cell, const Transform& transform_p, const float speed, float lookAtYaw)
{
	m_serverCellObject  = &cell->getOwner ();
	m_serverTransform_p = transform_p;
	m_serverSpeed       = speed;
	m_serverLookAtYaw   = lookAtYaw;

#ifdef _DEBUG
	//-- add to server path
	if (!ms_disablePathUpdate && ms_renderServerPath)
	{
		while (m_serverPath->size () >= ms_serverPathSize)
			IGNORE_RETURN(m_serverPath->erase (m_serverPath->begin ()));

		const Vector position = getPosition_w (m_serverCellObject, m_serverTransform_p);
		if (m_serverPath->empty () || m_serverPath->back () != position)
			m_serverPath->push_back (position);
	}
#endif
}

//-------------------------------------------------------------------

float RemoteCreatureController::getCurrentSpeed () const
{
	return m_currentSpeed;
}

//-------------------------------------------------------------------

void RemoteCreatureController::warpTransform (const Object* const cellObject, const Transform& transform_p)
{
	m_stuckWarpTimer = 0.0f;

	if (!cellObject)
	{
		DEBUG_WARNING (true, ("RemoteCreatureController: [%s] desired cell is NULL", getOwner ()->getNetworkId ().getValueString ().c_str ()));
		return;
	}

	//-- set our cell to the cell
	const CellProperty* const cell = cellObject->getCellProperty ();
	if (getOwner ()->getParentCell () != cell)
		getOwner ()->setParentCell (const_cast<CellProperty*> (cell));

	//-- set our transform to the transform
	CellProperty::setPortalTransitionsEnabled (false);
		getOwner ()->setTransform_o2p (transform_p);
	CellProperty::setPortalTransitionsEnabled (true);

	m_lastPosition_w = getOwner ()->getPosition_w ();
	m_distanceToMoveLastFrame = 0.f;

	CollisionWorld::objectWarped(getOwner());
}

//-------------------------------------------------------------------

void RemoteCreatureController::endBaselines ()
{
	updateDeadReckoningModel (getOwner ()->getParentCell (), getOwner ()->getTransform_o2p (), 0.f);

	CreatureController::endBaselines();
}

//-------------------------------------------------------------------

float RemoteCreatureController::realAlter (const float elapsedTime)
{
	NP_PROFILER_AUTO_BLOCK_DEFINE ("RemoteCreatureController::realAlter");
	
	CreatureObject* const creatureObject = safe_cast<CreatureObject*> (getOwner ());

	if (!creatureObject)
	{
		DEBUG_WARNING (true, ("RemoteCreatureController has owner with NULL owner, skipping RemoteCreatureController::realAlter ()."));
		return CreatureController::realAlter (elapsedTime);
	}

	//-- Mounts: figure out how to interact with dead reckoning.
	//
	// Mounted creature (creature being ridden):
	//   Do this as long as we're not the player's mounted creature.  The player's mounted creature
	//   is controlled via the PlayerCreatureController so this logic is unnecessary.
	//   @todo: -TRF- ensure we have a way to snap the mount to the correct position if the server
	//          resets the position of the mount.
	//
	// Rider:
	//   Always ignore dead reckoning on the rider.  The rider's position as sent from the server will
	//   be in world space, but on the client this needs to be snapped to the creature in ridden mount
	//   space.
	//
	// We also don't want to do dead reckoning for pilots or droids of ships.
	//
	// @todo: Generalize this at some point.  We should likely not do dead reckoning for creatures
	//        in slots of any kind, in addition to the mount for this client's player.

	bool const isMountForAndDrivenByClientPlayer = creatureObject->isMountForAndDrivenByClientPlayer();

	bool const ignoreDeadReckoning =
		isMountForAndDrivenByClientPlayer ||
		creatureObject->getState(States::RidingMount) ||
		creatureObject->getShipStation() != ShipStation::ShipStation_None;

	if (!ignoreDeadReckoning)
	{
		if (m_pauseDeadReckoning)
		{
			DEBUG_REPORT_PRINT (ms_printDeadReckoningUsage && isOwnerAnimationDebuggerTarget () && m_pauseDeadReckoning, ("dead reckoning usage: object id [%s]: disabled (paused)\n", creatureObject->getNetworkId ().getValueString ().c_str ()));
			DEBUG_REPORT_PRINT (ms_printDeadReckoningUsage && isOwnerAnimationDebuggerTarget () && ((creatureObject->getVisualPosture () == Postures::Incapacitated) || (creatureObject->getVisualPosture () == Postures::Dead)), ("dead reckoning usage: object id [%s]: disabled (posture=dead/incapacitated)\n", creatureObject->getNetworkId ().getValueString ().c_str ()));

			Vector const serverPosition_w = getPosition_w (m_serverCellObject, m_serverTransform_p);
 			Vector const clientPosition_w = creatureObject->getPosition_w ();
			if ((creatureObject->getVisualPosture () == Postures::Incapacitated) || (creatureObject->getVisualPosture () == Postures::Dead) && ((serverPosition_w != clientPosition_w) || (m_serverCellObject && m_serverCellObject->getCellProperty () != creatureObject->getParentCell ())))
			{
				//-- warp to desired
				warpTransform (m_serverCellObject, m_serverTransform_p);
			}

			if (m_pauseDeadReckoningTimer.updateZero (elapsedTime))
				m_pauseDeadReckoning = false;
		}
		else
		{
			if (ms_disableDeadReckoning || (creatureObject->getVisualPosture () == Postures::Incapacitated) || (creatureObject->getVisualPosture () == Postures::Dead))
			{
				DEBUG_REPORT_PRINT (ms_printDeadReckoningUsage && isOwnerAnimationDebuggerTarget (), ("dead reckoning usage: object id [%s]: warp-only (DR globally disabled)\n", creatureObject->getNetworkId ().getValueString ().c_str ()));

				//-- warp to desired
				bool doWarp = true;

				if((creatureObject->getVisualPosture () == Postures::Incapacitated) || (creatureObject->getVisualPosture () == Postures::Dead))
				{
					Vector const serverPosition_w = getPosition_w (m_serverCellObject, m_serverTransform_p);
 					Vector const clientPosition_w = creatureObject->getPosition_w ();
					if(serverPosition_w.magnitudeBetweenSquared(clientPosition_w) < ms_allowDeadSliding)
						doWarp = false;
				}

				if(doWarp)
					warpTransform (m_serverCellObject, m_serverTransform_p);
			}
			else
			{
				// Check if primary animation controller's action track has priority and is at priority >= locomotion.
				// If so, the animation needs to drive locomotion so we need to disable dead reckoning.
				//bool const onlyAnimationControlsLocomotion = doesBodyPlayOnceHaveLocomotionPriority (true);

				//if (onlyAnimationControlsLocomotion)
				//{
				//	DEBUG_REPORT_PRINT (ms_printDeadReckoningUsage && isOwnerAnimationDebuggerTarget (), ("dead reckoning usage: object id [%s]: disabled due to high-loc-priority play-once anim\n", creatureObject->getNetworkId ().getValueString ().c_str ()));
				//}
				//else
				{
					// Do the real dead reckoning.
					DEBUG_REPORT_PRINT (ms_printDeadReckoningUsage && isOwnerAnimationDebuggerTarget (), ("dead reckoning usage: object id [%s]: dead reckoning enabled\n", creatureObject->getNetworkId ().getValueString ().c_str ()));

					//-- determine our desired speed
					Vector const serverPosition_w = getPosition_w (m_serverCellObject, m_serverTransform_p);
 					Vector const clientPosition_w = creatureObject->getPosition_w ();
					Vector2d const serverPosition2d_w(serverPosition_w.x, serverPosition_w.z);
					Vector2d const clientPosition2d_w(clientPosition_w.x, clientPosition_w.z);
					float const yThreshold = 4.5f; // This value is based on the height of an average player. If we are in a tightly compact building (i.e. medic center) we'll still warp properly.
					float const goalDistance2d = clientPosition2d_w.magnitudeBetween(serverPosition2d_w);
					float const walkThreshold = creatureObject->getMaximumWalkSpeed();
					float const maximumSpeed = creatureObject->getMaximumRunSpeed();
					float const warpTolerance = std::max (creatureObject->getWarpTolerance(), ((maximumSpeed * 3.0f) + m_currentSpeed));
					bool const usesAnimationLocomotion = creatureObject->getClientUsesAnimationLocomotion();

					m_desiredSpeed = 0.0f;

					if (usesAnimationLocomotion)
					{
						//-- stop
						m_desiredSpeed = 0.f;

					}
					else if (((fabs(serverPosition_w.y - clientPosition_w.y) > yThreshold)) && (goalDistance2d < cms_stopThreshold))
					{
						//-- we're moving in y but not in x or z, so we're in an elevator or other conveyance; warp to desired
						warpTransform (m_serverCellObject, m_serverTransform_p);
					}
					else if (goalDistance2d < cms_stopThreshold)
					{
						//-- stop
						m_desiredSpeed = 0.f;

					}
					else if (goalDistance2d < walkThreshold)
					{
						//-- walk
						m_desiredSpeed = goalDistance2d;
					}
					else if (goalDistance2d < maximumSpeed)
					{
						//-- run
						m_desiredSpeed = maximumSpeed;
					}
					else if (goalDistance2d < warpTolerance * 2.f)
					{
						//-- run
						m_desiredSpeed = std::min (goalDistance2d, maximumSpeed * 4.f);
					}
					else
					{
						//-- warp to desired
						warpTransform (m_serverCellObject, m_serverTransform_p);

					}

					//-- if we have not moved at least 1/10 the distance we were supposed to move last frame, we're stuck and should warp
					if (m_desiredSpeed > 0.f && m_distanceToMoveLastFrame > 0.f)
					{
						const Vector & pos_w = creatureObject->getPosition_w ();
						const float distancedMovedLastFrameSquared = pos_w.magnitudeBetweenSquared (m_lastPosition_w);
						if (distancedMovedLastFrameSquared < sqr (m_distanceToMoveLastFrame * 0.1f))
						{
							if (m_stuckWarpTimer > cms_stuckWarpTimeThreshold)
								warpTransform (m_serverCellObject, m_serverTransform_p);
							else
								m_stuckWarpTimer += elapsedTime;
						}
						else
						{
							m_stuckWarpTimer = 0.0f;
						}
					}

					//-- warp if we've got nowhere to go but we're in different cells
					if (  m_desiredSpeed == 0.f
						&& m_serverCellObject
						&& m_serverCellObject->getCellProperty() != creatureObject->getParentCell()
						)
					{
						warpTransform (m_serverCellObject, m_serverTransform_p);
					}

					//-- accellerate or decelerate (only used for visuals)
					if(ms_disableAcceleration)
						m_currentSpeed = m_desiredSpeed;
					else
					{					
						if (m_desiredSpeed>m_currentSpeed)
						{
							const float accelerationScale = std::max (1.0f, std::min (2.0f, (goalDistance2d / warpTolerance) * 2.0f));

							//-- handle acceleration (vf - vi) / t
							const float acceleration = creatureObject->getMaximumAcceleration (m_currentSpeed) * accelerationScale;
							
							m_currentSpeed += acceleration * elapsedTime;

							if (m_currentSpeed > m_desiredSpeed)
							{
								m_currentSpeed = m_desiredSpeed;
							}
						}
						else if (m_desiredSpeed < m_currentSpeed)
						{
							// we're supposed to be stopped here now so slow down real fast
							// these numbers can be adjusted to move from running in place to moving while standing
							// larger thresholds mean less running in place but more sliding
							if(m_desiredSpeed == 0.0f && goalDistance2d == 0.0f)
							{
								m_currentSpeed *= cms_stopSpeedDecayFactor;
							}

							//-- handle deceleration (vf - vi) / t
							const float deceleration = -creatureObject->getMaximumAcceleration (m_currentSpeed);

							m_currentSpeed += deceleration * elapsedTime;

							if (m_currentSpeed < m_desiredSpeed)
							{
								m_currentSpeed = m_desiredSpeed;
							}
						}
							
					}
					
					const bool isPlayer = creatureObject->isPlayer();
					const bool isMountForAnyPlayer = (creatureObject->getRiderDriverCreature() != NULL);
					if(!isMountForAnyPlayer)
					{
						if(m_currentSpeed == 0.0f)
							m_serverSpeed = m_serverSpeed * (isPlayer ? ANIM_SPEED_SERVER_STOP_FACTOR_PLAYER : ANIM_SPEED_SERVER_STOP_FACTOR_NPC);
						else
							m_serverSpeed = (m_serverSpeed * (isPlayer ? ANIM_SPEED_SERVER_WEIGHT_FACTOR_PLAYER : ANIM_SPEED_SERVER_WEIGHT_FACTOR_NPC)) + (m_currentSpeed * (1.0f - (isPlayer ? ANIM_SPEED_SERVER_WEIGHT_FACTOR_PLAYER : ANIM_SPEED_SERVER_WEIGHT_FACTOR_NPC)));
						if(m_serverSpeed < 0.05f)
							m_serverSpeed = 0.0f;
					}
					else
					{
						m_serverSpeed = m_currentSpeed;
					}


					//-- tell the skeletal system how fast the animation should move
					{
						float animFactor = elapsedTime * cms_animationSpeedFilterFactor;
						animFactor = std::max(0.0f, std::min(1.0f, animFactor));
						m_currentAnimationSpeed += (m_serverSpeed - m_currentAnimationSpeed) * animFactor;

						m_currentAnimationSpeed = std::max( 0.f, std::min( m_currentAnimationSpeed, m_serverSpeed ) );

						SkeletalAppearance2* const appearance = creatureObject->getAppearance() ? creatureObject->getAppearance()->asSkeletalAppearance2() : 0;

						if (appearance)
						{
							float animationSpeed = (usesAnimationLocomotion) ? 0.0f : m_currentAnimationSpeed;

							/*
							DEBUG_REPORT_LOG(true, ("object [%s] goal[%5.3f] anim [%5.3f] server [%5.3f] current [%5.3f] usesAnim [%d]\n",
									creatureObject->getNetworkId().getValueString().c_str(), 
									goalDistance2d,
									animationSpeed, 
									m_serverSpeed, 
									m_currentSpeed,
									usesAnimationLocomotion
								));
							*/

							appearance->setDesiredVelocity (Vector::unitZ * animationSpeed);
						}
					}

					//-- fixup position
					Vector direction_o = creatureObject->rotate_w2o(serverPosition_w - clientPosition_w);
					const float distanceToMoveThisFrame = m_currentSpeed * elapsedTime;
					const float goalDistance = clientPosition_w.magnitudeBetween (serverPosition_w);
					if (distanceToMoveThisFrame <= goalDistance)
					{
						IGNORE_RETURN (direction_o.normalize ());
						direction_o *= m_currentSpeed * elapsedTime;
					}

					//-- record the current creature position to be used to potential warping
					m_lastPosition_w = creatureObject->getPosition_w ();
					m_distanceToMoveLastFrame = direction_o.magnitude ();

					//-- move the creature
					if (!usesAnimationLocomotion)
					{
						creatureObject->move_o (direction_o);
					}

					//-- fixup orientation
					if (!isFaceTracking()
						&& !creatureObject->getClientUsesAnimationLocomotion()
						&& !isMountForAndDrivenByClientPlayer)
					{
						const Vector facing_w = serverPosition_w + getLocalFrameK_w (m_serverCellObject, m_serverTransform_p) - clientPosition_w;
						const Vector facing_o = creatureObject->rotate_w2o (facing_w);

						const float desiredYaw = facing_o.theta();
						if (  desiredYaw >  ms_minimumCreatureRotation
							|| desiredYaw < -ms_minimumCreatureRotation
							)
						{
							const float  turnRate = creatureObject->getMaximumTurnRate(m_currentSpeed);
							const float  maximumYawThisFrame = convertDegreesToRadians (turnRate) * elapsedTime;
							const float yaw = clamp (-maximumYawThisFrame, desiredYaw, maximumYawThisFrame);
							creatureObject->yaw_o(yaw);
						}
					}	
					
					//-- fixup look at yaw
					if (!isFaceTracking()
						&& !creatureObject->getClientUsesAnimationLocomotion()
						&& !isMountForAndDrivenByClientPlayer
						&& !isMountForAnyPlayer
						&& creatureObject->getUseLookAtYaw())
					{
						const float currentLookAtYaw = creatureObject->getLookAtYaw();
						float desiredYaw = m_serverLookAtYaw - currentLookAtYaw;
						if((currentLookAtYaw > 2.5f) && (m_serverLookAtYaw < -2.5f)) // transition from PI to -PI
						{
							desiredYaw += 6.28f;	
						}
						else if ((currentLookAtYaw < -2.5f) && (m_serverLookAtYaw > 2.5f))
						{
							desiredYaw -= 6.28f;
						}
						
						if (  desiredYaw >  ms_minimumCreatureRotation
							|| desiredYaw < -ms_minimumCreatureRotation
							)
						{
							const float  turnRate = creatureObject->getMaximumTurnRate(m_currentSpeed);
							const float  maximumYawThisFrame = convertDegreesToRadians (turnRate) * elapsedTime;
							const float yaw = clamp (-maximumYawThisFrame, desiredYaw, maximumYawThisFrame);
							float finalYaw = currentLookAtYaw + yaw;
							if(finalYaw > 3.1415f)
								finalYaw -= 6.283f;
							else if(finalYaw < -3.1415f)
								finalYaw += 6.283f;
							if(fabs(finalYaw - m_serverLookAtYaw) < 0.05f)
								finalYaw = m_serverLookAtYaw;
							creatureObject->setLookAtYaw(finalYaw, true);
						}
					}
				}
			}
		}
	}

	//-- chain up
	float const baseAlterResult = CreatureController::realAlter (elapsedTime);
	UNREF (baseAlterResult);

#ifdef _DEBUG
	if (ms_reportTransforms)
	{
		_reportTransforms();
	}

	const Camera* const camera = Game::getCamera ();
	if (camera)
	{
		//-- add to client path
		if (!ms_disablePathUpdate && ms_renderClientPath)
		{
			while (m_clientPath->size () >= ms_clientPathSize)
				IGNORE_RETURN(m_clientPath->erase (m_clientPath->begin ()));

			if (m_clientPath->empty () || m_clientPath->back () != getOwner ()->getPosition_w ())
				m_clientPath->push_back (getOwner ()->getPosition_w ());
		}

		//-- render client transform
		if (ms_renderClientTransform)
		{
			camera->addDebugPrimitive (new FrameDebugPrimitive (FrameDebugPrimitive::S_none, getOwner ()->getTransform_o2w (), 0.5f));

			const Vector start_o = Vector::unitY * 0.5f;
			const Vector end_o   = start_o + Vector::unitZ * m_currentSpeed;
			camera->addDebugPrimitive (new Line3dDebugPrimitive (Line3dDebugPrimitive::S_none, getOwner ()->getTransform_o2w (), start_o, end_o, PackedArgb::solidWhite));
		}

		//-- render server transform
		if (ms_renderServerTransform && m_serverCellObject)
		{
			if (m_serverCellObject->getCellProperty () == CellProperty::getWorldCellProperty ())
			{
				camera->addDebugPrimitive (new FrameDebugPrimitive  (FrameDebugPrimitive::S_none,  m_serverTransform_p, 0.5f));
			}
			else
			{
				const Transform& cellTransform = m_serverCellObject->getTransform_o2w ();

				Transform t;
				t.multiply (cellTransform, m_serverTransform_p);
				camera->addDebugPrimitive (new FrameDebugPrimitive  (FrameDebugPrimitive::S_none, t, 0.5f));
			}
		}

		//-- render client path
		if (ms_renderClientPath && m_clientPath->size () > 1)
		{
			uint i;
			for (i = 0; i < m_clientPath->size () - 1; ++i)
			{
				camera->addDebugPrimitive (new Line3dDebugPrimitive (Line3dDebugPrimitive::S_none, Transform::identity, (*m_clientPath) [i], (*m_clientPath) [i + 1], PackedArgb::solidMagenta));
				camera->addDebugPrimitive (new Line3dDebugPrimitive (Line3dDebugPrimitive::S_none, Transform::identity, (*m_clientPath) [i], (*m_clientPath) [i] + Vector::unitY, PackedArgb::solidMagenta));
			}
		}

		//-- render server path
		if (ms_renderServerPath && m_serverPath->size () > 1)
		{
			uint i;
			for (i = 0; i < m_serverPath->size () - 1; ++i)
			{
				camera->addDebugPrimitive (new Line3dDebugPrimitive (Line3dDebugPrimitive::S_none, Transform::identity, (*m_serverPath) [i], (*m_serverPath) [i + 1], PackedArgb::solidCyan));
				camera->addDebugPrimitive (new Line3dDebugPrimitive (Line3dDebugPrimitive::S_none, Transform::identity, (*m_serverPath) [i], (*m_serverPath) [i] + Vector::unitY, PackedArgb::solidCyan));
			}
		}

		//-- render client to server line
		if (ms_renderLineFromClientToServerTransform && m_serverCellObject)
		{
			const Vector start_w = getOwner ()->getPosition_w ();

			Vector end_w = m_serverTransform_p.getPosition_p ();
			if (m_serverCellObject->getCellProperty () != CellProperty::getWorldCellProperty ())
			{
				const Transform& cellTransform = m_serverCellObject->getTransform_o2w ();

				Transform t;
				t.multiply (cellTransform, m_serverTransform_p);
				end_w = t.getPosition_p ();
			}

			camera->addDebugPrimitive (new Line3dDebugPrimitive (Line3dDebugPrimitive::S_none, Transform::identity, start_w, end_w, PackedArgb::solidWhite));
		}
	}
#endif
	return AlterResult::cms_alterNextFrame;
}

#ifdef _DEBUG
void RemoteCreatureController::_reportTransforms()
{
	CreatureObject* const creatureObject = safe_cast<CreatureObject*> (getOwner ());

	//-- client
	{
		//-- position_w
		const Vector& position_w = creatureObject->getPosition_w ();
		const int     frameK_w   = static_cast<int> (convertRadiansToDegrees (creatureObject->getObjectFrameK_w ().theta ()));
		DEBUG_REPORT_PRINT (true, ("cw %1.2f %1.2f %1.2f %i world\n", position_w.x, position_w.y, position_w.z, frameK_w));

		//-- position_p
		const Vector& position_p = creatureObject->getPosition_p ();
		const int     frameK_p   = static_cast<int> (convertRadiansToDegrees (creatureObject->getObjectFrameK_p ().theta ()));
		const CellProperty* const cellProperty  = creatureObject->getParentCell ();
		DEBUG_REPORT_PRINT (true, ("cc %1.2f %1.2f %1.2f %i %s [%s]\n", position_p.x, position_p.y, position_p.z, frameK_p, cellProperty == CellProperty::getWorldCellProperty () ? "world" : cellProperty->getCellName (), cellProperty == CellProperty::getWorldCellProperty () ? "NA" : cellProperty->getOwner ().getNetworkId ().getValueString ().c_str ()));

		//-- speed
		const float speed = getCurrentSpeed ();
		DEBUG_REPORT_PRINT (true, ("cs %1.2f\n", speed));
	}

	//-- server
	if (m_serverCellObject)
	{
		Transform t;
		if (m_serverCellObject->getCellProperty () == CellProperty::getWorldCellProperty ())
			t = m_serverTransform_p;
		else
		{
			const Transform& cellTransform = m_serverCellObject->getTransform_o2w ();
			t.multiply (cellTransform, m_serverTransform_p);
		}

		//-- position_w
		const Vector& position_w = t.getPosition_p ();
		const int     frameK_w   = static_cast<int> (convertRadiansToDegrees (t.getLocalFrameK_p ().theta ()));
		DEBUG_REPORT_PRINT (true, ("sw %1.2f %1.2f %1.2f %i world\n", position_w.x, position_w.y, position_w.z, frameK_w));

		//-- position_p
		const Vector& position_p = m_serverTransform_p.getPosition_p ();
		const int     frameK_p   = static_cast<int> (convertRadiansToDegrees (m_serverTransform_p.getLocalFrameK_p ().theta ()));
		DEBUG_REPORT_PRINT (true, ("sc %1.2f %1.2f %1.2f %i %s [%s]\n", position_p.x, position_p.y, position_p.z, frameK_p, m_serverCellObject->getCellProperty () == CellProperty::getWorldCellProperty () ? "world" : m_serverCellObject->getCellProperty ()->getCellName (), m_serverCellObject->getCellProperty () == CellProperty::getWorldCellProperty () ? "NA" : m_serverCellObject->getNetworkId ().getValueString ().c_str ()));

		//-- speed
		DEBUG_REPORT_PRINT (true, ("ss %1.2f\n", m_serverSpeed));
	}
}
#endif

//===================================================================
// PRIVATE RemoteCreatureController
//===================================================================

//-------------------------------------------------------------------
/**
 * Call this function when the creature is moved around in a cell other
 * than the world cell.
 */

void RemoteCreatureController::handleNetUpdateTransformWithParent (const MessageQueueDataTransformWithParent& message)
{
	DEBUG_REPORT_LOG (ms_logServerMovementData && (message.getSequenceNumber () <  m_serverSequenceNumber), ("handleNetUpdateTransformWithParent [%s]: disregarding older packet %i\n", getOwner ()->getNetworkId ().getValueString ().c_str (), m_serverSequenceNumber));
	DEBUG_REPORT_LOG (ms_logServerMovementData && (message.getSequenceNumber () == m_serverSequenceNumber), ("handleNetUpdateTransformWithParent [%s]: received same packet %i multiple times\n", getOwner ()->getNetworkId ().getValueString ().c_str (), m_serverSequenceNumber));

	if (message.getSequenceNumber () > m_serverSequenceNumber)
	{
		m_serverSequenceNumber = message.getSequenceNumber ();

#ifdef _DEBUG
		//-- verify that we did not receive the same transform
		if (message.getParent () == getOwner ()->getParentCell ()->getOwner ().getNetworkId () && message.getTransform () == getOwner ()->getTransform_o2p ())
			DEBUG_WARNING (true, ("handleNetUpdateTransformWithParent [%s]: received nutwp with identical cell and transform", getOwner ()->getNetworkId ().getValueString ().c_str ()));
#endif

#if 0
		//-- ignore if incapacitated or dead
		const CreatureObject* const creatureObject = safe_cast<const CreatureObject*> (getOwner ());
		if (creatureObject && (creatureObject->getVisualPosture () == Postures::Incapacitated || creatureObject->getVisualPosture () == Postures::Dead))
		{
			DEBUG_REPORT_LOG (ms_logServerMovementData, ("handleNetUpdateTransformWithParent [%s]: refused nutwp %i because object is incapacitated or dead\n", getOwner ()->getNetworkId ().getValueString ().c_str (), m_serverSequenceNumber));
			return;
		}
#endif

   		//-- ignore any malformed packets
		if (m_serverTransform_p == message.getTransform ())
		{
			DEBUG_REPORT_LOG (ms_logServerMovementData, ("handleNetUpdateTransformWithParent [%s]: refused nutwp %i with same transform\n", getOwner ()->getNetworkId ().getValueString ().c_str (), m_serverSequenceNumber));
			return;
		}

		//-- get parent
		const NetworkId cellNetworkId  = message.getParent ();
		const Object* const cellObject = NetworkIdManager::getObjectById (cellNetworkId);

		//-- handle a non-existent parent
		if (!cellObject)
		{
			DEBUG_WARNING (true, ("handleNetUpdateTransformWithParent [%s]: received nutwp with no parent [%s]", getOwner ()->getNetworkId ().getValueString ().c_str (), cellNetworkId.getValueString ().c_str ()));
			return;
		}

#ifdef _DEBUG
		DEBUG_REPORT_LOG (ms_logServerMovementData, ("handleNetUpdateTransformWithParent [%s]: accepted nutwp %i <%1.1f, %1.1f, %1.1f> <%1.1f> <%1.1f>\n",
			getOwner ()->getNetworkId ().getValueString ().c_str (),
			m_serverSequenceNumber,
			message.getTransform ().getPosition_p ().x,
			message.getTransform ().getPosition_p ().y,
			message.getTransform ().getPosition_p ().z,
			message.getTransform ().getLocalFrameK_p ().theta ()));
#endif

		//-- fixup potential broken transform from server
		Transform t = message.getTransform ();

		if (t.getLocalFrameJ_p () != Vector::unitY)
		{
#ifdef _DEBUG
			DEBUG_REPORT_LOG (ms_logBadServerTransforms, ("handleNetUpdateTransformWithParent [%s]: accepted nutwp has skewed transform <%1.1f, %1.1f, %1.1f>\n",
				getOwner ()->getNetworkId ().getValueString ().c_str (),
				t.getLocalFrameJ_p ().x,
				t.getLocalFrameJ_p ().y,
				t.getLocalFrameJ_p ().z));
#endif

			Vector k = t.getLocalFrameK_p ();
			Vector j = Vector::unitY;
			Vector i = j.cross (k);
			k = i.cross (j);
			t.setLocalFrameIJK_p (i, j, k);
		}

		CreatureObject* const creatureObject = safe_cast<CreatureObject*> (getOwner ());
		const bool isMountForAnyPlayer = (creatureObject->getRiderDriverCreature() != NULL);
		if(!isMountForAnyPlayer)
		{
			//CreatureObject* creatureObject = safe_cast<CreatureObject*> (getOwner ());
			//-- update creature yaw status but not its yaw so the blending code will get it later
			creatureObject->setLookAtYaw(creatureObject->getLookAtYaw(), message.getUseLookAtYaw());
			//-- update dead reckoning
			updateDeadReckoningModel (cellObject->getCellProperty (), t, message.getSpeed(), message.getLookAtYaw());  //-- todo need server speed
		}
		else
		{
			updateDeadReckoningModel (CellProperty::getWorldCellProperty (), t, 0.0f);
		}
	}
}

//-------------------------------------------------------------------
/**
 * Call this function when the creature is moved around in the world cell.
 */

void RemoteCreatureController::handleNetUpdateTransform (const MessageQueueDataTransform& message)
{
	DEBUG_REPORT_LOG (ms_logServerMovementData && (message.getSequenceNumber () < m_serverSequenceNumber), ("handleNetUpdateTransform [%s]: disregarding older packet %i\n", getOwner ()->getNetworkId ().getValueString ().c_str (), m_serverSequenceNumber));
	DEBUG_REPORT_LOG (ms_logServerMovementData && (message.getSequenceNumber () == m_serverSequenceNumber), ("handleNetUpdateTransform [%s]: received same packet %i multiple times\n", getOwner ()->getNetworkId ().getValueString ().c_str (), m_serverSequenceNumber));

	if (message.getSequenceNumber () > m_serverSequenceNumber)
	{
		m_serverSequenceNumber = message.getSequenceNumber ();

		CreatureObject* const creatureObject = safe_cast<CreatureObject*> (getOwner ());
		
#ifdef _DEBUG
		//-- verify that we did not receive the same transform
		if ( (message.getTransform () == getOwner ()->getTransform_o2p ()) && (message.getLookAtYaw() == creatureObject->getLookAtYaw()) )
			DEBUG_WARNING (true, ("handleNetUpdateTransform [%s]: received nut with identical transform", getOwner ()->getNetworkId ().getValueString ().c_str ()));
#endif

#if 0
		//-- ignore if incapacitated or dead
		if (creatureObject && (creatureObject->getVisualPosture () == Postures::Incapacitated || creatureObject->getVisualPosture () == Postures::Dead))
		{
			DEBUG_REPORT_LOG (ms_logServerMovementData, ("handleNetUpdateTransform [%s]: refused nut %i because object is incapacitated or dead\n", getOwner ()->getNetworkId ().getValueString ().c_str (), m_serverSequenceNumber));
			return;
		}
#endif

   		//-- ignore any malformed packets
		if ((m_serverTransform_p == message.getTransform ()) && (m_serverLookAtYaw == message.getLookAtYaw()))
		{
			DEBUG_REPORT_LOG (ms_logServerMovementData, ("handleNetUpdateTransform [%s]: refused nut %i with same transform\n", getOwner ()->getNetworkId ().getValueString ().c_str (), m_serverSequenceNumber));
			return;
		}

		//-- accept packet
#ifdef _DEBUG
		DEBUG_REPORT_LOG (ms_logServerMovementData, ("handleNetUpdateTransform [%s]: accepted nut %i <%1.1f, %1.1f, %1.1f> <%1.1f>\n",
			getOwner ()->getNetworkId ().getValueString ().c_str (),
			m_serverSequenceNumber,
			message.getTransform ().getPosition_p ().x,
			message.getTransform ().getPosition_p ().y,
			message.getTransform ().getPosition_p ().z,
			message.getTransform ().getLocalFrameK_p ().theta ()));
#endif

		//-- fixup potential broken transform from server
		Transform t = message.getTransform ();

		if (t.getLocalFrameJ_p () != Vector::unitY)
		{
#ifdef _DEBUG
			DEBUG_REPORT_LOG (ms_logBadServerTransforms, ("handleNetUpdateTransform [%s]: accepted nut has skewed transform <%1.1f, %1.1f, %1.1f>\n",
				getOwner ()->getNetworkId ().getValueString ().c_str (),
				t.getLocalFrameJ_p ().x,
				t.getLocalFrameJ_p ().y,
				t.getLocalFrameJ_p ().z));
#endif

			Vector k = t.getLocalFrameK_p ();
			Vector j = Vector::unitY;
			Vector i = j.cross (k);
			k = i.cross (j);
			t.setLocalFrameIJK_p (i, j, k);
		}

		const bool isMountForAnyPlayer = (creatureObject->getRiderDriverCreature() != NULL);
		if(!isMountForAnyPlayer)
		{
			//-- update creature yaw status but not its yaw so the blending code will get it later
			creatureObject->setLookAtYaw(creatureObject->getLookAtYaw(), message.getUseLookAtYaw());
			//-- update dead reckoning
			updateDeadReckoningModel (CellProperty::getWorldCellProperty (), t, message.getSpeed(), message.getLookAtYaw());  //-- todo need server speed
		}
		else
		{
			updateDeadReckoningModel (CellProperty::getWorldCellProperty (), t, 0.0f);
		}
	}
}

//-------------------------------------------------------------------

void RemoteCreatureController::pauseDeadReckoning (const float pauseTime)
{
	if (m_pauseDeadReckoning)
	{
		m_pauseDeadReckoningTimer.setExpireTime (std::max (pauseTime, m_pauseDeadReckoningTimer.getExpireTime ()));
	}
	else
	{
		m_pauseDeadReckoning = true;
		m_pauseDeadReckoningTimer.setExpireTime (pauseTime);
	}
}

// ----------------------------------------------------------------------

bool RemoteCreatureController::shouldApplyAnimationDrivenLocomotion () const
{
	bool result;

	CreatureObject const *const creatureObject = safe_cast<CreatureObject const*> (getOwner ());
	if (creatureObject && creatureObject->isMountForThisClientPlayer())
	{
		// Always use animation driven locomotion when we're the player's mount.
		result = true;
		DEBUG_REPORT_PRINT (ms_printDeadReckoningUsage && isOwnerAnimationDebuggerTarget (), ("dead reckoning usage: object id [%s]: animation-driven locomotion: applied (creature is the player's mount)\n", getOwner () ? getOwner ()->getNetworkId ().getValueString ().c_str () : "<NULL>"));
	}
	else
	{
		result = doesBodyPlayOnceHaveLocomotionPriority(false)
				|| creatureObject->getClientUsesAnimationLocomotion();

#ifdef _DEBUG
		if (result)
			DEBUG_REPORT_PRINT (ms_printDeadReckoningUsage && isOwnerAnimationDebuggerTarget (), ("dead reckoning usage: object id [%s]: animation-driven locomotion: applied (play once has loc priority)\n", getOwner () ? getOwner ()->getNetworkId ().getValueString ().c_str () : "<NULL>"));
		else
			DEBUG_REPORT_PRINT (ms_printDeadReckoningUsage && isOwnerAnimationDebuggerTarget (), ("dead reckoning usage: object id [%s]: animation-driven locomotion: disabled\n", getOwner () ? getOwner ()->getNetworkId ().getValueString ().c_str () : "<NULL>"));
#endif
	}

	return result;
}

// ----------------------------------------------------------------------

void RemoteCreatureController::updateDeadReckoningModel (Transform const & transform_w)
{
	updateDeadReckoningModel (CellProperty::getWorldCellProperty (), transform_w, m_serverSpeed);
	pauseDeadReckoning (0.5f);
}

//----------------------------------------------------------------------

float RemoteCreatureController::getDesiredSpeed () const
{
	return m_desiredSpeed;
}

//===================================================================
