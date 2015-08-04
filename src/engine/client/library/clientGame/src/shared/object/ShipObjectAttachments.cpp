//======================================================================
//
// ShipObjectAttachments.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ShipObjectAttachments.h"

#include "clientGame/ClientDataFile.h"
#include "clientGame/PlayerShipTurretController.h"
#include "clientGame/ShipObject.h"
#include "clientGame/ShipObjectEffects.h"
#include "clientGraphics/RenderWorld.h"
#include "clientParticle/ParticleEffectAppearance.h"
#include "sharedFoundation/PersistentCrcString.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedGame/ShipComponentAttachmentManager.h"
#include "sharedGame/ShipComponentType.h"
#include "sharedGame/ShipComponentWeaponManager.h"
#include "sharedGame/ShipTurretManager.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/Hardpoint.h"
#include "sharedObject/ObjectTemplateList.h"

//======================================================================

namespace ShipObjectAttachmentsNamespace
{

	typedef stdvector<Object *>::fwd  ObjectVector;

	void findObjectsWithHardpoint    (CrcString const & hardpointName, Object & obj, ObjectVector & ov)
	{
		Appearance const * const app = obj.getAppearance ();

		if (app == NULL)
			return;

		Transform t;

		if (app->findHardpoint (hardpointName, t))
		{
			ov.push_back (&obj);
		}

		int const numberOfChildObjects = obj.getNumberOfChildObjects ();
		for (int i = 0; i < numberOfChildObjects; ++i)
		{
			Object * const child = obj.getChildObject (i);
			if (child)
			{
				bool const isCreatureObject = child->asClientObject() && child->asClientObject()->asCreatureObject();

				if (!isCreatureObject)
					findObjectsWithHardpoint (hardpointName, *child, ov);
			}
		}
	}

	//----------------------------------------------------------------------

	typedef ShipObjectAttachments::TransformObjectList TransformObjectList;

	void findHardpoints_o(Object & object, std::string const & hardpointPrefixString, Transform const & parentTransform_w, TransformObjectList & transformObjectList)
	{
		//-- Finds all hardpoints that start with "muzzle" in object space for an object
		Appearance const * const appearance = object.getAppearance();
		if (appearance)
		{
			AppearanceTemplate const * const appearanceTemplate = appearance->getAppearanceTemplate();
			if (appearanceTemplate)
			{
				size_t const hardpointPrefixStringLength = hardpointPrefixString.size();

				for (int i = 0; i < appearanceTemplate->getHardpointCount(); ++i)
				{
					Hardpoint const & hardpoint = appearanceTemplate->getHardpoint(i);
					if (strncmp(hardpoint.getName().getString(), hardpointPrefixString.c_str (), hardpointPrefixStringLength) == 0)
					{
						ShipObjectAttachments::TransformObjectPair const 
							transformObjectPair(parentTransform_w.rotateTranslate_p2l(object.getTransform_o2w().rotateTranslate_l2p(hardpoint.getTransform())), &object);
						transformObjectList.push_back(transformObjectPair);
					}
				}
			}
		}

		//-- Recurse to children
		for (int i = 0; i < object.getNumberOfChildObjects(); ++i)
		{
			Object * const childObject = object.getChildObject(i);
			if (childObject)
				findHardpoints_o(*childObject, hardpointPrefixString, parentTransform_w, transformObjectList);
		}
	}

	//----------------------------------------------------------------------

	std::string const s_muzzleHardpointName = "muzzle";
}

using namespace ShipObjectAttachmentsNamespace;

//----------------------------------------------------------------------

ShipObjectAttachments::ShipObjectAttachments(ShipObject & ship) :
m_ship(&ship),
m_componentAttachments(new WatcherVectorMap),
m_componentDestroyedParticles(new WatcherVectorMap),
m_averageProjectileMuzzlePosition_p(),
m_averageProjectileRange(0.0f),
m_averageProjectileDataDirty(true)
{
}

//----------------------------------------------------------------------

