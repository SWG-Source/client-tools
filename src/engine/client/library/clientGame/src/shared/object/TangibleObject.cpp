// ======================================================================
//
// TangibleObject.cpp
// Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/TangibleObject.h"

#include "clientGame/ClientCollisionProperty.h"
#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientDataFile.h"
#include "clientGame/ClientSynchronizedUi.h"
#include "clientGame/ClientTangibleObjectTemplate.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/Game.h"
#include "clientGame/ManufactureSchematicObject.h"
#include "clientGame/PlayerObject.h"
#include "clientGame/SlotRuleManager.h"
#include "clientGraphics/RenderWorld.h"
#include "clientParticle/ParticleEffectAppearance.h"
#include "clientParticle/ParticleEffectAppearanceTemplate.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientUserInterface/CuiObjectTextManager.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/DebugInfoManager.h"
#include "sharedFoundation/Production.h"
#include "sharedGame/GameObjectTypes.h"
#include "sharedGame/PvpData.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedGame/SharedStringIds.h"
#include "sharedMath/VectorArgb.h"
#include "sharedMath/Transform.h"
#include "sharedMathArchive/VectorArchive.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/MessageQueueSitOnObject.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/Container.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/CustomizationDataProperty.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/RotationDynamics.h"
#include "sharedObject/SlottedContainer.h"
#include "sharedUtility/Callback.h"

#include <map>
#include <vector>

// ======================================================================

//lint -e1734 // difficulty compiling safe_cast template

// ======================================================================

class TangibleObject::CraftingToolSyncUi : public ClientSynchronizedUi
{
public:
	CraftingToolSyncUi(TangibleObject & owner) :
	  ClientSynchronizedUi(owner)
	  {
		  addToUiPackage(m_prototype);
		  addToUiPackage(m_manfSchematic);
	  }

	  const CachedNetworkId & getPrototype(void) const
	  {
		  return m_prototype.get();
	  }

	  const CachedNetworkId & getManfSchematic(void) const
	  {
		  return m_manfSchematic.get();
	  }

private:

	CraftingToolSyncUi(const CraftingToolSyncUi & src);
	CraftingToolSyncUi & operator =(const CraftingToolSyncUi & src);

private:
	Archive::AutoDeltaVariable<CachedNetworkId> m_prototype;
	Archive::AutoDeltaVariable<CachedNetworkId> m_manfSchematic;
};

// ======================================================================

namespace TangibleObjectNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef std::vector<Object*>  ObjectVector;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	namespace Transceivers
	{
		MessageDispatch::Transceiver<const TangibleObject::Messages::DamageTaken::Payload &,  TangibleObject::Messages::DamageTaken>  damageTaken;
	}

	const ConstCharCrcLowerString  ms_meshVariableSetName("owner_mesh");
	const ConstCharCrcLowerString  ms_textureVariableSetName("owner_texture");

	const ConstCharCrcLowerString cs_chairHardpointName("foot");

	bool ms_useTestDamageLevel;
	bool ms_logChangedConditions;

	void remove ();

	const uint32 hash_combatTarget   = ConstCharCrcLowerString ("combattarget").getCrc ();
	const uint32 hash_combatUntarget = ConstCharCrcLowerString ("combatuntarget").getCrc ();

	std::string const & ms_debugInfoSectionName = "TangibleObject";
}

using namespace TangibleObjectNamespace;

// ======================================================================
// class TangibleObject: public static member functions
// ======================================================================

void TangibleObject::install()
{
	InstallTimer const installTimer("TangibleObject::install");

	DebugFlags::registerFlag (ms_useTestDamageLevel, "ClientGame/TangibleObject", "useTestDamageLevel");
	DebugFlags::registerFlag(ms_logChangedConditions, "ClientGame/TangibleObject", "logChangedConditions");
	ExitChain::add (TangibleObjectNamespace::remove, "TangibleObjectNamespace::remove");
}

//----------------------------------------------------------------------

void TangibleObjectNamespace::remove ()
{
	DebugFlags::unregisterFlag (ms_useTestDamageLevel);
	DebugFlags::unregisterFlag(ms_logChangedConditions);
}

// ----------------------------------------------------------------------

static void TestObjectAndChildrenForChair(Vector const searchPosition_p, Object &object, TangibleObject *&closestChair, float &closestDistanceSquared)
{
	//-- Test this object.
	ClientObject *clientObject = object.asClientObject();
	if (clientObject)
	{
		TangibleObject *const tangibleObject = clientObject->asTangibleObject();
		if (tangibleObject && tangibleObject->canSitOn())
		{
			float const distanceSquared = searchPosition_p.magnitudeBetweenSquared(tangibleObject->getPosition_p());
			if ((distanceSquared < closestDistanceSquared) && (distanceSquared < MessageQueueSitOnObject::cs_maximumChairRangeSquared))
			{
				closestDistanceSquared = distanceSquared;
				closestChair           = tangibleObject;
			}
		}
	}

	//-- Test children of this object.
	int const childCount = object.getNumberOfChildObjects();
	for (int i = 0; i < childCount; ++i)
	{
		Object *const childObject = object.getChildObject(i);
		if (childObject)
			TestObjectAndChildrenForChair(searchPosition_p, *childObject, closestChair, closestDistanceSquared);
	}
}

// ----------------------------------------------------------------------

