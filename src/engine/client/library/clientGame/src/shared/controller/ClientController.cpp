// ======================================================================
//
// ClientController.cpp
// Copyright 2001, 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientController.h"

#include "LocalizationManager.h"
#include "UnicodeUtils.h"
#include "clientGame/ClientCombatActionInfo.h"
#include "clientGame/ClientCombatPlaybackManager.h"
#include "clientGame/ClientObject.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/CreatureInfo.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientGame/ObjectAttributeManager.h"
#include "clientGame/ProsePackageManagerClient.h"
#include "clientGame/WeaponObject.h"
#include "clientSkeletalAnimation/SkeletalAnimationDebugging.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/TransformAnimationResolver.h"
#include "clientUserInterface/CuiCraftManager.h"
#include "clientUserInterface/CuiCombatManager.h"
#include "clientUserInterface/CuiRadialMenuManager.h"
#include "clientUserInterface/CuiResourceManager.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "clientUserInterface/CuiTextManager.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/MessageQueue.h"
#include "sharedFoundation/Production.h"
#include "sharedFoundation/Timer.h"
#include "sharedGame/CombatDataTable.h"
#include "sharedMath/PackedRgb.h"
#include "sharedMath/VectorArgb.h"
#include "sharedNetworkMessages/MessageQueueCombatSpam.h"
#include "sharedNetworkMessages/MessageQueueDataTransform.h"
#include "sharedNetworkMessages/MessageQueueDataTransformWithParent.h"
#include "sharedNetworkMessages/MessageQueueDraftSchematics.h"
#include "sharedNetworkMessages/MessageQueueDraftSlots.h"
#include "sharedNetworkMessages/MessageQueueGenericIntResponse.h"
#include "sharedNetworkMessages/MessageQueueGenericResponse.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedNetworkMessages/MessageQueueNetworkId.h"
#include "sharedNetworkMessages/MessageQueueOpponentInfo.h"
#include "sharedNetworkMessages/MessageQueueResourceWeights.h"
#include "sharedNetworkMessages/MessageQueueShowFlyText.h"
#include "sharedNetworkMessages/MessageQueueShowCombatText.h"
#include "sharedNetworkMessages/MessageQueueString.h"
#include "sharedNetworkMessages/ObjectChannelMessages.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedTerrain/TerrainObject.h"
#include "swgSharedNetworkMessages/MessageQueueCombatAction.h"
#include "swgSharedNetworkMessages/MessageQueueCombatDamage.h"

#include <vector>

// ======================================================================

namespace ClientControllerNamespace
{
	void handleCombatActionMessage (const MessageQueueCombatAction &combatActionMessage);
}

using namespace ClientControllerNamespace;

// ======================================================================

