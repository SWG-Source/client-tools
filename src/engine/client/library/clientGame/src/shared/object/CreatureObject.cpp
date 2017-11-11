// ======================================================================
//
// CreatureObject.cpp
// Portions Copyright 1999, Bootprint Entertainment, Inc.
// Portions Copyright 2000-2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/CreatureObject.h"

#include "clientAnimation/PlaybackScript.h"
#include "clientAnimation/PlaybackScriptManager.h"
#include "clientAudio/Audio.h"
#include "clientGame/ClientBuffManager.h"
#include "clientGame/ClientCommandChecks.h"
#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientDataFile.h"
#include "clientGame/ClientEffectManager.h"
#include "clientGame/ClientEventManager.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureInfo.h"
#include "clientGame/Game.h"
#include "clientGame/GameAnimationMessageCallback.h"
#include "clientGame/GamePlaybackScript.h"
#include "clientGame/GroundScene.h"
#include "clientGame/GroundCombatActionManager.h"
#include "clientGame/GroupManager.h"
#include "clientGame/GroupObject.h"
#include "clientGame/LightsaberAppearance.h"
#include "clientGame/MoodManagerClient.h"
#include "clientGame/PlayerMusicManager.h"
#include "clientGame/PlayerObject.h"
#include "clientGame/RemoteCreatureController.h"
#include "clientGame/RiderSpineTransformModifier.h"
#include "clientGame/SaddleManager.h"
#include "clientGame/ShipObject.h"
#include "clientGame/ShipObjectEffects.h"
#include "clientGame/ShipStation.h"
#include "clientGame/Species.h"
#include "clientGame/TemplateCommandMappingManager.h"
#include "clientGame/WeaponObject.h"
#include "clientGraphics/DebugPrimitive.h"
#include "clientGraphics/RenderWorld.h"
#include "clientObject/GameCamera.h"
#include "clientObject/HardpointObject.h"
#include "clientObject/InteriorEnvironmentBlock.h"
#include "clientObject/InteriorEnvironmentBlockManager.h"
#include "clientObject/ShadowBlobObject.h"
#include "clientSkeletalAnimation/AnimationEnvironment.h"
#include "clientSkeletalAnimation/AnimationEnvironmentNames.h"
#include "clientSkeletalAnimation/AnimationHeldItemMapper.h"
#include "clientSkeletalAnimation/LookAtTransformModifier.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/SkeletalAppearanceTemplate.h"
#include "clientSkeletalAnimation/TargetPitchTransformModifier.h"
#include "clientSkeletalAnimation/TransformAnimationResolver.h"
#include "clientSkeletalAnimation/ZeroTranslationTransformModifier.h"
#include "clientUserInterface/CuiObjectTextManager.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "clientUserInterface/CuiUtils.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/CollisionWorld.h"
#include "sharedCollision/Floor.h"
#include "sharedCollision/FloorLocator.h"
#include "sharedCollision/Footprint.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/DebugInfoManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/Production.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedGame/AppearanceManager.h"
#include "sharedGame/Buff.h"
#include "sharedGame/Command.h"
#include "sharedGame/CommandTable.h"
#include "sharedGame/ConfigSharedGame.h"
#include "sharedGame/GameObjectTypes.h"
#include "sharedGame/PvpData.h"
#include "sharedGame/SharedCreatureObjectTemplate.h"
#include "sharedGame/SharedStringIds.h"
#include "sharedGame/SharedTerrainSurfaceObjectTemplate.h"
#include "sharedGame/SharedWeaponObjectTemplate.h"
#include "sharedGame/ShipSlotIdManager.h"
#include "sharedGame/SlopeEffectProperty.h"
#include "sharedGame/SlowDownProperty.h"
#include "sharedGame/VehicleHoverDynamics.h"
#include "sharedGame/WearableAppearanceMap.h"
#include "sharedMath/VectorArgb.h"
#include "sharedMathArchive/VectorArchive.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/ContainedByProperty.h"
#include "sharedObject/Container.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/CustomizationDataProperty.h"
#include "sharedObject/MovementTable.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/PortalProperty.h"
#include "sharedObject/RotationDynamics.h"
#include "sharedObject/SlotId.h"
#include "sharedObject/SlotIdManager.h"
#include "sharedObject/SlottedContainer.h"
#include "sharedObject/SlottedContainmentProperty.h"
#include "sharedSkillSystem/SkillManager.h"
#include "sharedSkillSystem/SkillObject.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"
#include "sharedUtility/DataTable.h"
#include "sharedTerrain/TerrainObject.h"
#include "swgSharedUtility/Postures.h"
#include "swgSharedUtility/States.h"

//lint -esym(641,GameObjectType) // convert enum to int
//lint -esym(641,CameraIds) // convert enum to int
//lint -esym(641,Conditions) // convert enum to int
//lint -esym(641,ShipStation) // convert enum to int

#ifndef M_PI
#define M_PI          3.14159265358979323846
#endif

//----------------------------------------------------------------------

namespace CreatureObjectNamespace
{
	namespace Slots
	{
		const char * const inventory = "inventory";
		const char * const datapad   = "datapad";
		const char * const hair      = "hair";
		const char * const player    = "ghost";
		const char * const bank      = "bank";
		const char * const appearance = "appearance_inventory";
	}

	namespace AppearanceTag
	{
		const char * const tagDefault = ":default";
		const char * const tagBlock   = ":block";
		const char * const tagHide    = ":hide";
	}

	namespace Transceivers
	{
		//-- non const payload
		MessageDispatch::Transceiver<CreatureObject::Messages::DamageTaken::Payload &,                   CreatureObject::Messages::DamageTaken>             damageTaken;
		MessageDispatch::Transceiver<const CreatureObject::Messages::IncapacityChanged::Payload &,       CreatureObject::Messages::IncapacityChanged>       incapacityChanged;
		MessageDispatch::Transceiver<const CreatureObject::Messages::LookAtTargetChanged::Payload &,     CreatureObject::Messages::LookAtTargetChanged>     lookAtTargetChanged;
		MessageDispatch::Transceiver<const CreatureObject::Messages::IntendedTargetChanged::Payload &,   CreatureObject::Messages::IntendedTargetChanged>   intendedTargetChanged;
		MessageDispatch::Transceiver<const CreatureObject::Messages::CurrentWeaponChanged::Payload &,    CreatureObject::Messages::CurrentWeaponChanged>    currentWeaponChanged;
		MessageDispatch::Transceiver<const CreatureObject::Messages::StatesChanged::Payload &,           CreatureObject::Messages::StatesChanged>           statesChanged;
		MessageDispatch::Transceiver<const CreatureObject::Messages::ServerPostureChanged::Payload &,    CreatureObject::Messages::ServerPostureChanged>    serverPostureChanged;
		MessageDispatch::Transceiver<const CreatureObject::Messages::VisualPostureChanged::Payload &,    CreatureObject::Messages::VisualPostureChanged>    visualPostureChanged;
		MessageDispatch::Transceiver<const CreatureObject::Messages::GroupChanged::Payload &,            CreatureObject::Messages::GroupChanged>            groupChanged;
		MessageDispatch::Transceiver<const CreatureObject::Messages::GroupInviterChanged::Payload &,     CreatureObject::Messages::GroupInviterChanged>     groupInviterChanged;
		MessageDispatch::Transceiver<const CreatureObject::Messages::SkillModsChanged::Payload &,        CreatureObject::Messages::SkillModsChanged>        skillModsChanged;
		MessageDispatch::Transceiver<const CreatureObject::Messages::SkillsChanged::Payload &,           CreatureObject::Messages::SkillsChanged>           skillsChanged;
		MessageDispatch::Transceiver<const CreatureObject::Messages::PlayerSetup::Payload &,             CreatureObject::Messages::PlayerSetup>             playerSetup;
		MessageDispatch::Transceiver<const CreatureObject::Messages::MaxAttributesChanged::Payload &,    CreatureObject::Messages::MaxAttributesChanged>    maxAttributesChanged;
		MessageDispatch::Transceiver<CreatureObject::Messages::GroupMissionCriticalObjectsChanged::Payload const &, CreatureObject::Messages::GroupMissionCriticalObjectsChanged> groupMissionCriticalObjectsChanged;
		MessageDispatch::Transceiver<CreatureObject::Messages::BuffsChanged::Payload const &,            CreatureObject::Messages::BuffsChanged>            buffsChanged;
		MessageDispatch::Transceiver<const CreatureObject::Messages::CommandsChanged::Payload &, CreatureObject::Messages::CommandsChanged> commandsChanged;
		MessageDispatch::Transceiver<const CreatureObject::Messages::CommandAdded::Payload &, CreatureObject::Messages::CommandAdded> commandAdded;
		MessageDispatch::Transceiver<const CreatureObject::Messages::CommandRemoved::Payload &, CreatureObject::Messages::CommandRemoved> commandRemoved;
		MessageDispatch::Transceiver<const CreatureObject::Messages::AdminChanged::Payload &, CreatureObject::Messages::AdminChanged> adminChanged;
		MessageDispatch::Transceiver<const CreatureObject::Messages::TotalLevelXpChanged::Payload &, CreatureObject::Messages::TotalLevelXpChanged> totalLevelXpChanged;
		MessageDispatch::Transceiver<const CreatureObject::Messages::LevelChanged::Payload &, CreatureObject::Messages::LevelChanged> levelChanged;
	}

	const ConstCharCrcLowerString  cs_moodAnimationVariableName("mood");
	const ConstCharCrcLowerString  cs_humanoidSkeletonTemplateName("appearance/skeleton/all_b.skt");
	const ConstCharCrcLowerString  cs_ithorianSkeletonTemplateName("appearance/skeleton/ithorian.skt");
	const ConstCharCrcString       cs_targetPitchJointName("spine3");
	const ConstCharCrcString       cs_lookAtMasterJointName("neck");
	const ConstCharCrcString       cs_lookAtSlaveJointName("head");

	const ConstCharCrcString       cs_riderHardpointName("player");
	const ConstCharCrcString       cs_droidHardpointName("astromech");

	const ConstCharCrcString       cs_riderSlotName("rider");

	const ConstCharCrcString       cs_rootJointName("root");
	const ConstCharCrcString       cs_spineJointName("spine1");

	const ConstCharCrcString	   cs_humanFaceBoneJointName("face_root__hum_m_face");
	const ConstCharCrcString       cs_ithFaceBoneJointName("face_root__ith_face");


	const float           cs_minimumScaleFactor            = 0.001f;
	const float           cs_revertToServerPostureInterval = 5.0f;

	const Tag             TAG_DDFS = TAG(D,D,F,S);
	const Tag             TAG_AANC = TAG(A,A,N,C);


	namespace AnimationMoods
	{
		const std::string neutral = "neutral";
	}

	SlotId          s_holdRightSlotId(SlotId::invalid);
	SlotId          s_holdLeftSlotId(SlotId::invalid);
	SlotId          s_riderSlotId(SlotId::invalid);

	bool            ms_disableCreaturePostureRevert = false;
	bool            ms_logAnimationMoodChanges;

#ifdef _DEBUG
	bool            s_logServerPostureChanges;
	bool            s_logVisualPostureChanges;
	bool            s_logMountStatusChanges;
	bool            s_logMountedRiderScale;
	bool ms_reportGroupMissionCriticalObjects;
#endif

	CachedNetworkId ms_lastLookAtTargetRequest;
	CachedNetworkId ms_lastIntendedTargetRequest;

	int ms_numberOfInstances;

	bool ms_showHealingObject;
	bool ms_doMountSanityChecking;

	int getShipStationForSlot(SlotId const &slotId);
	bool shouldCreatureBeVisible(CreatureObject const &creature);

	//---------------------------------------------------------------
	// For converting an integer rank value to a string token
	typedef stdmap<uint8, std::string>::fwd RankStringMap;

	RankStringMap   ms_rankStringMap;
	const std::string cms_tableNameString  = "datatables/faction/rank.iff";
	const std::string cms_columnNameString = "NAME";


	//-----------------------------------------------------------------
	// takeCover surface effects.
	//
	// Artists can create per surface cover effects kicked off on the player.
	// To do this, effects must be added to the client data file.
	// for example, in ../dsrc/sku.0/sys.client/compiled/game/clientdata/player/client_shared_player_human_f.mif
	// You can add entries of the following format (and rebuild the mif):
	// cover + _ + <method> + _ + <surface>
	//
	// methods:
	//	take  = taking cover. (disappearing)
	//	leave = leaving cover. (reappearing)
	//
	// surface:
	//	other = generic cover effect
	//	water = water
	//	metal = metal
	//	stone = stone
	//	wood  = wood
	//
	// Example mif file entry:
	// Event("cover_take_other", "clienteffect/combat_melee_swing_unarmed.cef")
	// Event("cover_leave_other", "clienteffect/combat_melee_swing_unarmed.cef")

	namespace CoverEffects
	{
		char const * const water = "water";
		char const * const other = "other";
		char const * const metal = "metal";
		char const * const stone = "stone";
		char const * const wood  = "wood";
		char const * const takeCover  = "cover_take_";
		char const * const leaveCover = "cover_leave_";
	}

	char const * getSurfaceType(CreatureObject const & object);

	std::string const & ms_debugInfoSectionName = "CreatureObject";
	char const * const s_adminCommand = "admin";

	//-----------------------------------------------------------------

	// Changing appearance for droids in space
	typedef std::map<std::string, std::string> DroidAppearanceMap;
	typedef std::map<std::string, DroidAppearanceMap> ShipToDroidAppearanceMapMap;
	static ShipToDroidAppearanceMapMap s_droidSpaceAppearanceMap;
	static bool s_spaceAppearanceMapInitialized = false;
	const char* const s_spaceDroidAppearanceMapFilename = "datatables/space/ship_droid_appearance_override.iff";

	// loads the space appearance override map from the datatable
	void loadSpaceAppearanceMap()
	{
		Iff iff;
		if(!iff.open (s_spaceDroidAppearanceMapFilename,true))
		{
			WARNING (true, ("Data file %s not available.",s_spaceDroidAppearanceMapFilename));
			return;
		}

		DataTable dt;
		dt.load (iff);
		iff.close();

		std::string const & s_colnameShip = "ship";
		std::string const & s_colnameGroundAppearance = "ground_appearance";
		std::string const & s_colnameSpaceAppearance = "space_appearance";

		const int numRows = dt.getNumRows ();

		for (int row = 0; row < numRows; ++row)
		{
			std::string shipName = dt.getStringValue (s_colnameShip, row);
			std::string droidAppearance = dt.getStringValue (s_colnameGroundAppearance, row);
			std::string spaceAppearance = dt.getStringValue (s_colnameSpaceAppearance, row);

			if(!shipName.empty () && !droidAppearance.empty () && ! spaceAppearance.empty ())
			{	
				ShipToDroidAppearanceMapMap::iterator shipItr = s_droidSpaceAppearanceMap.find(shipName);
				if(shipItr == s_droidSpaceAppearanceMap.end ())
				{
					s_droidSpaceAppearanceMap[shipName] = DroidAppearanceMap();
					shipItr = s_droidSpaceAppearanceMap.find (shipName);
				}

				DroidAppearanceMap& droidMap = (*shipItr).second;

				droidMap[droidAppearance] = spaceAppearance;
			}
		}

		s_spaceAppearanceMapInitialized = true;
	}

	// Returns the appearance override from the droid appearance override datatable
	// returns null if the ship/ground appearance pair does not appear in the table or 
	// if the appearance could not be created
	Appearance* getSpaceAppearance(const std::string& shipname, const std::string& groundAppearanceName)
	{
		DEBUG_WARNING(!s_spaceAppearanceMapInitialized, ("CreatureObject::getSpaceAppearance: conversion map not initialized"));

		Appearance* spaceAppearanceOverride = NULL;

		ShipToDroidAppearanceMapMap::iterator shipMapItr = s_droidSpaceAppearanceMap.find(shipname);
		if(shipMapItr != s_droidSpaceAppearanceMap.end ())
		{
			DroidAppearanceMap::iterator droidItr = (*shipMapItr).second.find (groundAppearanceName);
			if(droidItr != (*shipMapItr).second.end ())
			{
				spaceAppearanceOverride = AppearanceTemplateList::createAppearance ((*droidItr).second.c_str ());
			}	
		}
		
		return spaceAppearanceOverride;
	}

	bool ms_logAppearanceTabMessages = false;

	bool isWearableWithAppearanceUpdate(CreatureObject & wearer, Object & obj);
	void setCustomizationData(Object & wearer, Object & wornObject);
	void removeCustomizationData(Object & wearer, Object & wornObject);
}

using namespace CreatureObjectNamespace;

//-----------------------------------------------------------------

void CreatureObject::install()
{
	InstallTimer const installTimer("CreatureObject::install");

	// Lookup slot ids.
	s_holdRightSlotId = SlotIdManager::findSlotId(ConstCharCrcLowerString("hold_r"));
	s_holdLeftSlotId  = SlotIdManager::findSlotId(ConstCharCrcLowerString("hp_hold_l"));
	s_riderSlotId     = SlotIdManager::findSlotId(cs_riderSlotName);

	DebugFlags::registerFlag (ms_disableCreaturePostureRevert, "ClientGame/Posture", "disableCreaturePostureRevert");
	DebugFlags::registerFlag (ms_logAnimationMoodChanges, "ClientGame", "logAnimationMoodChanges");
	DebugFlags::registerFlag (ms_showHealingObject, "ClientGame", "showHealingObject");
	DebugFlags::registerFlag (ms_doMountSanityChecking, "ClientGame", "doMountSanityChecking");

#ifdef _DEBUG
	DebugFlags::registerFlag (s_logMountedRiderScale,    "ClientGame/Mounts",  "logMountedRiderScale");
	DebugFlags::registerFlag (s_logMountStatusChanges,   "ClientGame/Mounts",  "logMountStatusChanges");
	DebugFlags::registerFlag (s_logServerPostureChanges, "ClientGame/Posture", "logServerPostureChanges");
	DebugFlags::registerFlag (s_logVisualPostureChanges, "ClientGame/Posture", "logVisualPostureChanges");
	DebugFlags::registerFlag(ms_reportGroupMissionCriticalObjects, "ClientGame/CreatureObject", "reportGroupMissionCriticalObjects");
	DebugFlags::registerFlag (ms_logAppearanceTabMessages, "ClientGame/AppearanceTab", "logAppearanceTabMessages");
#endif

	loadSpaceAppearanceMap();

	ExitChain::add(&remove, "CreatureObject::remove");
}

//-----------------------------------------------------------------

void CreatureObject::remove()
{
	DebugFlags::unregisterFlag (ms_disableCreaturePostureRevert);
	DebugFlags::unregisterFlag (ms_logAnimationMoodChanges);
	DebugFlags::unregisterFlag (ms_showHealingObject);
	DebugFlags::unregisterFlag (ms_doMountSanityChecking);

#ifdef _DEBUG
	DebugFlags::unregisterFlag (s_logMountedRiderScale);
	DebugFlags::unregisterFlag (s_logMountStatusChanges);
	DebugFlags::unregisterFlag (s_logServerPostureChanges);
	DebugFlags::unregisterFlag (s_logVisualPostureChanges);
	DebugFlags::unregisterFlag(ms_reportGroupMissionCriticalObjects);
	DebugFlags::unregisterFlag (ms_logAppearanceTabMessages);
#endif
}

void CreatureObject::setDisablePostureRevert (const bool disablePostureRevert)
{
	ms_disableCreaturePostureRevert = disablePostureRevert;
}

//-----------------------------------------------------------------

int CreatureObject::getNumberOfInstances ()
{
	return ms_numberOfInstances;
}

//-----------------------------------------------------------------
// CPP file inlines
//-----------------------------------------------------------------

inline float CreatureObject::_getUnmountedMaximumTurnRate(const float currentSpeed) const
{
	return ((currentSpeed <= _getUnmountedWalkSpeed()) ? m_turnRateMaxWalk : m_turnRateMaxRun) * getTurnPercent();
}

inline float CreatureObject::_getMaximumTurnRate(float currentSpeed, bool mounted) const
{
	return (mounted) ? _getMountedMaximumTurnRate(currentSpeed) : _getUnmountedMaximumTurnRate(currentSpeed);
}

inline float CreatureObject::_getMovementPercent(bool mounted) const
{
	return (mounted) ? _getMountedMovementPercent() : m_movementPercent.get();
}

inline float CreatureObject::_getMovementScale(bool mounted) const
{
	return (mounted) ? _getMountedMovementScale() : m_movementScale.get();
}

inline float CreatureObject::_adjustMovementSpeed(float speed, bool mounted) const
{
	return speed * _getMovementScale(mounted) * _getMovementPercent(mounted);
}

inline float CreatureObject::_getUnmountedRunSpeed() const
{
	return _adjustMovementSpeed(m_runSpeed.get(), false);
}

inline float CreatureObject::_getRunSpeed(bool mounted) const
{
	return (mounted) ? _getMountedRunSpeed() : _getUnmountedRunSpeed();
}

inline float CreatureObject::_getUnmountedWalkSpeed() const
{
	return _adjustMovementSpeed(m_walkSpeed.get(), false);
}

inline float CreatureObject::_getWalkSpeed(bool mounted) const
{
	return (mounted) ? _getMountedWalkSpeed() : _getUnmountedWalkSpeed();
}

inline float CreatureObject::_getUnmountedMinimumSpeed() const
{
	return 0.0f;
}

inline float CreatureObject::_getMinimumSpeed(bool mounted) const
{
	return (mounted) ? _getMountedMinimumSpeed() : _getUnmountedMinimumSpeed();
}

//-----------------------------------------------------------------

CreatureObject::CreatureObject(const SharedCreatureObjectTemplate * const newTemplate) :
	TangibleObject               (newTemplate),
	m_targetUnderCursor			 (NetworkId::cms_invalid),
	m_visualPosture              (Postures::Upright),
	m_serverPosture              (Postures::Upright),
	m_revertToServerPostureTimer (cs_revertToServerPostureInterval),
	m_underWearChestObject       (NULL),
	m_underWearPantsObject       (NULL),
	m_attributes                 (Attributes::NumberOfAttributes),
	m_maxAttributes              (Attributes::NumberOfAttributes),
	m_unmodifiedMaxAttributes    (Attributes::NumberOfAttributes),
	m_attribBonus                (Attributes::NumberOfAttributes),
	m_shockWounds                (0),
	m_mood                       (0),
	m_sayMode                    (0),
	m_animationMood              (AnimationMoods::neutral),
	m_lookAtTarget               (),
	m_lookAtTargetSlot           (ShipChassisSlotType::SCST_invalid),
	m_intendedTarget             (),
	m_currentWeapon              (),
	m_skills                     (),
	m_movementScale              (1.0f),
	m_movementPercent            (1.0f),
	m_walkSpeed                  (newTemplate->getSpeed(SharedCreatureObjectTemplate::MT_walk)),
	m_runSpeed                   (newTemplate->getSpeed(SharedCreatureObjectTemplate::MT_run)),
	m_accelScale                 (1.0f),
	m_accelPercent               (1.0f),
	m_turnScale                  (1.0f),
	m_slopeModAngle              (newTemplate->getSlopeModAngle() * PI / 180),
	m_slopeModPercent            (newTemplate->getSlopeModPercent()),
	m_waterModPercent            (newTemplate->getWaterModPercent()),
	m_groupMissionCriticalObjectSet(),
	m_scaleFactor                (1.0f),
	m_states                     (0),
	m_modMap                     (),
	m_group                      (CachedNetworkId::cms_cachedInvalid),
	m_groupInviter               (PlayerAndShipPair(std::make_pair(NetworkId::cms_invalid, std::string()), NetworkId::cms_invalid)),
	m_performanceType            (0),
	m_performanceStartTime       (),
	m_performanceListenTarget    (NetworkId::cms_invalid),
	m_guildId                    (0),
	m_masterId                   (NetworkId::cms_invalid),
	m_rank                       (0),
	m_accumulatedHamDamage       (0),
	m_oldMood                    (0),
	m_temporaryMoodActive        (false),
	m_temporaryMoodDuration      (0.0f),
	m_settingTemporaryMood       (false),
	m_isSittingOnObject          (false),
	m_hasHeldStateObject         (false),
	m_heldStateObjectWatcher     (new Watcher<Object>()),
	m_animatingSkillData         (),
	m_level                      (0),
	m_totalLevelXp(0),
	m_levelHealthGranted         (0),
	m_wearableData               (),
	m_alternateAppearanceSharedObjectTemplateName (),
	m_coverVisibility            (true),
	m_isPlaybackScriptScheduledToIncapacitate (false),
	m_hasAltered                              (false),
	m_isRidingMount              (false),
	m_isMountForThisClientPlayer (false),
	m_turnRateMaxWalk            (newTemplate->getTurnRateMax(SharedCreatureObjectTemplate::MT_walk)),
	m_turnRateMaxRun             (newTemplate->getTurnRateMax(SharedCreatureObjectTemplate::MT_run)),
	m_accelerationMaxWalk        (newTemplate->getAccelerationMax(SharedCreatureObjectTemplate::MT_walk)),
	m_accelerationMaxRun         (newTemplate->getAccelerationMax(SharedCreatureObjectTemplate::MT_run)),
	m_warpTolerance              (newTemplate->getWarpTolerance()),
	m_shipStation                (ShipStation::ShipStation_None),
	m_lastWaterDamageTimeMs(0),
	m_healingAttachedObject(0),
	m_alternateSharedCreatureObjectTemplate (NULL),
	m_clientUsesAnimationLocomotion(false),
	m_buffs(),
	m_difficulty(static_cast<unsigned char>(D_normal)),
	m_commands(),
	m_hologramType(-1),
	m_visibleOnMapAndRadar(true),
	m_isAiming(false),
	m_lookAtYaw(0.0f),
	m_useLookAtYaw(0),
	m_localLookAtPositionObject(),
	m_isBeast(false),
	m_forceShowHam(false),
	m_wearableAppearanceData(),
	m_initAppearanceWearables(false),
	m_verifyAppearanceTimer(10.0f),
	m_dupedCreaturesDirty(false),
	m_decoyOrigin(NetworkId::cms_invalid)
{
	++ms_numberOfInstances;

	if (Game::getSinglePlayer())
	{
		// - jww tmp for testing
		for (int i = 0; i < Attributes::NumberOfAttributes; ++i)
		{
			const short cur   = static_cast<short>(Random::random (100, 300) + (i * 50));
			const short max   = static_cast<short>(Random::random (800, 1000) - (40 * i));

			clientResetAttribute          (i, cur, max);
			m_unmodifiedMaxAttributes.set (static_cast<unsigned int>(i), m_maxAttributes.get (static_cast<unsigned int>(i)));
		}
	}

	m_animationMood.setSourceObject      (this);
	m_scaleFactor.setSourceObject        (this);
	m_states.setSourceObject             (this);
	m_mood.setSourceObject               (this);
	m_visualPosture.setSourceObject      (this);
	m_serverPosture.setSourceObject      (this);
	m_group.setSourceObject              (this);
	m_performanceType.setSourceObject    (this);
	m_animatingSkillData.setSourceObject (this);
	m_attributes.setOnSet                (this, &CreatureObject::attributesOnSet);
	m_alternateAppearanceSharedObjectTemplateName.setSourceObject (this);
	m_coverVisibility.setSourceObject(this);
	m_totalLevelXp.setSourceObject(this);
	m_hologramType.setSourceObject(this);
	m_visibleOnMapAndRadar.setSourceObject(this);

	// set up automagic updating
	addClientServerVariable(m_unmodifiedMaxAttributes); //Corresponds to server side m_maxAttributes
	addClientServerVariable(m_skills);

	addClientServerVariable_np(m_accelPercent);
	addClientServerVariable_np(m_accelScale);
	addClientServerVariable_np(m_attribBonus);
	addClientServerVariable_np(m_modMap);
	addClientServerVariable_np(m_movementPercent);
	addClientServerVariable_np(m_movementScale);
	addClientServerVariable_np(m_performanceListenTarget);
	addClientServerVariable_np(m_runSpeed);
	addClientServerVariable_np(m_slopeModAngle);
	addClientServerVariable_np(m_slopeModPercent);
	addClientServerVariable_np(m_turnScale);
	addClientServerVariable_np(m_walkSpeed);
	addClientServerVariable_np(m_waterModPercent);
	addClientServerVariable_np(m_groupMissionCriticalObjectSet);
	m_groupMissionCriticalObjectSet.setOnChanged(this, &CreatureObject::groupMissionCriticalObjectsOnChanged);
	addClientServerVariable_np(m_commands);

	addClientServerVariable_np(m_totalLevelXp);

	addSharedVariable(m_serverPosture);
	addSharedVariable(m_rank);
	addSharedVariable(m_masterId);
	addSharedVariable(m_scaleFactor);
	addSharedVariable(m_shockWounds);
	addSharedVariable(m_states);

	addSharedVariable_np(m_level);
	addSharedVariable_np(m_levelHealthGranted);
	addSharedVariable_np(m_animatingSkillData);
	addSharedVariable_np(m_animationMood);
	addSharedVariable_np(m_currentWeapon);
	addSharedVariable_np(m_group);
	addSharedVariable_np(m_groupInviter);
	addSharedVariable_np(m_guildId);
	addSharedVariable_np(m_lookAtTarget);
	addSharedVariable_np(m_intendedTarget);
	addSharedVariable_np(m_mood);
	addSharedVariable_np(m_performanceStartTime);
	addSharedVariable_np(m_performanceType);
	addSharedVariable_np(m_attributes);    //Coresponds to server side m_totalAttributes
	addSharedVariable_np(m_maxAttributes); //Coresponds to server side m_totalMaxAttributes
	addSharedVariable_np(m_wearableData);
	addSharedVariable_np(m_alternateAppearanceSharedObjectTemplateName);
	addSharedVariable_np(m_coverVisibility);
	addSharedVariable_np(m_buffs);
	addSharedVariable_np(m_clientUsesAnimationLocomotion);
	addSharedVariable_np(m_difficulty);
	addSharedVariable_np(m_hologramType);
	addSharedVariable_np(m_visibleOnMapAndRadar);
	addSharedVariable_np(m_isBeast);
	addSharedVariable_np(m_forceShowHam);
	addSharedVariable_np(m_wearableAppearanceData);
	addSharedVariable_np(m_decoyOrigin);

	// this needs to be done in the constructor because we want/need
	// to get notified when this value is initially set 
	m_groupInviter.setSourceObject(this);

	//now that scale has its source set, initialize it from template
	const float scale = std::max (0.0f, newTemplate->getScale());
	m_scaleFactor = scale;

	SkeletalAppearance2 *const appearance = dynamic_cast<SkeletalAppearance2*> (getAppearance ());
	if (appearance)
	{
		//-- Tell animation system about the creature gender.
		const SharedCreatureObjectTemplate::Gender gender = static_cast<SharedCreatureObjectTemplate::Gender>(getGender ());
		char  genderLetter[2];

		genderLetter[1] = 0;
		switch (gender)
		{
			case SharedCreatureObjectTemplate::GE_male:
				genderLetter[0] = 'm';
				break;

			case SharedCreatureObjectTemplate::GE_female:
				genderLetter[0] = 'f';
				break;

			case SharedCreatureObjectTemplate::GE_other:
			default:
				genderLetter[0] = 'o';
				break;
		} //lint !e788 // error 788: (Info -- enum constant 'Gender::Gender_Last' not used within defaulted switch) // can't be used, it's a duplicate enum value.

		appearance->getAnimationEnvironment ().getString (AnimationEnvironmentNames::cms_gender).setString (genderLetter);
	}

	checkWearingUnderWear();

	//-- Default the animation to neutral.
	signalAnimationMoodModified(AnimationMoods::neutral);
	setAnimationMood("neutral");

	if (GameObjectTypes::isTypeOf (getGameObjectType (), SharedObjectTemplate::GOT_vehicle))
	{
		CustomizationData *const cd = fetchCustomizationData();
		if (cd)
		{
			VehicleHoverDynamics::setInitialParams (*cd,
				safe_cast<const SharedCreatureObjectTemplate*> (getObjectTemplate ())->getSpeedMin        (SharedCreatureObjectTemplate::MT_walk),
				safe_cast<const SharedCreatureObjectTemplate*> (getObjectTemplate ())->getSpeed           (SharedCreatureObjectTemplate::MT_run),
				safe_cast<const SharedCreatureObjectTemplate*> (getObjectTemplate ())->getTurnRate        (SharedCreatureObjectTemplate::MT_walk),
				safe_cast<const SharedCreatureObjectTemplate*> (getObjectTemplate ())->getTurnRate        (SharedCreatureObjectTemplate::MT_run),
				safe_cast<const SharedCreatureObjectTemplate*> (getObjectTemplate ())->getAccelerationMax (SharedCreatureObjectTemplate::MT_walk),
				safe_cast<const SharedCreatureObjectTemplate*> (getObjectTemplate ())->getAccelerationMax (SharedCreatureObjectTemplate::MT_run));

			cd->release ();
		}

		//AI controlled hover vehicles need to attach their saddles here because they will not be made mountable
		if(getGameObjectType() == SharedObjectTemplate::GOT_vehicle_hover_ai)
		{
			SaddleManager::addRiderSaddleToMount(*this);
		}	
	}
}