ShipObjectAttachments::~ShipObjectAttachments()
{
	m_ship = NULL;
	
	delete m_componentAttachments;
	m_componentAttachments = NULL;

	delete m_componentDestroyedParticles;
	m_componentDestroyedParticles = NULL;
}

//----------------------------------------------------------------------

void ShipObjectAttachments::updateComponentStatesForSlot(int const chassisSlot)
{
	//-- per-chassis slot operations
	switch (chassisSlot)
	{
	case ShipChassisSlotType::SCST_engine:
		{			
			if (!m_ship->isComponentFunctional(chassisSlot))
			{
				m_ship->clearCondition(TangibleObject::C_onOff);
			}
			else
			{
				m_ship->setCondition(TangibleObject::C_onOff);
			}
		}
		break;
	default:
		break;		
	}
	
	WatcherVectorMap::iterator wvm_it = m_componentAttachments->find(chassisSlot);
	if (wvm_it == m_componentAttachments->end())
		return;
	
	WatcherVector & watcherVector = (*wvm_it).second;
	
	float const hpMax = m_ship->getComponentHitpointsMaximum(chassisSlot);
	float const hpCur = m_ship->getComponentHitpointsCurrent(chassisSlot);
	
	float const armorMax = m_ship->getComponentArmorHitpointsMaximum(chassisSlot);
	float const armorCur = m_ship->getComponentArmorHitpointsCurrent(chassisSlot);
	
	int const damageMax = static_cast<int>(hpMax + armorMax);
	int const damageCur = static_cast<int>(hpCur + armorCur);
	
	const int componentType = ShipChassisSlotType::getComponentTypeForSlot(static_cast<ShipChassisSlotType::Type>(chassisSlot));

	for (WatcherVector::iterator wv_it = watcherVector.begin(); wv_it != watcherVector.end(); ++wv_it)
	{
		ObjectWatcher & objectWatcher = *wv_it;
		Object * const object = objectWatcher.getPointer();
		
		if (object == NULL)
			continue;
		
		TangibleObject * const tangibleObject = NON_NULL(NON_NULL(object->asClientObject())->asTangibleObject());
		
		if (tangibleObject == NULL)
			continue;		

		switch (componentType)
		{
		case ShipComponentType::SCT_booster:
			{
				if (m_ship->isComponentFunctional(chassisSlot) && m_ship->isBoosterActive())
					tangibleObject->setCondition(TangibleObject::C_onOff);
				else
					tangibleObject->clearCondition(TangibleObject::C_onOff);
			}
			break;
			
		default:
			if (!m_ship->isComponentFunctional(chassisSlot))
			{
				if (tangibleObject->hasCondition(TangibleObject::C_onOff))
				{
					if (ShipComponentType::SCT_weapon == componentType)
						m_averageProjectileDataDirty = true;
					tangibleObject->clearCondition(TangibleObject::C_onOff);
				}			
			}
			else
			{
				if (!tangibleObject->hasCondition(TangibleObject::C_onOff))
				{
					if (ShipComponentType::SCT_weapon == componentType)
						m_averageProjectileDataDirty = true;
					tangibleObject->setCondition(TangibleObject::C_onOff);
				}
			}
			break;
		}
		
		int const oldDamageMax = tangibleObject->getMaxHitPoints();
		int const oldDamageCur = oldDamageMax - tangibleObject->getDamageTaken();
		
		if (oldDamageMax == damageMax && oldDamageCur == damageCur)
			continue;
		
		tangibleObject->clientSetMaxHitPoints(damageMax);
		tangibleObject->clientSetDamageTaken(damageMax - damageCur);
		
		if (damageMax <= 0 || oldDamageMax <= 0)
			continue;
		
		const float currentDamageLevel = 1.0f - (static_cast<float>(damageCur) / damageMax);
		const float oldDamageLevel = 1.0f - (static_cast<float>(oldDamageCur) / oldDamageMax);
		
		//-- per-attachment, per-chassis slot operations relating to damage levels
		switch (chassisSlot)
		{
		case ShipChassisSlotType::SCST_engine:
			{
				m_ship->getShipObjectEffects().updateComponentState(tangibleObject, chassisSlot, oldDamageLevel, currentDamageLevel);
			}
			break;
		default:
			break;
		}
	}
}

