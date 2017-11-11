// ======================================================================
//
// ShipObject.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ShipObject_H
#define INCLUDED_ShipObject_H

// ======================================================================

#include "archive/AutoDeltaMap.h"
#include "archive/AutoDeltaPackedMap.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/TangibleObject.h"
#include "clientGame/ShipWeaponStatus.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/BitArray.h"
#include "sharedGame/ShipChassisSlotType.h"

// ======================================================================

class ClientDataFile;
class ClientEffect;
class CrcString;
class CreateMissileMessage;
class InterpolatedSoundRuntime;
class Plane;
class SharedShipObjectTemplate;
class ShipComponentData;
class ShipObjectAttachments;
class ShipObjectEffects;
class SoundTemplate;
class VehicleThrusterSoundRuntimeData;

template <typename T> class Watcher;

// ======================================================================

class ShipObject: public TangibleObject
{
public:

	static void install();
	static int getNumberOfInstances();

public:

	struct MissileUpdate;

	struct Messages
	{
		struct ComponentsChanged
		{
			typedef ShipObject Payload;
		};

		struct TargetAcquisition
		{
			typedef ShipObject Payload;
		};

		struct LookAtTargetChanged
		{
			typedef ShipObject Payload;
		};

		struct LookAtTargetSlotChanged
		{
			typedef int Payload;
		};

		struct TargetableSlotBitfieldChanged
		{
			typedef bool Payload;
		};

		struct PilotMounted
		{
			typedef ShipObject Payload;
		};

		struct PilotDismounted
		{
			typedef ShipObject Payload;
		};

		struct GunnerMounted
		{
			typedef ShipObject Payload;
		};

		struct GunnerDismounted
		{
			typedef ShipObject Payload;
		};

		struct DroidPcdChanged
		{
			typedef ShipObject Payload;
		};

		struct ShipIdChanged
		{
			typedef ShipObject Payload;
		};

		struct CargoChanged
		{
			typedef ShipObject Payload;
		};

	};

public:
	enum ShipLaunchable
	{
		SL_noReactor,
		SL_reactorDisabled,
		SL_reactorDemolished,
		SL_noEngine,
		SL_engineDisabled,
		SL_engineDemolished,
		SL_damaged,
		SL_pristine
	};

	typedef Watcher<Object> ObjectWatcher;
	typedef stdvector<ObjectWatcher>::fwd   WatcherVector;
	typedef stdvector<ShipObject *>::fwd ShipVector;
	typedef stdvector<int>::fwd IntVector;
	typedef stdvector<ClientDataFile const *>::fwd ClientDataFileVector;
	typedef stdvector<Plane>::fwd PlaneVector;
	typedef stdmap<NetworkId, int>::fwd NetworkIdIntMap;
	typedef std::pair<Unicode::String, std::string> ResourceTypeInfoPair;
	typedef stdmap<NetworkId, ResourceTypeInfoPair>::fwd ResourceTypeInfoMap;

	explicit ShipObject(SharedShipObjectTemplate const *newTemplate);
	virtual ~ShipObject();

	static ShipObject *findShipByShipId(uint16 shipId);
	static ShipObject *getContainingShip(ClientObject &obj);
	static ShipObject const *getContainingShip(ClientObject const &obj);
	static Object &getShipTurretMuzzle(Object &turret);
	static Object const &getShipTurretMuzzle(Object const &turret);

	bool getTurretYawAndPitchTo(int weaponIndex, Vector const &targetPosition_w, float &yaw, float &pitch) const;
	void orientShipTurret(int weaponIndex, float &yaw, float &pitch, bool inversePitch = false);
	void orientShipTurret(int weaponIndex, Vector const &targetPosition_w);

	void attachDisplayObjects();

	virtual float alter(float elapsedTime);

	virtual ShipObject * asShipObject();
	virtual ShipObject const * asShipObject() const;
	virtual InteriorLayoutReaderWriter const * getInteriorLayout() const;

