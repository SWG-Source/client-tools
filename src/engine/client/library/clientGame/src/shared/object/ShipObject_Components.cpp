//======================================================================
//
// ShipObject_Components.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ShipObject_Components.h"

#include "clientGame/Game.h"
#include "clientGame/MissileManager.h"
#include "clientGame/ResourceTypeManager.h"
#include "clientGame/ShipComponentDataManager.h"
#include "clientGame/ShipController.h"
#include "clientGame/ShipObjectEffects.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedGame/ShipChassis.h"
#include "sharedGame/ShipChassisSlot.h"
#include "sharedGame/ShipChassisSlotType.h"
#include "sharedGame/ShipComponentData.h"
#include "sharedGame/ShipComponentDescriptor.h"
#include "sharedGame/ShipComponentFlags.h"
#include "sharedGame/ShipComponentWeaponManager.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/ObjectTemplate.h"

//----------------------------------------------------------------------

namespace ShipObject_ComponentsNamespace
{
	namespace Transceivers
	{
		MessageDispatch::Transceiver<ShipObject::Messages::LookAtTargetSlotChanged::Payload &, ShipObject::Messages::LookAtTargetSlotChanged> lookAtTargetSlotChanged;
	}

	void enableTargetComponentAppearanceFromObjectId(CachedNetworkId const & objectid, int targetSlot)
	{
		UNREF(targetSlot);
		//-- Dis/Enable the target blip on this object.
		Object * const object = objectid.getObject();
		if (object)
		{
			ClientObject * const clientObject = object->asClientObject();
			if(clientObject)
			{
				ShipObject * const shipObject = clientObject->asShipObject();
				if (shipObject)
				{
					shipObject->getShipObjectEffects().setTargetAppearanceActive(true); 
					shipObject->getShipObjectEffects().setTargetAcquiredActive(false);
					shipObject->getShipObjectEffects().setTargetAcquiringActive(false, 0.0f);
				}
			}
		}
	}

	int ms_lastLookAtTargetSlotRequest;
}

using namespace ShipObject_ComponentsNamespace;

//----------------------------------------------------------------------

uint32 ShipObject::getChassisType () const
{
	return m_chassisType.get ();
}

//----------------------------------------------------------------------

/**
 * Returns [0.0, 1.0] relating to the hitpoints of the chassis and all installed components
 * 1.0 == pristine, 0.0 == totally broken (though there are other rules to determine if a ship is flyable)
 */
float ShipObject::getOverallHealth() const
{
	float total = 0.0f;
	int numComponents = 0;
	if(getMaximumChassisHitPoints() > 0.0)
	{
		total += getCurrentChassisHitPoints() / getMaximumChassisHitPoints();
		++numComponents;
	}

	for(int i = ShipChassisSlotType::SCST_first; i != ShipChassisSlotType::SCST_num_types; ++i)
	{
		if(isSlotInstalled(i))
		{
			if(getComponentHitpointsMaximum(i) > 0.0f)
			{
				total += getComponentHitpointsCurrent(i) / getComponentHitpointsMaximum(i);
				++numComponents;
			}
		}
	}

	if(numComponents <= 0)
		return 0.0f;
	else
		return clamp(0.0f, total / numComponents, 1.0f);
}

//----------------------------------------------------------------------

float ShipObject::getShieldHealth() const
{
	float front = 0.0f;
	float back = 0.0f;

	float const shieldHitpointsFrontMaximum = getShieldHitpointsFrontMaximum();
	if (shieldHitpointsFrontMaximum != 0.0)
		front = getShieldHitpointsFrontCurrent() / shieldHitpointsFrontMaximum;		

	float const shieldHitpointsBackMaximum = getShieldHitpointsBackMaximum();
	if (shieldHitpointsBackMaximum != 0.0)
		back = getShieldHitpointsBackCurrent() / shieldHitpointsBackMaximum;

	return (front + back) / 2.0f;
}

//----------------------------------------------------------------------

float ShipObject::getArmorHealth() const
{
	int numberObjectsToWeight = 0;
	float total = 0.0f;

	float const componentArmorHitpointsMaximum0 = getComponentArmorHitpointsMaximum(ShipChassisSlotType::SCST_armor_0);
	if (componentArmorHitpointsMaximum0 != 0.0)
	{
		total += getComponentArmorHitpointsCurrent(ShipChassisSlotType::SCST_armor_0) / componentArmorHitpointsMaximum0;
		++numberObjectsToWeight;
	}

	float const componentArmorHitpointsMaximum1 = getComponentArmorHitpointsMaximum(ShipChassisSlotType::SCST_armor_1);
	if (componentArmorHitpointsMaximum1 != 0.0)
	{
		total += getComponentArmorHitpointsCurrent(ShipChassisSlotType::SCST_armor_1) / componentArmorHitpointsMaximum1;
		++numberObjectsToWeight;
	}

	return (numberObjectsToWeight==0) ? 0 : total / numberObjectsToWeight;
}

//----------------------------------------------------------------------