//-------------------------------------------------------------------

CreatureObject::~CreatureObject ()
{
	--ms_numberOfInstances;

#ifdef _DEBUG
	//-- Remove associated data from CreatureInfo tracking class.
	CreatureInfo::removeCreature(getNetworkId());
#endif

	if (m_alternateSharedCreatureObjectTemplate)
	{
		m_alternateSharedCreatureObjectTemplate->releaseReference();
		m_alternateSharedCreatureObjectTemplate = NULL;
	}

	GroupManager::handleGroupChange(*this, getGroup(), NetworkId::cms_invalid);

	if (isInWorld())
		CreatureObject::removeFromWorld();

	delete m_heldStateObjectWatcher;
	m_heldStateObjectWatcher = NULL;

	delete m_underWearChestObject;
	m_underWearChestObject = NULL;

	delete m_underWearPantsObject;
	m_underWearPantsObject = NULL;

	if (m_healingAttachedObject)
	{
		delete m_healingAttachedObject;
		m_healingAttachedObject = 0;
	}
}

//-------------------------------------------------------------------

/**
 * Creates a default controller for this object.
 *
 * @return the object's controller
 */
void CreatureObject::createDefaultController()
{
	setController (NON_NULL (new RemoteCreatureController (this)));
}

//----------------------------------------------------------------------

float CreatureObject::alter (float deltaTime)
{
	//-- hovering vehicles should not render, only their saddles should render
	if (getGameObjectType () == SharedObjectTemplate::GOT_vehicle_hover || getGameObjectType() == SharedObjectTemplate::GOT_vehicle_hover_ai)
	{
		SkeletalAppearance2 *const appearance = const_cast<SkeletalAppearance2 *> (getAppearance () ? getAppearance ()->asSkeletalAppearance2 () : 0);
		if (appearance)
			appearance->setShowMesh (false);
	}

	PlayerObject * owner = getPlayerObject();
	if(owner)
		owner->updatePlayedTimeAccum (deltaTime);

	if (TangibleObject::alter (deltaTime) == AlterResult::cms_kill) //lint !e777 // Testing floats for equality // it's okay, we use defines on set and on test.
		return AlterResult::cms_kill;

	//-- Check if this creature is the client player's mounted creature.
	if ((getCondition() & static_cast<int>(TangibleObject::C_mount)) != 0)
	{
		CreatureObject *const playerCreatureObject = Game::getPlayerCreature();
		m_isMountForThisClientPlayer = playerCreatureObject && playerCreatureObject->isRidingMount() && (playerCreatureObject->getMountedCreature() == this);

		CollisionProperty * collision = getCollisionProperty();

		if(collision) collision->setMount(m_isMountForThisClientPlayer);
	}

	//-- Ensure held state object hasn't been destroyed.
	if (m_hasHeldStateObject && !*m_heldStateObjectWatcher)
		setAppearanceHeldItemState ();

	if (m_temporaryMoodActive)
	{
		m_temporaryMoodDuration -= deltaTime;

		if (m_temporaryMoodDuration < 0.0f)
			clientSetMood (m_oldMood);
	}

	if (m_accumulatedHamDamage)
	{
		Transceivers::damageTaken.emitMessage (*this);
		m_accumulatedHamDamage = 0;

		if (getAttribute(Attributes::Health) <= 0)
		{
			if (isPlayer())
			{
				if (getVisualPosture() != Postures::Incapacitated)
					setVisualPosture(Postures::Incapacitated);
			}
			else if (!ClientObject::isFakeNetworkId(getNetworkId()))
			{
				CreatureObject *playerObject = Game::getPlayerCreature();
				
				if(playerObject && playerObject->getIntendedTarget() == getNetworkId())
					playerObject->setIntendedTarget(NetworkId::cms_invalid);
				setVisualPosture(Postures::Dead);
			}
		}
	}

	CollisionProperty * collision = getCollisionProperty ();

	if(collision)
	{
		collision->setInCombat(getState (States::Combat));
	}

	//-- Ensure creature is faithfully representing server posture after a maximum deviation interval expires.
	if (!Game::getSinglePlayer () && !ms_disableCreaturePostureRevert)
	{
		const Postures::Enumerator currentPosture = m_visualPosture.get ();
		if (currentPosture != m_serverPosture.get())
		{
			m_revertToServerPostureTimer -= deltaTime;
			if (m_revertToServerPostureTimer <= 0.0f)
			{
				DEBUG_WARNING(currentPosture == static_cast<int>(Postures::Dead), ("CreatureObject: *** id=[%s],ot=[%s] reverting from dead to a non-dead server posture, client is [%s], server wants [%s], ignoring!  Makes no sense.", getNetworkId().getValueString().c_str(), getObjectTemplateName(), Postures::getPostureName(static_cast<Postures::Enumerator>(currentPosture)), Postures::getPostureName(static_cast<Postures::Enumerator>(m_serverPosture.get()))));
				DEBUG_REPORT_LOG(true ,("CreatureObject: id=[%s],ot=[%s] reverted to server posture, client was [%s], now is [%s].\n", getNetworkId().getValueString().c_str(), getObjectTemplateName(), Postures::getPostureName(static_cast<Postures::Enumerator>(currentPosture)), Postures::getPostureName(static_cast<Postures::Enumerator>(m_serverPosture.get()))));
				setVisualPosture (m_serverPosture.get());
			}
		}
	}

	//-- make sure lookAt target has not been lost

	{
		const CachedNetworkId lookAtTargetId = m_lookAtTarget.get ();
		if (lookAtTargetId != NetworkId::cms_invalid)
		{
			Object * const lookAtObj = lookAtTargetId.getObject ();
			if (!lookAtObj)
			{
				setLookAtTarget (NetworkId::cms_invalid);
			}
			else
			{
				//-- lookat target has been picked up or otherwise placed into a container
				if (!lookAtObj->isInWorld ())
					setLookAtTarget (NetworkId::cms_invalid);
				ClientObject * const clientObj = lookAtObj->asClientObject();
				if (clientObj && clientObj->asCreatureObject() && !clientObj->asCreatureObject()->getCoverVisibility() && !clientObj->asCreatureObject()->isPassiveRevealPlayerCharacter(getNetworkId()))
					setLookAtTarget (NetworkId::cms_invalid);
			}
		}
	}

	//-- now do the same with the intended target
	
	{
		const CachedNetworkId intendedTargetId = m_intendedTarget.get ();
		if (intendedTargetId != NetworkId::cms_invalid)
		{
			Object * const intendedObj = intendedTargetId.getObject ();
			if (!intendedObj)
			{
				setIntendedTarget (NetworkId::cms_invalid);
			}
			else
			{
				//-- intended target has been picked up or otherwise placed into a container
				if (!intendedObj->isInWorld ())
					setIntendedTarget (NetworkId::cms_invalid);
				ClientObject * const clientObj = intendedObj->asClientObject();
				if (clientObj && clientObj->asCreatureObject() && !clientObj->asCreatureObject()->getCoverVisibility() && !clientObj->asCreatureObject()->isPassiveRevealPlayerCharacter(getNetworkId()))
					setIntendedTarget (NetworkId::cms_invalid);
			}
		}
	}

	//-- handle playback script mechanism that ensures only one playback script plays that is scheduled to set creature to incapacitated or dead state.
	if (m_isPlaybackScriptScheduledToIncapacitate)
	{
		if (m_playbackScriptNoIncapacitationTimer.updateNoReset(deltaTime))
		{
			//-- Period for dead/incap playback script assignment is over.  This state ends after a few seconds
			//   because players/creatures can get incapacitated, then come back up again.  A set-once flag is not the way
			//   to handle this.
			m_isPlaybackScriptScheduledToIncapacitate = false;
		}
	}

	Object const * const thePlayer = Game::getPlayer ();
	if (!m_hasAltered)
	{
		if (this == thePlayer)
			setPlayerSpecificCallbacks ();

		m_hasAltered = true;
	}

	if (ms_showHealingObject)
	{
		if (m_healingAttachedObject)
		{
			delete m_healingAttachedObject;
			m_healingAttachedObject = 0;
		}

		if (m_healingAttachedObject)
		{
			Appearance const * const appearance = getAppearance ();
			if (appearance && appearance->getRenderedThisFrame ())
				m_healingAttachedObject->setPosition_p (CuiObjectTextManager::getCurrentObjectHeadPoint_o (*this) + Vector::unitY * 0.25f);
		}
	}
	else
	{
		if (m_healingAttachedObject)
		{
			delete m_healingAttachedObject;
			m_healingAttachedObject = 0;
		}
	}

	//-- Do mount verification
	if (ms_doMountSanityChecking)
	{
		CreatureObject const * const mount = getMountedCreature();
		CreatureObject const * const rider = getRiderDriverCreature();
		Object const * const containedByObject = ContainerInterface::getContainedByObject(*this);

		//-- Rider
		if (getState(States::RidingMount))
		{
			//-- We are riding a mount, so we should have a mount
			FATAL(!mount, ("mount is NULL"));

			//-- The contained by should be a CreatureObject
			FATAL(!containedByObject || !containedByObject->asClientObject() || !containedByObject->asClientObject()->asCreatureObject(), ("States::RidingMount is set and containedByObject is not a CreatureObject"));

			//-- The mount's rider should be this CreatureObject
			FATAL(mount->getRiderDriverCreature() != this, ("States::RidingMount is set and mount's rider is not 'this' rider"));
		}
		else
		{
			//-- We are not riding a mount, so we should not have a mount
			FATAL(mount, ("mount is non-NULL"));

			//-- Our contained by should not be a CreatureObject
			FATAL(containedByObject && containedByObject->asClientObject() && containedByObject->asClientObject()->asCreatureObject(), ("States::RidingMount is not set and containedByObject is a CreatureObject"));
		}

		//-- Mounted creature
		if (getState(States::MountedCreature))
		{
			//-- We are mounted, so we should have a rider
			FATAL(!rider, ("rider is NULL"));

			//-- Our rider's mounted creature should be this CreatureObject
			FATAL(rider->getMountedCreature() != this, ("States::MountedCreature is set and rider's mount is not 'this' mount"));
		}
		else
		{
			//-- We're not mounted, so we should not have a rider
			FATAL(rider, ("rider is non-NULL"));
		}
	}

	// see if we are performing a slow down effect
	Property * property = getProperty(SlowDownProperty::getClassPropertyId());
	if (property != NULL)
	{
		// has it expired?
		const GroundScene * groundScene = safe_cast<GroundScene *>(Game::getScene());
		if (groundScene != NULL)
		{
			SlowDownProperty * slowdown = safe_cast<SlowDownProperty *>(property);
			if (slowdown->getExpireTime() <= groundScene->getServerTime())
			{
				// yes, remove the effect
				removeProperty(SlowDownProperty::getClassPropertyId());
			}
			else
			{
				// we need to find if our player is in the effect's area, and and
				// tell him he is moving on our effect "hill" during his next
				// alter
				Object * target = slowdown->getTarget().getObject();
				if (target != NULL)
				{
					CreatureObject * ourPlayer = Game::getPlayerCreature();
					if (ourPlayer != NULL && ourPlayer->getNetworkId() != getNetworkId())
					{
						float testDistance = slowdown->getConeLength();
						if (getPosition_w().magnitudeBetweenSquared(ourPlayer->getPosition_w()) <= testDistance * testDistance &&
							ClientWorld::isObjectInCone(*ourPlayer, *this, *target, slowdown->getConeAngle()))
						{
							// compute the effective normal vector of our "hill"
							Vector normal(getPosition_w() - target->getPosition_w());
							normal.y = 0;
							normal.normalize();
							normal *= cos(slowdown->getSlopeAngle() + PI_OVER_2);
							normal.y = sin(slowdown->getSlopeAngle() + PI_OVER_2);
							ourPlayer->addTerrainSlopeEffect(normal);
						}
					}
				}
			}
		}
	}

	// make sure that "hidden" creatures are visible to admins
	SkeletalAppearance2 * const appearance = const_cast<SkeletalAppearance2 *>(getAppearance() ? getAppearance()->asSkeletalAppearance2() : 0);
	if (appearance)
	{
		if (m_coverVisibility.get() == false)
		{
			SkeletalAppearance2::FadeState fs = appearance->getFadeState();

			bool const isLocalPlayer = (Game::getPlayerCreature() == this);

			if (PlayerObject::isAdmin() || isPassiveRevealPlayerCharacter(Game::getPlayerNetworkId()))
			{
				if (fs == SkeletalAppearance2::FS_removed)
					appearance->setFadeState(SkeletalAppearance2::FS_fadeToHold, appearance->getFadeFraction(), 0.25f);
			}
			else if (!isLocalPlayer)
			{
				if (fs == SkeletalAppearance2::FS_hold)
				{
					appearance->setFadeState(SkeletalAppearance2::FS_fadeOutToRemove, appearance->getFadeFraction());
					ClientEffectManager::removeNonStealthClientEffects(this);
				}
			}
		}

		// make sure shadows appear/disappear properly with appearance
		if (appearance->getFadeState() == SkeletalAppearance2::FS_notFading)
		{
			if (m_shadowBlob.getPointer())
			{
				if (!m_shadowBlob->isVisible())
				{
					m_shadowBlob->setVisible(true);
				}
				
				if ((appearance->getHologramType() != SkeletalAppearance2::HT_none)
						|| appearance->getIsBlueGlowie()
						|| appearance->getIsHolonet()
				)
				{
					// don't display shadow blob when above appearances are used
					m_shadowBlob->setVisible(false);
				}
			}
		}
		else
		{
			if (m_shadowBlob.getPointer() && m_shadowBlob->isVisible())
				m_shadowBlob->setVisible(false);
			
		}
	}

#ifdef _DEBUG
	if (ms_reportGroupMissionCriticalObjects)
	{
		DEBUG_REPORT_PRINT(true, ("%s\n", getDebugInformation().c_str()));

		GroupMissionCriticalObjectSet const & groupMissionCriticalObjectSet = getGroupMissionCriticalObjects();
		for (GroupMissionCriticalObjectSet::iterator iter = groupMissionCriticalObjectSet.begin(); iter != groupMissionCriticalObjectSet.end(); ++iter)
			DEBUG_REPORT_PRINT(true, ("  <%s, %s>\n", iter->first.getValueString().c_str(), iter->second.getValueString().c_str()));
	}
#endif
	m_verifyAppearanceTimer.updateNoReset(deltaTime);

	if((m_initAppearanceWearables || m_verifyAppearanceTimer.isExpired()) && getAppearanceInventoryObject())
	{
		verifyWornAppearanceItems();
		checkWearingUnderWear();
		m_initAppearanceWearables = false;
		m_verifyAppearanceTimer.reset();
	}

	//--Set up the fake look at target, for every one except the main player
	if(this != Game::getPlayerCreature())
	{	
		if(m_useLookAtYaw)
		{		
			if(m_localLookAtPositionObject == NULL)
			{
				m_localLookAtPositionObject = new Object();
			}
			//--decide what position to actually look at.  If the position of the look at target is within
			//  a certain angle of the yaw, use the yaw instead, and just go out the same distance.  otherwise,
			//  use the position of the look at target
			Vector actualLookAtPosition;
			{
				Vector lookAtPosition;
				lookAtPosition.x = sin(m_lookAtYaw) * 10.0f;
				lookAtPosition.z = cos(m_lookAtYaw) * 10.0f;
				lookAtPosition += getPosition_w();
				actualLookAtPosition = lookAtPosition;			
			}
			m_localLookAtPositionObject->setPosition_w(actualLookAtPosition);
			CreatureController *const controller = dynamic_cast<CreatureController*>(getController());
			if (controller)
				controller->overrideAnimationTarget(m_localLookAtPositionObject, true, CrcLowerString());
		}
	}

	// @todo return appropriate alter interval.
	return AlterResult::cms_alterNextFrame;
}

// ----------------------------------------------------------------------

void CreatureObject::containedByModified(NetworkId const &oldValue, NetworkId const &newValue, bool isLocal)
{
	// We mount or dismount due to containment changes.
	// We only unpilot due to containment changes (piloting occurs due to arrangement change).

	bool const wasRidingMountBefore  = isRidingMount ();
	int const shipStationBefore = getShipStation();

	//-- Chain to base class.
	TangibleObject::containedByModified(oldValue, newValue, isLocal);

	//-- Check if we're riding a mount.
	updateRidingMountStatus();

	//-- Check if we're piloting or the droid for a ship.
	updateShipStation();

	//-- Update transform modifiers.
	setAppearanceTransformModifiers();

	bool const isRidingMountNow  = isRidingMount();
	int const shipStationNow = getShipStation();

	//-- Handle parent-child relationship associated with a mount change.
	if (!wasRidingMountBefore && isRidingMountNow)
		onRiderMountedMount();
	else if (wasRidingMountBefore && !isRidingMountNow)
		onRiderDismountedMount(oldValue);

	//-- Handle parent-child relationship associated with a pilot change.
	if (shipStationBefore != shipStationNow)
	{
		if (shipStationBefore == ShipStation::ShipStation_Pilot)
			onLeftPilotStation(oldValue);
		else if (shipStationBefore == ShipStation::ShipStation_Operations)
			onLeftOperationsStation(oldValue);
		else if (shipStationBefore == ShipStation::ShipStation_Droid)
			onLeftDroidStation(oldValue);
		else if (ShipStation::isGunnerStation(shipStationBefore))
			onLeftGunnerStation(oldValue, ShipStation::getWeaponIndexForGunnerStation(shipStationBefore));

		// clear out target on station change
		setLookAtTarget(NetworkId::cms_invalid);
	}

	bool const visibleNow = shouldCreatureBeVisible(*this);
	SkeletalAppearance2 * const appearance = getAppearance() ? getAppearance()->asSkeletalAppearance2() : 0;
	if (appearance)
		appearance->setShowMesh(visibleNow);
}

// ----------------------------------------------------------------------

void CreatureObject::arrangementModified(int oldValue, int newValue, bool isLocal)
{
	// We only pilot due to arrangement (unpiloting and mount operations are due to containment).
	int const shipStationBefore = getShipStation();

	//-- Chain to base class.
	TangibleObject::arrangementModified(oldValue, newValue, isLocal);

	//-- Check if we're piloting or the droid for a ship.
	updateShipStation();

	//-- Update transform modifiers.
	setAppearanceTransformModifiers();

	int const shipStationNow = getShipStation();

	//-- Handle parent-child relationship associated with a pilot change.
	if (shipStationBefore != shipStationNow)
	{
		if (shipStationNow == ShipStation::ShipStation_Pilot)
			onEnteredPilotStation();
		else if (shipStationNow == ShipStation::ShipStation_Droid)
			onEnteredDroidStation();
		else if (shipStationNow == ShipStation::ShipStation_Operations)
			onEnteredOperationsStation();
		else if (ShipStation::isGunnerStation(shipStationNow))
		{
			if (ShipStation::isGunnerStation(shipStationBefore))
				onLeftGunnerStation(NON_NULL(getContainedByProperty())->getContainedByNetworkId(), ShipStation::getWeaponIndexForGunnerStation(shipStationBefore));
			onEnteredGunnerStation(ShipStation::getWeaponIndexForGunnerStation(shipStationNow));
		}

		// clear out target on station change
		setLookAtTarget(NetworkId::cms_invalid);
	}

	bool const visibleNow = shouldCreatureBeVisible(*this);
	SkeletalAppearance2 * const appearance = getAppearance() ? getAppearance()->asSkeletalAppearance2() : 0;
	if (appearance)
		appearance->setShowMesh(visibleNow);
}

// ----------------------------------------------------------------------

void CreatureObject::doPostContainerChangeProcessing ()
{
	setAppearanceHeldItemState ();
	setAppearanceTransformModifiers ();

	//-- If we have customization variables, tell them to alter.
	//   This will handle things like updating procedural customization variables
	//   driven by if there is anything in certain slots.
	CustomizationData *const customizationData = fetchCustomizationData();
	if (customizationData)
	{
		customizationData->alterLocalVariables();
		customizationData->release();
	}
}

//-------------------------------------------------------------------

void CreatureObject::endBaselines()
{
	TangibleObject::endBaselines();

	//-- Use the ship animation mood to prevent incorrect motion for ship droids
	if (getShipStation() == ShipStation::ShipStation_Droid)
		setAnimationMood("ship");
	//-- Ensure baseline animation mood is flushed out to the animation system.
	signalAnimationMoodModified(m_animationMood.get());

	//-- Tell footprint about our per-object-scaled water height.
	Footprint *const footprint = getFootprint ();
	if (footprint)
		footprint->setSwimHeight (getSwimHeight ());

	//-- ALL client cached creatures are interesting, invul, and conversable
	if(isClientCached())
	{
		setCondition(C_interesting | C_invulnerable | C_conversable);
	}

	//-- create the shadow blob object
	float const collisionRadius = safe_cast<SharedCreatureObjectTemplate const *> (getObjectTemplate ())->getCollisionRadius ();
	if (collisionRadius > 0.f)
	{
		m_shadowBlob = new ShadowBlobObject (2.f * collisionRadius * getScaleFactor ());
		addChildObject_o ( m_shadowBlob );
	}
	else
		DEBUG_WARNING (true, ("CreatureObject::endBaselines: object [%s] has a collisionRadius of 0", getObjectTemplateName () ? getObjectTemplateName () : "null"));

	//-- Do post container change processing to setup animation state.
	doPostContainerChangeProcessing ();

	if (this != Game::getPlayer ())
	{
		//Set up streamed wearables for non-player creatures
		SlottedContainer * container = ContainerInterface::getSlottedContainer(*this);
		NOT_NULL(container);

		for (unsigned int k = 0; k < m_wearableData.size(); ++k)
		{
			Container::ContainerErrorCode tmp = Container::CEC_Success;
			const WearableEntry& w = m_wearableData.get(k);

			ConstCharCrcString cc = ObjectTemplateList::lookUp(w.m_objectTemplate);
			if (cc.isEmpty())
			{
				WARNING(true, ("SceneCreateObjectByCrc crc %08x was not found in object template list", cc.getCrc()));
				return;
			}

			DEBUG_WARNING(ms_logAppearanceTabMessages, ("Creating wearable object: [%s]", cc.getString()));

			TangibleObject* const wearable = safe_cast<TangibleObject*> (ObjectTemplate::createObject (cc.getString()));
			if (wearable)
			{
				wearable->setNetworkId(w.m_networkId);
				wearable->setAppearanceData(w.m_appearanceString);
				if (w.m_weaponSharedBaselines.get() != NULL)
					wearable->applyBaselines(*w.m_weaponSharedBaselines);
				if (w.m_weaponSharedNpBaselines.get() != NULL)
					wearable->applyBaselines(*w.m_weaponSharedNpBaselines);
				wearable->endBaselines();
				IGNORE_RETURN(container->add(*wearable, w.m_arrangement, tmp));
			}
			else
			{
				DEBUG_WARNING(ms_logAppearanceTabMessages, ("Failed to create wearable %s", cc.getString()));
			}

		}
		
		if(getAppearanceInventoryObject())
		{
			SlottedContainer * container = ContainerInterface::getSlottedContainer(*getAppearanceInventoryObject());
			for (unsigned int k = 0; k < m_wearableAppearanceData.size(); ++k)
			{
				Container::ContainerErrorCode tmp = Container::CEC_Success;
				const WearableEntry& w = m_wearableAppearanceData.get(k);

				ConstCharCrcString cc = ObjectTemplateList::lookUp(w.m_objectTemplate);
				if (cc.isEmpty())
				{
					WARNING(true, ("SceneCreateObjectByCrc crc %08x was not found in object template list", cc.getCrc()));
					return;
				}

				TangibleObject* const wearable = safe_cast<TangibleObject*> (ObjectTemplate::createObject (cc.getString()));
				DEBUG_WARNING(ms_logAppearanceTabMessages, ("Creating Appearance wearable object: [%s]", cc.getString()));
				if (wearable)
				{
					wearable->setNetworkId(w.m_networkId);
					wearable->setAppearanceData(w.m_appearanceString);
					wearable->endBaselines();
					IGNORE_RETURN(container->add(*wearable, w.m_arrangement, tmp));
				}
				else
				{
					DEBUG_WARNING(ms_logAppearanceTabMessages, ("Failed to create appearance wearable %s", cc.getString()));
				}
			}
		}
		else
			m_initAppearanceWearables = true;
	}
	else if (this == Game::getPlayer())
		m_initAppearanceWearables = true;

	m_wearableData.setOnErase(this, &CreatureObject::wearablesOnErase);
	m_wearableData.setOnInsert(this, &CreatureObject::wearablesOnInsert);
	m_wearableData.setOnChanged(this, &CreatureObject::wearablesOnChanged);

	m_wearableAppearanceData.setOnErase(this, &CreatureObject::appearanceWearablesOnErase);
	m_wearableAppearanceData.setOnInsert(this, &CreatureObject::appearanceWearablesOnInsert);
	m_wearableAppearanceData.setOnChanged(this, &CreatureObject::appearanceWearablesOnChanged);

	m_buffs.setOnErase(this, &CreatureObject::buffsOnErase);
	m_buffs.setOnInsert(this, &CreatureObject::buffsOnInsert);
	m_buffs.setOnSet(this, &CreatureObject::buffsOnSet);

	//-- Initialize visual posture to the server posture.
	DEBUG_REPORT_LOG(s_logVisualPostureChanges || s_logServerPostureChanges, ("posture change (both): object id [%s], baseline posture [%s].\n", getNetworkId().getValueString().c_str(), Postures::getPostureName(m_serverPosture.get())));
	m_visualPosture.set(m_serverPosture.get());

	//-- Handle sitting in chair.
	if ((m_visualPosture.get() == Postures::Sitting) && getState(States::SittingOnChair))
	{
		// Note: the client code does not look at the state flag for sitting on
		//       a chair because there is no guarantee it comes in at the right
		//       time (i.e. the state would have to be received prior to receiving
		//       the posture).  We look at this variable instead.  Set it up
		//       properly here during baselines processing.
		setSittingOnObject(true);

		// Note: we don't have a chair object, but we need the script to run.
		//       Set the chair to be this player.  The chair hardpoint won't
		//       exist, but at least the object scaling will occur.
		Object *const chairObject   = this;
		const int     positionIndex = 0;

		GamePlaybackScript::sitCreatureOnChair (*this, *chairObject, positionIndex);
	}

	//-- Because this is the first posture, we should not try to traverse from
	//   the client-initialized upright posture.  Tell the CreatureController to
	//   go to the new posture without traversing.
	CreatureController *const controller = dynamic_cast<CreatureController*> (getController ());
	if (controller)
	{
		const bool skipTransition = true;
		const bool skipWithDelay  = false;

		controller->changeAnimationStatePath (m_visualPosture.get(), skipTransition, skipWithDelay);
		controller->setCurrentSpeed (0.f);
	}

#ifdef _DEBUG
	//-- Posture debugging support: track incap/death setting via end baselines.
	Postures::Enumerator posture = getVisualPosture();

	CreatureInfo::PostureState postureState = CreatureInfo::PS_incapacitatedAtEndBaselines;
	bool setPostureState = false;

	switch (posture)
	{
		case Postures::Incapacitated:
			postureState    = CreatureInfo::PS_incapacitatedAtEndBaselines;
			setPostureState = true;
			break;

		case Postures::Dead:
			postureState    = CreatureInfo::PS_deadAtEndBaselines;
			setPostureState = true;
			break;

		default:
			// do nothing.
			break;
	}

	if (setPostureState)
		IGNORE_RETURN(CreatureInfo::setCreaturePostureState(getNetworkId(), postureState));
#endif

	// Handle any scale changes that may have come in after the saddle was created and the rider was mounted.
	doScaleChangeFixupsForMount();

	// Mounts: in the case where a rider/creature combo is coming across in baselines,
	//         the proper container structure is not available until the mount gets its
	//         endBaselines.  Handle rider mounting visuals at this time.
	if (getState(States::MountedCreature))
	{
		CreatureObject *const rider = getRiderDriverCreature();
		if (rider)
			rider->onRiderMountedMount();
	}

	bool const visibleNow = shouldCreatureBeVisible(*this);
	SkeletalAppearance2 * const appearance = getAppearance() ? getAppearance()->asSkeletalAppearance2() : 0;
	if (appearance)
		appearance->setShowMesh(visibleNow);

	m_commands.setOnErase(this, &CreatureObject::commandsOnErase);
	m_commands.setOnInsert(this, &CreatureObject::commandsOnInsert);

	if (this == Game::getPlayerCreature())
	{
		bool const wasAdmin = PlayerObject::isAdmin();

		PlayerObject::setAdmin(m_commands.find(s_adminCommand) != m_commands.getMap().end());
	
		if (wasAdmin != PlayerObject::isAdmin())
		{
			Transceivers::adminChanged.emitMessage(*this);
		}
	}
}

//----------------------------------------------------------------------

CreatureObject * CreatureObject::asCreatureObject()
{
	return this;
}

//-----------------------------------------------------------------------

CreatureObject const * CreatureObject::asCreatureObject() const
{
	return this;
}

//-----------------------------------------------------------------------