void ClientControllerNamespace::handleCombatActionMessage (const MessageQueueCombatAction &combatActionMessage)
{
	//-- Print message contents.
#ifdef _DEBUG
	static int receivedActionCount = 0;

	if (ConfigClientGame::getLogCombatActionMessages ())
	{
		DEBUG_REPORT_LOG(true, ("Received combat action #[%d]:\n", ++receivedActionCount));
		combatActionMessage.debugDump ();
	}
#endif

	//-- Build ClientCombatActionInfo.
	//   Why not just pass in combatActionMessage?  I do this because the
	//   MessageQueueCombatAction is SWG-specific (in SWG libraries), but there's no
	//   reason to make the client combat playback system game-specific.

	// Get defender's object from its network id.
	const MessageQueueCombatAction::DefenderDataVector &defenderDataVector = combatActionMessage.getDefenders ();
	const int                                           defenderCount      = static_cast<int> (defenderDataVector.size ());


	const MessageQueueCombatAction::AttackerData       &attackerData = combatActionMessage.getAttacker();
	
	//-- if we have no defenders we better be using location-based
	if ((defenderCount == 0) && (!attackerData.useLocation))
	{
		if(!attackerData.useLocation)
		{
			DEBUG_WARNING(true, ("Ignoring combat action: no defenders and not location-based..."));
			return;
		}
	}

	// Get attacker.
	Object *const attackerObject = NetworkIdManager::getObjectById(attackerData.id);
	if (!attackerObject)
	{
		DEBUG_WARNING(true, ("Ignoring combat action: attacker object for id [%u] is NULL (not on client?)", attackerData.id.getValue()));
		return;
	}	

	// If attacker is stealth and he is doing the command against
	// a player character who cannot see him (i.e. someone other
	// than himself or a player character who has passively revealed him)
	// temporary display him in translucent outline so other players
	// can see him do the command animation, but only if the command
	// would cause him to be removed from stealth

	// if the attacker is not going to be displayed, don't display
	// any attacker effect associated with the command, as that
	// will reveal the location of the hidden attacker
	bool attackerWillBeVisible = true;

	CreatureObject *const attackerCreatureObject = CreatureObject::asCreatureObject(attackerObject);
	if (attackerCreatureObject && !attackerCreatureObject->getCoverVisibility())
	{
		SkeletalAppearance2 * const appearance = const_cast<SkeletalAppearance2 *>(attackerCreatureObject->getAppearance() ? attackerCreatureObject->getAppearance()->asSkeletalAppearance2() : 0);
		if (appearance && (appearance->getFadeState() == SkeletalAppearance2::FS_removed))
		{
			attackerWillBeVisible = false;

			// can command cause the attacker to be pulled out of stealth?
			int const commandMinInvisLevelRequired = CombatDataTable::getMinInvisLevelRequired(attackerData.actionNameCrc);
			int invisibilityBuffPriority;
			if ((commandMinInvisLevelRequired == -1) || !attackerCreatureObject->hasInvisibilityBuff(invisibilityBuffPriority) || (invisibilityBuffPriority < commandMinInvisLevelRequired))
			{
				for (MessageQueueCombatAction::DefenderDataVector::const_iterator iterDefender = defenderDataVector.begin(); iterDefender != defenderDataVector.end(); ++iterDefender)
				{
					if ((iterDefender->id == attackerData.id) || (attackerCreatureObject->isPassiveRevealPlayerCharacter(iterDefender->id)))
						continue;

					appearance->setFadeState(SkeletalAppearance2::FS_fadeToHold, appearance->getFadeFraction(), 0.75f);
					attackerWillBeVisible = true;
					break;
				}
			}
		}
	}

	// Get attacker weapon.
	Object *const attackerWeaponObject = NetworkIdManager::getObjectById(attackerData.weapon);
	
	if(attackerData.useLocation)
	{
		// If attacker is this client, and action is tagged as doClientAnim in command_data - ignore, since we already played it
		// The exception to this rule is if we are using an alternate appearance. Alternate appearance does NOT use the client side
		// animation system. It allows the server to validate just in-case we want to remove the costume.
		if ( attackerObject->getNetworkId() == Game::getClientPlayer()->getNetworkId() )
		{
			if (CombatDataTable::isActionClientAnim(attackerData.actionNameCrc))
			{
				//we already played this animation
				return;
			}
		}
		
		ClientCombatActionInfo  clientCombatActionInfo(
			attackerWillBeVisible ? attackerObject : NULL,
			attackerWillBeVisible ? attackerWeaponObject : NULL,
			static_cast<int>(attackerData.endPosture),
			attackerData.trailBits,
			attackerData.clientEffectId,
			attackerData.actionNameCrc,
			attackerData.useLocation,
			attackerData.targetLocation,
			attackerData.targetCell,
			combatActionMessage.getActionId(),
			NULL,
			0, 
			static_cast<ClientCombatActionInfo::DefenderDefense>(0),
			0,
			0,
			0
			);
		
		ClientCombatPlaybackManager::handleCombatAction(clientCombatActionInfo);
	}
	else
	{	
		for ( int i = 0; i < defenderCount; ++i )
		{
			// If attacker is this client, and action is tagged as doClientAnim in command_data - ignore, since we already played it
			// The exception to this rule is if we are using an alternate appearance. Alternate appearance does NOT use the client side
			// animation system. It allows the server to validate just in-case we want to remove the costume.
			if (( attackerObject->getNetworkId() == Game::getClientPlayer()->getNetworkId()) && (i == 0))
			{
				if (CombatDataTable::isActionClientAnim(attackerData.actionNameCrc))
				{
					//we already played this animation
					return;
				}
			}

			Object *const defenderObject = NetworkIdManager::getObjectById(defenderDataVector[i].id);

			const MessageQueueCombatAction::DefenderData *const defenderData = &(defenderDataVector[i]);		

			// Handle defender defense (defender disposition).
			const int defenderDefense = defenderData ? static_cast<int>(defenderData->defense) : 0;

			ClientCombatActionInfo  clientCombatActionInfo(
				attackerWillBeVisible ? attackerObject : NULL,
				attackerWillBeVisible ? attackerWeaponObject : NULL,
				static_cast<int>(attackerData.endPosture),
				attackerData.trailBits,
				attackerData.clientEffectId,
				attackerData.actionNameCrc,
				attackerData.useLocation,
				attackerData.targetLocation,
				attackerData.targetCell,
				combatActionMessage.getActionId(),
				defenderObject,
				defenderData ? static_cast<int>(defenderData->endPosture) : 0, 
				static_cast<ClientCombatActionInfo::DefenderDefense>(defenderDefense),
				defenderData ? defenderData->clientEffectId : 0,
				defenderData ? defenderData->hitLocation : 0,
				defenderData ? defenderData->damageAmount : 0
				);

		#ifdef _DEBUG
			if (ConfigClientGame::getLogCombatActionMessages ())
			{
				{
					bool                        hasState;
					CreatureInfo::PostureState  state;

					CreatureInfo::getCreaturePostureState(attackerObject->getNetworkId(), hasState, state);
					if (hasState)
						DEBUG_REPORT_LOG(true, ("MQCA:**: attacker id [%s], ot [%s] already reported as dead/incap, reason [%s].\n", attackerObject->getNetworkId().getValueString().c_str(), attackerObject->getObjectTemplateName(), CreatureInfo::getPostureStateAsCharConst(state)));
				}

				if (defenderObject)
				{
					bool                        hasState;
					CreatureInfo::PostureState  state;

					CreatureInfo::getCreaturePostureState(defenderObject->getNetworkId(), hasState, state);
					if (hasState)
						DEBUG_REPORT_LOG(true, ("MQCA:**: attacker id [%s], ot [%s] already reported as dead/incap, reason [%s].\n", defenderObject->getNetworkId().getValueString().c_str(), defenderObject->getObjectTemplateName(), CreatureInfo::getPostureStateAsCharConst(state)));
				}
			}
		#endif

			ClientCombatPlaybackManager::handleCombatAction(clientCombatActionInfo);
		}
	}
}