float ShipObject::getOverallHealthWithShieldsAndArmor() const
{
	int numberObjectsToWeight = 0;

	float total = getOverallHealth();
	++numberObjectsToWeight;

	float const shieldHitpointsFrontMaximum = getShieldHitpointsFrontMaximum();
	if (shieldHitpointsFrontMaximum != 0.0)
	{
		total += getShieldHitpointsFrontCurrent() / shieldHitpointsFrontMaximum;
		++numberObjectsToWeight;
	}

	float const shieldHitpointsBackMaximum = getShieldHitpointsBackMaximum();
	if (shieldHitpointsBackMaximum != 0.0)
	{
		total += getShieldHitpointsBackCurrent() / shieldHitpointsBackMaximum;
		++numberObjectsToWeight;
	}

	float const componentArmorHitpointsMaximum0 = getComponentArmorHitpointsMaximum(ShipChassisSlotType::SCST_armor_0);
	if (componentArmorHitpointsMaximum0 != 0.0)
	{
		total += getComponentArmorHitpointsCurrent(ShipChassisSlotType::SCST_armor_0) / componentArmorHitpointsMaximum0;
		++numberObjectsToWeight;
	}

	float const componentArmorHitpointsMaximum1 = getComponentArmorHitpointsMaximum(ShipChassisSlotType::SCST_armor_1);
	if (componentArmorHitpointsMaximum1 != 0.0)
	{
		total += getComponentArmorHitpointsCurrent(ShipChassisSlotType::SCST_armor_1) / componentArmorHitpointsMaximum1;
		++numberObjectsToWeight;
	}

	return clamp(0.0f, total / static_cast<float>(numberObjectsToWeight), 1.0f);
}

//----------------------------------------------------------------------