TangibleObject *TangibleObject::findClosestInRangeChair(NetworkId const &searchCellId, Vector const &searchPosition_p)
{
	bool const isInWorldCell = (searchCellId == NetworkId::cms_invalid);

	if (isInWorldCell)
	{
		ObjectVector objectVector;

		// This creature is in the world cell. Use the ClientWorld
		// object-finding routines to find all the objects close to us.
		ClientWorld::findObjectsInRange(searchPosition_p, MessageQueueSitOnObject::cs_maximumChairRange, objectVector);

		TangibleObject *closestChair           = NULL;
		float           closestDistanceSquared = std::numeric_limits<float>::max();

		// NULL out anything that is not a chair.
		ObjectVector::iterator endIt = objectVector.end();
		for (ObjectVector::iterator it = objectVector.begin(); it != endIt; ++it)
		{
			if (*it)
				TestObjectAndChildrenForChair(searchPosition_p, *(*it), closestChair, closestDistanceSquared);
		}

		return closestChair;
	}
	else
	{
		// This creature is in another cell.  Go through all the objects in the
		// cell, finding the closest chair.
		Object *cellObject = NetworkIdManager::getObjectById(searchCellId);
		if (!cellObject)
		{
			DEBUG_WARNING(true, ("findClosestChairWithinRange(): specified chair parent cell id=[%s] could not be found on client.", searchCellId.getValueString().c_str()));
			return NULL;
		}

		// Get the cell property from the cell owner.  This really shouldn't be NULL.
		CellProperty *const cellProperty = cellObject->getCellProperty();
		if (!cellProperty)
		{
			DEBUG_WARNING(true, ("findClosestInRangeChair(): cellProperty for cell object was NULL, id=[%s].", searchCellId.getValueString().c_str()));
			return NULL;
		}

		TangibleObject *closestChair           = NULL;
		float           closestDistanceSquared = std::numeric_limits<float>::max();

		{
			ContainerIterator const endIt = cellProperty->end();
			for (ContainerIterator it = cellProperty->begin(); it != endIt; ++it)
			{
				if ((*it).getObject())
					TestObjectAndChildrenForChair(searchPosition_p, *((*it).getObject()), closestChair, closestDistanceSquared);
			}
		}

		//-- Also try the owner object's container property --- try to track down where the interior contents are located.
		Container *const cellObjectContainer = ContainerInterface::getContainer(*cellObject);
		if (cellObjectContainer)
		{
			ContainerIterator const endIt = cellObjectContainer->end();
			for (ContainerIterator it = cellObjectContainer->begin(); it != endIt; ++it)
			{
				if ((*it).getObject())
					TestObjectAndChildrenForChair(searchPosition_p, *((*it).getObject()), closestChair, closestDistanceSquared);
			}
		}

		//-- Also try the owner object.
		TestObjectAndChildrenForChair(searchPosition_p, *cellObject, closestChair, closestDistanceSquared);

		return closestChair;
	}
}

// ======================================================================
// class TangibleObject: public member functions
// ======================================================================

TangibleObject::TangibleObject(const SharedTangibleObjectTemplate* newTemplate) :
ClientObject             (newTemplate),
m_appearanceData         (),
m_damageTaken            (),
m_maxHitPoints           (),
m_components             (),
m_visible                (true),
m_inCombat				 (false),
m_count                  (0),
m_condition              (0),
m_accumulatedDamageTaken (0),
m_pvpFlags               (0),
m_pvpType                (0),
m_pvpFactionId           (0),
m_lastDamageLevel        (0.f),
m_testDamageLevel        (0.f),
m_lastOnOffStatus        (false),
m_isReceivingCallbacks   (false),
m_interestingAttachedObject(0),
m_untargettableOverride(false),
m_clientOnlyInteriorLayoutObjectList(0),
m_visabilityFlag(newTemplate->getClientVisabilityFlag()),
m_objectEffects(),
m_passiveRevealPlayerCharacter(),
m_mapColorOverride(0),
m_accessList(),
m_guildAccessList(),
m_effectsMap()
{
	NOT_NULL(newTemplate);
	m_appearanceData.setSourceObject (this);
	m_damageTaken.setSourceObject    (this);
	m_condition.setSourceObject      (this);
	m_maxHitPoints.setSourceObject   (this);

	addSharedVariable(m_pvpFactionId);
	addSharedVariable(m_pvpType);
	addSharedVariable(m_appearanceData);
	addSharedVariable(m_components);
	addSharedVariable(m_condition);
	addSharedVariable(m_count);
	addSharedVariable(m_damageTaken);
	addSharedVariable(m_maxHitPoints);
	addSharedVariable(m_visible);
	addSharedVariable_np(m_inCombat);
	addSharedVariable_np(m_passiveRevealPlayerCharacter);
	addSharedVariable_np(m_mapColorOverride);
	addSharedVariable_np(m_accessList);
	addSharedVariable_np(m_guildAccessList);
	addSharedVariable_np(m_effectsMap);

	m_effectsMap.setOnErase(this, &TangibleObject::OnObjectEffectErased);
	m_effectsMap.setOnInsert(this, &TangibleObject::OnObjectEffectInsert);
	m_effectsMap.setOnSet(this, &TangibleObject::OnObjectEffectModified);

	//-- set the default appearance.
	changeAppearance(*newTemplate);

	addProperty(*(new ClientCollisionProperty(*this)));

	if (Game::getSinglePlayer ())
		m_maxHitPoints = 1000;
}

//-----------------------------------------------------------------------

TangibleObject::~TangibleObject()
{
	//-- This must be the first line in the destructor to invalidate any watchers watching this object
	nullWatchers();

	if (isInWorld())
		removeFromWorld();

	if ( m_isReceivingCallbacks )
	{
		// We need to make sure the preferences are installed since we get destroyed
		// after the preferences get cleaned up when the game shuts down
		if ( CuiPreferences::isInstalled() )
		{
			// Stop observing client preference changes for displaying the interesting attached object
			CuiPreferences::getShowInterestingAppearanceCallback().detachReceiver(*this);
		}

		m_isReceivingCallbacks = false;
	}

	// -- Clean up any Object Effects
	{
		std::vector<std::string> toRemove;
		std::map<std::string, Object *>::const_iterator iter = m_objectEffects.begin();
		for(; iter != m_objectEffects.end(); ++iter)
			toRemove.push_back(iter->first);

		for(unsigned int i = 0; i < toRemove.size(); ++i)
			RemoveObjectEffect(toRemove[i]);
	}
}