//----------------------------------------------------------------------

Object *ShipObjectAttachments::getFirstAttachedObjectForWeapon(int weaponIndex, Transform &hardpointTransform_o2p)
{
	int const chassisSlot = weaponIndex + static_cast<int>(ShipChassisSlotType::SCST_weapon_first);
	WatcherVectorMap::iterator const wvm_it = m_componentAttachments->find(chassisSlot);
	if (wvm_it != m_componentAttachments->end())
	{
		WatcherVector &wv = (*wvm_it).second;
		if (!wv.empty())
		{
			Object * const turret = (*wv.begin()).getPointer();
			if (turret)
			{
				uint32 const componentCrc = m_ship->getComponentCrc(chassisSlot);
				ShipComponentAttachmentManager::TemplateHardpointPairVector const &thpv =	ShipComponentAttachmentManager::getAttachmentsForShip(m_ship->getChassisType(), componentCrc, chassisSlot);
				if (!thpv.empty())
				{
					PersistentCrcString const &hardpointName = (*thpv.begin()).second;
					Object const * const attachedTo = NON_NULL(turret->getAttachedTo());
					if (NULL != attachedTo->getAppearance())
					{
						if (attachedTo->getAppearance()->findHardpoint(hardpointName, hardpointTransform_o2p))
							return turret;
					}
				}
			}
		}
	}
	return NULL;
}

// ----------------------------------------------------------------------

/**
 * Find the first location of a particular component, either the first attached
 * object for that component, or the first "extra" hardpoint for the component (on capital ships)
 */
bool ShipObjectAttachments::getFirstComponentPosition_w(int const chassisSlot, Vector &componentPosition_w) const
{
	//look for attached objects
	WatcherVectorMap::const_iterator const wvm_it = m_componentAttachments->find(chassisSlot);
	if (wvm_it != m_componentAttachments->end())
	{
		WatcherVector const &wv = (*wvm_it).second;
		if (!wv.empty())
		{
			Object * const component = (*wv.begin()).getPointer();
			if (component)
			{
				componentPosition_w = component->getPosition_w();
				return true;
			}
		}
	}

	if (m_ship->isCapitalShip())
	{
		//look for "extra" slots
		ShipComponentAttachmentManager::HardpointVector const &extraHardpoints = ShipComponentAttachmentManager::getExtraHardpointsForComponent(m_ship->getChassisType(), m_ship->getComponentCrc(chassisSlot), chassisSlot);
		if (!extraHardpoints.empty())
		{
			Transform hardpointTransform;
			if (m_ship->getAppearance()->findHardpoint(*(extraHardpoints.begin()), hardpointTransform))
			{
				componentPosition_w = m_ship->rotateTranslate_o2w(hardpointTransform.getPosition_p());
				return true;
			}
		}
	}

	return false;
}

//----------------------------------------------------------------------