// ======================================================================

ClientController::ClientController (Object* newOwner) :
	NetworkController (newOwner),
	m_initialized(false),
	m_sequenceNumber (0)
{
	// objects' controllers created by the client are authoritative by default
	// objects created as a result of a server CREATE_OBJECT message should
	// not be authoritative, and should be handled accordingly
	NetworkController::setAuthoritative (true);
}

// ----------------------------------------------------------------------

ClientController::~ClientController()
{
}

// ----------------------------------------------------------------------

void ClientController::endBaselines()
{
	DEBUG_WARNING(m_initialized, ("ClientController::endBaselines: called multiple times for object [id=%s, template=%s]", getOwner()->getNetworkId().getValueString().c_str(), getOwner()->getObjectTemplateName()));
	m_initialized = true;
}

// ----------------------------------------------------------------------

bool ClientController::isSettingBaselines() const
{
	return safe_cast<const ClientObject*>(getOwner())->isSettingBaselines();
}

//-----------------------------------------------------------------------

int ClientController::getNextSequenceNumber ()
{
	return ++m_sequenceNumber;
}

//-----------------------------------------------------------------------

void ClientController::sendTransform (const Transform& transform_p, const bool reliable)
{
	Object* const attachedTo = getOwner ()->getAttachedTo ();

	uint32 syncStamp = GameNetwork::getServerSyncStampLong();
	uint32 flags     = GameControllerMessageFlags::SEND | GameControllerMessageFlags::DEST_AUTH_SERVER;
	if(reliable)
		flags = flags | GameControllerMessageFlags::RELIABLE;

	ClientObject *ownerAsClient = getOwner() ? getOwner()->asClientObject() : NULL;
	CreatureObject *ownerAsCreature = ownerAsClient ? (ownerAsClient->asCreatureObject()) : NULL;
	if (attachedTo)
	{		
		MessageQueueDataTransformWithParent data (syncStamp, getNextSequenceNumber (), attachedTo->getNetworkId (), transform_p, getCurrentSpeed (), 
			ownerAsCreature ? ownerAsCreature->getLookAtYaw() : 0.f, ownerAsCreature ? ownerAsCreature->getUseLookAtYaw() : false);
		ObjControllerMessage message (getOwner ()->getNetworkId (),
			static_cast<int>(CM_netUpdateTransformWithParent),
			0.0f,
			flags,
			&data);

		sendControllerMessage (message);
	}
	else
	{
		MessageQueueDataTransform data (syncStamp, getNextSequenceNumber (), transform_p, getCurrentSpeed (), 
			ownerAsCreature ? ownerAsCreature->getLookAtYaw() : 0.f, ownerAsCreature ? ownerAsCreature->getUseLookAtYaw() : false);
		ObjControllerMessage message (getOwner ()->getNetworkId (),
			static_cast<int>(CM_netUpdateTransform),
			0.0f,
			flags,
			&data);

		sendControllerMessage (message);
	}
}