	void setPilot(CreatureObject * pilot);
	CreatureObject const * getPilot() const;
	CreatureObject * getPilot();
	void findAllPassengers(stdvector<CreatureObject *>::fwd & passengers) const;
	void onShipPilotMounted(CreatureObject *pilot);
	void onShipPilotDismounted();
	void onEnterByHyperspace();
	void onLeaveByHyperspace();
	CachedNetworkId const & getTurretTarget(int weaponIndex) const;
	bool isTurret(int weaponIndex) const;
	int getTurretCount() const;
	bool isCountermeasure(int weaponIndex) const;
	int getCountermeasureCount() const;
	const Object * getTurret(int weaponIndex) const;
	void onShipGunnerMounted(CreatureObject const &gunner, int newWeaponIndex);
	void onShipGunnerDismounted(CreatureObject const &gunner, int oldWeaponIndex);
	void onLocalProjectileFired(int weaponIndex);
	void onRemoteProjectileFired(int weaponIndex, Vector const &endPosition_w);
	void onWeaponHitTarget(int weaponIndex, CachedNetworkId const &target);
	void setNextMissileLauncher(int const newNextMissileLauncher);
	int getNextMissileLauncher() const;
	ShipLaunchable getShipLaunchable() const;

	float getCurrentSpeed() const;
	float getSlideDampener() const;

	bool hasWings() const;
	bool wingsOpened() const;
	bool isCapitalShip() const;
	bool isPobShip() const;
	bool isMultiPassenger() const;

	bool fireWeapon(int weaponIndex);
	void stopFiringWeapon(int weaponIndex, bool notifyServer);
	void setWeaponFiring(int weaponIndex);
	bool isWeaponFiring(int weaponIndex) const;

	bool hasAmmoForAShot(int const weaponIndex) const;

	virtual void           endBaselines();
	virtual void addToWorld();

	//-- Target lead indicator.
	bool getTargetLeadPosition_w(Vector & targetPosition_w, Object const & lookAtShip) const;
	float getTargetLeadTimeOfFlight(Vector const & headingPlayerToTarget, Vector const & targetHeading, float distanceToTarget, float projectileSpeed, float lookAtShipSpeed) const;

	//--
	//-- Componentized ship physics system.
	//-- these accessors return the correctly modified physics parameters
	//-- these computed parameters include:
	//--    engine component parameters
	//--    engine efficiency
	//--    booster component parameters, if active
	//--    booster component efficiency, if active
	//--    mass of the ship
	//--    rotational inertia of the ship
	//--

	float     getShipActualAccelerationRate       () const;
	float     getShipActualDecelerationRate       () const;
	float     getShipActualPitchAccelerationRate  () const;
	float     getShipActualYawAccelerationRate    () const;
	float     getShipActualRollAccelerationRate   () const;
	float     getShipActualPitchRateMaximum       () const;
	float     getShipActualYawRateMaximum         () const;
	float     getShipActualRollRateMaximum        () const;
	float     getShipActualSpeedMaximum           () const;

	//--
	//-- Ship Component System
	//--

	float getShipTotalComponentEnergyRequirement() const;

	float getOverallHealth() const;
	float getOverallHealthWithShieldsAndArmor() const;
	float getShieldHealth() const;
	float getArmorHealth() const;

	uint32 getChassisType() const;
	float getChassisComponentMassMaximum() const;
	float getChassisComponentMassCurrent() const;

	//-- Chassis system
	float getCurrentChassisHitPoints() const;
	float getMaximumChassisHitPoints() const;

	ShipComponentData * createShipComponentData (int chassisSlot) const;

	bool      isSlotInstalled                          (int chassisSlot) const;
	bool      isComponentDisabled                      (int chassisSlot) const;
	bool      isComponentLowPower                      (int chassisSlot) const;
	bool      isComponentActive                        (int chassisSlot) const;
	bool      isComponentDemolished                    (int chassisSlot) const;
	bool isComponentFunctional(int chassisSlot) const;

	//-- all components
	float     getComponentArmorHitpointsMaximum        (int chassisSlot) const;
	float     getComponentArmorHitpointsCurrent        (int chassisSlot) const;
	float     getComponentEfficiencyGeneral            (int chassisSlot) const;
	float     getComponentEfficiencyEnergy             (int chassisSlot) const;
	float     getComponentEnergyMaintenanceRequirement (int chassisSlot) const;
	float     getComponentMass                         (int chassisSlot) const;
	uint32    getComponentCrc                          (int chassisSlot) const;
	float     getComponentHitpointsCurrent             (int chassisSlot) const;
	float     getComponentHitpointsMaximum             (int chassisSlot) const;
	int       getComponentFlags                        (int chassisSlot) const;
	Unicode::String getComponentName                   (int chassisSlot) const;