float ShipObject::getComponentArmorHitpointsMaximum (int chassisSlot) const
{
	Archive::AutoDeltaMap<int, float>::const_iterator const it = m_componentArmorHitpointsMaximum.find (chassisSlot);
	if (it != m_componentArmorHitpointsMaximum.end ())
		return (*it).second;
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getComponentArmorHitpointsCurrent (int chassisSlot) const
{
	Archive::AutoDeltaMap<int, float>::const_iterator const it = m_componentArmorHitpointsCurrent.find (chassisSlot);
	if (it != m_componentArmorHitpointsCurrent.end ())
		return (*it).second;
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getComponentEfficiencyGeneral (int chassisSlot) const
{
	Archive::AutoDeltaMap<int, float>::const_iterator const it = m_componentEfficiencyGeneral.find (chassisSlot);
	if (it != m_componentEfficiencyGeneral.end ())
		return (*it).second;
	return 1.0f;
}

//----------------------------------------------------------------------

float ShipObject::getComponentEfficiencyEnergy (int chassisSlot) const
{
	Archive::AutoDeltaMap<int, float>::const_iterator const it = m_componentEfficiencyEnergy.find (chassisSlot);
	if (it != m_componentEfficiencyEnergy.end ())
		return (*it).second;
	return 1.0f;
}

//----------------------------------------------------------------------

float ShipObject::getComponentEnergyMaintenanceRequirement (int chassisSlot) const
{
	Archive::AutoDeltaMap<int, float>::const_iterator const it = m_componentEnergyMaintenanceRequirement.find (chassisSlot);
	if (it != m_componentEnergyMaintenanceRequirement.end ())
		return (*it).second;
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getComponentMass (int chassisSlot) const
{
	Archive::AutoDeltaMap<int, float>::const_iterator const it = m_componentMass.find (chassisSlot);
	if (it != m_componentMass.end ())
		return (*it).second;
	return 0.0f;
}

//----------------------------------------------------------------------

uint32 ShipObject::getComponentCrc (int chassisSlot) const
{
	Archive::AutoDeltaMap<int, uint32>::const_iterator const it = m_componentCrc.find (chassisSlot);
	if (it != m_componentCrc.end ())
		return (*it).second;
	return 0;
}

//----------------------------------------------------------------------

float ShipObject::getComponentHitpointsCurrent (int chassisSlot) const
{
	Archive::AutoDeltaMap<int, float>::const_iterator const it = m_componentHitpointsCurrent.find (chassisSlot);
	if (it != m_componentHitpointsCurrent.end ())
		return (*it).second;
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getComponentHitpointsMaximum (int chassisSlot) const
{
	Archive::AutoDeltaMap<int, float>::const_iterator const it = m_componentHitpointsMaximum.find (chassisSlot);
	if (it != m_componentHitpointsMaximum.end ())
		return (*it).second;
	return 0.0f;
}

//----------------------------------------------------------------------

int ShipObject::getComponentFlags (int chassisSlot) const
{
	Archive::AutoDeltaMap<int, int>::const_iterator const it = m_componentFlags.find (chassisSlot);
	if (it != m_componentFlags.end ())
		return (*it).second;
	return 0;
}

//----------------------------------------------------------------------

Unicode::String ShipObject::getComponentName(int chassisSlot) const
{
	//if the name has been overridden, return that
	Archive::AutoDeltaMap<int, Unicode::String>::const_iterator const it = m_componentNames.find (chassisSlot);
	if (it != m_componentNames.end () && !(*it).second.empty())
		return (*it).second;

	Unicode::String result;
	//otherwise, look for the default name from the template
	ShipComponentDescriptor const * const shipComponentDescriptor = ShipComponentDescriptor::findShipComponentDescriptorByCrc (getComponentCrc(chassisSlot));
	if(shipComponentDescriptor)
	{
		ObjectTemplate const * const t = shipComponentDescriptor->getSharedObjectTemplate();
		if(t)
		{
			SharedObjectTemplate const * const sharedTemplate = t->asSharedObjectTemplate();
			if(sharedTemplate)
				result = sharedTemplate->getObjectName().localize();
		}
	}

	return result;
}

//----------------------------------------------------------------------

float ShipObject::getWeaponDamageMaximum (int chassisSlot) const
{
	Archive::AutoDeltaMap<int, float>::const_iterator const it = m_weaponDamageMaximum.find (chassisSlot);
	if (it != m_weaponDamageMaximum.end ())
		return (*it).second;
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getWeaponDamageMinimum (int chassisSlot) const
{
	Archive::AutoDeltaMap<int, float>::const_iterator const it = m_weaponDamageMinimum.find (chassisSlot);
	if (it != m_weaponDamageMinimum.end ())
		return (*it).second;
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getWeaponEffectivenessShields (int chassisSlot) const
{
	Archive::AutoDeltaMap<int, float>::const_iterator const it = m_weaponEffectivenessShields.find (chassisSlot);
	if (it != m_weaponEffectivenessShields.end ())
		return (*it).second;
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getWeaponEffectivenessArmor (int chassisSlot) const
{
	Archive::AutoDeltaMap<int, float>::const_iterator const it = m_weaponEffectivenessArmor.find (chassisSlot);
	if (it != m_weaponEffectivenessArmor.end ())
		return (*it).second;
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getWeaponEnergyPerShot (int chassisSlot) const
{
	Archive::AutoDeltaMap<int, float>::const_iterator const it = m_weaponEnergyPerShot.find (chassisSlot);
	if (it != m_weaponEnergyPerShot.end ())
		return (*it).second;
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getWeaponActualEnergyPerShot(int chassisSlot) const
{
	float const weaponEfficiencyEnergy = std::max(0.1f, getComponentEfficiencyEnergy(chassisSlot));
	return getWeaponEnergyPerShot(chassisSlot) / weaponEfficiencyEnergy;
}

//----------------------------------------------------------------------

float ShipObject::getWeaponRefireRate (int chassisSlot) const
{
	Archive::AutoDeltaMap<int, float>::const_iterator const it = m_weaponRefireRate.find (chassisSlot);
	if (it != m_weaponRefireRate.end ())
		return (*it).second;
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getWeaponEfficiencyRefireRate (int chassisSlot) const
{
	Archive::AutoDeltaMap<int, float>::const_iterator const it = m_weaponEfficiencyRefireRate.find (chassisSlot);
	if (it != m_weaponEfficiencyRefireRate.end ())
		return (*it).second;
	return 1.0f;
}

//----------------------------------------------------------------------

float ShipObject::getWeaponActualRefireRate (int chassisSlot) const
{
	if (!isSlotInstalled(chassisSlot))
		return 0.0f;

	float const eff = clamp(0.1f, getWeaponEfficiencyRefireRate(chassisSlot), 10.0f);
	return getWeaponRefireRate(chassisSlot) / eff;
}

//----------------------------------------------------------------------

int ShipObject::getWeaponAmmoCurrent(int chassisSlot) const
{
	Archive::AutoDeltaMap<int, int>::const_iterator const it = m_weaponAmmoCurrent.find (chassisSlot);
	if (it != m_weaponAmmoCurrent.end ())
		return (*it).second;
	return 0;
}

//----------------------------------------------------------------------

int ShipObject::getWeaponAmmoMaximum(int chassisSlot) const
{
	Archive::AutoDeltaMap<int, int>::const_iterator const it = m_weaponAmmoMaximum.find (chassisSlot);
	if (it != m_weaponAmmoMaximum.end ())
		return (*it).second;
	return 0;
}

//----------------------------------------------------------------------

int ShipObject::getWeaponAmmoType(int chassisSlot) const
{
	Archive::AutoDeltaMap<int, uint32>::const_iterator const it = m_weaponAmmoType.find (chassisSlot);
	if (it != m_weaponAmmoType.end ())
		return static_cast<int>((*it).second);
	return -1;
}

//----------------------------------------------------------------------

float ShipObject::getShieldHitpointsFrontCurrent () const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_shield_0))
		return m_shieldHitpointsFrontCurrent.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getShieldHitpointsFrontMaximum () const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_shield_0))
		return m_shieldHitpointsFrontMaximum.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getShieldHitpointsBackCurrent () const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_shield_0))
		return m_shieldHitpointsBackCurrent.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getShieldHitpointsBackMaximum () const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_shield_0))
		return m_shieldHitpointsBackMaximum.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getShieldRechargeRate () const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_shield_0))
		return m_shieldRechargeRate.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getCapacitorEnergyCurrent () const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_capacitor))
		return m_capacitorEnergyCurrent.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getCapacitorEnergyMaximum () const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_capacitor))
		return m_capacitorEnergyMaximum.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getCapacitorEnergyRechargeRate () const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_capacitor))
		return m_capacitorEnergyRechargeRate.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getEngineAccelerationRate () const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_engine))
		return m_engineAccelerationRate.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getEngineDecelerationRate () const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_engine))
		return m_engineDecelerationRate.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getEnginePitchAccelerationRate () const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_engine))
		return m_enginePitchAccelerationRate.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getEngineYawAccelerationRate () const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_engine))
		return m_engineYawAccelerationRate.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getEngineRollAccelerationRate () const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_engine))
		return m_engineRollAccelerationRate.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getEnginePitchRateMaximum () const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_engine))
		return m_enginePitchRateMaximum.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getEngineYawRateMaximum () const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_engine))
		return m_engineYawRateMaximum.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getEngineRollRateMaximum () const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_engine))
		return m_engineRollRateMaximum.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getEngineSpeedMaximum () const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_engine))
		return m_engineSpeedMaximum.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getReactorEnergyGenerationRate () const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_reactor))
		return m_reactorEnergyGenerationRate.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getBoosterEnergyCurrent () const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_booster))
		return m_boosterEnergyCurrent.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getBoosterEnergyMaximum () const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_booster))
		return m_boosterEnergyMaximum.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getBoosterEnergyRechargeRate () const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_booster))
		return m_boosterEnergyRechargeRate.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getBoosterEnergyConsumptionRate () const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_booster))
		return m_boosterEnergyConsumptionRate.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getBoosterAcceleration () const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_booster))
		return m_boosterAcceleration.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getBoosterSpeedMaximum () const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_booster))
		return m_boosterSpeedMaximum.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

