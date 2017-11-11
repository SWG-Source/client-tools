// ======================================================================
//
// ShipObject.cpp
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ShipObject.h"
#include "clientGame/ShipObject_MissileUpdate.h"

#include "clientAudio/Audio.h"
#include "clientGame/AutoCommManager.h"
#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientDataFile.h"
#include "clientGame/ClientDataFile_InterpolatedSoundRuntime.h"
#include "clientGame/ClientDataFile_VehicleThrusterSoundData.h"
#include "clientGame/ClientDataFile_VehicleThrusterSoundRuntimeData.h"
#include "clientGame/ClientDataTemplateList.h"
#include "clientGame/ClientEffect.h"
#include "clientGame/ClientEffectManager.h"
#include "clientGame/ClientEffectTemplate.h"
#include "clientGame/ClientShipObjectTemplate.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/DynamicDebrisObject.h"
#include "clientGame/FlyByCamera.h"
#include "clientGame/Game.h"
#include "clientGame/GroundScene.h"
#include "clientGame/MiningAsteroidController.h"
#include "clientGame/MissileManager.h"
#include "clientGame/NebulaManagerClient.h"
#include "clientGame/PlayerShipController.h"
#include "clientGame/ProjectileManager.h"
#include "clientGame/RemoteShipController.h"
#include "clientGame/ResourceTypeManager.h"
#include "clientGame/ShipComponentPowerEffectsManagerClient.h"
#include "clientGame/ShipObjectAttachments.h"
#include "clientGame/ShipObjectDestruction.h"
#include "clientGame/ShipObjectEffects.h"
#include "clientGame/ShipStation.h"
#include "clientGame/ShipWeaponBarrelDynamics.h"
#include "clientGame/ShipWeaponGroupManager.h"
#include "clientGame/WingDynamics.h"
#include "clientGame/ZoneMapObject.h"
#include "clientGraphics/Camera.h"
#include "clientGraphics/DebugPrimitive.h"
#include "clientGraphics/RenderWorld.h"
#include "clientObject/DynamicMeshAppearance.h"
#include "clientObject/HardpointObject.h"
#include "clientParticle/ParticleEffectAppearance.h"
#include "clientParticle/SwooshAppearance.h"
#include "clientUserInterface/CuiIoWin.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiSoundManager.h"
#include "clientUserInterface/CuiSounds.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/CollisionWorld.h"
#include "sharedCollision/SpaceAvoidanceManager.h"
#include "sharedCollision/SpatialDatabase.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedDebug/PerformanceTimer.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/CrcString.h"
#include "sharedGame/Nebula.h"
#include "sharedGame/NebulaManager.h"
#include "sharedGame/SharedShipObjectTemplate.h"
#include "sharedGame/ShipChassis.h"
#include "sharedGame/ShipChassisSlot.h"
#include "sharedGame/ShipComponentAttachmentManager.h"
#include "sharedGame/ShipComponentDescriptor.h"
#include "sharedGame/ShipComponentFlags.h"
#include "sharedGame/ShipComponentWeaponManager.h"
#include "sharedGame/ShipSlotIdManager.h"
#include "sharedGame/ShipTurretManager.h"
#include "sharedGame/SpaceStringIds.h"
#include "sharedMath/Plane.h"
#include "sharedMathArchive/TransformArchive.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/Container.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/CustomizationDataProperty.h"
#include "sharedObject/Hardpoint.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/PortalProperty.h"
#include "sharedObject/RotationDynamics.h"
#include "sharedObject/SlottedContainer.h"

#include <vector>
#include <map>

// ======================================================================

namespace ShipObjectNamespace
{
	namespace Transceivers
	{
		MessageDispatch::Transceiver<ShipObject::Messages::ComponentsChanged::Payload &, ShipObject::Messages::ComponentsChanged> componentsChanged;
		MessageDispatch::Transceiver<ShipObject::Messages::TargetAcquisition::Payload &, ShipObject::Messages::TargetAcquisition> targetAcquisition;
		MessageDispatch::Transceiver<ShipObject::Messages::LookAtTargetChanged::Payload &, ShipObject::Messages::LookAtTargetChanged> lookAtTargetChanged;
		MessageDispatch::Transceiver<ShipObject::Messages::PilotMounted::Payload &, ShipObject::Messages::PilotMounted> pilotMounted;
		MessageDispatch::Transceiver<ShipObject::Messages::PilotDismounted::Payload &, ShipObject::Messages::PilotDismounted> pilotDismounted;
		MessageDispatch::Transceiver<ShipObject::Messages::GunnerMounted::Payload &, ShipObject::Messages::GunnerMounted> gunnerMounted;
		MessageDispatch::Transceiver<ShipObject::Messages::GunnerDismounted::Payload &, ShipObject::Messages::GunnerDismounted> gunnerDismounted;
		MessageDispatch::Transceiver<ShipObject::Messages::DroidPcdChanged::Payload &, ShipObject::Messages::DroidPcdChanged> droidPcdChanged;
		MessageDispatch::Transceiver<ShipObject::Messages::CargoChanged::Payload &, ShipObject::Messages::CargoChanged> cargoChanged;
	}

	//----------------------------------------------------------------------

	void pushCustomizationToChildrenHelper(TangibleObject & obj, CustomizationData & rootCustomizationData)
	{
		int const numberOfChildObjects = obj.getNumberOfChildObjects ();
		for (int i = 0; i < numberOfChildObjects; ++i)
		{
			TangibleObject * const child = dynamic_cast<TangibleObject *>(obj.getChildObject (i));
			if (child && !child->asCreatureObject())
			{
				CustomizationData * const childCustomizationData = child->fetchCustomizationData ();

				if (childCustomizationData)
				{
					//-- make a link from child's /shared_owner directory to owner's /shared_owner/ customization variable directory
					IGNORE_RETURN(childCustomizationData->mountRemoteCustomizationData(rootCustomizationData, "/shared_owner/", "/shared_owner"));
					childCustomizationData->release ();
				}

				pushCustomizationToChildrenHelper(*child, rootCustomizationData);
			}
		}
	}
	

	//----------------------------------------------------------------------

	void setObjectAndChildrenActive(Object & obj, bool active)
	{
		obj.setActive(active);
		int const numberOfChildObjects = obj.getNumberOfChildObjects ();
		for (int i = 0; i < numberOfChildObjects; ++i)
		{
			Object * const child = obj.getChildObject (i);
			if (NULL != child && child->isChildObject())
			{
				setObjectAndChildrenActive(*child, active);
			}
		}
	}

	//----------------------------------------------------------------------

	CachedNetworkId ms_lastLookAtTargetRequest;

	//----------------------------------------------------------------------

	bool s_installed = false;
	void remove();

	bool s_logDestructionTimes = false;
	ConstCharCrcString const cs_turretPitchHardpoint("turretpitch1");
	unsigned long s_lastOpenWingsTimeMs = 0;

	int const cs_numberOfZoneDisplayHardpoints = 2;
	char const * const cs_zoneDisplayHardpoints[cs_numberOfZoneDisplayHardpoints] =
							{
								"display1",
								"display2",
							};

	float const cms_exaggerateAvoidanceValue = 100.f;

	bool ms_showAvoidancePathOther = false;
	bool ms_showAvoidancePathSelf = false;
	bool ms_showOrientation = false;
	bool ms_exaggerateAvoidancePathOther = false;
	bool ms_exaggerateAvoidancePathSelf = true;

	static ShipObject::ShipVector ms_shipsTargetingPlayer;

	//----------------------------------------------------------------------

	void onShipWeaponBarrelFired(Object &object)
	{
		ShipWeaponBarrelDynamics * const dynamics = dynamic_cast<ShipWeaponBarrelDynamics *>(object.getDynamics());
		if (dynamics)
			dynamics->onFired();

		for (int i = 0; i < object.getNumberOfChildObjects(); ++i)
		{
			Object * const childObject = object.getChildObject(i);
			if (childObject)
				onShipWeaponBarrelFired(*childObject);
		}
	}

	//----------------------------------------------------------------------

	char const * const s_enemyMissileAcquired = "sound/ui_all_target_enemy.snd";
	char const * const s_enemyMissileIncoming = "sound/cbt_msl_alarm_incoming.snd";
	
	//----------------------------------------------------------------------

	float const cs_minimumTargetLeadProjectSpeed = 0.001f;
	float const cs_oneOverTurretInterpolateTimeSeconds = 1.f/2.f;

	//----------------------------------------------------------------------

	typedef std::map<uint16, ShipObject *> ShipIdMap;
	ShipIdMap s_shipsByShipId;

	//----------------------------------------------------------------------

	int ms_numberOfInstances;

	float s_hideTimePostDestruction = 20.0f; 

	//----------------------------------------------------------------------

	//-- these values are used to throttle the frequency of shield effects played on ships
	float const s_timeShieldHitClient = 1.5f;
	float const s_timeShieldHitServer = 0.5f;
	float const s_timeShieldEventServer = 10.0f;

	Watcher<Object> s_cockpitFrame;
}

using namespace ShipObjectNamespace;

// ======================================================================

void ShipObject::install()
{
	InstallTimer const installTimer("ShipObject::install");

	DEBUG_FATAL(s_installed, ("ShipObject::install: already installed"));
	s_installed = true;

	DebugFlags::registerFlag(s_logDestructionTimes, "ClientGame/ShipObject", "logDestructionTimes");
	DebugFlags::registerFlag(ms_showAvoidancePathOther, "ClientGame/ShipObject", "showAvoidancePathOther");
	DebugFlags::registerFlag(ms_showAvoidancePathSelf, "ClientGame/ShipObject", "showAvoidancePathSelf");
	DebugFlags::registerFlag(ms_exaggerateAvoidancePathSelf, "ClientGame/ShipObject", "exaggerateAvoidancePathSelf");
	DebugFlags::registerFlag(ms_exaggerateAvoidancePathOther, "ClientGame/ShipObject", "exaggerateAvoidancePathOther");
	DebugFlags::registerFlag(ms_showOrientation, "ClientGame/ShipObject", "showOrientation");

	ExitChain::add(ShipObjectNamespace::remove, "ShipObject::remove");
}

// ----------------------------------------------------------------------

void ShipObjectNamespace::remove()
{
	DEBUG_FATAL(!s_installed, ("ShipObject::install: not installed"));
	s_installed = false;

	DebugFlags::unregisterFlag(s_logDestructionTimes);
	DebugFlags::unregisterFlag(ms_showAvoidancePathOther);
	DebugFlags::unregisterFlag(ms_showAvoidancePathSelf);
	DebugFlags::unregisterFlag(ms_exaggerateAvoidancePathSelf);
	DebugFlags::unregisterFlag(ms_exaggerateAvoidancePathOther);
	DebugFlags::unregisterFlag(ms_showOrientation);
}

// ----------------------------------------------------------------------

int ShipObject::getNumberOfInstances()
{
	return ms_numberOfInstances;
}

// ======================================================================