bool ShipObjectAttachments::getFirstComponentPosition_o(int const chassisSlot, Vector &componentPosition_o) const
{
	if (getFirstComponentPosition_w(chassisSlot, componentPosition_o))
	{
		componentPosition_o = m_ship->rotateTranslate_p2o(componentPosition_o);
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------

Object const *ShipObjectAttachments::getFirstAttachedObjectForWeapon(int weaponIndex, Transform &hardpointTransform_o2p) const
{
	return const_cast<ShipObjectAttachments *>(this)->getFirstAttachedObjectForWeapon(weaponIndex, hardpointTransform_o2p);
}

//----------------------------------------------------------------------

void ShipObjectAttachments::updateComponentAttachment (int chassisSlot)
{
	if (ShipChassisSlotType::isWeaponIndex(static_cast<ShipChassisSlotType::Type>(chassisSlot)))
		m_averageProjectileDataDirty = true;

	CustomizationData * const rootCustomizationData = m_ship->fetchCustomizationData ();
	
	ShipComponentAttachmentManager::TemplateHardpointPairVector const & thpv =
		ShipComponentAttachmentManager::getAttachmentsForShip (m_ship->getChassisType (), m_ship->getComponentCrc(chassisSlot), chassisSlot);
	
	bool needPlayerShipTurretController = false;
	if (chassisSlot >= ShipChassisSlotType::SCST_weapon_first && chassisSlot <= ShipChassisSlotType::SCST_weapon_last)
		needPlayerShipTurretController = m_ship->isWeaponPlayerControlled(chassisSlot-ShipChassisSlotType::SCST_weapon_first);
	
	if (thpv.empty())
	{
		m_ship->getShipObjectEffects().resetEngineEffects(m_ship, chassisSlot);
	}
	else
	{
		{
			WatcherVectorMap::iterator itParticles = m_componentDestroyedParticles->find(chassisSlot);

			if (itParticles != m_componentDestroyedParticles->end())
			{
				WatcherVector & particles = (*itParticles).second;

				for (WatcherVector::const_iterator it = particles.begin(); it != particles.end(); ++it)
				{
					Object * const obj = *it;
					if (NULL != obj)
						obj->kill();
				}
				m_componentDestroyedParticles->erase(itParticles);
			}
		}

		for (ShipComponentAttachmentManager::TemplateHardpointPairVector::const_iterator thpv_it = thpv.begin (); thpv_it != thpv.end (); ++thpv_it)
		{
			uint32 const objectTemplateCrc  = (*thpv_it).first;
			PersistentCrcString const & hardpointName = (*thpv_it).second;
			
			if (objectTemplateCrc == 0)
				continue;
			
			SharedObjectTemplate const * const ot = safe_cast<const SharedObjectTemplate *>(ObjectTemplateList::fetch (objectTemplateCrc));
			
			if (ot == NULL)
			{
				WARNING (true, ("ShipObject failed to resolve object template crc [%u]", objectTemplateCrc));
				continue;
			}
			
			static ObjectVector ov;
			ov.clear ();

			if (!hardpointName.isEmpty())
				findObjectsWithHardpoint (hardpointName, *m_ship, ov);
			else
				ov.push_back(m_ship);
			
			// HACK
			Vector previousHardpointLocation_w(Vector::zero);
			bool previousHardpointLocationValid = false;

			// END HACK
			
			for (ObjectVector::const_iterator it = ov.begin (); it != ov.end (); ++it)
			{
				Object * const obj = *it;
				
				if (!obj || obj->getKill())
					continue;
				
				//HACK -- ignore hardpoints with the same name that work out to have the same world space location
				{
					Appearance const * const app = obj->getAppearance ();
					
					if (app)
					{
						Transform hardpointTransform_o;
						if (app->findHardpoint (hardpointName, hardpointTransform_o))
						{
							Vector hardpointLocation_w = obj->rotateTranslate_o2w(hardpointTransform_o.getPosition_p());
							if (previousHardpointLocationValid && hardpointLocation_w == previousHardpointLocation_w)
							{
								DEBUG_WARNING(true,("Ignored duplicate hardpoint %s on object %s",hardpointName.getString(), m_ship->getDebugInformation().c_str()));
								continue;
							}
							previousHardpointLocation_w = hardpointLocation_w;
							previousHardpointLocationValid = true;
						}
					}
				}
				//END HACK
				
				ClientObject * const child = safe_cast<ClientObject *>(ot->createObject ());
				
				if (!child)
					continue;
				
				if (child->asTangibleObject() == NULL)
				{
					WARNING(true, ("ShipObject attachment [%s] is non-tangible", ot->getName()));
					delete child;
					continue;
				}

				// Give the client-only effect a unique id.
				child->setNetworkId(ClientObject::getNextFakeNetworkId());
				
				if (needPlayerShipTurretController)
				{
					PlayerShipTurretController * const turretController = new PlayerShipTurretController(child);
					child->setController(turretController);
					needPlayerShipTurretController = false;
				} //lint !e429 // child takes ownership of turretController
				
				child->endBaselines ();
				
				RenderWorld::addObjectNotifications (*child);
				
				obj->addChildObject_o (child);

				//-- glows must be applied _after_ the object is added as a child, because some
				//-- glows will be attached to hardpoints on the parent(s), not the child object
				ClientDataFile const * const cdf = child->getClientData();
				if (NULL != cdf)
				{
					cdf->applyGlows(*child, true);
				}
				
				Appearance const * const app = obj->getAppearance ();
				
				if (app)
				{
					Transform t;
					if (app->findHardpoint (hardpointName, t))
					{
						child->setTransform_o2p (t);
					}
				}
				
				if (rootCustomizationData)
				{
					TangibleObject * const tangible = child->asTangibleObject ();
					
					if (tangible)
					{
						CustomizationData * const childCustomizationData = tangible->fetchCustomizationData ();
						
						if (childCustomizationData)
						{
							//-- make a link from child's /shared_owner directory to owner's /shared_owner/ customization variable directory
							IGNORE_RETURN(childCustomizationData->mountRemoteCustomizationData(*rootCustomizationData, "/shared_owner/", "/shared_owner", true));
							childCustomizationData->release ();
						}
					}
				}
				
				(*m_componentAttachments) [chassisSlot].push_back (ObjectWatcher (child));
				
				//--
				//-- find the ClientDataFile for the component
				//--
				
				m_ship->getShipObjectEffects().resetEngineEffects(child, chassisSlot);
			}
			
			if (ot != NULL)
				ot->releaseReference();
		}
	}
	
	if (rootCustomizationData)
		rootCustomizationData->release ();

	m_ship->getShipObjectEffects().resetContrails();
	m_ship->getShipObjectEffects().updateGlowLists();
}

//----------------------------------------------------------------------

void ShipObjectAttachments::removeComponentAttachments (int chassisSlot)
{
	DEBUG_FATAL((chassisSlot < 0 || chassisSlot >= static_cast<int>(ShipChassisSlotType::SCST_num_types)), ("ShipObject::removeComponentAttachments chassisSlot [%d] out of range.", chassisSlot));

	WatcherVectorMap::iterator const wvm_it = m_componentAttachments->find (chassisSlot);

	if (wvm_it != m_componentAttachments->end ())
	{
		WatcherVector & wv = (*wvm_it).second;

		for (WatcherVector::iterator it = wv.begin (); it != wv.end (); ++it)
		{
			ObjectWatcher & watcher = *it;

			Object * const obj    = watcher.getPointer ();

			if (obj == NULL)
				continue;

			Object * const parent = obj->getParent ();

			if (parent)
			{
				TangibleObject * const tangible = dynamic_cast<TangibleObject *>(obj);

				if (tangible)
				{
					CustomizationData * const childCustomizationData = tangible->fetchCustomizationData ();

					if (childCustomizationData)
					{
						//-- remove the link from child's /shared_owner directory to owner's /shared_owner/ customization variable directory
						IGNORE_RETURN(childCustomizationData->dismountRemoteCustomizationData("/shared_owner", true));
						childCustomizationData->release ();
					}
				}

				obj->kill ();
			}
		}

		m_componentAttachments->erase (wvm_it);

		//--
		//-- remove the thrusters if applicable
		//--

		if (ShipChassisSlotType::getComponentTypeForSlot(static_cast<ShipChassisSlotType::Type>(chassisSlot)) == ShipComponentType::SCT_engine)
		{
			m_ship->getShipObjectEffects().removeThrusterEffects();
		}
	}

	if (ShipChassisSlotType::getComponentTypeForSlot(static_cast<ShipChassisSlotType::Type>(chassisSlot)) == ShipComponentType::SCT_booster)
	{
		m_ship->getShipObjectEffects().removeBoosterEffect();
	}
	
	//-- remove the ONOFF flag on the ship when there is no engine
	if (ShipChassisSlotType::getComponentTypeForSlot(static_cast<ShipChassisSlotType::Type>(chassisSlot)) == ShipComponentType::SCT_engine)
	{
		m_ship->clearCondition(TangibleObject::C_onOff);
	}

}

//----------------------------------------------------------------------

bool ShipObjectAttachments::findAttachmentHardpoints_o (int chassisSlot, std::string const & hardpointPrefixString, Transform const & parentTransform_w, TransformObjectList & transformObjectList) const
{
	//-- find all hardpoints on the chassis if an invalid slot is specified
	if (chassisSlot < 0)
	{
		findHardpoints_o (*m_ship, hardpointPrefixString, parentTransform_w, transformObjectList);
		return true;
	}

	if (!m_ship->isSlotInstalled (chassisSlot))
		return false;

	WatcherVectorMap::iterator const wvm_it = m_componentAttachments->find (chassisSlot);

	if (wvm_it != m_componentAttachments->end ())
	{
		WatcherVector & wv = (*wvm_it).second;

		for (WatcherVector::iterator it = wv.begin (); it != wv.end (); ++it)
		{
			ObjectWatcher & watcher = *it;

			Object * const obj    = watcher.getPointer ();

			if (obj != NULL)
			{
				findHardpoints_o (*obj, hardpointPrefixString, parentTransform_w, transformObjectList);
			}
		}
	}

	return true;
}

//----------------------------------------------------------------------

void ShipObjectAttachments::handleComponentDestruction(int const chassisSlot, float const severity)
{
	WatcherVectorMap::iterator const wvm_it = m_componentAttachments->find(chassisSlot);
	if (wvm_it != m_componentAttachments->end())
	{
		WatcherVector & watcherVector = (*wvm_it).second;

		for (WatcherVector::iterator wv_it = watcherVector.begin(); wv_it != watcherVector.end(); ++wv_it)
		{
			ObjectWatcher & objectWatcher = *wv_it;
			Object * const attachedObject = objectWatcher.getPointer();

			if (attachedObject == NULL)
				continue;
			
//-- code for duplicating particles from the destroyed component onto the ship
#if 0
			//-- find & copy particles
			int const attachedChildCount = attachedObject->getNumberOfChildObjects();
			for (int i = 0; i < attachedChildCount; ++i)
			{
				Object * const attachedChild = attachedObject->getChildObject(i);
				if (NULL != attachedChild)
				{
					char * const debugName = attachedChild->getDebugName();
					if (NULL != debugName)
					{
						//-- ignore thruster particles
						if (!_stricmp(debugName, "thruster"))
							continue;
					}

					Appearance * const attachedChildAppearance = attachedChild->getAppearance();
					ParticleEffectAppearance * const attachedChildParticleEffectAppearance = (NULL != attachedChildAppearance) ? attachedChildAppearance->asParticleEffectAppearance() : NULL;
					if (NULL != attachedChildParticleEffectAppearance)
					{
						AppearanceTemplate const * const at = attachedChildParticleEffectAppearance->getAppearanceTemplate();
						
						if (NULL != at)
						{
							ParticleEffectAppearance * const newParticleAppearance = safe_cast<ParticleEffectAppearance *>(at->createAppearance());
							if (newParticleAppearance != NULL)
							{
								newParticleAppearance->setUniformScale(attachedChildParticleEffectAppearance->getUniformScale());
								newParticleAppearance->setColorModifier(attachedChildParticleEffectAppearance->getColorModifier());
								Object * const newParticleObject = new Object;
								
								newParticleObject->setAppearance(newParticleAppearance);
								
								Object * const parentObject = attachedObject->getParent();
								
								newParticleObject->setTransform_o2p(attachedObject->getTransform_o2p().rotateTranslate_l2p(attachedChild->getTransform_o2p()));

								//-- @todo: transform the new particle along a ray toward the ship, putting it on a surface
								RenderWorld::addObjectNotifications (*newParticleObject);
								
								parentObject->addChildObject_o(newParticleObject);								
								(*m_componentDestroyedParticles)[chassisSlot].push_back(ObjectWatcher(newParticleObject));
							}
						}
					}
				}
			}
#endif
			
			m_ship->splitShipComponent(*attachedObject, 2, severity);

			ClientObject * const clientAttachedObject = attachedObject->asClientObject();
			if (clientAttachedObject != NULL)
			{
				ClientDataFile const * const clientDataFile = clientAttachedObject->getClientData();
				if (clientDataFile != NULL)
					clientDataFile->playDestructionEffect(*clientAttachedObject);
			}
		}
	}

	m_ship->getShipObjectEffects().releaseEffectsForSlot(chassisSlot);

	
}

//----------------------------------------------------------------------

ShipObjectAttachments::WatcherVector const * ShipObjectAttachments::getComponentAttachments(int const chassisSlot) const
{
	WatcherVectorMap::const_iterator const fit = m_componentAttachments->find(chassisSlot);
	if (fit != m_componentAttachments->end())
		return &(*fit).second;

	return NULL;
}

//----------------------------------------------------------------------

ShipObjectAttachments::WatcherVectorMap const * ShipObjectAttachments::getAllComponentAttachments() const
{
	return m_componentAttachments;
}

//----------------------------------------------------------------------

void ShipObjectAttachments::recalculateAverageProjectileData() const
{
	m_averageProjectileDataDirty = false;
	m_averageProjectileMuzzlePosition_p = Vector::zero;
	m_averageProjectileRange = 0.0f;

	if (NULL == m_ship)
		return;

	int positionCount = 0;
	int rangeCount = 0;
		
	TransformObjectList transformObjectList;

	for (int weaponIndex = 0; weaponIndex < ShipChassisSlotType::SCST_num_explicit_types; ++weaponIndex)
	{
		int const shipChassisSlotType = ShipChassisSlotType::SCST_weapon_first + weaponIndex;
		uint32 const crc = m_ship->getComponentCrc(shipChassisSlotType);
		
		if (0 != crc)
		{
			if (ShipTurretManager::isTurret(m_ship->getChassisType(), weaponIndex))
				continue;

			if (ShipComponentWeaponManager::isCountermeasure(crc))
				continue;

			if (ShipComponentWeaponManager::isMissile(crc))
				continue;
			
			if (!m_ship->isComponentFunctional(shipChassisSlotType))
				continue;

			// calculate an average range.
			float const weaponRange = m_ship->getWeaponRange(weaponIndex);
			if (weaponRange > 0.0f) 
			{
				if (rangeCount)
				{
					m_averageProjectileRange += weaponRange;
				}
				else
				{
					m_averageProjectileRange = weaponRange;
				}
				rangeCount++;
			}
			
			// calculate the average muzzle location.
			transformObjectList.clear();
			if (findAttachmentHardpoints_o(shipChassisSlotType, s_muzzleHardpointName, m_ship->getTransform_o2w(), transformObjectList) ||
				findAttachmentHardpoints_o(-1, s_muzzleHardpointName, m_ship->getTransform_o2w(), transformObjectList))
			{
				for (TransformObjectList::const_iterator it = transformObjectList.begin(); it != transformObjectList.end(); ++it)
				{
					Transform const & transform = (*it).first;
					m_averageProjectileMuzzlePosition_p += transform.getPosition_p();
					++positionCount;
				}
			}
		}
	}

	if (positionCount > 1)
	{
		m_averageProjectileMuzzlePosition_p /= static_cast<float>(positionCount);
	}

	if (rangeCount > 1) 
	{
		m_averageProjectileRange /= static_cast<float>(rangeCount);
	}
}

//----------------------------------------------------------------------

Vector const & ShipObjectAttachments::getAverageProjectileMuzzlePosition_p() const
{
	if (m_averageProjectileDataDirty)
		recalculateAverageProjectileData();

	return m_averageProjectileMuzzlePosition_p;
}

//----------------------------------------------------------------------

float ShipObjectAttachments::getAverageProjectileRange() const
{
	if (m_averageProjectileDataDirty)
		recalculateAverageProjectileData();

	return m_averageProjectileRange;
}


//======================================================================