float ShipObject::getDroidInterfaceCommandSpeed () const
{
	if (isSlotInstalled (ShipChassisSlotType::SCST_droid_interface))
		return m_droidInterfaceCommandSpeed.get ();
	return 0.0f;
}

//----------------------------------------------------------------------

ShipComponentData * ShipObject::createShipComponentData (int const chassisSlot) const
{
	ShipComponentDescriptor const * shipComponentDescriptor = NULL;

	uint32 const componentCrc = getComponentCrc (chassisSlot);
	if (componentCrc)
	{
		shipComponentDescriptor   = ShipComponentDescriptor::findShipComponentDescriptorByCrc (componentCrc);
		if (shipComponentDescriptor == NULL)
		{
			WARNING (true, ("ShipObject::createShipComponentData failed for slot [%d] because component crc [%d] could not map to a component descriptor", chassisSlot, componentCrc));
			return NULL;
		}
	}
	else
		return NULL;

	ShipComponentData * const shipComponentData = ShipComponentDataManager::create (*shipComponentDescriptor);
	if (shipComponentData == NULL)
	{
		WARNING (true, ("ShipObject::createShipComponentData failed for slot [%d] ship Component data could not be constructed", chassisSlot));
		return NULL;
	}

	if (!shipComponentData->readDataFromShip (chassisSlot, *this))
		WARNING (true, ("ShipObject::createShipComponentData failed for read data from the ship"));

	return shipComponentData;
}

//----------------------------------------------------------------------

bool ShipObject::isSlotInstalled                          (int chassisSlot) const
{
	return getComponentCrc (chassisSlot) != 0;
}

//----------------------------------------------------------------------

bool ShipObject::isBoosterActive                   () const
{
	return isComponentActive(ShipChassisSlotType::SCST_booster);
}

//----------------------------------------------------------------------

float ShipObject::getShipActualAccelerationRate       () const
{
	return m_shipActualAccelerationRate.get();
}

//----------------------------------------------------------------------

float ShipObject::getShipActualDecelerationRate       () const
{
	return m_shipActualDecelerationRate.get();
}

//----------------------------------------------------------------------

float ShipObject::getShipActualPitchAccelerationRate  () const
{
	return m_shipActualPitchAccelerationRate.get();
}

//----------------------------------------------------------------------

float ShipObject::getShipActualYawAccelerationRate    () const
{
	return m_shipActualYawAccelerationRate.get();
}

//----------------------------------------------------------------------

float ShipObject::getShipActualRollAccelerationRate   () const
{
	return m_shipActualRollAccelerationRate.get();
}

//----------------------------------------------------------------------

float ShipObject::getShipActualPitchRateMaximum       () const
{
	return m_shipActualPitchRateMaximum.get();
}

//----------------------------------------------------------------------

float ShipObject::getShipActualYawRateMaximum         () const
{
	return m_shipActualYawRateMaximum.get();
}

//----------------------------------------------------------------------

float ShipObject::getShipActualRollRateMaximum        () const
{
	return m_shipActualRollRateMaximum.get();
}

//----------------------------------------------------------------------

float ShipObject::getShipActualSpeedMaximum           () const
{
	return m_shipActualSpeedMaximum.get();
}

//----------------------------------------------------------------------

bool ShipObject::isComponentDisabled                      (int chassisSlot) const
{
	return (getComponentFlags (chassisSlot) & static_cast<int>(ShipComponentFlags::F_disabled)) != 0;
}

//----------------------------------------------------------------------

bool ShipObject::isComponentLowPower                      (int chassisSlot) const
{
	return (getComponentFlags (chassisSlot) & static_cast<int>(ShipComponentFlags::F_lowPower)) != 0;
}

//----------------------------------------------------------------------

bool ShipObject::isComponentActive(int chassisSlot) const
{
	return (getComponentFlags (chassisSlot) & static_cast<int>(ShipComponentFlags::F_active)) != 0;
}

//----------------------------------------------------------------------

bool ShipObject::isComponentDemolished(int chassisSlot) const
{
	return (getComponentFlags(chassisSlot) & static_cast<int>(ShipComponentFlags::F_demolished)) != 0;
}

//----------------------------------------------------------------------

int ShipObject::getProjectileIndexForWeapon(int const weaponIndex) const
{
	if (weaponIndex < 0 || weaponIndex >= ShipChassisSlotType::cms_numWeaponIndices)
		return -1;

	ShipChassisSlotType::Type const weaponChassisSlotType = static_cast<ShipChassisSlotType::Type>(weaponIndex + static_cast<int>(ShipChassisSlotType::SCST_weapon_0));

	uint32 const componentCrc = getComponentCrc(weaponChassisSlotType);
	if (componentCrc == 0)
		return -1;

	return ShipComponentWeaponManager::getProjectileIndex(componentCrc);
}