//-----------------------------------------------------------------------
/**
 * Do the same thing as sendTransform() but use the specified network id
 * for the parent rather than figuring that out in this function.
 *
 * When mounted, a player sends the position of the mount rather than the
 * position of the player.  This requires sending the mount's parent which
 * will be different than the player's parent, so we need to be able to
 * pass that in.
 *
 * -TRF- I may need to pass in speed for the mount as well.
 * -TRF- consider removing the other version of this function and pass
 *       in the real args.
 */

void ClientController::sendTransformUsingParent (const Transform& transform_p, NetworkId const &parentId, const bool reliable)
{
	uint32 syncStamp = GameNetwork::getServerSyncStampLong();
	uint32 flags     = GameControllerMessageFlags::SEND | GameControllerMessageFlags::DEST_AUTH_SERVER;
	if(reliable)
		flags = flags | GameControllerMessageFlags::RELIABLE;

	ClientObject *ownerAsClient = getOwner() ? getOwner()->asClientObject() : NULL;
	CreatureObject *ownerAsCreature = ownerAsClient ? (ownerAsClient->asCreatureObject()) : NULL;
	if (parentId.isValid())
	{
		MessageQueueDataTransformWithParent data (syncStamp, getNextSequenceNumber (), parentId, transform_p, getCurrentSpeed (), 
			ownerAsCreature ? ownerAsCreature->getLookAtYaw() : 0.f, ownerAsCreature ? ownerAsCreature->getUseLookAtYaw() : false);
		ObjControllerMessage message (getOwner ()->getNetworkId (),
			static_cast<int>(CM_netUpdateTransformWithParent),
			0.0f,
			flags,
			&data);

		sendControllerMessage (message);
	}
	else
	{
		MessageQueueDataTransform data (syncStamp, getNextSequenceNumber (), transform_p, getCurrentSpeed (), 
			ownerAsCreature ? ownerAsCreature->getLookAtYaw() : 0.f, ownerAsCreature ? ownerAsCreature->getUseLookAtYaw() : false);
		ObjControllerMessage message (getOwner ()->getNetworkId (),
			static_cast<int>(CM_netUpdateTransform),
			0.0f,
			flags,
			&data);

		sendControllerMessage (message);
	}
}

// ----------------------------------------------------------------------
/**
 * Returns true if this controller's owner is the animation debugger target
 * object or if there is no animation debugger target object.
 *
 * @return  true if this controller's owner is the animation debugger target
 *          object or if there is no animation debugger target object; false otherwise.
 */

bool ClientController::isOwnerAnimationDebuggerTarget() const
{
	//-- Get the owner.
	Object const *const owner = getOwner();

	//-- Test if owner exists.
	if (!owner)
	{
		// Don't print debug messages if there is no owner object.
		return false;
	}

	//-- Test if there is an animation debugger target.
	Object const *debuggerFocusObject = SkeletalAnimationDebugging::getFocusObject();
	if (!debuggerFocusObject)
	{
		// Print debug messages since animation debugger indicates no focus object.
		// Debugging for all objects is enabled.
		return true;
	}

	//-- Thest if the owner is the animation debugger target.
	return (debuggerFocusObject == owner);
}

//-----------------------------------------------------------------------