ShipObject::ShipObject(SharedShipObjectTemplate const *newTemplate) :
	TangibleObject(newTemplate),
	m_pilot(NULL),
	m_shipId(0),
	m_slideDampener(0.f),
	m_currentChassisHitPoints(0.f),
	m_maximumChassisHitPoints(0.f),
	m_chassisType                            (0),
	m_chassisComponentMassMaximum(0.0f),
	m_chassisComponentMassCurrent(0.0f),
	m_chassisSpeedMaximumModifier(1.0f),
	m_shipActualAccelerationRate             (0.0f),
	m_shipActualDecelerationRate             (0.0f),
	m_shipActualPitchAccelerationRate        (0.0f),
	m_shipActualYawAccelerationRate          (0.0f),
	m_shipActualRollAccelerationRate         (0.0f),
	m_shipActualPitchRateMaximum             (0.0f),
	m_shipActualYawRateMaximum               (0.0f),
	m_shipActualRollRateMaximum              (0.0f),
	m_shipActualSpeedMaximum                 (0.0f),
	m_componentArmorHitpointsMaximum         (),
	m_componentArmorHitpointsCurrent         (),
	m_componentEfficiencyGeneral             (),
	m_componentEfficiencyEnergy              (),
	m_componentEnergyMaintenanceRequirement  (),
	m_componentMass                          (),
	m_componentCrc                           (),
	m_componentHitpointsCurrent              (),
	m_componentHitpointsMaximum              (),
	m_componentFlags                         (),
	m_oldComponentFlags(),
	m_componentNames(),
	m_componentCreators(),
	m_weaponDamageMaximum                    (),
	m_weaponDamageMinimum                    (),
	m_weaponEffectivenessShields             (),
	m_weaponEffectivenessArmor               (),
	m_weaponEnergyPerShot                    (),
	m_weaponRefireRate                       (),
	m_weaponEfficiencyRefireRate(),
	m_weaponAmmoCurrent                      (),
	m_weaponAmmoMaximum                      (),
	m_weaponAmmoType                         (),
	m_shieldHitpointsFrontCurrent            (0.0f),
	m_shieldHitpointsFrontMaximum            (0.0f),
	m_shieldHitpointsBackCurrent             (0.0f),
	m_shieldHitpointsBackMaximum             (0.0f),
	m_shieldRechargeRate                     (0.0f),
	m_capacitorEnergyCurrent                 (0.0f),
	m_capacitorEnergyMaximum                 (0.0f),
	m_capacitorEnergyRechargeRate            (0.0f),
	m_engineAccelerationRate                 (0.0f),
	m_engineDecelerationRate                 (0.0f),
	m_enginePitchAccelerationRate            (0.0f),
	m_engineYawAccelerationRate              (0.0f),
	m_engineRollAccelerationRate             (0.0f),
	m_enginePitchRateMaximum                 (0.0f),
	m_engineYawRateMaximum                   (0.0f),
	m_engineRollRateMaximum                  (0.0f),
	m_engineSpeedMaximum                     (0.0f),
	m_reactorEnergyGenerationRate            (0.0f),
	m_boosterEnergyCurrent                   (0.0f),
	m_boosterEnergyMaximum                   (0.0f),
	m_boosterEnergyRechargeRate              (0.0f),
	m_boosterEnergyConsumptionRate           (0.0f),
	m_boosterAcceleration                    (0.0f),
	m_boosterSpeedMaximum                    (0.0f),
	m_droidInterfaceCommandSpeed             (0.0f),
	m_installedDroidControlDevice(),
	m_cargoHoldContentsMaximum(0),
	m_cargoHoldContentsCurrent(0),
	m_cargoHoldContents(),
	m_cargoHoldContentsResourceTypeInfo(),
	m_pilotLookAtTarget(),
	m_pilotLookAtTargetSlot(),
	m_targetableSlotBitfield(),
	m_wingName(),
	m_typeName(),
	m_difficulty(),
	m_faction(),
	m_guildId(),
	m_nebulas(new IntVector),
	m_nebulaEffectReactor(0.0f),
	m_nebulaEffectEngine(0.0f),
	m_nebulaEffectShields(0.0f),
	m_nextMissileLauncher(0),
	m_shipObjectEffects(NULL),
	m_shipObjectAttachments(NULL),
	m_missleUpdateData(new MissileUpdate[ShipChassisSlotType::cms_numWeaponIndices]),
	m_hasMissiles(false),
	m_hasTargetAcquiring(false),
	m_hasTargetAcquired(false),
	m_missileGroup(-1),
	m_missileSlot(-1),
	m_shipWeaponStatusMap(),
	m_timerPostDestruction(-1.0f),
	m_shipKillableByDestructionSequence(false),
	m_lastPositionAtDestruction_w(),
	m_timerShieldHitClient(s_timeShieldHitClient),
	m_timerShieldHitServer(s_timeShieldHitServer),
	m_timerShieldEventServer(s_timeShieldEventServer)
{
	++ms_numberOfInstances;

	//-- pre-expire these timers
	m_timerShieldHitClient.setElapsedTime(s_timeShieldHitClient);
	m_timerShieldHitServer.setElapsedTime(s_timeShieldHitServer);
	m_timerShieldEventServer.setElapsedTime(s_timeShieldEventServer);

	m_shipObjectEffects = new ShipObjectEffects(*this);
	m_shipObjectAttachments = new ShipObjectAttachments(*this);

	m_shipId.setSourceObject (this);
	m_pilotLookAtTarget.setSourceObject (this);
	m_pilotLookAtTargetSlot.setSourceObject (this);
	m_targetableSlotBitfield.setSourceObject (this);
	m_installedDroidControlDevice.setSourceObject (this);
	m_cargoHoldContentsMaximum.setSourceObject (this);
	m_cargoHoldContentsCurrent.setSourceObject (this);

	addSharedVariable    (m_slideDampener);
	addSharedVariable    (m_currentChassisHitPoints);
	addSharedVariable    (m_maximumChassisHitPoints);
	addSharedVariable    (m_chassisType);
	addSharedVariable    (m_componentArmorHitpointsMaximum);
	addSharedVariable    (m_componentArmorHitpointsCurrent);
	addSharedVariable    (m_componentHitpointsCurrent);
	addSharedVariable    (m_componentHitpointsMaximum);
	addSharedVariable    (m_componentFlags);
	addSharedVariable    (m_shieldHitpointsFrontMaximum);
	addSharedVariable    (m_shieldHitpointsBackMaximum);

	addClientServerVariable    (m_componentEfficiencyGeneral);
	addClientServerVariable    (m_componentEfficiencyEnergy);
	addClientServerVariable    (m_componentEnergyMaintenanceRequirement);
	addClientServerVariable    (m_componentMass);
	addClientServerVariable    (m_componentNames);
	addClientServerVariable    (m_componentCreators);
	addClientServerVariable    (m_weaponDamageMaximum);
	addClientServerVariable    (m_weaponDamageMinimum);
	addClientServerVariable    (m_weaponEffectivenessShields);
	addClientServerVariable    (m_weaponEffectivenessArmor);
	addClientServerVariable    (m_weaponEnergyPerShot);
	addClientServerVariable    (m_weaponRefireRate);
	addClientServerVariable    (m_weaponAmmoCurrent);
	addClientServerVariable    (m_weaponAmmoMaximum);
	addClientServerVariable    (m_weaponAmmoType);
	addClientServerVariable    (m_chassisComponentMassMaximum);
	addClientServerVariable    (m_shieldRechargeRate);
	addClientServerVariable    (m_capacitorEnergyMaximum);
	addClientServerVariable    (m_capacitorEnergyRechargeRate);
	addClientServerVariable    (m_engineAccelerationRate);
	addClientServerVariable    (m_engineDecelerationRate);
	addClientServerVariable    (m_enginePitchAccelerationRate);
	addClientServerVariable    (m_engineYawAccelerationRate);
	addClientServerVariable    (m_engineRollAccelerationRate);
	addClientServerVariable    (m_enginePitchRateMaximum);
	addClientServerVariable    (m_engineYawRateMaximum);
	addClientServerVariable    (m_engineRollRateMaximum);
	addClientServerVariable    (m_engineSpeedMaximum);
	addClientServerVariable    (m_reactorEnergyGenerationRate);
	addClientServerVariable    (m_boosterEnergyMaximum);
	addClientServerVariable    (m_boosterEnergyRechargeRate);
	addClientServerVariable    (m_boosterEnergyConsumptionRate);
	addClientServerVariable    (m_boosterAcceleration);
	addClientServerVariable    (m_boosterSpeedMaximum);
	addClientServerVariable    (m_droidInterfaceCommandSpeed);
	addClientServerVariable    (m_installedDroidControlDevice);
	addClientServerVariable    (m_cargoHoldContentsMaximum);
	addClientServerVariable    (m_cargoHoldContentsCurrent);
	addClientServerVariable    (m_cargoHoldContents);

	addSharedVariable_np (m_shipId);
	addSharedVariable_np (m_shipActualAccelerationRate);
	addSharedVariable_np (m_shipActualDecelerationRate);
	addSharedVariable_np (m_shipActualPitchAccelerationRate);
	addSharedVariable_np (m_shipActualYawAccelerationRate);
	addSharedVariable_np (m_shipActualRollAccelerationRate);
	addSharedVariable_np (m_shipActualPitchRateMaximum);
	addSharedVariable_np (m_shipActualYawRateMaximum);
	addSharedVariable_np (m_shipActualRollRateMaximum);
	addSharedVariable_np (m_shipActualSpeedMaximum);
	addSharedVariable_np (m_pilotLookAtTarget);
	addSharedVariable_np (m_pilotLookAtTargetSlot);
	addSharedVariable_np (m_targetableSlotBitfield);
	addSharedVariable_np (m_componentCrc);
	addSharedVariable_np (m_wingName);
	addSharedVariable_np (m_typeName);
	addSharedVariable_np (m_difficulty);
	addSharedVariable_np (m_faction);
	addSharedVariable_np (m_shieldHitpointsFrontCurrent);
	addSharedVariable_np (m_shieldHitpointsBackCurrent);
	addSharedVariable_np (m_guildId);

	addClientServerVariable_np(m_chassisComponentMassCurrent);
	addClientServerVariable_np(m_chassisSpeedMaximumModifier);
	addClientServerVariable_np(m_capacitorEnergyCurrent);
	addClientServerVariable_np(m_boosterEnergyCurrent);
	addClientServerVariable_np(m_weaponEfficiencyRefireRate);
	addClientServerVariable_np(m_cargoHoldContentsResourceTypeInfo);

	m_cargoHoldContentsResourceTypeInfo.setOnInsert(this, &ShipObject::cargoHoldContentsResourceTypeInfoOnInsert);
	m_cargoHoldContentsResourceTypeInfo.setOnSet(this, &ShipObject::cargoHoldContentsResourceTypeInfoOnSet);

	m_componentCrc.setOnErase            (this, &ShipObject::componentCrcOnErase);
	m_componentCrc.setOnInsert           (this, &ShipObject::componentCrcOnInsert);
	m_componentCrc.setOnSet              (this, &ShipObject::componentCrcOnSet);

	//-- setup single player ship to make it flyable
	if (Game::getSinglePlayer())
	{
		m_slideDampener = 1.85f;

		m_chassisType = Crc::normalizeAndCalculate("player_xwing");

		static uint32 const componentCrcEngine = Crc::normalizeAndCalculate("eng_generic");
		static uint32 const componentCrcWeapons[] =
		{
			Crc::normalizeAndCalculate("wpn_tractor_beam_gun"),
			Crc::normalizeAndCalculate("wpn_laser_beam_gun")
		};
		static int const numWeapons = static_cast<int>(sizeof(componentCrcWeapons) / sizeof(componentCrcWeapons[0]));

		static uint32 const componentCrcBooster = Crc::normalizeAndCalculate("bst_xwing_booster_s01");
		static uint32 const componentCrcCapacitor = Crc::normalizeAndCalculate("cap_generic");

		m_componentCrc.set(static_cast<int>(ShipChassisSlotType::SCST_engine), componentCrcEngine);
		m_engineAccelerationRate = 24.0f;
		m_engineDecelerationRate = 18.0f;
		m_enginePitchAccelerationRate = convertDegreesToRadians(133.0f);
		m_engineYawAccelerationRate = convertDegreesToRadians(133.0f);
		m_engineRollAccelerationRate = convertDegreesToRadians(133.0f);
		m_enginePitchRateMaximum = convertDegreesToRadians(70.0f);
		m_engineYawRateMaximum = convertDegreesToRadians(70.0f);
		m_engineRollRateMaximum = convertDegreesToRadians(192.0f);
		m_engineSpeedMaximum = 50.0f;

		{
			for (int i = 0; i < numWeapons; ++i)
			{
				int const chassisSlotType = static_cast<int>(ShipChassisSlotType::SCST_weapon_first) + i;
				m_componentCrc.set(chassisSlotType, componentCrcWeapons[i]);
				m_weaponDamageMaximum.set(chassisSlotType, 100.0f);
				m_weaponDamageMinimum.set(chassisSlotType, 20.0f);
				m_weaponEffectivenessShields.set(chassisSlotType, 0.5f);
				m_weaponEffectivenessArmor.set(chassisSlotType, 0.5f);
				m_weaponEnergyPerShot.set(chassisSlotType, 0.0f);
				m_weaponRefireRate.set(chassisSlotType, 0.5f);
			}
		}

		m_componentCrc.set(static_cast<int>(ShipChassisSlotType::SCST_booster), componentCrcBooster);
		m_boosterEnergyCurrent = 100.0f;
		m_boosterEnergyMaximum = 100.0f;
		m_boosterEnergyRechargeRate = 100.0f;
		m_boosterEnergyConsumptionRate = 10.0f;
		m_boosterAcceleration = 20.0f;
		m_boosterSpeedMaximum = 30.0f;

		m_componentCrc.set(static_cast<int>(ShipChassisSlotType::SCST_capacitor), componentCrcCapacitor);
		m_capacitorEnergyCurrent = 100.0f;
		m_capacitorEnergyMaximum = 100.0f;

		{
			for (int i = 0; i < static_cast<int>(ShipChassisSlotType::SCST_num_types); ++i)
				setSlotTargetable(static_cast<ShipChassisSlotType::Type>(i));
		}

		static uint32 componentCrcShield = Crc::normalizeAndCalculate("shd_generic");
		m_componentCrc.set(ShipChassisSlotType::SCST_shield_0, componentCrcShield);
		IGNORE_RETURN(clientSetComponentFlags(ShipChassisSlotType::SCST_shield_0, getComponentFlags(ShipChassisSlotType::SCST_shield_0) | ShipComponentFlags::F_shieldsFront));

		static uint32 componentCrcReactor = Crc::normalizeAndCalculate("rct_generic");
		m_componentCrc.set(ShipChassisSlotType::SCST_reactor, componentCrcReactor);

		static uint32 componentCrcCargoHold = Crc::normalizeAndCalculate("crg_starfighter_small");
		m_componentCrc.set(ShipChassisSlotType::SCST_cargo_hold, componentCrcCargoHold);

		
	}
}

// ----------------------------------------------------------------------

ShipObject::~ShipObject()
{
	//-- This must be the first line in the destructor to invalidate any watchers watching this object
	nullWatchers();

	if (isInWorld ())
		removeFromWorld ();

	stopFiringAllWeapons();

	--ms_numberOfInstances;

	AutoCommManager::removeShip(*this);

	if (m_shipId.get() != 0)
	{
		ShipIdMap::iterator i = s_shipsByShipId.find(m_shipId.get());
		if (i != s_shipsByShipId.end() && (*i).second == this)
			s_shipsByShipId.erase(i);
	}
		
	for (int i = 0; i < static_cast<int>(ShipChassisSlotType::SCST_num_types); ++i)
		m_shipObjectAttachments->removeComponentAttachments (i);

	m_pilot = NULL;

	IGNORE_RETURN(ms_shipsTargetingPlayer.erase(std::remove(ms_shipsTargetingPlayer.begin(), ms_shipsTargetingPlayer.end(), this), ms_shipsTargetingPlayer.end()));

	delete m_nebulas;
	m_nebulas = NULL;

	delete m_shipObjectEffects;
	m_shipObjectEffects = NULL;

	delete m_shipObjectAttachments;
	m_shipObjectAttachments = NULL;

	delete [] m_missleUpdateData;
	m_missleUpdateData = NULL;
}

// ----------------------------------------------------------------------

ShipObject *ShipObject::findShipByShipId(uint16 shipId) // static
{
	ShipIdMap::const_iterator i = s_shipsByShipId.find(shipId);
	if (i != s_shipsByShipId.end())
	{
		ShipObject * const ship = (*i).second;
		if (ship && ship->m_shipId.get() == shipId)
			return ship;
	}
	return 0;
}

// ----------------------------------------------------------------------

ShipObject *ShipObject::getContainingShip(ClientObject &obj) // static
{
	for (ClientObject *o = &obj; o; o = safe_cast<ClientObject *>(ContainerInterface::getContainedByObject(*o)))
	{
		ShipObject * const so = o->asShipObject();
		if (so)
			return so;
	}
	return 0;
}

// ----------------------------------------------------------------------

ShipObject const *ShipObject::getContainingShip(ClientObject const &obj) // static
{
	for (ClientObject const *o = &obj; o; o = safe_cast<ClientObject const *>(ContainerInterface::getContainedByObject(*o)))
	{
		ShipObject const * const so = o->asShipObject();
		if (so)
			return so;
	}
	return 0;
}

//----------------------------------------------------------------------

Object &ShipObject::getShipTurretMuzzle(Object &turret) // static
{
	Appearance const * const appearance = turret.getAppearance();
	Transform t;
	if (appearance && appearance->findHardpoint(cs_turretPitchHardpoint, t))
	{
		int const childObjectCount = turret.getNumberOfChildObjects();
		if (childObjectCount > 0)
		{
			Object * const childObject = turret.getChildObject(0);
			if (childObject)
				return *childObject;
		}
	}
	return turret;
}

// ----------------------------------------------------------------------

Object const &ShipObject::getShipTurretMuzzle(Object const &turret) // static
{
	return getShipTurretMuzzle(const_cast<Object &>(turret));
}

// ----------------------------------------------------------------------

void ShipObject::orientShipTurret(int weaponIndex, float &yaw, float &pitch, bool inversePitch)
{
	Transform hardpointTransform_o2p;
	Object * const turret = getShipObjectAttachments().getFirstAttachedObjectForWeapon(weaponIndex, hardpointTransform_o2p);
	if (turret)
	{
		ShipWeaponStatus &weaponStatus = m_shipWeaponStatusMap[weaponIndex];
		Transform turretTransform(hardpointTransform_o2p);
		
		if(weaponStatus.m_playerControlled)
		{
			Object &muzzle = getShipTurretMuzzle(*turret);

			Vector turretStart;
			Vector turretEnd;

			if(turret == &muzzle)
			{
				turretStart = turret->getPosition_w();
				turretEnd = turretStart + (turret->getTransform_o2w().getLocalFrameK_p() * 8000.0f);
			}
			else
			{
				turretStart = turret->getPosition_w();
				turretEnd = turretStart + (muzzle.getTransform_o2w().getLocalFrameK_p() * 8000.0f);
			}

			if(weaponStatus.m_lookAtTarget == Vector::unitZ)
			{
				weaponStatus.m_lookAtTarget = turretEnd;
			}
			if(CuiPreferences::getEnableGimbal())
	        {

				float deltaYaw = weaponStatus.m_previousYaw - yaw;
				float deltaPitch = weaponStatus.m_previousPitch - pitch;
				
				getTurretYawAndPitchTo(weaponIndex, weaponStatus.m_lookAtTarget, yaw, pitch);
				
				yaw -= deltaYaw;

				if(inversePitch)
				{
					pitch += deltaPitch;
				}
				else
				{
					pitch -= deltaPitch;
				}					

			}

		}

		uint32 const chassisCrc = getChassisType();
		float const minYaw = ShipTurretManager::getTurretMinYaw(chassisCrc, weaponIndex);
		float const maxYaw = ShipTurretManager::getTurretMaxYaw(chassisCrc, weaponIndex);
		float const minPitch = ShipTurretManager::getTurretMinPitch(chassisCrc, weaponIndex);
		float const maxPitch = ShipTurretManager::getTurretMaxPitch(chassisCrc, weaponIndex);

		DEBUG_FATAL(minYaw > maxYaw, ("ShipObject::orientShipTurret [%s] [%d] has invalid min/max yaw [%f/%f]", getDebugInformation().c_str(), weaponIndex, minYaw, maxYaw));
		DEBUG_FATAL(minPitch > maxPitch, ("ShipObject::orientShipTurret [%s] [%d] has invalid min/max pitch [%f/%f]", getDebugInformation().c_str(), weaponIndex, minPitch, maxPitch));

		// clamp yaw and/or pitch if they are not allowed complete rotation
		if (((maxYaw-minYaw)+0.001) < PI_TIMES_2)
		{
			yaw = clamp(minYaw, yaw, maxYaw);
		}
		if (((maxPitch-minPitch)+0.001) < PI_TIMES_2)
		{
			pitch = clamp(minPitch, pitch, maxPitch);
		}

		// update the stored orientation for the turret, for use in interpolation
		weaponStatus.m_isTurretOrientationSet = true;
		weaponStatus.m_turretYaw = yaw;
		weaponStatus.m_turretPitch = pitch;

		Object &muzzle = getShipTurretMuzzle(*turret);
		turretTransform.yaw_l(yaw);
		if (turret == &muzzle)
		{
			turretTransform.pitch_l(pitch);
			turret->setTransform_o2p(turretTransform);
		}
		else
		{
			turret->setTransform_o2p(turretTransform);
			Transform muzzleTransform;
			IGNORE_RETURN(NON_NULL(turret->getAppearance())->findHardpoint(cs_turretPitchHardpoint, muzzleTransform));
			muzzleTransform.pitch_l(pitch);
			muzzle.setTransform_o2p(muzzleTransform);
		}

		if(weaponStatus.m_playerControlled)
		{		
			Vector turretStart;
			Vector turretEnd;

			if(turret == &muzzle)
			{
				turretStart = turret->getPosition_w();
				turretEnd = turretStart + (turret->getTransform_o2w().getLocalFrameK_p() * 8000.0f);
			}
			else
			{
				turretStart = turret->getPosition_w();
				turretEnd = turretStart + (muzzle.getTransform_o2w().getLocalFrameK_p() * 8000.0f);
			}
			
			weaponStatus.m_lookAtTarget = turretEnd;
			weaponStatus.m_previousYaw = yaw;
			weaponStatus.m_previousPitch = pitch;
		}
	}
}

