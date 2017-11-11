// ======================================================================
// 
// CreatureObject.h
// Portions Copyright 1999, Bootprint Entertainment, Inc.
// Portions Copyright 2000-2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_CreatureObject_H
#define INCLUDED_CreatureObject_H

// ======================================================================

#include "Archive/AutoDeltaMap.h"
#include "Archive/AutoDeltaSet.h"
#include "clientGame/TangibleObject.h"
#include "clientObject/ShadowBlobObject.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Timer.h"
#include "sharedGame/Buff.h"
#include "sharedGame/CraftingDataArchive.h"
#include "sharedGame/ShipChassisSlotType.h"
#include "sharedGame/WearableEntry.h"
#include "sharedMathArchive/VectorArchive.h"
#include "sharedSkillSystem/SkillObjectArchive.h"
#include "swgSharedUtility/Attributes.def"
#include "swgSharedUtility/Locomotions.def"

// ======================================================================

class CrcString;
class DataTable;
class PlayerObject;
class SharedCreatureObjectTemplate;
class SkillObject;
class ShipObject;
class WeaponObject;
class MovementTable;

template <class T>
class Watcher;

// ======================================================================

class CreatureObject : public TangibleObject
{
private:

	CreatureObject ();
	CreatureObject (const CreatureObject&);
	CreatureObject& operator= (const CreatureObject&);

public:

	enum eIncapacity
	{
		kActive = 0,
		kIncapacitated = 1,
		kDead = 2
	};

	enum Difficulty
	{
		  D_normal
		, D_elite
		, D_boss
	};