void CreatureObject::setPlayerSpecificCallbacks ()
{
	m_modMap.setOnErase            (this, &CreatureObject::skillModsOnErase);
	m_modMap.setOnInsert           (this, &CreatureObject::skillModsOnInsert);
	m_modMap.setOnSet              (this, &CreatureObject::skillModsOnSet);

	m_skills.setOnChanged          (this, &CreatureObject::skillsOnChanged);

	m_maxAttributes.setOnChanged   (this, &CreatureObject::maxAttributesOnChanged);

	m_lookAtTarget.setSourceObject (this);
	m_intendedTarget.setSourceObject (this);
	m_currentWeapon.setSourceObject(this);

	if (Game::getSinglePlayer () && this == Game::getPlayer ())
	{
		PlayerObject * const po = safe_cast<PlayerObject *>(ObjectTemplate::createObject ("object/player/shared_player.iff"));
		NOT_NULL (po);

		//-- @todo -TRF- verify that this is not invalid.
		po->endBaselines();

		SlottedContainer * const slotted = ContainerInterface::getSlottedContainer (*this);
		if (slotted)
		{
			int arr = 0;
			Container::ContainerErrorCode error;
			if (slotted->getFirstUnoccupiedArrangement(*po, arr, error))
				IGNORE_RETURN(ContainerInterface::transferItemToSlottedContainer (*this, *po, arr));
		}

		SkillObject const * skill = SkillManager::getInstance ().getSkill ("combat_marksman_novice");
		if (skill)
			IGNORE_RETURN(clientGrantSkill (*skill));
		const SkillObject * const skill2 = SkillManager::getInstance ().getSkill ("combat_marksman_pistol_01");
		if (skill2)
			IGNORE_RETURN(clientGrantSkill (*skill2));
		const SkillObject * const skill3 = SkillManager::getInstance ().getSkill ("combat_marksman_pistol_02");
		if (skill3)
			IGNORE_RETURN(clientGrantSkill (*skill3));
		const SkillObject * const skill4 = SkillManager::getInstance ().getSkill ("combat_marksman_pistol_03");
		if (skill4)
			IGNORE_RETURN(clientGrantSkill (*skill4));
		const SkillObject * const skill5 = SkillManager::getInstance ().getSkill ("combat_marksman_pistol_04");
		if (skill5)
			IGNORE_RETURN(clientGrantSkill (*skill5));
		const SkillObject * const skill6 = SkillManager::getInstance ().getSkill ("combat_marksman_master");
		if (skill6)
			IGNORE_RETURN(clientGrantSkill (*skill6));
		const SkillObject * const skill7 = SkillManager::getInstance ().getSkill ("combat_pistol_novice");
		if (skill7)
			IGNORE_RETURN(clientGrantSkill (*skill7));
		const SkillObject * const skill8 = SkillManager::getInstance ().getSkill ("combat_pistol_ability_01");
		if (skill8)
			IGNORE_RETURN(clientGrantSkill (*skill8));
		skill = SkillManager::getInstance ().getSkill ("combat_pistol_ability_02");
		if (skill)
			IGNORE_RETURN(clientGrantSkill (*skill));
		skill = SkillManager::getInstance ().getSkill ("combat_pistol_ability_03");
		if (skill)
			IGNORE_RETURN(clientGrantSkill (*skill));
		skill = SkillManager::getInstance ().getSkill ("combat_pistol_ability_04");
		if (skill)
			IGNORE_RETURN(clientGrantSkill (*skill));
		skill = SkillManager::getInstance ().getSkill ("combat_pistol_accuracy_01");
		if (skill)
			IGNORE_RETURN(clientGrantSkill (*skill));
		skill = SkillManager::getInstance ().getSkill ("combat_pistol_accuracy_02");
		if (skill)
			IGNORE_RETURN(clientGrantSkill (*skill));
		skill = SkillManager::getInstance ().getSkill ("combat_pistol_accuracy_03");
		if (skill)
			IGNORE_RETURN(clientGrantSkill (*skill));
		skill = SkillManager::getInstance ().getSkill ("combat_pistol_accuracy_04");
		if (skill)
			IGNORE_RETURN(clientGrantSkill (*skill));
		skill = SkillManager::getInstance ().getSkill ("combat_pistol_speed_01");
		if (skill)
			IGNORE_RETURN(clientGrantSkill (*skill));
		skill = SkillManager::getInstance ().getSkill ("combat_pistol_speed_02");
		if (skill)
			IGNORE_RETURN(clientGrantSkill (*skill));
		skill = SkillManager::getInstance ().getSkill ("combat_pistol_speed_03");
		if (skill)
			IGNORE_RETURN(clientGrantSkill (*skill));
		skill = SkillManager::getInstance ().getSkill ("combat_pistol_speed_04");
		if (skill)
			IGNORE_RETURN(clientGrantSkill (*skill));
		skill = SkillManager::getInstance ().getSkill ("combat_pistol_ability_01");
		if (skill)
			IGNORE_RETURN(clientGrantSkill (*skill));
		skill = SkillManager::getInstance ().getSkill ("combat_pistol_ability_02");
		if (skill)
			IGNORE_RETURN(clientGrantSkill (*skill));
		skill = SkillManager::getInstance ().getSkill ("combat_pistol_ability_03");
		if (skill)
			IGNORE_RETURN(clientGrantSkill (*skill));
		skill = SkillManager::getInstance ().getSkill ("combat_pistol_ability_04");
		if (skill)
			IGNORE_RETURN(clientGrantSkill (*skill));
		skill = SkillManager::getInstance ().getSkill ("social_musician_novice");
		if (skill)
			IGNORE_RETURN(clientGrantSkill (*skill));
		skill = SkillManager::getInstance ().getSkill ("social_dancer_novice");
		if (skill)
			IGNORE_RETURN(clientGrantSkill (*skill));
		skill = SkillManager::getInstance ().getSkill ("combat_marksman_rifle_01");
		if (skill)
			IGNORE_RETURN(clientGrantSkill (*skill));
		skill = SkillManager::getInstance ().getSkill ("combat_marksman_rifle_02");
		if (skill)
			IGNORE_RETURN(clientGrantSkill (*skill));
		skill = SkillManager::getInstance ().getSkill ("combat_marksman_rifle_03");
		if (skill)
			IGNORE_RETURN(clientGrantSkill (*skill));
		skill = SkillManager::getInstance ().getSkill ("combat_marksman_rifle_04");
		if (skill)
			IGNORE_RETURN(clientGrantSkill (*skill));
		skill = SkillManager::getInstance ().getSkill ("combat_marksman_carbine_01");
		if (skill)
			IGNORE_RETURN(clientGrantSkill (*skill));
		skill = SkillManager::getInstance ().getSkill ("combat_marksman_carbine_02");
		if (skill)
			IGNORE_RETURN(clientGrantSkill (*skill));
		skill = SkillManager::getInstance ().getSkill ("combat_marksman_carbine_03");
		if (skill)
			IGNORE_RETURN(clientGrantSkill (*skill));
		skill = SkillManager::getInstance ().getSkill ("combat_marksman_carbine_04");
		if (skill)
			IGNORE_RETURN(clientGrantSkill (*skill));
		skill = SkillManager::getInstance ().getSkill ("combat_marksman_support_01");
		if (skill)
			IGNORE_RETURN(clientGrantSkill (*skill));
		skill = SkillManager::getInstance ().getSkill ("combat_marksman_support_02");
		if (skill)
			IGNORE_RETURN(clientGrantSkill (*skill));
		skill = SkillManager::getInstance ().getSkill ("combat_marksman_support_03");
		if (skill)
			IGNORE_RETURN(clientGrantSkill (*skill));
		skill = SkillManager::getInstance ().getSkill ("combat_marksman_support_04");
		if (skill)
			IGNORE_RETURN(clientGrantSkill (*skill));

		clientGrantExp ("combat_rangedspecialize_pistol",  800);
		clientGrantExp ("combat_general",                  200);

	}

	PlayerObject * const playerObject = getPlayerObject ();
	if (playerObject)
		playerObject->setPlayerSpecificCallbacks ();
	else
		WARNING (true, ("CreatureObject::setPlayerSpecificCallbacks could not find PlayerObject for player!"));

	Transceivers::playerSetup.emitMessage (*this);

	ms_lastLookAtTargetRequest = m_lookAtTarget.get ();
	ms_lastIntendedTargetRequest = m_intendedTarget.get ();
}

// ----------------------------------------------------------------------
/**
 * Find out this creature's current defender's disposition.
 *
 * This function returns whether this creature is currently in combat,
 * whether this creature is currently an attacker, and if so, returns
 * the defender's disposition.  See ClientCombatActionInfo for the
 * DefenderDefense enum returned in defenderDisposition.
 *
 * The caller should ignore the defenderDispositionValue if either
 * isInCombat or isAttacker returns false.
 *
 * @param isInCombat           returns true if this creature is currently in
 *                             combat as far as the combat playback system is concerned.
 *                             If not, the rest of the function is skipped.
 * @param isAttacker           returns true if this creatuer is currently the attacker
 *                             in a combat playback script.  If not, the rest of the
 *                             function is skipped.
 * @param defenderDisposition  returns an int-casted version of the
 *                             ClientCombatActionInfo::DefenderDefense enum for the defender
 *                             defending against this creature.
 * @param attackNameCrc        returns a crc of teh currently-executing attack command name
 */

void CreatureObject::getDefenderDisposition(bool &isInCombat, bool &isAttacker, int &defenderDisposition, int &attackNameCrc) const
{
	//-- Get the playback script for this player as an attacker.
	const PlaybackScript *const playbackScript = PlaybackScriptManager::getPlaybackScriptForActorAtIndex(0, this);
	if (!playbackScript)
	{
		//-- Return that player is not in combat as an attacker.
		isInCombat = false;
		isAttacker = false;
		return;
	}

	//-- Get the defender disposition.
	const bool hasDefenderDefense = playbackScript->getIntVariable(TAG_DDFS, defenderDisposition);
	if (hasDefenderDefense)
	{
		// We got it.  Return the defender disposition.
		isInCombat = true;
		isAttacker = true;

		attackNameCrc = 0;
		IGNORE_RETURN(playbackScript->getIntVariable(TAG_AANC, attackNameCrc));
	}
	else
	{
		// Must not be a combat playback script, so we must not be in combat.
		isInCombat = false;
		isAttacker = false;
	}
}

//----------------------------------------------------------------------

float CreatureObject::getMaximumTurnRate(const float currentSpeed, bool ignoreVehicle) const
{
	const bool mounted = !ignoreVehicle && (m_states.get() & States::getStateMask(States::MountedCreature))!=0;

	return _getMaximumTurnRate(currentSpeed, mounted);
}

//------------------------------------------------------------------------

float CreatureObject::_getMountedMaximumTurnRate(const float currentSpeed) const
{
	//-- vehicle
	if (GameObjectTypes::isTypeOf (getGameObjectType (), SharedObjectTemplate::GOT_vehicle))
	{
		
		const Object * const vehicleSaddle = SaddleManager::getSaddleObjectFromMount (*this);
		const VehicleHoverDynamics * const vhd = vehicleSaddle ? dynamic_cast<const VehicleHoverDynamics *>(vehicleSaddle->getDynamics ()) : 0;
		if (vhd)
		{
			return vhd->getTurnRateForSpeedDegrees (currentSpeed) * getTurnPercent();
		}
	}

	return _getUnmountedMaximumTurnRate(currentSpeed);
}

//------------------------------------------------------------------------

float CreatureObject::getMaximumAcceleration(const float currentSpeed, bool ignoreVehicle) const
{
	if (  !ignoreVehicle
		&& (m_states.get() & States::getStateMask(States::MountedCreature))!=0
		)
	{
		float mountedAcceleration(0);
		if (_getMountedMaximumAcceleration(mountedAcceleration, currentSpeed))
		{
			return mountedAcceleration;
		}
	}

	//-- return walk acceleration if walking
	if (currentSpeed <= getMaximumWalkSpeed())
	{
		return m_accelerationMaxWalk * getAccelScale() * getAccelPercent();
	}

	//-- return run acceleration
   return m_accelerationMaxRun * getAccelScale() * getAccelPercent();
}

//------------------------------------------------------------------------

bool CreatureObject::_getMountedMaximumAcceleration(float &o_rate, const float currentSpeed) const
{
	//-- vehicle
	if (GameObjectTypes::isTypeOf (getGameObjectType (), SharedObjectTemplate::GOT_vehicle))
	{
		const Object * const vehicleSaddle = SaddleManager::getSaddleObjectFromMount (*this);
		const VehicleHoverDynamics * const vhd = vehicleSaddle ? dynamic_cast<const VehicleHoverDynamics *>(vehicleSaddle->getDynamics ()) : 0;
		if (vhd)
		{
			o_rate=vhd->getAccelForSpeed (currentSpeed) * getAccelPercent();
			return true;
		}
	}

	return false;
}

//------------------------------------------------------------------------

float CreatureObject::getMaximumSpeed (bool ignoreVehicle) const
{
	return std::max(getRunSpeed(ignoreVehicle), getWalkSpeed(ignoreVehicle));
}

//------------------------------------------------------------------------

ClientObject* CreatureObject::getEquippedObject (const char * const slotName)
{
	NOT_NULL (slotName);
	return ContainerInterface::getObjectInSlot (*this, slotName);
}

//----------------------------------------------------------------------

const ClientObject * CreatureObject::getEquippedObject       (const char * const slotName) const
{
	NOT_NULL (slotName);
	return const_cast<CreatureObject *>(this)->getEquippedObject (slotName);
}

//------------------------------------------------------------------------

ClientObject* CreatureObject::getAppearanceEquippedObject (const char * const slotName)
{
	NOT_NULL (slotName);
	if(!getAppearanceInventoryObject())
		return NULL;
	return ContainerInterface::getObjectInSlot (*getAppearanceInventoryObject(), slotName);
}

//----------------------------------------------------------------------

const ClientObject * CreatureObject::getAppearanceEquippedObject       (const char * const slotName) const
{
	NOT_NULL (slotName);
	return const_cast<CreatureObject *>(this)->getAppearanceEquippedObject (slotName);
}
//------------------------------------------------------------------------

WeaponObject* CreatureObject::getCurrentWeapon()
{
	return dynamic_cast<WeaponObject*>(m_currentWeapon.get().getObject());
}

//------------------------------------------------------------------------

const WeaponObject* CreatureObject::getCurrentWeapon() const
{
	return dynamic_cast<const WeaponObject*>(m_currentWeapon.get().getObject());
}

//------------------------------------------------------------------------

const std::string& CreatureObject::getCurrentPrimaryActionName() const
{
	static std::string actionName = "meleeHit";
	const WeaponObject* playerWeapon = getCurrentWeapon();	

	PlayerObject const * playerObj = getPlayerObject();

	if(playerObj && !playerObj->getDefaultAttackOverride().empty())
	{
		actionName = playerObj->getDefaultAttackOverride();
	}
	else if (NULL != playerWeapon)
	{
		//NOTE: The ordering of these checks is important - the goupings are not exclusive (one weapon could be in multiple of these groups)
		if (playerWeapon->isHeavyWeapon())
		{
			//heavy weapons have their action specified in template_command_mapping
			actionName = TemplateCommandMappingManager::getCommandForTemplateCrc(Crc::normalizeAndCalculate(playerWeapon->getTemplateName()));
		}
		else if (playerWeapon->isLightsaberWeapon())
		{
			actionName = "saberHit";
		}
		else if (playerWeapon->isRangedWeapon())
		{
			actionName = "rangedShot";

			//ranged weapons need the weapon type appended to the action name
			actionName += WeaponObject::getWeaponTypeString(playerWeapon->getWeaponType());	
		}
		else if (playerWeapon->isMeleeWeapon())
		{
			actionName = "meleeHit";
		}
	}

	return actionName;
}

//------------------------------------------------------------------------

bool CreatureObject::getPrimaryActionOverridden() const
{
	PlayerObject const * playerObj = getPlayerObject();

	return playerObj && !playerObj->getDefaultAttackOverride().empty();
}

//------------------------------------------------------------------------

bool CreatureObject::getPrimaryActionIsLocationBased() const
{
	if(getPrimaryActionOverridden())
	{
		std::string const & primaryCommandName = getCurrentPrimaryActionName();
		uint32 const primaryCmdHash = Crc::normalizeAndCalculate(primaryCommandName.c_str());
		Command const & cmd = CommandTable::getCommand(primaryCmdHash);
		return (cmd.m_targetType == Command::CTT_Location);
	}
	else 
	{
		const WeaponObject *weapon = getCurrentWeapon();
		return (weapon && (weapon->isGroundTargetting() || weapon->isDirectionalTargetting()));
	}
	return false;
}

//------------------------------------------------------------------------

bool CreatureObject::getPrimaryActionWantsGroundReticule() const
{
	if(getPrimaryActionOverridden())
	{
		std::string const & primaryCommandName = getCurrentPrimaryActionName();
		uint32 const primaryCmdHash = Crc::normalizeAndCalculate(primaryCommandName.c_str());
		Command const & cmd = CommandTable::getCommand(primaryCmdHash);
		return (cmd.m_targetType == Command::CTT_Location);
	}
	else 
	{
		const WeaponObject *weapon = getCurrentWeapon();
		return (weapon && weapon->isGroundTargetting());
	}
	return false;
}

//------------------------------------------------------------------------

ClientObject* CreatureObject::getInventoryObject()
{
	return getEquippedObject(Slots::inventory);
}

//------------------------------------------------------------------------

const ClientObject* CreatureObject::getInventoryObject() const
{
	return const_cast<CreatureObject *>(this)->getEquippedObject(Slots::inventory);
}

//------------------------------------------------------------------------

ClientObject* CreatureObject::getAppearanceInventoryObject()
{
	return getEquippedObject(Slots::appearance);
}

//------------------------------------------------------------------------

const ClientObject* CreatureObject::getAppearanceInventoryObject() const
{
	return const_cast<CreatureObject *>(this)->getEquippedObject(Slots::appearance);
}

//----------------------------------------------------------------------

PlayerObject * CreatureObject::getPlayerObject()
{
	return dynamic_cast<PlayerObject *>(getEquippedObject(Slots::player));
}

//----------------------------------------------------------------------

const PlayerObject * CreatureObject::getPlayerObject() const
{
	return dynamic_cast<PlayerObject *>(const_cast<CreatureObject *>(this)->getEquippedObject(Slots::player));
}

//----------------------------------------------------------------------

ClientObject* CreatureObject::getHairObject()
{
	return getEquippedObject(Slots::hair);
}

//------------------------------------------------------------------------

const ClientObject* CreatureObject::getHairObject() const
{
	return const_cast<CreatureObject *>(this)->getEquippedObject(Slots::hair);
}

//----------------------------------------------------------------------

ClientObject* CreatureObject::getDatapadObject()
{
	return getEquippedObject(Slots::datapad);
}

//------------------------------------------------------------------------

const ClientObject* CreatureObject::getDatapadObject() const
{
	return const_cast<CreatureObject *>(this)->getEquippedObject(Slots::datapad);
}

//----------------------------------------------------------------------

ClientObject * CreatureObject::getBankObject           ()
{
	return getEquippedObject (Slots::bank);
}

//----------------------------------------------------------------------

const ClientObject * CreatureObject::getBankObject           () const
{
	return const_cast<CreatureObject *>(this)->getEquippedObject (Slots::bank);
}

//------------------------------------------------------------------------

float CreatureObject::getSwimHeight () const
{
	return safe_cast<const SharedCreatureObjectTemplate*> (getObjectTemplate ())->getSwimHeight () * getScaleFactor ();
}

//------------------------------------------------------------------------

float CreatureObject::getCameraHeight () const
{
	return safe_cast<const SharedCreatureObjectTemplate*> (getObjectTemplate ())->getCameraHeight () * getScaleFactor ();
}

//------------------------------------------------------------------------

bool CreatureObject::getAlignToTerrain() const
{
	return safe_cast<const SharedCreatureObjectTemplate *>(getObjectTemplate())->getPostureAlignToTerrain(static_cast<SharedCreatureObjectTemplate::Postures> (getVisualPosture ()));
}

//----------------------------------------------------------------------

/**
* Notify systems that our mood changed.  E.g. the animation system cares about this.
*/

void CreatureObject::Callbacks::IncapacityChanged::modified(CreatureObject & target, const int &, const int &, bool) const
{
	Transceivers::incapacityChanged.emitMessage (target);
}

//----------------------------------------------------------------------

/**
* Notify systems that our group inviter changed
*/

void CreatureObject::Callbacks::GroupInviterChanged::modified(CreatureObject & target, const CreatureObject::PlayerAndShipPair &, const CreatureObject::PlayerAndShipPair & newValue, bool) const
{
	GroupManager::handleGroupInviterChange (target);

	if (target.m_groupInviter.get () == newValue)
		Transceivers::groupInviterChanged.emitMessage (target);
}

//----------------------------------------------------------------------

void CreatureObject::Callbacks::TotalLevelXpChanged::modified(CreatureObject & target, const int & /*oldLevelXp*/, const int & /*newLevelXp*/, bool) const
{
	Transceivers::totalLevelXpChanged.emitMessage(target);
}

//----------------------------------------------------------------------

void CreatureObject::Callbacks::LevelChanged::modified(CreatureObject & target, const int & /*oldLevel*/, const int & /*newLevel*/, bool) const
{
	Transceivers::levelChanged.emitMessage(target);
}

//----------------------------------------------------------------------

/**
* Notify systems that our mood changed.  E.g. the animation system cares about this.
*/

void CreatureObject::Callbacks::MoodChanged::modified(CreatureObject & target, const uint8 &, const uint8 & current, bool) const
{
	if (!target.m_settingTemporaryMood)
	{
		target.m_temporaryMoodActive   = false;
		target.m_oldMood               = 0;
		target.m_temporaryMoodDuration = 0.0f;
	}

	std::string anim;
	if (MoodManagerClient::getMoodEmoteAnimation  (current, anim) ||
		MoodManagerClient::getMoodEmoteAnimation  (0, anim))
		target.setAnimationMood (anim);
	else
		DEBUG_WARNING (true, ("Could not find any valid mood animation state."));
}

//----------------------------------------------------------------------

void CreatureObject::setSayMode              (uint32 sayMode)
{
	WARNING_STRICT_FATAL (sayMode > 255, ("sayMode out of range"));
	m_sayMode.set (static_cast<uint8>(sayMode));
}

//------------------------------------------------------------------------

void CreatureObject::setAnimationMood(const std::string & mood)
{
	// @todo check to see if this is necessary, remove if not.
	m_animationMood = mood;
}

//----------------------------------------------------------------------

bool CreatureObject::clientGrantSkill (const SkillObject & skill)
{
	if (hasSkill(skill))
		return false;

	m_skills.insert(&skill);

	const SkillObject::StringVector & commands = skill.getCommandsProvided ();
	for (SkillObject::StringVector::const_iterator it = commands.begin (); it != commands.end (); ++it)
	{
		clientGrantCommand(*it);
	}

	return true;
}

//-----------------------------------------------------------------

bool CreatureObject::clientRevokeSkill       (const SkillObject & skill)
{
	if (m_skills.contains(&skill))
	{
		m_skills.erase(&skill);

		const SkillObject::StringVector & commands = skill.getCommandsProvided ();
		for (SkillObject::StringVector::const_iterator it = commands.begin (); it != commands.end (); ++it)
		{
			clientRevokeCommand(*it);
		}

		return true;
	}

	return false;
}

//----------------------------------------------------------------------

const CreatureObject::ExperiencePointMap & CreatureObject::getExperiencePointMap() const
{
	static const ExperiencePointMap npcMap;

	const PlayerObject * const owner = getPlayerObject();
	if (owner != NULL)
		return owner->getExperiencePointMap();
	return npcMap;
}

//----------------------------------------------------------------------

const std::map<std::pair<uint32,uint32>,int> & CreatureObject::getDraftSchematics() const
{
	static const std::map<std::pair<uint32,uint32>,int> npcSchematics;

	const PlayerObject * const owner = getPlayerObject();
	if (owner != NULL)
		return owner->getDraftSchematics();
	return npcSchematics;
}

//----------------------------------------------------------------------

void CreatureObject::clientGrantSchematic(uint32 serverCrc, uint32 sharedCrc)
{
	PlayerObject * const owner = getPlayerObject();
	if (owner != NULL)
		owner->clientGrantSchematic(serverCrc, sharedCrc);
}

//----------------------------------------------------------------------

void CreatureObject::clientRevokeSchematic(uint32 serverCrc, uint32 sharedCrc)
{
	PlayerObject * const owner = getPlayerObject();
	if (owner != NULL)
		owner->clientRevokeSchematic(serverCrc, sharedCrc);
}

//-----------------------------------------------------------------

void CreatureObject::clientGrantExp   (const std::string & exptype, int amount)
{
	PlayerObject * const owner = getPlayerObject();
	if (owner != NULL)
		owner->clientGrantExp(exptype, amount);
}

//----------------------------------------------------------------------

void CreatureObject::clientResetAttribute    (Attributes::Enumerator attrib, Attributes::Value cur, Attributes::Value max)
{
	max   = std::max (0, max);
	cur   = std::min (std::max (0, cur), max);

	m_attributes.set    (attrib, cur);
	m_maxAttributes.set (attrib, max);
	m_shockWounds       = 0;
}

//-----------------------------------------------------------------

bool CreatureObject::hasSkill (const SkillObject & skill) const
{
	return m_skills.contains(&skill);
}

//-----------------------------------------------------------------

bool CreatureObject::getExperience (const std::string & expName, int & result) const
{
	const PlayerObject * const owner = getPlayerObject();
	if (owner != NULL)
		return owner->getExperience (expName, result);
	else
	{
		result = 0;
		return false;
	}
}

//----------------------------------------------------------------------

int CreatureObject::getExperimentPoints() const
{
	const PlayerObject * const player = getPlayerObject();
	if (player != NULL)
		return player->getExperimentPoints();

	return 0;
}

//----------------------------------------------------------------------

Crafting::CraftingStage CreatureObject::getCraftingStage() const
{
	const PlayerObject * const player = getPlayerObject();
	if (player != NULL)
		return player->getCraftingStage();

	return Crafting::CS_none;
}

//----------------------------------------------------------------------

void CreatureObject::requestServerPostureChange (Postures::Enumerator posture)
{
	if (!Game::getSinglePlayer ())
	{
		//-- Handle multi-player case.
		if (Game::getPlayer () == this)
		{
			if (getController ())
				NON_NULL (safe_cast<CreatureController *> (getController ()))->requestServerPostureChange (posture);
		}
		else
			DEBUG_WARNING (true, ("attempting to request a server posture change [%s] for a non-authoratative creature on this client (id [%s], template [%s]).", Postures::getPostureName (posture), getNetworkId ().getValueString ().c_str (), getObjectTemplateName ()));
	}
	else
	{
		//-- Clear the chair-sitting state as necessary.
		if (posture != Postures::Sitting)
			setState (States::SittingOnChair, false);

		setVisualPosture (posture);
		m_serverPosture.set (posture);
	}
}

// ----------------------------------------------------------------------
/**
 * Set the posture for the object.
 *
 * ONLY call this when the client has been sent this posture as authoratative.
 * This is used to reflect a successful server-specified posture change.
 * (In single player, continue to call requestServerPostureChange() and the posture change
 * will still occur --- calling this function is unnecessary).
 *
 * @param posture  the posture that is in effect for this instance.
 */

void CreatureObject::setVisualPosture (Postures::Enumerator newPosture)
{
	Postures::Enumerator oldPosture = m_visualPosture.get();

	//-- Ignore posture changes to current posture.
	if (oldPosture == newPosture)
		return;

	//-- Warn when changing posture from dead or incapacitated to something else if it occurs during the dead/incap lockout period.  This shouldn't happen.
	if ( ((oldPosture == Postures::Dead) || (oldPosture == Postures::Incapacitated)) &&
	     ((newPosture != Postures::Dead) && (newPosture != Postures::Incapacitated)) )
	{
		DEBUG_WARNING(true, ("CreatureObject: !*! id [%s],template [%s] changing posture during incap/death lockout period from posture [%s] to [%s], this shouldn't happen.",
			getNetworkId().getValueString().c_str(), getObjectTemplateName(), Postures::getPostureName(oldPosture), Postures::getPostureName(newPosture)));

		if (ConfigClientGame::getEnablePostIncapDeathPostureChangeLockoutPeriod())
		{
			DEBUG_REPORT_LOG(true, ("  |^== Ignoring requested posture change during incap/death lockout.\n"));
			return;
		}
	}

	//-- Catch trying to revert from dead to anything else.
	if (oldPosture == Postures::Dead)
		DEBUG_WARNING(true, ("CreatureObject::setVisualPosture(): *!* id=[%s],ot=[%s] was dead, now trying to set posture [%s]!  This had better be a gm revive.", getNetworkId().getValueString().c_str(), getObjectTemplateName(), Postures::getPostureName(static_cast<Postures::Enumerator>(newPosture)) ));

	//-- Catch movement out of sitting posture.  In this case, we need to restore
	//   the player's authoratative scale.
	if ((oldPosture == Postures::Sitting) && (isSittingOnObject()))
	{
		GamePlaybackScript::standFromChair(*this);
		setSittingOnObject(false);
	}

	//-- if the creature is incapacitated, remove the ambient sound
	if (newPosture == Postures::Dead)
		Audio::detachSound (*this);

	m_visualPosture.set(newPosture);

	// Make creatures not collidable when they're in dead or incapacitated posture

	CollisionProperty * collision = getCollisionProperty();

	if(collision)
	{
		bool collidable = !((newPosture == Postures::Dead) || (newPosture == Postures::Incapacitated));

		collision->setCollidable(collidable);
	}
}

//----------------------------------------------------------------------

void CreatureObject::requestServerAttitudeChange (States::Enumerator attitude)
{
	if (attitude != States::CombatAttitudeEvasive &&
		attitude != States::CombatAttitudeNormal &&
		attitude != States::CombatAttitudeAggressive)
	{
		WARNING (true, ("setAttitude bad attitude %d", attitude));
		return;
	}

	if (!Game::getSinglePlayer () && Game::getPlayer () == this && getController ())
	{
		NON_NULL (safe_cast<CreatureController *> (getController ()))->requestServerAttitudeChange (attitude);
	}
	else
	{
		setState  (States::CombatAttitudeEvasive,    false);
		setState  (States::CombatAttitudeNormal,     false);
		setState  (States::CombatAttitudeAggressive, false);

		setState  (attitude,    true);
	}
}

//----------------------------------------------------------------------

States::Enumerator CreatureObject::getAttitude () const
{
	if (getState (States::CombatAttitudeEvasive))
		return States::CombatAttitudeEvasive;
	if (getState (States::CombatAttitudeAggressive))
		return States::CombatAttitudeAggressive;
	if (getState (States::CombatAttitudeNormal))
		return States::CombatAttitudeNormal;

	return States::Invalid;
}

// ----------------------------------------------------------------------

void CreatureObject::signalAnimationMoodModified (const std::string &newValue)
{
	//-- Get the skeletal appearance.
	SkeletalAppearance2 *const appearance = dynamic_cast<SkeletalAppearance2*>(getAppearance());
	if (appearance)
	{
		//-- Set the animation environment variable for mood.
		appearance->getAnimationEnvironment().getString(cs_moodAnimationVariableName) = CrcLowerString(newValue.c_str());
		DEBUG_REPORT_LOG(ms_logAnimationMoodChanges, ("animation mood: object id [%s] template [%s] set to mood [%s].\n", getNetworkId().getValueString().c_str(), getObjectTemplateName(), newValue.c_str()));
	}
	else
	{
		DEBUG_WARNING(ms_logAnimationMoodChanges, ("animation mood: object id [%s] template [%s] could not set to mood [%s] because not a skeletal appearance.", getNetworkId().getValueString().c_str(), getObjectTemplateName(), newValue.c_str()));
	}
}

// ======================================================================
// struct CreatureObject::AnimationMoodCallback: public member functions
// ======================================================================

void CreatureObject::Callbacks::AnimationMoodChanged::modified (CreatureObject &target, const std::string &, const std::string &newValue, bool) const
{
	target.signalAnimationMoodModified (newValue);
}

// ----------------------------------------------------------------------

void CreatureObject::Callbacks::AnimatingSkillDataChanged::modified (CreatureObject &target, const std::string &, const std::string &newValue, bool) const
{
	//-- Get the skeletal appearance.
	SkeletalAppearance2 *const appearance = dynamic_cast<SkeletalAppearance2*>(target.getAppearance());
	if (!appearance)
		return;

	CrcLowerString &skillVariable = appearance->getAnimationEnvironment().getString(AnimationEnvironmentNames::cms_animatingSkill);
	skillVariable.setString(newValue.c_str());
}

//----------------------------------------------------------------------

void CreatureObject::Callbacks::ScaleFactorChanged::modified (CreatureObject & target, const float & , const float & newValue, bool) const
{
	//-- Set the object scale.
	target.setScale (Vector (newValue, newValue, newValue));

	//-- Rebuild animation controllers since scale changes affect some cached data.
	SkeletalAppearance2 *const appearance = dynamic_cast<SkeletalAppearance2*> (target.getAppearance ());
	if (appearance)
	{
		appearance->getAnimationResolver ().recreateAnimationControllers (false);

		//-- Add game's animation message handler.
		GameAnimationMessageCallback::addCallback (target);
	}

	//-- Mounts: if the scale factor changed after we created the saddle, we need to update the saddle and rider scale.
	//   Cannot count on States::MountedCreature state since it might not yet have been set.
	target.doScaleChangeFixupsForMount();
}

//----------------------------------------------------------------------