//----------------------------------------------------------------------

bool ShipObject::getTurretYawAndPitchTo(int weaponIndex, Vector const &targetPosition_w, float &yaw, float &pitch) const
{
	if (isTurret(weaponIndex))
	{
		Transform hardpointTransform_o2p;
		Object const * const weaponObject = getShipObjectAttachments().getFirstAttachedObjectForWeapon(weaponIndex, hardpointTransform_o2p);

		if (weaponObject)
		{
			Object const * const attachedTo = NON_NULL(weaponObject->getAttachedTo());
			Object const &muzzle = getShipTurretMuzzle(*weaponObject);
			Transform turretDefaultTransform_o2w(Transform::IF_none);
			turretDefaultTransform_o2w.multiply(attachedTo->getTransform_o2w(), hardpointTransform_o2p);
			if(weaponObject != &muzzle)
			{
				Transform muzzleTransform;
				IGNORE_RETURN(NON_NULL(weaponObject->getAppearance())->findHardpoint(cs_turretPitchHardpoint, muzzleTransform));
				turretDefaultTransform_o2w.multiply(turretDefaultTransform_o2w, muzzleTransform);
			}
			Vector const targetPosition_o = turretDefaultTransform_o2w.rotateTranslate_p2l(targetPosition_w);
			yaw = targetPosition_o.theta();
			pitch = targetPosition_o.phi();
			return true;
		}
	}
	return false;
}

//----------------------------------------------------------------------

void ShipObject::orientShipTurret(int weaponIndex, Vector const &targetPosition_w)
{
	float yaw, pitch;
	if (getTurretYawAndPitchTo(weaponIndex, targetPosition_w, yaw, pitch))
		orientShipTurret(weaponIndex, yaw, pitch);
}

// ----------------------------------------------------------------------

void ShipObject::endBaselines()
{
	TangibleObject::endBaselines ();

	CustomizationData * const data = fetchCustomizationData();
	if(data)
	{
		Appearance * const app = getAppearance();
		if(app)
		{
			app->setCustomizationData(data);
		}
		data->release();
	}

	pushCustomizationToChildren();

	updateAllComponentAttachments();

	getShipObjectEffects().initialize();

	AutoCommManager::registerShip(*this);	
}

// ----------------------------------------------------------------------

void ShipObject::addToWorld()
{
	TangibleObject::addToWorld();

	//-- Create our interior data
#ifdef _DEBUG
	if (ConfigClientGame::getUseInteriorLayoutFiles())
#endif
	{
		safe_cast<ClientShipObjectTemplate const *>(getObjectTemplate())->applyInteriorLayout(this);
	}
}

//----------------------------------------------------------------------

void ShipObject::pushCustomizationToChildren()
{
	CustomizationData * const cd = fetchCustomizationData ();
	if (cd)
	{
		//-- force push customization to child objects (wings for instance)
		pushCustomizationToChildrenHelper(*this, *cd);
		cd->release();
	}
}

//----------------------------------------------------------------------

void ShipObject::resetAfterDestruction()
{
	//-- first remove all child objects so the CDF and other effects don't stack
	{
		for (int i = getNumberOfChildObjects() - 1; i >= 0; --i)
		{
			Object * const child = getChildObject(i);
			if (NULL != child)
			{
				if ((child != Game::getPlayer()) && (child != s_cockpitFrame.getPointer()))
				{
					child->kill();
				}
			}
		}
	}
	
	ClientDataFile const * const clientDataFile = getClientData();
	if (NULL != clientDataFile)
	{
		clientDataFile->apply (this);
		if (0 != getDamageLevel())
			clientDataFile->applyDamage (this, hasCondition(C_onOff), 0.0f, getDamageLevel());
		if (hasCondition(C_onOff))
			clientDataFile->applyOnOff(this, true);
	}

	pushCustomizationToChildren();

	updateAllComponentAttachments();

	ShipController * const shipController = (NULL != getController()) ? getController()->asShipController() : NULL;
	RemoteShipController * const remoteShipController = dynamic_cast<RemoteShipController *>(shipController);

	if (NULL != remoteShipController && NULL != shipController)
	{
		setTransform_o2w(remoteShipController->getServerTransform_p());
		shipController->setShipDynamicsModelTransform(remoteShipController->getServerTransform_p());
	}

}

// ----------------------------------------------------------------------

