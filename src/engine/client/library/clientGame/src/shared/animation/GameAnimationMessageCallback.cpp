// ======================================================================
//
// GameAnimationMessageCallback.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/GameAnimationMessageCallback.h"

#include "clientGame/ClientCombatActionInfo.h"
#include "clientGame/ClientEventManager.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/CombatEffectsManager.h"
#include "clientGame/Game.h"
#include "clientObject/InteriorEnvironmentBlock.h"
#include "clientObject/InteriorEnvironmentBlockManager.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/TransformAnimationResolver.h"
#include "clientUserInterface/CuiCombatManager.h"
#include "clientUserInterface/CuiPreferences.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/Floor.h"
#include "sharedCollision/Footprint.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedGame/SharedTerrainSurfaceObjectTemplate.h"
#include "sharedObject/CachedNetworkId.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/Object.h"
#include "sharedObject/PortalProperty.h"
#include "sharedObject/SlotId.h"
#include "sharedObject/SlotIdManager.h"
#include "sharedObject/SlottedContainer.h"
#include "sharedTerrain/TerrainObject.h"
#include "sharedUtility/FileName.h"

#include <string>

// ======================================================================

namespace GameAnimationMessageCallbackNamespace
{
	const std::string              cms_empty("");

	const char *const              cms_eventMessageNamePrefix                = "event_";
	const size_t                   cms_eventMessageNamePrefixLength          = 6;

	const char *const              cms_hardpointEventMessageNamePrefix       = "hpevent_";
	const size_t                   cms_hardpointEventMessageNamePrefixLength = 8;

	const char *const              cms_footstepEventName                     = "footstep";

	const char *const              cms_swingMessageNamePrefix                = "swing_";
	const size_t                   cms_swingMessageNamePrefixLength          = 6;
	const ConstCharCrcLowerString  cms_swingEventName("swing");

	const char *const              cms_unarmedSwingMessageNamePrefix         = "uswing_";
	const size_t                   cms_unarmedSwingMessageNamePrefixLength   = 7;

	const char *const              cms_hitMessageNamePrefix                  = "hit_";
	const size_t                   cms_hitMessageNamePrefixLength            = 4;
	const ConstCharCrcLowerString  cms_hitEventName("hit");

	const char *const              cms_unarmedHitMessageNamePrefix           = "uhit_";
	const size_t                   cms_unarmedHitMessageNamePrefixLength     = 5;

	const char* const              cms_waterNameSuffix = "water";
	const char* const              cms_otherNameSuffix = "other";
	const char* const              cms_metalNameSuffix = "metal";
	const char* const              cms_stoneNameSuffix = "stone";
	const char* const              cms_woodNameSuffix  = "wood";

	const char *const              cms_attackHitEventText                     = "attack";
	const char *const              cms_hitEventText                           = "hit";


	bool                           ms_logAnimationEvents;
	bool                           ms_ignoreFootstepAnimationEvents;

	const ConstCharCrcLowerString   cs_defaultWeaponSlotName("default_weapon");
	SlotId                         s_defaultWeaponSlotId(SlotId::invalid);

	const ConstCharCrcLowerString  cs_holdRightSlotName("hold_r");
	SlotId                         s_holdRightSlotId(SlotId::invalid);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	const char* getSurfaceType(const Object* const object);
	bool        getEmbeddedNameWithTrail(const char *sourceBuffer, char *destBuffer, size_t bufferLength, const char *& endEmbeddedName);

	Object     *getDefaultWeaponObject(Object *holder);
	Object     *getWeaponObject(Object *holder);
}

using namespace GameAnimationMessageCallbackNamespace;

// ======================================================================