	//-- weapons
	//-- The following methods are per weapon

	float     getWeaponDamageMaximum            (int chassisSlot) const;
	float     getWeaponDamageMinimum            (int chassisSlot) const;
	float     getWeaponEffectivenessShields     (int chassisSlot) const;
	float     getWeaponEffectivenessArmor       (int chassisSlot) const;
	float     getWeaponEnergyPerShot            (int chassisSlot) const;
	float     getWeaponActualEnergyPerShot(int chassisSlot) const;
	float     getWeaponRefireRate               (int chassisSlot) const;
	float     getWeaponActualRefireRate         (int chassisSlot) const;
	float     getWeaponEfficiencyRefireRate     (int chassisSlot) const;
	int getWeaponAmmoCurrent(int chassisSlot) const;
	int getWeaponAmmoMaximum(int chassisSlot) const;
	int getWeaponAmmoType(int chassisSlot) const;

	float getWeaponProjectileSpeed (int weaponIndex) const;
	int getProjectileIndexForWeapon(int weaponIndex) const;
	float getWeaponRange(int weaponIndex) const;
	float getMaxProjectileWeaponRange() const;
	float computeWeaponProjectileTimeToLive(int weaponIndex) const;

	//-- shields
	//-- The following methods are per shield

	float     getShieldHitpointsFrontCurrent    () const;
	float     getShieldHitpointsFrontMaximum    () const;
	float     getShieldHitpointsBackCurrent     () const;
	float     getShieldHitpointsBackMaximum     () const;
	float     getShieldRechargeRate             () const;

	//-- capacitor
	//-- The following methods are per capacitor

	float     getCapacitorEnergyCurrent         () const;
	float     getCapacitorEnergyMaximum         () const;
	float     getCapacitorEnergyRechargeRate    () const;

	//-- engine
	float     getEngineAccelerationRate         () const;
	float     getEngineDecelerationRate         () const;
	float     getEnginePitchAccelerationRate    () const;
	float     getEngineYawAccelerationRate      () const;
	float     getEngineRollAccelerationRate     () const;
	float     getEnginePitchRateMaximum         () const;
	float     getEngineYawRateMaximum           () const;
	float     getEngineRollRateMaximum          () const;
	float     getEngineSpeedMaximum             () const;

	//-- reactor
	float     getReactorEnergyGenerationRate    () const;

	//-- booster
	float     getBoosterEnergyCurrent           () const;
	float     getBoosterEnergyMaximum           () const;
	float     getBoosterEnergyRechargeRate      () const;
	float     getBoosterEnergyConsumptionRate   () const;
	float     getBoosterAcceleration            () const;
	float     getBoosterSpeedMaximum            () const;
	bool      isBoosterActive                   () const;

	//-- droid interface
	float     getDroidInterfaceCommandSpeed     () const;
	NetworkId const &getInstalledDroidControlDevice() const;

	int getCargoHoldContentsMaximum() const;
	int getCargoHoldContentsCurrent() const;
	NetworkIdIntMap const & getCargoHoldContents() const;
	int getCargoHoldContent(NetworkId const & resourceTypeId) const;

	//-- client side setters
	void clientSetComponentCrc(int chassisSlot, uint32 componentCrc);
	bool clientSetComponentFlags(int chassisSlot, int flags);
	bool clientSetComponentDisabled(int chassisSlot, bool disabled);
	bool clientSetComponentLowPower(int chassisSlot, bool lowPower);
	bool clientSetComponentHitPoints(int chassisSlot, float cur, float max);
	bool clientSetComponentArmorHitPoints(int chassisSlot, float cur, float max);
	bool clientSetComponentActive(int chassisSlot, bool active);
	void clientSetChassisHitPoints(float cur, float max);
	void clientSetChassisComponentMassMaximum(float massMaximum);
	void clientSetChassisComponentMassCurrent(float massCurrent);
	void clientSetCargoHoldContentsMaximum(int cargoHoldContentsMaximum);
	void clientSetCargoHoldContentsCurrent(int cargoHoldContentsCurrent);
	void clientSetCargoHoldContents(NetworkIdIntMap const & contents);
	void clientSetCargoHoldContent(NetworkId const & resourceTypeId, int amount);