void ClientController::handleNetUpdateTransform(const MessageQueueDataTransform& message)
{

	//-- verify the object-to-parent position is within sane limits.
	TerrainObject const *const terrain = TerrainObject::getInstance();
	if (terrain)
	{
		// It's okay if this object is contained and the position is not in world space.
		// This is still a good upper-bound sanity check on the value.
		Vector const newPosition = message.getTransform ().getPosition_p ();
		if (!terrain->isWithinTerrainBoundaries (newPosition))
		{
			WARNING (true, ("handleNetUpdateTransform(): object id=[%s] received invalid objectToParent position x=[%0.2f], y=[%.2f], z=[%0.2f], ignoring.", getOwner () ? getOwner ()->getNetworkId( ).getValueString ().c_str () : "<null owner>", newPosition.x, newPosition.y, newPosition.z));
			return;
		}
	}

	// transfer from cell to world
	if (getOwner ()->getAttachedTo() != NULL)
		getOwner ()->setParentCell(CellProperty::getWorldCellProperty());

	CellProperty::setPortalTransitionsEnabled(false);
#ifdef _DEBUG
		IGNORE_RETURN(message.getTransform().validate());
#endif
		getOwner ()->setTransform_o2p(message.getTransform());
	CellProperty::setPortalTransitionsEnabled(true);
}

//-----------------------------------------------------------------------

void ClientController::handleNetUpdateTransformWithParent(const MessageQueueDataTransformWithParent& message)
{
#ifdef _DEBUG
	//-- verify that we did not receive the same transform
	if (message.getParent () == getOwner ()->getParentCell ()->getOwner ().getNetworkId () && message.getTransform () == getOwner ()->getTransform_o2p ())
		DEBUG_WARNING (true, ("ClientController::handleNetUpdateTransformWithParent(): [%s] received nutwp with identical cell and transform", getOwner ()->getNetworkId ().getValueString ().c_str ()));
#endif

	if (message.getParent () != NetworkId::cms_invalid)
	{
		// Validate that the transform's position within the cell is within valid cell coordinates.
		Vector const newPosition = message.getTransform ().getPosition_p ();
		float const maxValidCellCoordinate = CellProperty::getMaximumValidCellSpaceCoordinate();

		if ((abs(newPosition.x) > maxValidCellCoordinate) || (abs(newPosition.y) > maxValidCellCoordinate) || (abs(newPosition.z) > maxValidCellCoordinate))
		{
			WARNING (true, ("handleNetUpdateTransformWithParent(): object id=[%s] received invalid objectToParent position x=[%0.2f], y=[%0.2f], z=[%0.2f], ignoring.", getOwner () ? getOwner ()->getNetworkId( ).getValueString ().c_str () : "<null owner>", newPosition.x, newPosition.y, newPosition.z));
			return;
		}
	}
	else
	{
		//-- verify the object-to-parent position is within sane limits.
		TerrainObject const *const terrain = TerrainObject::getInstance();
		if (terrain)
		{
			// It's okay if this object is contained and the position is not in world space.
			// This is still a good upper-bound sanity check on the value.
			Vector const newPosition = message.getTransform ().getPosition_p ();
			if (!terrain->isWithinTerrainBoundaries (newPosition))
			{
				WARNING (true, ("handleNetUpdateTransformWithParent(): object id=[%s] received invalid objectToParent position x=[%0.2f], y=[%0.2f], z=[%0.2f], ignoring.", getOwner () ? getOwner ()->getNetworkId( ).getValueString ().c_str () : "<null owner>", newPosition.x, newPosition.y, newPosition.z));
				return;
			}
		}
	}

	// check for transfer from world or another cell
	bool valid = true;
	const NetworkId &cellNetworkId = message.getParent();
	if (getOwner ()->getAttachedTo() == NULL || getOwner ()->getAttachedTo()->getNetworkId() != cellNetworkId)
	{
		Object *cellObject = NetworkIdManager::getObjectById(cellNetworkId);
		if (cellObject)
		{
			CellProperty *cellProperty = cellObject->getCellProperty();
			NOT_NULL(cellProperty);
			getOwner ()->setParentCell(cellProperty);
		}
		else
		{
			valid = false;
			DEBUG_WARNING (true, ("ClientController::handleNetUpdateTransformWithParent(): object %s [%s] cannot be put into non-existent cell %s", getOwner ()->getNetworkId ().getValueString ().c_str (), getOwner ()->getObjectTemplateName (), cellNetworkId.getValueString ().c_str ()));
		}
	}

	if (valid)
	{
		CellProperty::setPortalTransitionsEnabled(false);
#ifdef _DEBUG
			IGNORE_RETURN(message.getTransform().validate());
#endif
			getOwner ()->setTransform_o2p(message.getTransform());
		CellProperty::setPortalTransitionsEnabled(true);
	}
}