// ----------------------------------------------------------------------

TangibleObject * TangibleObject::asTangibleObject()
{
	return this;
}

// ----------------------------------------------------------------------

TangibleObject const * TangibleObject::asTangibleObject() const
{
	return this;
}

// ----------------------------------------------------------------------

float TangibleObject::alter(const float elapsedTime)
{
	const float alterResult = ClientObject::alter(elapsedTime);
	if (alterResult != AlterResult::cms_kill)
	{
		//-- update damage
		{
			if (getClientData ())
			{
				const float currentDamageLevel = ms_useTestDamageLevel ? m_testDamageLevel : getDamageLevel ();
				if (currentDamageLevel != m_lastDamageLevel)
				{
					getClientData ()->applyDamage (this, hasCondition (C_onOff), m_lastDamageLevel, currentDamageLevel);
					m_lastDamageLevel = currentDamageLevel;
				}
			}

			if (m_interestingAttachedObject)
			{
				const Appearance * const app = getAppearance ();
				if (app)
				{
					if (app->getRenderedThisFrame ())
						m_interestingAttachedObject->setPosition_p (CuiObjectTextManager::getCurrentObjectHeadPoint_o (*this) + Vector::unitY * 0.25f);
				}
			}

			if(isInWorld())
			{
				VerifyObjectEffects();
				// Restart any object effects that are finished playing.
				std::map<std::string, Object *>::const_iterator iter = m_objectEffects.begin();
				for(; iter != m_objectEffects.end(); ++iter)
					if(iter->second)
					{
						ParticleEffectAppearance * particle = ParticleEffectAppearance::asParticleEffectAppearance(iter->second->getAppearance());
						if(particle && particle->isDeletable())
							particle->restart();
					}
			}
			else
			{
				std::map<std::string, Object *>::const_iterator iter = m_objectEffects.begin();
				for(; iter != m_objectEffects.end(); ++iter)
					RemoveObjectEffect(iter->first);
			}

		}
	}

	if (m_visabilityFlag == SharedTangibleObjectTemplate::CVF_gm_only)
	{
		PlayerObject const * const playerObject = Game::getPlayerObject();
		bool const isGod = (playerObject != 0) ? playerObject->isAdmin() : false;
		if (!isGod)
		{
			RenderWorld::recursivelyDisableDpvsObjectsForThisRender(this);
		}
	}

	return alterResult;
}

// ----------------------------------------------------------------------
/**
* Default implementation for any code that needs to be processed after
* this container object has changed.
*
* This is called on the container object after its contents have been
* changed.
*
* Implementer note: it is best to chain up to the base class implementation
* for this function in derived class implementations.
*/

void TangibleObject::doPostContainerChangeProcessing ()
{
}

// ----------------------------------------------------------------------
/**
* Called once the object has all its initialization data.
*
* The server can stream down data after the object is created.
* That is why we don't do this activity in the constructor.
*/

void TangibleObject::endBaselines()
{
	//-- chain down to parent class first to perform any necessary processing
	ClientObject::endBaselines();

	// Unnecessary and problematic --- results in a double call
	// to conditionModified(). endBaselines messages already call
	// AutoDelta* modified callbacks.
	// conditionModified (0, getCondition ());

	// We many not want to call the virtual method conditionModified() but
	// TangibleObject still has some logic that needs to be performed
	handleConditionModified (0, getCondition ());

	m_lastOnOffStatus = (getCondition() & C_onOff) != 0;
	setChildWingsOpened((getCondition() & C_wingsOpened) != 0);
}

// ----------------------------------------------------------------------

void TangibleObject::addToWorld()
{
	ClientObject::addToWorld();

	// Update the interesting attached object based upon user preferences
	updateInterestingAttachedObject(getCondition ());
}

// ----------------------------------------------------------------------

void TangibleObject::removeFromWorld()
{
	if (m_clientOnlyInteriorLayoutObjectList)
	{
		for (size_t i = 0; i < m_clientOnlyInteriorLayoutObjectList->size(); ++i)
		{
			Object * const object = (*m_clientOnlyInteriorLayoutObjectList)[i];
			delete object;
		}

		delete m_clientOnlyInteriorLayoutObjectList;
		m_clientOnlyInteriorLayoutObjectList = 0;
	}

	if (m_interestingAttachedObject)
	{
		delete m_interestingAttachedObject;
		m_interestingAttachedObject = 0;
	}

	ClientObject::removeFromWorld();
}

// ----------------------------------------------------------------------

void TangibleObject::setUntargettableOverride(bool const untargettable) const
{
	m_untargettableOverride = untargettable;
}

// ----------------------------------------------------------------------

bool TangibleObject::isTargettable() const
{
	bool const targettable = !m_untargettableOverride && safe_cast<const SharedTangibleObjectTemplate*>(getObjectTemplate())->getTargetable();

	if (m_visabilityFlag == SharedTangibleObjectTemplate::CVF_gm_only)
	{
		PlayerObject const * const playerObject = Game::getPlayerObject();
		bool const isGod = (playerObject != 0) ? playerObject->isAdmin() : false;
		return targettable && isGod;
	}

	return targettable;
}

// ----------------------------------------------------------------------
/**
* Override the object's filenames for the slot descriptor and arrangement descriptor.
*
* In single player, these come from the client ObjectTemplate.  In multi-player,
* the server sends down the proper filenames.
*/

void TangibleObject::setSlotInfo(const std::string &slotDescriptorName, const std::string &arrangementDescriptorName) const
{
	UNREF(slotDescriptorName);
	UNREF(arrangementDescriptorName);
#if 0 //@todo remove when tested
	if (!m_slotDescriptorName)
		m_slotDescriptorName = new std::string(slotDescriptorName);
	else
		*m_slotDescriptorName = slotDescriptorName;

	if (!m_arrangementDescriptorName)
		m_arrangementDescriptorName = new std::string(arrangementDescriptorName);
	else
		*m_arrangementDescriptorName = arrangementDescriptorName;
#endif
}