	void clientPurgeComponent(int chassisSlot);
	bool clientInstallComponentFromData(int chassisSlot, ShipComponentData const & shipComponentData);
	bool clientPseudoInstallComponent(int chassisSlot, uint32 componentCrc);
	ShipComponentData * clientCreateShipComponentData (int chassisSlot) const;

	bool hasMissiles() const;
	bool hasMissileTargetAcquisition() const;
	bool isMissileTargetAcquiring() const;
	float getTimeUntilMissileTargetAcquisition() const;
	bool isMissile(int weaponIndex) const;
	int findFirstUsableMissileWeaponId() const;
	int findFirstUsableMissileWeaponIndex() const;
	void resetMissileAcquisitionTimers();

	bool isBeamWeapon(int weaponIndex);

	bool getWeaponRefireTimeRemaining(int weaponIndex, float & remaining, float & totalExpireTime) const;
	static ShipVector const & getShipsTargetingPlayer();

	IntVector const & getNebulas() const;

	float getNebulaEffectEngine() const;
	float computeActualComponentEfficiencyGeneral(int chassisType) const;

	const CachedNetworkId&     getPilotLookAtTarget() const;
	void                       setPilotLookAtTarget(const NetworkId&);
	ShipChassisSlotType::Type  getPilotLookAtTargetSlot() const;
	void                       setPilotLookAtTargetSlot(ShipChassisSlotType::Type slot);
	ShipChassisSlotType::Type  getPreviousValidTargetableSlot (ShipChassisSlotType::Type currentlyTargetedSlotType) const;
	ShipChassisSlotType::Type  getNextValidTargetableSlot (ShipChassisSlotType::Type currentlyTargetedSlotType) const;

	int getCurrentMissileTypeId() const;
	int getCurrentProjectileTypeId() const;
	void                       setSlotTargetable(ShipChassisSlotType::Type slot);
	void                       clearSlotTargetable(ShipChassisSlotType::Type slot);
	bool                       getSlotTargetable(ShipChassisSlotType::Type slot) const;
	bool                       isValidTargetableSlot (ShipChassisSlotType::Type slot) const;
	void                       updateTargetedSlot();

	void handleComponentDestruction(int chassisSlot, float severity, bool removeComponentCrc = true);

	void startShipDestructionSequence(float severity);
	typedef stdvector<Object *>::fwd DestructionDebrisList;
	void handleFinalShipDestruction(float severity, DestructionDebrisList * debrisList = 0);
	void handleFinalShipDestruction(float severity, PlaneVector const& planeVector, DestructionDebrisList * debrisList = 0);

	ShipObjectEffects const & getShipObjectEffects() const;
	ShipObjectEffects & getShipObjectEffects();

	ShipObjectAttachments const & getShipObjectAttachments() const;
	ShipObjectAttachments & getShipObjectAttachments();

	void updateTargetAcquisition(Object const * target, float elapsedTime, int weaponIndex, bool isCurrentWeapon, bool isReadyToFire);

	bool isWeaponPlayerControlled(int weaponIndex) const;

	void splitShipComponent(Object & object, int maxRandomPlanes, float severity, DestructionDebrisList * debris = 0) const;
	void splitShipComponent(Object & object, PlaneVector const & planeVector, DestructionDebrisList * debris = 0) const;

	Vector const getInterceptPosition_w(float timeElapsed) const;
	Vector const getShipVelocity_w() const;
	static Vector const getTargetInterceptPosition_w(float const frameTime, TangibleObject const & targetObject);

	std::string const & getWingName() const;
	std::string const & getTypeName() const;
	std::string const & getDifficulty() const;
	std::string const & getFaction() const;

	void hideShipPostDestruction();
	bool isShipKillableByDestructionSequence() const;
	void setShipKillableByDestructionSequence(bool b);

	static void generateDebrisSplittingPlanes(Object const & object, int maxRandomPlanes, float const severity, PlaneVector & planesVector);

	bool isTimerShieldHitClientExpired() const;
	bool isTimerShieldHitServerExpired() const;
	bool isTimerShieldEventServerExpired() const;