//-----------------------------------------------------------------------

void ClientController::handleMessage (const int message, const float value, const MessageQueue::Data* const data, const uint32 flags)
{
	switch(message)
	{
	case CM_combatAction:
		{
			const MessageQueueCombatAction *combatActionMessage = safe_cast<const MessageQueueCombatAction *>(data);
			if (combatActionMessage)
			{
				handleCombatActionMessage(*combatActionMessage);
				
			}
			else
				DEBUG_WARNING(true, ("received CM_combatAction message, but MessageQueueCombatAction-casted data is NULL."));
		}
		break;

	case CM_combatSpam:
		{
			const MessageQueueCombatSpam * const msg = dynamic_cast<const MessageQueueCombatSpam *>(data);
			NOT_NULL (msg);
			if (msg) //lint !e774 //true
				CuiCombatManager::processCombatSpam (*msg);
		}
		break;

	case CM_draftSchematicsMessage:
		{
			const MessageQueueDraftSchematics * const draftSchematicsMessage = dynamic_cast<const MessageQueueDraftSchematics *>(data);
			NOT_NULL(draftSchematicsMessage);

			CuiCraftManager::receiveDraftSchematics (*draftSchematicsMessage);
		}
		break;
	case CM_draftSlotsMessage:
		{
			const MessageQueueDraftSlots * const draftSlotsMessage = dynamic_cast<const MessageQueueDraftSlots *>(data);
			NOT_NULL(draftSlotsMessage);

			CuiCraftManager::receiveDraftSlots (*draftSlotsMessage);
		}
		break;

	case CM_craftingResult:
		{
			const MessageQueueGenericIntResponse * const msg = dynamic_cast<const MessageQueueGenericIntResponse *>(data);
			NOT_NULL(msg);
			CuiCraftManager::receiveGenericResponse (*msg);
		}
		break;

	case CM_nextCraftingStageResult:
		{
			const MessageQueueGenericIntResponse * const msg = dynamic_cast<const MessageQueueGenericIntResponse *>(data);
			NOT_NULL(msg);
			CuiCraftManager::receiveNextCraftingStageResult (*msg);
		}
		break;

	case CM_experimentResult:
		{
			const MessageQueueGenericIntResponse * const resultMessage = dynamic_cast<const MessageQueueGenericIntResponse *>(data);
			NOT_NULL(resultMessage);
			CuiCraftManager::receiveExperimentResult (*resultMessage);
		}
		break;

	case CM_craftingSessionEnded:
		{
			typedef MessageQueueGenericValueType<bool> MsgType;

			const MsgType * const resultMessage = dynamic_cast<const MsgType *>(data);
			NOT_NULL(resultMessage);
			CuiCraftManager::receiveSessionEnded (resultMessage->getValue ());
		}
		break;

	case CM_clientResourceHarvesterEmptyHopperResponse:
		{
			const MessageQueueGenericResponse * const msg = dynamic_cast<const MessageQueueGenericResponse *>(data);
			NOT_NULL(msg);
			CuiResourceManager::receiveGenericResponse (*msg);
		}
		break;

	case CM_opponentInfo:
		// We no longer do anything with this.
		// @todo remove oppoenent info message.
		break;

	case CM_combatAimToClient:
		{
			// @todo what the hell is going on here? :)
			//	const MessageQueueNetworkId * const msg = dynamic_cast<const MessageQueueNetworkId *>(data);
			//NOT_NULL (msg);
			// msg->getNetworkId() is the id of the object that is aiming
		}
		break;

	case CM_showCombatText:
		{
			const MessageQueueShowCombatText *const msg = safe_cast<const MessageQueueShowCombatText*> (data);
			if (data)
			{
				//-- Get the ClientObject for the emitter network id.
				ClientObject *const clientObject = safe_cast<ClientObject*> (NetworkIdManager::getObjectById (msg->getDefenderId ()));

				if (!clientObject)
					DEBUG_REPORT_LOG(true, ("ClientController: Cannot handle CM_showFlyText because the emitter object is not on this client.\n"));
				else
				{
					const float fadeTime = 3.0f;
					VectorArgb color = PackedRgb (static_cast<uint8> (msg->getRed ()), static_cast<uint8> (msg->getGreen ()), static_cast<uint8> (msg->getBlue ())).convert ();

					Unicode::String str;
					StringId const & id = msg->getOutputTextId ();
					if(id.isValid())
					{
						str = id.localize ();
					}
					else
					{
						Unicode::String const & oob = msg->getOutputTextOOB ();
						IGNORE_RETURN(ProsePackageManagerClient::appendAllProsePackages (oob, str));
					}

					const NetworkId &attackerId = msg->getAttackerId ();
					const NetworkId &defenderId = msg->getDefenderId ();
					CuiCombatManager::addDeferredCombatAction(attackerId, defenderId, str, fadeTime, color, msg->getScale (), true);
				}
			}
		}
		break;

	case CM_showFlyText:
		{
			const MessageQueueShowFlyText *const msg = safe_cast<const MessageQueueShowFlyText*> (data);
			if (data)
			{
				//-- Get the ClientObject for the emitter network id.
				ClientObject *const clientObject = safe_cast<ClientObject*> (NetworkIdManager::getObjectById (msg->getEmitterId ()));

				if (!clientObject)
					DEBUG_REPORT_LOG(true, ("ClientController: Cannot handle CM_showFlyText because the emitter object is not on this client.\n"));
				else
				{
					Unicode::String str;
					StringId const & id = msg->getOutputTextId ();
					if(id.isValid())
					{
						str = id.localize ();
					}
					else
					{
						Unicode::String const & oob = msg->getOutputTextOOB ();
						IGNORE_RETURN(ProsePackageManagerClient::appendAllProsePackages (oob, str));
					}
					
					if((msg->getFlags() & MessageQueueShowFlyText::F_isDamageFromPlayer) ||
						(msg->getFlags() & MessageQueueShowFlyText::F_isDot) ||
						(msg->getFlags() & MessageQueueShowFlyText::F_isBleed))
					{
						int val;
						sscanf(Unicode::wideToNarrow(str).c_str(), "%d", &val);
						
						if(val != 0)
							CuiCombatManager::addDamageDone(abs(val));
					}

					if( (msg->getFlags() & MessageQueueShowFlyText::F_isDamageFromPlayer) && !CuiCombatManager::getShowDamageOverHeadOthers())
						return;
					if( (msg->getFlags() & MessageQueueShowFlyText::F_isSnare) && !CuiCombatManager::getShowDamageSnare())
						return;
					if( (msg->getFlags() & MessageQueueShowFlyText::F_isGlancingBlow) && !CuiCombatManager::getShowDamageGlancingBlow())
						return;
					if( (msg->getFlags() & MessageQueueShowFlyText::F_isCriticalHit) && !CuiCombatManager::getShowDamageCriticalHit())
						return;
					if( (msg->getFlags() & MessageQueueShowFlyText::F_isLucky) && !CuiCombatManager::getShowDamageLucky())
						return;
					if( (msg->getFlags() & MessageQueueShowFlyText::F_isDot) && !CuiCombatManager::getShowDamageDot())
						return;
					if( (msg->getFlags() & MessageQueueShowFlyText::F_isBleed) && !CuiCombatManager::getShowDamageBleed())
						return;
					if( (msg->getFlags() & MessageQueueShowFlyText::F_isHeal) && !CuiCombatManager::getShowDamageHeal())
						return;
					if( (msg->getFlags() & MessageQueueShowFlyText::F_isFreeshot) && !CuiCombatManager::getShowDamageFreeshot())
						return;


					const float fadeTime = 1.0f;
					const VectorArgb & color = PackedRgb (static_cast<uint8> (msg->getRed ()), static_cast<uint8> (msg->getGreen ()), static_cast<uint8> (msg->getBlue ())).convert ();

					IGNORE_RETURN(clientObject->addFlyText(str, fadeTime, color, msg->getScale (), CuiTextManagerTextEnqueueInfo::TW_starwars));

					if ((msg->getFlags () & static_cast<int>(MessageQueueShowFlyText::F_showInChatBox)))
					{
						CuiSystemMessageManager::sendFakeSystemMessage (str, true);
					}
				}
			}
		}
		break;

	case CM_objectMenuDirty:
		CuiRadialMenuManager::setObjectMenuDirty (getOwner ()->getNetworkId ());	
		break;

	case CM_attributesDirty:
		ObjectAttributeManager::setObjectAttributesDirty (getOwner ()->getNetworkId ());	
		break;

	case CM_animationAction:
		{
			//-- Get the action name.
			const MessageQueueString * const msg = NON_NULL (safe_cast<const MessageQueueString *>(data));
			const std::string&         actionName = msg->getString ();

			//-- Get the owner Object.
			Object *const ownerObject = getOwner ();
			NOT_NULL (ownerObject);

			//-- Get the Object's skeletal Appearance.
			SkeletalAppearance2 *const appearance = dynamic_cast<SkeletalAppearance2*> (ownerObject->getAppearance ());
			if (!appearance)
				return;

			//-- Play the action.
			int   animationId;
			bool  animationIsAdd;
			appearance->getAnimationResolver().playAction(CrcLowerString(actionName.c_str()), animationId, animationIsAdd, NULL);

			return;
		}

	default:
		NetworkController::handleMessage(message, value, data, flags);
		break;
	}
}