// ======================================================================

/**
* Changes this object's appearance from a shared template.
*
* @param objectTemplateName		filename of a shared object template
*/
void TangibleObject::changeAppearance(const char * objectTemplateName)
{
	if (objectTemplateName == NULL || *objectTemplateName == '\0')
		return;

	const SharedTangibleObjectTemplate * objectTemplate = dynamic_cast<
		const SharedTangibleObjectTemplate *>(ObjectTemplateList::fetch(
		objectTemplateName));
	if (objectTemplate == NULL)
		return;

	changeAppearance(*objectTemplate);

	objectTemplate->releaseReference();
}	// TangibleObject::changeAppearance

/**
* Changes this object's appearance from a shared template.
*
* @param objectTemplate		shared object template to get the appearance from
*/
void TangibleObject::changeAppearance(const SharedTangibleObjectTemplate & objectTemplate)
{
	if (Game::isClient () && objectTemplate.getOnlyVisibleInTools ())
		return;

	//-- create appearance (but don't set it into the Object yet).
	Appearance *appearance = 0;

	const std::string &appearanceString = objectTemplate.getAppearanceFilename();
	if (!appearanceString.empty())
		appearance = AppearanceTemplateList::createAppearance(appearanceString.c_str());

	//-- Delete customization data if we already have it.
	{
		CustomizationDataProperty *const cdProperty = safe_cast<CustomizationDataProperty*>(getProperty(CustomizationDataProperty::getClassPropertyId()));
		if (cdProperty)
		{
			removeProperty(CustomizationDataProperty::getClassPropertyId());
			delete cdProperty;
		}
	}

	//-- create CustomizationDataProperty.
	//   Force it to be created (even without customizations owned by the Object)
	//   if the object is a SkeletalAppearance.  This is because most wearables are
	//   worn on objects that get deformed, and the wearable needs to receive the
	//   wearer's customization data.  If the wearable didn't have a CustomizationData
	//   instance, it would never mount the owner's CustomizationData, and the owner
	//   body stature would have no effect.
	objectTemplate.createCustomizationDataPropertyAsNeeded(*this, dynamic_cast<SkeletalAppearance2*>(appearance) != 0);

	//-- Add procedural customization variables before customization data gets associated with appearance.  Otherwise
	//   we have to rebake any skeletal appearances as soon as we add a new variable.
	SlottedContainer *const slottedContainer = ContainerInterface::getSlottedContainer(*this);
	if (slottedContainer)
	{
		int const ruleCount = SlotRuleManager::getRuleCount();

		//-- Determine if any rules apply.
		bool match = false;

		{
			for (int i = 0; i < ruleCount; ++i)
			{
				//-- If this tangible's container has the slot required for this rule, a customization variable will be added.
				match = slottedContainer->hasSlot(SlotRuleManager::getSlotRequiredForRule(i));
				if (match)
					break;
			}
		}

		if (match)
		{
			//-- Get or create customization data.
			CustomizationDataProperty *cdProperty = safe_cast<CustomizationDataProperty*>(getProperty(CustomizationDataProperty::getClassPropertyId()));
			if (!cdProperty)
			{
				cdProperty = new CustomizationDataProperty(*this);
				addProperty(*cdProperty);
			}

			NOT_NULL(cdProperty);
			CustomizationData *const customizationData = cdProperty->fetchCustomizationData();
			NOT_NULL(customizationData);

			//-- Add any rules that apply.
			for (int i = 0; i < ruleCount; ++i)
			{
				//-- If this tangible's container has the slot required for this rule, add the customization variable.
				if (slottedContainer->hasSlot(SlotRuleManager::getSlotRequiredForRule(i)))
					SlotRuleManager::addCustomizationVariableForRule(i, *customizationData);
			}

			//-- Release local references.
			customizationData->release();
		}
	}

	//-- set the appearance.
	if (appearance)
		setAppearance(appearance);
}	// TangibleObject::changeAppearance(const SharedObjectTemplate &)

// ======================================================================
/**
* This function is called after the TangibleObject has received a
* new appearance data customization string.
*
* @arg value  the value of the new appearance customization string
*/

void TangibleObject::appearanceDataModified(const std::string& value)
{
	// we don't value to use the value arg since we already have the
	// data stored in m_appearanceData, but just in case the interface
	// changes, we'll use it here.  Technically we don't need to keep
	// the m_appearanceData string around on the client.  We can build
	// it at will from the appearance.

	//-- skip no-content customization data
	if (value.empty())
		return;

	// check if the property exists
	CustomizationDataProperty *cdProperty = safe_cast<CustomizationDataProperty*>(getProperty( CustomizationDataProperty::getClassPropertyId()));

	if (!cdProperty)
	{
		DEBUG_WARNING(true, ("non-zero-length appearance string sent to object that doesn't declare any customization variables in its template."));
		return;
	}

	// retrieve the CustomizationData instance associated with the property
	CustomizationData *const customizationData = cdProperty->fetchCustomizationData();
	NOT_NULL(customizationData);

	//-- initialize CustomizationData variable values from the string
	customizationData->loadLocalDataFromString(value);

	//-- release local reference
	customizationData->release();

	//-- Make sure the damn appearance gets an alter --- static meshes will not alter unless instructed.
	scheduleForAlter();
}

//----------------------------------------------------------------------

CustomizationData *TangibleObject::fetchCustomizationData ()
{
	CustomizationDataProperty * const cdprop = safe_cast<CustomizationDataProperty *>(getProperty (CustomizationDataProperty::getClassPropertyId()));
	return cdprop ? cdprop->fetchCustomizationData () : 0;
}

//----------------------------------------------------------------------