	void resetTimerShieldHitClient();
	void resetTimerShieldHitServer();
	void resetTimerShieldEventServer();

#ifdef _DEBUG
	void alterDebug(float elapsedTime);
#endif

public:
	void createDefaultController();
	void onCreateMissileMessage(NetworkId const & missileTarget);

	Vector const & getMissileTargetPosition() const;

	bool isPlayerControlled() const;
	
	static void enableTargetAppearanceFromObjectId(TangibleObject const * const instigator, CachedNetworkId const & objectid, bool enabled, int targetSlot);

	float getChassisSpeedMaximumModifier() const;
	void setChassisSpeedMaximumModifier(float f);

	void stopFiringAllWeapons();

	void setCockpitFrame(Object * cockpitFrame);
	Object const * getCockpitFrame() const;
protected:

	virtual void conditionModified(int oldCondition, int newCondition);

private:
	ShipObject();
	ShipObject(ShipObject const &);
	ShipObject &operator=(ShipObject const &);

	//----------------------------------------------------------------------
	//-- AutoDeltaVariable Callbacks

	struct Callbacks
	{
		template <typename S, typename U> struct DefaultCallback
		{
			void modified (ShipObject & target, const U & old, const U & value, bool local) const;
		};

		typedef DefaultCallback<Messages::LookAtTargetChanged,    CachedNetworkId>  LookAtTargetChanged;
		typedef DefaultCallback<Messages::LookAtTargetSlotChanged, int> LookAtTargetSlotChanged;
		typedef DefaultCallback<Messages::TargetableSlotBitfieldChanged, BitArray> TargetableSlotBitfieldChanged;
		typedef DefaultCallback<Messages::DroidPcdChanged, NetworkId> DroidPcdChanged;
		typedef DefaultCallback<Messages::ShipIdChanged, uint16> ShipIdChanged;
		typedef DefaultCallback<Messages::CargoChanged, int> CargoChanged;
	};

	void componentCrcOnSet           (const int & key, uint32 const & oldValue, uint32 const & value);
	void componentCrcOnInsert        (const int & key, uint32 const & value);
	void componentCrcOnErase         (const int & key, uint32 const & value);

	void cargoHoldContentsResourceTypeInfoOnSet(NetworkId const & key, ResourceTypeInfoPair const & oldValue, ResourceTypeInfoPair const & value);
	void cargoHoldContentsResourceTypeInfoOnInsert(NetworkId const & key, ResourceTypeInfoPair const & value);

	void updateAllComponentAttachments();
	void updateComponentAttachment   (int chassisSlot);
	void updateNebulas(float elapsedTime);
	void updateComponentStates(float elapsedTime);
	void updateComponentStatesForSlot(int chassisSlot, float elapsedTime);

	void updateMissiles(float const elapsedTime);
	void pushCustomizationToChildren();
	void resetAfterDestruction();
	int computeCargoHoldContentsCurrent() const;

private:

	Watcher<CreatureObject> m_pilot;

	Archive::AutoDeltaVariableCallback<uint16, Callbacks::ShipIdChanged, ShipObject> m_shipId;

	//-- Engine system
	Archive::AutoDeltaVariable<float> m_slideDampener;

	//-- Chassis system
	Archive::AutoDeltaVariable<float> m_currentChassisHitPoints;
	Archive::AutoDeltaVariable<float> m_maximumChassisHitPoints;

	//--
	//-- Ship Component System
	//--

	//-- all components

	Archive::AutoDeltaVariable<uint32>    m_chassisType;
	Archive::AutoDeltaVariable<float>     m_chassisComponentMassMaximum;
	Archive::AutoDeltaVariable<float>     m_chassisComponentMassCurrent;
	Archive::AutoDeltaVariable<float>     m_chassisSpeedMaximumModifier;

	Archive::AutoDeltaVariable<float>     m_shipActualAccelerationRate;
	Archive::AutoDeltaVariable<float>     m_shipActualDecelerationRate;
	Archive::AutoDeltaVariable<float>     m_shipActualPitchAccelerationRate;
	Archive::AutoDeltaVariable<float>     m_shipActualYawAccelerationRate;
	Archive::AutoDeltaVariable<float>     m_shipActualRollAccelerationRate;
	Archive::AutoDeltaVariable<float>     m_shipActualPitchRateMaximum;
	Archive::AutoDeltaVariable<float>     m_shipActualYawRateMaximum;
	Archive::AutoDeltaVariable<float>     m_shipActualRollRateMaximum;
	Archive::AutoDeltaVariable<float>     m_shipActualSpeedMaximum;

