// ======================================================================
//
// PlayerCreatureController.cpp
// Portions Copyright 1999 Bootprint Entertainment Inc.
// Portions Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/PlayerCreatureController.h"

#include "clientAudio/Audio.h"
#include "clientGame/ClientAsteroidManager.h"
#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientSecureTradeManager.h"
#include "clientGame/ClientStringIds.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/CockpitCamera.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/DraftSchematicManager.h"
#include "clientGame/FormManagerClient.h"
#include "clientGame/FreeChaseCamera.h"
#include "clientGame/Game.h"
#include "clientGame/GroundCombatActionManager.h"
#include "clientGame/GroundScene.h"
#include "clientGame/GroupManager.h"
#include "clientGame/HyperspaceIoWin.h"
#include "clientGame/PlayerMusicManager.h"
#include "clientGame/PlayerObject.h"
#include "clientGame/PlayerShipController.h"
#include "clientGame/QuestJournalManager.h"
#include "clientGame/ShipController.h"
#include "clientGame/ShipControllerTargetingProcessMessage.h"
#include "clientGame/ShipObject.h"
#include "clientGame/ShipObjectAttachments.h"
#include "clientGame/ShipStation.h"
#include "clientGraphics/DebugPrimitive.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/Light.h"
#include "clientGraphics/RenderWorld.h"
#include "clientObject/GameCamera.h"
#include "clientObject/MouseCursor.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientUserInterface/CuiActionManager.h"
#include "clientUserInterface/CuiActions.h"
#include "clientUserInterface/CuiConversationManager.h"
#include "clientUserInterface/CuiIoWin.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMinigameManager.h"
#include "clientUserInterface/CuiMissionManager.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiRadialMenuManager.h"
#include "clientUserInterface/CuiSocialsManager.h"
#include "clientUserInterface/CuiSpatialChatManager.h"
#include "clientUserInterface/CuiStaticLootItemManager.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/CollisionWorld.h"
#include "sharedCollision/Footprint.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/Production.h"
#include "sharedGame/SharedBuffBuilderManager.h"
#include "sharedGame/HyperspaceManager.h"
#include "sharedGame/MatchMakingCharacterResult.h"
#include "sharedGame/SharedImageDesignerManager.h"
#include "sharedGame/SlopeEffectProperty.h"
#include "sharedMath/Quaternion.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/BuffBuilderChangeMessage.h"
#include "sharedNetworkMessages/BuffBuilderStartMessage.h"
#include "sharedNetworkMessages/ImageDesignChangeMessage.h"
#include "sharedNetworkMessages/ImageDesignStartMessage.h"
#include "sharedNetworkMessages/IncubatorStartMessage.h"
#include "sharedNetworkMessages/MessageQueueCommandQueueEnqueue.h"
#include "sharedNetworkMessages/MessageQueueCommandQueueRemove.h"
#include "sharedNetworkMessages/MessageQueueCommandTimer.h"
#include "sharedNetworkMessages/MessageQueueCyberneticsOpen.h"
#include "sharedNetworkMessages/MessageQueueDataTransform.h"
#include "sharedNetworkMessages/MessageQueueDataTransformWithParent.h"
#include "sharedNetworkMessages/MessageQueueDraftSlotsQueryResponse.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedNetworkMessages/MessageQueueHarvesterResourceData.h"
#include "sharedNetworkMessages/MessageQueueMissionDetailsResponse.h"
#include "sharedNetworkMessages/MessageQueueMissionGenericResponse.h"
#include "sharedNetworkMessages/MessageQueueMissionListResponse.h"
#include "sharedNetworkMessages/MessageQueueNetworkId.h"
#include "sharedNetworkMessages/MessageQueueNpcConversationMessage.h"
#include "sharedNetworkMessages/MessageQueueObjectMenuRequest.h"
#include "sharedNetworkMessages/MessageQueueQuestTaskCounterMessage.h"
#include "sharedNetworkMessages/MessageQueueQuestTaskLocationMessage.h"
#include "sharedNetworkMessages/MessageQueueQuestTaskTimerMessage.h"
#include "sharedNetworkMessages/MessageQueueResourceWeights.h"
#include "sharedNetworkMessages/MessageQueueSecureTrade.h"
#include "sharedNetworkMessages/MessageQueueSocial.h"
#include "sharedNetworkMessages/MessageQueueSpatialChat.h"
#include "sharedNetworkMessages/MessageQueueStartNpcConversation.h"
#include "sharedNetworkMessages/MessageQueueStopNpcConversation.h"
#include "sharedNetworkMessages/MessageQueueStringList.h"
#include "sharedNetworkMessages/MessageQueueTeleportAck.h"
#include "sharedNetworkMessages/ObjectChannelMessages.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/Object.h"
#include "sharedObject/PortalProperty.h"
#include "sharedObject/VolumeContainer.h"
#include "sharedTerrain/TerrainObject.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/LocalMachineOptionManager.h"
#include "sharedUtility/StartingLocationData.h"
#include "sharedUtility/ValueDictionary.h"

#include "clientGame/GroupObject.h"
#include "clientGame/BuildingObject.h"

#include <set>

//-- error paths
#include "swgSharedUtility/Postures.def"