const CustomizationData *TangibleObject::fetchCustomizationData () const
{
	const CustomizationDataProperty * const cdprop = safe_cast<const CustomizationDataProperty *>(getProperty (CustomizationDataProperty::getClassPropertyId()));
	return cdprop ? cdprop->fetchCustomizationData () : 0;
}

//----------------------------------------------------------------------

int TangibleObject::getDamageTaken () const
{
	// m_accumulatedDamageTaken is the damage amount that we don't want to show on the client
	//  until the combat action has been resolved
	return m_damageTaken.get ();
}

//----------------------------------------------------------------------

int TangibleObject::getMaxHitPoints () const
{
	return m_maxHitPoints.get ();
}

//----------------------------------------------------------------------

const ManufactureSchematicObject * TangibleObject::getCraftingManfSchematic(void) const
{
	const SharedObjectTemplate * const myTemplate = safe_cast<const SharedObjectTemplate *>(getObjectTemplate());
	if (myTemplate->getGameObjectType() != SharedObjectTemplate::GOT_misc_crafting_station)
		return NULL;

	const CraftingToolSyncUi * const sync = safe_cast<const CraftingToolSyncUi *>(getClientSynchronizedUi());
	if (sync == NULL)
		return NULL;

	return dynamic_cast<const ManufactureSchematicObject *>(sync->getManfSchematic().getObject());
}

//----------------------------------------------------------------------

void TangibleObject::startCrafting(void)
{
	static const uint32 listen = Crc::normalizeAndCalculate("synchronizedUiListen");

	const SharedObjectTemplate * myTemplate = safe_cast<const SharedObjectTemplate *>(getObjectTemplate());
	if (myTemplate->getGameObjectType() == SharedObjectTemplate::GOT_tool_crafting)
	{
		// create our synced UI
		if (getClientSynchronizedUi() == NULL)
			setSynchronizedUi (new CraftingToolSyncUi(*this) );

		// tell the server we want it's data
		ClientCommandQueue::enqueueCommand(listen, getNetworkId(), Unicode::String());
	}
}

//----------------------------------------------------------------------

void TangibleObject::stopCrafting(void)
{
	static const uint32 stopListen = Crc::normalizeAndCalculate("synchronizedUiStopListening");

	const SharedObjectTemplate * myTemplate = safe_cast<const SharedObjectTemplate *>(getObjectTemplate());
	if (myTemplate->getGameObjectType() == SharedObjectTemplate::GOT_tool_crafting)
	{
		// tell the server we don't want it's data
		ClientCommandQueue::enqueueCommand(stopListen, getNetworkId(), Unicode::String());
	}
}

//----------------------------------------------------------------------

void TangibleObject::clientSetDamageTaken     (int dam)
{
	m_damageTaken = dam;
}

//----------------------------------------------------------------------

void TangibleObject::clientSetMaxHitPoints    (int dam)
{
	m_maxHitPoints = dam;
}

//----------------------------------------------------------------------

void TangibleObject::Callbacks::DamageTaken::modified(TangibleObject & target, const int& oldValue, const int& value, bool) const
{
	if (target.isInitialized())
	{
		if (value > oldValue)
			Transceivers::damageTaken.emitMessage (Messages::DamageTaken::Payload (&target, value - oldValue));
	}
}

//----------------------------------------------------------------------

bool TangibleObject::isPlayer () const
{
	return (m_pvpFlags & PvpStatusFlags::IsPlayer) != 0 || static_cast<const Object *>(Game::getPlayer ()) == this;
}

//----------------------------------------------------------------------

bool TangibleObject::isAttackable () const
{
	return Game::getSinglePlayer () || (!isInvulnerable () && (getPvpFlags() & PvpStatusFlags::YouCanAttack) != 0);
}

//----------------------------------------------------------------------

bool TangibleObject::canAttackPlayer  () const
{
	return (getPvpFlags() & PvpStatusFlags::CanAttackYou) != 0;
}

//----------------------------------------------------------------------

bool TangibleObject::isEnemy() const
{
	return (getPvpFlags() & PvpStatusFlags::IsEnemy) != 0;
}

//----------------------------------------------------------------------

bool TangibleObject::isSameFaction            (const TangibleObject & other) const
{
	return getPvpFaction() && getPvpFaction() == other.getPvpFaction();
}

//----------------------------------------------------------------------

bool TangibleObject::canHelp() const
{
	return (getPvpFlags() & PvpStatusFlags::YouCanHelp) != 0;
}

//----------------------------------------------------------------------

const stdset<int>::fwd& TangibleObject::getComponents () const
{
	return m_components.get ();
}

//----------------------------------------------------------------------

bool TangibleObject::canSitOn () const
{
	Appearance const * const appearance = getAppearance ();
	if (appearance)
	{
		Transform  transform(Transform::IF_none);

		const bool hasChairHardpoint = appearance->findHardpoint (cs_chairHardpointName, transform);
		return hasChairHardpoint;
	}

	return false;
}

// ----------------------------------------------------------------------
/**
* This function builds the parameter string
*
* This function only makes sense to call if the caller is generating a
* command queue entry for sitting in chairs.
*/

Unicode::String TangibleObject::buildChairParameterString() const
{
	DEBUG_WARNING(!canSitOn(), ("buildChairParameterString(): it makes no sense to call this function since canSitOn() returns false."));

	char buffer[256];

	CellProperty const *const chairCell       = getParentCell();
	Vector       const        chairPosition_p = getPosition_p();

	snprintf(buffer, sizeof(buffer) - 1, "%g,%g,%g,%s", chairPosition_p.x, chairPosition_p.y, chairPosition_p.z, (chairCell != CellProperty::getWorldCellProperty()) ? chairCell->getOwner().getNetworkId().getValueString().c_str() : NetworkId::cms_invalid.getValueString().c_str());
	buffer[sizeof(buffer) - 1] = '\0';

	return Unicode::narrowToWide(buffer);
}

// ----------------------------------------------------------------------