	Archive::AutoDeltaPackedMap<int, float>     m_componentArmorHitpointsMaximum;
	Archive::AutoDeltaPackedMap<int, float>     m_componentArmorHitpointsCurrent;
	Archive::AutoDeltaPackedMap<int, float>     m_componentEfficiencyGeneral;
	Archive::AutoDeltaPackedMap<int, float>     m_componentEfficiencyEnergy;
	Archive::AutoDeltaPackedMap<int, float>     m_componentEnergyMaintenanceRequirement;
	Archive::AutoDeltaPackedMap<int, float>     m_componentMass;
	Archive::AutoDeltaMap<int, uint32, ShipObject>    m_componentCrc;
	Archive::AutoDeltaPackedMap<int, float>     m_componentHitpointsCurrent;
	Archive::AutoDeltaPackedMap<int, float>     m_componentHitpointsMaximum;
	Archive::AutoDeltaPackedMap<int, int>       m_componentFlags;
	stdmap<int, int>::fwd                       m_oldComponentFlags;
	Archive::AutoDeltaPackedMap<int, Unicode::String> m_componentNames;
	Archive::AutoDeltaPackedMap<int, NetworkId> m_componentCreators;

	//-- weapons
	//-- The following maps have one entry per installed weapon .

	Archive::AutoDeltaPackedMap<int, float>     m_weaponDamageMaximum;
	Archive::AutoDeltaPackedMap<int, float>     m_weaponDamageMinimum;
	Archive::AutoDeltaPackedMap<int, float>     m_weaponEffectivenessShields;
	Archive::AutoDeltaPackedMap<int, float>     m_weaponEffectivenessArmor;
	Archive::AutoDeltaPackedMap<int, float>     m_weaponEnergyPerShot;
	Archive::AutoDeltaPackedMap<int, float>     m_weaponRefireRate;
	Archive::AutoDeltaPackedMap<int, float>     m_weaponEfficiencyRefireRate;
	Archive::AutoDeltaPackedMap<int, int>       m_weaponAmmoCurrent;
	Archive::AutoDeltaPackedMap<int, int>       m_weaponAmmoMaximum;
	Archive::AutoDeltaPackedMap<int, uint32>    m_weaponAmmoType;

	//-- shields
	//-- The following maps have one entry per installed shield.

	Archive::AutoDeltaVariable<float>     m_shieldHitpointsFrontCurrent;
	Archive::AutoDeltaVariable<float>     m_shieldHitpointsFrontMaximum;
	Archive::AutoDeltaVariable<float>     m_shieldHitpointsBackCurrent;
	Archive::AutoDeltaVariable<float>     m_shieldHitpointsBackMaximum;
	Archive::AutoDeltaVariable<float>     m_shieldRechargeRate;

	//-- capacitor
	//-- The following data is for component types which may not be installed in multiples.

	Archive::AutoDeltaVariable<float>     m_capacitorEnergyCurrent;
	Archive::AutoDeltaVariable<float>     m_capacitorEnergyMaximum;
	Archive::AutoDeltaVariable<float>     m_capacitorEnergyRechargeRate;

	//-- engine
	Archive::AutoDeltaVariable<float>     m_engineAccelerationRate;
	Archive::AutoDeltaVariable<float>     m_engineDecelerationRate;
	Archive::AutoDeltaVariable<float>     m_enginePitchAccelerationRate;
	Archive::AutoDeltaVariable<float>     m_engineYawAccelerationRate;
	Archive::AutoDeltaVariable<float>     m_engineRollAccelerationRate;
	Archive::AutoDeltaVariable<float>     m_enginePitchRateMaximum;
	Archive::AutoDeltaVariable<float>     m_engineYawRateMaximum;
	Archive::AutoDeltaVariable<float>     m_engineRollRateMaximum;
	Archive::AutoDeltaVariable<float>     m_engineSpeedMaximum;

	//-- reactor
	Archive::AutoDeltaVariable<float>     m_reactorEnergyGenerationRate;