//----------------------------------------------------------------------

float ShipObject::getWeaponRange(int weaponIndex) const
{
	if (weaponIndex < 0 || weaponIndex >= ShipChassisSlotType::cms_numWeaponIndices)
		return 0.0f;

	ShipChassisSlotType::Type const weaponChassisSlotType = static_cast<ShipChassisSlotType::Type>(weaponIndex + static_cast<int>(ShipChassisSlotType::SCST_weapon_0));

	uint32 const componentCrc = getComponentCrc(weaponChassisSlotType);
	if (componentCrc == 0)
		return 0.0f;

	float range = 0.0f;

	if (isMissile(weaponIndex))
	{
		int const missileTypeId = getWeaponAmmoType(weaponChassisSlotType);
		range = MissileManager::getRange(missileTypeId);
	}
	else
	{
		range = ShipComponentWeaponManager::getRange(componentCrc);
	}

	return range;
}

//----------------------------------------------------------------------

float ShipObject::getWeaponProjectileSpeed(int weaponIndex) const
{
	if (weaponIndex < 0 || weaponIndex >= ShipChassisSlotType::cms_numWeaponIndices)
		return 0.0f;

	ShipChassisSlotType::Type const weaponChassisSlotType = static_cast<ShipChassisSlotType::Type>(weaponIndex + static_cast<int>(ShipChassisSlotType::SCST_weapon_0));

	uint32 const componentCrc = getComponentCrc(weaponChassisSlotType);
	if (componentCrc == 0)
		return 0.0f;
	
	float speed = 0.0f;
	
	if (isMissile(weaponIndex))
	{
		int const missileTypeId = getWeaponAmmoType(weaponChassisSlotType);
		speed = MissileManager::getSpeed(missileTypeId);
	}
	else
	{
		speed = ShipComponentWeaponManager::getProjectileSpeed(componentCrc);
	}

	return speed;
}
//----------------------------------------------------------------------

float ShipObject::computeWeaponProjectileTimeToLive(int weaponIndex) const
{
	if (weaponIndex < 0 || weaponIndex >= ShipChassisSlotType::cms_numWeaponIndices)
		return 0.0f;

	ShipChassisSlotType::Type const weaponChassisSlotType = static_cast<ShipChassisSlotType::Type>(weaponIndex + static_cast<int>(ShipChassisSlotType::SCST_weapon_0));

	uint32 const componentCrc = getComponentCrc(weaponChassisSlotType);
	if (componentCrc == 0)
		return 0.0f;

	float timeToLive = 0.0f;

	if (isMissile(weaponIndex))
	{
		int const missileTypeId = getWeaponAmmoType(weaponChassisSlotType);
		timeToLive = MissileManager::getTime(missileTypeId);
	}
	else
	{
		float const range = ShipComponentWeaponManager::getRange(componentCrc);
		float const projectileSpeed = ShipComponentWeaponManager::getProjectileSpeed(componentCrc);
		timeToLive = (projectileSpeed <= 0.0f) ? 0.0f : (range / projectileSpeed);
	}

	return timeToLive;
}

//----------------------------------------------------------------------

float ShipObject::computeActualComponentEfficiencyGeneral(int chassisType) const
{
	float efficiency = getComponentEfficiencyGeneral(chassisType);
	float nebulaEffect = 0.0f;

	switch (chassisType)
	{
	case ShipChassisSlotType::SCST_engine:
		nebulaEffect = m_nebulaEffectEngine;
		break;
	case ShipChassisSlotType::SCST_reactor:
		nebulaEffect = m_nebulaEffectReactor;
		break;
	case ShipChassisSlotType::SCST_shield_0:
		nebulaEffect = m_nebulaEffectShields;
		break;
	default:
		break;
	}

	efficiency = clamp(0.1f, efficiency + nebulaEffect, 10.0f);
	return efficiency;
}

//----------------------------------------------------------------------

bool ShipObject::clientSetComponentDisabled(int chassisSlot, bool disabled)
{
	if (isSlotInstalled(chassisSlot))
	{
		int flags = getComponentFlags(chassisSlot);
		if (disabled)
			flags |= ShipComponentFlags::F_disabled;
		else
			flags &= ~ShipComponentFlags::F_disabled;

		return clientSetComponentFlags(chassisSlot, flags);
	}

	return false;
}

//----------------------------------------------------------------------

bool ShipObject::clientSetComponentLowPower(int chassisSlot, bool lowPower)
{
	if (isSlotInstalled(chassisSlot))
	{
		int flags = getComponentFlags(chassisSlot);
		if (lowPower)
			flags |= ShipComponentFlags::F_lowPower;
		else
			flags &= ~ShipComponentFlags::F_lowPower;

		return clientSetComponentFlags(chassisSlot, flags);
	}

	return false;
}

//----------------------------------------------------------------------

bool ShipObject::clientSetComponentActive(int chassisSlot, bool active)
{
	if (isSlotInstalled(chassisSlot))
	{
		int flags = getComponentFlags(chassisSlot);
		if (active)
			flags |= ShipComponentFlags::F_active;
		else
			flags &= ~ShipComponentFlags::F_active;

		return clientSetComponentFlags(chassisSlot, flags);
	}

	return false;
}

//----------------------------------------------------------------------

void ShipObject::clientSetComponentCrc(int const chassisSlot, uint32 const componentCrc)
{
	if (componentCrc == 0)
		IGNORE_RETURN(m_componentCrc.erase(chassisSlot));
	else
		m_componentCrc.set(chassisSlot, componentCrc);
}