const char* GameAnimationMessageCallbackNamespace::getSurfaceType(const Object* const object)
{
	//-- is the object not in a cell?
	if (object->getParentCell() == CellProperty::getWorldCellProperty())
	{
		//-- is the object on the terrain?
		const CollisionProperty* const collision = object->getCollisionProperty ();
		const bool isOnSolidFloor = collision && collision->getFootprint () && collision->getFootprint ()->isOnSolidFloor ();

		if (isOnSolidFloor && collision->getFootprint ()->getSolidContact ())
		{
			const Object* const solidObject = collision->getFootprint ()->getSolidContact ()->getFloor ()->getOwner ();
			if (solidObject)
			{
				//-- grab the surfacetype from the object template
				const SharedObjectTemplate* const objectTemplate = safe_cast <const SharedObjectTemplate*> (solidObject->getObjectTemplate ());

				if (objectTemplate)
				{
					switch (objectTemplate->getSurfaceType ())
					{
					case SharedObjectTemplate::ST_other:
						return cms_otherNameSuffix;

					case SharedObjectTemplate::ST_metal:
						return cms_metalNameSuffix;

					case SharedObjectTemplate::ST_stone:
						return cms_stoneNameSuffix;

					case SharedObjectTemplate::ST_wood:
						return cms_woodNameSuffix;
					}
				}
			}
		}
		else
		{
			const TerrainObject* const terrainObject = TerrainObject::getInstance ();
			if (terrainObject)
			{
				//-- if we're below water, the surface type is water
				if (terrainObject->isBelowWater (object->getPosition_w ()))
					return cms_waterNameSuffix;

				//-- grab the surface type from the object template
				const SharedTerrainSurfaceObjectTemplate* const objectTemplate = safe_cast<const SharedTerrainSurfaceObjectTemplate*> (terrainObject->getSurfaceProperties (object->getPosition_w ()));
				if (objectTemplate)
					return objectTemplate->getSurfaceType ().c_str ();
			}
		}
	}
	else
	{
		//-- object is in a cell, so get the cell property's footstep id
		const CellProperty* const cellProperty = object->getParentCell ();
		const char* const cellName = cellProperty->getCellName ();
		const PortalProperty* const portalProperty = cellProperty->getPortalProperty ();
		const char* const pobShortName = portalProperty->getPobShortName ();

		return InteriorEnvironmentBlockManager::getEnvironmentBlock (pobShortName, cellName)->getSurfaceType ();
	}

	return 0;
}

// ----------------------------------------------------------------------

bool GameAnimationMessageCallbackNamespace::getEmbeddedNameWithTrail(const char *sourceBuffer, char *destBuffer, size_t bufferLength, const char *& endEmbeddedName)
{
	NOT_NULL(sourceBuffer);
	NOT_NULL(destBuffer);
	DEBUG_FATAL(bufferLength < 1, ("dest buffer is too small."));

	// Find end of hardpoint name.
	endEmbeddedName = strrchr(sourceBuffer, '_');
	if (!endEmbeddedName)
		return false;

	// Copy hardpoint name to dest buffer.
	strncpy(destBuffer, sourceBuffer, bufferLength - 1);
	destBuffer[bufferLength - 1] = '\0';

	// Cut off the hardpoint name at the end of the string.

	// Find the end of the embedded name, truncate the string to that point.
	const size_t embeddedNameLength = static_cast<size_t>(endEmbeddedName - sourceBuffer);
	if (embeddedNameLength < bufferLength)
		destBuffer[embeddedNameLength] = 0;

	return true;
}

// ----------------------------------------------------------------------

Object *GameAnimationMessageCallbackNamespace::getDefaultWeaponObject(Object *holder)
{
	NOT_NULL(holder);

	//-- Get the equipment container for the holder.
	SlottedContainer *const container = ContainerInterface::getSlottedContainer(*holder);
	if (!container)
	{
		DEBUG_WARNING(true, ("failed to get slotted container for object [id=%s].", holder->getNetworkId().getValueString().c_str()));
		return 0;
	}

	//-- Get the object in the default weapon slot.
	Container::ContainerErrorCode tmp = Container::CEC_Success;
	return container->getObjectInSlot(s_defaultWeaponSlotId, tmp).getObject();
}

// ----------------------------------------------------------------------

Object *GameAnimationMessageCallbackNamespace::getWeaponObject(Object *holder)
{
	NOT_NULL(holder);

	//-- Get the equipment container for the holder.
	SlottedContainer *const container = ContainerInterface::getSlottedContainer(*holder);
	if (!container)
	{
		DEBUG_WARNING(true, ("failed to get slotted container for object [id=%s].", holder->getNetworkId().getValueString().c_str()));
		return 0;
	}

	//-- Get the object in the hold right slot.
	Container::ContainerErrorCode tmp = Container::CEC_Success;
	Object *const holdRightObject = container->getObjectInSlot(s_holdRightSlotId, tmp).getObject();
	if (holdRightObject)
		return holdRightObject;
	else
	{
		// No object in the right hand.  Return the default weapon.
		return getDefaultWeaponObject(holder);
	}
}

// ======================================================================
// class GameAnimationMessageCallback: public static member functions
// ======================================================================