Footprint *TangibleObject::getFootprint ()
{
	CollisionProperty *const property = getCollisionProperty ();
	if (property)
		return property->getFootprint ();
	else
		return 0;
}

// ----------------------------------------------------------------------

float TangibleObject::getDamageLevel () const
{
	const int maxHitPoints = getMaxHitPoints ();
	const int damageTaken = getDamageTaken ();

	return maxHitPoints == 0 ? 0.f : static_cast<float> (damageTaken) / maxHitPoints;
}

//----------------------------------------------------------------------

void TangibleObject::setTestDamageLevel (const float testDamageLevel)
{
	m_testDamageLevel = testDamageLevel;
	scheduleForAlter();
}

//----------------------------------------------------------------------

uint32 TangibleObject::getPvpFlags() const
{
	return m_pvpFlags;
}

//----------------------------------------------------------------------

int TangibleObject::getPvpType() const
{
	return m_pvpType.get();
}

//----------------------------------------------------------------------

uint32 TangibleObject::getPvpFaction() const
{
	return m_pvpFactionId.get();
}

//----------------------------------------------------------------------

void TangibleObject::maxHitPointsModified (int oldMaxHitPoints, int newMaxHitPoints)
{
	if (!isInitialized ())
		return;

	//-- destroyed state has changed
	if ((oldMaxHitPoints <= 0 && newMaxHitPoints > 0) || (newMaxHitPoints <= 0 && oldMaxHitPoints > 0))
	{
		setLocalizedNameDirty ();
	}
}

//----------------------------------------------------------------------

void TangibleObject::conditionModified (int oldCondition, int newCondition)
{
	if (!isInitialized ())
		return;

	// We need to check our conditions once initialization is complete,
	// so all the logic exists in handleConditionModified()
	handleConditionModified(oldCondition, newCondition);
}

//----------------------------------------------------------------------

void TangibleObject::setCondition(int const condition)
{
	DEBUG_WARNING(ms_logChangedConditions && !isFakeNetworkId(getNetworkId()), ("TangibleObject::setCondition: called on object without a fake network id. condition=%i, object=%s", condition, getDebugInformation(true).c_str()));
	int const oldCondition = m_condition.get();
	m_condition = oldCondition | condition;

#if PRODUCTION == 0
	if (Game::getSinglePlayer())
		conditionModified(oldCondition, m_condition.get());
#endif
}

//----------------------------------------------------------------------

void TangibleObject::setCount(int count)
{
	if (Game::getSinglePlayer ())
	{
		m_count = count;
	}
}

//----------------------------------------------------------------------

void TangibleObject::clearCondition(int const condition)
{
	DEBUG_WARNING(ms_logChangedConditions && !isFakeNetworkId(getNetworkId()), ("TangibleObject::clearCondition: called on object without a fake network id. condition=%i, object=%s", condition, getDebugInformation(true).c_str()));
	int const oldCondition = m_condition.get();
	m_condition = oldCondition & (~condition);

#if PRODUCTION == 0
	if (Game::getSinglePlayer())
		conditionModified(oldCondition, m_condition.get());
#endif
}

//----------------------------------------------------------------------

void TangibleObject::setAppearanceData(const std::string &newAppearanceData)
{
	m_appearanceData = newAppearanceData;
}

//----------------------------------------------------------------------

void TangibleObject::filterLocalizedName (Unicode::String & localizedName) const
{
	ClientObject::filterLocalizedName (localizedName);

	if (GameObjectTypes::isTypeOf (getGameObjectType (), SharedObjectTemplate::GOT_vehicle))
	{
		if (getMaxHitPoints () == 0 && !isInvulnerable ())
		{
			Unicode::String result;
			CuiStringVariablesManager::process (SharedStringIds::vehicle_destroyed_name_prose, localizedName, Unicode::emptyString, Unicode::emptyString, result);
			localizedName = result;
		}
		else if (hasCondition (TangibleObject::C_disabled) && getMaxHitPoints () > 0 && !isInvulnerable ())
		{
			Unicode::String result;
			CuiStringVariablesManager::process (SharedStringIds::vehicle_disabled_name_prose, localizedName, Unicode::emptyString, Unicode::emptyString, result);
			localizedName = result;
		}
	}
}

// ----------------------------------------------------------------------

void TangibleObject::getRequiredCertifications(std::vector<std::string> & results) const
{
	const SharedTangibleObjectTemplate * const sharedObjectTemplate = safe_cast<const SharedTangibleObjectTemplate*>(getObjectTemplate());
	if (sharedObjectTemplate)
	{
		const int numRequired = sharedObjectTemplate->getCertificationsRequiredCount();
		for(int i=0; i<numRequired; ++i)
		{
			results.push_back(sharedObjectTemplate->getCertificationsRequired(i));
		}
	}
}

// ----------------------------------------------------------------------

InteriorLayoutReaderWriter const * TangibleObject::getInteriorLayout() const
{
	return 0;
}

// ----------------------------------------------------------------------

void TangibleObject::addClientOnlyInteriorLayoutObject(Object * const object)
{
	if (!m_clientOnlyInteriorLayoutObjectList)
		m_clientOnlyInteriorLayoutObjectList = new ClientOnlyInteriorLayoutObjectList;

	m_clientOnlyInteriorLayoutObjectList->push_back(Watcher<Object>(object));
}

//----------------------------------------------------------------------