void CreatureObject::Callbacks::StatesChanged::modified (CreatureObject & target, const uint64 &oldStates, const uint64 &newStates, bool) const
{
	Transceivers::statesChanged.emitMessage (target);

	//-- Check if mount has just been mounted or dismounted.
	bool const oldMountedCreatureState = (oldStates & States::getStateMask(States::MountedCreature)) != 0;
	bool const newMountedCreatureState = (newStates & States::getStateMask(States::MountedCreature)) != 0;

	if (oldMountedCreatureState ^ newMountedCreatureState)
	{
		// Old and new state differ.
		if (newMountedCreatureState)
			target.onMountRiderMounted();
		else
			target.onMountRiderDismounted();
	}

	//-- Check if we changed our glowing jedi state.
	bool const oldGlowingJediState = (oldStates & States::getStateMask(States::GlowingJedi)) != 0;
	bool const newGlowingJediState = (newStates & States::getStateMask(States::GlowingJedi)) != 0;

	if (oldGlowingJediState ^ newGlowingJediState)
	{
		Appearance *app = target.getAppearance();
		SkeletalAppearance2 *skelApp = dynamic_cast<SkeletalAppearance2 *>(app);
		if(skelApp)
			skelApp->setIsBlueGlowie(newGlowingJediState);
		if(newGlowingJediState)
		{
			static const ConstCharCrcLowerString c("appearance/pt_force_resist_disease.prt");
			ClientEffectManager::playClientEffect(c, &target, CrcLowerString::empty);
		}
			
	}

#ifdef _DEBUG
	if (&target == Game::getPlayerCreature())
	{
		bool const wasInCombat = oldStates & States::getStateMask(States::Combat);
		bool const inCombat = newStates & States::getStateMask(States::Combat);
		bool const enteringCombat = (inCombat && !wasInCombat);
		bool const exitingCombat = (!inCombat && wasInCombat);

		if (enteringCombat || exitingCombat)
		{
			Unicode::String date;
			CuiUtils::FormatDate(date, CuiUtils::GetSystemSeconds());

			DEBUG_REPORT_LOG(enteringCombat, ("**************** %s ENTERING COMBAT %s\n", Unicode::wideToNarrow(date).c_str(), target.getNetworkId().getValueString().c_str()));
			DEBUG_REPORT_LOG(exitingCombat, ("**************** %s EXITING COMBAT  %s\n", Unicode::wideToNarrow(date).c_str(), target.getNetworkId().getValueString().c_str()));
		}
	}
#endif // _DEBUG
}

//----------------------------------------------------------------------

void CreatureObject::Callbacks::VisualPostureChanged::modified (CreatureObject & target, const int8 & oldPosture, const int8 & newPosture, bool) const
{
	if (newPosture == Postures::Dead || oldPosture == Postures::Dead)
		target.setLocalizedNameDirty ();

	//-- Handle clearing the incap/dead lockout period for playback scripts if the player moves out of dead/incap into something else.
	if ( ((oldPosture == Postures::Dead) || (oldPosture == Postures::Incapacitated)) &&
	     ((newPosture != Postures::Dead) && (newPosture != Postures::Incapacitated)) )
	{
		DEBUG_REPORT_LOG(true, ("CreatureObject id [%s],template [%s] clearing incap/death interval because changing from posture [%s] to [%s].\n",
			target.getNetworkId().getValueString().c_str(), target.getObjectTemplateName(), Postures::getPostureName(oldPosture), Postures::getPostureName(newPosture)));
		target.setPlaybackScriptIsAssignedToIncapacitateMe(false);
	}

	Transceivers::visualPostureChanged.emitMessage (target);

	if (target.m_visualPosture.get() == target.m_serverPosture.get())
	{
		// Reset the time allowed for deviation between visual and server posture.
		target.m_revertToServerPostureTimer = cs_revertToServerPostureInterval;
	}

	DEBUG_REPORT_LOG(s_logVisualPostureChanges, ("posture change (visual): object id [%s], old posture [%s], new posture [%s].\n", target.getNetworkId().getValueString().c_str(), Postures::getPostureName(oldPosture), Postures::getPostureName(newPosture)));

	DEBUG_FATAL(newPosture < 0, ("Visual posture out of range: new value is [%d], old was [%d].", newPosture, oldPosture));
}

//----------------------------------------------------------------------

void CreatureObject::Callbacks::ServerPostureChanged::modified (CreatureObject & target, const int8 & oldPosture, const int8 & newPosture, bool) const
{
	if (newPosture != oldPosture)
	{
		// reset the timer for how long the client has before its posture should match the server's posture.
		target.m_revertToServerPostureTimer = cs_revertToServerPostureInterval;
		Transceivers::serverPostureChanged.emitMessage (target);
	}

	DEBUG_REPORT_LOG(s_logVisualPostureChanges, ("posture change (server): object id [%s], old posture [%s], new posture [%s].\n", target.getNetworkId().getValueString().c_str(), Postures::getPostureName(oldPosture), Postures::getPostureName(newPosture)));

	DEBUG_FATAL(newPosture < 0, ("Server posture out of range: new value is [%d], old was [%d].", newPosture, oldPosture));
}

//----------------------------------------------------------------------

void CreatureObject::Callbacks::LookAtTargetChanged::modified (CreatureObject & target, const CachedNetworkId & oldId, const CachedNetworkId & newId, bool) const
{
	//-- don't let the server clobber our lookat target when switching targets quickly
	//-- this is required because the client updates its lookAtTarget immediately without
	//-- waiting for the server to do it

	if (ms_lastLookAtTargetRequest == newId)
	{
		// Check to see if the player is at a ship station.
		if (&target == Game::getPlayerCreature() && Game::isHudSceneTypeSpace())
		{
			// You need the player ship to prevent triggering effects from other ships.
			ShipObject const * const playerShip = Game::getPlayerContainingShip();
			if (playerShip)
			{
				if (newId.getObject() == &target)
				{
					ShipObject::enableTargetAppearanceFromObjectId(playerShip, newId, false, static_cast<int>(ShipChassisSlotType::SCST_invalid));
					ShipObject::enableTargetAppearanceFromObjectId(playerShip, oldId, false, static_cast<int>(ShipChassisSlotType::SCST_invalid));
				}
				else
				{
					ShipObject::enableTargetAppearanceFromObjectId(playerShip, newId, true, static_cast<int>(target.getLookAtTargetSlot()));
					ShipObject::enableTargetAppearanceFromObjectId(playerShip, oldId, false, static_cast<int>(ShipChassisSlotType::SCST_invalid));
				}
			}
		}

		Transceivers::lookAtTargetChanged.emitMessage (target);
	}
	else
	{
		target.setLookAtTarget (ms_lastLookAtTargetRequest);
	}
}

//----------------------------------------------------------------------

void CreatureObject::Callbacks::IntendedTargetChanged::modified (CreatureObject & target, const CachedNetworkId & oldId, const CachedNetworkId & newId, bool) const
{
	//-- don't let the server clobber our lookat target when switching targets quickly
	//-- this is required because the client updates its lookAtTarget immediately without
	//-- waiting for the server to do it

	if (ms_lastIntendedTargetRequest == newId)
	{
		// Check to see if the player is at a ship station.
		if (&target == Game::getPlayerCreature() && Game::isHudSceneTypeSpace())
		{
			// You need the player ship to prevent triggering effects from other ships.
			ShipObject const * const playerShip = Game::getPlayerContainingShip();
			if (playerShip)
			{
				if (newId.getObject() == &target)
				{
					ShipObject::enableTargetAppearanceFromObjectId(playerShip, newId, false, static_cast<int>(ShipChassisSlotType::SCST_invalid));
					ShipObject::enableTargetAppearanceFromObjectId(playerShip, oldId, false, static_cast<int>(ShipChassisSlotType::SCST_invalid));
				}
				else
				{
					ShipObject::enableTargetAppearanceFromObjectId(playerShip, newId, true, static_cast<int>(target.getLookAtTargetSlot()));
					ShipObject::enableTargetAppearanceFromObjectId(playerShip, oldId, false, static_cast<int>(ShipChassisSlotType::SCST_invalid));
				}
			}
		}

		Transceivers::intendedTargetChanged.emitMessage (target);
	}
	else
	{
		target.setIntendedTarget (ms_lastIntendedTargetRequest);
	}
}

//----------------------------------------------------------------------

void CreatureObject::Callbacks::CurrentWeaponChanged::modified (CreatureObject & target, const CachedNetworkId & oldId, const CachedNetworkId & newId, bool) const
{
	UNREF(oldId);
	UNREF(newId);

	PlayerObject * player = target.getPlayerObject();
	if(player)
	{
		Transceivers::currentWeaponChanged.emitMessage(target);
		target.setDupedCreaturesDirty(true);
	}
}

//----------------------------------------------------------------------

void CreatureObject::Callbacks::GroupChanged::modified (CreatureObject & target, const CachedNetworkId & oldGroup, const CachedNetworkId & newGroup, bool) const
{
	GroupManager::handleGroupChange(target, oldGroup, newGroup);
	if (Game::getPlayer () == &target && oldGroup != newGroup)
		Transceivers::groupChanged.emitMessage (target);
}

//----------------------------------------------------------------------

void CreatureObject::Callbacks::PerformanceTypeChanged::modified (CreatureObject &target, const int &old, const int &value, bool local) const
{
	UNREF(old);
	UNREF(local);

	if (value > 0)
	{
		PlayerMusicManager::startPerformance(target);
	}
	else if (value == 0)
	{
		PlayerMusicManager::stopPerformance(target);
	}
}

// ----------------------------------------------------------------------

void CreatureObject::Callbacks::CoverVisibilityChanged::modified(CreatureObject & target, const bool & , const bool & isVisible, bool) const
{
	CreatureObject const * const player = Game::getPlayerCreature();
	bool const isLocalPlayer = (player == &target);
	target.setCoverVisibility(isVisible, isLocalPlayer);
}

// ----------------------------------------------------------------------

void CreatureObject::Callbacks::HologramTypeChanged::modified(CreatureObject &target, const int32 &old, const int32 &value, bool local) const 
{
	UNREF(old);
	UNREF(local);
	int32 holoType = value;
	SkeletalAppearance2 * const skeletalAppearance = dynamic_cast<SkeletalAppearance2 * const>(target.getAppearance());
	if (skeletalAppearance)
	{
		if (holoType < SkeletalAppearance2::HT_none || holoType >= SkeletalAppearance2::HT_num_types)
		{
			DEBUG_WARNING(true,("CreatureObject::Callbacks::HologramTypeChanged::modified - value (%d) outside range\n",value));
			holoType = SkeletalAppearance2::HT_none;
		}
		skeletalAppearance->setHologramType(static_cast<SkeletalAppearance2::HologramType>(holoType));
	}
}

void CreatureObject::Callbacks::VisibleOnMapAndRadarChanged::modified(CreatureObject &target, const bool &old, const bool &value, bool local) const 
{
	UNREF(target);
	UNREF(old);
	UNREF(value);
	UNREF(local);
	
	// not needed yet..
}


// ----------------------------------------------------------------------

bool CreatureObject::isTargettable() const
{
	if (!m_coverVisibility.get() && isPassiveRevealPlayerCharacter(Game::getPlayerNetworkId()) && TangibleObject::isTargettable())
		return true;

	return m_coverVisibility.get() && TangibleObject::isTargettable();
}

// ----------------------------------------------------------------------

void CreatureObject::Callbacks::AlternateAppearanceSharedObjectTemplateNameChanged::modified (CreatureObject &target, const std::string &oldTemplateName, const std::string &newTemplateName, bool) const
{
	//-- Create the appearance using the appropriate object template's appearance: it could be the default or alternate.
	if (oldTemplateName != newTemplateName)
	{
		std::string const none("none");
		if(newTemplateName.find(none) != std::string::npos  && newTemplateName.size() == none.size())
			target.useDefaultAppearance();
		else
		{

			if(target.getPerformanceType() != 0)
				PlayerMusicManager::stopPerformance(target);

			target.setVisualPosture(Postures::Upright);

			if (Game::getPlayerCreature() == &target)
			{
				target.requestServerPostureChange(Postures::Upright);
			}

			CreatureController *const controller = dynamic_cast<CreatureController*> (target.getController ());
			if (controller)
			{
				const bool skipTransition = true;
				const bool skipWithDelay  = false;


				controller->setHeldItemAnimationStateId(NULL);
				*target.m_heldStateObjectWatcher = NULL;
				target.m_hasHeldStateObject      = false;
				controller->setControllerToCurrentState(false, true);
				controller->changeAnimationStatePath (target.getVisualPosture(), skipTransition, skipWithDelay);

			}

			target.setAlternateAppearance(newTemplateName.c_str());
		}
	}
}

//----------------------------------------------------------------------

void CreatureObject::skillsOnChanged          ()
{
	Transceivers::skillsChanged.emitMessage (*this);
}

//----------------------------------------------------------------------

void CreatureObject::skillsOnErase            (const unsigned , const SkillObject * &)
{
}

//----------------------------------------------------------------------

void CreatureObject::skillsOnInsert           (const unsigned , const SkillObject * &)
{
}

//----------------------------------------------------------------------

void CreatureObject::skillsOnSet              (const unsigned , const SkillObject * &, const SkillObject * &)
{
}

//----------------------------------------------------------------------

void CreatureObject::wearablesOnErase (const unsigned , const WearableEntry &w)
{
	if (this != Game::getPlayer ())
	{
		SlottedContainer * container = ContainerInterface::getSlottedContainer(*this);
		NOT_NULL(container);

		ClientObject* clientObject = safe_cast<ClientObject*>(NetworkIdManager::getObjectById(w.m_networkId));
		if (clientObject)
		{
			Container::ContainerErrorCode tmp = Container::CEC_Success;
			IGNORE_RETURN(container->remove(*clientObject, tmp));
			delete clientObject;
		}

		this->setDupedCreaturesDirty(true);
	}
}

//----------------------------------------------------------------------

void CreatureObject::wearablesOnInsert (const unsigned , const WearableEntry &w)
{
	if (this != Game::getPlayer ())
	{
		//Set up streamed wearables for non-player creatures
		SlottedContainer * container = ContainerInterface::getSlottedContainer(*this);
		NOT_NULL(container);

		Container::ContainerErrorCode tmp = Container::CEC_Success;

		ConstCharCrcString cc = ObjectTemplateList::lookUp(w.m_objectTemplate);
		if (cc.isEmpty())
		{
			WARNING(true, ("SceneCreateObjectByCrc crc %08x was not found in object template list", cc.getCrc()));
			return;
		}


		TangibleObject* const wearable = safe_cast<TangibleObject*> (ObjectTemplate::createObject (cc.getString()));
		if (wearable)
		{
			wearable->setNetworkId(w.m_networkId);
			wearable->setAppearanceData(w.m_appearanceString);
			if (w.m_weaponSharedBaselines.get() != NULL)
				wearable->applyBaselines(*w.m_weaponSharedBaselines);
			if (w.m_weaponSharedNpBaselines.get() != NULL)
				wearable->applyBaselines(*w.m_weaponSharedNpBaselines);
			wearable->endBaselines();
			IGNORE_RETURN(container->add(*wearable, w.m_arrangement, tmp));
		}
		else
		{
			DEBUG_WARNING(true, ("Failed to create wearable %s", cc.getString()));
		}

		this->setDupedCreaturesDirty(true);
	}
}

//----------------------------------------------------------------------

void CreatureObject::wearablesOnChanged ()
{
	std::vector<WearableEntry> wearables = m_wearableData.get();
	for(std::vector<WearableEntry>::iterator i = wearables.begin(); i != wearables.end(); ++i)
	{
		Object * const o = NetworkIdManager::getObjectById(i->m_networkId);
		ClientObject * const co = o ? o->asClientObject() : NULL;
		TangibleObject * const wearable = co ? co->asTangibleObject() : NULL;
		if(wearable)
		{
			wearable->setAppearanceData(i->m_appearanceString);
		}
	}

	this->setDupedCreaturesDirty(true);
}

//----------------------------------------------------------------------

void CreatureObject::buffsOnInsert(const uint32 &, const Buff::PackedBuff &)
{
	Transceivers::buffsChanged.emitMessage (*this);
}

//----------------------------------------------------------------------

void CreatureObject::buffsOnErase(const uint32 &, const Buff::PackedBuff &)
{
	Transceivers::buffsChanged.emitMessage (*this);
}

//----------------------------------------------------------------------

void CreatureObject::buffsOnSet(const uint32 &, const Buff::PackedBuff &, const Buff::PackedBuff &)
{
	Transceivers::buffsChanged.emitMessage (*this);
}

//----------------------------------------------------------------------

void CreatureObject::maxAttributesOnChanged()
{
	Transceivers::maxAttributesChanged.emitMessage (*this);
}

//----------------------------------------------------------------------

void CreatureObject::skillModsOnSet (const std::string &, const std::pair<int, int> &, const std::pair<int, int> &)
{
	Transceivers::skillModsChanged.emitMessage (*this);
}

//----------------------------------------------------------------------

void CreatureObject::skillModsOnInsert        (const std::string &, const std::pair<int, int> &)
{
	Transceivers::skillModsChanged.emitMessage (*this);
}

//----------------------------------------------------------------------

void CreatureObject::skillModsOnErase         (const std::string &, const std::pair<int, int> &)
{
	Transceivers::skillModsChanged.emitMessage (*this);
}

//----------------------------------------------------------------------

void CreatureObject::attributesOnSet (const size_t elem, const Attributes::Value & oldValue, const Attributes::Value & newValue)
{
	if (newValue > oldValue)
		return;

	Attributes::Value const diff = oldValue - newValue;

	//-- add the positive difference

	if (elem == Attributes::Health)
		m_accumulatedHamDamage = static_cast<int16>(m_accumulatedHamDamage + diff);
}

// ----------------------------------------------------------------------
void CreatureObject::setTargetUnderCursor (const NetworkId& id)
{
	if (Game::getPlayer() == this)
	{
		m_targetUnderCursor = id;
	}
}
// ----------------------------------------------------------------------

void CreatureObject::setLookAtTarget (const NetworkId& id)
{
	if (Game::getPlayer () == this)
	{
		if (!Game::isSpace() 
			&& ms_lastLookAtTargetRequest != NetworkId::cms_invalid && id == NetworkId::cms_invalid)
		{
			ClientCommandQueue::enqueueCommand("target", NetworkId::cms_invalid, Unicode::narrowToWide("none"));
		}

		ms_lastLookAtTargetRequest = id;
		
		if (getController () && getLookAtTarget() != id)
		{
			NON_NULL(safe_cast<CreatureController *> (getController ()))->setLookAtTarget (id);
		}
	}

	//-- immediately set the lookAtTarget on the client, since setting it on the server should never fail
	m_lookAtTarget.set(CachedNetworkId (id));

	//-- If you select a new target, clear the slot.
	setLookAtTargetSlot(ShipChassisSlotType::SCST_invalid);

	ShipObject * const ship = Game::getPlayerPilotedShip();
	if(ship)
		ship->setPilotLookAtTarget(id);
}

// ----------------------------------------------------------------------

void CreatureObject::setIntendedTarget (const NetworkId& id)
{
	if (Game::getPlayer () == this)
	{
		if (!Game::isSpace() 
			&& ms_lastIntendedTargetRequest != NetworkId::cms_invalid && id == NetworkId::cms_invalid)
		{
			ClientCommandQueue::enqueueCommand("intendedTarget", NetworkId::cms_invalid, Unicode::emptyString);
		}

		ms_lastIntendedTargetRequest = id;

		if (getIntendedTarget() != id)
		{
			if (CuiPreferences::getAutoAimToggle())
				GroundCombatActionManager::attemptAction(GroundCombatActionManager::AT_cancelRepeatPrimaryAttack);

			if (getController ())
			{
				NON_NULL(safe_cast<CreatureController *> (getController ()))->setIntendedTarget (id);
			}
		}
	}

	//-- immediately set the lookAtTarget on the client, since setting it on the server should never fail
	m_intendedTarget.set(CachedNetworkId (id));
}

//----------------------------------------------------------------------

void CreatureObject::setLookAtAndIntendedTarget(const NetworkId& id)
{
	setLookAtTarget(id);
	setIntendedTarget(id);
}

//----------------------------------------------------------------------

/**
 * Returns my target's target.
 */
const CachedNetworkId & CreatureObject::getAssistTarget() const
{
	// TPERRY - changed this method to only return the combat target (which only valid when in combat)
	//  since you should only be assisting an object in combat
	const CachedNetworkId & lookAtTarget = getLookAtTarget();
	CreatureObject const * const lookAtCreatureObject = CreatureObject::asCreatureObject(lookAtTarget.getObject());
	if (lookAtCreatureObject)
	{
		return lookAtCreatureObject->getCombatTarget();
	}

	return CachedNetworkId::cms_cachedInvalid;
}

//----------------------------------------------------------------------

void CreatureObject::requestServerSetMood    (uint32 mood)
{
	if (!Game::getSinglePlayer () && Game::getPlayer () == this && getController ())
		NON_NULL (safe_cast<CreatureController *> (getController ()))->setMood (mood);
	else
		clientSetMood (mood);
}

//----------------------------------------------------------------------

void CreatureObject::clientSetMood(const uint32 mood)
{
	m_mood.set (static_cast<uint8>(mood));
}

//----------------------------------------------------------------------

void CreatureObject::clientSetMoodTemporary (const uint32 mood, float duration)
{
	if (!m_temporaryMoodActive)
		m_oldMood = m_mood.get ();

	m_temporaryMoodActive   = true;
	m_temporaryMoodDuration = duration;

	m_settingTemporaryMood  = true;
	clientSetMood (mood);
	m_settingTemporaryMood  = false;
}

//----------------------------------------------------------------------

uint32  CreatureObject::getMood() const
{
	if (m_temporaryMoodActive)
		return m_oldMood;

	return static_cast<uint32>(m_mood.get());
}

//----------------------------------------------------------------------

void CreatureObject::setScaleFactor (float f)
{
	//-- Clamp the scale.
	f = std::max (cs_minimumScaleFactor, f);

	//-- Keep track of the scale factor.
	m_scaleFactor = f;
}

// ----------------------------------------------------------------------
/**
 * Determines whether an appearance is wearable on a creature.
 */

bool CreatureObject::isAppearanceWearable(Object const &object) const
{
	return isAppearanceWearable(object, NULL);
}

// ----------------------------------------------------------------------

bool CreatureObject::isAppearanceWearable(Object const &object, std::string *appearancePath, bool * const prohibitAttachmentVisuals ) const
{
	UNREF( prohibitAttachmentVisuals );
	
	bool result = true;
	bool clearAppearancePath = false;
	bool useCurrentAppearancePath = false;
	const SharedCreatureObjectTemplate::Gender gender = static_cast<SharedCreatureObjectTemplate::Gender>(getGender());

	// Make sure this object has a valid appearance

	if (object.getAppearance() == NULL)
	{
		clearAppearancePath = true;
		result = false;
	}
	else if ((gender != SharedCreatureObjectTemplate::GE_male) &&
	         (gender != SharedCreatureObjectTemplate::GE_female))
	{
		useCurrentAppearancePath = true;
	}
	else
	{
		// Make sure this is a gender/species that can switch appearances

		SharedCreatureObjectTemplate::Species species = static_cast<SharedCreatureObjectTemplate::Species> (getSpecies());

		int column = -1;

		// Get the species index

		switch (species)
		{
			default:
			case SharedCreatureObjectTemplate::SP_human:       { column =  1; } break;
			case SharedCreatureObjectTemplate::SP_rodian:      { column =  3; } break;
			case SharedCreatureObjectTemplate::SP_monCalamari: { column =  5; } break;
			case SharedCreatureObjectTemplate::SP_wookiee:     { column =  7; } break;
			case SharedCreatureObjectTemplate::SP_twilek:      { column =  9; } break;
			case SharedCreatureObjectTemplate::SP_trandoshan:  { column = 11; } break;
			case SharedCreatureObjectTemplate::SP_zabrak:      { column = 13; } break;
			case SharedCreatureObjectTemplate::SP_bothan:      { column = 15; } break;
			case SharedCreatureObjectTemplate::SP_ithorian:    { column = 17; } break;
			case SharedCreatureObjectTemplate::SP_sullustan:   { column = 19; } break;
		} //lint !e788 //enums not used

		// Add in the gender index

		switch (gender)
		{
			case SharedCreatureObjectTemplate::GE_male:   { } break;
			case SharedCreatureObjectTemplate::GE_female: { column += 1; } break;
			default: { } break;
		}

		if (column > 0)
		{
			// See if there is a new appearance we can set, otherwise don't change anything

			std::string objectTemplateName(object.getObjectTemplateName());

			if (!objectTemplateName.empty())
			{
				// Strip out just the object template name

				size_t const slashIndex = objectTemplateName.rfind('/');
				size_t const dotIndex   = objectTemplateName.rfind('.');

				std::string fileName;

				if (slashIndex == std::string::npos)
				{
					fileName = objectTemplateName.substr(0, dotIndex);
				}
				else if (dotIndex == std::string::npos)
				{
					fileName = objectTemplateName.substr(slashIndex + 1);
				}
				else
				{
					fileName = objectTemplateName.substr(slashIndex + 1, dotIndex - slashIndex - 1);
				}
				if (AppearanceManager::isAppearanceManaged(fileName))
				{
					std::string destAppearancePath;
					IGNORE_RETURN(AppearanceManager::getAppearanceName(destAppearancePath, fileName, column));

					if ( destAppearancePath == AppearanceTag::tagBlock )
					{
						clearAppearancePath = true;
						result = false;
					}
					else if ( destAppearancePath == AppearanceTag::tagDefault )
					{
						// Default to the human male or female appearance

						switch (gender)
						{
							case SharedCreatureObjectTemplate::GE_male:
								{
									if (appearancePath != NULL)
									{
										IGNORE_RETURN(AppearanceManager::getAppearanceName(*appearancePath, fileName, 1));
									}
								}
								break;
							case SharedCreatureObjectTemplate::GE_female:
								{
									if (appearancePath != NULL)
									{
										IGNORE_RETURN(AppearanceManager::getAppearanceName(*appearancePath, fileName, 2));
									}
								}
								break;
							default:
								{
									// We have shuttles and other objects that have gender set to GE_other: this is not an error condition.
									clearAppearancePath = true;
									result = false;
								}
								break;
						}
					}
					else if ( destAppearancePath == AppearanceTag::tagHide )
					{
						if ( prohibitAttachmentVisuals )
						{
							*prohibitAttachmentVisuals = true;
						}
					}
					else if ( !destAppearancePath.empty() && destAppearancePath[0] == ':' )
					{
						/* invalid tag in appearance table */
						FATAL( true, ( "invalid tag '%s' in appearance table\n", destAppearancePath.c_str() ) );
					}
					else if ( appearancePath )
					{
						*appearancePath = destAppearancePath;
					}


				}
				else
				{
					useCurrentAppearancePath = true;
				}

			}
			else
			{
				useCurrentAppearancePath = true;
			}
		}
		else
		{
			useCurrentAppearancePath = true;
		}
	}

	if (appearancePath != NULL)
	{
		if (clearAppearancePath)
		{
			// Clear the result appearance path

			if (!appearancePath->empty())
			{
				appearancePath->clear();
			}
		}
		else if (useCurrentAppearancePath)
		{
			// Use the current appearance (no change)

			*appearancePath = object.getAppearance()->getAppearanceTemplate()->getName();
		}
	}

	return result;
}

//----------------------------------------------------------------------

bool CreatureObject::getState(int8 state) const
{
	if (state == States::Combat)
	{
		return isInCombat();
	}

	return (m_states.get() & States::getStateMask(static_cast<States::Enumerator>(state))) != 0;
}

//----------------------------------------------------------------------

void CreatureObject::setState(States::Enumerator whichState, bool b)
{
	if(whichState == States::Combat)
	{
		setIsInCombat(b);
		return;
	}

	if (b)
		setStates(m_states.get() | States::getStateMask(whichState));
	else
		setStates(m_states.get() & ~States::getStateMask(whichState));
}

//-----------------------------------------------------------------------

const int CreatureObject::getModValue(const std::string & modName, bool capped) const
{
	int result = 0;
	const SkillModMap::const_iterator f = m_modMap.find(modName);
	if(f != m_modMap.end())
		result = (*f).second.first;
	if (capped && result > ConfigSharedGame::getMaxCreatureSkillModBonus())
		result = ConfigSharedGame::getMaxCreatureSkillModBonus();
	return result;
}

//----------------------------------------------------------------------

void CreatureObject::getAccumulatedHamDamage    (int & health, int & action, int & mind) const
{
	health = m_accumulatedHamDamage;
	action = 0;
	mind   = 0;
}

// ----------------------------------------------------------------------
/**
 * Set the appropriate "held item" state for the animation system
 * based on the contents of the hands.
 */

void CreatureObject::setAppearanceHeldItemState ()
{
	m_hasHeldStateObject      = false;
	*m_heldStateObjectWatcher = 0;

	//-- Check if there is anything in the "holding" slots for the container.  If so, set the Object's hold item state
	//   to the appropriate state for the held item.
	CreatureController *const creatureController = dynamic_cast<CreatureController*>(getController());
	if (creatureController)
	{
		const SlottedContainer *const slottedContainer = ContainerInterface::getSlottedContainer(*this);
		if (slottedContainer)
		{
			//-- Get the held object.
			Object *heldItemObject = 0;

			// Check right/left hold position.
			Container::ContainerErrorCode tmp = Container::CEC_Success;
			if (slottedContainer->hasSlot(s_holdRightSlotId))
			{
				CachedNetworkId  heldNetworkId = slottedContainer->getObjectInSlot(s_holdRightSlotId, tmp);
				if (tmp == Container::CEC_Success)
					heldItemObject = heldNetworkId.getObject();
				else if (tmp != Container::CEC_NotFound)
				{
					DEBUG_WARNING(true, ("setAppearanceHeldItemState(): creature id [%s] reports having slot [%s] but failed with container error code [%d] when calling getObjectInSlot(); animation state related to held items cannot be set properly.", getNetworkId().getValueString().c_str(), SlotIdManager::getSlotName(s_holdRightSlotId).getString(), static_cast<int>(tmp)));
#ifdef _DEBUG
					slottedContainer->debugLog();
#endif
				}
			}
			else if (slottedContainer->hasSlot(s_holdLeftSlotId))
			{
				CachedNetworkId  heldNetworkId = slottedContainer->getObjectInSlot(s_holdLeftSlotId, tmp);
				if (tmp == Container::CEC_Success)
					heldItemObject = heldNetworkId.getObject();
				else if (tmp != Container::CEC_NotFound)
				{
					DEBUG_WARNING(true, ("setAppearanceHeldItemState(): creature id [%s] reports having slot [%s] but failed with container error code [%d] when calling getObjectInSlot(); animation state related to held items cannot be set properly.", getNetworkId().getValueString().c_str(), SlotIdManager::getSlotName(s_holdLeftSlotId).getString(), static_cast<int>(tmp)));
#ifdef _DEBUG
					slottedContainer->debugLog();
#endif
				}
			}

			//-- Get the appearance template name for the held item.
			const char *heldItemAppearanceTemplateName = 0;
			if (heldItemObject)
			{
				//-- Object is holding something.  Modify held state for animation purposes.
				Appearance *const heldAppearance = heldItemObject->getAppearance();
				if (heldAppearance)
				{
					const AppearanceTemplate *const heldAppearanceTemplate = heldAppearance->getAppearanceTemplate();
					if (heldAppearanceTemplate)
						heldItemAppearanceTemplateName = heldAppearanceTemplate->getCrcName().getString();
				}
			}

			//-- If there's a held item, lookup and map held item's appearance template name to the associated animation state id.
			AnimationStateNameId  heldItemStateId;
			if (heldItemAppearanceTemplateName && AnimationHeldItemMapper::mapHeldItem(heldItemAppearanceTemplateName, heldItemStateId))
			{
				// Container object is both holding something and the held item has an associated hold state with it.
				// Tell the object controller about it so the animation state path contains the appropriate data.
				creatureController->setHeldItemAnimationStateId(&heldItemStateId);

				// Keep track of the object so we can catch when the object is destroyed.
				*m_heldStateObjectWatcher = heldItemObject;
				m_hasHeldStateObject      = true;
			}
			else
			{
				// Container object is not holding anything or is holding something that doesn't have a mapped entry.
				// Tell the object controller about it so the animation state path contains the appropriate data.
				creatureController->setHeldItemAnimationStateId(0);
			}
		}
	}
}

// ----------------------------------------------------------------------