// ======================================================================
namespace PlayerCreatureControllerNamespace
{
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	namespace Transceivers
	{
		MessageDispatch::Transceiver<const MessageQueueHarvesterResourceData &, PlayerCreatureController> s_installationResourceData;

		MessageDispatch::Transceiver<const PlayerCreatureController::Messages::BiographyRetrieved::BiographyOwner &, PlayerCreatureController::Messages::BiographyRetrieved>
			biographyRetrieved;

		MessageDispatch::Transceiver<const PlayerCreatureController::Messages::BuffBuilderChangeReceived::Payload &, PlayerCreatureController::Messages::BuffBuilderChangeReceived>
			buffBuilderChangeReceived;

		MessageDispatch::Transceiver<const PlayerCreatureController::Messages::BuffBuilderCancelReceived::Payload &, PlayerCreatureController::Messages::BuffBuilderCancelReceived>
			buffBuilderCancelReceived;

		MessageDispatch::Transceiver<const PlayerCreatureController::Messages::CharacterMatchRetrieved::MatchResults &, PlayerCreatureController::Messages::CharacterMatchRetrieved>
			characterMatchRetrieved;

		MessageDispatch::Transceiver<const PlayerCreatureController::Messages::StartingLocationsReceived::Payload &, PlayerCreatureController::Messages::StartingLocationsReceived>
			startingLocationsReceived;

		MessageDispatch::Transceiver<const PlayerCreatureController::Messages::StartingLocationSelectionResult::Payload &, PlayerCreatureController::Messages::StartingLocationSelectionResult>
			startingLocationSelectionResult;

		MessageDispatch::Transceiver<const PlayerCreatureController::Messages::ImageDesignerChangeReceived::Payload &, PlayerCreatureController::Messages::ImageDesignerChangeReceived>
			imageDesignerChangeReceived;

		MessageDispatch::Transceiver<const PlayerCreatureController::Messages::ImageDesignerCancelReceived::Payload &, PlayerCreatureController::Messages::ImageDesignerCancelReceived>
			imageDesignerCancelReceived;

		MessageDispatch::Transceiver<const PlayerCreatureController::Messages::ShipParkingDataReceived::Payload &, PlayerCreatureController::Messages::ShipParkingDataReceived>
			shipParkingDataReceived;

		MessageDispatch::Transceiver<const PlayerCreatureController::Messages::GroupMemberInvitationToLaunchReceived::Payload &, PlayerCreatureController::Messages::GroupMemberInvitationToLaunchReceived>
			GroupMemberInvitationToLaunchReceived;

		MessageDispatch::Transceiver<const PlayerCreatureController::Messages::CommandTimerDataReceived::Payload &, PlayerCreatureController::Messages::CommandTimerDataReceived>
			commandTimerDataReceived;

		MessageDispatch::Transceiver<const PlayerCreatureController::Messages::AutoAimToggled::Payload &, PlayerCreatureController::Messages::AutoAimToggled>
			autoAimToggled;
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	const float ms_defaultReliableTransformUpdateTime = 4.0f;
	const float ms_defaultTransformUpdateTime         = 0.2f;
	const float cs_autoFollowMinDistance              = 3.0f;
	const float cs_autoFollowMinDistanceEnemy         = 0.5f;
	const float cs_autoFollowSlowSpeedDistance        = 2.0f * cs_autoFollowMinDistance;
	const float cs_autoFollowSlowSpeedDistanceEnemy   = 3.0f * cs_autoFollowMinDistanceEnemy;
	const float cs_autoFollowSlowSpeedFactor          = 0.3f;
//	const float cs_minAutoFollowFaceThreshold         = 5.0f * PI_OVER_180;
	bool        ms_debugReport                        = false;
	bool        ms_debugPrintClosestObject            = false;
	bool        ms_logTransformUpdates                = false;
	bool        ms_renderServerTransform              = false;
	bool        ms_logServerMovementData              = false;
	const VectorArgb ms_lightColor (1.f, 0.255f, 0.232f, 0.102f);
	const float ms_lightRangeNormal                   = 8.0f;
	const float ms_lightRangeMounted                  = 16.0f;
	const float ms_lightConstantAttenuationNormal       = 0.25f;
	const float ms_lightConstantAttenuationMounted      = 0.25f;
	const float ms_lightLinearAttenuationNormal       = 0.2f;
	const float ms_lightLinearAttenuationMounted      = 0.05f;

	const float ms_lookAtYawChangeThreshold           = 0.05f;

#ifdef _DEBUG
	bool        ms_printMovementSpeed;
#endif

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void computeAutoFollowInputEquivalent(float cameraYaw_w, const CreatureObject &sourceObject, const CreatureObject &initialTargetObject, Vector &desiredDirection, const Vector & autoFollowOffset);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	float computeTerrainMovementModifier (CreatureObject* const object)
	{
		if (!object)
		{
			DEBUG_FATAL(true, ("object is NULL."));
			return 0.0f; //lint !e527 // Unreachable // Reachable in release.
		}

		//-- verify that the object is not on a floor
		const CollisionProperty* const collision = object->getCollisionProperty ();
		const bool isOnSolidFloor = collision && collision->getFootprint () && collision->getFootprint ()->isOnSolidFloor ();

		float speedModifier = 1.0f;
		Vector objectFrameK_w(object->getObjectFrameK_w());

		if (object->isInWorldCell () && !isOnSolidFloor)
		{
			const Vector position = object->getPosition_w ();
			const TerrainObject* const terrainObject = TerrainObject::getInstance ();

			if (terrainObject)
			{
				if (terrainObject->isBelowWater (position))
				{
					speedModifier = object->getWaterModPercent();
				}
				else
				{
				float terrainHeight;
				Vector normal;
				if (terrainObject->getHeight (position, terrainHeight, normal))
				{
					const float slopeTolerance  = object->getSlopeModAngle();
					const float slopeThreshold  = -cos (convertDegreesToRadians (90.f - slopeTolerance));
					const float speedMultiplier = object->getSlopeModPercent();

					float slope  = normal.dot (object->getObjectFrameK_w ());
					if (slope < slopeThreshold)
					{
						const float ratio = (slope - slopeThreshold) / (-1.f - slopeThreshold);
							speedModifier = linearInterpolate (1.f, speedMultiplier, ratio);
						}
					}
				}
					}
				}

		// if the creature has a slope effect property, see if it has a greater
		// (more negative) effect on the creature than the terrain
		const Property * property = object->getProperty(SlopeEffectProperty::getClassPropertyId());
		if (property != NULL)
		{
			// note we use the creature's base speed modifier, not the one modified by skills
			// (although for ai they're probably the same)
			const float baseSpeedMultiplier = object->getBaseSlopeModPercent();
			const SlopeEffectProperty * slopeEffect = safe_cast<const SlopeEffectProperty *>(property);
			const std::vector<Vector> & normals = slopeEffect->getNormals();
			for (std::vector<Vector>::const_iterator i = normals.begin(); i != normals.end(); ++i)
			{
				float testSlope = (*i).dot(objectFrameK_w);
				if (testSlope < 0)
				{
					const float testModifier = linearInterpolate (1.f, baseSpeedMultiplier, -testSlope);
					if (testModifier < speedModifier)
						speedModifier = testModifier;
			}
		}
			object->removeProperty(SlopeEffectProperty::getClassPropertyId());
		}

		return speedModifier;
	}

	Timer gs_forceTransformTimer(300.0f); // force update as a keepalive
	bool  gs_forceTransformThisFrame = false;

	// this timer and bool are used to flag the client to report it's
	// location for a few more seconds after the character has begun
	// playing the sitting animation.  Since this sitting animtion will
	// actually modify the character's location, we want to guarantee
	// that what is finally reported to the server is actually the correct
	// location and not what is reported at the beginning of the sit.
	bool  gs_checkUpdateTransformAfterSittingTimer = false;
	Timer gs_updateTransformAfterSittingTimer(10.0f);

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	MouseCursor s_systemMouseCursor (0);

	bool  ms_runWhenMoving = true;

	//-- 2 seconds of movement breaks autofollow
	float s_playerMovementStopAutoFollowTime = 2.0f;

	// set to true when the player turns the light on, so we can reactivate it
	//  when every time we create the PCC (scene changes)
	bool s_playerLeftLightOn = false;

#if PRODUCTION == 0
	bool s_usePlayerServerSpeed = true;
#endif

	float ms_playerServerMovementSpeed = -1.0f;
}

using namespace PlayerCreatureControllerNamespace;

// ======================================================================

void PlayerCreatureControllerNamespace::computeAutoFollowInputEquivalent(float cameraYaw_w, const CreatureObject &sourceObject, const CreatureObject &initialTargetObject, Vector &desiredDirection, const Vector & autoFollowOffset)
{
	//-- Determine if initial follow target is an enemy.
	const bool isEnemy = sourceObject.getCombatTarget () == initialTargetObject.getNetworkId () || initialTargetObject.getCombatTarget () == sourceObject.getNetworkId ();

	//-- Setup the target object.  If the target is not an enemy and is mounted,
	//   use the target's mount for appearance radius and position; otherwise,
	//   use the initial target object.
	CreatureObject const *targetObject = &initialTargetObject;

	if (!isEnemy && initialTargetObject.getState(States::RidingMount))
	{
		CreatureObject const *const mount = initialTargetObject.getMountedCreature();
		if (mount)
			targetObject = mount;
	}
	NOT_NULL(targetObject);

	//-- Setup a transform at source object's world position relative to camera's facing.
	Transform t;

	t.yaw_l (cameraYaw_w);

	const float autoFollowMinDistance = isEnemy ? cs_autoFollowMinDistanceEnemy : cs_autoFollowMinDistance;

	//-- Compute target location in camera rotation space.
	const Vector & targetPosition_c = t.rotateTranslate_p2l   (targetObject->getPosition_w () - sourceObject.getPosition_w ());

	//-- If we're within a threshold number of meters from our target, don't try to move any more.
	const float separationDistance = targetPosition_c.approximateMagnitude () - targetObject->getAppearanceSphereRadius ();

	if (separationDistance <= autoFollowMinDistance)
	{
		// Don't try to move, we're close enough.
		desiredDirection = Vector::zero;
	}
	else
	{
		float offsetDistance = 0.0f;
		float speedFactor    = 1.0f;

		const float autoFollowSlowSpeedDistance = isEnemy ? cs_autoFollowSlowSpeedDistanceEnemy : cs_autoFollowSlowSpeedDistance;

		//-- Slow down movement request if we're within the slow-speed threshold.
		if (separationDistance <= autoFollowSlowSpeedDistance)
		{
			//-- walk toward a slightly closer offset when closing in
			offsetDistance    = autoFollowMinDistance * 0.8f;
			speedFactor       = cs_autoFollowSlowSpeedFactor;
		}
		else
		{
			//-- run toward a more distant point while still running fast
			offsetDistance    = autoFollowMinDistance + (autoFollowSlowSpeedDistance - autoFollowMinDistance) * 0.5f;
		}

		const Vector & offset_w         = targetObject->rotate_o2w (autoFollowOffset) * offsetDistance;
		const Vector & runToPosition_c  = t.rotateTranslate_p2l   ((targetObject->getPosition_w () - sourceObject.getPosition_w ()) - offset_w);

		// Target direction is along line from player to target.
		desiredDirection = runToPosition_c;
		IGNORE_RETURN (desiredDirection.normalize ());
		desiredDirection *= speedFactor;
	}
}

// ======================================================================
// STATIC PUBLIC PlayerCreatureController
// ======================================================================

void PlayerCreatureController::install ()
{
	InstallTimer const installTimer("PlayerCreatureController::install");
	ms_runWhenMoving = ConfigClientGame::getRunWhenMoving ();

	DebugFlags::registerFlag (ms_debugReport,             "ClientGame/PlayerCreatureController", "debugReport");
	DebugFlags::registerFlag (ms_debugPrintClosestObject, "ClientGame/PlayerCreatureController", "debugPrintClosestObject");
	DebugFlags::registerFlag (ms_logTransformUpdates,     "ClientGame/PlayerCreatureController", "logTransformUpdates");
	DebugFlags::registerFlag (ms_renderServerTransform,   "ClientGame/PlayerCreatureController", "renderServerTransform");
	DebugFlags::registerFlag (ms_logServerMovementData,   "ClientGame/PlayerCreatureController", "logServerMovementData");

#ifdef _DEBUG
	DebugFlags::registerFlag (ms_printMovementSpeed,      "ClientGame/PlayerCreatureController", "printMovementSpeed");
#endif

	const char * const section = "clientGame";
	LocalMachineOptionManager::registerOption (ms_runWhenMoving, section, "runWhenMoving");

	ExitChain::add(&remove, "PlayerCreatureController::remove");
}

// ======================================================================
// PUBLIC PlayerCreatureController
// ======================================================================

PlayerCreatureController::PlayerCreatureController (CreatureObject* const newOwner) :
	CreatureController (newOwner),
	m_serverSequenceNumber (0),
	m_desiredYaw_w (0.f),
	m_shouldFaceDesiredYaw (false),
	m_currentSpeed (0.f),
	m_sendTransformTimer (ms_defaultTransformUpdateTime),
	m_sendTransformThisFrame (false),
	m_previousTransform_p (),
	m_previousLookAtYaw(0.f),
	m_sendReliableTransformTimer (ms_defaultReliableTransformUpdateTime),
	m_sendReliableTransformThisFrame (false),
	m_previousReliableTransform_p (),
	m_previousReliableLookAtYaw(0.f),
	m_modeCallback (0),
	m_context (0),
	m_currentMode (NetworkId::cms_invalid),
	m_autoFollowTarget (new Watcher<CreatureObject>),
	m_autoFollowTargetOffset (),
	m_autoRun (false),
	m_mouseWalkMode (MWM_none),
	m_light (0),
	m_serverCellObject(NULL),
	m_serverTransform(Transform::identity),
	m_autoFollowTargetName (),
	m_playerMovementTimer  (0.0f),
	m_desiredSpeed(0.0f),
	m_buildingSharedTemplateNameCrc(Crc::crcNull),
	m_cellNameCrc(Crc::crcNull),
	m_allowMovement(true)
{
}

//----------------------------------------------------------------------

PlayerCreatureController::~PlayerCreatureController ()
{
	delete m_light;
	m_light = 0;
	delete m_autoFollowTarget;
	m_autoFollowTarget = 0;

	m_context = 0;
}

//----------------------------------------------------------------------

void PlayerCreatureController::setDesiredYaw_w (const float desiredYaw, bool shouldFaceDesiredYaw)
{
	float const epsilon = 0.001f;

	float const delta = m_desiredYaw_w - desiredYaw;
	if ((delta < -epsilon) || (delta > epsilon))
	{
		m_desiredYaw_w = desiredYaw;
		m_shouldFaceDesiredYaw = shouldFaceDesiredYaw;
	}
}

// ----------------------------------------------------------------------

float PlayerCreatureController::realAlter (const float elapsedTime)
{
	NP_PROFILER_AUTO_BLOCK_DEFINE ("PlayerCreatureController::realAlter");

	//-- Get owner.
	CreatureObject* const creatureObject = safe_cast<CreatureObject*> (getOwner ());
	if (!creatureObject)
	{
		DEBUG_WARNING (true, ("PlayerCreatureController::realAlter (): skipping rest of alter because owner is NULL, chaining up to parent class."));
		return CreatureController::realAlter (elapsedTime);
	}

	updateLookAtTargetSlot(*creatureObject);

	checkPlayerMusic(elapsedTime);

	Object *shipTargetReferenceObject = 0;

	//-- find the reference object for ship target processing, if any
	int const shipStation = creatureObject->getShipStation();
	if (shipStation != ShipStation::ShipStation_None)
	{
		ShipObject * const ship = Game::getPlayerContainingShip();
		if (ship)
		{
			if (ShipStation::isGunnerStation(shipStation))
			{
				int const weaponIndex = ShipStation::getWeaponIndexForGunnerStation(shipStation);
				Transform hardpointTransform_o2p;
				shipTargetReferenceObject = ship->getShipObjectAttachments().getFirstAttachedObjectForWeapon(weaponIndex, hardpointTransform_o2p);
				if (shipTargetReferenceObject)
					shipTargetReferenceObject = &ShipObject::getShipTurretMuzzle(*shipTargetReferenceObject);
			}
			else
				shipTargetReferenceObject = ship;
		}
	}

	// automatically turn the light on/off UNLESS the player has toggled it this session
	if (!m_light && s_playerLeftLightOn)
	{
		activatePlayerLight();
	}

	//-- read queue for piloting-independent messages (most movement messages will be ignored here if you're a pilot and the PlayerShipController will handle them)
	{
		int i;
		for (i = 0; i < getMessageQueue ()->getNumberOfMessages (); i++)
		{
			int                 message;
			float               value;
			MessageQueue::Data* data;

			getMessageQueue ()->getMessage (i, &message, &value, &data);

			if (shipTargetReferenceObject)
				shipControllerTargetingProcessMessage(message, shipTargetReferenceObject, creatureObject);

			switch (message)
			{
			case CM_toggleLight:
				{
					if (m_light)
					{
						m_light->removeFromWorld ();

						delete m_light;
						m_light = 0;

						s_playerLeftLightOn = false;
					}
					else
					{
						activatePlayerLight ();
						s_playerLeftLightOn = true;
					}

				}
				break;
			case CM_toggleAutoAim:
				{
					CuiPreferences::setAutoAimToggle(!CuiPreferences::getAutoAimToggle());
					Transceivers::autoAimToggled.emitMessage(*creatureObject);

					GroundCombatActionManager::ActionType actionType = GroundCombatActionManager::AT_cancelRepeatPrimaryAttack;
					GroundCombatActionManager::attemptAction(actionType);
				}
				break;
			case CM_primaryAttack:
				{
					GroundCombatActionManager::ActionType actionType = GroundCombatActionManager::AT_primaryAttack;

					//rsmith: intentionally temporary, hack, make sure we remove "lastActionStarted", and it's use, this after testing
					static GroundCombatActionManager::ActionType lastActionStarted = GroundCombatActionManager::AT_none;

					if(value == 1.0f) // 1.0f for initial down
					{
						GroundCombatActionManager::attemptAction(actionType);
					}
					else if(value == 0.0f) // 0.0f for held down
					{
					}
					else if(value == -1.0f) // -1.0f for release
					{
						GroundCombatActionManager::clearAction(actionType);
					}
				}
				break;
			case CM_secondaryAttack:
				{
					GroundCombatActionManager::ActionType actionType = GroundCombatActionManager::AT_secondaryAttack;
					
					if(value == 1.0f) // 1.0f for initial down
					{
						GroundCombatActionManager::attemptAction(actionType);
					}
					else if(value == 0.0f) // 0.0f for held down
					{
					}
					else if(value == -1.0f) // -1.0f for release
					{
						GroundCombatActionManager::clearAction(actionType);
					}
				}
				break;

			case CM_primaryAction:
				{
					GroundCombatActionManager::ActionType actionType = GroundCombatActionManager::AT_primaryAction;

					if(value == 1.0f) // 1.0f for initial down
					{
						GroundCombatActionManager::attemptAction(actionType);
					}
					//else if(value == 0.0f) // 0.0f for held down
					//{
						// placeholder - do nothing
					//}
					else if(value == -1.0f) // -1.0f for release
					{
						GroundCombatActionManager::clearAction(actionType);
					}
					break;
				}
			case CM_primaryActionAndAttack:
				{
					GroundCombatActionManager::ActionType actionType = GroundCombatActionManager::AT_primaryActionAndAttack;

					if(value == 1.0f) // 1.0f for initial down
					{
						GroundCombatActionManager::attemptAction(actionType);
					}
					else if(value == 0.0f) // 0.0f for held down
					{
					}
					else if(value == -1.0f) // -1.0f for release
					{
						GroundCombatActionManager::clearAction(actionType);
					}
					break;
				}

			case CM_toggleRepeatPrimaryAttack:
				{
					if (CuiPreferences::getAutoAimToggle())
					{
						GroundCombatActionManager::ActionType actionType = GroundCombatActionManager::AT_toggleRepeatPrimaryAttack;
						GroundCombatActionManager::attemptAction(actionType);
					}
					break;
				}

			case CM_setIntendedAndSummonRadialMenu:
				{
					if(value == 0.0f) // initial down
						IGNORE_RETURN(CuiActionManager::performAction (CuiActions::setIntendedAndSummonRadialMenu, Unicode::narrowToWide ("down")));
					else if(value == 1.0f) 
						IGNORE_RETURN(CuiActionManager::performAction (CuiActions::setIntendedAndSummonRadialMenu, Unicode::narrowToWide ("up")));
					break;
				}
			default:
				break;
			}
		}
	}

	//-- Get mount info.  Setup the movementCreatureObject to be the creature (player or mount)
	//   that will actually do the moving.
	bool const isRidingMount = creatureObject ? creatureObject->isRidingMount () : false;
	CreatureObject * const movementCreatureObject = isRidingMount ? NON_NULL(creatureObject)->getMountedCreature() : creatureObject;
	if (!movementCreatureObject)
	{
		DEBUG_WARNING (true, ("PlayerCreatureController::realAlter (): skipping rest of alter because owner is NULL, chaining up to parent class."));
		return CreatureController::realAlter (elapsedTime);
	}

	bool useVehicleControls = !(movementCreatureObject->getCanStrafe());

	//-- Handle the player's personal light.
	if (m_light && creatureObject)
	{
		const Vector offset (0.f, creatureObject->getAppearanceSphereRadius () * 2.0f + 0.1f, 1.f);
		const Vector & pos_w = creatureObject->rotateTranslate_o2w (offset);
		m_light->setParentCell (creatureObject->getParentCell ());
		m_light->setPosition_w (pos_w);

		if (isRidingMount)
		{
			m_light->setRange (ms_lightRangeMounted);
			m_light->setLinearAttenuation (ms_lightLinearAttenuationMounted);
			m_light->setConstantAttenuation(ms_lightConstantAttenuationMounted);
		}
		else
		{
			m_light->setRange (ms_lightRangeNormal);
			m_light->setLinearAttenuation (ms_lightLinearAttenuationNormal);
			m_light->setConstantAttenuation(ms_lightConstantAttenuationNormal);
		}
	}

	if (shouldProcessMovement())
	{
		//-- Clear auto-follow target if the target is dead or incapacitated.
		const CreatureObject * const autoFollowTarget = m_autoFollowTarget->getPointer ();
		if (autoFollowTarget)
		{
			//-- Break auto follow if the target is incapacitated or dead.
			Postures::Enumerator const targetPosture = static_cast<Postures::Enumerator> (autoFollowTarget->getVisualPosture ());
			if ((targetPosture == Postures::Incapacitated) || (targetPosture == Postures::Dead))
				setAutoFollowTarget (0);
			else
			{
				//-- Shut off auto run if following a target.  This prevents /follow from auto-run from disabling follow.
				m_autoRun = false;
			}
		}

		Vector desiredVelocity_c;
		bool   running = ms_runWhenMoving;
		float  turn = 0.f;

		if (m_autoRun)
			desiredVelocity_c.z = 1.0f;

		const bool isTurnStrafe  = CuiPreferences::isTurnStrafe ();
		const bool isFirstPerson = safe_cast<const GroundScene*> (Game::getConstScene ())->isFirstPerson ();

		bool shouldOverrideMovement = false; // for strafing

		//-- read queue
		{
			int i;
			for (i = 0; i < getMessageQueue ()->getNumberOfMessages (); i++)
			{
				int                 message;
				float               value;
				MessageQueue::Data* data;

				getMessageQueue ()->getMessage (i, &message, &value, &data);

				switch (message)
				{
				case CM_right:
					desiredVelocity_c.x += 1.f;
					break;

				case CM_left:
					desiredVelocity_c.x -= 1.f;
					break;

				case CM_walk:
					if ( desiredVelocity_c.z > -FLT_EPSILON )
					{
						desiredVelocity_c.z += 1.f;
					}
					break;

				case CM_mouseWalkStart:
					m_autoRun = false;

					if (CuiManager::getPointerInputActive ())
						m_mouseWalkMode = MWM_followMouse;
					else
						m_mouseWalkMode = MWM_ahead;
					break;

				case CM_mouseWalk:
					if (m_mouseWalkMode == MWM_followMouse)
					{
						if (!CuiManager::getPointerInputActive ())
							m_mouseWalkMode = MWM_ahead;

						const int x = s_systemMouseCursor.getX ();
						const int y = s_systemMouseCursor.getY ();

						//-- convert mouse coords into world velocity
						const float centerX = static_cast<float> (static_cast<uint32> (Graphics::getCurrentRenderTargetWidth ()) >> 1);
						const float centerY = static_cast<float> (static_cast<uint32> (Graphics::getCurrentRenderTargetHeight ()) >> 1);

						const Vector direction_c (static_cast<float> (x) - centerX, 0.f, centerY - static_cast<float> (y));
						desiredVelocity_c += direction_c;
					}
					else if (m_mouseWalkMode == MWM_ahead)
					{
						if (CuiManager::getPointerInputActive ())
							m_mouseWalkMode = MWM_followMouse;

						desiredVelocity_c.z += 1.f;
					}
					break;

				case CM_down:

					if ( desiredVelocity_c.z > FLT_EPSILON )
					{
						desiredVelocity_c.z = 0.f;
					}
					
					desiredVelocity_c.z -= 1.f;
					break;

				case CM_moveLateral:
					desiredVelocity_c.x += value;
					break;

				case CM_moveLongitudinal:
					desiredVelocity_c.x += value;
					break;

				case CM_toggleRunOn:
					running = !ms_runWhenMoving;
					break;

				case CM_turn:
					{
						if (isTurnStrafe)
						{
							if (value > 0.0f)
								getMessageQueue ()->appendMessage (static_cast<int>(CM_right), 0.0f);
							else
								getMessageQueue ()->appendMessage (static_cast<int>(CM_left),  0.0f);
						}
						else if ( !isFirstPerson && !m_shouldFaceDesiredYaw )
							turn += value;
					}
					break;

				case CM_cancelAutoRun:
					m_autoRun = false;
					break;

				case CM_autoRun:
					{
						// Ignore if we have an auto-follow target.
						if (m_autoFollowTarget->getPointer () == NULL)
							m_autoRun = !m_autoRun;
						else
							m_autoRun = false;
					}
					break;

				case CM_movePlayer:
					{
						typedef std::pair<float, float> Point;
						MessageQueueGenericValueType<Point>* const msg = safe_cast<MessageQueueGenericValueType<Point>*> (data);
						desiredVelocity_c.x += msg->getValue ().first;
						desiredVelocity_c.z += msg->getValue ().second;

						clearMessageAndData (i);
						delete msg;
					}
					break;

				default:
					break;
				}
			}
		}

		//if we have no input but the vehicle must be moving, just push it along the camera direction by default
		if (useVehicleControls && (desiredVelocity_c.magnitudeSquared() < FLT_EPSILON) && (movementCreatureObject->getMinimumSpeed() > FLT_EPSILON))
			desiredVelocity_c.z = 1.0f;

		//-- Clear auto follow target if we try to move.
		if (desiredVelocity_c != Vector::zero)
		{
			m_playerMovementTimer += elapsedTime;

			if (m_playerMovementTimer > s_playerMovementStopAutoFollowTime && m_autoFollowTarget->getPointer ())
				setAutoFollowTarget (0);
		}
		else
			m_playerMovementTimer = 0.0f;

		//-- Clear auto-run if in a non-auto-run posture.
		if (m_autoRun)
		{
			Postures::Enumerator const posture = movementCreatureObject->getVisualPosture();

			switch (posture)
			{
				case Postures::Crouched:
				case Postures::Blocking:
				case Postures::LyingDown:
				case Postures::Sitting:
				case Postures::SkillAnimating:
				case Postures::KnockedDown:
				case Postures::Incapacitated:
				case Postures::Dead:
					// Turn off auto-run, it should not be on in any of these postures.
					m_autoRun = false;

					// Kill any current movement so it doesn't kick us immediately out of the non-standing posture.
					// This handles the case where auto-running and immediately try to change posture.  Any movement
					// due to deceleration will trigger the user leaving the very non-standing posture that they
					// just went into.
					m_currentSpeed    = 0.0f;
					desiredVelocity_c = Vector::zero;
					break;

				default:
					// do nothing.
					break;
			}
		}

		const float           turnRate            = convertDegreesToRadians (movementCreatureObject->getMaximumTurnRate (m_currentSpeed));
		const float           maximumYawThisFrame = turnRate * elapsedTime;

		static float m_lastTurnRate =  0.0f;

		turn *= turnRate;

		turn = std::max (-maximumYawThisFrame, std::min (maximumYawThisFrame, turn));
		if (turn > 0 && m_lastTurnRate >= 0.0f && turn > m_lastTurnRate)
			turn = linearInterpolate (m_lastTurnRate, turn, elapsedTime);
		else if (turn < 0 && m_lastTurnRate <= 0.0f && turn < m_lastTurnRate)
			turn = linearInterpolate (m_lastTurnRate, turn, elapsedTime);

		m_lastTurnRate = turn;

		if (!m_autoFollowTargetName.empty () && m_autoFollowTarget->getPointer () == NULL)
			setAutoFollowTarget (0);

		//-- If player is auto-following, but their target has gone invisible, stop auto following and send them a message
		if(!m_autoFollowTargetName.empty () && (m_autoFollowTarget->getPointer() != NULL) &&
			(m_autoFollowTarget->getPointer()->getCoverVisibility() == false) && !PlayerObject::isAdmin() && !m_autoFollowTarget->getPointer()->isPassiveRevealPlayerCharacter(Game::getPlayerNetworkId()))
		{
			setAutoFollowTarget (0);
		}

		//-- If player is auto-following and has not specified input,
		//   generate the user input that would be required to do the auto-follow.
		if (NULL != creatureObject && (m_autoFollowTarget->getPointer() != NULL) && (desiredVelocity_c == Vector::zero) && (turn == 0.0f))
			computeAutoFollowInputEquivalent(m_desiredYaw_w, *creatureObject, **m_autoFollowTarget, desiredVelocity_c, m_autoFollowTargetOffset);
		else
		{
			// Only normalize direction vector when we're not auto following.  Auto follow needs the ability to
			// adjust speed via direction vector magnitude.
			IGNORE_RETURN(desiredVelocity_c.normalize ());
			
			/* rsmith: not sure what this block was for exactly - but it busted the left/right/back keys in chase mode
			if (m_shouldFaceDesiredYaw && CuiPreferences::getUseModelessInterface() == false )
			{
				const float theta = normalized ? desiredVelocity_c.theta () : 0.0f;
				m_desiredYaw_w += theta;
			}
			*/

		}

	#if PRODUCTION == 0
		desiredVelocity_c *= ConfigClientGame::getHackMovementSpeed ();
	#endif

		//-- Track initial desired velocity, used by code that issues a stand request
		//   when a sitting player tries to move.
		const Vector initialDesiredVelocity = desiredVelocity_c;

		//-- Shut off face tracking if we attempt to move or turn.
		if (isFaceTracking() && (turn != 0.f) || (desiredVelocity_c.x != 0.f) || (desiredVelocity_c.z != 0.f))
			stopFaceTracking ();

		//-- compute desired animation velocity
		{
			//-- CreatureObject already takes movement scale and movement percent into effect.
			const float runSpeed  = movementCreatureObject->getRunSpeed ();
			const float walkSpeed = movementCreatureObject->getWalkSpeed ();

			//-- Get movement modifier (this is now just the modifier applicable for the underlying terrain).
			const float terrainMovementModifier = isSwimming() ? 1.0f : computeTerrainMovementModifier(movementCreatureObject);

			//-- are we accelerating or decelerating? for simplicity, only support constant acceleration
			const float speed  = running ? (runSpeed > 0 ? runSpeed : walkSpeed) : walkSpeed;
			
			if(useVehicleControls)
			{
				Transform t;
				t.yaw_l (m_desiredYaw_w);
				Vector const & desiredVelocity_w = t.rotate_l2p (desiredVelocity_c);
				Vector const forward = movementCreatureObject->getTransform_o2w().getLocalFrameK_p();

				m_desiredSpeed = speed * desiredVelocity_w.dot(forward) / forward.magnitude();

				//clamp within min and max speeds
				m_desiredSpeed = std::min (movementCreatureObject->getMaximumSpeed(), std::max (movementCreatureObject->getMinimumSpeed(), m_desiredSpeed));
			}
			else
			{
				m_desiredSpeed = speed * desiredVelocity_c.magnitude ();
			}

			float desiredSpeed = m_desiredSpeed * terrainMovementModifier;

			//-- accellerate or decelerate
			if (desiredSpeed > m_currentSpeed)
			{
				//-- handle acceleration (vf - vi) / t
				const float acceleration = movementCreatureObject->getMaximumAcceleration (m_currentSpeed);

				m_currentSpeed += acceleration * elapsedTime;
				if (m_currentSpeed > desiredSpeed)
					m_currentSpeed = desiredSpeed;
				
				// Not allowed to move faster than our server limit.
#if PRODUCTION == 0
				if(s_usePlayerServerSpeed && ms_playerServerMovementSpeed > 0.0f)
#else
				if(ms_playerServerMovementSpeed > 0.0f)
#endif
					m_currentSpeed = std::min(m_currentSpeed, ms_playerServerMovementSpeed);
			}
			else
				if (desiredSpeed < m_currentSpeed)
				{
					//-- handle deceleration (vf - vi) / t
					const float deceleration = -movementCreatureObject->getMaximumAcceleration (m_currentSpeed);

					m_currentSpeed += deceleration * elapsedTime;

					if ( m_currentSpeed < desiredSpeed )
					{
						m_currentSpeed = desiredSpeed;
					}
				}

			//-- handle auto posture change when player tries to move in posture crouched, knocked down or block.
			const Postures::Enumerator currentPosture = movementCreatureObject->getVisualPosture ();

			bool standPostureRequired;

			switch (currentPosture)
			{
				case Postures::Crouched:
				case Postures::KnockedDown:
				case Postures::Blocking:
				case Postures::Sitting:
				case Postures::SkillAnimating:
					standPostureRequired = true;
					break;

				default:
					standPostureRequired = false;
			}

			if (standPostureRequired)
			{
				// A stand posture is required to move from the current posture.  If the
				// player is trying to move, make a request to the server to stand.

				//-- It is not sufficient to test desiredSpeed for movement from KnockedDown or Blocking --- those postures don't
				//   allow movement, so desiredSpeed will always be zero (due to max acceleration and other values above).  Use
				//   the initial desired velocity instead.
				if ((initialDesiredVelocity.x != 0.0f) || (initialDesiredVelocity.z != 0.0f))
					requestStand ();
			}

			//-- handle swimming
			if (isSwimming() && (movementCreatureObject->getVisualPosture () == Postures::Prone))
				requestStand ();
		}

		//-- handle swimming
		if (isSwimming() && (movementCreatureObject->getVisualPosture () == Postures::Prone))
			requestStand ();

		bool const isMoving = m_currentSpeed > 0.0f;		

		if (m_shouldFaceDesiredYaw || isFirstPerson || isMoving )
		{
			if (m_shouldFaceDesiredYaw || (isFirstPerson && !isMoving))
			{
				//-- dont turn mount to face direction when riding a mount
				if ( !isRidingMount  )
				{
					if ( !isMoving )
					{
						desiredVelocity_c = Vector::unitZ;
					}
				}
			}

			Vector movementOverrideVect(0.0f,0.0f,0.0f);
			
			if ( desiredVelocity_c.magnitude () > Vector::NORMALIZE_THRESHOLD)
			{
				Transform t;

 				t.yaw_l (m_desiredYaw_w);

				Vector const & desiredVelocity_w = t.rotate_l2p (desiredVelocity_c);
				Vector desiredVelocity_o;

				ShipObject * const ship = Game::getPlayerContainingShip();
				if (ship)
				{
					movementOverrideVect = movementCreatureObject->rotate_p2w(desiredVelocity_w);
					desiredVelocity_o = movementCreatureObject->rotate_p2o (desiredVelocity_w);
				}
				else
				{
					movementOverrideVect = desiredVelocity_w;
					desiredVelocity_o = movementCreatureObject->rotate_w2o (desiredVelocity_w);
				}

				const float  yaw = clamp (-maximumYawThisFrame, desiredVelocity_o.theta (), maximumYawThisFrame);
				shouldOverrideMovement = (yaw > FLT_EPSILON || yaw < -FLT_EPSILON);
				movementCreatureObject->yaw_o (yaw);
			}

			if(shouldOverrideMovement)
			{
				if(useVehicleControls)
				{
					Vector const & worldPos = movementCreatureObject->getPosition_w();
					Transform o2w = movementCreatureObject->getTransform_o2w();
					Vector worldDelta = o2w.rotate_l2p(Vector::unitZ) * m_currentSpeed * elapsedTime;

					movementCreatureObject->setPosition_w(worldPos + worldDelta);
				}	
				else
				{
					movementOverrideVect.normalize();
					movementCreatureObject->setPosition_w(
						movementCreatureObject->getPosition_w() 
						+ movementOverrideVect * m_currentSpeed * elapsedTime
						);	
				}
			}
		}

		//-- turn
		if ( ( turn != 0.f ) && ( !isFirstPerson || isMoving ) )
		{
			movementCreatureObject->yaw_o (turn);
		}

		// disable movement if the inventory is overloaded
		if (!PlayerObject::isAdmin())
		{
			ClientObject *const inventoryObject = (NULL != creatureObject) ? creatureObject->getInventoryObject() : NULL;
			if (inventoryObject)
			{
				VolumeContainer *const inventoryContainer = ContainerInterface::getVolumeContainer(*inventoryObject);
				if ((inventoryContainer) && (inventoryContainer->getCurrentVolume() >= (inventoryContainer->getTotalVolume() + ConfigClientGame::getDisableMovementInventoryOverload())))
				{
					m_currentSpeed = 0.f;
				}
			}
		}

		DEBUG_REPORT_PRINT(ms_printMovementSpeed, ("PCC: final movement speed given to animation system: [%g]\n", m_currentSpeed));

		//-- tell the skeletal system about our desired motion
		{
			SkeletalAppearance2* const appearance = safe_cast<SkeletalAppearance2*> (movementCreatureObject->getAppearance ());
			if (appearance)
				appearance->setDesiredVelocity (Vector::unitZ * m_currentSpeed);

			if(appearance && shouldOverrideMovement)
			{
				Quaternion rot;
				Vector trans;
				appearance->getObjectLocomotion(rot,trans,elapsedTime);
				movementCreatureObject->move_o(-trans);
			}
		}
	}

	//-- chain up to actually move the player
	float const baseAlterResult = CreatureController::realAlter (elapsedTime);
	UNREF (baseAlterResult);

	if (shouldSendUpdatedTransform())
	{
		//-- cheese a glass wall
		{
			if (movementCreatureObject->isInWorldCell ())
			{
				const float mapWidthInMeters = TerrainObject::getConstInstance ()->getMapWidthInMeters () * 0.5f;
				const float boundary         = 512.f;
				const float minimumBorder    = -mapWidthInMeters + boundary;
				const float maximumBorder    =  mapWidthInMeters - boundary;

				Vector position = movementCreatureObject->getPosition_w ();

				bool clipped = false;

				if (position.x > maximumBorder)
				{
					position.x = maximumBorder;
					clipped = true;
				}

				if (position.x < minimumBorder)
				{
					position.x = minimumBorder;
					clipped = true;
				}

				if (position.z > maximumBorder)
				{
					position.z = maximumBorder;
					clipped = true;
				}

				if (position.z < minimumBorder)
				{
					position.z = minimumBorder;
					clipped = true;
				}

				if (clipped)
					movementCreatureObject->setPosition_w (position);
			}
		}

		//-- handle the mode callback
		if (m_modeCallback)
		{
			const NetworkId& networkId = (NULL != creatureObject) ? creatureObject->getLookAtTarget (): NetworkId::cms_invalid;
			if (m_currentMode != networkId && networkId != NetworkId::cms_invalid)
			{
				m_modeCallback (m_context);

				m_modeCallback = 0;
				m_context = 0;
			}
		}

		//-- update the send transform timer
		if (m_sendTransformTimer.updateZero (elapsedTime))
		{
			m_sendTransformThisFrame = true;
		}

		if (gs_forceTransformTimer.updateZero (elapsedTime))
		{
			m_sendTransformThisFrame = true;
			gs_forceTransformThisFrame = true;
			m_sendReliableTransformThisFrame = true;
		}

		if (m_sendReliableTransformTimer.updateZero (elapsedTime))
		{
			m_sendReliableTransformThisFrame = true;
		}

		// also see the comments in the declaration.
		if (gs_checkUpdateTransformAfterSittingTimer)
		{
			// force the update even if the position has not changed
			if ((m_sendTransformThisFrame) || (m_sendReliableTransformThisFrame))
			{
				gs_forceTransformThisFrame = true;
			}

			// deactivate the timer when expired
			if (gs_updateTransformAfterSittingTimer.updateZero(elapsedTime))
			{
				gs_checkUpdateTransformAfterSittingTimer = false;
				m_sendTransformThisFrame = true;
				gs_forceTransformThisFrame = true;
				m_sendReliableTransformThisFrame = true;
			}
		}
	}

	updateBuildingAndCellInformation(creatureObject);

#ifdef _DEBUG
	const Camera* const camera = Game::getCamera ();
	if (camera)
	{
		//-- render server transform
		if (ms_renderServerTransform)
		{
			CellProperty const * playerCell = (NULL != creatureObject) ? creatureObject->getParentCell(): NULL;
			CellProperty const * serverCell = m_serverCellObject ? m_serverCellObject->getCellProperty() : NULL;

			if(serverCell == NULL) serverCell = CellProperty::getWorldCellProperty();

			Transform t = m_serverTransform;

			if(serverCell != CellProperty::getWorldCellProperty())
			{
				t.multiply(m_serverCellObject->getTransform_o2w(), m_serverTransform);
			}

			FrameDebugPrimitive * frame = new FrameDebugPrimitive(UtilityDebugPrimitive::S_none, t, 0.5f);
			camera->addDebugPrimitive(frame);

			if(playerCell != serverCell)
			{
				OctahedronDebugPrimitive * oct = new OctahedronDebugPrimitive(UtilityDebugPrimitive::S_none,t,Vector::zero,0.3f);

				oct->setColor(VectorArgb::solidRed);

				camera->addDebugPrimitive(oct);
			}
		}
	}
#endif

	return AlterResult::cms_alterNextFrame;
}

//-------------------------------------------------------------------

void PlayerCreatureController::conclude ()
{
#ifdef _DEBUG
	//-- debug print
	if (ms_debugReport)
	{
		const CreatureObject* const creatureObject = safe_cast<CreatureObject*> (getOwner ());
		const SharedCreatureObjectTemplate* const creatureObjectTemplate = safe_cast<const SharedCreatureObjectTemplate*> (creatureObject->getObjectTemplate ());

		DEBUG_REPORT_PRINT (true, ("                  currentSpeed = %1.2f\n", m_currentSpeed));
		DEBUG_REPORT_PRINT (true, ("                 movementScale = %1.2f\n", creatureObject->getMovementScale ()));
		DEBUG_REPORT_PRINT (true, ("               movementPercent = %1.2f\n", creatureObject->getMovementPercent ()));
		DEBUG_REPORT_PRINT (true, ("                      runSpeed = %1.2f\n", creatureObject->getRunSpeed ()));
		DEBUG_REPORT_PRINT (true, ("                     walkSpeed = %1.2f\n", creatureObject->getWalkSpeed ()));
		DEBUG_REPORT_PRINT (true, ("               waterModPercent = %1.2f\n", creatureObject->getWaterModPercent ()));
		DEBUG_REPORT_PRINT (true, (" objectTemplateMaximumRunSpeed = %1.2f\n", creatureObjectTemplate->getSpeed (SharedCreatureObjectTemplate::MT_run)));
		DEBUG_REPORT_PRINT (true, ("objectTemplateMaximumWalkSpeed = %1.2f\n", creatureObjectTemplate->getSpeed (SharedCreatureObjectTemplate::MT_walk)));
		DEBUG_REPORT_PRINT (true, (" objectTemplateMaximumRunSpeed = %1.2f\n", creatureObjectTemplate->getSpeed (SharedCreatureObjectTemplate::MT_run)));
		DEBUG_REPORT_PRINT (true, ("        scaledMaximumWalkSpeed = %1.2f\n", creatureObject->getMaximumWalkSpeed ()));
		DEBUG_REPORT_PRINT (true, ("         scaledMaximumRunSpeed = %1.2f\n", creatureObject->getMaximumRunSpeed ()));
		DEBUG_REPORT_PRINT (true, ("                 lightAttached = %s\n", m_light ? "yes" : "no"));
	}

	//-- debug print closest object
	if (ms_debugPrintClosestObject)
	{
		const Object* const object = ClientWorld::findClosestConstObjectTo (getOwner (), static_cast<int>(WOL_Tangible)); //lint !e1060 // protected member World::* not accessible // wrong, look at decl for ClientWorld.

		if (object)
		{
			const Vector position = object->getPosition_w ();

			DEBUG_REPORT_PRINT (true, ("closest name = %s\n", object->getDebugName () ? object->getDebugName () : "none"));
			DEBUG_REPORT_PRINT (true, ("closest object template name = %s\n", object->getObjectTemplate () ? object->getObjectTemplate ()->getName () : "none"));
			DEBUG_REPORT_PRINT (true, ("closest position <%1.1f, %1.1f, %1.1f>\n", position.x, position.y, position.z));
		}
		else
			DEBUG_REPORT_PRINT (true, ("closest object not found\n"));
	}
#endif

	//-- send movement data to server
	CreatureObject * owner = safe_cast<CreatureObject*>(getOwner());
	NOT_NULL(owner);

	if (shouldSendUpdatedTransform())
	{
		//-- only send for objects that the server knows about.
		if (owner->getNetworkId ().getValue () == 0)
			return;

		bool const sendReliable   = m_sendReliableTransformThisFrame || gs_forceTransformThisFrame;
		bool const sendUnreliable = !sendReliable && m_sendTransformThisFrame;

		if (sendReliable || sendUnreliable)
		{
			Transform const *transform_p = NULL;
			NetworkId        parentId;

			//-- Handle a rider on a mount: the rider sends the position update of the mount, not the rider.
			bool const isRidingMount = owner->isRidingMount();
			if (isRidingMount)
			{
				// Use the rider's mount's position and parent.

				// Get the mount.
				CreatureObject const *mount = owner->getMountedCreature();
				NOT_NULL(mount);

				// Use the mount's position.
				transform_p = &(mount->getTransform_o2p());

				// Use the mount's attachment network id.
				Object const *const attachedToObject = mount->getAttachedTo();
				if (attachedToObject)
					parentId = attachedToObject->getNetworkId();
			}
			else
			{
				// Use the creature object's position and parent.
				transform_p = &(owner->getTransform_o2p());

				Object const *const attachedToObject = owner->getAttachedTo();
				if (attachedToObject)
					parentId = attachedToObject->getNetworkId();
			}
			NOT_NULL(transform_p);

			//-- send reliable transform to server
			if (sendReliable)
			{
				if (!transform_p->approximates (m_previousReliableTransform_p) || hasLookAtYawChanged(true) || gs_forceTransformThisFrame)
				{
					m_sendReliableTransformThisFrame = false;
					gs_forceTransformThisFrame = false;

					m_previousReliableTransform_p = *transform_p;
					m_previousTransform_p = *transform_p;
					m_previousReliableLookAtYaw = owner->getLookAtYaw();
					m_previousLookAtYaw = owner->getLookAtYaw();
					sendTransformUsingParent (*transform_p, parentId, true);
				}
			}
			else // if (m_sendUnreliable) // we know this must be true here by test for either above.
			{
				if (!transform_p->approximates (m_previousTransform_p) || hasLookAtYawChanged(false) || gs_forceTransformThisFrame)
				{
					m_sendTransformThisFrame = false;
					gs_forceTransformThisFrame  = false;

					m_previousTransform_p = *transform_p;
					m_previousLookAtYaw = owner->getLookAtYaw();
					sendTransformUsingParent (*transform_p, parentId);
				}
			}
		}
	}

	//-- perform rest of conclude operations
	CreatureController::conclude ();
}

//----------------------------------------------------------------------

float PlayerCreatureController::getCurrentSpeed () const
{
	return m_currentSpeed;
}

//----------------------------------------------------------------------

void PlayerCreatureController::setCurrentSpeed (float const currentSpeed)
{
#if PRODUCTION == 0
	if(s_usePlayerServerSpeed && ms_playerServerMovementSpeed > 0.0f)
#else
	if(ms_playerServerMovementSpeed > 0.0f)
#endif
		m_currentSpeed = std::min(m_currentSpeed, ms_playerServerMovementSpeed);
	else
		m_currentSpeed = currentSpeed;
}

//----------------------------------------------------------------------

void PlayerCreatureController::warpClient (const Transform& transform_p)
{
	m_previousTransform_p = transform_p;
	
	CreatureObject const *const creatureObject = safe_cast<CreatureObject *>(getOwner());
	float lookAtYaw = creatureObject ? creatureObject->getLookAtYaw() : 0.f;
	bool useLookAtYaw = creatureObject ? creatureObject->getUseLookAtYaw() : false;

	m_previousLookAtYaw = lookAtYaw;
	MessageQueue::Data* data = new MessageQueueDataTransform (0, getNextSequenceNumber (), transform_p, 0.f, lookAtYaw, useLookAtYaw);
	appendMessage(
		static_cast<int> (CM_netUpdateTransform),
		0.f,
		data,
		GameControllerMessageFlags::SEND |
			GameControllerMessageFlags::RELIABLE |
			GameControllerMessageFlags::DEST_AUTH_SERVER |
			GameControllerMessageFlags::DEST_AUTH_CLIENT);
}

//-------------------------------------------------------------------

bool PlayerCreatureController::shouldApplyAnimationDrivenLocomotion () const
{
	Object const *const         baseObject     = getOwner();
	ClientObject const *const   clientObject   = baseObject ? baseObject->asClientObject() : NULL;
	CreatureObject const *const creatureObject = clientObject ? clientObject->asCreatureObject() : NULL;

	//-- Don't apply when the rider is mounted.
	bool const apply = creatureObject && !creatureObject->isRidingMount();

	return apply;
}

//-------------------------------------------------------------------

void PlayerCreatureController::setModeCallback (ModeCallback modeCallback, void* const context)
{
	m_modeCallback = modeCallback;
	m_context = context;
	m_currentMode = safe_cast<const CreatureObject*> (getOwner ())->getLookAtTarget ();
}

// ----------------------------------------------------------------------

MessageQueue *PlayerCreatureController::getMessageQueue()
{
	return m_messageQueue;
}

// ----------------------------------------------------------------------

const MessageQueue *PlayerCreatureController::getMessageQueue() const
{
	return m_messageQueue;
}

// ======================================================================
// STATIC PRIVATE PlayerCreatureController
// ======================================================================

void PlayerCreatureController::remove ()
{
	DebugFlags::unregisterFlag (ms_debugReport);
	DebugFlags::unregisterFlag (ms_debugPrintClosestObject);
	DebugFlags::unregisterFlag (ms_logTransformUpdates);
	DebugFlags::unregisterFlag (ms_renderServerTransform);
	DebugFlags::unregisterFlag (ms_logServerMovementData);

#ifdef _DEBUG
	DebugFlags::unregisterFlag (ms_printMovementSpeed);
#endif

}

// ======================================================================
// PRIVATE PlayerCreatureController
// ======================================================================

void PlayerCreatureController::handleMessage (const int message, const float value, const MessageQueue::Data* const data, const uint32 flags)
{
	switch (message)
	{
	case CM_commandTimer:
		{
			const MessageQueueCommandTimer * const msg = NON_NULL( dynamic_cast< const MessageQueueCommandTimer * >( data ) );
			if(msg)
				Transceivers::commandTimerDataReceived.emitMessage (*msg);

		}
		break;

	case CM_clientResourceHarvesterResourceData:
		{
			const MessageQueueHarvesterResourceData * const resourceData = NON_NULL (dynamic_cast<const MessageQueueHarvesterResourceData *>(data));
			if (resourceData)
				Transceivers::s_installationResourceData.emitMessage (*resourceData);
		}
		break;

	case CM_missionListResponse:
		{
			DEBUG_WARNING(true, ("Deprecated MessageQueueMissionListResponse message received"));
		}
		break;

	case CM_missionDetailsResponse:
		{
			const MessageQueueMissionDetailsResponse * const response = NON_NULL (dynamic_cast<const MessageQueueMissionDetailsResponse *>(data));
			CuiMissionManager::receiveDetails (*response);
		}
		break;
	case CM_missionAcceptResponse:
		{
			const MessageQueueMissionGenericResponse * const response = NON_NULL (dynamic_cast<const MessageQueueMissionGenericResponse *>(data));
			if (response)
				CuiMissionManager::receiveResponseAccept (*response);
		}
		break;
	case CM_missionRemoveResponse:
		{
			const MessageQueueMissionGenericResponse * const response = NON_NULL (dynamic_cast<const MessageQueueMissionGenericResponse *>(data));
			if (response)
				CuiMissionManager::receiveResponseRemove (*response);
		}
		break;
	case CM_missionAbort:
		{
			const MessageQueueNetworkId * const response = NON_NULL (dynamic_cast<const MessageQueueNetworkId *>(data));
			if (response)
				CuiMissionManager::receiveResponseAbort (*response);
		}
		break;
/*
	case CM_missionCreateResponse:
		{
			const MessageQueueMissionGenericResponse * const response = NON_NULL (dynamic_cast<const MessageQueueMissionGenericResponse *>(data));
			if (response)
				CuiMissionManager::receiveResponseCreate (*response);
		}
		break;
*/
	case CM_secureTrade:
		{
			const MessageQueueSecureTrade * const mqstMessage = NON_NULL(dynamic_cast<const MessageQueueSecureTrade*>(data));
			if (mqstMessage)
				handleSecureTradeMessage(*mqstMessage);
		}
		break;

	case CM_commandQueueRemove:
		{
			MessageQueueCommandQueueRemove const *msg = safe_cast<MessageQueueCommandQueueRemove const *>(data);
			if (msg)
			{
				Command::ErrorCode status = static_cast<Command::ErrorCode>(msg->getStatus());

				if (status < 0 || status >= Command::CEC_Max)
				{
					WARNING_DEBUG_FATAL(true, ("PlayerCreatureController CM_commandQueueRemove received with invalid status=[%d]", status));
					status = Command::CEC_Success;
				}

				ClientCommandQueue::handleCommandRemoved(msg->getSequenceId(), msg->getWaitTime(), status, msg->getStatusDetail());
			}
			else
			{
				DEBUG_FATAL(true, ("NULL data when expecting a MessageQueueCommandQueueRemove message payload."));
			}
		}
		break;

	case CM_spatialChatReceive:
		{
			const MessageQueueSpatialChat * const spatialChat = dynamic_cast<const MessageQueueSpatialChat *>(data);
			NOT_NULL (spatialChat);
			if (spatialChat) //lint !e774 //true
				CuiSpatialChatManager::processMessage (*spatialChat);
		}
		break;

	case CM_socialReceive:
		{
			const MessageQueueSocial * const socialMsg = NON_NULL (dynamic_cast<const MessageQueueSocial *>(data));
			if (socialMsg)
				CuiSocialsManager::processMessage (*socialMsg);
		}
		break;

	case CM_npcConversationStart:
		{
			MessageQueueStartNpcConversation const * const startNpcConversationMessage = dynamic_cast<const MessageQueueStartNpcConversation *>(data);
			NOT_NULL(startNpcConversationMessage);

			if (startNpcConversationMessage) //lint !e774 //true
			{
				static std::string noSound;
				CuiConversationManager::setTarget(startNpcConversationMessage->getNpc(), startNpcConversationMessage->getAppearanceOverrideTemplateCrc(), noSound);
			}
		}
		break;

	case CM_npcConversationStop:
		{
			const MessageQueueStopNpcConversation * const stopNpcConversationMessage = dynamic_cast<const MessageQueueStopNpcConversation *>(data);
			NOT_NULL(stopNpcConversationMessage);

			if (stopNpcConversationMessage) //lint !e774 //true
			{
				CuiConversationManager::onServerStopConversing( stopNpcConversationMessage->getNpc(), 
																stopNpcConversationMessage->getFinalMessageId(), 
																stopNpcConversationMessage->getFinalMessageProse(), 
																stopNpcConversationMessage->getFinalResponse());
			}
			else
			{
				CuiConversationManager::stop();
			}
		}
		break;

	case CM_npcConversationMessage:
		{
			const MessageQueueNpcConversationMessage * const npcConversationMessage = dynamic_cast<const MessageQueueNpcConversationMessage *>(data);
			NOT_NULL(npcConversationMessage);

			if (npcConversationMessage) //lint !e774 //true
			{
				const Unicode::String & msg = npcConversationMessage->getNpcMessage();
				CuiConversationManager::issueMessage (msg);
			}
		}
		break;

	case CM_npcConversationResponses:
		{
			const MessageQueueStringList * const npcConversationResponsesMessage = dynamic_cast<const MessageQueueStringList *>(data);
			NOT_NULL(npcConversationResponsesMessage);

			if (npcConversationResponsesMessage) //lint !e774 //true
			{
				CuiConversationManager::StringVector sv;

				const int count = npcConversationResponsesMessage->getStringCount();
				for (int i = 0; i < count; ++i)
				{
					const Unicode::String & response = npcConversationResponsesMessage->getString(i);
					sv.push_back (response);
				}

				CuiConversationManager::setResponses (sv);
			}
		}
		break;

	case CM_objectMenuResponse:
		{
			const MessageQueueObjectMenuRequest * const msg = dynamic_cast<const MessageQueueObjectMenuRequest *>(data);
			NOT_NULL (msg);
			if (msg) //lint !e774 //true
				CuiRadialMenuManager::handleObjectMenuRequestMessage(*msg);
		}
		break;

	case CM_draftSlotsQueryResponse:
		{
			const MessageQueueDraftSlotsQueryResponse * const msg = dynamic_cast<const MessageQueueDraftSlotsQueryResponse *>(data);
			NOT_NULL (msg);
			if (msg) //lint !e774 //true
				DraftSchematicManager::receiveDraftSlotsQueryResponse (*msg);
		}
		break;

	case CM_resourceWeights:
		{
			const MessageQueueResourceWeights * const msg = dynamic_cast<const MessageQueueResourceWeights *>(data);
			NOT_NULL (msg);
			if (msg) //lint !e774 //true
				DraftSchematicManager::receiveResourceWeights (*msg);
		}
		break;


	case CM_biographyRetrieved:
		{
			typedef std::pair<NetworkId, Unicode::String> DataType ;
			const MessageQueueGenericValueType<DataType> * const genericMessage = NON_NULL(safe_cast<const MessageQueueGenericValueType<DataType> *>(data));

			NetworkId const &networkId = genericMessage->getValue().first;
			Unicode::String const &biography = genericMessage->getValue().second;
			Object* obj = NetworkIdManager::getObjectById(networkId);
			CreatureObject* creatureObj = dynamic_cast<CreatureObject*>(obj);
			if(creatureObj)
			{
				PlayerObject* playerObj = creatureObj->getPlayerObject();
				if(playerObj)
				{
					PlayerObject* po = Game::getPlayerObject();
					if(po == playerObj)
						playerObj->setBiography(biography, true);
					else
						playerObj->setBiography(biography, false);
					playerObj->setHaveBiography(true);
					// Send transciever
					Messages::BiographyRetrieved::BiographyOwner biographyRetrieved(std::make_pair(networkId, playerObj));
					Transceivers::biographyRetrieved.emitMessage(biographyRetrieved);
				}
			}
		}
		break;

	case CM_characterMatchRetrieved:
		{
			const MessageQueueGenericValueType<MatchMakingCharacterResult> * const matchMessage = safe_cast<const MessageQueueGenericValueType<MatchMakingCharacterResult> *>(data);

			if (matchMessage != NULL)
			{
				// Send transciever

				Messages::CharacterMatchRetrieved::MatchResults const matchResults(matchMessage->getValue());

				Transceivers::characterMatchRetrieved.emitMessage(matchResults);
			}
		}
		break;

	case CM_startingLocations:
		{
			typedef std::pair<StartingLocationData, bool>   PayloadData;
			typedef std::vector<PayloadData>                Payload;
			typedef MessageQueueGenericValueType<Payload>   MessageType;

			const MessageQueueGenericValueType<Payload> * const genericMessage = safe_cast<const MessageType *>(data);
			if (genericMessage)
				Transceivers::startingLocationsReceived.emitMessage (genericMessage->getValue ());
		}
		break;

	case CM_startingLocationSelectionResult:
		{
			typedef std::pair<std::string, bool> Payload;
			typedef MessageQueueGenericValueType<Payload>   MessageType;

			const MessageQueueGenericValueType<Payload> * const genericMessage = safe_cast<const MessageType *>(data);
			if (genericMessage)
				Transceivers::startingLocationSelectionResult.emitMessage (genericMessage->getValue ());
		}
		break;

	case CM_imageDesignerStart:
		{
			ImageDesignStartMessage const * const inMsg = safe_cast<ImageDesignStartMessage const *>(data);
			if (inMsg)
			{
				Object const * const player = Game::getPlayer();
				if(player)
				{
					std::string designerParams = inMsg->getRecipientId().getValueString();
					designerParams.push_back(' ');
					designerParams += inMsg->getTerminalId().getValueString();
					designerParams.push_back(' ');
					designerParams += inMsg->getCurrentHoloEmote().empty() ? "none" : inMsg->getCurrentHoloEmote();

					std::string recipientParams = inMsg->getDesignerId().getValueString();
					recipientParams.push_back(' ');
					recipientParams += inMsg->getTerminalId().getValueString();
					recipientParams.push_back(' ');
					recipientParams += inMsg->getCurrentHoloEmote().empty() ? "none" : inMsg->getCurrentHoloEmote();

					NetworkId const & playerId = player->getNetworkId();
					//if we're the designer
					if(playerId == inMsg->getDesignerId() && playerId != inMsg->getRecipientId())
					{
						IGNORE_RETURN(CuiActionManager::performAction (CuiActions::imageDesignerDesigner, Unicode::narrowToWide(designerParams)));
					//if we're the recipient
					}
					else if(playerId != inMsg->getDesignerId() && playerId == inMsg->getRecipientId())
					{
						IGNORE_RETURN(CuiActionManager::performAction (CuiActions::imageDesignerRecipient, Unicode::narrowToWide(recipientParams)));
					}
					//if we're designing ourself
					else if(playerId == inMsg->getDesignerId() && playerId == inMsg->getRecipientId())
					{
						IGNORE_RETURN(CuiActionManager::performAction (CuiActions::imageDesignerDesigner, Unicode::narrowToWide(designerParams)));
					}
					else
						DEBUG_FATAL(true, ("ImageDesignStartMessage message received, but neither party, [%s] or [%s], is me [%s]", inMsg->getDesignerId().getValueString().c_str(), inMsg->getRecipientId().getValueString().c_str(), playerId.getValueString().c_str()));
				}
			}
		}
		break;

	case CM_imageDesignerChange:
		{
			ImageDesignChangeMessage const * const inMsg = safe_cast<ImageDesignChangeMessage const *>(data);
			if (inMsg)
			{
				PlayerCreatureController::Messages::ImageDesignerChangeReceived::Payload const payload(*inMsg);
				Transceivers::imageDesignerChangeReceived.emitMessage (payload);
			}
		}
		break;

	case CM_imageDesignerCancel:
		{
			ImageDesignChangeMessage const * const inMsg = safe_cast<ImageDesignChangeMessage const *>(data);
			if (inMsg)
			{
				PlayerCreatureController::Messages::ImageDesignerCancelReceived::Payload const payload(*inMsg);
				Transceivers::imageDesignerCancelReceived.emitMessage (payload);
			}
		}
		break;

	case CM_buffBuilderStart:
		{
			BuffBuilderStartMessage const * const inMsg = safe_cast<BuffBuilderStartMessage const *>(data);
			if (inMsg)
			{
				Object const * const player = Game::getPlayer();
				if(player)
				{
					std::string bufferParams = inMsg->getRecipientId().getValueString();
					bufferParams.push_back(' ');

					std::string recipientParams = inMsg->getBufferId().getValueString();
					recipientParams.push_back(' ');
			
					NetworkId const & playerId = player->getNetworkId();
					//if we're the buffer
					if(playerId == inMsg->getBufferId() && playerId != inMsg->getRecipientId())
					{
						IGNORE_RETURN(CuiActionManager::performAction (CuiActions::openBuffBuilderBufferWindow, Unicode::narrowToWide(bufferParams)));
					//if we're the recipient
					}
					else if(playerId != inMsg->getBufferId() && playerId == inMsg->getRecipientId())
					{
						IGNORE_RETURN(CuiActionManager::performAction (CuiActions::openBuffBuilderBuffeeWindow, Unicode::narrowToWide(recipientParams)));
					}
					//if we're buffing ourself
					else if(playerId == inMsg->getBufferId() && playerId == inMsg->getRecipientId())
					{
						IGNORE_RETURN(CuiActionManager::performAction (CuiActions::openBuffBuilderBufferWindow, Unicode::narrowToWide(bufferParams)));
					}
					else
					{
						DEBUG_FATAL(true, ("PlayerCreatureController::handleMessage - BuffBuilderStartMessage message received, but neither party, [%s] or [%s], is me [%s]", inMsg->getBufferId().getValueString().c_str(), inMsg->getRecipientId().getValueString().c_str(), playerId.getValueString().c_str()));
					}
				}
			}
		}
		break;

	case CM_buffBuilderChange:
		{
			BuffBuilderChangeMessage const * const inMsg = safe_cast<BuffBuilderChangeMessage const *>(data);
			if (inMsg)
			{
				PlayerCreatureController::Messages::BuffBuilderChangeReceived::Payload const payload(*inMsg);
				Transceivers::buffBuilderChangeReceived.emitMessage (payload);
			}
		}
		break;

	case CM_buffBuilderCancel:
		{
			BuffBuilderChangeMessage const * const inMsg = safe_cast<BuffBuilderChangeMessage const *>(data);
			if (inMsg)
			{
				PlayerCreatureController::Messages::BuffBuilderCancelReceived::Payload const payload(*inMsg);
				Transceivers::buffBuilderCancelReceived.emitMessage (payload);
			}
		}
		break;

	case CM_incubatorStart:
		{
			IncubatorStartMessage const * const inMsg = safe_cast<IncubatorStartMessage const *>(data);
			if (inMsg)
			{
				Object const * const player = Game::getPlayer();
				if(player)
				{
					NetworkId const & playerId = player->getNetworkId();
					if(playerId == inMsg->getPlayerId())
					{
						std::string params;
						inMsg->getStringRepresentation(params);
						IGNORE_RETURN(CuiActionManager::performAction (CuiActions::openIncubatorWindow, Unicode::narrowToWide(params)));
					}
					else
					{
						DEBUG_FATAL(true, ("PlayerCreatureController::handleMessage - IncubatorStartMessage message received, but [%s] is me [%s] ... terminalid == [%s]", inMsg->getPlayerId().getValueString().c_str(),  playerId.getValueString().c_str(),inMsg->getTerminalId().getValueString().c_str()));
					}
				}
			}
		}
		break;

	case CM_incubatorCommit:
		{
			// JU_TODO: remove
			DEBUG_REPORT_LOG(true,("PlayerCreatureController::handleMessage -- CM_incubatorCommit\n"));
		}
		break;

	case CM_incubatorCancel:
		{
			// JU_TODO: remove
			DEBUG_REPORT_LOG(true,("PlayerCreatureController::handleMessage -- CM_incubatorCancel\n"));
		}
		break;

	case CM_formDataForEdit:
		{
			typedef std::pair<NetworkId, Unicode::String> Payload;
			typedef MessageQueueGenericValueType<Payload> MessageType;
			MessageQueueGenericValueType<Payload> const * const genericMessage = safe_cast<MessageType const *>(data);
			if(genericMessage)
			{
				NetworkId const & objectToEdit = genericMessage->getValue().first;
				Unicode::String const & packedData = genericMessage->getValue().second;
				Object* obj = NetworkIdManager::getObjectById(objectToEdit);
				ClientObject * const objClient = obj ? obj->asClientObject() : NULL;
				if(objClient)
				{
					FormManager::UnpackedFormData const unpackedData = FormManager::unpackFormData(packedData);
					FormManagerClient::handleReceiveEditObjectData(objectToEdit, unpackedData);
				}
			}
		}
		break;

	case CM_serverAsteroidDebugData:
		{
			MessageQueueGenericValueType<std::vector<Sphere> > const * const serverAsteroidLocations = dynamic_cast<const MessageQueueGenericValueType<std::vector<Sphere> > *>(data);
			NOT_NULL(serverAsteroidLocations);

			if (serverAsteroidLocations) //lint !e774 //true
			{
				ClientAsteroidManager::setServerAsteroidData(serverAsteroidLocations->getValue());
			}
		}
		break;

	case CM_aboutToHyperspace:
		{
			ShipObject * const ship = Game::getPlayerContainingShip();
			if(ship)
			{
				GroundScene * const groundScene = safe_cast<GroundScene * const>(Game::getScene());
				if (groundScene != 0)
				{
					GameCamera * const gameCamera = groundScene->getCamera(GroundScene::CI_cockpit);
					CockpitCamera * const cockpitCamera = safe_cast<CockpitCamera * const>(gameCamera);
					if (cockpitCamera != 0)
					{
						IGNORE_RETURN(new HyperspaceIoWin(cockpitCamera, ship, HyperspaceIoWin::S_enter));
					}
				}
			}
		}
		break;

	case CM_hyperspaceOrientShipToPointAndLockPlayerInput:
		{
			Vector pos;
			ShipObject * const ship = Game::getPlayerContainingShip();
			if (ship != 0)
			{
				MessageQueueGenericValueType<std::pair<std::string, Vector > > const * const aboutToHyperspace = dynamic_cast<const MessageQueueGenericValueType<std::pair<std::string, Vector > > *>(data);
				if (aboutToHyperspace != 0)
				{
					Vector const & direction = HyperspaceManager::getDirectionToHyperspacePoint_w(Game::getSceneId(), ship->getPosition_w(), aboutToHyperspace->getValue().first, aboutToHyperspace->getValue().second);
					Controller * const controller = ship->getController();
					PlayerShipController * const playerShipController = dynamic_cast<PlayerShipController *>(controller);
					if(playerShipController != 0)
					{
						playerShipController->setThrottlePosition(0.0f, true);
						playerShipController->lockInputState(true);
						playerShipController->engageAutopilotToDirection(direction);
					}
				}
			}
		}
		break;


	case CM_lockPlayerShipInputOnClient:
		{
			ShipObject * const ship = Game::getPlayerContainingShip();
			Controller * const controller = (ship != 0) ? ship->getController() : 0;
			PlayerShipController * const playerShipController = dynamic_cast<PlayerShipController *>(controller);

			if (playerShipController != 0)
			{
				playerShipController->lockInputState(true);
			}
		}
		break;

	case CM_unlockPlayerShipInputOnClient:
		{
			ShipObject * const ship = Game::getPlayerContainingShip();
			Controller * const controller = (ship != 0) ? ship->getController() : 0;
			PlayerShipController * const playerShipController = dynamic_cast<PlayerShipController *>(controller);

			if (playerShipController != 0)
			{
				playerShipController->lockInputState(false);
			}
		}
		break;

	case CM_uiPlayEffect:
		{
			MessageQueueGenericValueType<std::pair<NetworkId, std::string> > const * const msg = safe_cast<const MessageQueueGenericValueType<std::pair<NetworkId, std::string> > *>(data);
			if (msg)
			{
				std::string const & effectName = msg->getValue().second;
				CuiManager::playUiEffect(effectName);
			}
		}
		break;

	case CM_spaceTerminalResponse:
		{
			typedef std::vector<std::pair<NetworkId, std::string> > IncomingPayload;
			MessageQueueGenericValueType<IncomingPayload> const * const msg = safe_cast<MessageQueueGenericValueType<IncomingPayload> const *>(data);
			if (msg)
			{
				IncomingPayload const & data = msg->getValue();
				PlayerCreatureController::Messages::ShipParkingDataReceived::Payload const payload(data);
				Transceivers::shipParkingDataReceived.emitMessage (payload);
			}
		}
		break;

	case CM_askGroupMemberToLaunchIntoSpace:
		{
			MessageQueueGenericValueType<NetworkId> const * const msg = safe_cast<MessageQueueGenericValueType<NetworkId> const *>(data);
			if (msg != 0)
			{
				NetworkId const & acceptingPlayer = msg->getValue();
				GroupManager::handleInviteToLaunchIntoSpace(acceptingPlayer);
			}
		}
		break;


	case CM_relayGroupMemberInvitationToLaunchAnswer:
		{
			typedef std::pair<NetworkId, bool> Payload;
			MessageQueueGenericValueType<Payload> const * const msg = safe_cast<MessageQueueGenericValueType<Payload> const *>(data);
			if (msg != 0)
			{
				Payload const & payload = msg->getValue();
				Transceivers::GroupMemberInvitationToLaunchReceived.emitMessage(payload);
			}
		}
		break;

	case CM_sitOnObject:
		{
			// activate the timer
			gs_checkUpdateTransformAfterSittingTimer = true;
		}
		break;

	case CM_groupOpenLotteryWindowOnClient:
		{
			typedef MessageQueueGenericValueType<NetworkId> Message;
			Message const * const msg = dynamic_cast<const Message *>(data);
			NOT_NULL(msg);

			if (msg != 0)
			{
				Unicode::String const target(Unicode::narrowToWide(msg->getValue().getValueString()));
				CuiActionManager::performAction(CuiActions::spawnGroupLootLottery, target);
			}
		}
		break;

	case CM_groupCloseLotteryWindowOnClient:
		{
			typedef MessageQueueGenericValueType<NetworkId> Message;
			Message const * const msg = dynamic_cast<const Message *>(data);
			NOT_NULL(msg);

			if (msg != 0)
			{
				Unicode::String const target(Unicode::narrowToWide(msg->getValue().getValueString()));
				CuiActionManager::performAction(CuiActions::closeGroupLootLottery, target);
			}
		}
		break;

	case CM_questTaskCounterData:
		{
			MessageQueueQuestTaskCounterMessage const * const msg = safe_cast<MessageQueueQuestTaskCounterMessage const *>(data);
			if (msg != NULL)
			{
				QuestJournalManager::setCounter(msg->getQuestName(), msg->getTaskId(), msg->getSourceName(), msg->getCounter(), msg->getCounterMax());
			}
		}
		break;

	case CM_questTaskLocationData:
		{
			MessageQueueQuestTaskLocationMessage const * const msg = safe_cast<MessageQueueQuestTaskLocationMessage const *>(data);
			if (msg != NULL)
			{
				QuestJournalManager::setLocation(msg->getQuestName(), msg->getTaskId(), msg->getLocation());
			}
		}
		break;

	case CM_questTaskTimerData:
		{
			MessageQueueQuestTaskTimerMessage const * const msg = safe_cast<MessageQueueQuestTaskTimerMessage const *>(data);
			if (msg != NULL)
			{
				QuestJournalManager::setTimer(msg->getQuestName(), msg->getTaskId(), msg->getSourceName(), msg->getTimerLength());
			}
		}
		break;

	case CM_cyberneticsOpen:
		{
			MessageQueueCyberneticsOpen const * const msg = dynamic_cast<MessageQueueCyberneticsOpen const *>(data);
			NOT_NULL(msg);
			if (msg != NULL)
			{
				MessageQueueCyberneticsOpen::OpenType const openType = msg->getOpenType();
				NetworkId const & npc = msg->getNPC();
				Unicode::String parms = Unicode::narrowToWide(npc.getValueString() + " ");
				char buffer[64];
				_itoa(openType, buffer, 10);
				parms += Unicode::narrowToWide(buffer);
				CuiActionManager::performAction(CuiActions::cybernetics, parms);
			}
		}
		break;

	case CM_spaceMiningSaleOpen:
		{
			typedef std::pair<NetworkId /*spaceStationId*/, std::string /*spaceStationName*/> MessageData;
			typedef MessageQueueGenericValueType<MessageData> MessageType;
			MessageType const * const msg = dynamic_cast<MessageType const *>(data);

			if (NULL != msg)
			{
				std::string params;
				params += msg->getValue().first.getValueString().c_str();
				params.push_back(' ');
				params += msg->getValue().second;
				CuiActionManager::performAction(CuiActions::spaceMiningSale, Unicode::narrowToWide(params));
			}
		}
		break;

	case CM_requestActivateQuest:
		{
			MessageQueueGenericValueType<int> const * const msg = dynamic_cast<MessageQueueGenericValueType<int> const *> (data);
			if(msg)
			{
				Unicode::String params;
				params =  Unicode::narrowToWide("0");
				params += Unicode::narrowToWide(" ");
				char buffer[64];
				_itoa(msg->getValue(), buffer, 10);
				params += Unicode::narrowToWide(buffer);
				CuiActionManager::performAction(CuiActions::questAcceptance, params);
			}
		}
		break;

	case CM_requestCompleteQuest:
		{
			MessageQueueGenericValueType<int> const * const msg = dynamic_cast<MessageQueueGenericValueType<int> const *> (data);
			if(msg)
			{
				Unicode::String params;
				params =  Unicode::narrowToWide("1");
				params += Unicode::narrowToWide(" ");
				char buffer[64];
				_itoa(msg->getValue(), buffer, 10);
				params += Unicode::narrowToWide(buffer);
				CuiActionManager::performAction(CuiActions::questAcceptance, params);
			}
		}
		break;

	case CM_forceActivateQuest:
		{
			MessageQueueGenericValueType<int> const * const msg = dynamic_cast<MessageQueueGenericValueType<int> const *> (data);
			if(msg)
			{
				Unicode::String params;
				params =  Unicode::narrowToWide("2");
				params += Unicode::narrowToWide(" ");
				char buffer[64];
				_itoa(msg->getValue(), buffer, 10);
				params += Unicode::narrowToWide(buffer);
				CuiActionManager::performAction(CuiActions::questAcceptance, params);
			}
		}
		break;

	case CM_staticLootItemData:
		{
			MessageQueueGenericValueType<std::vector<Unicode::String> > const * const msg = dynamic_cast<MessageQueueGenericValueType<std::vector<Unicode::String> > const *>(data);
			if(msg)
			{
				//this vector represents a flatted dictionary, with ALL keys, then ALL values stored into it
				std::vector<Unicode::String> const & flattedDictionary = msg->getValue();
				DEBUG_FATAL(flattedDictionary.size() % 2 != 0, ("Flatted dictionary does not have an even number of values"));
				int const realDictionarySize = flattedDictionary.size() / 2;
				std::vector<Unicode::String> keys;
				keys.reserve(realDictionarySize);
				std::vector<Unicode::String> values;
				values.reserve(realDictionarySize);
				//split flatted dictionary into parallel arrays
				int counter = 0;
				for(std::vector<Unicode::String>::const_iterator i = flattedDictionary.begin(); i != flattedDictionary.end(); ++i, ++counter)
				{
					if(counter < realDictionarySize)
						keys.push_back(*i);
					else
						values.push_back(*i);
				}
				DEBUG_FATAL(keys.size() != values.size(), ("Did not end up with a matching set of vectors"));
				CuiStaticLootItemManager::setItemData(keys, values);
			}
		}
		break;

	case CM_showLootBox:
		{
			MessageQueueGenericValueType<std::vector<NetworkId> > const * const msg = dynamic_cast<MessageQueueGenericValueType<std::vector<NetworkId> > const *>(data);
			if(msg)
			{
				std::vector<NetworkId> const & objects = msg->getValue();
				Unicode::String str;
				for(std::vector<NetworkId>::const_iterator i = objects.begin(); i != objects.end(); ++i)
				{
					str += Unicode::narrowToWide(i->getValueString() + " ");
				}
				IGNORE_RETURN(CuiActionManager::performAction (CuiActions::lootBox, str));
			}
		}
		break;

	case CM_clientMinigameOpen:
		{
			typedef ValueDictionary Payload;			
			MessageQueueGenericValueType<Payload> const * const msg = dynamic_cast<MessageQueueGenericValueType<Payload> const *>(data);

			if(msg)
			{
				CuiMinigameManager::startMinigame(msg->getValue());
			}
		}
		break;

	case CM_clientMinigameClose:
		{
			typedef ValueDictionary Payload;			
			MessageQueueGenericValueType<Payload> const * const msg = dynamic_cast<MessageQueueGenericValueType<Payload> const *>(data);

			if(msg)
			{
				CuiMinigameManager::closeMinigame(msg->getValue());
			}
		}
		break;

	default:
		break;
	}

	CreatureController::handleMessage (message, value, data, flags);
}

//----------------------------------------------------------------------

void PlayerCreatureController::ackTeleport(int sequenceId)
{
	MessageQueueTeleportAck *msg = new MessageQueueTeleportAck(sequenceId);
	appendMessage(
		static_cast<int>(CM_teleportAck),
		0.f,
		msg,
		GameControllerMessageFlags::SEND |
		GameControllerMessageFlags::RELIABLE |
		GameControllerMessageFlags::DEST_AUTH_SERVER);
}

//----------------------------------------------------------------------

//@todo HACK - There's no warp message yet, and we need one to make elevators work under the new collision
// system. We're hacking around it by assuming that any hNUT* message sent to the PlayerCreatureController is
// intended to be a warp.

void PlayerCreatureController::handleNetUpdateTransform (const MessageQueueDataTransform& message)
{
	DEBUG_REPORT_LOG (ms_logServerMovementData && (message.getSequenceNumber () <  m_serverSequenceNumber), ("PlayerCreatureController::handleNetUpdateTransform [%s]: disregarding older packet %i\n", getOwner ()->getNetworkId ().getValueString ().c_str (), m_serverSequenceNumber));
	DEBUG_REPORT_LOG (ms_logServerMovementData && (message.getSequenceNumber () == m_serverSequenceNumber), ("PlayerCreatureController::handleNetUpdateTransform [%s]: received same packet %i multiple times\n", getOwner ()->getNetworkId ().getValueString ().c_str (), m_serverSequenceNumber));

#if 0
	if (message.getSequenceNumber () <= m_serverSequenceNumber)
		return;
#endif

	m_serverSequenceNumber = message.getSequenceNumber ();

	DEBUG_REPORT_LOG (ms_logServerMovementData, ("PlayerCreatureController::handleNetUpdateTransform [%s]: accepted nut %i <%1.1f, %1.1f, %1.1f> <%1.1f> <%1.1f %d>\n",
		getOwner ()->getNetworkId ().getValueString ().c_str (),
		m_serverSequenceNumber,
		message.getTransform ().getPosition_p ().x,
		message.getTransform ().getPosition_p ().y,
		message.getTransform ().getPosition_p ().z,
		message.getTransform ().getLocalFrameK_p ().theta (),
		message.getLookAtYaw (),
		message.getUseLookAtYaw ()));


	//-- If we're mounted, we ignore updates for the player's position.
	//   The client has the player parented to the mount.  The server
	//   has the player position in the same space as the mount.  Since
	//   the server controls the mount, we don't need to listen to player
	//   updates.
	CreatureObject *owner = safe_cast<CreatureObject*>(getOwner());
	if (owner)
	{
		CreatureObject     *creatureForUpdate   = owner;
		CreatureController *controllerForUpdate = this;

		if (owner->isRidingMount())
		{
			// since we're riding a mount, apply the net update to the mount.
			CreatureObject     *const mountCreature   = owner->getMountedCreature();
			CreatureController *const mountController = mountCreature ? dynamic_cast<CreatureController*>(mountCreature->getController()) : NULL;

			if (mountCreature && mountController)
			{
				creatureForUpdate   = mountCreature;
				controllerForUpdate = mountController;
			}
		}
		NOT_NULL(creatureForUpdate);
		NOT_NULL(controllerForUpdate);

		// if we are not initialized, or we are passed a negative sequence number, that means we're just locking out movement updates until we're ready
		if (creatureForUpdate->isInitialized() && message.getSequenceNumber() >= 0)
		{
			// We specifically want to drive this up to the CreatureController level or closer to the base of the inheritance chain.
			controllerForUpdate->doClientHandleNetUpdateTransform(message);
			CollisionWorld::objectWarped(creatureForUpdate);

			// Make sure the camera is pointing in the correct direction since the free chase camera is authoritative for orientation
			GroundScene * const groundScene = safe_cast<GroundScene * const>(Game::getScene());
			if (groundScene != 0)
			{
				GameCamera * const gameCamera = groundScene->getCamera(GroundScene::CI_freeChase);
				FreeChaseCamera * const freeChaseCamera = safe_cast<FreeChaseCamera * const>(gameCamera);
				if (freeChaseCamera != 0)
				{
					// If the message does not tell us explicitly to change the yaw
					// we will keep our old yaw so the camera will stay in the same
					// place for things like getting out of vehicles or chairs
					freeChaseCamera->setTarget(owner, true, message.getUseLookAtYaw());
				}
			}
		}
	}

	ackTeleport(message.getSequenceNumber());
}

//----------------------------------------------------------------------

void PlayerCreatureController::handleNetUpdateTransformWithParent (const MessageQueueDataTransformWithParent& message)
{
	DEBUG_REPORT_LOG (ms_logServerMovementData && (message.getSequenceNumber () <  m_serverSequenceNumber), ("PlayerCreatureController::handleNetUpdateTransformWithParent [%s]: disregarding older packet %i\n", getOwner ()->getNetworkId ().getValueString ().c_str (), m_serverSequenceNumber));
	DEBUG_REPORT_LOG (ms_logServerMovementData && (message.getSequenceNumber () == m_serverSequenceNumber), ("PlayerCreatureController::handleNetUpdateTransformWithParent [%s]: received same packet %i multiple times\n", getOwner ()->getNetworkId ().getValueString ().c_str (), m_serverSequenceNumber));

#if 0
	if (message.getSequenceNumber () <= m_serverSequenceNumber)
		return;
#endif

	m_serverSequenceNumber = message.getSequenceNumber ();

	DEBUG_REPORT_LOG (ms_logServerMovementData, ("PlayerCreatureController::handleNetUpdateTransformWithParent [%s]: accepted nutwp %i %s <%1.1f, %1.1f, %1.1f> <%1.1f> <%1.1f> <%1.1f %d>\n",
		getOwner ()->getNetworkId ().getValueString ().c_str (),
		m_serverSequenceNumber,
		message.getParent ().getValueString ().c_str (),
		message.getTransform ().getPosition_p ().x,
		message.getTransform ().getPosition_p ().y,
		message.getTransform ().getPosition_p ().z,
		message.getTransform ().getLocalFrameK_p ().theta (),
		message.getLookAtYaw(),
		message.getUseLookAtYaw()));

	//-- If we're mounted, we ignore updates for the player's position.
	//   The client has the player parented to the mount.  The server
	//   has the player position in the same space as the mount.  Since
	//   the server controls the mount, we don't need to listen to player
	//   updates.
	CreatureObject *owner = safe_cast<CreatureObject *>(getOwner());
	if (owner)
	{
		CreatureObject     *creatureForUpdate   = owner;
		CreatureController *controllerForUpdate = this;

		if (owner->isRidingMount())
		{
			// since we're riding a mount, apply the net update to the mount.
			CreatureObject     *const mountCreature   = owner->getMountedCreature();
			CreatureController *const mountController = mountCreature ? dynamic_cast<CreatureController*>(mountCreature->getController()) : NULL;

			if (mountCreature && mountController)
			{
				creatureForUpdate   = mountCreature;
				controllerForUpdate = mountController;
			}
		}
		NOT_NULL(creatureForUpdate);
		NOT_NULL(controllerForUpdate);

		// if we are not initialized, or we are passed a negative sequence number, that means we're just locking out movement updates until we're ready
		if (owner->isInitialized() && message.getSequenceNumber() >= 0)
		{
			controllerForUpdate->doClientHandleNetUpdateTransformWithParent(message);
			CollisionWorld::objectWarped(creatureForUpdate);

			// Make sure the camera is pointing in the correct direction since the free chase camera is authoritative for orientation
			GroundScene * const groundScene = safe_cast<GroundScene * const>(Game::getScene());
			if (groundScene != 0)
			{
				GameCamera * const gameCamera = groundScene->getCamera(GroundScene::CI_freeChase);
				FreeChaseCamera * const freeChaseCamera = safe_cast<FreeChaseCamera * const>(gameCamera);
				if (freeChaseCamera != 0)
				{
					freeChaseCamera->setTarget(owner, true, message.getUseLookAtYaw());
				}
			}
		}
	}

	ackTeleport(message.getSequenceNumber());
}

//----------------------------------------------------------------------

void PlayerCreatureController::handleSecureTradeMessage(const MessageQueueSecureTrade & msg)
{
	switch (msg.getTradeMessageId())
	{
	case MessageQueueSecureTrade::TMI_TradeRequested:
		ClientSecureTradeManager::onTradeRequested (msg.getInitiator ());
		break;
	case MessageQueueSecureTrade::TMI_DeniedTrade:
		ClientSecureTradeManager::onTradeRequestDenied (msg.getRecipient ());
		break;
	case MessageQueueSecureTrade::TMI_DeniedPlayerBusy:
		ClientSecureTradeManager::onTradeRequestDeniedPlayerBusy (msg.getRecipient ());
		break;
	case MessageQueueSecureTrade::TMI_DeniedPlayerUnreachable:
		ClientSecureTradeManager::onTradeRequestDeniedPlayerUnreachable (msg.getRecipient ());
		break;
	case MessageQueueSecureTrade::TMI_RequestTrade:
	case MessageQueueSecureTrade::TMI_AcceptTrade:
	case MessageQueueSecureTrade::TMI_RequestTradeReversed:
	case MessageQueueSecureTrade::TMI_LastTradeMessageId:
	default:
		{
			DEBUG_REPORT_LOG(true, ("Unhandled secure trade message %d\n", static_cast<int>(msg.getTradeMessageId())));
		}
		break;
	}
}  //lint !e1762 //not const

//-----------------------------------------------------------------------

void PlayerCreatureController::sendCommandQueueEnqueue(uint32 sequenceId,
	uint32 commandHash, NetworkId const &targetId, Unicode::String const &params)
{
	MessageQueueCommandQueueEnqueue *msg = new MessageQueueCommandQueueEnqueue(sequenceId, commandHash, targetId, params);
	appendMessage(
		static_cast<int>(CM_commandQueueEnqueue),
		0.f,
		msg,
		GameControllerMessageFlags::SEND |
		GameControllerMessageFlags::RELIABLE |
		GameControllerMessageFlags::DEST_AUTH_SERVER);
}

//-----------------------------------------------------------------------

void PlayerCreatureController::sendCommandQueueRemove(uint32 sequenceId)
{
	appendMessage(
		static_cast<int>(CM_commandQueueRemove),
		0.f,
		new MessageQueueCommandQueueRemove(sequenceId, 0.0f, 0, 0),
		GameControllerMessageFlags::SEND |
		GameControllerMessageFlags::RELIABLE |
		GameControllerMessageFlags::DEST_AUTH_SERVER);
}

// ----------------------------------------------------------------------

void PlayerCreatureController::setAutoFollowTarget(CreatureObject *target)
{
	CreatureObject * const oldTarget = m_autoFollowTarget->getPointer ();

	//-- allow re-following the current target
	//-- this allows the player to refresh his m_autoFollowTargetOffset

	Unicode::String result;
	if (oldTarget != target && !m_autoFollowTargetName.empty ())
	{
		CuiStringVariablesManager::process (ClientStringIds::follow_stop_prose, Unicode::emptyString, m_autoFollowTargetName, Unicode::emptyString, result);
		CuiSystemMessageManager::sendFakeSystemMessage (result);
	}

	*m_autoFollowTarget = target;

	if (target)
	{
		m_autoFollowTargetOffset = getOwner ()->rotateTranslate_w2o (target->getPosition_w ());
		m_autoFollowTargetOffset.y = 0.0f;

		//-- if target is behind us, flip the vector across the x axis
		if (m_autoFollowTargetOffset.z < 0.0f)
			m_autoFollowTargetOffset.z = -m_autoFollowTargetOffset.z;

		IGNORE_RETURN(m_autoFollowTargetOffset.normalize ());

		m_autoFollowTargetName = target->getLocalizedName ();
		CuiStringVariablesManager::process (ClientStringIds::follow_start_prose, Unicode::emptyString, m_autoFollowTargetName, Unicode::emptyString, result);
		CuiSystemMessageManager::sendFakeSystemMessage (result);
	}
	else
		m_autoFollowTargetName.clear ();
}

// ----------------------------------------------------------------------

CreatureObject *PlayerCreatureController::getAutoFollowTarget()
{
	return m_autoFollowTarget->getPointer();
} //lint !e1762 //not const

// ----------------------------------------------------------------------

const CreatureObject *PlayerCreatureController::getAutoFollowTarget() const
{
	return m_autoFollowTarget->getPointer();
}

//----------------------------------------------------------------------

void PlayerCreatureController::setRunWhenMoving (bool b)
{
	ms_runWhenMoving = b;
}

//----------------------------------------------------------------------

bool PlayerCreatureController::getRunWhenMoving ()
{
	return ms_runWhenMoving;
}

// ----------------------------------------------------------------------

void PlayerCreatureController::checkPlayerMusic(const float elapsedTime)
{
	static float timer = 0.0f;
	timer += elapsedTime;

	if (timer > 1.0f)
	{
		timer = 0.0f;

		const CreatureObject *creatureObject = dynamic_cast<const CreatureObject *>(getOwner());

		if (creatureObject != NULL)
		{
			PlayerMusicManager::checkStatus(creatureObject);
		}
	}
}

//----------------------------------------------------------------------

void PlayerCreatureController::activatePlayerLight ()
{
	if (m_light)
		return;

	m_light = new Light (Light::T_point_multicell, ms_lightColor);
	m_light->setRange (ms_lightRangeNormal);
	m_light->setConstantAttenuation (ms_lightConstantAttenuationNormal);
	m_light->setLinearAttenuation (ms_lightLinearAttenuationNormal);
	m_light->setQuadraticAttenuation (0.f);
	m_light->addToWorld ();
}

//----------------------------------------------------------------------

float PlayerCreatureController::getDesiredSpeed () const
{
	return m_desiredSpeed;
}

//----------------------------------------------------------------------

void PlayerCreatureController::updateLookAtTargetSlot(CreatureObject & owner) const
{
	ShipChassisSlotType::Type targetSlot = owner.getLookAtTargetSlot();
	if (targetSlot != ShipChassisSlotType::SCST_invalid)
	{
		CachedNetworkId const & target = owner.getLookAtTarget();

		if (target.getObject() &&
			target.getObject()->asClientObject() &&
			target.getObject()->asClientObject()->asShipObject() &&
			target.getObject()->asClientObject()->asShipObject()->isComponentDemolished(targetSlot))
			owner.setLookAtTargetSlot(target.getObject()->asClientObject()->asShipObject()->getNextValidTargetableSlot(targetSlot));
	}
}

//----------------------------------------------------------------------

bool PlayerCreatureController::shouldSendUpdatedTransform() const
{
	if (!m_allowMovement)
	{
		return false;
	}

	CreatureObject const * const owner = safe_cast<CreatureObject const *>(getOwner());

	if (owner != 0)
	{
		// we should not process creature movement if we're controlling a ship station.
		if (owner->getShipStation() != ShipStation::ShipStation_None)
		{
			return false;
		}
	}

	return true;
}

//----------------------------------------------------------------------

void PlayerCreatureController::allowMovement(bool const allow)
{
	m_allowMovement = allow;
}

//----------------------------------------------------------------------

bool PlayerCreatureController::shouldProcessMovement() const
{
	if (!m_allowMovement)
	{
		return false;
	}

	if (!shouldSendUpdatedTransform())
	{
		return false;
	}

	CreatureObject const * const owner = safe_cast<CreatureObject const *>(getOwner());

	if ((owner != 0) && (owner->isRidingMount()))
	{
		// check if we're riding a mount that isn't ours.
		CreatureObject const * const mount = owner->getMountedCreature();

		if (mount != 0 && mount->getMasterId().isValid())
		{
			return owner->getNetworkId() == mount->getMasterId();
		}
	}

	return true;
}

//----------------------------------------------------------------------

void PlayerCreatureController::updateBuildingAndCellInformation(CreatureObject const * const owner)
{
	// set the cell values to null.
	m_cellNameCrc = m_buildingSharedTemplateNameCrc = Crc::crcNull;

	if (owner && owner->isInWorld() && !owner->isInWorldCell() && owner->getParentCell()) 
	{
		TemporaryCrcString cellNameCrc(owner->getParentCell()->getCellName(), true);
		m_cellNameCrc = cellNameCrc.getCrc();

		Object const * const building = ContainerInterface::getTopmostContainer(*owner, false);
		if (building)
		{
			SharedObjectTemplate const * const sharedObjectTemplate = building->getObjectTemplate () ? building->getObjectTemplate ()->asSharedObjectTemplate () : NULL;
			if (sharedObjectTemplate)
			{
				m_buildingSharedTemplateNameCrc = sharedObjectTemplate->getCrcName().getCrc();
			}
		}
	}
}

//----------------------------------------------------------------------

bool PlayerCreatureController::hasLookAtYawChanged(bool reliable)
{
	CreatureObject* const creatureObject = safe_cast<CreatureObject*> (getOwner ());
	if(reliable)
		return (fabs(m_previousReliableLookAtYaw - creatureObject->getLookAtYaw()) > ms_lookAtYawChangeThreshold);
	else
		return (fabs(m_previousLookAtYaw - creatureObject->getLookAtYaw()) > ms_lookAtYawChangeThreshold);
}

// ======================================================================