//----------------------------------------------------------------------

bool ShipObject::clientSetComponentFlags(int chassisSlot, int flags)
{
	if (isSlotInstalled(chassisSlot))
	{
		m_componentFlags.set(chassisSlot, flags);
		return true;
	}

	return false;
}

//----------------------------------------------------------------------

bool ShipObject::clientSetComponentHitPoints(int chassisSlot, float cur, float max)
{
	if (isSlotInstalled(chassisSlot))
	{
		m_componentHitpointsCurrent.set(chassisSlot, cur);
		m_componentHitpointsMaximum.set(chassisSlot, max);
		return true;
	}

	return false;
}

//----------------------------------------------------------------------

bool ShipObject::clientSetComponentArmorHitPoints(int chassisSlot, float cur, float max)
{
	if (isSlotInstalled(chassisSlot))
	{
		m_componentArmorHitpointsCurrent.set(chassisSlot, cur);
		m_componentArmorHitpointsMaximum.set(chassisSlot, max);
		return true;
	}

	return false;
}
//----------------------------------------------------------------------

void ShipObject::setPilotLookAtTargetSlot (ShipChassisSlotType::Type slot)
{
	if (Game::getPlayerPilotedShip() == this)
	{
		ms_lastLookAtTargetSlotRequest = slot;

		if (getController()) 
		{
			NON_NULL(safe_cast<ShipController *>(getController()))->setLookAtTargetSlot(slot);
		}
	}
	
	//immediately set the component target on the client, since setting it on the server should never fail
	m_pilotLookAtTargetSlot.set(slot);
}

//----------------------------------------------------------------------

void ShipObject::updateTargetedSlot()
{
	// An equivalent function may need to be added to CreatureObject:

	Object const * const targetedObject = getPilotLookAtTarget().getObject();
	ClientObject const * const targetedClientObject = targetedObject ? targetedObject->asClientObject() : NULL;
	ShipObject const * const targetShip = targetedClientObject ? targetedClientObject->asShipObject() : NULL;
	if(!targetShip)
	{
		setPilotLookAtTargetSlot(ShipChassisSlotType::SCST_num_types);
		return;
	}

	ShipChassisSlotType::Type const currentSlot = getPilotLookAtTargetSlot();
	//targeting a "none" slot is always valid
	if(currentSlot == ShipChassisSlotType::SCST_num_types)
		return;

	//select the next slot if the current slot isn't a valid target
	if(!targetShip->isValidTargetableSlot(currentSlot))
		setPilotLookAtTargetSlot(targetShip->getNextValidTargetableSlot(currentSlot));
}

//----------------------------------------------------------------------

bool ShipObject::isValidTargetableSlot (ShipChassisSlotType::Type slot) const
{
	if(getSlotTargetable(slot))
	{
		if(isSlotInstalled(slot))
		{
			if(!isComponentDisabled(slot))
			{
				return true;
			}
		}
	}
	return false;
}

//----------------------------------------------------------------------

ShipChassisSlotType::Type ShipObject::getNextValidTargetableSlot (ShipChassisSlotType::Type currentlyTargetedSlotType) const
{
	ShipChassisSlotType::Type slot = currentlyTargetedSlotType;
	
	//wrap from last slot to "none"
	if(slot == ShipChassisSlotType::SCST_weapon_last)
		return ShipChassisSlotType::SCST_num_types;

	//wrap from "none" to first slot
	if(slot == ShipChassisSlotType::SCST_num_types)
		slot = ShipChassisSlotType::SCST_first;
	else
		slot = static_cast<ShipChassisSlotType::Type>(slot + 1);

	for(int i = slot; i < ShipChassisSlotType::SCST_num_types; ++i)
	{
		slot = static_cast<ShipChassisSlotType::Type>(i);
		if(isValidTargetableSlot(slot))
			return slot;
	}

	return ShipChassisSlotType::SCST_num_types;
}

//----------------------------------------------------------------------

ShipChassisSlotType::Type ShipObject::getPreviousValidTargetableSlot (ShipChassisSlotType::Type currentlyTargetedSlotType) const
{
	ShipChassisSlotType::Type slot = currentlyTargetedSlotType;

	//wrap from first slot to "none"
	if(slot == ShipChassisSlotType::SCST_first)
		return ShipChassisSlotType::SCST_num_types;

	//wrap from "none" to last slot
	if(slot == ShipChassisSlotType::SCST_num_types)
		slot = static_cast<ShipChassisSlotType::Type>(ShipChassisSlotType::SCST_num_types - 1);
	else
		slot = static_cast<ShipChassisSlotType::Type>(slot - 1);
	for(int i = slot; i >= ShipChassisSlotType::SCST_first; --i)
	{
		slot = static_cast<ShipChassisSlotType::Type>(i);
		if(isValidTargetableSlot(slot))
			return slot;
	}

	return ShipChassisSlotType::SCST_num_types;
}

//----------------------------------------------------------------------

void ShipObject::setSlotTargetable(ShipChassisSlotType::Type slot)
{
	BitArray tmp(m_targetableSlotBitfield.get());
	tmp.setBit(slot);
	m_targetableSlotBitfield = tmp;
}

//----------------------------------------------------------------------

void ShipObject::clearSlotTargetable(ShipChassisSlotType::Type slot)
{
	BitArray tmp(m_targetableSlotBitfield.get());
	tmp.clearBit(slot);
	m_targetableSlotBitfield = tmp;
}