void CreatureObject::setAppearanceTransformModifiers()
{
	m_isAiming = false;

	//-- Get the skeletal appearance.
	Appearance *const baseAppearance = getAppearance();
	if (!baseAppearance)
		return;

	SkeletalAppearance2 *const appearance = baseAppearance->asSkeletalAppearance2();
	if (!appearance)
		return;

	//-- Clear transform modifiers on the appearance.
	appearance->clearAllTransformModifiers();

	//-- Add appropriate transform modifiers.
	if (isInCombat()) 
	{
		// Get a tangible object.
		Object const * const oTarget = appearance->getTargetObject();
		ClientObject const * const coTarget = oTarget ? oTarget->asClientObject() : NULL;
		TangibleObject const * const toTarget = coTarget ? coTarget->asTangibleObject() : NULL;
		if (toTarget)
		{
			// Check for dead or incapacitated creatures.
			CreatureObject const * const croTarget = toTarget->asCreatureObject();
			if (!croTarget || (!croTarget->isIncapacitated() && !croTarget->isDead() && !croTarget->isInCombat()))
			{
				if (m_hasHeldStateObject)
				{
					Object const *const heldObject = *m_heldStateObjectWatcher;
					if (heldObject)
					{
						SharedWeaponObjectTemplate const *const objectTemplate = dynamic_cast<SharedWeaponObjectTemplate const*>(heldObject->getObjectTemplate());
						if (objectTemplate)
						{
							//@TODO - This code is bogus, and as a result, the whole m_isAiming is bogus.  A lot of code should be removed.
							//m_isAiming = (objectTemplate->getAttackType() == SharedWeaponObjectTemplate::AT_ranged);
						}
					}

					if (m_isAiming)
					{
						appearance->addTransformModifierTakeOwnership(cs_targetPitchJointName, new TargetPitchTransformModifier);
					}
				}
			}
		}
	}

	//-- Add look-at target modifier for anything that has the all_b.skt skeleton.
	SkeletalAppearanceTemplate const *const appearanceTemplate = safe_cast<SkeletalAppearanceTemplate const*>(appearance->getAppearanceTemplate());
	if (appearanceTemplate)
	{
		bool applyLookAtModifier = false;
		
		int const skeletonTemplateCount = appearanceTemplate->getSkeletonTemplateCount();
		for (int i = 0; i < skeletonTemplateCount; ++i)
		{
			if ( (appearanceTemplate->getSkeletonTemplateName(appearanceTemplate->getSkeletonTemplateInfo(i)) == cs_humanoidSkeletonTemplateName) ||
				(appearanceTemplate->getSkeletonTemplateName(appearanceTemplate->getSkeletonTemplateInfo(i)) == cs_ithorianSkeletonTemplateName) )
			{
				applyLookAtModifier = true;
				break;
			}
		}

		if(isRidingMount())
			applyLookAtModifier = false;
		
		if (applyLookAtModifier)
		{
			LookAtTransformModifier *const masterModifier = new LookAtTransformModifier();
			LookAtTransformModifier *const slaveModifier  = new LookAtTransformModifier();
			
			slaveModifier->setMaster(masterModifier);
			
			appearance->addTransformModifierTakeOwnership(cs_lookAtMasterJointName, masterModifier);
			appearance->addTransformModifierTakeOwnership(cs_lookAtSlaveJointName,  slaveModifier);
		}
	}

	//-- If riding a mount or at a ship station, remove the translation from the root node so it appears as if the root
	//   is snapping to the object position.
	if (isRidingMount() || getShipStation() != ShipStation::ShipStation_None)
	{
		appearance->addTransformModifierTakeOwnership(cs_rootJointName, new ZeroTranslationTransformModifier());

		const CreatureObject * const mount = getMountedCreature ();
		if (mount)
		{
			if (mount->getGameObjectType () != SharedObjectTemplate::GOT_vehicle_hover)
			{
				Vector const worldUpInSpineJointSpace = Vector::unitY;
				if(isRidingMount())
				{
					if(getSpecies() == static_cast<int>(SharedCreatureObjectTemplate::SP_ithorian))
						appearance->addTransformModifierTakeOwnership(cs_ithFaceBoneJointName,  new RiderSpineTransformModifier(worldUpInSpineJointSpace));
					else
						appearance->addTransformModifierTakeOwnership(cs_humanFaceBoneJointName, new RiderSpineTransformModifier(worldUpInSpineJointSpace));
				}
				else
					appearance->addTransformModifierTakeOwnership(cs_spineJointName, new RiderSpineTransformModifier(worldUpInSpineJointSpace));
			}
		}
	}
}

// ----------------------------------------------------------------------

void CreatureObject::updateRidingMountStatus()
{
	m_isRidingMount = (getMountedCreature() != NULL);
}

// ----------------------------------------------------------------------
/**
 * Return the alternate shared object template if set or the original shared object template.
 *
 * This call fetches a reference count for the caller.  The caller is responsible for
 * releasing the reference when the caller is done using the object template.
 */

SharedCreatureObjectTemplate const *CreatureObject::fetchEffectiveObjectTemplate () const
{
	if (m_alternateSharedCreatureObjectTemplate)
	{
		m_alternateSharedCreatureObjectTemplate->addReference();
		return m_alternateSharedCreatureObjectTemplate;
	}
	else
	{
		// Get this object's constructor-time object template.
		SharedCreatureObjectTemplate const *objectTemplate = safe_cast<SharedCreatureObjectTemplate const*>(getObjectTemplate());
		if (!objectTemplate)
			return NULL;

		// Fetch a reference to obey the contract of this function.
		objectTemplate->addReference();
		return objectTemplate;
	}
}

// ----------------------------------------------------------------------

void CreatureObject::updateShipStation()
{
	// This function needs to consider a creature to be in a particular station
	// if and only if the both the creature says it is in an arrangement which
	// takes up the correct slot for that station, and the container has that
	// slot, so we have a consistant view during the 2-phase update of the
	// container/arrangement relationship.
	Object const *containingObject = ContainerInterface::getContainedByObject(*this);
	if (containingObject)
	{
		SlottedContainer const *slottedContainer = ContainerInterface::getSlottedContainer(*containingObject);
		if (slottedContainer)
		{
		  SlottedContainmentProperty const *slottedContainmentProperty = ContainerInterface::getSlottedContainmentProperty(*this);
			if (slottedContainmentProperty)
			{
				int currentArrangement = slottedContainmentProperty->getCurrentArrangement();
				if (currentArrangement != -1)
				{
					SlottedContainmentProperty::SlotArrangement const &slotArrangement = slottedContainmentProperty->getSlotArrangement(currentArrangement);
					for (SlottedContainmentProperty::SlotArrangement::const_iterator i = slotArrangement.begin(); i != slotArrangement.end(); ++i)
					{
						int const shipStation = getShipStationForSlot(*i);
						if (shipStation != ShipStation::ShipStation_None && slottedContainer->hasSlot(*i))
						{
							m_shipStation = static_cast<int16>(shipStation);
							return;
						}
					}
				}
			}
		}
	}

	m_shipStation = ShipStation::ShipStation_None;
}

// ----------------------------------------------------------------------

void CreatureObject::onRiderMountedMount()
{
	// Just got on a mount.

	// Detach the rider from whatever it currently is attached to --- we'll handle
	// attachment to the proper object here.
	if (getAttachedTo())
		detachFromObject(Object::DF_none);

	// Since a mount's container exposes its contents to the world,
	// the client container code will not alter the mount.  Add
	// the player as a child of a HardpointObject attached to the mount.

	//-- Attach saddle wearable and saddle static mesh, return saddle static mesh object.
	CreatureObject *const mountCreature = getMountedCreature();
	if (!mountCreature)
	{
		DEBUG_WARNING(true, ("CreatureObject::onRiderMountedMount(): reported that we just mounted but mount creature is NULL, rider id=[%s],template=[%s].", getNetworkId().getValueString().c_str(), getObjectTemplateName()));
		return;
	}

	//-- Move the mount to the player's position if the player is authoritative on this client.
	if (this == mountCreature->getRiderDriverCreature())
	{
		mountCreature->setPosition_w(getPosition_w());
	}

	// Report log it.
	DEBUG_REPORT_LOG(s_logMountStatusChanges, ("onRiderMountedMount(): rider id=[%s],template=[%s] mounted on mount id=[%s],template=[%s].\n", getNetworkId().getValueString().c_str(), getObjectTemplateName(), mountCreature->getNetworkId().getValueString().c_str(), mountCreature->getObjectTemplateName()));

	HardpointObject * const riderHardpointObject = SaddleManager::createRiderHardpointObjectAndAttachToSaddle(*mountCreature);

	// Attach rider to the hardpoint object.  The rider will get an alter update from the parent.
	// This function removes the child from the alter scheduler.
	attachToObject_p(riderHardpointObject, true);

	// Remove the rider from the collision world.
	CollisionWorld::removeObject(this);

	// Prevent CollisionNotification::addToWorld() from accidentally adding
	// the player back to the CollisionWorld.
	CollisionProperty *const collisionProperty = getCollisionProperty();
	if (collisionProperty)
		collisionProperty->setDisableCollisionWorldAddRemove(true);

	// Clear player position relative to parent hardpoint object.
	resetRotateTranslate_o2p();

	//-- Set rider's riding pose.
	// ... get riding pose.
	// these are 1-based indices.
	int const riderSeatIndex = SaddleManager::getRiderSeatIndex(*mountCreature, *this);
	CrcString const *riderPoseName = SaddleManager::getRiderPoseNameForMountSeatIndex(*mountCreature, riderSeatIndex);
	if (!riderPoseName)
	{
		DEBUG_WARNING(true, ("CreatureObject::onRiderMountedMount(): SaddleManager failed to find a rider pose for mount id=[%s],template=[%s], seat index=[%d].", mountCreature->getNetworkId().getValueString().c_str(), mountCreature->getObjectTemplateName(), riderSeatIndex));
	}
	else
	{
		// ... get rider skeletal appearance.
		Appearance *const          riderBaseAppearance = getAppearance();
		SkeletalAppearance2 *const riderAppearance     = riderBaseAppearance ? riderBaseAppearance->asSkeletalAppearance2() : NULL;
		if (riderAppearance)
		{
			// ... set animation environment string variable's value.
			// The .LAT file does string selector animation selections based on the
			// value of this variable.
			AnimationEnvironment &riderAnimationEnvironment = riderAppearance->getAnimationEnvironment();
			riderAnimationEnvironment.getString(AnimationEnvironmentNames::cms_riderPose).set(riderPoseName->getString(), riderPoseName->getCrc());
		}
		else
			DEBUG_WARNING(true, ("onRiderMountedMount(): unexpected: rider id=[%s],template=[%s] does not have a skeletal appearance.", getNetworkId().getValueString().c_str(), getObjectTemplateName()));
	}

	//-- Force rider to set scale of 1.
	GamePlaybackScript::sitOnSaddle(*this, *riderHardpointObject, cs_riderHardpointName);

	//-- If we are this client's player, tell our mount it is "the player" so that
	//   the collision system does collision for the mount like it does for the player.
	//
	//   Note: I set this here on the player end because I'm guaranteed that I know both
	//         the player and the mount in this function while I'm not guaranteed that
	//         in onMountRiderMounted().
	if (Game::getPlayer() == this)
	{
		CollisionProperty *const mountCollisionProperty = mountCreature->getCollisionProperty();
		if (mountCollisionProperty)
			mountCollisionProperty->setPlayer(true);
	}
}

// ----------------------------------------------------------------------

void CreatureObject::onRiderDismountedMount(NetworkId const &oldMountId)
{
	// Just got off the mount.
	DEBUG_REPORT_LOG(s_logMountStatusChanges, ("onRiderDismountedMount(): rider id=[%s],template=[%s] dismounted mount id=[%s].\n", getNetworkId().getValueString().c_str(), getObjectTemplateName(), oldMountId.getValueString().c_str()));

	//-- Detach from and delete the hardpoint object we added to the mount to attach to the saddle hardpoint.
	Object *const deadHardpointObject = getAttachedTo();
	Vector const offset_p(deadHardpointObject->getPosition_p());

	if (getAttachedTo())
		detachFromObject(Object::DF_none);

	if (dynamic_cast<HardpointObject*>(deadHardpointObject))
		delete deadHardpointObject;
	else
		DEBUG_WARNING(deadHardpointObject, ("onRiderDismountedMount(): saddle hardpoint exists [%s] but it isn't a hardpoint object.  Rider [%s] was attached to something unexpected.", deadHardpointObject->getDebugInformation().c_str(), getDebugInformation().c_str()));

	Object         *const mountObject   = NetworkIdManager::getObjectById(oldMountId);
	ClientObject   *const mountClient   = mountObject ? mountObject->asClientObject() : NULL;
	CreatureObject *const mountCreature = mountClient ? mountClient->asCreatureObject() : NULL;

	// Ensure that the rider's position is identical to the mount's position
	if (mountCreature)
	{
		CellProperty::setPortalTransitionsEnabled(false);

			// offset the dismounting player's by their seat offsets so
			// that they won't stack on one another at the mount origin
			Transform const mount_o2c(mountCreature->getTransform_o2c());
			setPosition_p(mount_o2c.rotateTranslate_l2p(offset_p));

		CellProperty::setPortalTransitionsEnabled(true);
	}

	// Fixup the player's transform aligning to world up
	Vector const frameK_w = getObjectFrameK_w();
	setTransformKJ_o2p(frameK_w, Vector::unitY);

	// Make sure the new posture takes effect immediately
	CreatureController *const controller = dynamic_cast<CreatureController*> (getController ());
	if (controller)
	{
		const bool skipTransition = true;
		const bool skipWithDelay  = false;

		controller->changeAnimationStatePath (m_visualPosture.get(), skipTransition, skipWithDelay);
	}

	// Get player to morph back to normal scale.
	GamePlaybackScript::standFromSaddle(*this);

	CollisionProperty *const collisionProperty = getCollisionProperty();
	if (collisionProperty)
	{
		// Enable CollisionNotification::addToWorld() to add us back into CollisionWorld.
		collisionProperty->setDisableCollisionWorldAddRemove(false);
		collisionProperty->storePosition();
	}

	CollisionWorld::addObject(this);

	if (collisionProperty)
	{
		Footprint *const footprint = collisionProperty->getFootprint();
		if (footprint)
			footprint->alignToGroundNoFloat();
	}

	//-- Clear the rider's rider pose.
	Appearance *const          riderBaseAppearance = getAppearance();
	SkeletalAppearance2 *const riderAppearance     = riderBaseAppearance ? riderBaseAppearance->asSkeletalAppearance2() : NULL;
	if (riderAppearance)
	{
		AnimationEnvironment &riderAnimationEnvironment = riderAppearance->getAnimationEnvironment();
		riderAnimationEnvironment.getString(AnimationEnvironmentNames::cms_riderPose).set("", false);
	}
	else
		DEBUG_WARNING(true, ("onRiderDismountedMount(): unexpected: rider id=[%s],template=[%s] does not have a skeletal appearance.", getNetworkId().getValueString().c_str(), getObjectTemplateName()));

	//-- If we are this client's player, tell our mount it is no longer "the player" so that
	//   the collision system reverts back to doing collision the way it normally does.
	if (Game::getPlayer() == this)
	{
		if (mountCreature)
		{
			CollisionProperty *const mountCollisionProperty = mountCreature->getCollisionProperty();
			if (mountCollisionProperty)
				mountCollisionProperty->setPlayer(false);
		}
	}
	else
	{
		//-- Remote clients need to have their dead reckoning models updated.  The creatures
		//   position is correct because when the object dismounts (detaches), it's position
		//   will be set by the parent object.
		RemoteCreatureController * const remoteCreatureController = dynamic_cast<RemoteCreatureController *> (getController ());
		if (remoteCreatureController)
			remoteCreatureController->updateDeadReckoningModel (getTransform_o2w ());
	}
}

// ----------------------------------------------------------------------

void CreatureObject::onEnteredPilotStation()
{
	// Detach the pilot from whatever it currently is attached to --- we'll handle
	// attachment to the proper object here.
	if (getAttachedTo())
		detachFromObject(Object::DF_none);

	// Close the overhead map if it's open.
	GroundScene * const gs = safe_cast<GroundScene *> (Game::getScene());
	if(gs)
		gs->turnOffOverheadMap();

	//-- Attach saddle wearable and saddle static mesh, return saddle static mesh object.
	ShipObject * const shipObject = getPilotedShip();
	if (!shipObject)
	{
		DEBUG_WARNING(true, ("CreatureObject::onPilotMountedShip(): reported that we just mounted a ship but ship is NULL, rider id=[%s],template=[%s].", getNetworkId().getValueString().c_str(), getObjectTemplateName()));
		return;
	}

	Object * const containedBy = getContainedBy();
	NOT_NULL(containedBy);

	// no collision while in a ship slot
	CollisionWorld::removeObject(this);
	CollisionProperty * const collisionProperty = getCollisionProperty();
	if (collisionProperty)
		collisionProperty->setDisableCollisionWorldAddRemove(true);

	// Attach rider to the containing object.  The rider will get an alter update from the parent.
	// This function removes the child from the alter scheduler.
	attachToObject_p(containedBy, true);

	// Clear player position relative to parent hardpoint object.
	resetRotateTranslate_o2p();

	if (shipObject != containedBy)
	{
		Appearance * const appearance = getAppearance();
		if (appearance == NULL)
			WARNING(true, ("CreatureObject pilot appearance is null"));
		else
		{
			SkeletalAppearance2 * const pilotAppearance = (appearance != NULL) ? appearance->asSkeletalAppearance2() : NULL;
			if (pilotAppearance == NULL)
				WARNING(true, ("CreatureObject pilot appearance is not skeletal"));
			else
			{
				SharedObjectTemplate const * const seatSharedObjectTemplate = NON_NULL(safe_cast<SharedObjectTemplate const *>(containedBy->getObjectTemplate()));

				Appearance * const seatAppearance = containedBy->getAppearance();
				if (seatAppearance == NULL)
					WARNING(true, ("CreatureObject seat appearance is null"));
				else
				{
					Transform hardpointTransform;
					if (!seatAppearance->findHardpoint(cs_riderHardpointName, hardpointTransform))
						WARNING(true, ("CreatureObject seat appearance does not have the player hardpoint [%s]", cs_riderHardpointName.getString()));

					setTransform_o2p(hardpointTransform);

					TemporaryCrcString const seatAppearanceTemplateCrcName(seatSharedObjectTemplate->getAppearanceFilename().c_str(), true);

					int const pilotSeatIndex = 1; // these are 1-based indices.
					CrcString const * const riderPoseName = SaddleManager::getRiderPoseForSaddleAppearanceNameAndRiderSeatIndex(seatAppearanceTemplateCrcName, pilotSeatIndex);

					if (riderPoseName == NULL)
						WARNING(true, ("CreatureObject riderPose not found for ship seat [%s]", seatAppearanceTemplateCrcName.getString()));
					else
					{
						// ... set animation environment string variable's value.
						// The .LAT file does string selector animation selections based on the
						// value of this variable.
						AnimationEnvironment & riderAnimationEnvironment = pilotAppearance->getAnimationEnvironment();
						riderAnimationEnvironment.getString(AnimationEnvironmentNames::cms_riderPose).set(riderPoseName->getString(), riderPoseName->getCrc());
					}
				}
			}
		}
	}

	//-- Force rider to set scale of 1.
	GamePlaybackScript::sitOnSaddle(*this, *shipObject, cs_riderHardpointName);

	//self-specific pilot mounting code
	if (Game::getPlayer() == this)
	{
		setIntendedTarget(NetworkId::cms_invalid);
		GroundScene * const groundScene = safe_cast<GroundScene *>(Game::getScene());
		if (groundScene)
			groundScene->setView(GroundScene::CI_cockpit);
	}

	shipObject->onShipPilotMounted(this);
}

// ----------------------------------------------------------------------

void CreatureObject::onLeftPilotStation(NetworkId const &oldContainerId)
{
	ClientObject * const oldContainer = NON_NULL(safe_cast<ClientObject *>(NetworkIdManager::getObjectById(oldContainerId)));
	ShipObject * const ship = NON_NULL(ShipObject::getContainingShip(*oldContainer));

	//-- Detach from the ship, since we're a child object on the client
	if (getAttachedTo())
		detachFromObject(Object::DF_none);

	// Just got out of the ship.

	// Ensure that the pilot's position is identical to the position of the object previously attached to,
	// but push them back 1m if it's a pob ship
	if (oldContainer->getAttachedTo())
		setParentCell(oldContainer->getAttachedTo()->getCellProperty());
	CellProperty::setPortalTransitionsEnabled(false);
	Transform tr(oldContainer->getTransform_o2c());
	if (!oldContainer->asShipObject())
		tr.move_l(Vector(0.f, 0.f, -1.f));
	setTransform_o2p(tr);
	CellProperty::setPortalTransitionsEnabled(true);

	// Clear the player's orientation so that it is the identity relative to its parent
	resetRotate_o2p();

	// Get player to morph back to normal scale.
	GamePlaybackScript::standFromSaddle(*this);

	CollisionProperty * const collisionProperty = getCollisionProperty();
	if (collisionProperty)
	{
		collisionProperty->setDisableCollisionWorldAddRemove(false);
		collisionProperty->storePosition();
	}

	CollisionWorld::addObject(this);

	if (collisionProperty)
	{
		Footprint * const footprint = collisionProperty->getFootprint();
		if (footprint)
			footprint->alignToGroundNoFloat();
	}

	//-- Clear the pilot's rider pose.
	Appearance * const pilotBaseAppearance = getAppearance();
	SkeletalAppearance2 * const pilotAppearance = (pilotBaseAppearance != NULL) ? pilotBaseAppearance->asSkeletalAppearance2() : NULL;
	if (pilotAppearance != NULL)
	{
		AnimationEnvironment &riderAnimationEnvironment = pilotAppearance->getAnimationEnvironment();
		riderAnimationEnvironment.getString(AnimationEnvironmentNames::cms_riderPose).set("", false);
	}

	//self-specific pilot dismounting code
	if (Game::getPlayer() == this)
	{
		GroundScene * const groundScene = safe_cast<GroundScene *>(Game::getScene());
		if (groundScene)
			groundScene->setView(GroundScene::CI_freeChase);
	}

	ship->onShipPilotDismounted();
}

// ----------------------------------------------------------------------

void CreatureObject::onEnteredDroidStation()
{
	// Detach the droid from whatever it currently is attached to --- we'll handle
	// attachment to the proper object here.
	if (getAttachedTo())
		detachFromObject(Object::DF_none);

	//-- Attach saddle wearable and saddle static mesh, return saddle static mesh object.
	ShipObject * const shipObject = ShipObject::getContainingShip(*this);
	if (!shipObject)
	{
		DEBUG_WARNING(true, ("CreatureObject::onDroidMountedShip(): reported that we just mounted a ship but ship is NULL, droid id=[%s],template=[%s].", getNetworkId().getValueString().c_str(), getObjectTemplateName()));
		return;
	}

	Object * const droidHardpointObject = new HardpointObject(cs_droidHardpointName);
	droidHardpointObject->attachToObject_p(shipObject, true);

	// Attach rider to the hardpoint object.  The droid will get an alter update from the parent.
	// This function removes the child from the alter scheduler.
	attachToObject_p(droidHardpointObject, true);

	// no collision while in a ship slot
	CollisionWorld::removeObject(this);
	CollisionProperty * const collisionProperty = getCollisionProperty();
	if (collisionProperty)
		collisionProperty->setDisableCollisionWorldAddRemove(true);

	// Clear droid position relative to parent hardpoint object.
	resetRotateTranslate_o2p();

	//-- Force droid to set scale of 1.
	GamePlaybackScript::sitOnSaddle(*this, *droidHardpointObject, cs_droidHardpointName);

	//change our appearance to match the ship we are entering

	Appearance *const droidAppearance = getAppearance();
	if (droidAppearance)
	{
		const char* shipName = shipObject->getTemplateName();
		const char* oldAppearanceName = droidAppearance->getAppearanceTemplateName();

		Appearance* spaceAppearance = getSpaceAppearance(shipName, oldAppearanceName);

		if(spaceAppearance != NULL)
		{
			setAppearance(spaceAppearance);
		}
	}

}

// ----------------------------------------------------------------------

void CreatureObject::onLeftDroidStation(NetworkId const &)
{
	Appearance *const appearance = getAppearance();
	if (appearance)
	{
		setEffectiveAppearance();
	}
}

// ----------------------------------------------------------------------

void CreatureObject::onEnteredOperationsStation()
{
	if (getAttachedTo())
		detachFromObject(Object::DF_none);

	Object * const containedBy = getContainedBy();
	NOT_NULL(containedBy);

	// Turn off our overhead map.
	GroundScene * const gs = safe_cast<GroundScene *> (Game::getScene());
	if(gs)
		gs->turnOffOverheadMap();

	// no collision while in a ship slot
	CollisionWorld::removeObject(this);
	CollisionProperty * const collisionProperty = getCollisionProperty();
	if (collisionProperty)
		collisionProperty->setDisableCollisionWorldAddRemove(true);

	// Attach creature to the containing object.  The creature will get an alter update from the parent.
	// This function removes the child from the alter scheduler.
	attachToObject_p(containedBy, true);

	// Clear player position relative to parent hardpoint object.
	resetRotateTranslate_o2p();

	Appearance * const appearance = getAppearance();
	if (appearance == NULL)
		WARNING(true, ("CreatureObject pilot appearance is null"));
	else
	{
		SkeletalAppearance2 * const pilotAppearance = (appearance != NULL) ? appearance->asSkeletalAppearance2() : NULL;
		if (pilotAppearance == NULL)
			WARNING(true, ("CreatureObject pilot appearance is not skeletal"));
		else
		{
			SharedObjectTemplate const * const seatSharedObjectTemplate = NON_NULL(safe_cast<SharedObjectTemplate const *>(containedBy->getObjectTemplate()));

			Appearance * const seatAppearance = containedBy->getAppearance();
			if (seatAppearance == NULL)
				WARNING(true, ("CreatureObject seat appearance is null"));
			else
			{
				Transform hardpointTransform;
				if (!seatAppearance->findHardpoint(cs_riderHardpointName, hardpointTransform))
					WARNING(true, ("CreatureObject seat appearance does not have the player hardpoint [%s]", cs_riderHardpointName.getString()));

				setTransform_o2p(hardpointTransform);

				TemporaryCrcString const seatAppearanceTemplateCrcName(seatSharedObjectTemplate->getAppearanceFilename().c_str(), true);

				int const pilotSeatIndex = 1; // these are 1-based indices.
				CrcString const * const riderPoseName = SaddleManager::getRiderPoseForSaddleAppearanceNameAndRiderSeatIndex(seatAppearanceTemplateCrcName, pilotSeatIndex);

				if (riderPoseName == NULL)
					WARNING(true, ("CreatureObject riderPose not found for ship seat [%s]", seatAppearanceTemplateCrcName.getString()));
				else
				{
					// ... set animation environment string variable's value.
					// The .LAT file does string selector animation selections based on the
					// value of this variable.
					AnimationEnvironment & riderAnimationEnvironment = pilotAppearance->getAnimationEnvironment();
					riderAnimationEnvironment.getString(AnimationEnvironmentNames::cms_riderPose).set(riderPoseName->getString(), riderPoseName->getCrc());
				}
			}
		}
	}

	GamePlaybackScript::sitOnSaddle(*this, *containedBy, cs_riderHardpointName);

	if (Game::getPlayer() == this)
	{
		GroundScene * const groundScene = safe_cast<GroundScene *>(Game::getScene());
		if (groundScene)
			groundScene->setView(GroundScene::CI_cockpit);
	}
}

// ----------------------------------------------------------------------

void CreatureObject::onLeftOperationsStation(NetworkId const &oldContainerId)
{
	ClientObject * const oldContainer = NON_NULL(safe_cast<ClientObject *>(NetworkIdManager::getObjectById(oldContainerId)));

	//-- Detach from the ship, since we're a child object on the client
	if (getAttachedTo())
		detachFromObject(Object::DF_none);

	// Ensure that the pilot's position is identical to the position of the object previously attached to
	// but push them back 1m if it's a pob ship
	if (oldContainer->getAttachedTo())
		setParentCell(oldContainer->getAttachedTo()->getCellProperty());
	CellProperty::setPortalTransitionsEnabled(false);
	Transform tr(oldContainer->getTransform_o2c());
	if (!oldContainer->asShipObject())
		tr.move_l(Vector(0.f, 0.f, -1.f));
	setTransform_o2p(tr);
	CellProperty::setPortalTransitionsEnabled(true);

	// Clear the player's orientation so that it is the identity relative to its parent
	resetRotate_o2p();

	// Get player to morph back to normal scale.
	GamePlaybackScript::standFromSaddle(*this);

	CollisionProperty * const collisionProperty = getCollisionProperty();
	if (collisionProperty)
	{
		collisionProperty->setDisableCollisionWorldAddRemove(false);
		collisionProperty->storePosition();
	}

	CollisionWorld::addObject(this);

	if (collisionProperty)
	{
		Footprint * const footprint = collisionProperty->getFootprint();
		if (footprint)
			footprint->alignToGroundNoFloat();
	}

	//-- Clear the pilot's rider pose.
	Appearance * const pilotBaseAppearance = getAppearance();
	SkeletalAppearance2 * const pilotAppearance = (pilotBaseAppearance != NULL) ? pilotBaseAppearance->asSkeletalAppearance2() : NULL;
	if (pilotAppearance != NULL)
	{
		AnimationEnvironment &riderAnimationEnvironment = pilotAppearance->getAnimationEnvironment();
		riderAnimationEnvironment.getString(AnimationEnvironmentNames::cms_riderPose).set("", false);
	}

	if (Game::getPlayer() == this)
	{
		GroundScene * const groundScene = safe_cast<GroundScene *>(Game::getScene());
		if (groundScene)
			groundScene->setView(GroundScene::CI_freeChase);
	}
}

// ----------------------------------------------------------------------

void CreatureObject::onEnteredGunnerStation(int newWeaponIndex)
{
	ShipObject * const ship = NON_NULL(ShipObject::getContainingShip(*this));

	if (getAttachedTo())
		detachFromObject(Object::DF_none);

	// Turn off our overhead map.
	GroundScene * const gs = safe_cast<GroundScene *> (Game::getScene());
	if(gs)
		gs->turnOffOverheadMap();

	attachToObject_p(ship, true);

	// no collision while in a ship slot
	CollisionWorld::removeObject(this);
	CollisionProperty *const collisionProperty = getCollisionProperty();
	if (collisionProperty)
		collisionProperty->setDisableCollisionWorldAddRemove(true);

	resetRotateTranslate_o2p();

	if (Game::getPlayer() == this)
	{
		// Clear our currently selected target.
		setIntendedTarget(NetworkId::cms_invalid);

		GroundScene * const groundScene = safe_cast<GroundScene *>(Game::getScene());
		if (groundScene)
			groundScene->setView(GroundScene::CI_shipTurret);
	}

	ship->onShipGunnerMounted(*this, newWeaponIndex);
}

// ----------------------------------------------------------------------

void CreatureObject::onLeftGunnerStation(NetworkId const &oldContainerId, int oldWeaponIndex)
{
	ClientObject * const oldContainer = NON_NULL(safe_cast<ClientObject *>(NetworkIdManager::getObjectById(oldContainerId)));
	ShipObject * const ship = NON_NULL(ShipObject::getContainingShip(*oldContainer));

	if (getAttachedTo())
		detachFromObject(Object::DF_none);

	if (oldContainer->getAttachedTo())
		setParentCell(oldContainer->getAttachedTo()->getCellProperty());
	CellProperty::setPortalTransitionsEnabled(false);
		setTransform_o2p(oldContainer->getTransform_o2c());
	CellProperty::setPortalTransitionsEnabled(true);

	CollisionProperty * const collisionProperty = getCollisionProperty();
	if (collisionProperty)
	{
		collisionProperty->setDisableCollisionWorldAddRemove(false);
		collisionProperty->storePosition();
	}

	CollisionWorld::addObject(this);

	if (collisionProperty)
	{
		Footprint *const footprint = collisionProperty->getFootprint();
		if (footprint)
			footprint->alignToGroundNoFloat();
	}

	if (Game::getPlayer() == this)
	{
		GroundScene * const groundScene = safe_cast<GroundScene *>(Game::getScene());
		if (groundScene)
			groundScene->setView(GroundScene::CI_freeChase);
	}

	ship->onShipGunnerDismounted(*this, oldWeaponIndex);
}