void ShipObject::attachDisplayObjects()
{
	PortalProperty * const portalProperty = getPortalProperty();
	if (portalProperty != 0)
	{
		Appearance const * const appearance = getAppearance();
		if (appearance != 0)
		{
			float dynamicDirection = 1.0f;
			for (int i = 0; i < cs_numberOfZoneDisplayHardpoints; ++i)
			{
				Transform transform_o;
				ConstCharCrcString const hardpointToFind(cs_zoneDisplayHardpoints[i]);
				if (appearance->findHardpoint(hardpointToFind, transform_o))
				{
					PortalProperty::CellNameList const & cellNameList = portalProperty->getCellNames();
					{
						PortalProperty::CellNameList::const_iterator ii = cellNameList.begin();
						PortalProperty::CellNameList::const_iterator iiEnd = cellNameList.end();
						for (; ii != iiEnd; ++ii)
						{
							char const * const cellName = *ii;
							if (strcmp(cellName, "cockpit") == 0)
							{
								CellProperty * const cellProperty = portalProperty->getCell(cellName);

								if (cellProperty != 0)
								{
									ZoneMapObject * const zoneMapObject = new ZoneMapObject(Game::getSceneId(), 0);
									zoneMapObject->scheduleForAlter();

									ZoneMapObject::createAsteroids(zoneMapObject);
									ZoneMapObject::createNebulas(zoneMapObject);
									ZoneMapObject::createHyperspacePoints(zoneMapObject);

									zoneMapObject->showAsteroid(true);
									zoneMapObject->showNebula(true);
									zoneMapObject->showHyperspacePoint(true);

									zoneMapObject->scaleContents(Vector(0.03f, 0.03f, 0.03f));

									addClientOnlyInteriorLayoutObject(zoneMapObject);

									zoneMapObject->setParentCell(cellProperty);
									CellProperty::setPortalTransitionsEnabled(false);

									// this move is temporary for now...
									// the art really needs the point moved
									transform_o.move_l(Vector::unitY);
									zoneMapObject->setTransform_o2p(transform_o);
									CellProperty::setPortalTransitionsEnabled(true);
						
									RenderWorld::addObjectNotifications(*zoneMapObject);
									zoneMapObject->addToWorld();

									// hard coded here in for now needs to be configurable
									Vector const rotations(0.2f, 0.0f, 0.0f);

									RotationDynamics * const dynamics = new RotationDynamics(zoneMapObject, dynamicDirection * rotations);
									dynamics->setState(true);
									zoneMapObject->setDynamics(dynamics);
									dynamicDirection *= -1.0f;
								} //lint !e429 //custodial pointer zoneMapObject dynamics
							}
						}
					}
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

float ShipObject::alter(float const elapsedTime)
{
	if (!isInWorld())
		return TangibleObject::alter(elapsedTime);

	if (!m_timerShieldHitClient.isExpired())
		IGNORE_RETURN(m_timerShieldHitClient.updateNoReset(elapsedTime));

	if (!m_timerShieldHitServer.isExpired())
		IGNORE_RETURN(m_timerShieldHitServer.updateNoReset(elapsedTime));

	if (!m_timerShieldEventServer.isExpired())
		IGNORE_RETURN(m_timerShieldEventServer.updateNoReset(elapsedTime));

	if (hasWings() && !hasCondition(TangibleObject::C_wingsOpened))
		stopFiringAllWeapons();


	float const energyAvailable = getCapacitorEnergyCurrent();

	if (energyAvailable <= 0.0f)
	{
		stopFiringAllWeapons();
	}

#ifdef _DEBUG
	alterDebug(elapsedTime);
#endif

	if (Game::getSinglePlayer())
	{
		m_shipActualAccelerationRate = m_engineAccelerationRate;
		m_shipActualDecelerationRate = m_engineDecelerationRate;
		m_shipActualPitchAccelerationRate = m_enginePitchAccelerationRate;
		m_shipActualYawAccelerationRate = m_engineYawAccelerationRate;
		m_shipActualRollAccelerationRate = m_engineRollAccelerationRate;
		m_shipActualPitchRateMaximum = m_enginePitchRateMaximum;
		m_shipActualYawRateMaximum = m_engineYawRateMaximum;
		m_shipActualRollRateMaximum = m_engineRollRateMaximum;
		m_shipActualSpeedMaximum = m_engineSpeedMaximum;

		if (isBoosterActive())
		{
			m_shipActualAccelerationRate.set(m_shipActualAccelerationRate.get() + getBoosterAcceleration());
			m_shipActualSpeedMaximum.set(m_shipActualSpeedMaximum.get() + getBoosterSpeedMaximum());
		}
	
		if (hasWings() && hasCondition(TangibleObject::C_wingsOpened))
		{
			ShipChassis const * const shipChassis = ShipChassis::findShipChassisByCrc (getChassisType ());
			if (NULL != shipChassis)			
			{
				float const wingOpenSpeedFactor = shipChassis->getWingOpenSpeedFactor();
				m_shipActualSpeedMaximum.set(m_shipActualSpeedMaximum.get() * wingOpenSpeedFactor);
			}
		}
	}

	updateNebulas(elapsedTime);

	updateMissiles(elapsedTime);

	updateComponentStates(elapsedTime);

	getShipObjectEffects().updateEngineEffects(elapsedTime);
	getShipObjectEffects().updateTargetAppearance(elapsedTime);

	ShipObject const * const playerContainingShip = Game::getPlayerContainingShip();
	if (this == playerContainingShip)
	{
		getShipObjectEffects().updateFlybySounds(elapsedTime);
		//-- force a recalculation for player ships with wings
		if (hasWings())
			getShipObjectAttachments().recalculateAverageProjectileData();
	}

	{
		// Rotate all the turrets to their targets

		for (std::map<int, ShipWeaponStatus>::const_iterator i = m_shipWeaponStatusMap.begin(); i != m_shipWeaponStatusMap.end(); ++i)
		{
			int const weaponIndex = (*i).first;
			ShipWeaponStatus const &weaponStatus = (*i).second;
			if (isTurret(weaponIndex) && !weaponStatus.m_playerControlled)
			{
				//-- don't re-orient beam weapons that are currently firing
				if (!isBeamWeapon(weaponIndex) || !isWeaponFiring(weaponIndex))
				{
					float desiredYaw = 0.f;
					float desiredPitch = 0.f;

					Object const * const object = weaponStatus.m_turretTarget.getObject();
					if (object)
						IGNORE_RETURN(getTurretYawAndPitchTo(weaponIndex, object->getPosition_w(), desiredYaw, desiredPitch));

					if (weaponStatus.m_isTurretOrientationSet)
					{
						desiredYaw = linearInterpolate(weaponStatus.m_turretYaw, desiredYaw, clamp(0.f, elapsedTime*cs_oneOverTurretInterpolateTimeSeconds, 1.f));
						desiredPitch = linearInterpolate(weaponStatus.m_turretPitch, desiredPitch, clamp(0.f, elapsedTime*cs_oneOverTurretInterpolateTimeSeconds, 1.f));
					}
					orientShipTurret(weaponIndex, desiredYaw, desiredPitch);
				}
			}
		}
	}


	//-- we are timing post destruction
	//-- when the timer expires, reactivate the ship
	if (m_timerPostDestruction.getExpireTime() > 0.0f)
	{
		ShipController * const shipController = (NULL != getController()) ? getController()->asShipController() : NULL;
		RemoteShipController * const remoteShipController = dynamic_cast<RemoteShipController *>(shipController);

		Vector serverPos_w;

		if (NULL != remoteShipController)
			serverPos_w = remoteShipController->getServerTransform_p().getPosition_p();
		else
			serverPos_w = getPosition_w();
		
		//-- if the ship has moved or the timer has expired, recreate it.
		if ((serverPos_w.magnitudeBetweenSquared(m_lastPositionAtDestruction_w) > 0.01f) ||
			m_timerPostDestruction.updateNoReset(elapsedTime))
		{
			//-- stop running the post destruction timer
			m_timerPostDestruction.setExpireTime(-1.0f);
			setObjectAndChildrenActive(*this, true);
			resetAfterDestruction();
		}
	}

	float alterResult = AlterResult::cms_alterNextFrame;

	float const tangibleAlterResult = TangibleObject::alter(elapsedTime);

	AlterResult::incorporateAlterResult(alterResult, tangibleAlterResult);

	return alterResult;
}

//----------------------------------------------------------------------

void ShipObject::updateAllComponentAttachments()
{
	//-- force an update of the component attachments
	for (std::map<int, uint32>::const_iterator it = m_componentCrc.begin (); it != m_componentCrc.end (); ++it)
		updateComponentAttachment ((*it).first);
}

//----------------------------------------------------------------------

void ShipObject::updateComponentAttachment (int chassisSlot)
{
	if (!isInitialized())
		return;

	DEBUG_FATAL((chassisSlot < 0 || chassisSlot >= static_cast<int>(ShipChassisSlotType::SCST_num_types)), ("ShipObject::updateComponentAttachment chassisSlot [%d] out of range.", chassisSlot));
	getShipObjectAttachments().removeComponentAttachments (chassisSlot);

	if (ShipChassisSlotType::isWeaponChassisSlot(static_cast<ShipChassisSlotType::Type>(chassisSlot)) && 
		Game::getPlayerPilotedShip() == this)
		ShipWeaponGroupManager::resetDefaultsForShip(getNetworkId(), false);

	uint32 const componentCrc = getComponentCrc (chassisSlot);

	ShipChassisSlotType::Type const chassisSlotType = static_cast<ShipChassisSlotType::Type>(chassisSlot);

	if (componentCrc == 0)
	{
		getShipObjectEffects().resetContrails();
		IGNORE_RETURN(m_oldComponentFlags.erase(chassisSlot));

		if (ShipChassisSlotType::isWeaponChassisSlot(chassisSlotType))
		{
			int const weaponIndex = ShipChassisSlotType::getWeaponIndex(chassisSlotType);
			stopFiringWeapon(weaponIndex, true);
			m_shipWeaponStatusMap.erase(weaponIndex);
		}

		return;
	}

	m_oldComponentFlags[chassisSlot] = getComponentFlags(chassisSlot);

	ShipChassis const * const shipChassis = ShipChassis::findShipChassisByCrc (getChassisType ());
	if (shipChassis == NULL )
	{
		WARNING (true, ("ShipObject component attached to ship with null chassis"));
		return;
	}

	ShipComponentDescriptor const * const shipComponentDescriptor = ShipComponentDescriptor::findShipComponentDescriptorByCrc (componentCrc);

	if (shipComponentDescriptor == NULL)
	{
		WARNING (true, ("ShipObject component attachment crc [%u] not valid", componentCrc));
		return;
	}

	if (isComponentDemolished(chassisSlot))
	{
		//-- demolished components still might need contrail information on their parents
		getShipObjectEffects().resetContrails();
		if (ShipChassisSlotType::isWeaponChassisSlot(chassisSlotType))
			stopFiringWeapon(ShipChassisSlotType::getWeaponIndex(chassisSlotType), true);
		return;
	}
	
	m_shipObjectAttachments->updateComponentAttachment(chassisSlot);
}

//----------------------------------------------------------------------

void ShipObject::updateComponentStatesForSlot(int const chassisSlot, float const elapsedTime)
{
	UNREF(elapsedTime);

	int const componentFlags = getComponentFlags(chassisSlot);
	int const oldComponentFlags = m_oldComponentFlags[chassisSlot];

	bool const wasDemolished = (oldComponentFlags & ShipComponentFlags::F_demolished) != 0;
	bool const isDemolished = (componentFlags & ShipComponentFlags::F_demolished) != 0;

	bool const wasDisabled = (oldComponentFlags & ShipComponentFlags::F_disabled) != 0;
	bool const isDisabled = (componentFlags & ShipComponentFlags::F_disabled) != 0;

	ShipChassisSlotType::Type const chassisSlotType = static_cast<ShipChassisSlotType::Type>(chassisSlot);

	//-- component is no longer disabled
	if (wasDisabled && !isDisabled)
	{
		ShipComponentPowerEffectsManagerClient::playEnableEffect(*this, chassisSlot);
	}

	//-- component is newly disabled
	else if (isDisabled && !wasDisabled)
	{
		ShipComponentPowerEffectsManagerClient::playDisableEffect(*this, chassisSlot);

		if (ShipChassisSlotType::isWeaponChassisSlot(chassisSlotType))
			stopFiringWeapon(ShipChassisSlotType::getWeaponIndex(chassisSlotType), true);
	}

	//-- component is no longer demolished
	if (wasDemolished && !isDemolished)
	{
		updateComponentAttachment(chassisSlot);
	}

	//-- component is newly demolished
	else if (isDemolished && !wasDemolished)
	{
		handleComponentDestruction(chassisSlot, 0.0f, false);
		//--
		updateComponentAttachment(chassisSlot);
	}

	m_oldComponentFlags[chassisSlot] = componentFlags;

	m_shipObjectAttachments->updateComponentStatesForSlot(chassisSlot);
}

//----------------------------------------------------------------------

void ShipObject::updateComponentStates(float const elapsedTime)
{
	for (std::map<int, uint32>::const_iterator it = m_componentCrc.begin (); it != m_componentCrc.end (); ++it)
	{
		int const chassisSlot = (*it).first;
		updateComponentStatesForSlot(chassisSlot, elapsedTime);
	}

	//-- mining asteroids don't modify hitpoints
	int const got = getGameObjectType();
	if (got == SharedObjectTemplate::GOT_ship_mining_asteroid_dynamic ||
		got == SharedObjectTemplate::GOT_ship_mining_asteroid_static)
	{
		return;
	}

	//-- update for chassis & wings

	{
		float const hpMax = getMaximumChassisHitPoints();
		float const hpCur = getCurrentChassisHitPoints();

		int const armorChassisSlot_0 = static_cast<int>(ShipChassisSlotType::SCST_armor_0);
		int const armorChassisSlot_1 = static_cast<int>(ShipChassisSlotType::SCST_armor_1);

		float const armorMax = getComponentArmorHitpointsMaximum(armorChassisSlot_0) + getComponentArmorHitpointsMaximum(armorChassisSlot_1);
		float const armorCur = getComponentArmorHitpointsCurrent(armorChassisSlot_0) + getComponentArmorHitpointsCurrent(armorChassisSlot_1);

		int const damageMax = static_cast<int>(hpMax + armorMax);
		int const damageCur = static_cast<int>(hpCur + armorCur);

		clientSetMaxHitPoints(damageMax);
		clientSetDamageTaken(damageMax - damageCur);

		{
			int const numberOfChildObjects = getNumberOfChildObjects();

			for (int i = 0; i < numberOfChildObjects; ++i)
			{
				TangibleObject * const wing = dynamic_cast<TangibleObject *>(getChildObject(i));
				if (wing != NULL)
				{
					if (dynamic_cast<WingDynamics *>(wing->getDynamics()) != NULL)
					{
						wing->clientSetMaxHitPoints(damageMax);
						wing->clientSetDamageTaken(damageMax - damageCur);
					}
				}
			}
		}
	}
}

//----------------------------------------------------------------------

void ShipObject::updateNebulas(float /*elapsedTime*/)
{
	//-- update the list of nebulas containing the ship

	bool const isPilotPlayer = Game::getPlayer() == getPilot();

	{
		m_nebulaEffectReactor = 0.0f;
		m_nebulaEffectEngine = 0.0f;
		m_nebulaEffectShields = 0.0f;

		static IntVector oldNebulas;
		oldNebulas = *m_nebulas;

		m_nebulas->clear();

		static NebulaManager::NebulaVector nebulaVector;
		nebulaVector.clear();
		NebulaManager::getNebulasAtPosition(getPosition_w(), nebulaVector);
		if (!nebulaVector.empty())
		{
			m_nebulas->reserve(nebulaVector.size());
			for (NebulaManager::NebulaVector::const_iterator it = nebulaVector.begin(); it != nebulaVector.end(); ++it)
			{
				Nebula const * const nebula = NON_NULL(*it);
				int const id = nebula->getId();
				m_nebulas->push_back(id);

				if (!std::binary_search(oldNebulas.begin(), oldNebulas.end(), id))
				{
					if (isPilotPlayer)
					{
						NebulaManagerClient::handlePlayerEnter(id);
					}
				}

				m_nebulaEffectReactor += nebula->getEffectReactor();
				m_nebulaEffectEngine += nebula->getEffectEngine();
				m_nebulaEffectShields += nebula->getEffectShields();
			}

			std::sort(m_nebulas->begin(), m_nebulas->end());
		}

		for (IntVector::const_iterator it = oldNebulas.begin(); it != oldNebulas.end(); ++it)
		{
			int const id = *it;
			if (!std::binary_search(m_nebulas->begin(), m_nebulas->end(), id))
			{
				if (isPilotPlayer)
				{
					NebulaManagerClient::handlePlayerExit(id);
				}
			}
		}

		m_nebulaEffectReactor = clamp(-0.5f, m_nebulaEffectReactor, 1.0f);
		m_nebulaEffectEngine = clamp(-0.5f, m_nebulaEffectEngine, 1.0f);
		m_nebulaEffectShields = clamp(-2.0f, m_nebulaEffectShields, 1.0f);
	}
}

// ----------------------------------------------------------------------

ShipObject * ShipObject::asShipObject()
{
	return this;
}

// ----------------------------------------------------------------------

ShipObject const * ShipObject::asShipObject() const
{
	return this;
}

// ----------------------------------------------------------------------

InteriorLayoutReaderWriter const * ShipObject::getInteriorLayout() const
{
	return safe_cast<ClientShipObjectTemplate const *>(getObjectTemplate())->getInteriorLayout();
}

// ----------------------------------------------------------------------

void ShipObject::setPilot(CreatureObject * const pilot)
{
	m_pilot = pilot;
}

// ----------------------------------------------------------------------

CreatureObject const * ShipObject::getPilot() const
{
	return m_pilot;
}

// ----------------------------------------------------------------------

CreatureObject * ShipObject::getPilot()
{
	return m_pilot;
} //lint !e1762 //no const

// ----------------------------------------------------------------------

void ShipObject::findAllPassengers(stdvector<CreatureObject *>::fwd & passengers) const
{
	if (!isPobShip())
	{
		SlottedContainer const * const slottedContainer = getSlottedContainerProperty();
		if (slottedContainer != 0)
		{
			ContainerConstIterator ii = slottedContainer->begin();
			ContainerConstIterator iiEnd = slottedContainer->end();

			for (; ii != iiEnd; ++ii)
			{
				Object * const object = (*ii).getObject();
				ClientObject * const clientObject = (object != 0) ? object->asClientObject() : 0;
				CreatureObject * const creatureObject = (clientObject != 0) ? clientObject->asCreatureObject() : 0;
				if (creatureObject != 0)
				{
					passengers.push_back(creatureObject);
				}
			}
		}
	}

	else
	{
		// ship is a POB ship
		Container const * const pobContainer = ContainerInterface::getContainer(*this);
		if (pobContainer != 0)
		{
			ContainerConstIterator ii = pobContainer->begin();
			ContainerConstIterator iiEnd = pobContainer->end();
			for (; ii != iiEnd; ++ii)
			{
				Object * const containerObject = (*ii).getObject();
				ClientObject * const cell = (containerObject != 0) ? containerObject->asClientObject() : 0;

				if (cell != 0)
				{
					Container const * const cellContainer = ContainerInterface::getContainer(*cell);

					if (cellContainer != 0)
					{
						ContainerConstIterator jj = cellContainer->begin();
						ContainerConstIterator jjEnd = cellContainer->end();
						for (; jj != jjEnd; ++jj)
						{
							Object * const cellObject = (*jj).getObject();
							ClientObject * const cellContent = (cellObject != 0) ? cellObject->asClientObject() : 0;
							CreatureObject * const cellCreatureObject = (cellContent != 0) ? cellContent->asCreatureObject() : 0;

							if (cellCreatureObject != 0)
							{
								passengers.push_back(cellCreatureObject);
							}
							else
							{
								SlottedContainer const * const slottedContainer = (cellContent != 0) ? cellContent->getSlottedContainerProperty() : 0;

								if (slottedContainer != 0)
								{
									ContainerConstIterator kk = slottedContainer->begin();
									ContainerConstIterator kkEnd = slottedContainer->end();

									for (; kk != kkEnd; ++kk)
									{
										Object * const object = (*kk).getObject();
										ClientObject * const clientObject = (object != 0) ? object->asClientObject() : 0;
										CreatureObject * const slotCreatureObject = (clientObject != 0) ? clientObject->asCreatureObject() : 0;

										if (slotCreatureObject != 0)
										{
											passengers.push_back(slotCreatureObject);
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

/**
 * Creates a default controller for this object.
 *
 * @return the object's controller
 */
void ShipObject::createDefaultController()
{
	Controller * controller = NULL;
	if (getGameObjectType() == SharedObjectTemplate::GOT_ship_mining_asteroid_dynamic ||
			getGameObjectType() == SharedObjectTemplate::GOT_ship_mining_asteroid_static)
		controller = new MiningAsteroidController(this);
	else
		controller = new RemoteShipController(this);

	setController(NON_NULL(controller));
}

// ----------------------------------------------------------------------

void ShipObject::onShipPilotMounted(CreatureObject* pilot)
{
	setPilot(pilot);
	if (m_pilot.getPointer() != NULL && m_pilot.getPointer() == Game::getPlayerCreature())
	{
		CollisionProperty * const shipCollisionProperty = getCollisionProperty();
		if (shipCollisionProperty)
			shipCollisionProperty->setPlayer(true);

		PlayerShipController * const shipController = new PlayerShipController(this);
		if (isInitialized())
			shipController->endBaselines();
		setController(shipController);

		ShipWeaponGroupManager::updateWeaponGroups();
	} //lint !e429 shipController not freed or returned.  The setController() call passes ownership of this memory

	Transceivers::pilotMounted.emitMessage (*this);
}

// ----------------------------------------------------------------------

void ShipObject::onShipPilotDismounted()
{
	if (m_pilot.getPointer() != NULL && m_pilot.getPointer() == Game::getPlayerCreature())
	{
		CollisionProperty * const shipCollisionProperty = getCollisionProperty();
		if (shipCollisionProperty)
			shipCollisionProperty->setPlayer(false);

		RemoteShipController * const shipController = new RemoteShipController(this);
		shipController->setAuthoritative(false);
		if (isInitialized())
			shipController->endBaselines();
		setController(shipController);
	} //lint !e429 shipController not freed or returned.  The setController() call passes ownership of this memory

	setPilot(0);

	Transceivers::pilotDismounted.emitMessage (*this);
}

// ----------------------------------------------------------------------

void ShipObject::onShipGunnerMounted(CreatureObject const &gunner, int newWeaponIndex)
{
	if (&gunner == Game::getPlayerCreature())
	{
		m_shipWeaponStatusMap[newWeaponIndex].m_playerControlled = true;
		updateComponentAttachment(newWeaponIndex + static_cast<int>(ShipChassisSlotType::SCST_weapon_first));
		ShipWeaponGroupManager::updateWeaponGroups();
	}

	Transceivers::gunnerMounted.emitMessage (*this);
}

// ----------------------------------------------------------------------

void ShipObject::onShipGunnerDismounted(CreatureObject const &gunner, int oldWeaponIndex)
{
	if (&gunner == Game::getPlayerCreature())
	{
		m_shipWeaponStatusMap[oldWeaponIndex].m_playerControlled = false;
		updateComponentAttachment(oldWeaponIndex + static_cast<int>(ShipChassisSlotType::SCST_weapon_first));
	}

	Transceivers::gunnerDismounted.emitMessage (*this);
}

// ----------------------------------------------------------------------

void ShipObject::onEnterByHyperspace()
{
	RemoteShipController * remoteShipController = dynamic_cast<RemoteShipController *>(getController());
	if (remoteShipController != 0)
	{
		remoteShipController->enterByHyperspace();
	}
}

// ----------------------------------------------------------------------

void ShipObject::onLeaveByHyperspace()
{
	RemoteShipController * remoteShipController = dynamic_cast<RemoteShipController *>(getController());
	if (remoteShipController != 0)
	{
		remoteShipController->leaveByHyperspace();
	}
}

// ----------------------------------------------------------------------

CachedNetworkId const & ShipObject::getTurretTarget(int weaponIndex) const
{
	std::map<int, ShipWeaponStatus>::const_iterator const it = m_shipWeaponStatusMap.find(weaponIndex);
	if (it != m_shipWeaponStatusMap.end())
		return (*it).second.m_turretTarget;
	return CachedNetworkId::cms_cachedInvalid;
}

// ----------------------------------------------------------------------

bool ShipObject::isTurret(int weaponIndex) const
{
	ShipChassisSlotType::Type const shipChassisSlotType = static_cast<ShipChassisSlotType::Type>(static_cast<int>(ShipChassisSlotType::SCST_weapon_0) + weaponIndex);
	if (isSlotInstalled(shipChassisSlotType))
		return ShipTurretManager::isTurret(getChassisType(), weaponIndex);
	return false;
}

// ----------------------------------------------------------------------

bool ShipObject::isCountermeasure(int weaponIndex) const
{
	ShipChassisSlotType::Type const shipChassisSlotType = static_cast<ShipChassisSlotType::Type>(static_cast<int>(ShipChassisSlotType::SCST_weapon_0) + weaponIndex);
	if (isSlotInstalled(shipChassisSlotType))
	{
		uint32 const componentCrc = getComponentCrc(shipChassisSlotType);
		return ShipComponentWeaponManager::isCountermeasure(componentCrc);
	}
	return false;
}

//----------------------------------------------------------------------

const Object * ShipObject::getTurret(int weaponIndex) const
{
	if(isTurret(weaponIndex))
	{
		Transform hardpointTransform_o2p;
		return getShipObjectAttachments().getFirstAttachedObjectForWeapon(weaponIndex, hardpointTransform_o2p);
	}
	else
		return NULL;
}

// ----------------------------------------------------------------------

float ShipObject::getCurrentSpeed() const
{
	ShipController const * const shipController = safe_cast<ShipController const *>(getController());

	float currentSpeed = 0.0f;

	if (shipController)
	{
		currentSpeed = shipController->getCurrentSpeed();
	}

	return currentSpeed;
}

// ----------------------------------------------------------------------

float ShipObject::getSlideDampener() const
{
	return m_slideDampener.get();
}

// ----------------------------------------------------------------------

bool ShipObject::hasWings() const
{
	SharedShipObjectTemplate const * const sharedShipObjectTemplate = dynamic_cast<SharedShipObjectTemplate const *>(getObjectTemplate());
	if (!sharedShipObjectTemplate)
	{
		DEBUG_WARNING(true, ("ShipObject::hasWings(): ShipObject %s sharedTemplate is not a SharedShipObjectTemplate [template=%s]", getNetworkId().getValueString().c_str(), getObjectTemplateName()));
		return false;
	}

	return sharedShipObjectTemplate->getHasWings();
}

//----------------------------------------------------------------------

void ShipObject::stopFiringWeapon(int weaponIndex, bool notifyServer)
{
	ShipWeaponStatusMap::iterator const it = m_shipWeaponStatusMap.find(weaponIndex);

	if (it != m_shipWeaponStatusMap.end())
	{
		ShipWeaponStatus & status = (*it).second;

		if (status.m_isFiring && isBeamWeapon(weaponIndex))
			ProjectileManager::stopBeam(*this, weaponIndex, notifyServer);
		status.m_isFiring = false;
	}
}

//----------------------------------------------------------------------

void ShipObject::setWeaponFiring(int weaponIndex)
{
	//-- create new entry if needed
	m_shipWeaponStatusMap[weaponIndex].m_isFiring = true;
}

//----------------------------------------------------------------------

bool ShipObject::isWeaponFiring(int weaponIndex) const
{
	ShipWeaponStatusMap::const_iterator const it = m_shipWeaponStatusMap.find(weaponIndex);
	if (it != m_shipWeaponStatusMap.end())
		return (*it).second.m_isFiring;

	return false;
}

// ----------------------------------------------------------------------

/**
* @param weaponIndex the zero-based index of the weapon to fire.  A negative value indicates to fire all weapons
*/

bool ShipObject::fireWeapon(int weaponIndex)
{
	if (hasCondition(TangibleObject::C_docking))
	{
		DEBUG_REPORT_LOG(true, ("ShipObject::fireWeapon() Rejecting projectile due to TangibleObject::C_docking condition.\n"));
		return false;
	}

	Controller * const controller = getController();
	ShipController * const shipController = (controller != 0) ? controller->asShipController() : 0;
	PlayerShipController * const playerShipController = (shipController != 0) ? shipController->asPlayerShipController() : 0;

	if (playerShipController != 0)
	{
		if (playerShipController->isInputStateLocked())
		{
			DEBUG_REPORT_LOG(true, ("ShipObject::fireWeapon() Rejecting projectile due to PlayerShipController being locked.\n"));
			return false;
		}

		//-- Make sure if the ship has wings, that the wings are opened
		if (!isCountermeasure(weaponIndex) && hasWings() && !hasCondition(TangibleObject::C_wingsOpened))
		{
			// only try to open the wings if we haven't tried in the last 2 seconds
			unsigned long const currentTime = Clock::timeMs();
			if (currentTime < s_lastOpenWingsTimeMs || currentTime > s_lastOpenWingsTimeMs+2000u)
			{
				s_lastOpenWingsTimeMs = currentTime;
				static uint32 const hash_openWings = Crc::normalizeAndCalculate("openWings");
				if (Game::getSinglePlayer())
					setCondition(TangibleObject::C_wingsOpened);
				else
					IGNORE_RETURN(ClientCommandQueue::enqueueCommand(hash_openWings, NetworkId::cms_invalid, Unicode::emptyString));
			}
			return false;
		}
	}

	//-- fire all weapons
	if (weaponIndex < 0)
	{
		for (int i = 0; i < ShipChassisSlotType::cms_numWeaponIndices; ++i)
			IGNORE_RETURN(fireWeapon(i));

		return false;
	}

	if (weaponIndex >= ShipChassisSlotType::cms_numWeaponIndices)
		return false;

	ShipChassisSlotType::Type const weaponChassisSlotType = static_cast<ShipChassisSlotType::Type>(weaponIndex + static_cast<int>(ShipChassisSlotType::SCST_weapon_first));

	float const refireRate = getWeaponActualRefireRate(weaponChassisSlotType);

	bool const isBeam = isBeamWeapon(weaponIndex);
	bool const isBeamFiring = isBeam && m_shipWeaponStatusMap[weaponIndex].m_isFiring;

	//-- Verify that the refire rate is non-zero, the refire rate has elapsed and that there is enough energy in the capacitor
	if (refireRate <= 0.f)
		return false;

	Timer &refireTimer = m_shipWeaponStatusMap[weaponIndex].m_refireTimer;
	if (refireTimer.isExpired())
	{
		float const energyReq       = getWeaponActualEnergyPerShot(weaponChassisSlotType);
		float const energyAvailable = getCapacitorEnergyCurrent();
						
		//-- don't do a full reset of the timer just yet.
		//-- for error conditions that don't actually fire the weapon, we only want to wait a short time for next attempt
		
		refireTimer.setElapsedTime(std::max(0.0f, refireTimer.getExpireTime() - 0.5f));
		
		if (isComponentDisabled(weaponChassisSlotType))
		{
			CuiSystemMessageManager::sendFakeSystemMessage(SpaceStringIds::wpn_fire_fail_disabled.localize());
			CuiSoundManager::play(CuiSounds::negative);
			if (isBeamFiring)
				stopFiringWeapon(weaponIndex, true);
			return false;
		}
		
		if (energyReq > energyAvailable)
		{
			CuiSoundManager::play(CuiSounds::negative);
			if (isBeamFiring)
				stopFiringWeapon(weaponIndex, true);
			return false;
		}

		uint32 const componentCrc = getComponentCrc(weaponChassisSlotType);
		
		if (ShipComponentWeaponManager::isAmmoConsuming(componentCrc))
		{
			int const ammoAvailable = getWeaponAmmoCurrent (weaponChassisSlotType);
			if (ammoAvailable <= 0)
			{
				CuiSystemMessageManager::sendFakeSystemMessage(SpaceStringIds::wpn_fire_fail_ammo.localize());
				CuiSoundManager::play(CuiSounds::negative);

				if (isBeamFiring)
					stopFiringWeapon(weaponIndex, true);

				return false;
			}
		}
		
		if (ShipComponentWeaponManager::isMissile(componentCrc))
		{
			CreatureObject const * const playerCreature = NON_NULL(Game::getPlayerCreature());
			ClientObject const * const clientTarget = safe_cast<ClientObject const *>(playerCreature->getLookAtTarget().getObject());
			
			if (clientTarget == NULL)
			{
				CuiSystemMessageManager::sendFakeSystemMessage(SpaceStringIds::wpn_fire_fail_missile_target.localize());
				CuiSoundManager::play(CuiSounds::negative);
				return false;
			}
			
			if (clientTarget == this)
			{
				CuiSystemMessageManager::sendFakeSystemMessage(SpaceStringIds::wpn_fire_fail_missile_self.localize());
				CuiSoundManager::play(CuiSounds::negative);
				return false;
			}
			
			// Check to see if the target is in range.
			if ((clientTarget->getPosition_w() - getPosition_w()).magnitudeSquared() > sqr(getWeaponRange(weaponIndex)))
			{
				CuiSystemMessageManager::sendFakeSystemMessage(SpaceStringIds::wpn_fire_fail_missile_range.localize());
				CuiSoundManager::play(CuiSounds::negative);
				return false;
			}
			
			// Check to see if the target is acquired.
			if (!m_missleUpdateData[weaponIndex].m_acquired)
			{
				CuiSystemMessageManager::sendFakeSystemMessage(SpaceStringIds::wpn_fire_fail_missile_acquire.localize());
				CuiSoundManager::play(CuiSounds::negative);
				return false;
			}
		}
		
		float const projectileTimeToLive = computeWeaponProjectileTimeToLive(weaponIndex);
		float const projectileSpeed = getWeaponProjectileSpeed(weaponIndex);
		int const projectileIndex = getProjectileIndexForWeapon(weaponIndex);

		if (projectileTimeToLive <= 0.0f)
			WARNING(true, ("ShipObject weapon [%d] has invalid projectile time to live (projectile index=%d).", weaponIndex, projectileIndex));
		else
		{
			refireTimer.reset();
						
			ProjectileManager::createClientProjectile(this, weaponIndex, projectileIndex, projectileSpeed, projectileTimeToLive);
			m_shipWeaponStatusMap[weaponIndex].m_isFiring = true;
		}
	}

	return true;
}

// ----------------------------------------------------------------------

void ShipObject::componentCrcOnSet           (const int & key, uint32 const & oldValue, uint32 const & value)
{
	UNREF (oldValue);
	UNREF (value);
	updateComponentAttachment (key);
	Transceivers::componentsChanged.emitMessage (*this);
}

//----------------------------------------------------------------------

void ShipObject::componentCrcOnInsert        (const int & key, uint32 const & value)
{
	UNREF (value);
	updateComponentAttachment (key);
	Transceivers::componentsChanged.emitMessage (*this);
}

//----------------------------------------------------------------------

void ShipObject::componentCrcOnErase         (const int & key, uint32 const & value)
{
	UNREF (value);
	updateComponentAttachment (key);
	Transceivers::componentsChanged.emitMessage (*this);
}

//----------------------------------------------------------------------

void ShipObject::cargoHoldContentsResourceTypeInfoOnSet(NetworkId const & key, ResourceTypeInfoPair const & oldValue, ResourceTypeInfoPair const & value)
{
	UNREF(oldValue);
	ResourceTypeManager::setTypeInfo(key, value.first, value.second);
}

//----------------------------------------------------------------------

void ShipObject::cargoHoldContentsResourceTypeInfoOnInsert(NetworkId const & key, ResourceTypeInfoPair const & value)
{
	ResourceTypeManager::setTypeInfo(key, value.first, value.second);
}

// ----------------------------------------------------------------------

float ShipObject::getCurrentChassisHitPoints() const
{
	return m_currentChassisHitPoints.get();
}

// ----------------------------------------------------------------------

float ShipObject::getMaximumChassisHitPoints() const
{
	return m_maximumChassisHitPoints.get();
}

// ----------------------------------------------------------------------

bool ShipObject::getTargetLeadPosition_w(Vector & targetPosition_w, Object const & lookAtObject) const
{
	bool hasValidTargetPosition = false;
	bool isTargetingSelf = &lookAtObject == this;

	ClientObject const * const lookAtClientObject = lookAtObject.asClientObject();
	TangibleObject const * const lookAtTangibleObject = lookAtClientObject ? lookAtClientObject->asTangibleObject() : NULL;
	
	if (!isTargetingSelf && lookAtTangibleObject)
	{
		float const timeDelta = Clock::frameTime();

		ShipObject const * const lookAtShipObject = lookAtTangibleObject->asShipObject();
		float const lookAtObjectSpeed = lookAtShipObject ? lookAtShipObject->getShipVelocity_w().approximateMagnitude() : 0.0f;

		// First, check to make sure the thing is moving.
		if (lookAtObjectSpeed > FLT_MIN && lookAtShipObject != NULL)
		{
			Vector const & lookAtObjectPosition_w = getTargetInterceptPosition_w(timeDelta, *lookAtShipObject);
			Vector const & localShipPosition_w = getInterceptPosition_w(timeDelta);
			
			float const lookAtObjectDistance = lookAtObjectPosition_w.magnitudeBetween(localShipPosition_w);

			int weaponIndex = ShipChassisSlotType::cms_numWeaponIndices;

			bool const isPlayerShip = Game::getPlayerPilotedShip() == this;
			if (isPlayerShip)
			{
				//-- invalid is ShipChassisSlotType::cms_numWeaponIndices
				weaponIndex = ShipWeaponGroupManager::getWeaponIndexFromCurrentGroup(ShipWeaponGroupManager::GT_Projectile);
			}
			else
			{
				CreatureObject const * const player = Game::getPlayerCreature();
				if (NULL != player)
				{
					//-- invalid is neg 1
					weaponIndex = ShipStation::getWeaponIndexForGunnerStation(player->getShipStation());
				}
			}
			
			if (weaponIndex >= 0 && weaponIndex < ShipChassisSlotType::cms_numWeaponIndices)
			{
				float const projectileSpeed = getWeaponProjectileSpeed(weaponIndex);
				
				//-- Check the projectile range here.
				if (lookAtObjectDistance <= projectileSpeed * computeWeaponProjectileTimeToLive(weaponIndex) && lookAtObjectDistance > FLT_MIN)
				{
					Vector lookAtObjectDirection = lookAtShipObject->getShipVelocity_w();
					
					if (lookAtObjectDirection.approximateNormalize())
					{
						if (!WithinEpsilonInclusive(0.0f, projectileSpeed, cs_minimumTargetLeadProjectSpeed))
						{
							Vector lookAtObjectHeading = localShipPosition_w - lookAtObjectPosition_w;
							
							if (lookAtObjectHeading.approximateNormalize())
							{
								float const projectileTimeOfFlight = getTargetLeadTimeOfFlight(lookAtObjectHeading, lookAtObjectDirection, lookAtObjectDistance, projectileSpeed, lookAtObjectSpeed);
								targetPosition_w = (lookAtObjectDirection * (projectileTimeOfFlight * lookAtObjectSpeed)) + lookAtObjectPosition_w;
								hasValidTargetPosition = true;
							}
						}
					}
				}
			}
		}

		// If the target lead fails, but you have a valid ship, just use it's intercept position.
		if (!hasValidTargetPosition)
		{
			targetPosition_w = getTargetInterceptPosition_w(timeDelta, *lookAtTangibleObject);
			hasValidTargetPosition = true;
		}
	}

	return hasValidTargetPosition;
}

// ----------------------------------------------------------------------
//            1
// Ta = _______________ * (-2*s*cos(g)+2*(s^2*cos(g)^2+p^2-s^2)^(1/2)) * r
//     2 * (p^2 - s^2)
//            1
// Tb = _______________ * (-2*s*cos(g)-2*(s^2*cos(g)^2+p^2-s^2)^(1/2)) * r
//     2 * (p^2 - s^2)
//
// t = projectile time of flight
// ----------------------------------------------------------------------
// g = angle between the heading of the target and the direction between the target and the player.
// r = distance between player and target
// s = speed of the target
// p = speed of the projectile

float ShipObject::getTargetLeadTimeOfFlight(Vector const & headingTargetToPlayer, Vector const & targetHeading, float distanceToTarget, float projectileSpeed, float lookAtShipSpeed) const
{
	float timeOfFlight = 0.0f;

	//-- This is an optimized version of the formula listed above.
	if (WithinEpsilonInclusive(projectileSpeed, lookAtShipSpeed, cs_minimumTargetLeadProjectSpeed))
	{
		float cosineGamma                                 = headingTargetToPlayer.dot(targetHeading);
		float cosineGammaSquared                          = sqr(cosineGamma);
		float projectileSpeedSquared                      = sqr(projectileSpeed);
		float lookAtShipSpeedSquared                      = sqr(lookAtShipSpeed);
		float diffProjectileSpeedSquaredLookAtShipSquared = projectileSpeedSquared - lookAtShipSpeedSquared;

		float oo2PsQsR                    = (1.0f / (2.0f * diffProjectileSpeedSquaredLookAtShipSquared)) * distanceToTarget;
		float neg2SCosGamma               = -2.0f * lookAtShipSpeed * cosineGamma;

		//-- Imaginary positions are bad.
		float nonNegativeTerms            = lookAtShipSpeedSquared * cosineGammaSquared + diffProjectileSpeedSquaredLookAtShipSquared;
		if (nonNegativeTerms > 0.0f)
		{
			float pos2SsCosGammasPlusPsMinSs  = 2.0f * sqrtf(nonNegativeTerms);

			//-- Take the largest solution.
			float solutionA                   = oo2PsQsR * (neg2SCosGamma - pos2SsCosGammasPlusPsMinSs);
			float solutionB                   = oo2PsQsR * (neg2SCosGamma + pos2SsCosGammasPlusPsMinSs);

			if (solutionA > 0.0f || solutionB > 0.0f)
			{
				timeOfFlight = std::max(solutionA, solutionB);
			}
		}
	}

	//-- If the time of flight is invalid, default to this value.
	if (timeOfFlight <= 0.0f)
	{
		timeOfFlight = distanceToTarget / projectileSpeed;
	}

	return timeOfFlight;
}

//----------------------------------------------------------------------

bool ShipObject::hasMissileTargetAcquisition() const
{
	return 	m_hasTargetAcquired; 
}

//----------------------------------------------------------------------

bool ShipObject::isMissileTargetAcquiring() const
{
	return m_hasTargetAcquiring;
}

//----------------------------------------------------------------------

bool ShipObject::hasMissiles() const
{
	return m_hasMissiles;
}

//----------------------------------------------------------------------

bool ShipObject::getWeaponRefireTimeRemaining(int weaponIndex, float & remaining, float & totalExpireTime) const
{
	if (weaponIndex >= 0 && weaponIndex < ShipChassisSlotType::cms_numWeaponIndices)
	{
		std::map<int, ShipWeaponStatus>::const_iterator i = m_shipWeaponStatusMap.find(weaponIndex);
		if (i != m_shipWeaponStatusMap.end())
		{
			Timer const &refireTimer = (*i).second.m_refireTimer;
			remaining = -refireTimer.getExpirationOvershoot();
			totalExpireTime = refireTimer.getExpireTime();
			return true;
		}
	}
	return false;
}

//----------------------------------------------------------------------

float ShipObject::getTimeUntilMissileTargetAcquisition() const
{
	float timeToAcquire = 0.0f;

	int currentWeaponIndex = ShipWeaponGroupManager::getWeaponIndexFromCurrentGroup(ShipWeaponGroupManager::GT_Missile);
	if (isMissile(currentWeaponIndex))
	{
		timeToAcquire = -m_missleUpdateData[currentWeaponIndex].m_missileTargetAcquisitionTimer.getExpirationOvershoot();
	}

	return timeToAcquire;
}

//----------------------------------------------------------------------

float ShipObject::getNebulaEffectEngine() const
{
	return m_nebulaEffectEngine;
}

//----------------------------------------------------------------------

void ShipObject::setPilotLookAtTarget (const NetworkId& id)
{
	DEBUG_FATAL (Game::getPlayerPilotedShip() != this,("setPilotLookAtTarget should only be called on the ship the player is targeting"));
	
	if (Game::getPlayerPilotedShip() == this && getController ())
	{
		ms_lastLookAtTargetRequest = id;
		NON_NULL (safe_cast<ShipController *> (getController ()))->setLookAtTarget (id);
	}


	if (m_hasTargetAcquired)
	{
		m_hasTargetAcquiring = false;
		m_hasTargetAcquired = false;
		Transceivers::targetAcquisition.emitMessage(*this);
	}

	//since we just targeted a new ship, make sure we aren't targeting a component
	setPilotLookAtTargetSlot(ShipChassisSlotType::SCST_num_types);

	//immediately set the lookAtTarget on the client, since setting it on the server should never fail
	m_pilotLookAtTarget.set(CachedNetworkId (id));

	//Set the current missile group.
	m_missileGroup = ShipWeaponGroupManager::getCurrentGroup(ShipWeaponGroupManager::GT_Missile);
	m_missileSlot = -1;

	//-- reset missile locks
	resetMissileAcquisitionTimers();
}

//----------------------------------------------------------------------

void ShipObject::onLocalProjectileFired(int weaponIndex)
{
	Transform hardpointTransform_o2p;
	Object * const weaponObject = getShipObjectAttachments().getFirstAttachedObjectForWeapon(weaponIndex, hardpointTransform_o2p);

	if (weaponObject)
		onShipWeaponBarrelFired(*weaponObject);
}

//----------------------------------------------------------------------

void ShipObject::onRemoteProjectileFired(int weaponIndex, Vector const &endPosition_w)
{
	if (isTurret(weaponIndex))
	{
		// snap to the orientation for firing
		orientShipTurret(weaponIndex, endPosition_w);
	}

	Transform hardpointTransform_o2p;
	Object * const weaponObject = getShipObjectAttachments().getFirstAttachedObjectForWeapon(weaponIndex, hardpointTransform_o2p);
	if (weaponObject)
		onShipWeaponBarrelFired(*weaponObject);
}

//----------------------------------------------------------------------

void ShipObject::onWeaponHitTarget(int weaponIndex, CachedNetworkId const &target)
{
	if (isTurret(weaponIndex))
		m_shipWeaponStatusMap[weaponIndex].m_turretTarget = target;
}

//----------------------------------------------------------------------

void ShipObject::Callbacks::LookAtTargetChanged::modified (ShipObject & target, const CachedNetworkId & oldId, const CachedNetworkId & newId, bool) const
{
	if (&target == Game::getPlayerPilotedShip())
	{
		if (ms_lastLookAtTargetRequest == newId)
		{
			if (newId.getObject() == &target)
			{
				enableTargetAppearanceFromObjectId(&target, newId, false, static_cast<int>(ShipChassisSlotType::SCST_invalid));
				enableTargetAppearanceFromObjectId(&target, oldId, false, static_cast<int>(ShipChassisSlotType::SCST_invalid));
			}
			else
			{
				enableTargetAppearanceFromObjectId(&target, newId, true, static_cast<int>(target.getPilotLookAtTargetSlot()));
				enableTargetAppearanceFromObjectId(&target, oldId, false, static_cast<int>(ShipChassisSlotType::SCST_invalid));
			}

			Transceivers::lookAtTargetChanged.emitMessage (target);
		}	
	}

	//-- update the targeting player list
	if (Game::getPlayerContainingShip() != NULL)
	{
		if (oldId == Game::getPlayerContainingShip()->getNetworkId())
		{
			IGNORE_RETURN(ms_shipsTargetingPlayer.erase(std::remove(ms_shipsTargetingPlayer.begin(), ms_shipsTargetingPlayer.end(), &target), ms_shipsTargetingPlayer.end()));
		}
		if (newId == Game::getPlayerContainingShip()->getNetworkId())
		{
			ms_shipsTargetingPlayer.push_back(&target);
		}
	}
	else
	{
		ms_shipsTargetingPlayer.clear();
	}
}

//----------------------------------------------------------------------

void ShipObject::generateDebrisSplittingPlanes(Object const & object, int maxRandomPlanes, float const severity, PlaneVector & planeVector)
{
	Appearance const * const appearance = object.getAppearance();
	if (appearance != NULL)
	{
		ClientObject const * const clientAttachedObject = object.asClientObject();
		ClientDataFile const * const clientDataFile = clientAttachedObject != NULL ? clientAttachedObject->getClientData() : NULL;

		if (clientDataFile == NULL || clientAttachedObject == NULL || !clientDataFile->processBreakpoints(*clientAttachedObject, severity, planeVector))
		{
			//-- if the client data file does not contain breakpoints, just generate some randomly

			float const radius = appearance->getSphere().getRadius();
			float const halfRadius = radius * 0.5f;
			int const numPlanes = Random::random(maxRandomPlanes/2, maxRandomPlanes);
			for (int i = 0; i < numPlanes; ++i)
			{
				Vector const & planePos = Vector::randomUnit() * Random::randomReal(0.0f, halfRadius);
				Vector const & planeNormal = Vector::randomUnit();
				Plane const plane(planeNormal, planePos);

				planeVector.push_back(plane);
			}
		}
	}

	if (planeVector.empty())
	{
		//-- just use a plane totally outside the object extent, so we can generate a debris object
		AxialBox const & box = object.getTangibleExtent();
		Plane const plane(Vector::unitZ, box.getExtentZ() + 1.0f);
		planeVector.push_back(plane);
	}
}

//----------------------------------------------------------------------

void ShipObject::splitShipComponent(Object & object, int maxRandomPlanes, float severity, DestructionDebrisList * debrisList) const
{
	PlaneVector planeVector;
	generateDebrisSplittingPlanes(object, maxRandomPlanes, severity, planeVector);
	splitShipComponent(object, planeVector, debrisList);
}

//----------------------------------------------------------------------

void ShipObject::splitShipComponent(Object & object, PlaneVector const & planeVector, DestructionDebrisList * const debrisList) const
{
	Vector const & currentVelocity = getCurrentSpeed() * getObjectFrameK_w();

	int const oldDynamicMeshAppearanceDetailLevelBias = DynamicMeshAppearance::getDetailLevelBias();
	DynamicMeshAppearance::setDetailLevelBias(CuiPreferences::getShipDestroyDetailBias());

	//-- use generated planes to split the object

	typedef stdvector<DynamicDebrisObject *>::fwd DynamicDebrisObjectVector;
	DynamicDebrisObjectVector dynamicDebrisObjectVector;

	Vector const & shipCenter_w = rotateTranslate_o2w(getAppearanceSphereCenter());
	Vector const & attachmentToShipOffset = object.rotateTranslate_o2w(object.getAppearanceSphereCenter()) - shipCenter_w;

	float const debrisParticleThreshold = object.getTangibleExtent().getRadius() * 0.5f;

	if (!DynamicDebrisObject::splitObjects(object, planeVector, dynamicDebrisObjectVector, debrisParticleThreshold))
	{
		DEBUG_FATAL(!dynamicDebrisObjectVector.empty(), ("ShipObject call to DynamicDebrisObject::splitObjects failed but created object(s)"));
		return;
	}

	float const shipDebrisLifespan = CuiPreferences::getShipDebrisLifespan();

	for (DynamicDebrisObjectVector::iterator it = dynamicDebrisObjectVector.begin(); it != dynamicDebrisObjectVector.end(); ++it)
	{
		DynamicDebrisObject * const dynamicDebrisObject = *it;
		if (dynamicDebrisObject == NULL)
			continue;

		dynamicDebrisObject->setTransform_o2w(object.getTransform_o2w());

		//-- compute the offset velocity
		static float const s_velocityOffsetMax = 30.0f;

		Vector const & debrisAppearanceCenter_o = dynamicDebrisObject->getTangibleExtent().getCenter();
		Vector const & offset_w = dynamicDebrisObject->rotate_o2w(debrisAppearanceCenter_o);
		Vector velocityOffset = (attachmentToShipOffset + offset_w);
//		velocityOffset.x += Random::randomReal(-5.0f, 5.0f);
//		velocityOffset.y += Random::randomReal(-5.0f, 5.0f);
//		velocityOffset.z += Random::randomReal(-5.0f, 5.0f);
		velocityOffset *= Random::randomReal(0.5f, 1.0f);
		velocityOffset.x = clamp(-s_velocityOffsetMax, velocityOffset.x, s_velocityOffsetMax);
		velocityOffset.y = clamp(-s_velocityOffsetMax, velocityOffset.y, s_velocityOffsetMax);
		velocityOffset.z = clamp(-s_velocityOffsetMax, velocityOffset.z, s_velocityOffsetMax);

		//-- compute the rotation

		float const appearanceRadius = object.getAppearanceSphereRadius();
		Vector const & sphereCenter = dynamicDebrisObject->getAppearanceSphereCenter();

		//-- offsets affect rotation:
		//-- x: yaw
		//-- z: pitch
		//-- y: roll

		Vector rotation(sphereCenter.x, sphereCenter.z, sphereCenter.y);
		if (appearanceRadius > 0.0f)
			rotation /= appearanceRadius;

		rotation *= Random::randomReal(1.0f, 8.0f);

		static float const s_rotationMinimumThreshold = 0.05f;
		static float const s_rotationRateMax = 4.0f;

		//-- if the rotation vector is too small, find the largest component and amplify it
		if (rotation.magnitudeSquared() < s_rotationMinimumThreshold)
		{
			Vector fabsRotation(fabsf(rotation.x), fabsf(rotation.y), fabsf(rotation.z));

			if ((fabsRotation.x > fabsRotation.y) && (fabsRotation.x > fabsRotation.z))
				rotation.x = Random::randomReal(rotation.x, rotation.x + s_rotationRateMax);
			else if ((fabsRotation.y > fabsRotation.x) && (fabsRotation.y > fabsRotation.z))
				rotation.y = Random::randomReal(rotation.y, rotation.y + s_rotationRateMax);
			else if ((fabsRotation.z > fabsRotation.x) && (fabsRotation.z > fabsRotation.y))
				rotation.z = Random::randomReal(rotation.z, rotation.z + s_rotationRateMax);
		}

		rotation.x = clamp(-s_rotationRateMax, rotation.x, s_rotationRateMax);
		rotation.y = clamp(-s_rotationRateMax, rotation.y, s_rotationRateMax);
		rotation.z = clamp(-s_rotationRateMax, rotation.z, s_rotationRateMax);

		dynamicDebrisObject->setPhysicsParameters(shipDebrisLifespan, currentVelocity + velocityOffset, rotation);

		RenderWorld::addObjectNotifications (*dynamicDebrisObject);
		dynamicDebrisObject->addNotification(ClientWorld::getIntangibleNotification());
		dynamicDebrisObject->addToWorld();
		dynamicDebrisObject->scheduleForAlter();

		if (debrisList)
			debrisList->push_back(dynamicDebrisObject);
	}

	DynamicMeshAppearance::setDetailLevelBias(oldDynamicMeshAppearanceDetailLevelBias);
}

//----------------------------------------------------------------------

void ShipObject::startShipDestructionSequence(float const severity)
{
	DEBUG_FATAL(hasCondition(C_destroying), ("ShipObject already in destruction"));

	ClientDataFile const * const cdf = getClientData();
	if (NULL != cdf)
	{
		if (NULL != cdf->getDestructionSequence())
		{
			setCondition(C_destroying);
			IGNORE_RETURN(new ShipObjectDestruction(*this, severity));
			return;
		}
	}

	//-- fall through to simply destroy the ship
	handleFinalShipDestruction(severity, NULL);
}

//----------------------------------------------------------------------

void ShipObject::handleFinalShipDestruction(float severity, DestructionDebrisList * const debrisList)
{
	static PlaneVector emptyPlaneVector;
	handleFinalShipDestruction(severity, emptyPlaneVector, debrisList);
}

//----------------------------------------------------------------------

void ShipObject::hideShipPostDestruction()
{
	//-- start running the post destruction timer
	m_timerPostDestruction.setExpireTime(s_hideTimePostDestruction);
	m_timerPostDestruction.reset();
	setObjectAndChildrenActive(*this, false);
	ShipController * const shipController = (NULL != getController()) ? getController()->asShipController() : NULL;
	RemoteShipController * const remoteShipController = dynamic_cast<RemoteShipController *>(shipController);
	
	Vector serverPos_w;
	
	if (NULL != remoteShipController)
		m_lastPositionAtDestruction_w = remoteShipController->getServerTransform_p().getPosition_p();
	else
		m_lastPositionAtDestruction_w = getPosition_w();
}

//----------------------------------------------------------------------

void ShipObject::handleFinalShipDestruction(float severity, PlaneVector const & planeVector, DestructionDebrisList * const debrisList)
{
	getShipObjectEffects().removeAllTargetingEffects();

	PerformanceTimer ptimer;
	ptimer.start();

	//-- rip off any damaged components as needed
	{
		for (int chassisSlot = 0; chassisSlot < static_cast<int>(ShipChassisSlotType::SCST_num_types); ++chassisSlot)
		{
			if (!isSlotInstalled(chassisSlot))
				continue;

			float const hpMax = getComponentHitpointsMaximum(chassisSlot);
			float const hpCur = getComponentHitpointsCurrent(chassisSlot);

			float const armorMax = getComponentArmorHitpointsMaximum(chassisSlot);
			float const armorCur = getComponentArmorHitpointsCurrent(chassisSlot);

			int const damageMax = static_cast<int>(hpMax + armorMax);
			int const damageCur = static_cast<int>(hpCur + armorCur);

			float const damageLevel = (damageMax > 0) ? (static_cast<float>(damageCur) / damageMax) : 1.0f;

			//-- damageLevel is 0.0f for fully damaged, 1.0f for undamaged
			if (Random::randomReal(0.0f, 2.0f) > damageLevel)
			{
				//we don't want to remove the crc in case the ship re-spawns.
				handleComponentDestruction(chassisSlot, severity, false);
			}
		}
	}

	//-- rip off the wings if necessary
	{
		//-- if the wings are opened, we must rip at least one of them off
		bool const mustRipOffWing = hasWings() && hasCondition(static_cast<int>(C_wingsOpened));
		bool hasRippedOffWing = false;

		for (int i = 0; i < getNumberOfChildObjects(); ++i)
		{
			TangibleObject * const wing = dynamic_cast<TangibleObject *>(getChildObject(i));
			if (wing != NULL)
			{
				if (dynamic_cast<WingDynamics *>(wing->getDynamics()) != NULL)
				{
					//-- damgeLevel is 1.0f for fully damaged, 0.0f for undamaged
					if ((mustRipOffWing && !hasRippedOffWing) ||
						Random::randomReal(0.0f, 1.0f) < wing->getDamageLevel())
					{
						splitShipComponent(*wing, 3, severity, debrisList);
						delete wing;
						hasRippedOffWing = true;
					}
				}
			}
		}
	}

	if (planeVector.empty())
	{
		//-- mining asteroids don't split
		int const got = getGameObjectType();
		if (got != SharedObjectTemplate::GOT_ship_mining_asteroid_dynamic &&
			got != SharedObjectTemplate::GOT_ship_mining_asteroid_static)
		{
			splitShipComponent(*this, 4, severity, debrisList);
		}
	}
	else
		splitShipComponent(*this, planeVector, debrisList);

	//-- hide ship AFTER components, wings, effects have been split off, we don't want to deactivate before they get transferred
	hideShipPostDestruction();


	ClientDataFile const * const clientDataFile = getClientData();
	if (clientDataFile != NULL)
		clientDataFile->playDestructionEffect(*this);

	ptimer.stop();
	if (s_logDestructionTimes)
		ptimer.logElapsedTime(" *** ShipObject::handleShipDestruction split ship in ");
}

//----------------------------------------------------------------------

void ShipObject::handleComponentDestruction(int chassisSlot, float severity, bool removeComponentCrc)
{
	if (!isSlotInstalled(chassisSlot))
		return;

	PerformanceTimer ptimer;
	ptimer.start();

	m_shipObjectAttachments->handleComponentDestruction(chassisSlot, severity);

	if (removeComponentCrc)
	{
		//-- changing this auto delta variable triggers the attachment removal
		IGNORE_RETURN(m_componentCrc.erase(chassisSlot));
	}
	else
	{
		//-- manually handle the attachment removal since the crc is not changing
		getShipObjectAttachments().removeComponentAttachments (chassisSlot);
	}

	ptimer.stop();
	if (s_logDestructionTimes)
		ptimer.logElapsedTime("    *** ShipObject::handleComponentDestruction split component in ");
}

//----------------------------------------------------------------------

void ShipObject::clientSetChassisHitPoints(float cur, float max)
{
	m_maximumChassisHitPoints = max;
	m_currentChassisHitPoints = cur;
}

//----------------------------------------------------------------------

bool ShipObject::isCapitalShip() const
{
	return (static_cast<SharedObjectTemplate::GameObjectType>(getGameObjectType()) == SharedObjectTemplate::GOT_ship_capital || 
		static_cast<SharedObjectTemplate::GameObjectType>(getGameObjectType()) == SharedObjectTemplate::GOT_ship_station);
}

//----------------------------------------------------------------------

bool ShipObject::isPobShip() const
{
	return getPortalProperty() != NULL;
}

//----------------------------------------------------------------------

bool ShipObject::isMultiPassenger() const
{
	if (isPobShip())
	{
		return true;
	}

	SlottedContainer const * const slottedContainer = getSlottedContainerProperty();
	if (slottedContainer != 0)
	{
		typedef std::vector<SlotId> SlotVector;

		SlotVector slots;
		slottedContainer->getSlotIdList(slots);

		SlotVector::const_iterator ii = slots.begin();
		SlotVector::const_iterator iiEnd = slots.end();

		for (; ii != iiEnd; ++ii)
		{
			SlotId const slotId = *ii;

			if (slotId == ShipSlotIdManager::getShipPilotSlotId())
			{
				continue;
			}

			if (slotId == ShipSlotIdManager::getShipDroidSlotId())
			{
				continue;
			}

			return true;
		}
	}

	return false;
}

//----------------------------------------------------------------------

void ShipObject::Callbacks::DroidPcdChanged::modified(ShipObject &target, NetworkId const &, NetworkId const &, bool) const
{
	Transceivers::droidPcdChanged.emitMessage(target);
}

//----------------------------------------------------------------------

void ShipObject::Callbacks::ShipIdChanged::modified(ShipObject &target, uint16 const &oldId, uint16 const &newId, bool) const
{
	ShipIdMap::iterator i = s_shipsByShipId.find(oldId);
	if (i != s_shipsByShipId.end() && (*i).second == &target)
		s_shipsByShipId.erase(i);
	if (newId != 0)
		s_shipsByShipId[newId] = &target;
}

//----------------------------------------------------------------------

void ShipObject::Callbacks::CargoChanged::modified(ShipObject &target, int const &, int const &, bool) const
{
	Transceivers::cargoChanged.emitMessage(target);
}

//----------------------------------------------------------------------

NetworkId const &ShipObject::getInstalledDroidControlDevice() const
{
	return m_installedDroidControlDevice.get();
}

//----------------------------------------------------------------------

Vector const ShipObject::getInterceptPosition_w(float const timeElapsed) const
{
	ShipController const * const shipController = getController() ? getController()->asShipController() : NULL;

	if (NULL != shipController && timeElapsed > 0.0f)
		return shipController->getServerPredictedTransform_o2w().getPosition_p() + (shipController->getVelocity_p() * timeElapsed);
	else
		return getPosition_w() + (getShipVelocity_w() * timeElapsed);
}

// ----------------------------------------------------------------------

Vector const ShipObject::getShipVelocity_w() const
{
	Vector const & velocity = getController() ? safe_cast<ShipController const *>(getController())->getVelocity_p() : getObjectFrameK_w() * getCurrentSpeed();
	return velocity;
}

// ----------------------------------------------------------------------
// Return the lookAt target's position with an offset if the player is 
// targeting a slot on the target ship.

Vector const ShipObject::getTargetInterceptPosition_w(float const frameTime, TangibleObject const & targetObject)
{
	ShipObject const * const targetShipObject = targetObject.asShipObject();
	
	if (NULL == targetShipObject)
	{
		return targetObject.getPosition_w();
	}
	
	Vector targetPosition_w = targetShipObject->getInterceptPosition_w(frameTime);
	
	CreatureObject const * const playerCreature = Game::getPlayerCreature();
	CachedNetworkId const & playerTargetId = playerCreature ? playerCreature->getLookAtTarget() : CachedNetworkId::cms_cachedInvalid;
	ShipChassisSlotType::Type const playerTargetSlotType = playerCreature ? playerCreature->getLookAtTargetSlot() : ShipChassisSlotType::SCST_invalid;
	
	//-- targetting a slot, push the targetPosition out to compensate
	if ((targetObject.getNetworkId() == playerTargetId) && (playerTargetSlotType != ShipChassisSlotType::SCST_invalid))
	{
		Vector componentPosition_o;
		if (targetShipObject->getShipObjectAttachments().getFirstComponentPosition_o(playerTargetSlotType, componentPosition_o))
		{
			targetPosition_w += targetShipObject->rotate_o2w(componentPosition_o);
		}
	}
	
	return targetPosition_w;
}

//----------------------------------------------------------------------

#ifdef _DEBUG
void ShipObject::alterDebug(float const elapsedTime)
{
	Camera const * const camera = Game::getCamera();

	if (camera != NULL && (ms_showOrientation || ms_showAvoidancePathSelf || ms_showAvoidancePathOther))
	{
		bool isPlayerShip = Game::getPlayerPilotedShip() == this;

		Vector const & shipPosition = getPosition_w();
		Vector const & shipOrientation = getTransform_o2w().getLocalFrameK_p();
		float const currentSpeed = getCurrentSpeed();
		float forwardSpeed = currentSpeed;
		
		if((!isPlayerShip && ms_exaggerateAvoidancePathOther) || (isPlayerShip && ms_exaggerateAvoidancePathSelf))
		{
			forwardSpeed += cms_exaggerateAvoidanceValue;
		}

		float const radius = getAppearanceSphereRadius();
		Vector const shipRightAxis(rotateTranslate_o2w(Vector(radius * 1.4f, 0.0f, 0.0f)) - getPosition_w());
		Vector const shipUpAxis(rotateTranslate_o2w(Vector(0.0f, radius * 1.4f, 0.0f)) - getPosition_w());
		Vector const shipForwardAxis(rotateTranslate_o2w(Vector(0.0f, 0.0f, radius * 2.5f)) - getPosition_w());

		// Draw the orientation of the ship
		if(ms_showOrientation)
		{
			camera->addDebugPrimitive(new Line3dDebugPrimitive(UtilityDebugPrimitive::S_z, Transform::identity, shipPosition - shipRightAxis, shipPosition + shipRightAxis, PackedArgb::solidRed));
			camera->addDebugPrimitive(new Line3dDebugPrimitive(UtilityDebugPrimitive::S_z, Transform::identity, shipPosition, shipPosition + shipUpAxis, PackedArgb::solidGreen));
			camera->addDebugPrimitive(new Line3dDebugPrimitive(UtilityDebugPrimitive::S_z, Transform::identity, shipPosition, shipPosition + shipForwardAxis, PackedArgb::solidBlue));
		}
		
		// Draw the avoidance position
		if(   ((forwardSpeed > 0.0f) && !isPlayerShip && ms_showAvoidancePathOther)
		    || (isPlayerShip && ms_showAvoidancePathSelf))
		{
			Vector avoidancePosition_w;
			Vector targetPosition = (shipOrientation * elapsedTime * forwardSpeed) + shipPosition;

			if (SpaceAvoidanceManager::getAvoidancePosition(*this, shipOrientation * forwardSpeed, targetPosition, avoidancePosition_w))
			{
				static const int rings = 8;
				static const int segments = 8;
				
				camera->addDebugPrimitive(new SphereDebugPrimitive(UtilityDebugPrimitive::S_z, Transform::identity, avoidancePosition_w, this->getCollisionProperty()->getBoundingSphere_w().getRadius(), rings, segments));
				camera->addDebugPrimitive(new Line3dDebugPrimitive(UtilityDebugPrimitive::S_z, Transform::identity, getPosition_w(), avoidancePosition_w, PackedArgb::solidWhite));
			}
		}
	}
}
#endif

//----------------------------------------------------------------------

void ShipObject::updateTargetAcquisition(Object const * const target, float const elapsedTime, int const weaponIndex, bool const isCurrentWeapon, bool const isMissileReadyToFire)
{
	//TODO:  check this for cases in which the player is not the pilot of the ship

	if (weaponIndex < 0 && weaponIndex >= ShipChassisSlotType::cms_numWeaponIndices)
		return;

	MissileUpdate & missileData = m_missleUpdateData[weaponIndex];
	bool const hadTargetAcquired = missileData.m_acquired;
	bool const hadTargetAcquiring = missileData.m_acquiring;
	int missileTypeId = -1;

	missileData.m_hasmissiles = false;

	ClientObject const * const targetAsClient = target ? target->asClientObject() : NULL;
	TangibleObject const * const targetAsTangible = targetAsClient ? targetAsClient->asTangibleObject() : NULL;
	bool attackable = targetAsTangible ? targetAsTangible->isAttackable() : true;
	if (attackable && 
		targetAsClient && 
		targetAsClient->asShipObject() && 
		targetAsClient->asShipObject()->isCapitalShip() && 
		static_cast<ShipChassisSlotType::Type>(m_pilotLookAtTargetSlot.get()) == ShipChassisSlotType::SCST_invalid)
	{
		// Can only launch missiles against components of capital ships, not against the ship itself
		attackable = false;
	}

	if (targetAsTangible != NULL && target != NULL && attackable && hasAmmoForAShot(weaponIndex))
	{
		missileTypeId = getWeaponAmmoType(ShipChassisSlotType::SCST_weapon_first + weaponIndex);

		if (missileTypeId != -1)
		{
			//-- You have missiles in this group.
			missileData.m_hasmissiles = true;

			//-- if this is a remote ship, use its server transform for target lock testing
			RemoteShipController const * const myRemoteShipController = dynamic_cast<RemoteShipController const *>(getController());
			Transform const & myTransform_o2w = (myRemoteShipController != NULL) ? myRemoteShipController->getServerTransform_p() : getTransform_o2w();

			//-- check for target in cone(s)
			RemoteShipController const * const targetRemoteShipController = dynamic_cast<RemoteShipController const *>(target->getController());
			Vector const & targetPos_o = myTransform_o2w.rotateTranslate_p2l(getTargetInterceptPosition_w(elapsedTime, *targetAsTangible));
			bool const useRemoteController = static_cast<ShipChassisSlotType::Type>(m_pilotLookAtTargetSlot.get()) == ShipChassisSlotType::SCST_invalid;
			Vector const & targetServerPos_o = (useRemoteController && (targetRemoteShipController != NULL)) ? myTransform_o2w.rotateTranslate_p2l(targetRemoteShipController->getServerTransform_p().getPosition_p()) : targetPos_o;

			float const distanceToTarget = targetPos_o.approximateMagnitude();

			float minAngleToTargets = PI * 2.0f;

			if (distanceToTarget > 0.0f)
				minAngleToTargets = std::min(minAngleToTargets, acos(targetPos_o.dot(Vector::unitZ) / distanceToTarget));

			if (targetServerPos_o != targetPos_o)
			{
				float const distanceToServerTarget = targetServerPos_o.approximateMagnitude();
				if (distanceToServerTarget > 0.0f)
					minAngleToTargets = std::min(minAngleToTargets, acos(targetServerPos_o.dot(Vector::unitZ) / distanceToServerTarget));
			}

			float const defaultConeRadiusRadians = convertDegreesToRadians(MissileManager::getTargetAcquisitionAngle(missileTypeId));
			missileData.m_acquiring = (minAngleToTargets < defaultConeRadiusRadians) && (distanceToTarget <= getWeaponRange(weaponIndex));
			if (missileData.m_acquiring)
				missileData.m_acquired = missileData.m_acquired || (missileData.m_missileTargetAcquisitionTimer.updateNoReset(elapsedTime) && isMissileReadyToFire);
			else
				missileData.m_acquired = false;
		}
	}

	if (missileData.m_hasmissiles)
	{
		m_hasMissiles = true;
	}
	else
	{
		missileData.m_acquired = false;
		missileData.m_acquiring = false;
	}

	if (NULL != target)
	{
		ClientObject * const targetClient = safe_cast<ClientObject *>(getPilotLookAtTarget().getObject());
		ShipObject * const targetShip = targetClient ? targetClient->asShipObject() : NULL;
		if (NULL != targetShip)
		{
			int const targetGot = targetShip->getGameObjectType();
						
			//-- some ship types don't get missile acqusition indicators
			if (targetGot != SharedObjectTemplate::GOT_ship_station && 
				targetGot != SharedObjectTemplate::GOT_ship_mining_asteroid_static &&
				targetGot != SharedObjectTemplate::GOT_ship_mining_asteroid_dynamic)
			{
				//-- Update on current weapon.
				bool playTargetAcquiredSound = false;

				if (isCurrentWeapon)
				{
					if(missileData.m_acquiring)
						m_hasTargetAcquiring = true;
					
					if (missileData.m_acquired)
						m_hasTargetAcquired = true;

					//-- Kick off the acquiring effect for the current player for the current weapon.
					if (target != this)
					{
						ShipObject const * const playerShip = Game::getPlayerPilotedShip();
						if (playerShip == this)
						{
							if ((missileData.m_acquiring && !hadTargetAcquiring) || (!missileData.m_acquiring && hadTargetAcquiring))
								targetShip->getShipObjectEffects().setTargetAcquiringActive(missileData.m_acquiring, missileData.m_missileTargetAcquisitionTimer.getExpireTime());
							
							if ((missileData.m_acquired && !hadTargetAcquired) || (!missileData.m_acquired && hadTargetAcquired))
								targetShip->getShipObjectEffects().setTargetAcquiredActive(missileData.m_acquired);
						}
						else if(playerShip == targetShip)
						{
							playTargetAcquiredSound = missileData.m_acquired;
						}
					}
				}

				//-- see if target acquisition state has changed
				if ((missileData.m_acquired && !hadTargetAcquired) || (!missileData.m_acquired && hadTargetAcquired))
				{
					if (missileTypeId != -1)
					{
						missileData.m_missileTargetAcquisitionTimer.setExpireTime(MissileManager::getTargetAcquisitionSeconds(missileTypeId));
						missileData.m_missileTargetAcquisitionTimer.reset();
						
						Transceivers::targetAcquisition.emitMessage(*this);

						if (playTargetAcquiredSound)
							IGNORE_RETURN(Audio::playSound(s_enemyMissileAcquired, NULL));
					}
				}
			}
		}
	}
}

//----------------------------------------------------------------------

ShipObjectEffects const & ShipObject::getShipObjectEffects() const
{
	return *NON_NULL(m_shipObjectEffects);
}

//----------------------------------------------------------------------

ShipObjectEffects & ShipObject::getShipObjectEffects()
{
	return *NON_NULL(m_shipObjectEffects);
}

//----------------------------------------------------------------------

ShipObjectAttachments const & ShipObject::getShipObjectAttachments() const
{
	return *NON_NULL(m_shipObjectAttachments);
}

//----------------------------------------------------------------------

ShipObjectAttachments & ShipObject::getShipObjectAttachments()
{
	return *NON_NULL(m_shipObjectAttachments);
}

//----------------------------------------------------------------------

bool ShipObject::isWeaponPlayerControlled(int weaponIndex) const
{
	std::map<int, ShipWeaponStatus>::const_iterator const i = m_shipWeaponStatusMap.find(weaponIndex);
	return i != m_shipWeaponStatusMap.end() && (*i).second.m_playerControlled;
}

//----------------------------------------------------------------------

std::string const & ShipObject::getWingName() const
{
	return m_wingName.get();
}

//----------------------------------------------------------------------

std::string const & ShipObject::getTypeName() const
{
	return m_typeName.get();
}

//----------------------------------------------------------------------

std::string const & ShipObject::getDifficulty() const
{
	return m_difficulty.get();
}

//----------------------------------------------------------------------

std::string const & ShipObject::getFaction() const
{
	return m_faction.get();
}

//----------------------------------------------------------------------

bool ShipObject::isMissile(int const weaponIndex) const
{
	bool ismissile = false;

	if(weaponIndex >= 0 && weaponIndex < ShipChassisSlotType::cms_numWeaponIndices)
	{
		int const chassisSlot = static_cast<int>(ShipChassisSlotType::SCST_weapon_first) + weaponIndex;
		uint32 const componentCrc = getComponentCrc(chassisSlot);
		if (componentCrc != 0)
		{
			ismissile = ShipComponentWeaponManager::isMissile(componentCrc);
		}
	}

	return ismissile;
}

//----------------------------------------------------------------------

bool ShipObject::isBeamWeapon(int weaponIndex)
{
	if(weaponIndex >= 0 && weaponIndex < ShipChassisSlotType::cms_numWeaponIndices)
	{
		int const chassisSlot = static_cast<int>(ShipChassisSlotType::SCST_weapon_first) + weaponIndex;
		uint32 const componentCrc = getComponentCrc(chassisSlot);
		if (componentCrc != 0)
			return ShipComponentWeaponManager::hasFlags(componentCrc, ShipComponentWeaponManager::F_beam);

	}

	return false;
}

//----------------------------------------------------------------------

int ShipObject::getCurrentMissileTypeId() const
{
	int missileId = -1;

	if (Game::getPlayerPilotedShip() == this)
	{
		missileId = ShipWeaponGroupManager::getMissileTypeIdFromCurrentGroup();
	}
	else if(Game::getPlayerContainingShip() == this)
	{
		missileId = -1; // RLS TODO: Should shoot missiles in a turret.
	}
	else
	{
		missileId = findFirstUsableMissileWeaponId();
	}

	return missileId;
}

//----------------------------------------------------------------------

int ShipObject::findFirstUsableMissileWeaponId() const
{
	int missileTypeId = -1;

	for (int weaponIndex = 0; weaponIndex < ShipChassisSlotType::cms_numWeaponIndices; ++weaponIndex)
	{
		if (isMissile(weaponIndex))
		{
			missileTypeId = getWeaponAmmoType(ShipChassisSlotType::SCST_weapon_first + weaponIndex);
			break;
		}
	}

	return missileTypeId;
}

//----------------------------------------------------------------------

int ShipObject::findFirstUsableMissileWeaponIndex() const
{
	int weaponIndex;
	for (weaponIndex = 0; weaponIndex < ShipChassisSlotType::cms_numWeaponIndices; ++weaponIndex)
		if (isMissile(weaponIndex))
			return weaponIndex;
	
	return weaponIndex;
}

//----------------------------------------------------------------------

void ShipObject::resetMissileAcquisitionTimers()
{
	for (int weaponIndex = 0; weaponIndex < ShipChassisSlotType::cms_numWeaponIndices; ++weaponIndex)
	{
		if (isMissile(weaponIndex))
		{
			int const missileTypeId = getWeaponAmmoType(ShipChassisSlotType::SCST_weapon_first + weaponIndex);
			float missileAcquisitionTimer = missileTypeId!=-1 ? MissileManager::getTargetAcquisitionSeconds(missileTypeId) : FLT_MAX;
			m_missleUpdateData[weaponIndex].m_missileTargetAcquisitionTimer.setExpireTime(missileAcquisitionTimer);
			m_missleUpdateData[weaponIndex].m_missileTargetAcquisitionTimer.reset();
		}
	}
}

//----------------------------------------------------------------------

void ShipObject::onCreateMissileMessage(NetworkId const & missileTarget)
{
	ShipObject const * const playerShip = Game::getPlayerContainingShip();
	
	if (playerShip && playerShip->getNetworkId() == getNetworkId() && getNetworkId() == missileTarget)
	{
		IGNORE_RETURN(Audio::playSound(s_enemyMissileIncoming, NULL));
	}
} //lint !e1762 //Making this const would break the Transceiver signature.

//----------------------------------------------------------------------

void ShipObject::conditionModified(int oldCondition, int newCondition)
{
	TangibleObject::conditionModified(oldCondition, newCondition);
	
	if (newCondition != oldCondition)
	{
		if (Game::getPlayer() == m_pilot)
		{
			GroundScene * const groundScene = safe_cast<GroundScene *>(Game::getScene());
			
			if (groundScene != NULL)
			{
				if (   (oldCondition & static_cast<int>(TangibleObject::C_docking))
					&& !hasCondition(static_cast<int>(TangibleObject::C_docking)))
				{
					// Change back to the previous non-docking camera
					
					int view = static_cast<int>(GroundScene::CI_freeChase);
					
					if (m_pilot != NULL)
					{
						int const shipStation = m_pilot->getShipStation();
						
						if (   (shipStation == static_cast<int>(ShipStation::ShipStation_Pilot))
							|| (shipStation == static_cast<int>(ShipStation::ShipStation_Operations)))
						{
							view = static_cast<int>(GroundScene::CI_cockpit);
						}
						else if (   (shipStation >= static_cast<int>(ShipStation::ShipStation_Gunner_First))
							&& (shipStation <= static_cast<int>(ShipStation::ShipStation_Gunner_Last)))
						{
							view = static_cast<int>(GroundScene::CI_shipTurret);
						}
					}
					
					groundScene->setView(view);
				}
				else if (hasCondition(static_cast<int>(TangibleObject::C_docking)))
				{
					// Change to the docking camera
					
					groundScene->setView(static_cast<int>(GroundScene::CI_flyBy));
					
					FlyByCamera * const flyByCamera = dynamic_cast<FlyByCamera *>(groundScene->getCurrentCamera());
					
					if (flyByCamera != NULL)
					{
						flyByCamera->setTarget(this);
					}
					else
					{
						DEBUG_FATAL(true, ("The flyby camera should exist."));
					}
				}
			}
		}
	}
}

//----------------------------------------------------------------------

ShipObject::ShipLaunchable ShipObject::getShipLaunchable() const
{
	if(!isSlotInstalled(ShipChassisSlotType::SCST_reactor))
	{
		return SL_noReactor;
	}
	else if(isComponentDisabled(ShipChassisSlotType::SCST_reactor))
	{
		return SL_reactorDisabled;
	}
	else if(isComponentDemolished(ShipChassisSlotType::SCST_reactor))
	{
		return SL_reactorDemolished;
	}
	else if(!isSlotInstalled(ShipChassisSlotType::SCST_engine))
	{
		return SL_noEngine;
	}
	else if(isComponentDisabled(ShipChassisSlotType::SCST_engine))
	{
		return SL_engineDisabled;
	}
	else if(isComponentDemolished(ShipChassisSlotType::SCST_engine))
	{
		return SL_engineDemolished;
	}
	else if(getOverallHealth() < 1.0f)
	{
		return SL_damaged;
	}
	else
	{
		return SL_pristine;
	}
}


//----------------------------------------------------------------------


void ShipObject::updateMissiles(float const elapsedTime)
{
	// Do not update for ships that the player isn't in
	if (Game::getPlayerContainingShip() != this)
		return;
	
	// Get current missile index and group.
	CachedNetworkId const & targetId = getPilotLookAtTarget();
	Object const * const targetObject = targetId.getObject();
	
	int const currentMissileIndex = ShipWeaponGroupManager::getWeaponIndexFromCurrentGroup(ShipWeaponGroupManager::GT_Missile);
	int const currentMissileGroup = ShipWeaponGroupManager::getCurrentGroup(ShipWeaponGroupManager::GT_Missile);
	int const currentMissileSlot = getPilotLookAtTargetSlot();
	bool currentMissileGroupChanged = m_missileGroup != currentMissileGroup || m_missileSlot != currentMissileSlot;
	
	if (currentMissileGroupChanged)
	{
		// Reset target timers.
		resetMissileAcquisitionTimers();

		// Assign the new group indices.
		m_missileGroup = currentMissileGroup;
		m_missileSlot = currentMissileSlot;
	}
	
	//-- Reset flags.
	m_hasMissiles = false;
	m_hasTargetAcquiring = false;
	m_hasTargetAcquired = false;

	{
		//-- update refire and missile timers.
		for (int weaponIndex = 0; weaponIndex < ShipChassisSlotType::cms_numWeaponIndices; ++weaponIndex)
		{
			ShipChassisSlotType::Type const weaponChassisSlotType = static_cast<ShipChassisSlotType::Type>(weaponIndex + static_cast<int>(ShipChassisSlotType::SCST_weapon_first));
			//-- we need to bias the refire rate a little bit to help ensure that the server expires first.
			Timer &refireTimer = m_shipWeaponStatusMap[weaponIndex].m_refireTimer;
			refireTimer.setExpireTime(getWeaponActualRefireRate(weaponChassisSlotType) + 0.1f);
			bool const isMissileReadyToFire = refireTimer.updateNoReset(elapsedTime);
			
			updateTargetAcquisition(targetObject, elapsedTime, weaponIndex, currentMissileIndex == weaponIndex, isMissileReadyToFire);
		}
	}
}

// ----------------------------------------------------------------------

bool ShipObject::isPlayerControlled() const
{
	bool playerControlled = false;

	SharedShipObjectTemplate const * const sharedShipObjectTemplate = dynamic_cast<SharedShipObjectTemplate const *>(getObjectTemplate());
	if (sharedShipObjectTemplate)
	{
		playerControlled = sharedShipObjectTemplate->getPlayerControlled();
	}
	else
	{
		DEBUG_WARNING(true, ("ShipObject::isPlayerControlled(): ShipObject %s sharedTemplate is not a SharedShipObjectTemplate [template=%s]", getNetworkId().getValueString().c_str(), getObjectTemplateName()));
	}


	return playerControlled;
}

// ----------------------------------------------------------------------

ShipObject::ShipVector const & ShipObject::getShipsTargetingPlayer()
{
	return ms_shipsTargetingPlayer;
}

//----------------------------------------------------------------------

bool ShipObject::isShipKillableByDestructionSequence() const
{
	return m_shipKillableByDestructionSequence;
}

//----------------------------------------------------------------------

void ShipObject::setShipKillableByDestructionSequence(bool b)
{
	m_shipKillableByDestructionSequence = b;
}

// ----------------------------------------------------------------------

int ShipObject::getTurretCount() const
{
	int result = 0;

	for (int weaponIndex = 0; weaponIndex < ShipChassisSlotType::cms_numWeaponIndices; ++weaponIndex)
	{
		if (isTurret(weaponIndex))
		{
			++result;
		}
	}

	return result;
}

// ----------------------------------------------------------------------

int ShipObject::getCountermeasureCount() const
{
	int result = 0;

	for (int weaponIndex = 0; weaponIndex < ShipChassisSlotType::cms_numWeaponIndices; ++weaponIndex)
	{
		if (isCountermeasure(weaponIndex))
		{
			++result;
		}
	}

	return result;
}

// ----------------------------------------------------------------------

float ShipObject::getMaxProjectileWeaponRange() const
{
	float result = 0.0f;

	for (int weaponIndex = 0; weaponIndex < ShipChassisSlotType::cms_numWeaponIndices; ++weaponIndex)
	{
		result = std::max(result, getWeaponRange(weaponIndex));
	}

	return result;
}

//----------------------------------------------------------------------

void ShipObject::enableTargetAppearanceFromObjectId(TangibleObject const * const instigator, CachedNetworkId const & objectid, bool enabled, int targetSlot)
{
	UNREF(targetSlot);
	if (instigator != 0 && Game::getPlayerContainingShip() == instigator) 
	{
		//-- Dis/Enable the target blip on this object.
		Object * const object = objectid.getObject();
		ClientObject * const clientObject = object ? object->asClientObject() : NULL;
		ShipObject * const shipObject = clientObject ? clientObject->asShipObject() : NULL;
		
		if (shipObject)
		{
			shipObject->getShipObjectEffects().setTargetAppearanceActive(enabled); 
			shipObject->getShipObjectEffects().setTargetAcquiredActive(false);
			shipObject->getShipObjectEffects().setTargetAcquiringActive(false, 0.0f);
		}
	}
}

//----------------------------------------------------------------------

float ShipObject::getChassisSpeedMaximumModifier() const
{
	return m_chassisSpeedMaximumModifier.get();
}

//----------------------------------------------------------------------

void ShipObject::setChassisSpeedMaximumModifier(float f)
{
	m_chassisSpeedMaximumModifier = f;
}

//----------------------------------------------------------------------

bool ShipObject::isTimerShieldHitClientExpired() const
{
	return m_timerShieldHitClient.isExpired();
}

//----------------------------------------------------------------------

bool ShipObject::isTimerShieldHitServerExpired() const
{
	return m_timerShieldHitServer.isExpired();
}

//----------------------------------------------------------------------

bool ShipObject::isTimerShieldEventServerExpired() const
{
	return m_timerShieldEventServer.isExpired();
}

//----------------------------------------------------------------------

void ShipObject::resetTimerShieldHitClient()
{
	m_timerShieldHitClient.reset();
}

//----------------------------------------------------------------------

void ShipObject::resetTimerShieldHitServer()
{
	m_timerShieldHitServer.reset();
}

//----------------------------------------------------------------------

void ShipObject::resetTimerShieldEventServer()
{
	m_timerShieldEventServer.reset();
}

//----------------------------------------------------------------------

bool ShipObject::hasAmmoForAShot(int const weaponIndex) const
{
	if (weaponIndex < 0 || weaponIndex >= ShipChassisSlotType::cms_numWeaponIndices)
		return false;
	
	ShipChassisSlotType::Type const weaponChassisSlotType = static_cast<ShipChassisSlotType::Type>(weaponIndex + static_cast<int>(ShipChassisSlotType::SCST_weapon_first));
	
	uint32 const componentCrc = getComponentCrc(weaponChassisSlotType);
	
	if (componentCrc)
	{
		if (ShipComponentWeaponManager::isAmmoConsuming(componentCrc))
		{
			int const ammoAvailable = getWeaponAmmoCurrent (weaponChassisSlotType);
			return ammoAvailable > 0;
		}
		return true;
	}
	
	return false;
}

//----------------------------------------------------------------------

bool ShipObject::wingsOpened() const
{
	return hasCondition(TangibleObject::C_wingsOpened);
}

//----------------------------------------------------------------------

void ShipObject::stopFiringAllWeapons()
{
	for (ShipWeaponStatusMap::iterator it = m_shipWeaponStatusMap.begin(); it != m_shipWeaponStatusMap.end(); ++it)
	{
		int const weaponIndex = (*it).first;
		stopFiringWeapon(weaponIndex, true);
	}
}

//----------------------------------------------------------------------

void ShipObject::setCockpitFrame(Object * const cockpitFrame)
{
	// we only want to have the cockpit set up for the player's ship

	if (Game::getPlayerContainingShip() != this)
	{
		return;
	}

	Object * const previousCockpitFrame = s_cockpitFrame;

	s_cockpitFrame = cockpitFrame;

	if (s_cockpitFrame.getPointer() != previousCockpitFrame)
	{
		if (previousCockpitFrame != 0)
		{
			previousCockpitFrame->kill();
		}

		if (s_cockpitFrame != 0)
		{
			RenderWorld::addObjectNotifications(*s_cockpitFrame);
			addChildObject_o(s_cockpitFrame);
		}
	}
}

//----------------------------------------------------------------------

Object const * ShipObject::getCockpitFrame() const
{
	return s_cockpitFrame;
}

// ======================================================================