void TangibleObject::handleConditionModified (int oldCondition, int newCondition)
{
	//-- disabled state has changed
	if ((oldCondition & TangibleObject::C_disabled) != (newCondition & TangibleObject::C_disabled))
	{
		setLocalizedNameDirty ();
	}

	if (getClientData ())
	{
		const bool on = (newCondition & C_onOff) != 0;
		if (on != m_lastOnOffStatus)
		{
			getClientData ()->applyOnOff (this, on);
			m_lastOnOffStatus = on;
		}
	}

	// We will not attach an interesting object unless this object is in the world
	if (isInWorld())
	{
		updateInterestingAttachedObject( newCondition );
	}

	// If the object is interesting we need to keep track of whether they want to see the interesting object
	bool const interesting = (newCondition & C_interesting) != 0;
	bool const spaceInteresting = (newCondition & C_spaceInteresting) != 0;

	if (interesting || spaceInteresting)
	{
		// Start observing client preference changes for displaying the interesting attached object
		if (!m_isReceivingCallbacks)
		{
			CuiPreferences::getShowInterestingAppearanceCallback().attachReceiver(*this);
			m_isReceivingCallbacks = true;
		}
	}
	else
	{
		// Stop observing client preference changes for displaying the interesting attached object
		if (m_isReceivingCallbacks)
		{
			CuiPreferences::getShowInterestingAppearanceCallback().detachReceiver(*this);
			m_isReceivingCallbacks = false;
		}
	}

	//-- Pass wingsOpened to any child objects
	bool const oldWingsOpened = oldCondition & C_wingsOpened;
	bool const newWingsOpened = newCondition & C_wingsOpened;
	if (newWingsOpened != oldWingsOpened)
		setChildWingsOpened(newWingsOpened);

	//-- Make sure we get an alter
	if (isInWorld())
		scheduleForAlter();
}

// ----------------------------------------------------------------------

void TangibleObject::setChildWingsOpened(bool const wingsOpened)
{
	for (int i = 0; i < getNumberOfChildObjects(); ++i)
	{
		Object * const childObject = getChildObject(i);
		if (childObject && childObject->isChildObject() && childObject->asClientObject() && childObject->asClientObject()->asTangibleObject())
		{
			if (wingsOpened)
				childObject->asClientObject()->asTangibleObject()->setCondition(C_wingsOpened);
			else
				childObject->asClientObject()->asTangibleObject()->clearCondition(C_wingsOpened);
		}
	}
}

// ======================================================================

void TangibleObject::updateInterestingAttachedObject(int objectCondition)
{
	// Remove the old interesting attached object since we may need to create a different one
	if (m_interestingAttachedObject)
	{
		delete m_interestingAttachedObject;
		m_interestingAttachedObject = 0;
	}

	// Check whether the user wants to see interesting attached objects
	if (CuiPreferences::getShowInterestingAppearance ())
	{
		// Determine in what way the object is interesting
		bool const interesting = (objectCondition & C_interesting) != 0;
		bool const spaceInteresting = (objectCondition & C_spaceInteresting) != 0;
		bool const holidayInteresting = (objectCondition & C_holidayInteresting) != 0;

		if (interesting || spaceInteresting || holidayInteresting)
		{
			m_interestingAttachedObject = new Object ();

			char const * const appearanceTemplateName =  holidayInteresting ? "appearance/item_present_partcile_icon.apt" : (spaceInteresting ? "appearance/space_info_disk.apt" : "appearance/info_disk.apt");
			m_interestingAttachedObject->setAppearance(AppearanceTemplateList::createAppearance(appearanceTemplateName));
			m_interestingAttachedObject->addNotification(ClientWorld::getIntangibleNotification());
			RenderWorld::addObjectNotifications (*m_interestingAttachedObject);
			m_interestingAttachedObject->attachToObject_p (this, false);
			m_interestingAttachedObject->addToWorld();

			RotationDynamics * const rotationDynamics = new RotationDynamics (m_interestingAttachedObject, Vector (0.5f, 0.f, 0.f));
			rotationDynamics->setState (true);
			m_interestingAttachedObject->setDynamics (rotationDynamics);
		}
	}
}

//----------------------------------------------------------------------

void TangibleObject::performCallback()
{
	// We will not attach an interesting object unless this object is in the world
	if (isInWorld())
	{
		// Update the interesting attached object based upon user preferences
		updateInterestingAttachedObject(getCondition ());
	}
}

// ----------------------------------------------------------------------

TangibleObject * TangibleObject::getTangibleObject(NetworkId const & networkId)
{
	return asTangibleObject(NetworkIdManager::getObjectById(networkId));
}

// ----------------------------------------------------------------------

TangibleObject const * TangibleObject::asTangibleObject(Object const * object)
{
	ClientObject const * const clientObject = (object != NULL) ? object->asClientObject() : NULL;

	return (clientObject != NULL) ? clientObject->asTangibleObject() : NULL;
}

// ----------------------------------------------------------------------

TangibleObject * TangibleObject::asTangibleObject(Object * object)
{
	ClientObject * clientObject = (object != NULL) ? object->asClientObject() : NULL;

	return (clientObject != NULL) ? clientObject->asTangibleObject() : NULL;
}

// ----------------------------------------------------------------------
bool TangibleObject::isInCombat() const
{
	return m_inCombat.get();
}

// ----------------------------------------------------------------------

void TangibleObject::setIsInCombat( bool inCombat)
{
	m_inCombat.set(inCombat);
}

// ----------------------------------------------------------------------