//----------------------------------------------------------------------

bool ShipObject::getSlotTargetable(ShipChassisSlotType::Type slot) const
{
	return m_targetableSlotBitfield.get().testBit(slot);
}


//----------------------------------------------------------------------

void ShipObject::Callbacks::LookAtTargetSlotChanged::modified (ShipObject &target, const int &old, const int &value, bool local) const
{
	// Player object may need a corresponding function

	UNREF(local);
	UNREF(old);
	int v = value;
	if (ms_lastLookAtTargetSlotRequest == v)
		Transceivers::lookAtTargetSlotChanged.emitMessage (v);
	else
	{
		target.setPilotLookAtTargetSlot (static_cast<ShipChassisSlotType::Type>(ms_lastLookAtTargetSlotRequest));
	}
	
	if (&target == Game::getPlayerPilotedShip())
	{
		enableTargetComponentAppearanceFromObjectId(target.getPilotLookAtTarget(), value);
	}
}

//----------------------------------------------------------------------

void ShipObject::Callbacks::TargetableSlotBitfieldChanged::modified (ShipObject &target, const BitArray &old, const BitArray &value, bool local) const
{
	UNREF(local);
	UNREF(old);
	UNREF(value);

	//if the player is targeting the ship whose targetable slots have changed, update the player's current targeted slot
	ShipObject * const playerShip = Game::getPlayerPilotedShip();
	if(playerShip)
	{
		Object const * const playerTargetedObject = playerShip->getPilotLookAtTarget().getObject();
		ClientObject const * const playerTargetedClientObject = playerTargetedObject ? playerTargetedObject->asClientObject() : NULL;
		ShipObject const * const playerTargetedShip = playerTargetedClientObject ? playerTargetedClientObject->asShipObject() : NULL;

		if(playerTargetedShip == &target)
			playerShip->updateTargetedSlot();
	}
} //lint !e1764 //not const ref

//----------------------------------------------------------------------

float ShipObject::getChassisComponentMassMaximum() const
{
	return m_chassisComponentMassMaximum.get();
}

//----------------------------------------------------------------------

float ShipObject::getChassisComponentMassCurrent() const
{
	return m_chassisComponentMassCurrent.get();
}

//----------------------------------------------------------------------

void ShipObject::clientSetChassisComponentMassMaximum(float massMaximum)
{
	m_chassisComponentMassMaximum.set(massMaximum);
}

//----------------------------------------------------------------------

void ShipObject::clientSetChassisComponentMassCurrent(float massCurrent)
{
	m_chassisComponentMassCurrent.set(massCurrent);
}

//----------------------------------------------------------------------

void ShipObject::clientSetCargoHoldContentsMaximum(int cargoHoldContentsMaximum)
{
	m_cargoHoldContentsMaximum = cargoHoldContentsMaximum;
}

//----------------------------------------------------------------------

void ShipObject::clientSetCargoHoldContentsCurrent(int cargoHoldContentsCurrent)
{
	m_cargoHoldContentsCurrent = cargoHoldContentsCurrent;
}

//----------------------------------------------------------------------

void ShipObject::clientPurgeComponent(int const chassisSlot)
{
	clientSetComponentCrc(chassisSlot, 0);
}

//----------------------------------------------------------------------

bool ShipObject::clientInstallComponentFromData(int const chassisSlot, ShipComponentData const & shipComponentData)
{
	ShipChassis const * const shipChassis = ShipChassis::findShipChassisByCrc (getChassisType ());
	if (shipChassis == NULL)
	{
		WARNING (true, ("Ship [%s] chassis [%d] is invalid for installing component [%s]",
			getNetworkId().getValueString().c_str(), 
			static_cast<int>(getChassisType ()),
			shipComponentData.getDescriptor().getName().getString()));
		return false;
	}
	
	if (isSlotInstalled (chassisSlot))
	{
		WARNING (true, ("Ship [%s] chassis [%s] slot [%s] is already filled, cannot install [%s]",
			getNetworkId().getValueString().c_str(), 
			shipChassis->getName().getString(),
			ShipChassisSlotType::getNameFromType(static_cast<ShipChassisSlotType::Type>(chassisSlot)).c_str(),
			shipComponentData.getDescriptor().getName().getString()));
		
		return false;
	}
	
	ShipChassisSlot const * const slot = shipChassis->getSlot (static_cast<ShipChassisSlotType::Type>(chassisSlot));
	if (slot == NULL)
	{
		WARNING (true, ("Ship [%s] chassis [%s] does not support slot [%s] for installing component [%s]",
			getNetworkId().getValueString().c_str(), 
			shipChassis->getName().getString(),
			ShipChassisSlotType::getNameFromType(static_cast<ShipChassisSlotType::Type>(chassisSlot)).c_str(),
			shipComponentData.getDescriptor().getName().getString()));
		return false;
	}
	
	if (!slot->canAcceptComponent(shipComponentData.getDescriptor()))
	{
		WARNING (true, ("Component [%s], compat [%s] cannot be installed in ship [%s] chassis [%s], slot [%s], compats [%s].", 
			shipComponentData.getDescriptor().getName().getString(), 
			shipComponentData.getDescriptor().getCompatibility().getString(),
			getNetworkId().getValueString().c_str(), 
			shipChassis->getName().getString(),
			ShipChassisSlotType::getNameFromType(static_cast<ShipChassisSlotType::Type>(chassisSlot)).c_str(),
			slot->getCompatibilityString().c_str()));
		return false;
	}
	
	clientSetComponentCrc(chassisSlot, shipComponentData.getDescriptor().getCrc());

	shipComponentData.writeDataToShip (chassisSlot, *this);

	return true;
}