void GameAnimationMessageCallback::install()
{
	InstallTimer const installTimer("GameAnimationMessageCallback::install");

	DebugFlags::registerFlag(ms_logAnimationEvents, "ClientGame/Character", "logAnimationEvents");
	DebugFlags::registerFlag(ms_ignoreFootstepAnimationEvents, "ClientGame/Character", "ignoreFootstepAnimationEvents");
	
	s_defaultWeaponSlotId = SlotIdManager::findSlotId(cs_defaultWeaponSlotName);
	s_holdRightSlotId     = SlotIdManager::findSlotId(cs_holdRightSlotName);
}

// ----------------------------------------------------------------------

void GameAnimationMessageCallback::addCallback(Object &object)
{
	//-- Get skeletal appearance.
	SkeletalAppearance2 *const appearance = dynamic_cast<SkeletalAppearance2*>(object.getAppearance());
	if (!appearance)
		return;

	//-- Tell AnimationResolver to add the callback to all animation controllers.
	appearance->getAnimationResolver().addAnimationCallback(animationMessageToClientEventCallback, &object);
}

// ----------------------------------------------------------------------

void GameAnimationMessageCallback::animationMessageToClientEventCallback(void *context, const CrcLowerString &animationMessageName, const TransformAnimationController * /* controller */)
{
	//-- Recover source Object from context.
	NOT_NULL(context);
	Object *const object = static_cast<Object*>(context);

	//-- Get info on message name.
	const char *const cMessageName      = animationMessageName.getString();
	const size_t      messageNameLength = strlen(cMessageName);

	bool hitEvent    = false;
	bool attackEvent = false;
	bool isUnarmed   = false;

	char hardpointName[128] = "";

	//-- If the animation message name starts with "event_", signal a non-hardpoint-style event.
	if ((messageNameLength > cms_eventMessageNamePrefixLength) && (strncmp(cMessageName, cms_eventMessageNamePrefix, cms_eventMessageNamePrefixLength) == 0))
	{
		// FORMAT: event_<eventName>

		//-- Signal client event with same name as animation message name.

		//-- Is this a footstep event?
		if (_stricmp(cms_footstepEventName, cMessageName + cms_eventMessageNamePrefixLength) == 0)
		{
			const char* const surfaceType = getSurfaceType (object);

			char buffer [128];
			strcpy (buffer, cms_footstepEventName);

			if (surfaceType)
			{
				strcat (buffer, "_");
				strcat (buffer, surfaceType);
			}

			DEBUG_REPORT_LOG(!ms_ignoreFootstepAnimationEvents && ms_logAnimationEvents, ("ANIM: %s firing event [%s].\n", object ? object->getObjectTemplateName() : "invalid object", cMessageName + cms_eventMessageNamePrefixLength));
			IGNORE_RETURN(ClientEventManager::playEvent(CrcLowerString(buffer), object, CrcLowerString::empty));
		}
		else
		{
			if (strstr(cMessageName, cms_attackHitEventText))
			{
				hitEvent = true;
			}

			DEBUG_REPORT_LOG(ms_logAnimationEvents, ("ANIM: %s firing event [%s].\n", object ? object->getObjectTemplateName() : "invalid object", cMessageName + cms_eventMessageNamePrefixLength));
			IGNORE_RETURN(ClientEventManager::playEvent(CrcLowerString(cMessageName + cms_eventMessageNamePrefixLength), object, CrcLowerString::empty));
		}
	}
	else if ((messageNameLength > cms_hardpointEventMessageNamePrefixLength) && (strncmp(cMessageName, cms_hardpointEventMessageNamePrefix, cms_hardpointEventMessageNamePrefixLength) == 0))
	{
		//-- The animation message name starts with "hpevent_", signal a hardpoint-style event.
		// FORMAT: hpevent_<hardpointName>_<eventName>

		//-- Build hardpoint name.
		const char *endHardpointName;

		const bool gotHardpointName = getEmbeddedNameWithTrail(cMessageName + cms_hardpointEventMessageNamePrefixLength, hardpointName, sizeof(hardpointName), endHardpointName);
		if (!gotHardpointName)
		{
			DEBUG_WARNING(true, ("malformed animation message name [%s]: missing '_' separating event type from hardpoint.", cMessageName));
			return;
		}

		const CrcLowerString hardpointName(hardpointName);

		//-- Signal client event with same name as animation message name.

		//-- Is this a footstep event?
		if (_stricmp(cms_footstepEventName, endHardpointName + 1) == 0)
		{
			const char* const surfaceType = getSurfaceType (object);

			char buffer [128];
			strcpy (buffer, cms_footstepEventName);

			if (surfaceType)
			{
				strcat (buffer, "_");
				strcat (buffer, surfaceType);
			}

			DEBUG_REPORT_LOG(!ms_ignoreFootstepAnimationEvents && ms_logAnimationEvents, ("ANIM: %s firing hardpoint event [%s] at hardpoint [%s].\n", object ? object->getObjectTemplateName() : "invalid object", endHardpointName + 1, hardpointName));
			IGNORE_RETURN(ClientEventManager::playEvent(CrcLowerString(buffer), object, hardpointName));
		}
		else
		{
			if ((strstr(cMessageName, cms_attackHitEventText)) || (strstr(cMessageName, cms_hitEventText)))
			{
				hitEvent = true;
			}

			DEBUG_REPORT_LOG(ms_logAnimationEvents, ("ANIM: %s firing hardpoint event [%s] at hardpoint [%s].\n", object ? object->getObjectTemplateName() : "invalid object", endHardpointName + 1, hardpointName));
			IGNORE_RETURN(ClientEventManager::playEvent(CrcLowerString(endHardpointName + 1), object, hardpointName));
		}
	}
	else if ((messageNameLength > cms_swingMessageNamePrefixLength) && (strncmp(cMessageName, cms_swingMessageNamePrefix, cms_swingMessageNamePrefixLength) == 0))
	{
		// The animation message names starts with "swing_", signal a "swing" event on the weapon.
		// FORMAT: swing_<hardpointName>

		//-- Get the name of the emitter hardpoint.

		strncpy(hardpointName, cMessageName + cms_swingMessageNamePrefixLength, sizeof(hardpointName) - 1);
		hardpointName[sizeof(hardpointName) - 1] = '\0';

#ifdef _DEBUG
		if (strlen(hardpointName) < 1)
		{
			DEBUG_WARNING(true, ("invalid animation message: missing hardpoint information, message = [%s].", cMessageName));
			return;
		}
#endif

		attackEvent = true;
	}
	else if ((messageNameLength > cms_unarmedSwingMessageNamePrefixLength) && (strncmp(cMessageName, cms_unarmedSwingMessageNamePrefix, cms_unarmedSwingMessageNamePrefixLength) == 0))
	{
		// The animation message names starts with "uswing_", signal a "swing" event on the default (unarmed) weapon.
		// FORMAT: uswing_<hardpointName>

		//-- Get the name of the emitter hardpoint.
		strncpy(hardpointName, cMessageName + cms_unarmedSwingMessageNamePrefixLength, sizeof(hardpointName) - 1);
		hardpointName[sizeof(hardpointName) - 1] = '\0';

#ifdef _DEBUG
		if (strlen(hardpointName) < 1)
		{
			DEBUG_WARNING(true, ("invalid animation message: missing hardpoint information, message = [%s].", cMessageName));
			return;
		}
#endif

		isUnarmed = true;
		attackEvent = true;
	}
	else if ((messageNameLength > cms_hitMessageNamePrefixLength) && (strncmp(cMessageName, cms_hitMessageNamePrefix, cms_hitMessageNamePrefixLength) == 0))
	{
		// The animation message names starts with "hit_", signal a "hit" event on the weapon if this creature is in combat as an attacker
		// and if the defender is considered hit (hit, block, or redirect).
		//
		// FORMAT: hit_<hardpointName>

		//-- Get the name of the emitter hardpoint.
		strncpy(hardpointName, cMessageName + cms_hitMessageNamePrefixLength, sizeof(hardpointName) - 1);
		hardpointName[sizeof(hardpointName) - 1] = '\0';

#ifdef _DEBUG
		if (strlen(hardpointName) < 1)
		{
			DEBUG_WARNING(true, ("invalid animation message: missing hardpoint information, message = [%s].", cMessageName));
			return;
		}
#endif

		hitEvent = true;
	}
	else if ((messageNameLength > cms_unarmedHitMessageNamePrefixLength) && (strncmp(cMessageName, cms_unarmedHitMessageNamePrefix, cms_unarmedHitMessageNamePrefixLength) == 0))
	{
		// The animation message names starts with "uhit_", signal a "hit" event on the default (unarmed) weapon if this creature is in combat as an attacker
		// and if the defender is considered hit (hit, block, or redirect).
		//
		// FORMAT: uhit_<hardpointName>

		//-- Get the name of the emitter hardpoint.
		strncpy(hardpointName, cMessageName + cms_unarmedHitMessageNamePrefixLength, sizeof(hardpointName) - 1);
		hardpointName[sizeof(hardpointName) - 1] = '\0';

#ifdef _DEBUG
		if (strlen(hardpointName) < 1)
		{
			DEBUG_WARNING(true, ("invalid animation message: missing hardpoint information, message = [%s].", cMessageName));
			return;
		}
#endif

		isUnarmed = true;
		hitEvent = true;
	}

	if (hitEvent)
	{
		//-- Check if attack is a hit.

		// Get the defender disposition value for the combatant defending against this creature.
		CreatureObject *const creature = safe_cast<CreatureObject*>(object);
		NOT_NULL(creature);

		int  defenderDispositionValue = 0;
		int  attackNameCrc = 0;
		bool isInCombat               = false;
		bool isAttacker               = false;

		creature->getDefenderDisposition(isInCombat, isAttacker, defenderDispositionValue, attackNameCrc);

		// Check the defender disposition against the set we consider to be a successful hit.
		const ClientCombatActionInfo::DefenderDefense  defense = static_cast<ClientCombatActionInfo::DefenderDefense>(defenderDispositionValue);
		const bool  attackerDidHit = isInCombat && isAttacker && ((defense == ClientCombatActionInfo::DD_hit) || (defense == ClientCombatActionInfo::DD_block) || (defense == ClientCombatActionInfo::DD_redirect));

		Object * defenderObject = NULL;
		if (!Game::isSpace() && CuiPreferences::getAutoAimToggle())
			defenderObject = creature->getIntendedTarget().getObject();
		else
			defenderObject = creature->getLookAtTarget().getObject();

		//-- Skip the rest of the processing if the creature didn't hit its defender.
		if (!attackerDidHit)
		{
			DEBUG_REPORT_LOG(ms_logAnimationEvents, ("ANIM: %s ignoring hit event because attack wasn't a hit.\n", object ? object->getObjectTemplateName() : "invalid object"));
		}
		else
		{
			if (attackerDidHit)
			{
				//-- Get the weapon object.
				Object * weaponObject = isUnarmed ? getDefaultWeaponObject(object) : getWeaponObject(object);
				if (!weaponObject)
				{
					DEBUG_WARNING(true, ("failed to get a weapon object for object %s [id=%s].", object ? object->getObjectTemplateName() : "invalid object", object->getNetworkId().getValueString().c_str()));
					return;
				}

				//-- play any extra weapon/mode hit effects
				WeaponObject * objectAsWeaponObject = dynamic_cast<WeaponObject *>(weaponObject);
				if (objectAsWeaponObject && ( objectAsWeaponObject->isMeleeWeapon() || objectAsWeaponObject->isLightsaberWeapon() ) ) 
				{
					CombatEffectsManager::createMeleeHitEffect(*objectAsWeaponObject, CrcLowerString(hardpointName), defenderObject, attackNameCrc);
				}
			}

			//-- Play the swing event on the animation object looked up via the weapon object.
			DEBUG_REPORT_LOG(ms_logAnimationEvents, ("ANIM: %s processed %s hit event at hardpoint [%s].\n", object ? object->getObjectTemplateName() : "invalid object", (isUnarmed ? "unarmed" : "armed"), hardpointName));
		}

		// show any combat text for this combat action
		if (isAttacker && defenderObject)
		{
			CuiCombatManager::removeCompletedCombatAction(creature->getNetworkId(), defenderObject->getNetworkId());
		}
	}

	if (attackEvent)
	{
		//-- Get the weapon object.
		Object * weaponObject = isUnarmed ? getDefaultWeaponObject(object) : getWeaponObject(object);
		if (!weaponObject)
		{

			if (!isUnarmed)
				weaponObject = getDefaultWeaponObject(object);

			if (!weaponObject)
			{
				DEBUG_WARNING(true, ("failed to get a weapon object for object %s [id=%s].", object ? object->getObjectTemplateName() : "invalid object", object->getNetworkId().getValueString().c_str()));
				return;
			}
		}

		WeaponObject * objectAsWeaponObject = dynamic_cast<WeaponObject *>(weaponObject);
		if (objectAsWeaponObject && (objectAsWeaponObject->isMeleeWeapon() || objectAsWeaponObject->isLightsaberWeapon()) )
		{
			CombatEffectsManager::createMeleeSwingEffect(*objectAsWeaponObject);
		}

		//-- Play the swing event on the animation object looked up via the weapon object.
		DEBUG_REPORT_LOG(ms_logAnimationEvents, ("ANIM: %s processed %s swing event at hardpoint [%s].\n", object ? object->getObjectTemplateName() : "invalid object", (isUnarmed ? "unarmed" : "armed"), hardpointName));
	}
}

// ======================================================================