void TangibleObject::AddObjectEffect(std::string const & label, std::string const & effectFile, std::string const & hardpoint, Vector const & offset, float const scale)
{
	UNREF(offset);
	UNREF(hardpoint);

	if(m_objectEffects.find(label) != m_objectEffects.end())
		return;

	if(!isInWorld())
		return;

	ParticleEffectAppearance * newObjectEffect = NULL;
	ParticleEffectAppearanceTemplate const * particleTemplate = dynamic_cast<ParticleEffectAppearanceTemplate const *>(AppearanceTemplateList::fetch(effectFile.c_str()));
	if(!particleTemplate)
	{
		DEBUG_WARNING(true, ("AddObjectEffect: Could not fetch the effect using template name[%s]", effectFile.c_str()));
		m_objectEffects[label] = NULL;
		return;
	}

	Object *newObject = new Object();
	NOT_NULL(newObject);

	RenderWorld::addObjectNotifications(*newObject);

	newObjectEffect = ParticleEffectAppearance::asParticleEffectAppearance(particleTemplate->createAppearance());
	NOT_NULL(newObjectEffect);

	AppearanceTemplateList::release(particleTemplate);

	newObjectEffect->setAutoDelete(false);
	newObjectEffect->setOwner(newObject);
	newObjectEffect->setPlayBackRate(1.0f);
	newObjectEffect->setScale(Vector(scale, scale, scale));

	newObject->setAppearance(newObjectEffect);

	if(getCellProperty())
	{
		//this MUST MUST MUST MUST MUST MUST be reset to true before returning!!!
		CellProperty::setPortalTransitionsEnabled(false);
		newObject->setParentCell(getCellProperty());
	}

	newObject->addNotification(ClientWorld::getIntangibleNotification());

	newObject->attachToObject_w(this, true);

	if(!hardpoint.empty())
	{
		Appearance const * const thisApp = this->getAppearance();
		Transform hardpointToParent = Transform::identity;
		if(thisApp)
		{
			if(thisApp->findHardpoint(CrcLowerString(hardpoint.c_str()), hardpointToParent))
			{
				Vector finalOffset = hardpointToParent.getPosition_p() + offset;
				hardpointToParent.setPosition_p(finalOffset);

				newObject->setTransform_o2p(hardpointToParent);
			}
		}
	}

	m_objectEffects[label] = newObject;

	if(getCellProperty())
	{
		CellProperty::setPortalTransitionsEnabled(true);
	}

	return;
}

// ----------------------------------------------------------------------

void TangibleObject::RemoveObjectEffect(std::string const & label)
{
	std::map<std::string, Object *>::iterator iter = m_objectEffects.find(label);
	if(iter == m_objectEffects.end())
		return;

	if(iter->second)
	{

		this->removeChildObject(iter->second, DF_none);

		iter->second->removeFromWorld();

		delete iter->second;
	}

	m_objectEffects.erase(iter);
}

void TangibleObject::VerifyObjectEffects()
{
	if(m_objectEffects.size() != m_effectsMap.size())
	{
		std::vector<std::string> toRemove;
		std::map<std::string, Object *>::const_iterator iter = m_objectEffects.begin();
		for(; iter != m_objectEffects.end(); ++iter)
			toRemove.push_back(iter->first);

		for(unsigned int i = 0; i < toRemove.size(); ++i)
			RemoveObjectEffect(toRemove[i]);
		// <std::string, std::pair<std::string, std::pair<std::string, std::pair<Vector, float> >
		Archive::AutoDeltaMap< std::string, std::pair<std::string, std::pair<std::string, std::pair<Vector, float> > > >::const_iterator addIter = m_effectsMap.begin();
		for(; addIter != m_effectsMap.end(); ++addIter)
			AddObjectEffect(addIter->first, addIter->second.first, addIter->second.second.first, addIter->second.second.second.first, addIter->second.second.second.second );
	}
}

// ----------------------------------------------------------------------

void TangibleObject::getObjectInfo(std::map<std::string, std::map<std::string, Unicode::String> > & propertyMap) const
{
	/**
	When adding a variable to this class, please add it here.  Variable that aren't easily displayable are still listed, for tracking purposes.
	*/

	/**
	Don't compile in production build because this maps human-readable values to data members and makes hacking easier
	*/
#if PRODUCTION == 0
	//	Archive::AutoDeltaVariableCallback<std::string, Callbacks::AppearanceData, TangibleObject> m_appearanceData;
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "DamageTaken", m_damageTaken.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "MaxHitPoints", m_maxHitPoints.get());
	//	Archive::AutoDeltaSet<int>                m_components;      // component table ids of visible components attached to this object
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "Visible", m_visible.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "Count", m_count.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "Condition", m_condition.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "AccumulatedDamageTaken", m_accumulatedDamageTaken);
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "PVPFlags", m_pvpFlags);
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "PVPType", m_pvpType.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "PVPFactionId", m_pvpFactionId.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "LastDamageLevel", m_lastDamageLevel);
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "TestDamageLevel", m_testDamageLevel);
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "LastOnOffStatus", m_lastOnOffStatus);
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "IsReceivingCallbacks", m_isReceivingCallbacks);
	//	Watcher<Object>                           m_interestingAttachedObject;
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "UntargettableOverride", m_untargettableOverride);
	//ClientOnlyInteriorLayoutObjectList * m_clientOnlyInteriorLayoutObjectList;

	std::string passiveRevealPlayerCharacter;
	for (Archive::AutoDeltaSet<NetworkId>::const_iterator i = m_passiveRevealPlayerCharacter.begin(); i != m_passiveRevealPlayerCharacter.end(); ++i)
	{
		passiveRevealPlayerCharacter += i->getValueString();
		passiveRevealPlayerCharacter += ", ";
	}
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "PassiveRevealPlayerCharacter", passiveRevealPlayerCharacter);

	ClientObject::getObjectInfo(propertyMap);

#else
	UNREF(propertyMap);
#endif
}


void TangibleObject::OnObjectEffectInsert(std::string const & key, std::pair<std::string, std::pair<std::string, std::pair<Vector, float> > > const & value)
{
	AddObjectEffect(key, value.first, value.second.first, value.second.second.first, value.second.second.second);
}

void TangibleObject::OnObjectEffectErased(std::string const & key, std::pair<std::string, std::pair<std::string, std::pair<Vector, float> > > const & value)
{
	UNREF(value);

	RemoveObjectEffect(key);
}

void TangibleObject::OnObjectEffectModified(std::string const & key, std::pair<std::string, std::pair<std::string, std::pair<Vector, float> > > const & oldValue, std::pair<std::string, std::pair<std::string, std::pair<Vector, float> > > const & newValue)
{
	UNREF(oldValue);

	RemoveObjectEffect(key);
	AddObjectEffect(key, newValue.first, newValue.second.first, newValue.second.second.first, newValue.second.second.second);
}

// ======================================================================