//----------------------------------------------------------------------

bool ShipObject::clientPseudoInstallComponent(int const chassisSlot, uint32 const componentCrc)
{
	ShipComponentDescriptor const * const shipComponentDescriptor = ShipComponentDescriptor::findShipComponentDescriptorByCrc (componentCrc);
	if (shipComponentDescriptor == NULL)
	{
		WARNING (true, ("Invalid component name"));
		return false;
	}

	ShipComponentData * const shipComponentData = ShipComponentDataManager::create (*shipComponentDescriptor);

	if (shipComponentData == NULL)
	{
		WARNING(true, ("ShipObject::pseudoInstallComponent invalid descriptor"));
		return false;
	}

	bool const retval = clientInstallComponentFromData(chassisSlot, *shipComponentData);

	delete shipComponentData;

	return retval;
}

//----------------------------------------------------------------------

ShipComponentData * ShipObject::clientCreateShipComponentData (int chassisSlot) const
{
	ShipComponentDescriptor const * shipComponentDescriptor = NULL;

	uint32 const componentCrc = getComponentCrc (chassisSlot);
	if (componentCrc)
	{
		shipComponentDescriptor   = ShipComponentDescriptor::findShipComponentDescriptorByCrc (componentCrc);
		if (shipComponentDescriptor == NULL)
		{
			WARNING (true, ("ShipObject::createShipComponentData failed for slot [%d] because component crc [%d] could not map to a component descriptor", chassisSlot, componentCrc));
			return NULL;
		}
	}
	else
		return NULL;

	ShipComponentData * const shipComponentData = ShipComponentDataManager::create (*shipComponentDescriptor);
	if (shipComponentData == NULL)
	{
		WARNING (true, ("ShipObject::createShipComponentData failed for slot [%d] ship Component data could not be constructed", chassisSlot));
		return NULL;
	}

	if (!shipComponentData->readDataFromShip (chassisSlot, *this))
		WARNING (true, ("ShipObject::createShipComponentData failed for read data from the ship"));

	return shipComponentData;
}

//----------------------------------------------------------------------

float ShipObject::getShipTotalComponentEnergyRequirement() const
{
	float totalEnergyUsage = 0.0f;
	
	for(int i = ShipChassisSlotType::SCST_first; i != ShipChassisSlotType::SCST_num_types; ++i)
	{
		if(isSlotInstalled(i))
		{
			totalEnergyUsage += getComponentEnergyMaintenanceRequirement(i);
		}
	}

	return totalEnergyUsage;
}

//----------------------------------------------------------------------

int ShipObject::getCargoHoldContentsMaximum() const
{
	return m_cargoHoldContentsMaximum.get();
}

//----------------------------------------------------------------------

int ShipObject::getCargoHoldContentsCurrent() const
{
	return m_cargoHoldContentsCurrent.get();
}

//----------------------------------------------------------------------

ShipObject::NetworkIdIntMap const & ShipObject::getCargoHoldContents() const
{
	return m_cargoHoldContents.getMap();
}

//----------------------------------------------------------------------

void ShipObject::clientSetCargoHoldContents(NetworkIdIntMap const & contents)
{
	if (isSlotInstalled(static_cast<int>(ShipChassisSlotType::SCST_cargo_hold)))
	{
		m_cargoHoldContents.clear();

		NetworkIdIntMap::const_iterator const end = contents.end();
		for (NetworkIdIntMap::const_iterator it = contents.begin(); it != end; ++it)
		{
			m_cargoHoldContents.insert(it->first, it->second);
		}

		clientSetCargoHoldContentsCurrent(computeCargoHoldContentsCurrent());
	}
}

//----------------------------------------------------------------------

void ShipObject::clientSetCargoHoldContent(NetworkId const & resourceTypeId, int amount)
{
	if (isSlotInstalled(static_cast<int>(ShipChassisSlotType::SCST_cargo_hold)))
	{
		if (amount <= 0)
			m_cargoHoldContents.erase(resourceTypeId);
		else
		{
			if (!ResourceTypeManager::hasTypeInfo(resourceTypeId))
			{
				WARNING(true, ("ShipObject::clientSetCargoHoldContent invalid resource type [%s]", resourceTypeId.getValueString().c_str()));
				return;
			}

			m_cargoHoldContents.set(resourceTypeId, amount);
		}

		clientSetCargoHoldContentsCurrent(computeCargoHoldContentsCurrent());
	}
}

//----------------------------------------------------------------------

int ShipObject::getCargoHoldContent(NetworkId const & resourceTypeId) const
{
	if (isSlotInstalled(static_cast<int>(ShipChassisSlotType::SCST_cargo_hold)))
	{
		NetworkIdIntMap::const_iterator const it = m_cargoHoldContents.find(resourceTypeId);
		if (it != m_cargoHoldContents.end())
		{
			return (*it).second;
		}
	}

	return 0;
}

//----------------------------------------------------------------------

int ShipObject::computeCargoHoldContentsCurrent() const
{
	int total = 0;
	NetworkIdIntMap::const_iterator const end = m_cargoHoldContents.end();
	for (NetworkIdIntMap::const_iterator it = m_cargoHoldContents.begin(); it != end; ++it)
	{
		total += (*it).second;
	}

	return total;
}

//======================================================================
