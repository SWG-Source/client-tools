//===================================================================
//
// ClientDataFile.h
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_ClientDataFile_H
#define INCLUDED_ClientDataFile_H

//===================================================================

#include "sharedGame/SharedObjectTemplateClientData.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/LessPointerComparator.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"

class Breakpoint;
class ClientEffectTemplate;
class ContrailData;
class CrcLowerString;
class DestructionSequence;
class GlowData;
class Iff;
class InterpolatedSound;
class Object;
class Plane;
class SoundTemplate;
class Vector;
class VehicleGroundEffectData;
class VehicleThrusterSoundData;
class VehicleLightningEffectData;

//===================================================================

class ClientDataFile : public SharedObjectTemplateClientData
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

public:

	typedef stdvector<std::pair<Vector, float> >::fwd ClearFloraEntryList;
	typedef stdvector<std::string>::fwd StringVector;
	struct HardpointChildObject;
	struct InitialHardpointChildObject;
	typedef stdvector<Breakpoint>::fwd BreakpointVector;
	typedef stdvector<InterpolatedSound>::fwd InterpolatedSoundVector;

public:

	static void install ();

	static ClientDataFile* create (const std::string& filename);

public:

	explicit ClientDataFile (const char* filename);
	virtual ~ClientDataFile ();

	virtual void preloadAssets () const;

	void apply(Object * object) const;
	void applyGlows(Object & object, bool skipSelfGlows) const;

	void applyDamage (Object* object, bool on, float currentDamageLevel, float desiredDamageLevel) const;
	void applyOnOff (Object* object, bool on) const;
	void playDestructionEffect(Object const & object) const;

	const CrcLowerString& getEffectForEvent(const CrcLowerString& event) const;
	ClearFloraEntryList const * getClearFloraEntryList() const;

	void loadFromIff (Iff& iff);

	bool hasWearables() const;

	typedef stdvector<Object *>::fwd ObjectVector;
	bool updateVehicleThrusters (Object & obj, float oldDamageLevel, float newDamageLevel, ObjectVector & thrusters, VehicleThrusterSoundData & vsd) const;
	bool updateVehicleThrusters (Object & obj, int vehicleThrusterIndex, ObjectVector & thrusters, VehicleThrusterSoundData & sounds) const;
	bool initVehicleThrusters   (Object & obj, ObjectVector & thrusters, VehicleThrusterSoundData & vsd) const;

	typedef stdvector<VehicleGroundEffectData*>::fwd VehicleGroundEffectDataList;

	const VehicleGroundEffectDataList * getVehicleGroundEffectsData () const;

	typedef stdvector<VehicleLightningEffectData*>::fwd VehicleLightningEffectDataList;

	const VehicleLightningEffectDataList * getVehicleLightningEffectsData () const;

	typedef stdvector<ContrailData const *>::fwd ContrailDataVector;

	ContrailDataVector const * getContrailDataVector() const;

	InterpolatedSound const * getInterpolatedSound() const;

	typedef stdvector<Plane>::fwd PlaneVector;

	bool processBreakpoints(Object const & object, BreakpointVector const & breakpoints, float probabilityDelta, PlaneVector & planeVector) const;
	bool processBreakpoints(Object const & object, float probabilityDelta, PlaneVector & planeVector) const;

	void getAllChildObjects(StringVector & results) const;
	void getAllChildAppearances(StringVector & results) const;

	DestructionSequence const * getDestructionSequence() const;
	InterpolatedSoundVector const * getInterpolatedSoundVector() const;

private:

	static void remove ();

private:

	// Disabled.
	ClientDataFile ();
	ClientDataFile (const ClientDataFile&);
	ClientDataFile& operator= (const ClientDataFile&);

	void load (Iff& iff);
	void load_0000 (Iff& iff);

	void applyAmbientSounds (Object* object, bool on) const;
	void applyDamage (Object* object, int index) const;
	void attachSound (Object const* object, const std::string& soundTemplateName) const;
	void applyBakedWearables (Object* object) const;

	int findVehicleThrusterIndex (float damageLevel) const;

	void release () const;

	void applyCustomizationVariableOverrides(Object &object) const;

	void cacheClientEffectTemplate(char const * fileName) const;
	void cacheSoundTemplate(char const * fileName) const;