	//-- booster
	Archive::AutoDeltaVariable<float>     m_boosterEnergyCurrent;
	Archive::AutoDeltaVariable<float>     m_boosterEnergyMaximum;
	Archive::AutoDeltaVariable<float>     m_boosterEnergyRechargeRate;
	Archive::AutoDeltaVariable<float>     m_boosterEnergyConsumptionRate;
	Archive::AutoDeltaVariable<float>     m_boosterAcceleration;
	Archive::AutoDeltaVariable<float>     m_boosterSpeedMaximum;

	//-- droid interface
	Archive::AutoDeltaVariable<float>     m_droidInterfaceCommandSpeed;
	Archive::AutoDeltaVariableCallback<NetworkId, Callbacks::DroidPcdChanged, ShipObject> m_installedDroidControlDevice;

	Archive::AutoDeltaVariableCallback<int, Callbacks::CargoChanged, ShipObject>       m_cargoHoldContentsMaximum;
	Archive::AutoDeltaVariableCallback<int, Callbacks::CargoChanged, ShipObject>       m_cargoHoldContentsCurrent;
	Archive::AutoDeltaPackedMap<NetworkId, int> m_cargoHoldContents;
	Archive::AutoDeltaMap<NetworkId, std::pair<Unicode::String, std::string>, ShipObject> m_cargoHoldContentsResourceTypeInfo;

	Archive::AutoDeltaVariableCallback<CachedNetworkId, Callbacks::LookAtTargetChanged, ShipObject> m_pilotLookAtTarget;
	Archive::AutoDeltaVariableCallback<int,             Callbacks::LookAtTargetSlotChanged, ShipObject> m_pilotLookAtTargetSlot;
	Archive::AutoDeltaVariableCallback<BitArray,        Callbacks::TargetableSlotBitfieldChanged, ShipObject> m_targetableSlotBitfield;

	Archive::AutoDeltaVariable<std::string>  m_wingName;
	Archive::AutoDeltaVariable<std::string>  m_typeName;
	Archive::AutoDeltaVariable<std::string>  m_difficulty;
	Archive::AutoDeltaVariable<std::string>  m_faction;

	Archive::AutoDeltaVariable<int> m_guildId;

	IntVector *                           m_nebulas;

	float                                 m_nebulaEffectReactor;
	float                                 m_nebulaEffectEngine;
	float                                 m_nebulaEffectShields;

	int m_nextMissileLauncher;

	ShipObjectEffects * m_shipObjectEffects;
	ShipObjectAttachments * m_shipObjectAttachments;


	MissileUpdate * m_missleUpdateData;
	bool m_hasMissiles;
	bool m_hasTargetAcquiring;
	bool m_hasTargetAcquired;
	int m_missileGroup;
	int m_missileSlot;

	typedef stdmap<int, ShipWeaponStatus>::fwd ShipWeaponStatusMap;
	ShipWeaponStatusMap m_shipWeaponStatusMap;

	Timer m_timerPostDestruction;
	bool m_shipKillableByDestructionSequence;
	Vector m_lastPositionAtDestruction_w;

	Timer m_timerShieldHitClient;
	Timer m_timerShieldHitServer;
	Timer m_timerShieldEventServer;

};

//----------------------------------------------------------------------

inline const CachedNetworkId& ShipObject::getPilotLookAtTarget() const
{
	return m_pilotLookAtTarget.get();
}

//----------------------------------------------------------------------

inline ShipChassisSlotType::Type ShipObject::getPilotLookAtTargetSlot() const
{
	return static_cast<ShipChassisSlotType::Type>(m_pilotLookAtTargetSlot.get());
}

//----------------------------------------------------------------------

inline ShipObject::IntVector const & ShipObject::getNebulas() const
{
	return *NON_NULL(m_nebulas);
}

//----------------------------------------------------------------------

inline void ShipObject::setNextMissileLauncher(int const newNextMissileLauncher)
{
	m_nextMissileLauncher = newNextMissileLauncher;
}

//----------------------------------------------------------------------

inline int ShipObject::getNextMissileLauncher() const
{
	return m_nextMissileLauncher;
}

//----------------------------------------------------------------------

inline bool ShipObject::isComponentFunctional(int chassisSlot) const
{
	return isSlotInstalled(chassisSlot) && !isComponentDisabled(chassisSlot) && !isComponentDemolished(chassisSlot);
}

// ======================================================================

#endif