// ----------------------------------------------------------------------

void CreatureObject::onMountRiderMounted()
{
	// This creature is a mount.
	if (SaddleManager::countTotalNumberOfPassengers(*this) == 1)
	{
		// It has just been mounted by the first rider, ie the player
		DEBUG_REPORT_LOG(s_logMountStatusChanges, ("onMountRiderMounted(): First rider : mount id=[%s],template=[%s].\n", getNetworkId().getValueString().c_str(), getObjectTemplateName()));

		//-- Set the animation mounted_creature variable to "1".
		Appearance *const          mountBaseAppearance = getAppearance();
		SkeletalAppearance2 *const mountAppearance     = mountBaseAppearance ? mountBaseAppearance->asSkeletalAppearance2() : NULL;
		if (mountAppearance)
		{
			// ... set animation environment string variable's value.
			AnimationEnvironment &mountAnimationEnvironment = mountAppearance->getAnimationEnvironment();
			mountAnimationEnvironment.getString(AnimationEnvironmentNames::cms_mountedCreature).set("1", false);
		}
		else
		{
			DEBUG_WARNING(true, ("onMountRiderMounted(): unexpected: mount id=[%s],template=[%s] does not have a skeletal appearance.", getNetworkId().getValueString().c_str(), getObjectTemplateName()));
		}
	}
	else
	{
		// It has just been mounted by another rider
		DEBUG_REPORT_LOG(s_logMountStatusChanges, ("onMountRiderMounted(): mount id=[%s],template=[%s].\n", getNetworkId().getValueString().c_str(), getObjectTemplateName()));
	}
}

// ----------------------------------------------------------------------

void CreatureObject::onMountRiderDismounted()
{
	// This creature is a mount.
	if (SaddleManager::countTotalNumberOfPassengers(*this) == 0)
	{
		// This must be false once we dismount.
		m_isMountForThisClientPlayer = false;

		// It has just been dismounted by the final rider, ie the player
		DEBUG_REPORT_LOG(s_logMountStatusChanges, ("onMountRiderDismounted(): Final rider : mount id=[%s],template=[%s].\n", getNetworkId().getValueString().c_str(), getObjectTemplateName()));

		//-- Set the animation mounted_creature variable to "1".
		Appearance *const          mountBaseAppearance = getAppearance();
		SkeletalAppearance2 *const mountAppearance     = mountBaseAppearance ? mountBaseAppearance->asSkeletalAppearance2() : NULL;
		if (mountAppearance)
		{
			// ... set animation environment string variable's value.
			AnimationEnvironment &mountAnimationEnvironment = mountAppearance->getAnimationEnvironment();
			mountAnimationEnvironment.getString(AnimationEnvironmentNames::cms_mountedCreature).set("0", false);
		}
		else
		{
			DEBUG_WARNING(true, ("onMountRiderDismounted(): unexpected: mount id=[%s],template=[%s] does not have a skeletal appearance.", getNetworkId().getValueString().c_str(), getObjectTemplateName()));
		}
	}
	else
	{
		// It has just been dismounted by a rider
		DEBUG_REPORT_LOG(s_logMountStatusChanges, ("onMountRiderDismounted(): mount id=[%s],template=[%s].\n", getNetworkId().getValueString().c_str(), getObjectTemplateName()));
	}
}

// ----------------------------------------------------------------------

void CreatureObject::onJustBecameMountable()
{
	//-- Only add the saddle dressing (the wearable part) when this is not a vehicle.
	bool const isVehicle = GameObjectTypes::isTypeOf (getGameObjectType (), static_cast<int>(SharedObjectTemplate::GOT_vehicle));
	if (!isVehicle)
		SaddleManager::addDressingToMount(*this);

	//-- Add the saddle (static mesh portion) to the mount.  For vehicles this will be
	//   the visible portion of the vehicle.
	UNREF(SaddleManager::addRiderSaddleToMount(*this));
}

// ----------------------------------------------------------------------

void CreatureObject::onJustBecameNonmountable()
{
	WARNING(true, ("Mount id=[%s],template=[%s] just became non-mountable.  Unexpected, we don't have visual support for this.", getNetworkId().getValueString().c_str(), getObjectTemplateName()));
}

// ----------------------------------------------------------------------

void CreatureObject::setupAlternateSharedCreatureObjectTemplate(CrcString const &templateName)
{
	if (strlen(templateName.getString()) < 1)
	{
		//-- Clear the alternate shared creature object template.
		if (m_alternateSharedCreatureObjectTemplate)
		{
			m_alternateSharedCreatureObjectTemplate->releaseReference();
			m_alternateSharedCreatureObjectTemplate = NULL;
		}
	}
	else
	{
		//-- Handle setting the alternate object template to something new.
		SharedCreatureObjectTemplate const *const oldTemplate = m_alternateSharedCreatureObjectTemplate;

		// Fetch the object template.
		ObjectTemplate const *const newObjectTemplate = ObjectTemplateList::fetch(templateName);
		m_alternateSharedCreatureObjectTemplate = dynamic_cast<SharedCreatureObjectTemplate const*>(newObjectTemplate);
		if (newObjectTemplate && !m_alternateSharedCreatureObjectTemplate)
		{
			// We have an alternate shared object template that is not SharedCreatureObjectTemplate-derived.
			newObjectTemplate->releaseReference();

			WARNING(true,
				("invalid: object id=[%s] specified a non SharedCreatureObjectTemplate-derived shared object template [%s], resetting to constructor object template.",
				getNetworkId().getValueString().c_str(),
				templateName.getString()
				));
		}

		//-- Release old template.
		if (oldTemplate)
			oldTemplate->releaseReference();
	}

	//-- Ensure appearance stays in sync.
	setEffectiveAppearance();
}

// ----------------------------------------------------------------------
/**
 * Ensure this creature is using the appropriate appearance, recreating the appearance as necessary.
 *
 * This function will determine the appropriate appearance based on the
 * effective shared object template's appearance, derived from
 * m_alternateAppearanceSharedObjectTemplateName.  If this string is NULL,
 * then the appearance name from the associated shared object template should
 * be used.  If that string is non-NULL, then the appearance template listed
 * in that alternate shared object template should be used.
 */

void CreatureObject::setEffectiveAppearance()
{
	TemporaryCrcString  desiredAppearanceTemplateName;

	{
		//-- Get the appropriate object template.
		SharedObjectTemplate const *const objectTemplate = fetchEffectiveObjectTemplate();
		if (!objectTemplate)
		{
			WARNING(true, ("createAppearance(): object id=[%s]: effective shared object template is NULL.", getNetworkId().getValueString().c_str()));
			return;
		}

		//-- Get the appearance template name from the shared object template.
		// Convert to a SharedObjectTemplate.
		desiredAppearanceTemplateName.set(objectTemplate->getAppearanceFilename().c_str(), true);

		// Release resources.
		objectTemplate->releaseReference();
	}

	//-- Check if we need to change the existing appearance.
	// Ensure we have an appearance template name.
	if (strlen(desiredAppearanceTemplateName.getString()) < 1)
	{
		WARNING(true, ("CreatureObject::createAppearance(): object id=[%s]: no appearance template name found, keeping existing appearance.", getNetworkId().getValueString().c_str()));
		return;
	}

	Appearance const *const appearance = getAppearance();
	AppearanceTemplate const *const appearanceTemplate = appearance ? appearance->getAppearanceTemplate() : NULL;
	CrcString const *const appearanceTemplateName = appearanceTemplate ? &appearanceTemplate->getCrcName() : NULL;

	if (appearanceTemplateName && (*appearanceTemplateName == desiredAppearanceTemplateName))
	{
		DEBUG_REPORT_LOG(true,
			("CreatureObject::createAppearance(): object id=[%s]: no need to change appearance, desired and existing appearance are the same name [%s].\n",
			getNetworkId().getValueString().c_str(),
			desiredAppearanceTemplateName.getString()
			));
		return;
	}

	//-- Delete the existing appearance and replace with desired appearance.
	setAppearance(AppearanceTemplateList::createAppearance(desiredAppearanceTemplateName.getString()));

	//-- Tell the new appearance's animation controller about our existing animation mood.
	signalAnimationMoodModified(m_animationMood.get());

	// option: walk through equipment, redo appearance-related portion for each equipped item.
	// No need to do this now because we force a player to unequip all player-equippable items
	// on the server when calling /setPlayerAppearance.

	//-- Ensure we remove the underwear if already present.  We'll add it back if necessary.
	// Delete the underwear objects.  The old creature appearance has been deleted
	// so there's no need to stop wearing the underwear on the creature appearance.
	delete m_underWearChestObject;
	m_underWearChestObject = NULL;

	delete m_underWearPantsObject;
	m_underWearPantsObject = NULL;

	checkWearingUnderWear();

	//-- Add game's animation message handler.
	GameAnimationMessageCallback::addCallback(*this);
}

//-----------------------------------------------------------------------

void CreatureObject::addedToContainer(ClientObject &containedObject, int arrangement)
{
	bool prohibitAttachmentVisuals = false;

	bool const altAppearance = isUsingAlternateAppearance();

	if(altAppearance)
		useDefaultAppearance();

	DEBUG_WARNING(ms_logAppearanceTabMessages && containedObject.getAppearance(), ("CreatureObject: Added To Container: %s", containedObject.getAppearance()->getAppearanceTemplateName()));

	//-- If the new item is a wearable, remap the appearance as necessary.
	bool const isMountable = (getCondition() & C_mount) != 0;

	// @todo: -TRF- consolidate player species appearance wearable data into
	//        WearableAppearanceMap.  For now mounts and player species wearable
	//        remapping are handled differently.

	if (isMountable)
	{
		Appearance *const wearableAppearance     = containedObject.getAppearance();
		char const *const wearableAppearancePath = wearableAppearance ? wearableAppearance->getAppearanceTemplateName() : NULL;
		
		if (!wearableAppearancePath)
		{
			DEBUG_WARNING(true, ("Cannot check for appearance remapping on CreatureObject id=[%s],template=[%s] because wearable template=[%s] does not have an appearance.", 
				getNetworkId().getValueString().c_str(), getObjectTemplateName(), containedObject.getNetworkId().getValueString().c_str()));
		}
		else
		{
			Appearance const *const wearerAppearance     = getAppearance();
			char const       *const wearerAppearancePath = wearerAppearance ? wearerAppearance->getAppearanceTemplateName() : NULL;
			
			if (!wearerAppearancePath)
			{
				DEBUG_WARNING(true, ("Cannot check for appearance remapping on CreatureObject id=[%s],template=[%s] because it does not have an appearance.", 
					getNetworkId().getValueString().c_str(), getObjectTemplateName()));
			}
			else
			{
				TemporaryCrcString const wearableAppearancePathCrc(wearableAppearancePath, false);
				TemporaryCrcString const wearerAppearancePathCrc(wearerAppearancePath, false);
				
				WearableAppearanceMap::MapResult const mapResult = WearableAppearanceMap::getMapResultForWearableAndWearer(wearableAppearancePathCrc, wearerAppearancePathCrc);
				if (mapResult.hasMapping())
				{
					// Check if the mapping is forbidden.
					if (mapResult.isWearableForbidden())
						prohibitAttachmentVisuals = true;
					else
					{
						CrcString const *const mappedWearableAppearancePathCrc = mapResult.getMappedWearableAppearanceName();
						NOT_NULL(mappedWearableAppearancePathCrc);

						if (mappedWearableAppearancePathCrc && (wearableAppearancePathCrc != *mappedWearableAppearancePathCrc))
						{
							// Change the wearable appearance to the mapped appearance name.
							containedObject.setAppearanceByName(mappedWearableAppearancePathCrc->getString());
						}
					}
				}
				else
				{
					// No mapping means don't change the source apperance name.
				}
			}
		}
	}
	else
	{
		// Make sure we don't need to change the appearance of this contained object
		std::string appearancePath;

		const bool wearable = isAppearanceWearable(containedObject, &appearancePath, &prohibitAttachmentVisuals );

		if (wearable)
		{
			// Make sure the new appearance is different from the old appearance

			char const *currentAppearancePath = containedObject.getAppearance()->getAppearanceTemplate()->getName();

			if ((containedObject.getAppearance() != NULL) &&
				(strcmp(currentAppearancePath, appearancePath.c_str()) != 0))
			{
				// Set the appropriate new appearance

				containedObject.setAppearanceByName(appearancePath.c_str());
			}
		}
	}

	//-- Do the visual attachment if not prohibited.
	if (!prohibitAttachmentVisuals)
		handleAttachWearForContainerAdd(containedObject, arrangement);

	TangibleObject::addedToContainer(containedObject, arrangement);

	checkWearingUnderWear();

	if(altAppearance)
		useAlternateAppearance();

	if (Game::getSinglePlayer () && this == Game::getPlayer ())
	{
		if (dynamic_cast<const WeaponObject *>(&containedObject))
			m_currentWeapon = CachedNetworkId (containedObject);
	}
}

//-----------------------------------------------------------------------

void CreatureObject::removedFromContainer(ClientObject &containedObject)
{
	handleDetachUnwearForContainerRemove(containedObject);
	TangibleObject::removedFromContainer(containedObject);

	checkWearingUnderWear();

	if (Game::getSinglePlayer () && this == Game::getPlayer ())
	{
		if (m_currentWeapon.get () == containedObject.getNetworkId ())
			m_currentWeapon = CachedNetworkId::cms_cachedInvalid;
	}
}

//-----------------------------------------------------------------------

void CreatureObject::checkWearingUnderWear()
{
	//-- Skip this process if there is a client data file and the data file contains
	//   client-baked wearables.  Client-baked wearables completely ignore the slot
	//   system and confuse the underwear logic.  If there are any wearables in the
	//   client data file, they must include the minimum acceptable to clothe the
	//   character.
	ClientDataFile const *const clientDataFile = getClientData ();
	if (clientDataFile && clientDataFile->hasWearables ())
	{
		// Skip attempting to clothe this creature.
		return;
	}

	const char underWearChestPath[] = "object/tangible/wearables/underwear/shared_underwear_top.iff";
	const char underWearPantsPath[] = "object/tangible/wearables/underwear/shared_underwear_bottom.iff";

	static StringVector chestSlots;
	static StringVector pantsSlots;

	if (chestSlots.empty())
	{
		chestSlots.push_back("chest1");
		chestSlots.push_back("chest2");
	}

	if (pantsSlots.empty())
	{
		pantsSlots.push_back("pants1");
		pantsSlots.push_back("pants2");
	}

	// Only check the chest slot for non-male gender

	SharedCreatureObjectTemplate::Gender gender = static_cast<SharedCreatureObjectTemplate::Gender>(getGender());

	// If we are currently disguised, we need to make sure we put the underwear on the REAL appearance and not our current one.
	bool const isDisguised = isUsingAlternateAppearance();

	if(isDisguised)
		useDefaultAppearance();

	if (gender != SharedCreatureObjectTemplate::GE_male)
	{
		checkWearing(&m_underWearChestObject, chestSlots, underWearChestPath);
	}

	// Check the pants slot

	checkWearing(&m_underWearPantsObject, pantsSlots, underWearPantsPath);

	if(isDisguised)
		useAlternateAppearance();
}

//-----------------------------------------------------------------------

void CreatureObject::checkWearing(Object **underWearObject, StringVector const &slots, const char *objectTemplateName)
{
	// If an alternate appearance is being specified, ignore the underwear check.
	// Since we delete the underwear when an alternate appearance is specified,
	// we don't have to check to do removal here.
	if (m_alternateSharedCreatureObjectTemplate)
		return;

	bool usingInvisibleAppearanceItem = false;

	SharedCreatureObjectTemplate::Gender gender = static_cast<SharedCreatureObjectTemplate::Gender>(getGender());

	if ((gender == SharedCreatureObjectTemplate::GE_male) ||
	    (gender == SharedCreatureObjectTemplate::GE_female))
	{
		switch (getSpecies())
		{
			default:
			case SharedCreatureObjectTemplate::SP_wookiee:
				{
					// Wookies are allowed to be naked
				}
				break;
			case SharedCreatureObjectTemplate::SP_human:
			case SharedCreatureObjectTemplate::SP_rodian:
			case SharedCreatureObjectTemplate::SP_trandoshan:
			case SharedCreatureObjectTemplate::SP_monCalamari:
			case SharedCreatureObjectTemplate::SP_bothan:
			case SharedCreatureObjectTemplate::SP_twilek:
			case SharedCreatureObjectTemplate::SP_zabrak:
			case SharedCreatureObjectTemplate::SP_ithorian:
			case SharedCreatureObjectTemplate::SP_sullustan:
				{
					// Make sure this is a player (or hologram of a player) and not an npc
					if (strstr(getObjectTemplate()->getName(), "player") != 0
						||	strstr(getObjectTemplate()->getName(), "hologram") != 0
					)
					{
						// Check all the slots to see if there is something in any slot

						ClientObject *clientObject = NULL;

						StringVector::const_iterator iterSlots = slots.begin();
						SkeletalAppearance2 * const skeletalAppearance = dynamic_cast<SkeletalAppearance2 * const>(getAppearance());

						for (; iterSlots != slots.end(); ++iterSlots)
						{
							char const *slot = iterSlots->c_str();

							clientObject = ContainerInterface::getObjectInSlot(*this, slot);

							if (clientObject != NULL)
							{
								if(skeletalAppearance && !skeletalAppearance->isWearing(clientObject))
								{
									ClientObject* appearanceInv = getAppearanceInventoryObject();
									bool appearanceObj = false;
									if(appearanceInv)
									{
										StringVector::const_iterator iterSlots = slots.begin();

										for (; iterSlots != slots.end(); ++iterSlots)
										{
											char const *slot = iterSlots->c_str();

											clientObject = ContainerInterface::getObjectInSlot(*appearanceInv, slot);

											if (clientObject != NULL)
											{
												if(clientObject->getGameObjectType() == SharedObjectTemplate::GOT_misc_appearance_only_invisible)
													usingInvisibleAppearanceItem = true;

												appearanceObj = true;
												break;
											}
										}
									}

									if(!appearanceObj)
									{
										// Dupes in slots. We aren't actually wearing this item.
										clientObject = NULL;

										continue;
									}
									else
										break;

								}

								break;
							}
							else
							{
								// Before we go putting on underwear, see if our appearance inventory has items that cover us.
								ClientObject* appearanceInv = getAppearanceInventoryObject();

								if(appearanceInv)
								{
									StringVector::const_iterator iterSlots = slots.begin();

									for (; iterSlots != slots.end(); ++iterSlots)
									{
										char const *slot = iterSlots->c_str();

										clientObject = ContainerInterface::getObjectInSlot(*appearanceInv, slot);

										if (clientObject != NULL)
										{
											if(clientObject->getGameObjectType() == SharedObjectTemplate::GOT_misc_appearance_only_invisible)
												usingInvisibleAppearanceItem = true;
											break;
										}
									}
								}
							}
						}

						if (clientObject == NULL)
						{
							// Put on the underwear

							if ((*underWearObject) == NULL)
							{
								// Create the underwear object

								(*underWearObject) = ObjectTemplate::createObject(objectTemplateName);

								if ((*underWearObject) == NULL)
								{
									DEBUG_WARNING(true, ("Underwear object not found: %s", (objectTemplateName != NULL) ? objectTemplateName : ""));
									return;
								}

								// Get the CustomizationData instance for the underwear object

								CustomizationDataProperty *const cdProperty = safe_cast<CustomizationDataProperty *>((*underWearObject)->getProperty(CustomizationDataProperty::getClassPropertyId()));

								if (cdProperty == NULL)
								{
									DEBUG_WARNING(true, ("CustomizationDataProperty for underwear object not found: %s", (objectTemplateName != NULL) ? objectTemplateName : ""));
									return;
								}

								CustomizationData *underWearCd = cdProperty->fetchCustomizationData();

								if (underWearCd == NULL)
								{
									DEBUG_WARNING(true, ("CustomizationData for CustomizationDataProperty of the underwear object not found: %s", (objectTemplateName != NULL) ? objectTemplateName : ""));
									return;
								}

								CustomizationData *ownerCd = fetchCustomizationData();
								if (ownerCd)
									IGNORE_RETURN(underWearCd->mountRemoteCustomizationData(*ownerCd, "/shared_owner/", "/shared_owner"));
								else
								{
									DEBUG_WARNING(true, ("CreatureObject from template [%s] has no customization data.", getObjectTemplateName() ? getObjectTemplateName() : "<NULL object template name>"));
								}

								// Select the correct appearance depending on the species/gender

								std::string appearancePath;
								const bool appearanceWearable = isAppearanceWearable(*(*underWearObject), &appearancePath);

								if (appearanceWearable)
								{
									Appearance *underWearAppearance = AppearanceTemplateList::createAppearance(appearancePath.c_str());

									if (underWearAppearance != NULL)
									{
										(*underWearObject)->setAppearance(underWearAppearance);
									}
								}

								underWearCd->release();

								if (ownerCd)
									ownerCd->release();

								// Put the underwear object on the skeleton

								SkeletalAppearance2 * const skeletalAppearance = dynamic_cast<SkeletalAppearance2 * const>(getAppearance());

								if (skeletalAppearance != NULL)
								{
									skeletalAppearance->wear(*underWearObject);
								}
							}
						}
						else if ((*underWearObject) != NULL)
						{
							SkeletalAppearance2 * const skeletalAppearance = dynamic_cast<SkeletalAppearance2 * const>(getAppearance());

							if (skeletalAppearance != NULL)
							{
								skeletalAppearance->stopWearing((*underWearObject));
							}

							delete (*underWearObject);
							(*underWearObject) = NULL;
						}

						if(gender == SharedCreatureObjectTemplate::GE_female && usingInvisibleAppearanceItem)
						{
							// Put on the underwear

							if ((*underWearObject) == NULL)
							{
								// Create the underwear object

								(*underWearObject) = ObjectTemplate::createObject(objectTemplateName);

								if ((*underWearObject) == NULL)
								{
									DEBUG_WARNING(true, ("Underwear object not found: %s", (objectTemplateName != NULL) ? objectTemplateName : ""));
									return;
								}

								// Get the CustomizationData instance for the underwear object

								CustomizationDataProperty *const cdProperty = safe_cast<CustomizationDataProperty *>((*underWearObject)->getProperty(CustomizationDataProperty::getClassPropertyId()));

								if (cdProperty == NULL)
								{
									DEBUG_WARNING(true, ("CustomizationDataProperty for underwear object not found: %s", (objectTemplateName != NULL) ? objectTemplateName : ""));
									return;
								}

								CustomizationData *underWearCd = cdProperty->fetchCustomizationData();

								if (underWearCd == NULL)
								{
									DEBUG_WARNING(true, ("CustomizationData for CustomizationDataProperty of the underwear object not found: %s", (objectTemplateName != NULL) ? objectTemplateName : ""));
									return;
								}

								CustomizationData *ownerCd = fetchCustomizationData();
								if (ownerCd)
									IGNORE_RETURN(underWearCd->mountRemoteCustomizationData(*ownerCd, "/shared_owner/", "/shared_owner"));
								else
								{
									DEBUG_WARNING(true, ("CreatureObject from template [%s] has no customization data.", getObjectTemplateName() ? getObjectTemplateName() : "<NULL object template name>"));
								}

								// Select the correct appearance depending on the species/gender

								std::string appearancePath;
								const bool appearanceWearable = isAppearanceWearable(*(*underWearObject), &appearancePath);

								if (appearanceWearable)
								{
									Appearance *underWearAppearance = AppearanceTemplateList::createAppearance(appearancePath.c_str());

									if (underWearAppearance != NULL)
									{
										(*underWearObject)->setAppearance(underWearAppearance);
									}
								}

								underWearCd->release();

								if (ownerCd)
									ownerCd->release();

								// Put the underwear object on the skeleton

								SkeletalAppearance2 * const skeletalAppearance = dynamic_cast<SkeletalAppearance2 * const>(getAppearance());

								if (skeletalAppearance != NULL)
								{
									skeletalAppearance->wear(*underWearObject);
								}
							}
							else if((*underWearObject) != NULL)
							{
								SkeletalAppearance2 * const skeletalAppearance = dynamic_cast<SkeletalAppearance2 * const>(getAppearance());

								if (skeletalAppearance != NULL)
								{
									skeletalAppearance->wear(*underWearObject);
								}
							}
						}

					}
				}
				break;
		}
	}
}

//----------------------------------------------------------------------

NetworkId const & CreatureObject::getAppropriateGroupInviterBasedOffSceneType() const
{
	CreatureObject::PlayerAndShipPair const & playerAndShip = m_groupInviter.get();
	NetworkId const & playerId = playerAndShip.first.first;
	NetworkId const & shipId = playerAndShip.second;
	NetworkId const & appropriateId = (Game::isSpace() ? (shipId.isValid() ? shipId : playerId) : playerId);
	return appropriateId;
}

//----------------------------------------------------------------------

void CreatureObject::targetGroupMember      (int index)
{
	const CachedNetworkId & groupId = getGroup ();
	const GroupObject * const group = safe_cast<GroupObject *>(groupId.getObject ());
	if (group)
	{
		if (getShipStation()==ShipStation::ShipStation_None)
		{
			const GroupObject::GroupMember * const member = group->findMemberByIndex (index, getNetworkId ());
			if (member)
			{
				setLookAtAndIntendedTarget (member->first);
			}
			else
				setLookAtAndIntendedTarget (NetworkId::cms_invalid);
		}
		else
		{
			NetworkId const shipId = group->findShipByIndex(index+1);
				setLookAtTarget (shipId);
		}
	}
}

//----------------------------------------------------------------------

bool CreatureObject::isGroupedWith(const CreatureObject & creature) const
{
	const CachedNetworkId & groupId = getGroup ();
	const GroupObject * const group = safe_cast<GroupObject *>(groupId.getObject ());
	if (group != NULL)
		return group->isGroupMember(creature.getNetworkId());

	return false;
}

//----------------------------------------------------------------------

bool CreatureObject::isIncapacitated() const
{
	return m_visualPosture.get () == Postures::Incapacitated;
}

//----------------------------------------------------------------------

bool CreatureObject::isDead() const
{
	return m_visualPosture.get () == Postures::Dead;
}

//----------------------------------------------------------------------

bool CreatureObject::isBeast() const
{
	return m_isBeast.get ();
}

//----------------------------------------------------------------------

void CreatureObject::filterLocalizedName (Unicode::String & localizedName) const
{
	m_localizedNameUnfiltered = localizedName;

	TangibleObject::filterLocalizedName (localizedName);

	if (m_visualPosture.get () == Postures::Dead)
	{
		static const StringId corpse_id ("obj_n", "corpse_name_prose");
		Unicode::String result;
		CuiStringVariablesManager::process (corpse_id, localizedName, Unicode::emptyString, Unicode::emptyString, result);
		localizedName = result;
	}
}

// ----------------------------------------------------------------------

void CreatureObject::conditionModified(int oldCondition, int newCondition)
{
	//-- Handle base class.
	TangibleObject::conditionModified(oldCondition, newCondition);

	//-- Handle mountable species (i.e. vehicles and living mounts).
	bool const wasMount = (oldCondition & C_mount) != 0;
	bool const isMount  = (newCondition & C_mount) != 0;

	if (wasMount ^ isMount)
	{
		// Mount status has changed.
		if (isMount)
			onJustBecameMountable();
		else
			onJustBecameNonmountable();
	}
}

//----------------------------------------------------------------------

bool CreatureObject::getSpeciesString(Unicode::String &species) const
{
	species = Species::getLocalizedName(static_cast<SharedCreatureObjectTemplate::Species>(getSpecies()));
	return true;
}

//----------------------------------------------------------------------

const Unicode::String &CreatureObject::getLocalizedNameUnfiltered() const
{
	if (m_localizedNameDirty)
	{
		updateLocalizedName();
		m_localizedNameDirty = false;
	}

	return m_localizedNameUnfiltered;
}

//----------------------------------------------------------------------

uint32 CreatureObject::getPvpFlags() const
{
	NetworkId const &masterId = getMasterId();
	if (masterId != NetworkId::cms_invalid)
	{
		CreatureObject const *master = dynamic_cast<CreatureObject const *>(NetworkIdManager::getObjectById(masterId));
		if (master)
			return master->getPvpFlags();
	}
	return TangibleObject::getPvpFlags();
}

//----------------------------------------------------------------------

int CreatureObject::getPvpType() const
{
	NetworkId const &masterId = getMasterId();
	if (masterId != NetworkId::cms_invalid)
	{
		CreatureObject const *master = dynamic_cast<CreatureObject const *>(NetworkIdManager::getObjectById(masterId));
		if (master)
			return master->getPvpType();
	}
	return TangibleObject::getPvpType();
}

//----------------------------------------------------------------------

uint32 CreatureObject::getPvpFaction() const
{
	NetworkId const &masterId = getMasterId();

	if (masterId != NetworkId::cms_invalid)
	{
		CreatureObject const *master = dynamic_cast<CreatureObject const *>(NetworkIdManager::getObjectById(masterId));
		if (master)
			return master->getPvpFaction();
	}
	return TangibleObject::getPvpFaction();
}

//----------------------------------------------------------------------
/**
 * Return the effective species of this creature.
 *
 * The effective species is the species of the alternate appearance shared object template, if non-empty;
 * otherwise, it is the species of the object template that created this object.
 *
 * @return the effective (visible) species of this creature.
 */

int CreatureObject::getSpecies() const
{
	SharedCreatureObjectTemplate const *const creatureTemplate = fetchEffectiveObjectTemplate();
	if (!creatureTemplate)
		return static_cast<int>(SharedCreatureObjectTemplate::SP_human);
	else
	{
		int const species = static_cast<int>(creatureTemplate->getSpecies());
		creatureTemplate->releaseReference();
		return species;
	}
}

//----------------------------------------------------------------------
/**
 * Return the effective gender of this creature.
 *
 * The effective gender is the gender of the alternate appearance shared object template, if non-empty;
 * otherwise, it is the gender of the object template that created this object.
 *
 * @return the effective (visible) gender of this creature.
 */

int CreatureObject::getGender() const
{
	SharedCreatureObjectTemplate const *const creatureTemplate = fetchEffectiveObjectTemplate();
	if (!creatureTemplate)
		return static_cast<int>(SharedCreatureObjectTemplate::GE_other);
	else
	{
		int const gender = static_cast<int>(creatureTemplate->getGender());
		creatureTemplate->releaseReference();
		return gender;
	}
}

//----------------------------------------------------------------------

int16 CreatureObject::getGroupLevel          () const
{
	const GroupObject * const group = safe_cast<GroupObject *>(m_group.get ().getObject ());
	if (group)
		return group->getGroupLevel ();

	return m_level.get ();
}

// ----------------------------------------------------------------------

void CreatureObject::setPlaybackScriptIsAssignedToIncapacitateMe(bool isAssigned)
{
	if (isAssigned)
	{
		//-- Start the incap/dead lock-out timer to prevent multiple playback scripts from killing this creature.
		if (m_isPlaybackScriptScheduledToIncapacitate)
		{
			DEBUG_WARNING(true, ("called CreatureObject::setPlaybackScriptIsAssignedToIncapacitateMe() on id=[%s] ot=[%s] when a script is already assigned to do so --- logic error!", getNetworkId().getValueString().c_str(), getObjectTemplateName()));
			return;
		}

		m_isPlaybackScriptScheduledToIncapacitate = true;
		m_playbackScriptNoIncapacitationTimer.setExpireTime(6.0f);
		m_playbackScriptNoIncapacitationTimer.reset();
	}
	else
	{
		//-- Clear the lockout timer if set.
		m_isPlaybackScriptScheduledToIncapacitate = false;
	}
}

// ----------------------------------------------------------------------