// ----------------------------------------------------------------------

void ClientController::doClientHandleNetUpdateTransform (const MessageQueueDataTransform& message)
{
	// Call this class specific implementation of handleNetUpdateTransform.
	// -TRF- added this for mounts --- I needed to be able to redirect a call
	// on the player to this function on the player's mount without going
	// through RemoteCreatureController.
	ClientController::handleNetUpdateTransform (message);
}

// ----------------------------------------------------------------------

void ClientController::doClientHandleNetUpdateTransformWithParent (const MessageQueueDataTransformWithParent& message)
{
	// Call this class specific implementation of handleNetUpdateTransform.
	// -TRF- added this for mounts --- I needed to be able to redirect a call
	// on the player to this function on the player's mount without going
	// through RemoteCreatureController.
	ClientController::handleNetUpdateTransformWithParent (message);
}

// ======================================================================

float ClientController::realAlter(float deltaTime)
{
	DEBUG_FATAL(!m_initialized, ("ClientController::realAlter: controller for object [id=%s, template=%s] has not had endBaselines called on it", getOwner()->getNetworkId().getValueString().c_str(), getOwner()->getObjectTemplateName()));

	int message;
	float value;
	MessageQueue::Data* data;
	uint32 flags;

	Object *target = getOwner();

	int const messageCount = getNumberOfMessages();
	for (int i = 0; i < messageCount; ++i)
	{
		getMessage(i, &message, &value, &data, &flags);
		if (message)
		{
			bool processMessage = false;

#if PRODUCTION == 0
			if (Game::getSinglePlayer())
				processMessage = true;
			else
#endif
			{
				// if the message is for an authoritative/proxy, make sure we are
				// the right type
				if ((flags & GameControllerMessageFlags::DEST_AUTH_CLIENT) && target->isAuthoritative())
					processMessage = true;

				if ((flags & GameControllerMessageFlags::DEST_PROXY_CLIENT))
				{
					if(!target->isAuthoritative())
					{
						processMessage = true;
					}
					else
					{
						if(target->asClientObject() && target->asClientObject()->isClientCached())
						{
							processMessage = true;
						}
					}
				}
			}

			if (processMessage)
				handleMessage(message, value, data, flags);
		}
	}

	// Controller makes sure we get an update when there are messages added to the
	// message queue.  There's no reason to force an alter every frame here.
	return NetworkController::realAlter(deltaTime);
}

//-----------------------------------------------------------------------

float ClientController::getCurrentSpeed () const
{
	return 0.f;
}

//-----------------------------------------------------------------------
/**
 * Send a controller message to wherever it is supposed to go.
 *
 * @param msg		the message to send
 */
void ClientController::sendControllerMessage(ObjControllerMessage const &msg)
{
	uint32 const destFlags = (msg.getFlags() & GameControllerMessageFlags::DESTINATIONS);
	bool const reliable = ((msg.getFlags() & GameControllerMessageFlags::RELIABLE) > 0);

	if (destFlags & (  GameControllerMessageFlags::DEST_AUTH_SERVER
	                 | GameControllerMessageFlags::DEST_PROXY_SERVER
	                 | GameControllerMessageFlags::DEST_SERVER))
	{
		if (GameNetwork::getConnectionServerConnection())
			GameNetwork::send(msg, reliable);
	}
}

// ======================================================================