	typedef stdmap<std::string, int>::fwd                  ExperiencePointMap;
	typedef stdset<const SkillObject *>::fwd               SkillList;
	typedef stdvector<std::string>::fwd                    StringVector;
	typedef stdvector<std::pair<uint32, uint32> >::fwd     SchematicVector;
	typedef stdmap<std::string, std::pair<int, int> >::fwd SkillModMap;
	typedef std::pair<std::pair<NetworkId /*player*/, std::string /*name*/>, NetworkId /*ship*/> PlayerAndShipPair;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct Messages
	{
		struct AnimationMoodChanged;
		struct AnimatingSkillData;

		struct AdminChanged
		{
			typedef CreatureObject Payload;
		};

		struct CommandsChanged
		{
			typedef CreatureObject Payload;
		};

		struct CommandAdded
		{
			typedef std::pair<CreatureObject *, std::string> Payload;
		};

		struct CommandRemoved
		{
			typedef std::pair<CreatureObject *, std::string> Payload;
		};

		struct StatesChanged
		{
			typedef CreatureObject Payload;
		};

		struct ServerPostureChanged
		{
			typedef CreatureObject Payload;
		};

		struct VisualPostureChanged
		{
			typedef CreatureObject Payload;
		};

		struct ScaleFactorChanged;
		struct MoodChanged;

		struct IncapacityChanged
		{
			typedef CreatureObject Payload;
		};

		struct LookAtTargetChanged
		{
			typedef CreatureObject Payload;
		};

		struct IntendedTargetChanged
		{
			typedef CreatureObject Payload;
		};

		struct CurrentWeaponChanged
		{
			typedef CreatureObject Payload;
		};

		struct DamageTaken
		{
			typedef CreatureObject Payload;
		};

		struct GroupChanged
		{
			typedef CreatureObject Payload;
		};

		struct GroupInviterChanged
		{
			typedef CreatureObject Payload;
		};

		struct SkillModsChanged
		{
			typedef CreatureObject Payload;
		};

		struct SkillsChanged
		{
			typedef CreatureObject Payload;
		};

		struct BuffsChanged
		{
			typedef CreatureObject Payload;
		};

		struct PlayerSetup
		{
			typedef CreatureObject Payload;
		};

		struct MaxAttributesChanged
		{
			typedef CreatureObject Payload;
		};

		struct PerformanceTypeChanged;

		struct AlternateAppearanceSharedObjectTemplateNameChanged;

		struct CoverVisibilityChanged;

		struct HologramTypeChanged;

		struct VisibleOnMapAndRadarChanged;

		struct GroupMissionCriticalObjectsChanged
		{
			typedef CreatureObject Payload;
		};

		struct TotalLevelXpChanged
		{
			typedef CreatureObject Payload;
		};

		struct LevelChanged
		{
			typedef CreatureObject Payload;
		};

	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

public:
	explicit                   CreatureObject                  (const SharedCreatureObjectTemplate* newTemplate);
	virtual                   ~CreatureObject                  ();

	static void                install();
	static void                remove();
	static void                setDisablePostureRevert (bool disablePostureRevert);
	static int                 getNumberOfInstances ();

	virtual float              alter                           (float deltaTime);
	virtual void               containedByModified             (NetworkId const &oldValue, NetworkId const &newValue, bool isLocal);
	virtual void               arrangementModified             (int oldValue, int newValue, bool isLocal);
	virtual void               addedToContainer                (ClientObject &containedObject, int arrangement);
	virtual void               removedFromContainer            (ClientObject &containedObject);
	virtual void               doPostContainerChangeProcessing ();

	virtual void               createDefaultController ();
	virtual void               endBaselines            ();

	static CreatureObject const * asCreatureObject(Object const * object);
	static CreatureObject * asCreatureObject(Object * object);

	virtual CreatureObject *       asCreatureObject();
	virtual CreatureObject const * asCreatureObject() const;

	Attributes::Value          getAttribute              (Attributes::Enumerator attrib) const;
	Attributes::Value          getMaxAttribute           (Attributes::Enumerator attrib) const;
	Attributes::Value          getCurrentMaxAttribute    (Attributes::Enumerator attrib) const;
	Attributes::Value          getUnmodifiedMaxAttribute (Attributes::Enumerator attrib) const;
	int                        getShockWounds            () const;

	PlayerObject *             getPlayerObject         ();
	const PlayerObject *       getPlayerObject         () const;
	ClientObject*              getHairObject           ();
	const ClientObject*        getHairObject           () const;
	ClientObject*              getInventoryObject      ();
	const ClientObject*        getInventoryObject      () const;
	ClientObject*              getDatapadObject        ();
	const ClientObject*        getDatapadObject        () const;
	ClientObject *             getBankObject           ();
	const ClientObject *       getBankObject           () const;
	ClientObject *             getAppearanceInventoryObject ();
	const ClientObject *       getAppearanceInventoryObject () const;

	ClientObject*              getEquippedObject       (const char * const slotName);
	const ClientObject*        getEquippedObject       (const char * const slotName) const;

	ClientObject*              getAppearanceEquippedObject       (const char * const slotName);
	const ClientObject*        getAppearanceEquippedObject       (const char * const slotName) const;

	WeaponObject*              getCurrentWeapon        ();
	const WeaponObject*        getCurrentWeapon        () const;

	const std::string&         getCurrentPrimaryActionName() const;
	bool                       getPrimaryActionOverridden() const;
	bool                       getPrimaryActionIsLocationBased() const;
	bool                       getPrimaryActionWantsGroundReticule() const;

	float                      getMaximumTurnRate      (float currentSpeed, bool ignoreVehicle = false) const;
	float                      getMaximumAcceleration  (float currentSpeed, bool ignoreVehicle = false) const;
	float                      getMaximumWalkSpeed     (bool ignoreVehicle = false) const;
	float                      getMaximumRunSpeed      (bool ignoreVehilce = false) const;
	float                      getMaximumSpeed         (bool ignoreVehicle = false) const;
	float                      getMinimumSpeed         (bool ignoreVehicle = false) const;
	bool                       getCanStrafe            (bool ignoreVehicle = false) const;

	int8                       getVisualPosture            () const;
	void                       requestServerPostureChange  (int8 posture);
	void                       setVisualPosture            (int8 posture);

	void                       requestServerAttitudeChange (int8 attitude);
	int8                       getAttitude                 () const;

	bool                       getAlignToTerrain          () const;

	uint32                     getMood                    () const;
	uint32                     getSayMode                 () const;

	const std::string &        getAnimationMood           () const;
	void                       setAnimationMood           (const std::string &);

	void                       requestServerSetMood    (uint32 mood);
	void                       setSayMode              (uint32 mood);
	void                       clientSetMood           (uint32 mood);
	void                       clientSetMoodTemporary  (const uint32 mood, float duration);

	int                        getSpecies   () const;
	bool                       getSpeciesString(Unicode::String &species) const;
	int                        getGender    () const;

	const Unicode::String &    getLocalizedNameUnfiltered() const;

	bool                       isIncapacitated         () const;
	bool                       isDead                  () const;

	bool                       isBeast                 () const;

	CachedNetworkId const & getCombatTarget() const;
	const CachedNetworkId&     getLookAtTarget() const;
	const NetworkId&           getTargetUnderCursor() const;
	void                       setLookAtTarget(const NetworkId&);
	void                       setTargetUnderCursor(const NetworkId&);
	ShipChassisSlotType::Type  getLookAtTargetSlot() const;
	void                       setLookAtTargetSlot(ShipChassisSlotType::Type value);
	const CachedNetworkId &    getAssistTarget() const;	
	const CachedNetworkId&     getIntendedTarget() const;
	void                       setIntendedTarget(const NetworkId&);
	void                       setLookAtAndIntendedTarget(const NetworkId&);


	virtual bool               isTargettable           () const;

	const ExperiencePointMap & getExperiencePointMap   () const;
	const SkillList &          getSkills               () const;
	const SkillModMap &        getSkillModMap          () const;

	const std::map<std::pair<uint32,uint32>,int> & getDraftSchematics() const;
	bool                       hasCertificationsForItem(const TangibleObject & item) const;

	bool                       clientGrantSkill        (const SkillObject & skill);
	bool                       clientRevokeSkill       (const SkillObject & skill);                         
	void                       clientGrantExp          (const std::string & exptype, int amount);
	void                       clientResetAttribute    (Attributes::Enumerator attrib, Attributes::Value cur, Attributes::Value max);
	void                       clientGrantSchematic    (uint32 serverCrc, uint32 sharedCrc);
	void                       clientGrantCommand      (const std::string & command);
	void                       clientRevokeSchematic   (uint32 serverCrc, uint32 sharedCrc);
	void                       clientRevokeCommand     (const std::string & command);
	void                       clientSetMovementScale (float f);
	void                       clientSetAccelScale    (float f);
	void                       clientSetTurnPercent   (float f);
	
	bool                       hasSkill                (const SkillObject & skill) const;
	bool                       getExperience           (const std::string & expName, int & result) const;

	int                        getExperimentPoints     () const;
	Crafting::CraftingStage    getCraftingStage        () const;

	float                      getSlopeModAngle() const;
	float                      getBaseSlopeModPercent() const;
	float                      getSlopeModPercent() const;
	float                      getWaterModPercent() const;
	float                      getMovementScale() const;
	float                      getMovementPercent() const;
	float                      getTurnPercent() const;
	float                      getAccelScale() const;
	float                      getAccelPercent() const;
	float                      getRunSpeed(bool ignoreVehicle = false) const;
	float                      getWalkSpeed(bool ignoreVehicle = false) const;
	float                      getSwimHeight() const;
	float                      getCameraHeight() const;

	const int                  getModValue(const std::string & modName, bool capped=true) const;
	const int                  getEnhancedModValue(const std::string & modName) const;
//	void                       setModValue(const std::string & modName, const int value);

	float                      getScaleFactor          () const;
	void                       setScaleFactor          (float f);

	bool                       getState                (int8 whichState) const;
	uint64                     getStates               () const;
	void                       setState                (int8 whichState, bool b);
	void                       setStates               (uint64 states);
	CachedNetworkId const &    getGroup                () const;
	PlayerAndShipPair const &  getGroupInviter         () const;
	bool                       isGroupedWith           (const CreatureObject & creature) const;

	// this method returns a network id for the inviter that the
	// currently running game will know about.  For the ground game
	// this will be the CreatureObject.  For the space game, this
	// will be the ShipObject
	NetworkId const &          getAppropriateGroupInviterBasedOffSceneType() const;
	std::string const &        getGroupInviterName     () const;
	void                       targetGroupMember       (int index);

	int                        getPerformanceType      () const;
	NetworkId const &          getPerformanceListenTarget () const;
	int                        getPerformanceStartTime () const;
	int                        getGuildId              () const;
	NetworkId const &          getMasterId             () const;
	
	void                       signalAnimationMoodModified (const std::string &newValue);

	bool                       isAppearanceWearable        (const Object &object) const;

	void                       getAccumulatedHamDamage     (int & health, int & action, int & mind) const;

	void                       setAppearanceHeldItemState  ();

	void                       setSittingOnObject          (bool isSittingOnObject);
	bool                       isSittingOnObject           () const;

	void                       setPlayerSpecificCallbacks  ();

	void                       getDefenderDisposition      (bool &isInCombat, bool &isAttacker, int &defenderDisposition, int &attackNameCrc) const;

	virtual uint32             getPvpFlags                 () const;
	virtual int                getPvpType                  () const;
	virtual uint32             getPvpFaction               () const;

	float                      getWarpTolerance            () const;

	void                       setLevel                    (int16);
	int16                      getLevel                    () const;
	int16                      getGroupLevel               () const;
	int                        getLevelXp() const;

	void                       setPlaybackScriptIsAssignedToIncapacitateMe(bool isAssigned);
	bool                       isPlaybackScriptAssignedToIncapacitateMe() const;

	static std::string         getRankString(uint8 factionRank);
	static Unicode::String     getLocalizedRankString(uint8 factionRank);
	static Unicode::String     getLocalizedGcwRankString(int gcwFactionRank, uint32 factionCrc);
	
	TangibleObject            *getClosestChair();

	// Mount functionality.
	bool                       isRidingMount() const;
	bool                       isMountForThisClientPlayer() const;
	bool                       isMountForAndDrivenByClientPlayer() const;
	CreatureObject            *getMountedCreature();
	CreatureObject const      *getMountedCreature() const;
	CreatureObject            *getRiderDriverCreature();
	CreatureObject const      *getRiderDriverCreature() const;

	void                       fixupMountedPlayerOnCmdSceneReady();
	void                       riderDoDismountVisuals(NetworkId const &oldMountId);

	void                       doScaleChangeFixupsForMount();

	stdvector<WearableEntry>::fwd const & getWearableData() const;
	void                       setWearableData(stdvector<WearableEntry>::fwd const & wearableData);

	stdvector<WearableEntry>::fwd const & getAppearanceWearableData() const;

	// Ship functionality
	int                        getShipStation() const;
	void                       updateShipStation();
	ShipObject                *getPilotedShip();
	ShipObject const          *getPilotedShip() const;
	bool                       getCoverVisibility() const;
	ShipObject                *getContainingShip();
	ShipObject const          *getContainingShip() const;
	bool                       isAPilot() const;
	
	bool isGroupMissionCriticalObject(NetworkId const & networkId) const;
	typedef std::set<std::pair<NetworkId, NetworkId> > GroupMissionCriticalObjectSet;
	GroupMissionCriticalObjectSet const & getGroupMissionCriticalObjects() const;

	bool getClientUsesAnimationLocomotion() const;
	void addSlowDownEffect(const TangibleObject & defender, float coneLength, float coneAngle, float slopeAngle, unsigned long expireTime);
	void removeSlowDownEffect();
	void addTerrainSlopeEffect(const Vector & normal);

	void setCoverVisibility(bool const visible, bool const islocal);

	void   getBuffs(stdvector<Buff>::fwd & buffs) const;
	void   getBuffs(stdmap<uint32, Buff>::fwd & buffs) const;
	uint32 getPlayedTime() const;
	void addBuffDebug(uint32 const buffHash, uint32 const timestamp, float const value);
	void removeBuffDebug(uint32 const buffHash);

	unsigned long getLastWaterDamageTimeMs();
	void setLastWaterDamageTimeMs(unsigned long time);

	Difficulty getDifficulty() const;
	
	int32 getHologramType() const;

	bool getVisibleOnMapAndRadar() const;

	bool getForceShowHam() const;

	virtual void getObjectInfo(stdmap<std::string, stdmap<std::string, Unicode::String>::fwd >::fwd & propertyMap) const;

	stdmap<std::string, int>::fwd const & getCommands() const;
	bool hasCommand(std::string const & cmdName) const;
	void commandsOnErase(const std::string & cmd, const int & value);
	void commandsOnInsert(const std::string & cmd, const int & value);

	bool isAiming() const;
	
	Locomotions::Enumerator getLocomotion() const;
	int getNiche() const;

	bool hasBuff(uint32 buffNameCrc) const;
	bool hasInvisibilityBuff(int & buffPriority) const;
	
	float getLookAtYaw() const;
	bool getUseLookAtYaw() const;
	void setLookAtYaw(float lookAtYaw, bool useLookAtYaw);

	void onContainedSlottedContainerChanged(ClientObject &containedObject, ClientObject &newContainer, int arrangement, int oldArrangement);

#ifdef _DEBUG
	void setMovementPercent(float percent);
#endif
	bool isAppearanceWearable     (const Object &object, std::string *appearancePath, bool *prohibitAttachmentVisuals = 0 ) const;

	void setDupedCreaturesDirty(bool dirty);
	bool getDupedCreaturesDirty() const;

	NetworkId const & getDecoyOrigin() const;

protected:

	void                      filterLocalizedName          (Unicode::String &) const;
	virtual void              conditionModified            (int oldCondition, int newCondition);

private:

	struct Callbacks
	{
		template <typename S, typename U> struct DefaultCallback
		{
			void modified (CreatureObject & target, const U & old, const U & value, bool local) const;
		};

		typedef DefaultCallback<Messages::IncapacityChanged,      int>              IncapacityChanged;
		typedef DefaultCallback<Messages::MoodChanged,            uint8>            MoodChanged;
		typedef DefaultCallback<Messages::LookAtTargetChanged,    CachedNetworkId>  LookAtTargetChanged;
		typedef DefaultCallback<Messages::IntendedTargetChanged,    CachedNetworkId>  IntendedTargetChanged;
		typedef DefaultCallback<Messages::CurrentWeaponChanged,    CachedNetworkId>  CurrentWeaponChanged;
		typedef DefaultCallback<Messages::ScaleFactorChanged,     float>            ScaleFactorChanged;
		typedef DefaultCallback<Messages::StatesChanged,          uint64>           StatesChanged;
		typedef DefaultCallback<Messages::ServerPostureChanged,   int8>             ServerPostureChanged;
		typedef DefaultCallback<Messages::VisualPostureChanged,   int8>             VisualPostureChanged;
		typedef DefaultCallback<Messages::AnimationMoodChanged,   std::string>      AnimationMoodChanged;
		typedef DefaultCallback<Messages::GroupChanged,           CachedNetworkId>  GroupChanged;
		typedef DefaultCallback<Messages::PerformanceTypeChanged, int>              PerformanceTypeChanged;
		typedef DefaultCallback<Messages::AnimatingSkillData,     std::string>      AnimatingSkillDataChanged;
		typedef DefaultCallback<Messages::AlternateAppearanceSharedObjectTemplateNameChanged, std::string>  AlternateAppearanceSharedObjectTemplateNameChanged;
		typedef DefaultCallback<Messages::GroupInviterChanged,    PlayerAndShipPair> GroupInviterChanged;
		typedef DefaultCallback<Messages::CoverVisibilityChanged, bool>             CoverVisibilityChanged;
		typedef DefaultCallback<Messages::TotalLevelXpChanged, int>                 TotalLevelXpChanged;
		typedef DefaultCallback<Messages::LevelChanged, int>                        LevelChanged;
		typedef DefaultCallback<Messages::HologramTypeChanged, int32> HologramTypeChanged;
		typedef DefaultCallback<Messages::VisibleOnMapAndRadarChanged, bool> VisibleOnMapAndRadarChanged;
	};

	//-----------------------------------------------------------------------
	
	void checkWearing             (Object **underWearObject, StringVector const &slots, const char *objectTemplateName);
	void checkWearingUnderWear    ();
	
	void skillModsOnSet           (const std::string & key, const std::pair<int, int> & oldValue, const std::pair<int, int> & value);
	void skillModsOnInsert        (const std::string & key, const std::pair<int, int> & value);
	void skillModsOnErase         (const std::string & key, const std::pair<int, int> & value);

	void skillsOnChanged          ();
	void skillsOnErase            (const unsigned int, const SkillObject * &);
	void skillsOnInsert           (const unsigned int, const SkillObject * &);
	void skillsOnSet              (const unsigned int, const SkillObject * &, const SkillObject * &);

	void wearablesOnErase         (const unsigned , const WearableEntry &w);
	void wearablesOnInsert        (const unsigned , const WearableEntry &w);
	void wearablesOnChanged       ();

	void appearanceWearablesOnErase         (const unsigned , const WearableEntry &w);
	void appearanceWearablesOnInsert        (const unsigned , const WearableEntry &w);
	void appearanceWearablesOnChanged       ();

	void buffsOnErase         (const uint32 &, const Buff::PackedBuff &);
	void buffsOnInsert        (const uint32 &, const Buff::PackedBuff &);
	void buffsOnSet           (const uint32 &, const Buff::PackedBuff &, const Buff::PackedBuff &);
	
	void maxAttributesOnChanged   ();

	void attributesOnSet          (const size_t elem, const Attributes::Value & oldValue, const Attributes::Value & newValue);

	void setAppearanceTransformModifiers ();

	void handleAttachWearForContainerAdd(ClientObject &containedObject, int arrangement);
	void handleDetachUnwearForContainerRemove(ClientObject &containedObject);

	void updateRidingMountStatus();

	SharedCreatureObjectTemplate const *fetchEffectiveObjectTemplate () const;
	void setEffectiveAppearance();

	void groupMissionCriticalObjectsOnChanged();

	// Mounts: called on the rider.
	void onRiderMountedMount();
	void onRiderDismountedMount(NetworkId const &oldMountId);

	// Ships: called on the pilot.
	void onEnteredPilotStation();
	void onLeftPilotStation(NetworkId const &oldContainerId);

	// Ships: called on the droid
	void onEnteredDroidStation();
	void onLeftDroidStation(NetworkId const &oldContainerId);

	// Ships: called on the operatey-type-person
	void onEnteredOperationsStation();
	void onLeftOperationsStation(NetworkId const &oldContainerId);

	// Ships: called on the gunner
	void onEnteredGunnerStation(int newWeaponIndex);
	void onLeftGunnerStation(NetworkId const &oldContainerId, int oldWeaponIndex);

	void verifyWornAppearanceItems();

	// Mounts: called on the mount.  Do not call these directly!  Must be public because of the auto delta callback callsite.
public:
	void onMountRiderMounted();
	void onMountRiderDismounted();

	void setupAlternateSharedCreatureObjectTemplate(CrcString const &templateName);

private:

	// Mounts: called on mount when it becomes mountable/non-mountable.
	void onJustBecameMountable();
	void onJustBecameNonmountable();

	// ----------------------------------------------------------------------

	float _getMountedMaximumTurnRate(float currentSpeed) const;
	float _getUnmountedMaximumTurnRate(const float currentSpeed) const;
	float _getMaximumTurnRate(float currentSpeed, bool mounted) const;

	// ----------------------------------------------------------------------

	bool _getMountedMaximumAcceleration(float &o_rate, float currentSpeed) const;

	// ----------------------------------------------------------------------

	float _getMountedMovementScale() const;
	float _getMovementScale(bool mounted) const;

	// ----------------------------------------------------------------------

	float _getMountedMovementPercent() const;
	float _getMovementPercent(bool mounted) const;

	// ----------------------------------------------------------------------

	float _adjustMovementSpeed(float speed, bool mounted) const;

	// ----------------------------------------------------------------------

	float _getMountedWalkSpeed() const;
	float _getUnmountedWalkSpeed() const;
	float _getWalkSpeed(bool mounted) const;

	// ----------------------------------------------------------------------

	float _getMountedRunSpeed() const;
	float _getUnmountedRunSpeed() const;
	float _getRunSpeed(bool mounted) const;

	// ----------------------------------------------------------------------

	float _getMountedMinimumSpeed() const;
	float _getUnmountedMinimumSpeed() const;
	float _getMinimumSpeed(bool mounted) const;

	// ----------------------------------------------------------------------

	const MovementTable *_getMovementTable() const;

private:

	NetworkId		m_targetUnderCursor;
	friend Callbacks::AnimationMoodChanged;
	friend Callbacks::ScaleFactorChanged;
	friend Callbacks::MoodChanged;
	friend Callbacks::ServerPostureChanged;
	friend Callbacks::VisualPostureChanged;

	Archive::AutoDeltaVariableCallback<int8, Callbacks::VisualPostureChanged, CreatureObject>  m_visualPosture;        ///< The posture currently displayed for the mobile.
	Archive::AutoDeltaVariableCallback<int8, Callbacks::ServerPostureChanged, CreatureObject>  m_serverPosture;        ///< The posture of the mobile on the server.
	float                                                                                      m_revertToServerPostureTimer;

	Object *                                                                   m_underWearChestObject;
	Object *                                                                   m_underWearPantsObject;

	Archive::AutoDeltaVector          <Attributes::Value, CreatureObject>      m_attributes;                                   ///< The current attributes (health, action, mind) of the mobile.
	Archive::AutoDeltaVector          <Attributes::Value, CreatureObject>      m_maxAttributes;                                ///< The max unaffected attributes of this mobile.
	Archive::AutoDeltaVector          <Attributes::Value>                      m_unmodifiedMaxAttributes;                      ///< The max unaffected attributes of this mobile, unmodified by attrib mods.
	Archive::AutoDeltaVector          <Attributes::Value>                      m_attribBonus;                                  ///< Bonus from items that are added to the max attrib values
	Archive::AutoDeltaVariable        <int>                                    m_shockWounds;                                  ///< Shock wounds taken
	Archive::AutoDeltaVariableCallback<uint8,             Callbacks::MoodChanged, CreatureObject>
	                                                                           m_mood;            ///< The mood the mobile is in.

	Archive::AutoDeltaVariable        <uint8>                                  m_sayMode;         ///< The current default say text that a character uses when speaking.

	Archive::AutoDeltaVariableCallback<std::string,       Callbacks::AnimationMoodChanged, CreatureObject>
	                                                                           m_animationMood; ///< The current animation mood. @todo this probably can/should be unified with m_mood, talk to John Watson.

	Archive::AutoDeltaVariableCallback<CachedNetworkId,   Callbacks::LookAtTargetChanged, CreatureObject>
	                                                                           m_lookAtTarget;    ///< This creature's current target	
	ShipChassisSlotType::Type                                                  m_lookAtTargetSlot; ///< This creature's targeted component, not synchronized

	Archive::AutoDeltaVariableCallback<CachedNetworkId,   Callbacks::IntendedTargetChanged, CreatureObject>
																	   		   m_intendedTarget;    // This creature's intended target

	Archive::AutoDeltaVariableCallback<CachedNetworkId,   Callbacks::CurrentWeaponChanged, CreatureObject>  
		                                                                       m_currentWeapon; ///< current weapon creature is using in combat

	Archive::AutoDeltaSet <const SkillObject *, CreatureObject >               m_skills;

	Archive::AutoDeltaVariable<float>                                          m_movementScale;     // scale to the creature's base movement rate
	Archive::AutoDeltaVariable<float>                                          m_movementPercent;   // script-defined mod to the creature's base movement rate
	Archive::AutoDeltaVariable<float>                                          m_walkSpeed;         // scale to the creature's base movement rate
	Archive::AutoDeltaVariable<float>                                          m_runSpeed;          // scale to the creature's base movement rate
	Archive::AutoDeltaVariable<float>                                          m_accelScale;        // scale to the creature's base acceleration rate
	Archive::AutoDeltaVariable<float>                                          m_accelPercent;      // script-defined mod to the creature's acceleration rate
	Archive::AutoDeltaVariable<float>                                          m_turnScale;         // scale to the creature's base turn rate
	Archive::AutoDeltaVariable<float>                                          m_slopeModAngle;     // angle at which the creature starts to slow down
	Archive::AutoDeltaVariable<float>                                          m_slopeModPercent;   // scale to the creature's base movement rate for a 90 degree slope
	Archive::AutoDeltaVariable<float>                                          m_waterModPercent;   // scale to the creature's base movement rate for movement in water
	Archive::AutoDeltaSet<std::pair<NetworkId, NetworkId>, CreatureObject> m_groupMissionCriticalObjectSet;

	Archive::AutoDeltaVariableCallback<float, Callbacks::ScaleFactorChanged, CreatureObject>             m_scaleFactor;             // amount to scale the creature's appearance by
	Archive::AutoDeltaVariableCallback<uint64, Callbacks::StatesChanged, CreatureObject>                 m_states;

	// pair: unmodified skill mod, bonus from equipped objects
	Archive::AutoDeltaMap<std::string, std::pair<int, int>, CreatureObject>                                                                           m_modMap;
	Archive::AutoDeltaVariableCallback<CachedNetworkId, Callbacks::GroupChanged,        CreatureObject>  m_group;
	Archive::AutoDeltaVariableCallback<PlayerAndShipPair, Callbacks::GroupInviterChanged, CreatureObject>m_groupInviter;
	Archive::AutoDeltaVariableCallback<int, Callbacks::PerformanceTypeChanged, CreatureObject>           m_performanceType;
	Archive::AutoDeltaVariable<int>                                                                      m_performanceStartTime;
	Archive::AutoDeltaVariable<NetworkId>                                                                m_performanceListenTarget;
	Archive::AutoDeltaVariable<int>                                                                      m_guildId;
	Archive::AutoDeltaVariable<NetworkId>                                                                m_masterId;

	// ***DEPRECATED*** use PlayerObject::m_currentGcwRank instead
	Archive::AutoDeltaVariable<uint8>                                          m_rank;
	
	int16                                                                      m_accumulatedHamDamage; // the health damage accumulated since the last alter

	uint8                                                                      m_oldMood;
	bool                                                                       m_temporaryMoodActive : 1;
	float                                                                      m_temporaryMoodDuration;
	bool                                                                       m_settingTemporaryMood : 1;

	bool                                                                       m_isSittingOnObject : 1;

	bool                                                                       m_hasHeldStateObject : 1;
	Watcher<Object>                                                           *m_heldStateObjectWatcher;

	/// Specifies the string used by the client to determine what to do when in the Postures::SkillAnimating posture.
	Archive::AutoDeltaVariableCallback<std::string, Callbacks::AnimatingSkillDataChanged, CreatureObject> m_animatingSkillData;

	Archive::AutoDeltaVariableCallback<int16, Callbacks::LevelChanged, CreatureObject> m_level;
	Archive::AutoDeltaVariable<int>                                            m_levelHealthGranted;
	Archive::AutoDeltaVariableCallback<int, Callbacks::TotalLevelXpChanged, CreatureObject> m_totalLevelXp;
	Archive::AutoDeltaVector<WearableEntry, CreatureObject>                    m_wearableData;
	Archive::AutoDeltaVariableCallback<std::string, Callbacks::AlternateAppearanceSharedObjectTemplateNameChanged, CreatureObject>  m_alternateAppearanceSharedObjectTemplateName;
	Archive::AutoDeltaVariableCallback<bool, Callbacks::CoverVisibilityChanged, CreatureObject>  m_coverVisibility;
	Timer                                                                      m_playbackScriptNoIncapacitationTimer;
	bool                                                                       m_isPlaybackScriptScheduledToIncapacitate : 1;
	bool                                                                       m_hasAltered : 1;
	bool                                                                       m_isRidingMount : 1;
	bool                                                                       m_isMountForThisClientPlayer : 1;
	const float                                                                m_turnRateMaxWalk;
	const float                                                                m_turnRateMaxRun;
	const float                                                                m_accelerationMaxWalk;
	const float                                                                m_accelerationMaxRun;
	const float                                                                m_warpTolerance;
	int16                                                                      m_shipStation;
	unsigned long                                                              m_lastWaterDamageTimeMs;

	Watcher<Object>                                                            m_healingAttachedObject;

	SharedCreatureObjectTemplate const                                        *m_alternateSharedCreatureObjectTemplate;

	Archive::AutoDeltaVariable<bool>                                           m_clientUsesAnimationLocomotion;

	//Buffs
	Archive::AutoDeltaMap<uint32, Buff::PackedBuff, CreatureObject>            m_buffs;
	Archive::AutoDeltaVariable<unsigned char>                                  m_difficulty;
	Archive::AutoDeltaMap<std::string, int, CreatureObject> m_commands; // game commands the creature may execute
	Watcher<ShadowBlobObject>                                                  m_shadowBlob;

	Archive::AutoDeltaVariableCallback<int32,Callbacks::HologramTypeChanged, CreatureObject> m_hologramType;

	Archive::AutoDeltaVariableCallback<bool,Callbacks::VisibleOnMapAndRadarChanged, CreatureObject> m_visibleOnMapAndRadar;
	
	// Set an aiming flag in combat.
	bool m_isAiming : 1;

	// -- fields relating to the ability to look at a position instead of a target
	float                                                                      m_lookAtYaw;
	bool                                                                       m_useLookAtYaw;
	Watcher<Object>                                                            m_localLookAtPositionObject;

	mutable Unicode::String         m_localizedNameUnfiltered;

	Archive::AutoDeltaVariable<bool>										   m_isBeast;
	Archive::AutoDeltaVariable<bool>                                           m_forceShowHam;

	Archive::AutoDeltaVector<WearableEntry, CreatureObject>                    m_wearableAppearanceData;

	bool																	   m_initAppearanceWearables;

	Timer																	   m_verifyAppearanceTimer;

	bool                                                                       m_dupedCreaturesDirty;

	Archive::AutoDeltaVariable<NetworkId>									   m_decoyOrigin;
};


// ======================================================================

inline Attributes::Value CreatureObject::getAttribute(Attributes::Enumerator attrib) const
{
	return m_attributes.get(attrib);
}

//------------------------------------------------------------------------

inline Attributes::Value CreatureObject::getMaxAttribute(Attributes::Enumerator attrib) const
{
	return m_maxAttributes.get(attrib);
}

//------------------------------------------------------------------------

inline Attributes::Value CreatureObject::getCurrentMaxAttribute(Attributes::Enumerator attrib) const
{
	return static_cast<Attributes::Value> (m_maxAttributes.get (attrib));
}

//----------------------------------------------------------------------

inline Attributes::Value CreatureObject::getUnmodifiedMaxAttribute (Attributes::Enumerator attrib) const
{
	// @NOTE: special case for health attribute adds level health
	return (Attributes::Health == attrib) ? m_unmodifiedMaxAttributes.get(attrib) + m_levelHealthGranted.get() : m_unmodifiedMaxAttributes.get(attrib);
}

//----------------------------------------------------------------------

inline int CreatureObject::getShockWounds() const
{
	return m_shockWounds.get ();
}

//------------------------------------------------------------------------

inline int8 CreatureObject::getVisualPosture() const
{
	return m_visualPosture.get ();
}


//----------------------------------------------------------------------

inline uint32  CreatureObject::getSayMode() const
{
	return static_cast<uint32>(m_sayMode.get());
}

//----------------------------------------------------------------------

inline const std::string & CreatureObject::getAnimationMood() const
{
	return m_animationMood.get();
}

//----------------------------------------------------------------------

inline const CachedNetworkId& CreatureObject::getLookAtTarget() const
{
	return m_lookAtTarget.get();
}

//----------------------------------------------------------------------

inline const NetworkId& CreatureObject::getTargetUnderCursor() const
{
	return m_targetUnderCursor;
}

//----------------------------------------------------------------------

inline const CachedNetworkId& CreatureObject::getIntendedTarget() const
{
	return m_intendedTarget.get();
}

//----------------------------------------------------------------------

inline const CreatureObject::SkillList & CreatureObject::getSkills () const
{
	return m_skills.get ();
}

//--------------------------------------------------------------------

inline const CreatureObject::SkillModMap & CreatureObject::getSkillModMap () const
{
	return m_modMap.getMap ();
}

//--------------------------------------------------------------------

inline float CreatureObject::getSlopeModAngle() const
{
	return m_slopeModAngle.get();
}

//--------------------------------------------------------------------

inline float CreatureObject::getSlopeModPercent() const
{
	return m_slopeModPercent.get();
}

//--------------------------------------------------------------------

inline float CreatureObject::getWaterModPercent() const
{
	return m_waterModPercent.get();
}

//--------------------------------------------------------------------

inline float CreatureObject::getTurnPercent() const
{
	return m_turnScale.get();
}

// ---------------------------------------------------------------------

inline float CreatureObject::getAccelScale() const
{
	return m_accelScale.get();
}

//--------------------------------------------------------------------

inline float CreatureObject::getAccelPercent() const
{
	return m_accelPercent.get();
}

//----------------------------------------------------------------------

inline float CreatureObject::getScaleFactor () const
{
	return m_scaleFactor.get();
}

//----------------------------------------------------------------------

inline CachedNetworkId const &CreatureObject::getGroup () const
{
	return m_group.get ();
}

//----------------------------------------------------------------------

inline CreatureObject::PlayerAndShipPair const & CreatureObject::getGroupInviter() const
{
	return m_groupInviter.get ();
}

// ----------------------------------------------------------------------

inline int CreatureObject::getPerformanceType () const
{
	return m_performanceType.get ();
}

// ----------------------------------------------------------------------

inline NetworkId const &CreatureObject::getPerformanceListenTarget () const
{
	return m_performanceListenTarget.get ();
}

// ----------------------------------------------------------------------

inline int CreatureObject::getPerformanceStartTime () const
{
	return m_performanceStartTime.get ();
}

// ----------------------------------------------------------------------

inline int CreatureObject::getGuildId () const
{
	return m_guildId.get ();
}

// ----------------------------------------------------------------------

inline NetworkId const & CreatureObject::getMasterId () const
{
	return m_masterId.get ();
}

// ----------------------------------------------------------------------
/**
 * Set the "sitting on object" state for this instance.
 *
 * When set to true, the game's sitting posture will map to the animation
 * system posture for sitting on a chair.  When set to false, the game's 
 * sitting posture will map to the sitting on ground animation.
 *
 * @param isSittingOnObject  indicates whether this creature instance is 
 *                           sitting on a chair-like object.
 */

inline void CreatureObject::setSittingOnObject (bool isSittingOnObject)
{
	m_isSittingOnObject = isSittingOnObject;
}

// ----------------------------------------------------------------------

inline bool CreatureObject::isSittingOnObject () const
{
	return m_isSittingOnObject;
}

//----------------------------------------------------------------------

inline int16 CreatureObject::getLevel() const
{
	return m_level.get ();
}

// ----------------------------------------------------------------------

inline int CreatureObject::getLevelXp() const
{
	return m_totalLevelXp.get();
}

//----------------------------------------------------------------------

inline bool CreatureObject::getClientUsesAnimationLocomotion() const
{
	return m_clientUsesAnimationLocomotion.get();
}

//----------------------------------------------------------------------

inline uint64 CreatureObject::getStates() const
{
	return m_states.get();
}

//----------------------------------------------------------------------

inline void CreatureObject::setStates(uint64 states)
{
	m_states = states;
}

//------------------------------------------------------------------------

inline float CreatureObject::getMaximumWalkSpeed (bool ignoreVehicle) const
{
    return getWalkSpeed(ignoreVehicle);
}

// ----------------------------------------------------------------------

inline bool CreatureObject::isMountForThisClientPlayer() const
{
	return m_isMountForThisClientPlayer;
}

// ----------------------------------------------------------------------

inline bool CreatureObject::isRidingMount() const
{
	return m_isRidingMount;
}

// ----------------------------------------------------------------------

inline int CreatureObject::getShipStation() const
{
	return m_shipStation;
}

//----------------------------------------------------------------------

inline void CreatureObject::clientSetMovementScale (float f)
{
	m_movementScale = f;
}

//----------------------------------------------------------------------

inline void CreatureObject::clientSetAccelScale    (float f)
{
	m_accelScale = f;
}

//----------------------------------------------------------------------

inline void CreatureObject::clientSetTurnPercent   (float f)
{
	m_turnScale = f;
}

//----------------------------------------------------------------------

inline std::vector<WearableEntry> const & CreatureObject::getWearableData() const
{
	return m_wearableData.get();
}


//----------------------------------------------------------------------

inline std::vector<WearableEntry> const & CreatureObject::getAppearanceWearableData() const
{
	return m_wearableAppearanceData.get();
}

// ----------------------------------------------------------------------

inline ShipChassisSlotType::Type CreatureObject::getLookAtTargetSlot() const
{
	return m_lookAtTargetSlot;
}

//----------------------------------------------------------------------

inline CreatureObject::GroupMissionCriticalObjectSet const & CreatureObject::getGroupMissionCriticalObjects() const
{
	return m_groupMissionCriticalObjectSet.get();
}

// ----------------------------------------------------------------------

inline bool CreatureObject::getCoverVisibility() const
{
	return m_coverVisibility.get();
}

//------------------------------------------------------------------------

inline float CreatureObject::getMaximumRunSpeed (bool ignoreVehicle) const
{
    return getRunSpeed(ignoreVehicle);
}

//----------------------------------------------------------------------

inline float CreatureObject::getWarpTolerance() const
{
	return m_warpTolerance;
}

//----------------------------------------------------------------------

inline void CreatureObject::setLevel(int16 level)
{
	m_level = level;
}

// ----------------------------------------------------------------------

inline bool CreatureObject::isPlaybackScriptAssignedToIncapacitateMe() const
{
	return m_isPlaybackScriptScheduledToIncapacitate;
}

// ----------------------------------------------------------------------

inline unsigned long CreatureObject::getLastWaterDamageTimeMs()
{
	return m_lastWaterDamageTimeMs;
}

// ----------------------------------------------------------------------

inline int32 CreatureObject::getHologramType() const
{
	return m_hologramType.get();
}

// ----------------------------------------------------------------------

inline bool CreatureObject::getVisibleOnMapAndRadar() const
{
	return m_visibleOnMapAndRadar.get();
}

//----------------------------------------------------------------------

inline bool CreatureObject::getForceShowHam() const
{
	return m_forceShowHam.get();
}

//----------------------------------------------------------------------

inline void CreatureObject::setDupedCreaturesDirty(bool dirty)
{
	m_dupedCreaturesDirty = dirty;
}

//----------------------------------------------------------------------

inline bool CreatureObject::getDupedCreaturesDirty() const
{
	return m_dupedCreaturesDirty;
}

//----------------------------------------------------------------------

inline NetworkId const & CreatureObject::getDecoyOrigin() const
{
	return m_decoyOrigin.get();
}

// ======================================================================

#endif