std::string CreatureObject::getRankString(uint8 rank)  // static
{
	std::string result;

	RankStringMap::const_iterator i = ms_rankStringMap.find( rank );
	if ( i != ms_rankStringMap.end() )
	{
		result = i->second;
	}
	else
	{
		if ( rank > 0 )
		{

			// Retrieve the table that translates the rank to an actual string
			DataTable const * const dt = DataTableManager::getTable( cms_tableNameString, true );
			if ( dt )
			{
				// Make sure the column name is valid
				if ( dt->doesColumnExist( cms_columnNameString ) )
				{
					// Finally, make sure the column is the correct data type
					if ( dt->getDataTypeForColumn( cms_columnNameString ).getBasicType() == DataTableColumnType::DT_String )
					{
						// The player rank gives us the row in the table, so make sure it is valid
						if ( rank < dt->getNumRows() )
						{
							result = dt->getStringValue( cms_columnNameString, rank );
						}
						else
						{
							WARNING(true, ("CreatureObject::getRankString: Rank [%d] does not correspond to a row in data table [%s]", rank, cms_tableNameString.c_str()));
						}
					}
					else
					{
						WARNING_STRICT_FATAL(true, ("CreatureObject::getRankString: Column [%s] for data table [%s] has invalid data type", cms_columnNameString.c_str(), cms_tableNameString.c_str()));
					}
				}
				else
				{
					WARNING_STRICT_FATAL(true, ("CreatureObject::getRankString: Column [%s] does not exist for data table [%s]", cms_columnNameString.c_str(), cms_tableNameString.c_str()));
				}
			}
			else
			{
				WARNING_STRICT_FATAL(true, ("CreatureObject::getRankString: data table [%s] not found", cms_tableNameString.c_str()));
			}
		}

		// Store the result for future reference
		ms_rankStringMap[ rank ] = result;
	}

	return result;
}

// ----------------------------------------------------------------------

Unicode::String CreatureObject::getLocalizedRankString(uint8 rank)  // static
{
	Unicode::String result;

	std::string rankString = getRankString( rank );
	if ( !rankString.empty() )
	{
		result = StringId("faction_recruiter", rankString.c_str()).localize();
	}

	return result;
}

// ----------------------------------------------------------------------

Unicode::String CreatureObject::getLocalizedGcwRankString(int gcwFactionRank, uint32 factionCrc) 
{
	static char buffer[256];
	if (PvpData::isRebelFactionId(factionCrc))
	{
		snprintf(buffer, sizeof(buffer)-1, "rebel_rank%d", gcwFactionRank);
		buffer[sizeof(buffer)-1] = '\0';
	}
	else if (PvpData::isImperialFactionId(factionCrc))
	{
		snprintf(buffer, sizeof(buffer)-1, "imperial_rank%d", gcwFactionRank);
		buffer[sizeof(buffer)-1] = '\0';
	}
	else
	{
		strcpy(buffer, "unranked");
	}
	
	return StringId("gcw_rank", buffer).localize();
}

// ----------------------------------------------------------------------

/**
 * Find the chair closest to this creature.
 *
 * @return  the ClientObject representing the chair closest to this creature.
 */

TangibleObject *CreatureObject::getClosestChair()
{
	//-- Ensure we're in the world.
	if (!isInWorld())
	{
		DEBUG_WARNING(true, ("getClosestChair() called on id=[%s],template=[%s] but CreatureObject is not in world!  No chairs returned.", getNetworkId().getValueString().c_str(), getObjectTemplateName()));
		return NULL;
	}

	//-- Return the closest chair that is considered to be within range in this creature's current cell.
	NetworkId  creatureCellId;

	if (!isInWorldCell())
		creatureCellId = NON_NULL(getParentCell())->getOwner().getNetworkId();
	else
		creatureCellId = NetworkId::cms_invalid;

	return findClosestInRangeChair(creatureCellId, getPosition_p());
}

// ----------------------------------------------------------------------

bool CreatureObject::isMountForAndDrivenByClientPlayer() const
{
	if (isMountForThisClientPlayer ())
	{
		CreatureObject const * const player = Game::getPlayerCreature();
		return getRiderDriverCreature() == player;
	}
	return false;
}

// ----------------------------------------------------------------------
/**
 * Called on a rider to find the CreatureObject on which the rider is mounted.
 */

CreatureObject *CreatureObject::getMountedCreature()
{
	Object *const containerObject = ContainerInterface::getContainedByObject(*this);
	if (!containerObject)
		return NULL;

	ClientObject *const containerClient = containerObject->asClientObject();
	if (!containerClient)
		return NULL;

	CreatureObject *const containerCreature = containerClient->asCreatureObject();
	if (!containerCreature)
		return NULL;

	if ((containerCreature->getCondition() & TangibleObject::C_mount) != 0)
		return containerCreature;
	else
		return NULL;
}

// ----------------------------------------------------------------------
/**
 * Called on a rider to find the CreatureObject on which the rider is mounted.
 */

CreatureObject const *CreatureObject::getMountedCreature() const
{
	// Pass along to non-const version rather than duplicate code.
	return const_cast<CreatureObject*>(this)->getMountedCreature();
}

// ----------------------------------------------------------------------
/**
 * Called on a pilot to find the ShipObject on which the pilot is mounted.
 */

ShipObject *CreatureObject::getPilotedShip()
{
	return m_shipStation == ShipStation::ShipStation_Pilot ? ShipObject::getContainingShip(*this) : 0;
}

// ----------------------------------------------------------------------
/**
 * Called on a pilot to find the ShipObject on which the pilot is mounted.
 */

ShipObject const *CreatureObject::getPilotedShip() const
{
	// Pass along to non-const version rather than duplicate code.
	return const_cast<CreatureObject *>(this)->getPilotedShip();
}


// ----------------------------------------------------------------------
/**
 * Called to find the ShipObject that contains the player.
 */

ShipObject *CreatureObject::getContainingShip()
{
	return ShipObject::getContainingShip(*this);
}

// ----------------------------------------------------------------------
/**
* Called to find the ShipObject that contains the player.
 */

ShipObject const *CreatureObject::getContainingShip() const
{
	// Pass along to non-const version rather than duplicate code.
	return const_cast<CreatureObject *>(this)->getPilotedShip();
}

// ----------------------------------------------------------------------
/**
 * Called on a creature to see if it has any pilot skills.
 */

bool CreatureObject::isAPilot() const
{
	SkillList const & skills = getSkills();
	for (SkillList::const_iterator i = skills.begin(); i != skills.end(); ++i)
	{
		SkillObject const * const skill = *i;
		if (skill)
		{
			if(skill->getSkillName() == "pilot_rebel_navy_novice" ||
			   skill->getSkillName() == "pilot_imperial_navy_novice" ||
			   skill->getSkillName() == "pilot_neutral_novice")
			{
				return true;
			}
		}
	}
	return false;
}

// ----------------------------------------------------------------------
/**
 * Called on a mount to find the CreatureObject that is driving the creature.
 */

CreatureObject *CreatureObject::getRiderDriverCreature()
{
	//-- If we're not marked as being mounted, ignore.
	if (!getState(States::MountedCreature))
		return NULL;

	//-- Grab the container.
	SlottedContainer *const container = getSlottedContainerProperty();
	if (!container)
		return NULL;

	//-- Grab the item in the rider slot.
	Container::ContainerErrorCode  errorCode = Container::CEC_Success;
	Object *const riderAsObject = container->getObjectInSlot(s_riderSlotId, errorCode).getObject();
	if (!riderAsObject || (errorCode != Container::CEC_Success))
		return NULL;

	//-- Convert rider to CreatureObject.
	ClientObject *const   riderAsClient   = riderAsObject->asClientObject();
	CreatureObject *const riderAsCreature = riderAsClient ? riderAsClient->asCreatureObject() : NULL;

	return riderAsCreature;
}

// ----------------------------------------------------------------------

CreatureObject const *CreatureObject::getRiderDriverCreature() const
{
	// Pass along to non-const version rather than duplicate code.
	return const_cast<CreatureObject*>(this)->getRiderDriverCreature();
}

// ----------------------------------------------------------------------

void CreatureObject::fixupMountedPlayerOnCmdSceneReady()
{
	if (this == Game::getPlayerCreature())
		onRiderMountedMount();
}

// ----------------------------------------------------------------------

void CreatureObject::riderDoDismountVisuals(NetworkId const &oldMountId)
{
	bool const oldRidingStatus = isRidingMount();
	if (oldRidingStatus)
	{
		m_isRidingMount = false;
		onRiderDismountedMount(oldMountId);

		// Ask the mount to do its thing.  Failure to do this prevents
		// some state from being cleaned up that will cause this player's mount
		// to hang around erroneously if the mount is immediately destroyed
		// after this.
		Object         *const mountAsObject   = NetworkIdManager::getObjectById(oldMountId);
		ClientObject   *const mountAsClient   = mountAsObject ? mountAsObject->asClientObject() : NULL;
		CreatureObject *const mountAsCreature = mountAsClient ? mountAsClient->asCreatureObject() : NULL;
		if (mountAsCreature)
			mountAsCreature->onMountRiderDismounted();
	}
}

// ----------------------------------------------------------------------

void CreatureObject::doScaleChangeFixupsForMount()
{
	//-- Check that we're a mountable creature.
	if ((getCondition() & C_mount) == 0)
		return;

	//-- Mounts: if the scale factor changed after we created the saddle, we need to update the saddle and rider scale.
	// Handle the saddle.
	Object *const saddleObject = SaddleManager::getSaddleObjectFromMount(*this);
	float const newValue = getScaleFactor();
	if (saddleObject)
	{
		if (newValue > 0.0)
		{
			float const saddleScale = 1.0f / newValue;
			DEBUG_REPORT_LOG(s_logMountedRiderScale, ("CreatureObject: scale factor changed saddle scale. Mount id=[%s], mount scale=[%.2f], reverse saddle scale=[%.2f].\n", getNetworkId().getValueString().c_str(), newValue, saddleScale));
			saddleObject->setScale(Vector(saddleScale, saddleScale, saddleScale));
		}
	}
}

//-----------------------------------------------------------------------

float CreatureObject::getMinimumSpeed(bool ignoreVehicle) const
{
	const bool mounted = !ignoreVehicle && getState(States::MountedCreature);
	return _getMinimumSpeed(mounted);
}

//-----------------------------------------------------------------------

float CreatureObject::_getMountedMinimumSpeed() const
{
	float minSpeed = 0.0f;

	// Mounts: we stuff the mount's movement speed info into the player that is riding the mount.
	// This is important on the rider's client for the rider to drive the pet at the proper speed.

	// min speed only applies to vehicles
	if (GameObjectTypes::isTypeOf (getGameObjectType (), SharedObjectTemplate::GOT_vehicle))
	{
		//-- vehicle
		const Object * const vehicleSaddle = SaddleManager::getSaddleObjectFromMount (*this);
		const VehicleHoverDynamics * const vhd = vehicleSaddle ? dynamic_cast<const VehicleHoverDynamics *>(vehicleSaddle->getDynamics ()) : 0;
		if (vhd)
			minSpeed = vhd->getSpeedMin ();
	}

	return _adjustMovementSpeed(minSpeed, true);
}

//-----------------------------------------------------------------------

bool CreatureObject::getCanStrafe(bool ignoreVehicle) const
{
	const bool mounted = !ignoreVehicle && getState(States::MountedCreature);
	if(mounted)
	{
		if (GameObjectTypes::isTypeOf (getGameObjectType (), SharedObjectTemplate::GOT_vehicle))
		{
			//-- vehicle
			const Object * const vehicleSaddle = SaddleManager::getSaddleObjectFromMount (*this);
			const VehicleHoverDynamics * const vhd = vehicleSaddle ? dynamic_cast<const VehicleHoverDynamics *>(vehicleSaddle->getDynamics ()) : 0;
			if (vhd)
				return vhd->getCanStrafe ();
		}
	}

	// only driven vehicles have movement restrictions
	return true;
}

//-----------------------------------------------------------------------

float CreatureObject::getRunSpeed(bool ignoreVehicle) const
{
	// Mounts: we stuff the mount's movement speed info into the player that is riding the mount.
	// This is important on the rider's client for the rider to drive the pet at the proper speed.
	const bool mounted = !ignoreVehicle && getState(States::MountedCreature);
	return _getRunSpeed(mounted);
}

//-----------------------------------------------------------------------

float CreatureObject::_getMountedRunSpeed() const
{
	float runSpeed = m_runSpeed.get();

	// Mounts: we stuff the mount's movement speed info into the player that is riding the mount.
	// This is important on the rider's client for the rider to drive the pet at the proper speed.

	if (GameObjectTypes::isTypeOf (getGameObjectType (), SharedObjectTemplate::GOT_vehicle))
	{
		//-- vehicle
		const Object * const vehicleSaddle = SaddleManager::getSaddleObjectFromMount (*this);
		const VehicleHoverDynamics * const vhd = vehicleSaddle ? dynamic_cast<const VehicleHoverDynamics *>(vehicleSaddle->getDynamics ()) : 0;
		if (vhd)
			runSpeed = vhd->getSpeedMax ();
	}
	else if (isMountForThisClientPlayer())
	{
		//-- mount
		CreatureObject const *playerCreatureObject = Game::getPlayerCreature();
		if (playerCreatureObject)
			runSpeed = playerCreatureObject->m_runSpeed.get();
	}

	return _adjustMovementSpeed(runSpeed, true);
}

//-----------------------------------------------------------------------

float CreatureObject::getWalkSpeed(bool ignoreVehicle) const
{
	// Mounts: we stuff the mount's movement speed info into the player that is riding the mount.
	// This is important on the rider's client for the rider to drive the pet at the proper speed.
	const bool mounted = !ignoreVehicle && getState(States::MountedCreature);
	return _getWalkSpeed(mounted);
}

// ----------------------------------------------------------------------

float CreatureObject::_getMountedWalkSpeed() const
{
	float walkSpeed = m_walkSpeed.get();

	// Mounts: we stuff the mount's movement speed info into the player that is riding the mount.
	// This is important on the rider's client for the rider to drive the pet at the proper speed.
	if (GameObjectTypes::isTypeOf (getGameObjectType (), SharedObjectTemplate::GOT_vehicle))
	{
		//-- vehicle
		const Object * const vehicleSaddle = SaddleManager::getSaddleObjectFromMount (*this);
		const VehicleHoverDynamics * const vhd = vehicleSaddle ? dynamic_cast<const VehicleHoverDynamics *>(vehicleSaddle->getDynamics ()) : 0;
		if (vhd)
		{
			walkSpeed = vhd->getSpeedMax () * 0.5f;
		}
	}
	else if (isMountForThisClientPlayer())
	{
		//-- mount
		CreatureObject const *playerCreatureObject = Game::getPlayerCreature();
		if (playerCreatureObject && playerCreatureObject->getMountedCreature() == this)
		{
			walkSpeed = playerCreatureObject->m_walkSpeed.get();
		}
	}

	return _adjustMovementSpeed(walkSpeed, true);
}

// ----------------------------------------------------------------------

float CreatureObject::getMovementScale() const
{
	return _getMovementScale(getState(States::MountedCreature) && isMountForThisClientPlayer());
}

// ----------------------------------------------------------------------

float CreatureObject::_getMountedMovementScale() const
{
	float movementScale = m_movementScale.get();

	CreatureObject const *playerCreatureObject = Game::getPlayerCreature();
	if (playerCreatureObject)
	{
		movementScale = playerCreatureObject->getMovementScale();
	}

	return movementScale;
}

// ----------------------------------------------------------------------

float CreatureObject::getMovementPercent() const
{
	return _getMovementPercent(getState(States::MountedCreature) && isMountForThisClientPlayer());
}

//----------------------------------------------------------------------

float CreatureObject::_getMountedMovementPercent() const
{
	float movementPercent = m_movementPercent.get();

	// Mounts: we stuff the mount's movement speed info into the player that is riding the mount.
	// This is important on the rider's client for the rider to drive the pet at the proper speed.
	CreatureObject const *playerCreatureObject = Game::getPlayerCreature();
	if (playerCreatureObject)
	{
		movementPercent = playerCreatureObject->getMovementPercent();
	}

	return movementPercent;
}

//----------------------------------------------------------------------

void CreatureObject::setWearableData(std::vector<WearableEntry> const & wearableData)
{
	m_wearableData.clear();
	for(std::vector<WearableEntry>::const_iterator i = wearableData.begin(); i != wearableData.end(); ++i)
	{
		m_wearableData.push_back(*i);
	}
}

// ----------------------------------------------------------------------

bool CreatureObject::hasCertificationsForItem(const TangibleObject & item) const
{
	PlayerObject const * const playerObject = getPlayerObject();
	if (NULL != playerObject)
	{
		if (playerObject->isAdmin())
		{
			return true;
		}
	}

	std::vector<std::string> requiredCerts;
	item.getRequiredCertifications(requiredCerts);

	for (std::vector<std::string>::const_iterator i=requiredCerts.begin(); i!=requiredCerts.end(); ++i)
	{
		if (!hasCommand(*i))
			return false;
	}

	return true;
}

// ----------------------------------------------------------------------

int CreatureObjectNamespace::getShipStationForSlot(SlotId const &slotId)
{
	if (   slotId == ShipSlotIdManager::getPobShipPilotSlotId()
		  || slotId == ShipSlotIdManager::getShipPilotSlotId())
		return ShipStation::ShipStation_Pilot;
	if (slotId == ShipSlotIdManager::getShipDroidSlotId())
		return ShipStation::ShipStation_Droid;
	if (   slotId == ShipSlotIdManager::getPobShipOperationsSlotId()
	    || slotId == ShipSlotIdManager::getShipOperationsSlotId())
		return ShipStation::ShipStation_Operations;
	int weaponIndex = ShipSlotIdManager::getGunnerSlotWeaponIndex(slotId);
	if (weaponIndex != -1)
		return ShipStation::ShipStation_Gunner_First + weaponIndex;
	return ShipStation::ShipStation_None;
}

// ----------------------------------------------------------------------

bool CreatureObjectNamespace::shouldCreatureBeVisible(CreatureObject const &creature)
{
	// uncontained creatures in space are only visible to admins
	if (Game::isSpace() && !ContainerInterface::getContainedByObject(creature) && !ClientObject::isFakeNetworkId(creature.getNetworkId()) && (creature.getNetworkId() != NetworkId::cms_invalid))
		return PlayerObject::isAdmin();

	int const shipStation = creature.getShipStation();

	// the droid and none stations are always visible
	if (shipStation == ShipStation::ShipStation_None || shipStation == ShipStation::ShipStation_Droid)
		return true;

	// the pilot and operations stations are only visible for pob ships
	if (shipStation == ShipStation::ShipStation_Pilot || shipStation == ShipStation::ShipStation_Operations)
	{
		// only pob ship pilots are visible (pilots not directly contained by a ship)
		Object const * const containedByObject = ContainerInterface::getContainedByObject(creature);
		if (containedByObject && containedByObject->asClientObject() && !containedByObject->asClientObject()->asShipObject())
			return true;
	}

	// other stations are never visible
	return false;
}

// ----------------------------------------------------------------------

void CreatureObject::setLookAtTargetSlot(ShipChassisSlotType::Type value)
{
	ShipObject::enableTargetAppearanceFromObjectId(this, m_lookAtTarget.get(), false, m_lookAtTargetSlot);
	m_lookAtTargetSlot = value;
	ShipObject::enableTargetAppearanceFromObjectId(this, m_lookAtTarget.get(), true, m_lookAtTargetSlot);

	if (Game::getPlayerPilotedShip())
		Game::getPlayerPilotedShip()->setPilotLookAtTargetSlot(value);
}


// ----------------------------------------------------------------------

void CreatureObject::groupMissionCriticalObjectsOnChanged()
{
	Transceivers::groupMissionCriticalObjectsChanged.emitMessage(*this);
}

//----------------------------------------------------------------------

bool CreatureObject::isGroupMissionCriticalObject(NetworkId const & target) const
{
	for (GroupMissionCriticalObjectSet::iterator iter = m_groupMissionCriticalObjectSet.begin(); iter != m_groupMissionCriticalObjectSet.end(); ++iter)
		if (iter->second == target)
			return true;

	return false;
}

//----------------------------------------------------------------------

CachedNetworkId const & CreatureObject::getCombatTarget() const
{
	if (getState(States::Combat))
	{
		return m_lookAtTarget.get();
	}

	return CachedNetworkId::cms_cachedInvalid;
}

// ----------------------------------------------------------------------

CreatureObject const * CreatureObject::asCreatureObject(Object const * object)
{
	ClientObject const * const clientObject = (object != NULL) ? object->asClientObject() : NULL;

	return (clientObject != NULL) ? clientObject->asCreatureObject() : NULL;
}

// ----------------------------------------------------------------------

CreatureObject * CreatureObject::asCreatureObject(Object * object)
{
	ClientObject * clientObject = (object != NULL) ? object->asClientObject() : NULL;

	return (clientObject != NULL) ? clientObject->asCreatureObject() : NULL;
}

// ----------------------------------------------------------------------

CreatureObject::Difficulty CreatureObject::getDifficulty() const
{
	return static_cast<Difficulty>(m_difficulty.get());
}

// ----------------------------------------------------------------------
/**
 * Creates a "slow down" effect between this creature and a defender. The effect acts as if
 * there was a hill between the combatants, with the attacker at the top and the defender
 * at the bottom. The area of the effect is a cone between the attacker and defender.
 * The "hill" will be maintained between the combatants even if they move.
 *
 * @param defender		the target that defines the direction of the "hill"
 * @param coneLength	the length of the effect cone
 * @param coneAngle		the angle of the effect cone, in radians
 * @param slopeAngle	the angle of the "hill", in radians
 * @param expireTime	the game time when the effect expires
 */
void CreatureObject::addSlowDownEffect(const TangibleObject & defender, float coneLength, float coneAngle, float slopeAngle, unsigned long expireTime)
{
	// if we already have a slow down effect going, remove it
	removeSlowDownEffect();
	Property * property = new SlowDownProperty(*this, CachedNetworkId(defender), coneLength, coneAngle, slopeAngle, expireTime);
	addProperty(*property, true);
}

// ----------------------------------------------------------------------

/**
 * Causes any slow down effect being used by us to be canceled early.
 */
void CreatureObject::removeSlowDownEffect()
{
	Property * property = getProperty(SlowDownProperty::getClassPropertyId());
	if (property != NULL)
		removeProperty(SlowDownProperty::getClassPropertyId());
}

// ----------------------------------------------------------------------

/**
 * Adds an additional terrain slope effect that will be taken into account the
 * next time we alter this creature. Note that this function only affects our
 * player's creature.
 *
 * @param normal		the normal vector of the "terrain" that is affecting us
 */
void CreatureObject::addTerrainSlopeEffect(const Vector & normal)
{
	CreatureObject * ourPlayer = Game::getPlayerCreature();
	if (ourPlayer != NULL && ourPlayer->getNetworkId() == getNetworkId())
	{
		Property * property = getProperty(SlopeEffectProperty::getClassPropertyId());
		if (property == NULL)
		{
			property = new SlopeEffectProperty(*this);
			addProperty(*property, true);
		}
		safe_cast<SlopeEffectProperty*>(property)->addNormal(normal);
	}
}

// ----------------------------------------------------------------------

float CreatureObject::getBaseSlopeModPercent() const
{
	const SharedCreatureObjectTemplate * myTemplate = safe_cast<const SharedCreatureObjectTemplate *>(getObjectTemplate());
	return myTemplate->getSlopeModPercent();
}

// ----------------------------------------------------------------------

char const * CreatureObjectNamespace::getSurfaceType(CreatureObject const & object)
{
	//-- is the object not in a cell?
	if (object.getParentCell() == CellProperty::getWorldCellProperty())
	{
		//-- is the object on the terrain?
		CollisionProperty const * const collision = object.getCollisionProperty();
		Footprint const * const footprint = collision ? collision->getFootprint() : 0;
		bool const isOnSolidFloor = footprint ? footprint->isOnSolidFloor() : false;
		FloorLocator const * const floorLocator = footprint ? footprint->getSolidContact() : 0;

		if (isOnSolidFloor && floorLocator)
		{
			Object const * const solidObject = floorLocator->getFloor()->getOwner();
			if (solidObject)
			{
				//-- grab the surfacetype from the object template
				SharedObjectTemplate const * const objectTemplate = safe_cast<const SharedObjectTemplate*>(solidObject->getObjectTemplate());
				switch(objectTemplate->getSurfaceType())
				{
				case SharedObjectTemplate::ST_other:
					return CoverEffects::other;

				case SharedObjectTemplate::ST_metal:
					return CoverEffects::metal;

				case SharedObjectTemplate::ST_stone:
					return CoverEffects::stone;

				case SharedObjectTemplate::ST_wood:
					return CoverEffects::wood;
				}
			}
		}
		else
		{
			TerrainObject const * const terrainObject = TerrainObject::getInstance();
			if (terrainObject)
			{
				//-- if we're below water, the surface type is water
				if (terrainObject->isBelowWater(object.getPosition_w()))
					return CoverEffects::water;

				//-- grab the surface type from the object template
				SharedTerrainSurfaceObjectTemplate const * const objectTemplate = safe_cast<const SharedTerrainSurfaceObjectTemplate*> (terrainObject->getSurfaceProperties (object.getPosition_w()));
				if (objectTemplate)
					return objectTemplate->getSurfaceType().c_str();
			}
		}
	}
	else
	{
		//-- object is in a cell, so get the cell property's footstep id.
		CellProperty const * const cellProperty = object.getParentCell();

		if (cellProperty)
		{
			PortalProperty const * const portalProperty = cellProperty->getPortalProperty();
			char const * const cellName = cellProperty->getCellName();

			if (cellName && portalProperty)
			{
				char const * const pobShortName = portalProperty->getPobShortName();

				if (pobShortName)
				{
					InteriorEnvironmentBlock const * envBlock = InteriorEnvironmentBlockManager::getEnvironmentBlock(pobShortName, cellName);

					if (envBlock)
					{
						return envBlock->getSurfaceType();
					}
				}
			}
		}
	}

	return 0;
}

// ----------------------------------------------------------------------

void CreatureObject::setCoverVisibility(bool const visible, bool const islocal)
{
	// Get the current player.
	CreatureObject * const player = Game::getPlayerCreature();

	// Reset target information.
	if (player && (player != this))
	{
		if (player->getLookAtTarget() == getNetworkId())
		{
			player->setLookAtTarget(NetworkId::cms_invalid);
		}
	}

	// Trigger a client effect based on surface type and cover state.
	char const * surfaceType = getSurfaceType(*this);

	if (surfaceType)
	{
		std::string const surfaceTypeName(surfaceType);
		std::string const coverEffectTypeName(visible ? CoverEffects::leaveCover : CoverEffects::takeCover);
		CrcLowerString const coverEffectEvent((coverEffectTypeName + surfaceTypeName).c_str());
		IGNORE_RETURN(ClientEventManager::playEvent(coverEffectEvent, this, CrcLowerString::empty));
	}

	// Hide object on non-authoritative clients.
	SkeletalAppearance2 * const appearance = const_cast<SkeletalAppearance2 *>(getAppearance() ? getAppearance()->asSkeletalAppearance2() : 0);
	if (appearance)
	{
		if (visible)
		{
			// Fade in.
			appearance->setFadeState(SkeletalAppearance2::FS_fadingIn, appearance->getFadeFraction());
		}
		else
		{
			// Hide the player.
			if (islocal || PlayerObject::isAdmin() || isPassiveRevealPlayerCharacter(Game::getPlayerNetworkId()))
			{
				appearance->setFadeState(SkeletalAppearance2::FS_fadeToHold, appearance->getFadeFraction(), 0.25f);
				if(getCurrentWeapon())
				{
					if(getCurrentWeapon()->isLightsaberWeapon())
					{
						LightsaberAppearance* lsAppear = dynamic_cast<LightsaberAppearance*>(getCurrentWeapon()->getAppearance());
						if(lsAppear)
						{
							for(int i = 0; i < lsAppear->getBladeCount(); ++i)
								lsAppear->turnBladeOn(i, false, true);
						}
						else
						{
							DEBUG_WARNING(true, ("Creature Object: Creature ID [%s] has a lightsabre weapon but not a lightsabre appearance!\nDebug Info:\n", this->getNetworkId().getValueString()));
							DEBUG_WARNING(true, ("Creature Object Template Name: [%s]", this->getObjectTemplateName()));
							DEBUG_WARNING(true, ("Creature Object Appearance Name: [%s]", getAppearance()->getAppearanceTemplateName()));
							DEBUG_WARNING(true, ("Creature Current Weapon Template Name: [%s]", getCurrentWeapon()->getObjectTemplateName()));
							DEBUG_WARNING(true, ("Creature Current Weapon Appearance Name: [%s]",getCurrentWeapon()->getAppearanceTemplateName()));
							DEBUG_WARNING(true, ("Creature Current Weapon GOT: [%i]", getCurrentWeapon()->getGameObjectType()));
							//DEBUG_FATAL(true, ("Fatal due to invalid appearance on a lightsabre weapon.\n"));
						}
					}
				}
				ClientEffectManager::removeNonStealthClientEffects(this);
			}
			else
			{
				appearance->setFadeState(SkeletalAppearance2::FS_fadeOutToRemove, appearance->getFadeFraction());
				if(getCurrentWeapon())
				{
					if(getCurrentWeapon()->isLightsaberWeapon())
					{
						LightsaberAppearance* lsAppear = dynamic_cast<LightsaberAppearance*>(getCurrentWeapon()->getAppearance());
						if(lsAppear)
						{
							for(int i = 0; i < lsAppear->getBladeCount(); ++i)
								lsAppear->turnBladeOn(i, false, true);
						}
						else
						{
							DEBUG_WARNING(true, ("Creature Object: Creature ID [%s] has a lightsabre weapon but not a lightsabre appearance!\nDebug Info:\n", this->getNetworkId().getValueString()));
							DEBUG_WARNING(true, ("Creature Object Template Name: [%s]", this->getObjectTemplateName()));
							DEBUG_WARNING(true, ("Creature Object Appearance Name: [%s]", getAppearance()->getAppearanceTemplateName()));
							DEBUG_WARNING(true, ("Creature Current Weapon Template Name: [%s]", getCurrentWeapon()->getObjectTemplateName()));
							DEBUG_WARNING(true, ("Creature Current Weapon Appearance Name: [%s]",getCurrentWeapon()->getAppearanceTemplateName()));
							DEBUG_WARNING(true, ("Creature Current Weapon GOT: [%i]", getCurrentWeapon()->getGameObjectType()));
							//DEBUG_FATAL(true, ("Fatal due to invalid appearance on a lightsabre weapon.\n"));
						}
					}
				}
				ClientEffectManager::removeNonStealthClientEffects(this);
			}
		}
	}
}

// ----------------------------------------------------------------------

void CreatureObject::getBuffs(std::vector<Buff> & buffs) const
{
	for (std::map<uint32, Buff::PackedBuff>::const_iterator i = m_buffs.begin(); i != m_buffs.end(); ++i)
	{
		const std::pair<uint32, Buff::PackedBuff> & p = *i;
		Buff b(p.first, p.second);
		buffs.push_back(b);
	}
}

// ----------------------------------------------------------------------

void CreatureObject::getBuffs(stdmap<uint32, Buff>::fwd & buffs) const
{
	for (std::map<uint32, Buff::PackedBuff>::const_iterator i = m_buffs.begin(); i != m_buffs.end(); ++i)
	{
		const std::pair<uint32, Buff::PackedBuff> & p = *i;
		Buff b(p.first, p.second);
		buffs.insert(std::make_pair(b.m_nameCrc, b));
	}
}

// ----------------------------------------------------------------------

bool CreatureObject::hasBuff(uint32 buffNameCrc) const
{
	return (m_buffs.end() != m_buffs.find(buffNameCrc));
}

// ----------------------------------------------------------------------