private:

	typedef stdmap<CrcString const *, ClientEffectTemplate const *, LessPointerComparator>::fwd CachedClientEffectTemplateMap;
	CachedClientEffectTemplateMap mutable * m_cachedClientEffectTemplateMap;

	typedef stdmap<CrcString const *, SoundTemplate const *, LessPointerComparator>::fwd CachedSoundTemplateMap;
	CachedSoundTemplateMap mutable * m_cachedSoundTemplateMap;

	struct TransformChildObject;
	typedef stdvector<TransformChildObject*>::fwd TransformChildObjectList;
	TransformChildObjectList * m_transformChildObjectList;

	typedef stdvector<HardpointChildObject*>::fwd HardpointChildObjectList;
	HardpointChildObjectList * m_hardpointChildObjectList;

	typedef stdvector<InitialHardpointChildObject*>::fwd InitialHardpointChildObjectList;
	InitialHardpointChildObjectList * m_initialHardpointChildObjectList;

	struct TransformLightObject;
	typedef stdvector<TransformLightObject*>::fwd TransformLightObjectList;
	TransformLightObjectList * m_transformLightObjectList;

	struct HardpointLightObject;
	typedef stdvector<HardpointLightObject*>::fwd HardpointLightObjectList;
	HardpointLightObjectList * m_hardpointLightObjectList;

	typedef stdmap<CrcLowerString, CrcLowerString>::fwd EventToEffectMap;
	EventToEffectMap * m_eventToEffectMap;

	std::string * m_ambientSoundFileName;

	struct SunTrackingObjectData;
	SunTrackingObjectData * m_sunTrackingObjectData;

	struct TurretObjectData;
	TurretObjectData * m_turretObjectData;

	ClearFloraEntryList * m_clearFloraEntryList;

	class DamageProperty;
	struct DamageLevel;
	typedef stdvector<DamageLevel*>::fwd DamageLevelList;
	DamageLevelList * m_damageLevelList;

	struct OnOffObject;
	typedef stdvector<OnOffObject*>::fwd OnOffObjectList;
	OnOffObjectList * m_onOffObjectList;

	class Wearable;
	typedef stdvector<Wearable*>::fwd WearableList;
	WearableList * m_wearableList; 

	class Flag;
	typedef stdvector<Flag*>::fwd  FlagList;
	FlagList* m_flagList;

	class Banner;
	typedef stdvector<Banner*>::fwd  BannerList;
	BannerList* m_bannerList;

	struct CustomizationInt;
	typedef stdvector<CustomizationInt*>::fwd CustomizationIntList;
	CustomizationIntList* m_customizationIntList;

	class VehicleThrusterData;
	typedef stdvector<VehicleThrusterData*>::fwd VehicleThrusterDataList;
	VehicleThrusterDataList* m_vehicleThrusterDataList;

	VehicleGroundEffectDataList* m_vehicleGroundEffectDataList;

	VehicleLightningEffectDataList * m_vehicleLightningEffectDataList;

	ContrailDataVector * m_contrailDataVector;

	typedef stdvector<GlowData *>::fwd GlowDataVector;
	GlowDataVector * m_glowDataVector;

	InterpolatedSound * m_interpolatedSound;

	BreakpointVector * m_breakpointVector;

	ClientEffectTemplate const * m_destructionEffect;

	DestructionSequence * m_destructionSequence;

	InterpolatedSoundVector * m_interpolatedSoundVector;

};

//----------------------------------------------------------------------

inline const ClientDataFile::VehicleGroundEffectDataList * ClientDataFile::getVehicleGroundEffectsData () const
{
	return m_vehicleGroundEffectDataList;
}

//----------------------------------------------------------------------


inline const ClientDataFile::VehicleLightningEffectDataList * ClientDataFile::getVehicleLightningEffectsData () const
{
	return m_vehicleLightningEffectDataList;
}

//===================================================================

#endif