bool CreatureObject::hasInvisibilityBuff(int & buffPriority) const
{
	static uint32 const invisibilityCrc = Crc::normalizeAndCalculate("invisibility");

	uint32 group1Crc, group2Crc;
	int priority;
	for (std::map<uint32, Buff::PackedBuff>::const_iterator i = m_buffs.begin(); i != m_buffs.end(); ++i)
	{
		const std::pair<uint32, Buff::PackedBuff> & p = *i;
		if (ClientBuffManager::getBuffGroupAndPriority(p.first, group1Crc, group2Crc, priority) && ((group1Crc == invisibilityCrc) || (group2Crc == invisibilityCrc)))
		{
			buffPriority = priority;
			return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------

uint32 CreatureObject::getPlayedTime() const
{
	const PlayerObject * owner = getPlayerObject();
	if (owner != NULL)
	{
		return owner->getPlayedTime();
	}
	else
	{
		const GroundScene * groundScene = dynamic_cast<GroundScene *>(Game::getScene());
		if (groundScene != NULL)
			return groundScene->getServerTime();
	}
	DEBUG_WARNING(true, ("CreatureObject::getPlayedTime unable to get time source"));
	return 0;
}

// ----------------------------------------------------------------------

void CreatureObject::addBuffDebug(uint32 const buffHash, uint32 const timestamp, float value)
{
	Buff newBuff(buffHash, timestamp, value, 0);
	m_buffs.insert(buffHash, newBuff.getPackedBuffValue());
}

// ----------------------------------------------------------------------

void CreatureObject::removeBuffDebug(uint32 const buffHash)
{
	m_buffs.erase(buffHash);
}

// ----------------------------------------------------------------------

void CreatureObject::getObjectInfo(std::map<std::string, std::map<std::string, Unicode::String> > & propertyMap) const
{
/**
  When adding a variable to this class, please add it here.  Variable that aren't easily displayable are still listed, for tracking purposes.
*/

/**
	Don't compile in production build because this maps human-readable values to data members and makes hacking easier
*/
#if PRODUCTION == 0
//	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "", 

	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "VisualPosture", m_visualPosture.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "ServerPosture", m_serverPosture.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "RevertToServerPostureTimer", m_revertToServerPostureTimer);
//	Object *                                                                   m_underWearChestObject;
//	Object *                                                                   m_underWearPantsObject;
//	Archive::AutoDeltaVector          <Attributes::Value, CreatureObject>      m_attributes;
//	Archive::AutoDeltaVector          <Attributes::Value, CreatureObject>      m_maxAttributes;
//	Archive::AutoDeltaVector          <Attributes::Value>                      m_unmodifiedMaxAttributes;
//	Archive::AutoDeltaVector          <Attributes::Value>                      m_attribBonus;
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "ShockWounds", m_shockWounds.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "Mood", m_mood.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "SayMode", m_sayMode.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "AnimationMood", m_animationMood.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "LookAtTarget", m_lookAtTarget.get().getValueString());
//	ShipChassisSlotType::Type                                                  m_lookAtTargetSlot;
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "CurrentWeapon", m_currentWeapon.get().getValueString());
//	Archive::AutoDeltaSet <const SkillObject *, CreatureObject >               m_skills;
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "MovementScale", m_movementScale.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "MovementPercent", m_movementPercent.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "WalkSpeed", m_walkSpeed.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "RunSpeed", m_runSpeed.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "AccelScale", m_accelScale.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "AccelPercent", m_accelPercent.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "TurnScale", m_turnScale.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "SlopeModAngle", m_slopeModAngle.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "SlopeModPercent", m_slopeModPercent.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "WaterModPercent", m_waterModPercent.get());
//	Archive::AutoDeltaSet<std::pair<NetworkId, NetworkId>, CreatureObject> m_groupMissionCriticalObjectSet;
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "ScaleFactor", m_scaleFactor.get());
//	Archive::AutoDeltaVariableCallback<uint64, Callbacks::StatesChanged, CreatureObject>                 m_states;
//	Archive::AutoDeltaMap<std::string, std::pair<int, int>, CreatureObject>                                                                           m_modMap;
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "Group", m_group.get().getValueString());
//	Archive::AutoDeltaVariableCallback<PlayerAndShipPair, Callbacks::GroupInviterChanged, CreatureObject>m_groupInviter;
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "PerformanceType", m_performanceType.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "PerformanceStartTime", m_performanceStartTime.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "PerformanceListenTarget", m_performanceListenTarget.get().getValueString());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "GuildId", m_guildId.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "MasterId", m_masterId.get().getValueString());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "AccumulatedHamDamage", m_accumulatedHamDamage);
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "OldMood", m_oldMood);
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "TemporaryMoodActive", m_temporaryMoodActive);
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "TemporaryMoodDuration", m_temporaryMoodDuration);
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "SettingTemporaryMood", m_settingTemporaryMood);
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "IsSittingOnObject", m_isSittingOnObject);
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "HasHeldStateObject", m_hasHeldStateObject);
//	Watcher<Object>                                                           *m_heldStateObjectWatcher;
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "AnimatingSkillData", m_animatingSkillData.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "Level", m_level.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "LevelHealthGranted", m_levelHealthGranted.get());
//	Archive::AutoDeltaVector<WearableEntry, CreatureObject>                    m_wearableData;
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "AlternateAppearanceSharedObjectTemplateName", m_alternateAppearanceSharedObjectTemplateName.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "CoverVisibility", m_coverVisibility.get());

	int buffPriority;
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "InvisibilityBuffPriority", (hasInvisibilityBuff(buffPriority) ? buffPriority : -1));
//	Timer                                                                      m_playbackScriptNoIncapacitationTimer;
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "IsPlaybackScriptScheduledToIncapacitate", m_isPlaybackScriptScheduledToIncapacitate);
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "HasAltered", m_hasAltered);
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "IsRidingMount", m_isRidingMount);
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "IsMountForThisClientPlayer", m_isMountForThisClientPlayer);
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "TurnRateMaxWalk", m_turnRateMaxWalk);
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "TurnRateMaxRun", m_turnRateMaxRun);
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "AccelerationMaxWalk", m_accelerationMaxWalk);
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "AccelerationMaxRun", m_accelerationMaxRun);
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "WarpTolerance", m_warpTolerance);
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "ShipStation", m_shipStation);
//	Watcher<Object>                                                            m_healingAttachedObject;
//	SharedCreatureObjectTemplate const                                        *m_alternateSharedCreatureObjectTemplate;
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "ClientUsesAnimationLocomotion", m_clientUsesAnimationLocomotion.get());
//	Archive::AutoDeltaMap<uint32, uint64, CreatureObject>                     m_buffs;
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "Difficulty", m_difficulty.get());

	PlayerObject const * const playerObject = getPlayerObject();
	std::string const & playerId = playerObject ? playerObject->getNetworkId().getValueString() : "0";
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "PlayerObjectId", playerId);

	ClientObject const * const invObj = getInventoryObject();
	std::string const & invId = invObj ? invObj->getNetworkId().getValueString() : "0";
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "InventoryId", invId);

	ClientObject const * const datapadObj = getDatapadObject();
	std::string const & datapadId = datapadObj ? datapadObj->getNetworkId().getValueString() : "0";
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "DatapadId", datapadId);

	ClientObject const * const bankObj = getBankObject();
	std::string const & bankId = bankObj ? bankObj->getNetworkId().getValueString() : "0";
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "BankId", bankId);

	TangibleObject::getObjectInfo(propertyMap);
#else
	UNREF(propertyMap);
#endif
}

//----------------------------------------------------------------------

std::map<std::string, int> const & CreatureObject::getCommands() const
{
	return m_commands.getMap ();
}

//----------------------------------------------------------------------

bool CreatureObject::hasCommand(std::string const & cmdName) const
{
	const std::map<std::string, int> & cmds = getCommands ();

	return (cmds.find(cmdName) != cmds.end());
}

//-----------------------------------------------------------------------

void CreatureObject::commandsOnInsert(const std::string & cmd, const int & /*value*/)
{
	if (this == Game::getPlayerCreature())
	{
		if (!PlayerObject::isAdmin() && (cmd == s_adminCommand))
		{
			PlayerObject::setAdmin(true);

			Transceivers::adminChanged.emitMessage(*this);
		}
	}

	Transceivers::commandsChanged.emitMessage(*this);
	Transceivers::commandAdded.emitMessage(Messages::CommandAdded::Payload(this, cmd));
}

//----------------------------------------------------------------------

void CreatureObject::commandsOnErase(const std::string & cmd, const int & /*value*/)
{
	if (this == Game::getPlayerCreature())
	{
		if (PlayerObject::isAdmin() && (cmd == s_adminCommand))
		{
			PlayerObject::setAdmin(false);

			Transceivers::adminChanged.emitMessage(*this);
		}
	}
	
	Transceivers::commandsChanged.emitMessage(*this);
	Transceivers::commandRemoved.emitMessage(Messages::CommandRemoved::Payload(this, cmd));
}

//-----------------------------------------------------------------------

void CreatureObject::clientGrantCommand (const std::string & command)
{
	std::map<std::string, int>::const_iterator found = m_commands.find(command);
	if (found != m_commands.end())
		m_commands.set(command, (*found).second + 1);
	else
		m_commands.set(command, 1);
}

//-----------------------------------------------------------------------

void CreatureObject::clientRevokeCommand(const std::string & command)
{
	std::map<std::string, int>::const_iterator found = m_commands.find(command);
	if (found != m_commands.end())
	{
		if ((*found).second > 1)
			m_commands.set(command, (*found).second - 1);	
		else
			m_commands.erase(found);
	}
}

//-----------------------------------------------------------------------

void CreatureObject::setLastWaterDamageTimeMs(unsigned long time)
{
	m_lastWaterDamageTimeMs = time;
}

//-----------------------------------------------------------------------
bool CreatureObject::isAiming() const
{
	return getState(States::Aiming) || m_isAiming;
}


// ======================================================================

const MovementTable *CreatureObject::_getMovementTable() const
{
	const SharedCreatureObjectTemplate * sharedTemplate = safe_cast<const SharedCreatureObjectTemplate *>(getObjectTemplate());
	const MovementTable * table = sharedTemplate->getMovementTable();
	
	return table;
}

Locomotions::Enumerator CreatureObject::getLocomotion() const
{
	const CreatureController *controller = dynamic_cast<const CreatureController *> (getController ());
	const float playerCurrentSpeed = controller ? controller->getCurrentSpeed () : 0.0f;

	MovementTable::eLocomotionSpeed locSpeed = MovementTable::kStationary;
	if (playerCurrentSpeed > getMaximumWalkSpeed())
		locSpeed = MovementTable::kFast;
	else if (playerCurrentSpeed > 0.0f)
		locSpeed = MovementTable::kSlow;
	else
		locSpeed = MovementTable::kStationary;

	return _getMovementTable()->getLocomotion(m_visualPosture.get(), m_states.get(), locSpeed);
}

// ======================================================================

int CreatureObject::getNiche() const
{
	const SharedCreatureObjectTemplate * sharedTemplate = safe_cast<const SharedCreatureObjectTemplate *>(getObjectTemplate());
	return static_cast<unsigned int>(sharedTemplate->getNiche());
}

// ======================================================================

float CreatureObject::getLookAtYaw() const
{
	return m_lookAtYaw;
}

// ======================================================================

bool CreatureObject::getUseLookAtYaw() const
{
	return m_useLookAtYaw;
}

// ======================================================================

void CreatureObject::setLookAtYaw(float lookAtYaw, bool useLookAtYaw)
{
	m_lookAtYaw = lookAtYaw;
	m_useLookAtYaw = useLookAtYaw;
}

// ======================================================================
#ifdef _DEBUG
void CreatureObject::setMovementPercent(float percent)
{
	m_movementPercent = percent;
}
#endif

void CreatureObject::onContainedSlottedContainerChanged(ClientObject &containedObject, ClientObject &newContainer, int arrangement, int oldArrangement)
{
	UNREF(containedObject);
	UNREF(arrangement);

	DEBUG_WARNING(ms_logAppearanceTabMessages, ("Creature Object: OnContainedSlottedContainerChanged called. Object [%s].", containedObject.getAppearance()->getAppearanceTemplate()->getName()));

	ClientObject* inventory = getInventoryObject();
	ClientObject* appearance = getAppearanceInventoryObject();

	if(appearance && &newContainer == appearance) // Transfering an item to our appearance inventory.
	{
		DEBUG_WARNING(ms_logAppearanceTabMessages, ("Moving object from inventory to appearance"));

		if(!containedObject.getAppearance()) // Does it have an appearance?
			return;

		if(!containedObject.getAppearance()->asSkeletalAppearance2()) // Does it have a skeletal appearance
			return;

		if(!isWearableWithAppearanceUpdate(*this, containedObject)) // Can we wear it?
			return;

		SkeletalAppearance2 * skeleAppearance = getAppearance()->asSkeletalAppearance2();
		if(!skeleAppearance)
			return;

		SlottedContainmentProperty * slottedContainment = ContainerInterface::getSlottedContainmentProperty(containedObject); // Slot information for the object.

		if(slottedContainment)
		{
			SlottedContainmentProperty::SlotArrangement const slots = slottedContainment->getSlotArrangement(arrangement); // What arrangement does it go in?
			for(unsigned int i = 0; i < slots.size(); ++i)
			{
				DEBUG_WARNING(ms_logAppearanceTabMessages, ("Checking slot[%s] for a normal inventory item.",SlotIdManager::getSlotName(slots[i]).getString()));
				ClientObject* normalInv = ContainerInterface::getObjectInSlot(*this, SlotIdManager::getSlotName(slots[i]).getString()); // Grab the item from the appropriate slot on our normal inventory
				if(normalInv && skeleAppearance->isWearing(normalInv)) // Is the item valid and are we wearing it?
				{
					skeleAppearance->stopWearing(normalInv); // Stop wearing it.
					removeCustomizationData(*this, *normalInv);
				}
			}
		}

		if(skeleAppearance)
		{
			//Stupid special code to make sure we obey our /showHelmet overlords.
			if(containedObject.getAppearance() && PlayerObject::isObjectAHelmet(containedObject.getAppearance()) && this->getPlayerObject())
				containedObject.setShouldBakeIntoMesh(this->getPlayerObject()->getShowHelmet());

			if(containedObject.getAppearance() && PlayerObject::isObjectABackpack(containedObject.getAppearance()) && this->getPlayerObject())
				containedObject.setShouldBakeIntoMesh(this->getPlayerObject()->getShowBackpack());

			if(containedObject.getGameObjectType() == SharedObjectTemplate::GOT_misc_appearance_only_invisible)
				containedObject.setShouldBakeIntoMesh(false);
			
			setCustomizationData(*this, containedObject); // Set customization data.
			skeleAppearance->wear(&containedObject); // Wear the new item.
		}
		
	}
	else if(inventory && &newContainer == inventory) // Are we going into our inventory?
	{
		DEBUG_WARNING(ms_logAppearanceTabMessages, ("Moving object from appearance to inventory."));
		SkeletalAppearance2 * skeleAppearance = getAppearance() ? getAppearance()->asSkeletalAppearance2() : NULL;
		SkeletalAppearance2 * wearableAppearance = containedObject.getAppearance() ? containedObject.getAppearance()->asSkeletalAppearance2() : NULL;

		if(!skeleAppearance || !wearableAppearance)
			return;

		if(skeleAppearance && skeleAppearance->isWearing(&containedObject)) 
		{
			skeleAppearance->stopWearing(&containedObject); // Stop wearing the appearance item.
			removeCustomizationData(*this, containedObject); // Remove the appearance customization data.
		}

		SlottedContainmentProperty * slottedContainment = ContainerInterface::getSlottedContainmentProperty(containedObject); // Get the slot property of our item.

		if(slottedContainment && oldArrangement >= 0)
		{
			SlottedContainmentProperty::SlotArrangement const slots = slottedContainment->getSlotArrangement(oldArrangement); // Get the old arrangement
			for(unsigned int i = 0; i < slots.size(); ++i)
			{
				ClientObject* normalInv = ContainerInterface::getObjectInSlot(*this, SlotIdManager::getSlotName(slots[i]).getString()); // Grab our item from our normal inventory.
				if(normalInv && !skeleAppearance->isWearing(normalInv)) // Make sure we aren't currently wearing the item (dupe items causes a debug FATAL)
				{
					bool wearAllowed = true;

					SlottedContainmentProperty * slottedContainmentNormalInv = ContainerInterface::getSlottedContainmentProperty(*normalInv); // Get the slot property of our item.
					SlottedContainmentProperty::SlotArrangement const slotsNI = slottedContainmentNormalInv->getSlotArrangement(slottedContainmentNormalInv->getCurrentArrangement()); // What arrangement does it go in?
					for(unsigned int j = 0; j < slotsNI.size(); ++j)
					{
						DEBUG_WARNING(ms_logAppearanceTabMessages, ("Checking appearance item unworn, checking slot [%s]",  SlotIdManager::getSlotName(slotsNI[j]).getString()));
						ClientObject* appearanceInv = ContainerInterface::getObjectInSlot(*appearance, SlotIdManager::getSlotName(slotsNI[j]).getString()); // Grab our item from our normal inventory.
						if(appearanceInv && skeleAppearance->isWearing(appearanceInv))
						{
							wearAllowed = false;
							break;
						}
					}

					if(wearAllowed)
					{
						if(!isWearableWithAppearanceUpdate(*this, *normalInv)) // Can we wear it?
							continue;

						if(normalInv->getAppearance() && PlayerObject::isObjectAHelmet(normalInv->getAppearance()) && this->getPlayerObject())
							normalInv->setShouldBakeIntoMesh(this->getPlayerObject()->getShowHelmet());

						if(normalInv->getAppearance() && PlayerObject::isObjectABackpack(normalInv->getAppearance()) && this->getPlayerObject())
							normalInv->setShouldBakeIntoMesh(this->getPlayerObject()->getShowBackpack());
						
						setCustomizationData(*this, *normalInv);
						skeleAppearance->wear(normalInv); // Wear our original item.

					}
				}
			}
		}
	}

	doPostContainerChangeProcessing();

	checkWearingUnderWear();
}

void CreatureObject::verifyWornAppearanceItems()
{

	NP_PROFILER_AUTO_BLOCK_DEFINE ("CreatureObject::verifyWornAppearanceItems");

	ClientObject* appearance = getAppearanceInventoryObject();

	if(!appearance || !getAppearance()) // Make sure we actually have an appearance to wear things on.
		return;
	
	if(isUsingAlternateAppearance()) // Not while wearing costumes.
		return; 

	SkeletalAppearance2* skeleAppearance = getAppearance()->asSkeletalAppearance2();
	if(!skeleAppearance)
		return;

#if _DEBUG
	unsigned int dataTest = m_wearableAppearanceData.size();
	DEBUG_WARNING(ms_logAppearanceTabMessages, ("Character [%s] has %d streamed appearance wearables.", getNetworkId().getValueString().c_str(), dataTest));
#endif


	SlottedContainer * slottedContainer = ContainerInterface::getSlottedContainer(*appearance); // Grab our appearance inventory container
	if(slottedContainer)
	{
		if(this == Game::getPlayer()) // if this is the local client
		{
			std::vector<SlotId> totalSlots;
			slottedContainer->getSlotIdList(totalSlots); // Get all our slots.
			for(unsigned int i = 0; i < totalSlots.size(); ++i)
			{
				DEBUG_WARNING(ms_logAppearanceTabMessages, ("Checking Slot %s for item.",  SlotIdManager::getSlotName(totalSlots[i]).getString()));
				ClientObject* appearObj = ContainerInterface::getObjectInSlot(*appearance, SlotIdManager::getSlotName(totalSlots[i]).getString());
				if(appearObj && !skeleAppearance->isWearing(appearObj)) // Do we have an object in that slot and are we currently wearing it?
				{
					if(!isWearableWithAppearanceUpdate(*this, *appearObj)) // Make sure we're allowed to wear it.
						continue;

					setCustomizationData(*this, *appearObj); // Set the customization data up.

					ClientObject* normalInvObj = ContainerInterface::getObjectInSlot(*this, SlotIdManager::getSlotName(totalSlots[i]).getString());
					if(normalInvObj && skeleAppearance->isWearing(normalInvObj)) // Grab the item from our normal inventory and unwear it.
					{
						skeleAppearance->stopWearing(normalInvObj);
						removeCustomizationData(*this, *normalInvObj);
					}

					if(appearObj->getAppearance() && PlayerObject::isObjectAHelmet(appearObj->getAppearance()))
						appearObj->setShouldBakeIntoMesh(CuiPreferences::getShowHelmet());

					if(appearObj->getAppearance() && PlayerObject::isObjectABackpack(appearObj->getAppearance()))
						appearObj->setShouldBakeIntoMesh(CuiPreferences::getShowBackpack());

					if(appearObj->getGameObjectType() == SharedObjectTemplate::GOT_misc_appearance_only_invisible)
						appearObj->setShouldBakeIntoMesh(false);

					skeleAppearance->wear(appearObj); // Wear our appearance item.
				}
				else if (appearObj) // We are wearing the item, make sure we don't have two items in same slot being worn.
				{
					ClientObject* normalInvObj = ContainerInterface::getObjectInSlot(*this, SlotIdManager::getSlotName(totalSlots[i]).getString());
					if(normalInvObj && skeleAppearance->isWearing(normalInvObj)) // Grab the item from our normal inventory and unwear it.
					{
						skeleAppearance->stopWearing(normalInvObj);
						removeCustomizationData(*this, *normalInvObj);
					}
				}
			}
		}
		else // Not local clients
		{
			std::map<NetworkId, TangibleObject const *> wornAppearanceMap;
			
			for(int i = 0; i < skeleAppearance->getWearableCount(); ++i)
			{
				TangibleObject const * currentItem = dynamic_cast<TangibleObject const *>(skeleAppearance->getWearableObject(i));
				wornAppearanceMap.insert(std::make_pair<NetworkId, TangibleObject const *>(currentItem->getNetworkId(), currentItem));
			}

			for (unsigned int k = 0; k < m_wearableAppearanceData.size(); ++k) // go through our streamed wearables
			{
				Container::ContainerErrorCode tmp = Container::CEC_Success;
				const WearableEntry& w = m_wearableAppearanceData.get(k);

				ConstCharCrcString cc = ObjectTemplateList::lookUp(w.m_objectTemplate);
				if (cc.isEmpty())
				{
					WARNING(true, ("SceneCreateObjectByCrc crc %08x was not found in object template list", cc.getCrc()));
					continue;
				}

				if(wornAppearanceMap.find(w.m_networkId) != wornAppearanceMap.end()) // Already wearing this object.
					continue;

				TangibleObject* const wearable = safe_cast<TangibleObject*> (ObjectTemplate::createObject (cc.getString())); // Create the object
				if (wearable)
				{
					wearable->setNetworkId(w.m_networkId); 
					wearable->setAppearanceData(w.m_appearanceString);
					wearable->endBaselines();
					
					SlottedContainmentProperty* scp = ContainerInterface::getSlottedContainmentProperty(*wearable);
					if(!scp)
					{
						WARNING(true, ("VerifyWornAppearanceItems: Tried to create a wearable that isn't slottable."));
						delete wearable;
						continue;
					}
					
					// Code to remove any normal equipped items that may be in the slot we want.
					std::vector<int> validArrangements;
					Container::ContainerErrorCode ec;
					slottedContainer->getValidArrangements(*wearable, validArrangements, ec, true, false);

					for(std::vector<int>::size_type i = 0; i < validArrangements.size(); ++i)
					{
						SlottedContainmentProperty::SlotArrangement const slotsDO = scp->getSlotArrangement(validArrangements[i]); 
						for(unsigned int j = 0; j < slotsDO.size(); ++j)
						{
							Object * slotObj = ContainerInterface::getObjectInSlot(*this, SlotIdManager::getSlotName(slotsDO[j]).getString());

							if(slotObj) // You shouldn't be here!
							{
								if(slottedContainer->remove(*slotObj, ec))
								{
									// Remove successful. Clean up the object.
									delete slotObj;
								}
								else
								{
									WARNING(true, ("VerifyWornAppearanceObjects: Failed to remove an object from a remote creature container."));
									continue;
								}
							}
						}
					}
					///////////////////////////
					
					if(wearable->getAppearance() && PlayerObject::isObjectAHelmet(wearable->getAppearance()))
					{
						if(this->getPlayerObject())
						{
							wearable->setShouldBakeIntoMesh(this->getPlayerObject()->getShowHelmet());
						}
					}

					if(wearable->getAppearance() && PlayerObject::isObjectABackpack(wearable->getAppearance()))
					{
						if(this->getPlayerObject())
						{
							wearable->setShouldBakeIntoMesh(this->getPlayerObject()->getShowBackpack());
						}
					}

					if(wearable->getGameObjectType() == SharedObjectTemplate::GOT_misc_appearance_only_invisible)
						wearable->setShouldBakeIntoMesh(false);

					if(slottedContainer->mayAdd(*wearable, w.m_arrangement, tmp)) // Can we add it?
						slottedContainer->add(*wearable, w.m_arrangement, tmp); // Add it.
					else
						delete wearable;
						
				}
				else
				{
					DEBUG_WARNING(ms_logAppearanceTabMessages, ("Failed to create appearance wearable %s", cc.getString()));
				}
			}

		}
	}

}

bool CreatureObjectNamespace::isWearableWithAppearanceUpdate(CreatureObject & wearer, Object & obj)
{
	// Make sure we don't need to change the appearance of this contained object
	std::string appearancePath;
	bool prohibitAttachmentVisuals = false;

	const bool wearable = wearer.isAppearanceWearable(obj, &appearancePath, &prohibitAttachmentVisuals );

	if (wearable && !prohibitAttachmentVisuals)
	{
		// Make sure the new appearance is different from the old appearance

		char const *currentAppearancePath = obj.getAppearance()->getAppearanceTemplate()->getName();

		if ((obj.getAppearance() != NULL) &&
			(strcmp(currentAppearancePath, appearancePath.c_str()) != 0))
		{
			// Set the appropriate new appearance
			DEBUG_WARNING(ms_logAppearanceTabMessages, ("Appearance Item is wearable, but needs an appearance update. Altering appearance from [%s] to [%s].", currentAppearancePath, appearancePath.c_str()));
			obj.setAppearanceByName(appearancePath.c_str());
		}
	}
	else
		return false;

	return true;
}

void CreatureObjectNamespace::setCustomizationData(Object & wearer, Object & wornObject)
{
	SkeletalAppearance2 * wearerAppearance = dynamic_cast<SkeletalAppearance2*>(wearer.getAppearance());
    SkeletalAppearance2 * wornObjAppearance = dynamic_cast<SkeletalAppearance2*>(wornObject.getAppearance());

	if(!wearerAppearance || !wornObjAppearance)
		return;

	//-- get owner's customization data
	CustomizationData *const ownerCd = wearerAppearance->fetchCustomizationData();

	//-- get wearable's customization data
	CustomizationData *const wearableCd = wornObjAppearance->fetchCustomizationData();

	if(!ownerCd || !wearableCd)
		return;

	//-- make a link from wearable's /shared_owner directory to owner's /shared_owner/ customization variable directory
	IGNORE_RETURN(wearableCd->mountRemoteCustomizationData(*ownerCd, "/shared_owner/", "/shared_owner"));

	ownerCd->release();
	wearableCd->release();
}

void CreatureObjectNamespace::removeCustomizationData(Object & wearer, Object & wornObject)
{
	SkeletalAppearance2 * skeleAppearance = dynamic_cast<SkeletalAppearance2*>(wearer.getAppearance());
	SkeletalAppearance2 * wearableAppearance = dynamic_cast<SkeletalAppearance2*>(wornObject.getAppearance());

	if(!skeleAppearance || !wearableAppearance)
		return;

	//-- get owner's customization data
	CustomizationData *const ownerCd = skeleAppearance->fetchCustomizationData();

	//-- get wearable's customization data
	CustomizationData *const wearableCd = wearableAppearance->fetchCustomizationData();

	if(!ownerCd || !wearableCd)
		return;

	//-- remove the link from wearable's /shared_owner directory to owner's /shared_owner/ customization variable directory
	IGNORE_RETURN(wearableCd->dismountRemoteCustomizationData("/shared_owner"));

	ownerCd->release();
	wearableCd->release();
}

//----------------------------------------------------------------------

void CreatureObject::appearanceWearablesOnErase (const unsigned , const WearableEntry &w)
{
	{
		DEBUG_WARNING(ms_logAppearanceTabMessages, ("Character [%s] WearablesOnErase called.", getNetworkId().getValueString().c_str()));
	}

	if (this != Game::getPlayer () && getAppearanceInventoryObject()) // We only care about remote characters. Our local client is already taken care of.
	{
		{
			DEBUG_WARNING(ms_logAppearanceTabMessages, ("Character [%s] WearablesOnErase entered.", getNetworkId().getValueString().c_str()));
		}
         
		// Grab our appearance inventory. 
		SlottedContainer * container = ContainerInterface::getSlottedContainer(*getAppearanceInventoryObject());
		NOT_NULL(container);

		// Grab the object.
		ClientObject* clientObject = safe_cast<ClientObject*>(NetworkIdManager::getObjectById(w.m_networkId));
		if (clientObject)
		{
			Container::ContainerErrorCode tmp = Container::CEC_Success;
			SlottedContainmentProperty * scp = ContainerInterface::getSlottedContainmentProperty(*clientObject); // Get the items slotted containment property.
			int previousArrangement = scp ? scp->getCurrentArrangement() : -1; // Find out what arrangement/slot he's currently in.
			IGNORE_RETURN(container->remove(*clientObject, tmp)); // Remove the object.			
			if(scp && getInventoryObject()) // This should never fail, but we check to be sure.
				onContainedSlottedContainerChanged(*clientObject, *getInventoryObject(), 0/*Doesn't matter*/, previousArrangement); // This callback isn't called when the object is removed this way, so we have to manually construct the call.
			else
				checkWearingUnderWear(); // this is called by OnContainedSlottedContainerChanged as well. We just want to doubly make sure we have pants on.

			delete clientObject; // Finally delete the object.
		}
	}
}

//----------------------------------------------------------------------

void CreatureObject::appearanceWearablesOnInsert (const unsigned , const WearableEntry &w)
{
	{
		DEBUG_WARNING(ms_logAppearanceTabMessages, ("Character [%s] WearablesOnInsert called.", getNetworkId().getValueString().c_str()));
	}

	if (this != Game::getPlayer () && getAppearanceInventoryObject()) // Only care about remote players.
	{
		{
			DEBUG_WARNING(ms_logAppearanceTabMessages, ("Character [%s] WearablesOnInsert entered.", getNetworkId().getValueString().c_str()));
		}
		//Set up streamed wearables for non-player creatures
		SlottedContainer * container = ContainerInterface::getSlottedContainer(*getAppearanceInventoryObject()); // Grab our appearance inventory.
		NOT_NULL(container);

		Container::ContainerErrorCode tmp = Container::CEC_Success;

		ConstCharCrcString cc = ObjectTemplateList::lookUp(w.m_objectTemplate);
		if (cc.isEmpty())
		{
			WARNING(true, ("SceneCreateObjectByCrc crc %08x was not found in object template list", cc.getCrc()));
			return;
		}


		TangibleObject* const wearable = safe_cast<TangibleObject*> (ObjectTemplate::createObject (cc.getString())); // Create our object
		if (wearable)
		{
			wearable->setNetworkId(w.m_networkId);
			wearable->setAppearanceData(w.m_appearanceString);
			wearable->endBaselines();
			if(container->mayAdd(*wearable, w.m_arrangement, tmp)) // Can we add the item?
				container->add(*wearable, w.m_arrangement, tmp); // Add it.
			else
			{
				delete wearable;
				DEBUG_WARNING(ms_logAppearanceTabMessages, ("Could not add wearable %s, due to container error code %d.", cc.getString(), tmp));
			}
		}
		else
		{
			DEBUG_WARNING(ms_logAppearanceTabMessages, ("Failed to create appearance wearable %s", cc.getString()));
		}
	}
}

//----------------------------------------------------------------------

void CreatureObject::appearanceWearablesOnChanged ()
{
	{
		DEBUG_REPORT_PRINT(ms_logAppearanceTabMessages, ("Character [%s] WearablesOnChanged called.", getNetworkId().getValueString().c_str()));
	}
	// This should be only called if for some reason we change the appearance data of an item.
	std::vector<WearableEntry> wearables = m_wearableAppearanceData.get();
	for(std::vector<WearableEntry>::iterator i = wearables.begin(); i != wearables.end(); ++i)
	{
		Object * const o = NetworkIdManager::getObjectById(i->m_networkId);
		ClientObject * const co = o ? o->asClientObject() : NULL;
		TangibleObject * const wearable = co ? co->asTangibleObject() : NULL;
		if(wearable)
		{
			wearable->setAppearanceData(i->m_appearanceString);
		}
	}
}
