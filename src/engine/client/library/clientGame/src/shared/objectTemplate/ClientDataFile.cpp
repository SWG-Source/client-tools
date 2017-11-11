//===================================================================
//
// ClientDataFile.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientDataFile.h"

#include "clientAudio/Audio.h"
#include "clientAudio/SoundTemplate.h"
#include "clientAudio/SoundTemplateList.h"
#include "clientGame/ClearNonCollidableFloraNotification.h"
#include "clientGame/ClientDataFile_Banner.h"
#include "clientGame/ClientDataFile_Breakpoint.h"
#include "clientGame/ClientDataFile_ContrailData.h"
#include "clientGame/ClientDataFile_DestructionSequence.h"
#include "clientGame/ClientDataFile_Flag.h"
#include "clientGame/ClientDataFile_GlowData.h"
#include "clientGame/ClientDataFile_HardpointChildObject.h"
#include "clientGame/ClientDataFile_InitialHardpointChildObject.h"
#include "clientGame/ClientDataFile_InterpolatedSound.h"
#include "clientGame/ClientDataFile_VehicleGroundEffectData.h"
#include "clientGame/ClientDataFile_VehicleLightningEffectData.h"
#include "clientGame/ClientDataFile_VehicleThrusterData.h"
#include "clientGame/ClientDataFile_VehicleThrusterSoundData.h"
#include "clientGame/ClientDataFile_Wearable.h"
#include "clientGame/ClientDataTemplateList.h"
#include "clientGame/ClientEffect.h"
#include "clientGame/ClientEffectTemplate.h"
#include "clientGame/ClientEffectTemplateList.h"
#include "clientGame/ClientObject.h"
#include "clientGame/ClientWeaponObjectTemplate.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/GameLight.h"
#include "clientGame/GlowAppearance.h"
#include "clientGame/ShipWeaponBarrelDynamics.h"
#include "clientGame/SunTrackingObject.h"
#include "clientGame/TurretObject.h"
#include "clientGame/WingDynamics.h"
#include "clientGraphics/Light.h"
#include "clientGraphics/RenderWorld.h"
#include "clientObject/HardpointObject.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedFoundation/VoidBindSecond.h"
#include "sharedFoundation/VoidMemberFunction.h"
#include "sharedGame/AssetCustomizationManager.h"
#include "sharedMath/Plane.h"
#include "sharedMath/VectorArgb.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/BasicRangedIntCustomizationVariable.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/CustomizationDataProperty.h"
#include "sharedObject/Object.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/RotationDynamics.h"
#include "sharedObject/SeesawDynamics.h"
#include "sharedObject/SpringDynamics.h"
#include "sharedObject/StateDynamics.h"

#include <algorithm>
#include <map>
#include <cstdio>

// ==================================================================

#ifdef _DEBUG
#  define CUSTOMIZATION_WARN_ON_OVERRIDE_NON_DEFAULT  1
#else
#  define CUSTOMIZATION_WARN_ON_OVERRIDE_NON_DEFAULT  0
#endif

//===================================================================

// See transform.h 
// "warning, member 'matrix' not initialized // that's right, caller beware"
#pragma warning(disable : 4701)

namespace ClientDataFileNamespace
{
	const Tag TAG_APPR = TAG (A,P,P,R);
	const Tag TAG_ASND = TAG (A,S,N,D);
	const Tag TAG_BANR = TAG (B,A,N,R);
	const Tag TAG_CEFT = TAG (C,E,F,T);
	const Tag TAG_CHLD = TAG (C,H,L,D);
	const Tag TAG_CHL2 = TAG (C,H,L,2);
	const Tag TAG_CLDF = TAG (C,L,D,F);
	const Tag TAG_CNCF = TAG (C,N,C,F);
	const Tag TAG_CSND = TAG (C,S,N,D);
	const Tag TAG_CSSI = TAG (C,S,S,I);
	const Tag TAG_DAMA = TAG (D,A,M,A);
	const Tag TAG_EVNT = TAG (E,V,N,T);
	const Tag TAG_FLAG = TAG (F,L,A,G);
	const Tag TAG_HARD = TAG (H,A,R,D);
	const Tag TAG_HLOB = TAG (H,L,O,B);
	const Tag TAG_HOBJ = TAG (H,O,B,J);
	const Tag TAG_IHOB = TAG (I,H,O,B);
	const Tag TAG_LOBJ = TAG (L,O,B,J);
	const Tag TAG_ONOF = TAG (O,N,O,F);
	const Tag TAG_PSOR = TAG (P,S,O,R);
	const Tag TAG_SUNT = TAG (S,U,N,T);
	const Tag TAG_TURR = TAG (T,U,R,R);
	const Tag TAG_WEAR = TAG (W,E,A,R);
	const Tag TAG_VTHR = TAG (V,T,H,R);
	const Tag TAG_VGEF = TAG (V,G,E,F);
	const Tag TAG_VLEF = TAG (V,L,E,F);
	const Tag TAG_WING = TAG (W,I,N,G);
	const Tag TAG_CONT = TAG (C,O,N,T);
	const Tag TAG_GLOW = TAG (G,L,O,W);
	const Tag TAG_INTS = TAG (I,N,T,S);
	const Tag TAG_BRKP = TAG (B,R,K,P);
	const Tag TAG_DSTR = TAG (D,S,T,R);
	const Tag TAG_DSEF = TAG (D,S,E,F);
	const Tag TAG_ENGS = TAG (E,N,G,S);

	const ConstCharCrcLowerString cms_empty ("");
	const ConstCharCrcLowerString cms_footstep ("footstep");

	struct LightObject;
	void readLightObject (Iff& iff, LightObject& lightObject);
	Light* createLight (const LightObject& lightObject);
	void removeChildObjects (Object* object, const char* debugName);

	bool s_logUndeclaredCustomizations;

	//----------------------------------------------------------------------

	typedef ConstWatcher<Object> ObjectWatcher;
	typedef stdmap<ObjectWatcher, SoundId>::fwd ObjectSoundIdMap;
	ObjectSoundIdMap s_ambientSounds;

	//----------------------------------------------------------------------

	const SoundId & findAmbientSoundForObject (const Object & obj)
	{ //lint !e1929 // returns reference
		const ObjectSoundIdMap::const_iterator it = s_ambientSounds.find (ObjectWatcher (&obj));
		if (it != s_ambientSounds.end ())
			return (*it).second;

		return SoundId::getInvalid();
	}

	//----------------------------------------------------------------------

	void purgeAmbientSounds ()
	{
		for (ObjectSoundIdMap::iterator it = s_ambientSounds.begin (); it != s_ambientSounds.end ();)
		{
			const ObjectWatcher & ow =(*it).first;
			if (ow.getPointer () == 0)
				s_ambientSounds.erase (it++);
			else
				++it;
		}
	}

	//----------------------------------------------------------------------

	void setAmbientSoundForObject (const Object & obj, const SoundId & soundId)
	{
		if (!soundId.isValid ())
			IGNORE_RETURN(s_ambientSounds.erase (ObjectWatcher (&obj)));
		else
			s_ambientSounds [ObjectWatcher (&obj)] = soundId;
	}

	//----------------------------------------------------------------------

	Object *findHardpointParent(Object &root, CrcString const &hardpointName, Transform &hardpointTransform)
	{
		Appearance const * const app = root.getAppearance();
		if (app)
		{
			if (app->findHardpoint(hardpointName, hardpointTransform))
				return &root;
		}

		int const numberOfChildObjects = root.getNumberOfChildObjects ();
		for (int i = 0; i < numberOfChildObjects; ++i)
		{
			Object * const child = root.getChildObject(i);
			if (child)
			{
				Object * const ret = findHardpointParent(*child, hardpointName, hardpointTransform);
				if (ret)
					return ret;
			}
		}
		
		return 0;
	}
	
	//----------------------------------------------------------------------
		
	bool addHardpointObjectToFirstAppropriateParent(HardpointObject & objectToAdd, Object & potentialParent)
	{
		if (objectToAdd.getHardpointName().isEmpty())
		{
			potentialParent.addChildObject_o(&objectToAdd);
			return true;
		}
		
		Appearance const * const app = potentialParent.getAppearance();
		if (NULL != app)
		{
			Transform t;
			if (app->findHardpoint(objectToAdd.getHardpointName(), t))
			{
				potentialParent.addChildObject_o(&objectToAdd);
				return true;
			}
		}
		
		Object * const nextParent = potentialParent.getParent();
		if (NULL != nextParent)
		{
			return addHardpointObjectToFirstAppropriateParent(objectToAdd, *nextParent);
		}
		
		return false;
	}

	//----------------------------------------------------------------------
}

using namespace ClientDataFileNamespace;

//===================================================================
// PRIVATE ClientDataFile::TransformChildObject
//===================================================================

struct ClientDataFile::TransformChildObject
{
public:

	bool        m_useHardpoint;
	Vector      m_position;
	Vector      m_orientation;
	std::string m_hardpointName;
	std::string m_objectTemplateName;
	std::string m_appearanceTemplateName;
	Vector      m_rotationsPerSecond;
	Vector      m_seesawsPerSecond;
	float       m_seesawMagnitude;
	Vector      m_springOffset;
	float       m_springsPerSecond;
	float       m_rotation;
	float       m_rotationTime;
	std::string m_soundTemplateName;

public:

	const AppearanceTemplate* m_appearanceTemplate;
	const ObjectTemplate* m_objectTemplate;
	SoundTemplate const * m_soundTemplate;

public:

	TransformChildObject () :
		m_useHardpoint(false),
		m_position (),
		m_orientation (),
		m_hardpointName(),
		m_objectTemplateName (),
		m_appearanceTemplateName (),
		m_rotationsPerSecond (),
		m_seesawsPerSecond (),
		m_seesawMagnitude (0.f),
		m_springOffset (),
		m_springsPerSecond (0.f),
		m_rotation(0.f),
		m_rotationTime(0.f),
		m_soundTemplateName(),
		m_appearanceTemplate (0),
		m_objectTemplate (0),
		m_soundTemplate(0)
	{
	}

	~TransformChildObject ()
	{
		if (m_appearanceTemplate)
		{
			AppearanceTemplateList::release (m_appearanceTemplate);
			m_appearanceTemplate = 0;
		}

		if (m_objectTemplate)
		{
			m_objectTemplate->releaseReference ();
			m_objectTemplate = 0;
		}

		if (m_soundTemplate)
		{
			SoundTemplateList::release(m_soundTemplate);
			m_soundTemplate = 0;
		}
	}
};


//===================================================================
// PRIVATE ClientDataFile::LightObject
//===================================================================

struct ClientDataFileNamespace::LightObject
{
public:

	int         m_type;
	std::string m_appearanceTemplateName;
	VectorArgb  m_color1;
	VectorArgb  m_color2;
	float       m_range1;
	float       m_range2;
	float       m_time1;
	float       m_time2;
	float       m_constantAttenuation;
	float       m_linearAttenuation;
	float       m_quadraticAttenuation;
	bool        m_timeOfDayAware;
	bool		m_onOffAware;

public:

	const AppearanceTemplate* m_appearanceTemplate;

public:

	LightObject () :
		m_type (0),
		m_appearanceTemplateName (),
		m_color1 (),
		m_color2 (),
		m_range1 (0.f),
		m_range2 (0.f),
		m_time1 (0.f),
		m_time2 (0.f),
		m_constantAttenuation (0.f),
		m_linearAttenuation (0.f),
		m_quadraticAttenuation (0.f),
		m_timeOfDayAware (false),
		m_onOffAware (false),
		m_appearanceTemplate (0)
	{
	}

	virtual ~LightObject ()
	{
		if (m_appearanceTemplate)
		{
			AppearanceTemplateList::release (m_appearanceTemplate);
			m_appearanceTemplate = 0;
		}
	}
};

//===================================================================
// PRIVATE ClientDataFile::HardpointLightObject
//===================================================================

struct ClientDataFile::HardpointLightObject : public LightObject
{
public:

	std::string m_hardpointName;

public:

	HardpointLightObject () :
		LightObject (),
		m_hardpointName ()
	{
	}

	virtual ~HardpointLightObject ()
	{
	}
};

//===================================================================
// PRIVATE ClientDataFile::TransformLightObject
//===================================================================

struct ClientDataFile::TransformLightObject : public LightObject
{
public:

	Vector      m_position;
	Vector      m_orientation;

public:

	TransformLightObject () :
		LightObject (),
		m_position (),
		m_orientation ()
	{
	}

	virtual ~TransformLightObject ()
	{
	}
};

//===================================================================
// PRIVATE ClientDataFile::SunTrackingObjectData
//===================================================================

struct ClientDataFile::SunTrackingObjectData
{
public:

	std::string    m_sunTrackingAppearanceTemplateName;
	CrcLowerString m_sunTrackingHardpointName;
	float          m_sunTrackingYawMaximumRadiansPerSecond;
	std::string    m_barrelAppearanceTemplateName;
	CrcLowerString m_barrelHardpointName;
	float          m_barrelPitchMinimumRadians;
	float          m_barrelPitchMaximumRadians;
	float          m_barrelPitchMaximumRadiansPerSecond;
	bool           m_valid;

public:

	const AppearanceTemplate* m_sunTrackingAppearanceTemplate;
	const AppearanceTemplate* m_barrelAppearanceTemplate;

public:

	SunTrackingObjectData () :
		m_sunTrackingAppearanceTemplateName (),
		m_sunTrackingHardpointName (),
		m_sunTrackingYawMaximumRadiansPerSecond (0.f),
		m_barrelAppearanceTemplateName (),
		m_barrelHardpointName (),
		m_barrelPitchMinimumRadians (0.f),
		m_barrelPitchMaximumRadians (0.f),
		m_barrelPitchMaximumRadiansPerSecond (0.f),
		m_valid (false),
		m_sunTrackingAppearanceTemplate (0),
		m_barrelAppearanceTemplate (0)
	{
	}

	~SunTrackingObjectData ()
	{
		if (m_sunTrackingAppearanceTemplate)
		{
			AppearanceTemplateList::release (m_sunTrackingAppearanceTemplate);
			m_sunTrackingAppearanceTemplate = 0;
		}

		if (m_barrelAppearanceTemplate)
		{
			AppearanceTemplateList::release (m_barrelAppearanceTemplate);
			m_barrelAppearanceTemplate = 0;
		}
	}
};

//===================================================================
// PRIVATE ClientDataFile::TurretObjectData
//===================================================================

struct ClientDataFile::TurretObjectData
{
public:

	std::string    m_turretAppearanceTemplateName;
	CrcLowerString m_turretHardpointName;
	float          m_turretYawMaximumRadiansPerSecond;
	std::string    m_barrelAppearanceTemplateName;
	CrcLowerString m_barrelHardpointName;
	float          m_barrelPitchMinimumRadians;
	float          m_barrelPitchMaximumRadians;
	float          m_barrelPitchMaximumRadiansPerSecond;
	std::string    m_weaponObjectTemplateName;
	CrcLowerString m_muzzleHardpointName;
	float          m_speed;
	float          m_expirationTime;
	bool           m_valid;

public:

	const AppearanceTemplate*         m_turretAppearanceTemplate;
	const AppearanceTemplate*         m_barrelAppearanceTemplate;
	const ClientWeaponObjectTemplate* m_weaponObjectTemplate;

public:

	TurretObjectData () :
		m_turretAppearanceTemplateName (),
		m_turretHardpointName (),
		m_turretYawMaximumRadiansPerSecond (0.f),
		m_barrelAppearanceTemplateName (),
		m_barrelHardpointName (),
		m_barrelPitchMinimumRadians (0.f),
		m_barrelPitchMaximumRadians (0.f),
		m_barrelPitchMaximumRadiansPerSecond (0.f),
		m_weaponObjectTemplateName (),
		m_muzzleHardpointName (),
		m_speed (0.f),
		m_expirationTime (0.f),
		m_valid (false),
		m_turretAppearanceTemplate (0),
		m_barrelAppearanceTemplate (0),
		m_weaponObjectTemplate (0)
	{
	}

	~TurretObjectData ()
	{
		if (m_turretAppearanceTemplate)
		{
			AppearanceTemplateList::release (m_turretAppearanceTemplate);
			m_turretAppearanceTemplate = 0;
		}

		if (m_barrelAppearanceTemplate)
		{
			AppearanceTemplateList::release (m_barrelAppearanceTemplate);
			m_barrelAppearanceTemplate = 0;
		}

		if (m_weaponObjectTemplate)
		{
			m_weaponObjectTemplate->releaseReference ();
			m_weaponObjectTemplate = 0;
		}
	}
};

//===================================================================
// PRIVATE ClientDataFile::DamageProperty
//===================================================================

class ClientDataFile::DamageProperty : public Property
{
public:

	static PropertyId getClassPropertyId ()
	{
		return PROPERTY_HASH (DamageProperty, 0xa0bd02c5);
	}

public:

	explicit DamageProperty (Object& owner) :
		Property (getClassPropertyId (), owner),
		m_currentDamageLevelIndex (0)
	{
	}

	virtual ~DamageProperty ()
	{
	}

	int getCurrentDamageLevelIndex () const
	{
		return m_currentDamageLevelIndex;
	}

	void setCurrentDamageLevelIndex (const int currentDamageLevelIndex)
	{
		m_currentDamageLevelIndex = currentDamageLevelIndex;
	}

private:

	DamageProperty ();
	DamageProperty (const DamageProperty&);            //lint -esym(754, DamageProperty::DamageProperty) // unreferenced // it's okay, defensive hiding.
	DamageProperty& operator= (const DamageProperty&);

private:

	int m_currentDamageLevelIndex;
};

//===================================================================
// PRIVATE ClientDataFile::DamageLevel
//===================================================================

struct ClientDataFile::DamageLevel
{
public:

	float       m_minimumDamageLevel;
	float       m_maximumDamageLevel;
	std::string m_clientEffectTemplateName;
	std::string m_appearanceTemplateName;
	bool        m_useHardpoint;
	Vector      m_position;
	Vector      m_orientation;
	std::string m_hardpointName;
	std::string m_ambientSoundTemplateName;

	static bool DamageSorter (const ClientDataFile::DamageLevel * a, const ClientDataFile::DamageLevel * b)
	{
		NOT_NULL (a);
		NOT_NULL (b);
		return a->m_minimumDamageLevel < b->m_minimumDamageLevel;
	}


public:

	const AppearanceTemplate*   m_appearanceTemplate;
	const ClientEffectTemplate* m_clientEffectTemplate;

public:

	DamageLevel () :
		m_minimumDamageLevel (0.f),
		m_maximumDamageLevel (0.f),
		m_clientEffectTemplateName (),
		m_appearanceTemplateName (),
		m_useHardpoint (false),
		m_position (),
		m_orientation (),
		m_hardpointName (),
		m_ambientSoundTemplateName (),
		m_appearanceTemplate (0),
		m_clientEffectTemplate (0)
	{
	}

	~DamageLevel ()
	{
		if (m_appearanceTemplate)
		{
			AppearanceTemplateList::release (m_appearanceTemplate);
			m_appearanceTemplate = 0;
		}

		if (m_clientEffectTemplate)
		{
			m_clientEffectTemplate->release ();
			m_clientEffectTemplate = 0;
		}
	}
};

//===================================================================
// PRIVATE ClientDataFile::OnOffObject
//===================================================================

struct ClientDataFile::OnOffObject
{
public:

	bool        m_on;
	std::string m_appearanceTemplateName;
	bool        m_useHardpoint;
	Vector      m_position;
	Vector      m_orientation;
	std::string m_hardpointName;
	std::string m_ambientSoundTemplateName;

public:

	const AppearanceTemplate* m_appearanceTemplate;

public:

	OnOffObject () :
		m_on (false),
		m_appearanceTemplateName (),
		m_useHardpoint (false),
		m_position (),
		m_orientation (),
		m_hardpointName (),
		m_ambientSoundTemplateName (),
		m_appearanceTemplate (0)
	{
	}

	~OnOffObject ()
	{
		if (m_appearanceTemplate)
		{
			AppearanceTemplateList::release (m_appearanceTemplate);
			m_appearanceTemplate = 0;
		}
	}
};

// ======================================================================
// PRIVATE ClientDataFile::CustomizationInt
// ======================================================================

struct ClientDataFile::CustomizationInt
{
public:

	CustomizationInt (char const* variableName, int value) :
		m_variableName (variableName),
		m_value (value)
		{
		}

public:

	std::string  m_variableName;
	int          m_value;

private:

	// Disabled.
	CustomizationInt ();
	CustomizationInt (CustomizationInt const& rhs); //lint -esym(754, CustomizationInt::CustomizationInt) // unreferenced // it's okay, defensive hiding.

};

//===================================================================
// STATIC PUBLIC ClientDataFile
//===================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL(ClientDataFile, true, 0, 0, 0);

// ======================================================================

void ClientDataFile::install ()
{
	InstallTimer const installTimer("ClientDataFile::install");

	installMemoryBlockManager();

	ClientDataTemplateList::assignBinding (TAG_CLDF, create);

	DebugFlags::registerFlag (s_logUndeclaredCustomizations, "ClientGame/Customization", "logUndeclaredCustomizations");

	ExitChain::add (remove, "ClientDataFile::remove");
}

//-------------------------------------------------------------------

void ClientDataFile::remove ()
{
	removeMemoryBlockManager();

	ClientDataTemplateList::removeBinding (TAG_CLDF);
}

//-------------------------------------------------------------------

ClientDataFile* ClientDataFile::create (const std::string& filename)
{
	return new ClientDataFile (filename.c_str ());
}

//===================================================================
// PUBLIC ClientDataFile
//===================================================================

ClientDataFile::ClientDataFile (const char* filename) :
	SharedObjectTemplateClientData (filename),
	m_cachedClientEffectTemplateMap(0),
	m_cachedSoundTemplateMap(0),
	m_transformChildObjectList(0),
	m_hardpointChildObjectList(0),
	m_initialHardpointChildObjectList(0),
	m_transformLightObjectList(0),
	m_hardpointLightObjectList(0),
	m_eventToEffectMap(0),
	m_ambientSoundFileName(0),
	m_sunTrackingObjectData(0),
	m_turretObjectData(0),
	m_clearFloraEntryList(0),
	m_damageLevelList(0),
	m_onOffObjectList(0),
	m_wearableList(0),
	m_flagList (0),
	m_bannerList (0),
	m_customizationIntList (0),
	m_vehicleThrusterDataList (0),
	m_vehicleGroundEffectDataList (0),
	m_vehicleLightningEffectDataList (0),
	m_contrailDataVector(NULL),
	m_glowDataVector(NULL),
	m_interpolatedSound(NULL),
	m_breakpointVector(NULL),
	m_destructionEffect(NULL),
	m_destructionSequence(NULL),
	m_interpolatedSoundVector(NULL)
{
}

//-------------------------------------------------------------------

ClientDataFile::~ClientDataFile ()
{
	if (m_cachedClientEffectTemplateMap)
	{
		for (CachedClientEffectTemplateMap::iterator iter = m_cachedClientEffectTemplateMap->begin(); iter != m_cachedClientEffectTemplateMap->end(); ++iter)
		{
			//-- If the template doesn't exist, we store a newed CrcString.  Otherwise, the CrcString is in the template.
			if (iter->second)
				iter->second->release();
			else
				delete iter->first;
		}

		m_cachedClientEffectTemplateMap->clear();
		delete m_cachedClientEffectTemplateMap;
	}

	if (m_cachedSoundTemplateMap)
	{
		for (CachedSoundTemplateMap::iterator iter = m_cachedSoundTemplateMap->begin(); iter != m_cachedSoundTemplateMap->end(); ++iter)
		{
			//-- If the template doesn't exist, we store a newed CrcString.  Otherwise, the CrcString is in the template.
			if (iter->second)
				SoundTemplateList::release(iter->second);
			else
				delete iter->first;
		}

		m_cachedSoundTemplateMap->clear();
		delete m_cachedSoundTemplateMap;
	}

	delete m_interpolatedSoundVector;
	m_interpolatedSoundVector = NULL;

	delete m_destructionSequence;
	m_destructionSequence = NULL;

	delete m_interpolatedSound;
	m_interpolatedSound = NULL;
	
	if (m_destructionEffect != NULL)
	{
		m_destructionEffect->release();
		m_destructionEffect = NULL;
	}

	if (m_breakpointVector != NULL)
	{
		delete m_breakpointVector;
		m_breakpointVector = NULL;
	}

	if (m_glowDataVector != NULL)
	{
		std::for_each (m_glowDataVector->begin(), m_glowDataVector->end(), PointerDeleter ());
		delete m_glowDataVector;
		m_glowDataVector = NULL;
	}

	if (m_vehicleThrusterDataList)
	{
		std::for_each (m_vehicleThrusterDataList->begin(), m_vehicleThrusterDataList->end(), PointerDeleter ());
		delete m_vehicleThrusterDataList;
		m_vehicleThrusterDataList = 0;
	}

	if (m_vehicleGroundEffectDataList)
	{
		std::for_each (m_vehicleGroundEffectDataList->begin(), m_vehicleGroundEffectDataList->end(), PointerDeleter ());
		delete m_vehicleGroundEffectDataList;
		m_vehicleGroundEffectDataList = 0;
	}
	
	if (m_vehicleLightningEffectDataList)
	{
		std::for_each (m_vehicleLightningEffectDataList->begin(), m_vehicleLightningEffectDataList->end(), PointerDeleter ());
		delete m_vehicleLightningEffectDataList;
		m_vehicleLightningEffectDataList = 0;
	}

	if (m_contrailDataVector)
	{
		std::for_each (m_contrailDataVector->begin(), m_contrailDataVector->end(), PointerDeleter ());
		delete m_contrailDataVector;
		m_contrailDataVector = NULL;
	}

	if (m_customizationIntList)
	{
		std::for_each (m_customizationIntList->begin(), m_customizationIntList->end(), PointerDeleter ());
		delete m_customizationIntList;
	}

	if (m_bannerList)
	{
		std::for_each (m_bannerList->begin(), m_bannerList->end(), PointerDeleter ());
		delete m_bannerList;
	}

	if (m_flagList)
	{
		std::for_each (m_flagList->begin(), m_flagList->end(), PointerDeleter ());
		delete m_flagList;
	}

	if (m_wearableList)
	{
		std::for_each (m_wearableList->begin(), m_wearableList->end(), PointerDeleter ());
		delete m_wearableList;
	}

	if (m_transformChildObjectList)
	{
		std::for_each (m_transformChildObjectList->begin (), m_transformChildObjectList->end (), PointerDeleter ());
		delete m_transformChildObjectList;
	}

	if (m_hardpointChildObjectList)
	{
		std::for_each (m_hardpointChildObjectList->begin (), m_hardpointChildObjectList->end (), PointerDeleter ());
		delete m_hardpointChildObjectList;
	}

	if (m_initialHardpointChildObjectList)
	{
		std::for_each (m_initialHardpointChildObjectList->begin (), m_initialHardpointChildObjectList->end (), PointerDeleter ());
		delete m_initialHardpointChildObjectList;
	}

	if (m_transformLightObjectList)
	{
		std::for_each (m_transformLightObjectList->begin (), m_transformLightObjectList->end (), PointerDeleter ());
		delete m_transformLightObjectList;
	}

	if (m_hardpointLightObjectList)
	{
		std::for_each (m_hardpointLightObjectList->begin (), m_hardpointLightObjectList->end (), PointerDeleter ());
		delete m_hardpointLightObjectList;
	}

	if (m_eventToEffectMap)
		delete m_eventToEffectMap;

	if (m_ambientSoundFileName)
		delete m_ambientSoundFileName;

	if (m_sunTrackingObjectData)
		delete m_sunTrackingObjectData;

	if (m_turretObjectData)
		delete m_turretObjectData;

	if (m_clearFloraEntryList)
		delete m_clearFloraEntryList;

	if (m_damageLevelList)
	{
		std::for_each (m_damageLevelList->begin (), m_damageLevelList->end (), PointerDeleter ());
		delete m_damageLevelList;
	}

	if (m_onOffObjectList)
	{
		std::for_each (m_onOffObjectList->begin (), m_onOffObjectList->end (), PointerDeleter ());
		delete m_onOffObjectList;
	}
}

//-------------------------------------------------------------------

void ClientDataFile::loadFromIff (Iff& iff)
{
	load (iff);
}

// ------------------------------------------------------------------
/**
 * Returns true if there are any client-baked wearables associated with this
 * client data file.
 *
 * @return  true if there are any wearables associated with the client data file;
 *          otherwise returns false.
 */

bool ClientDataFile::hasWearables() const
{
	return m_wearableList && !m_wearableList->empty();
}

//-------------------------------------------------------------------

void ClientDataFile::load (Iff& iff)
{
	if (iff.enterForm (TAG_CLDF, true))
	{
		switch (iff.getCurrentName ())
		{
		case TAG_0000:
			load_0000 (iff);
			break;

		default:
			{
#ifdef _DEBUG
				char tagString [5];
				ConvertTagToString (iff.getCurrentName (), tagString);
				DEBUG_WARNING (true, ("ClientDataFile: [%s] - unknown version %s", iff.getFileName (), tagString));
#endif
			}
			break;
		}

		iff.exitForm (TAG_CLDF);
	}
	else
	{
#ifdef _DEBUG
		char tagString [5];
		ConvertTagToString (iff.getCurrentName (), tagString);
		DEBUG_WARNING (true, ("ClientDataFile: [%s] - unknown format %s, expecting CLDF", iff.getFileName (), tagString));
#endif
	}

	//-- sort damage levels
	if (m_damageLevelList)
		std::sort(m_damageLevelList->begin(), m_damageLevelList->end(), DamageLevel::DamageSorter);
}

//-------------------------------------------------------------------

void ClientDataFile::preloadAssets () const
{
	//-- preload transform child objects
	if (m_transformChildObjectList)
	{
		TransformChildObjectList::const_iterator end = m_transformChildObjectList->end ();
		for (TransformChildObjectList::const_iterator i = m_transformChildObjectList->begin (); i != end; ++i)
		{
			TransformChildObject* const transformChildObject = (*i);
			NOT_NULL (transformChildObject);

			if (transformChildObject->m_objectTemplate)
				transformChildObject->m_objectTemplate->preloadAssets ();

			if (transformChildObject->m_appearanceTemplate)
				transformChildObject->m_appearanceTemplate->preloadAssets ();
		}
	}

	//-- preload hardpoint child objects
	if (m_hardpointChildObjectList)
	{
		HardpointChildObjectList::const_iterator end = m_hardpointChildObjectList->end ();
		for (HardpointChildObjectList::const_iterator i = m_hardpointChildObjectList->begin (); i != end; ++i)
		{
			HardpointChildObject* const hardpointChildObject = (*i);
			NOT_NULL (hardpointChildObject);

			if (hardpointChildObject->m_appearanceTemplate)
				hardpointChildObject->m_appearanceTemplate->preloadAssets ();
		}
	}

	//-- preload initial hardpoint child objects
	if (m_initialHardpointChildObjectList)
	{
		InitialHardpointChildObjectList::const_iterator end = m_initialHardpointChildObjectList->end ();
		for (InitialHardpointChildObjectList::const_iterator i = m_initialHardpointChildObjectList->begin (); i != end; ++i)
		{
			InitialHardpointChildObject* const hardpointChildObject = (*i);
			NOT_NULL (hardpointChildObject);

			if (hardpointChildObject->m_appearanceTemplate)
				hardpointChildObject->m_appearanceTemplate->preloadAssets ();
		}
	}

	//-- sun tracking object
	if (m_sunTrackingObjectData && m_sunTrackingObjectData->m_valid)
	{
		m_sunTrackingObjectData->m_sunTrackingAppearanceTemplate->preloadAssets ();
		m_sunTrackingObjectData->m_barrelAppearanceTemplate->preloadAssets ();
	}

	//-- turrets
	if (m_turretObjectData && m_turretObjectData->m_valid)
	{
		m_turretObjectData->m_turretAppearanceTemplate->preloadAssets ();
		m_turretObjectData->m_barrelAppearanceTemplate->preloadAssets ();
		m_turretObjectData->m_weaponObjectTemplate->preloadAssets ();
	}

	//-- damage level objects
	if (m_damageLevelList)
	{
		DamageLevelList::const_iterator end = m_damageLevelList->end ();
		for (DamageLevelList::const_iterator i = m_damageLevelList->begin (); i != end; ++i)
		{
			const DamageLevel* const damageLevel = (*i);
			if (damageLevel->m_appearanceTemplate)
				damageLevel->m_appearanceTemplate->preloadAssets();
		}
	}

	//-- on/off objects
	if (m_onOffObjectList)
	{
		OnOffObjectList::const_iterator end = m_onOffObjectList->end ();
		for (OnOffObjectList::const_iterator i = m_onOffObjectList->begin (); i != end; ++i)
		{
			const OnOffObject * const onOffObject = (*i);
			if (onOffObject->m_appearanceTemplate)
				onOffObject->m_appearanceTemplate->preloadAssets();
		}
	}

	//-- ambient sound
	if (m_ambientSoundFileName && !m_ambientSoundFileName->empty ())
		cacheSoundTemplate(m_ambientSoundFileName->c_str());

	//-- damage level objects
	if (m_damageLevelList)
	{
		DamageLevelList::const_iterator end = m_damageLevelList->end ();
		for (DamageLevelList::const_iterator i = m_damageLevelList->begin (); i != end; ++i)
		{
			const DamageLevel* const damageLevel = (*i);
			if (!damageLevel->m_ambientSoundTemplateName.empty ())
				cacheSoundTemplate(damageLevel->m_ambientSoundTemplateName.c_str());
		}
	}

	//-- on/off objects
	if (m_onOffObjectList)
	{
		OnOffObjectList::const_iterator end = m_onOffObjectList->end ();
		for (OnOffObjectList::const_iterator i = m_onOffObjectList->begin (); i != end; ++i)
		{
			const OnOffObject* const onOffObject = (*i);
			if (!onOffObject->m_ambientSoundTemplateName.empty ())
				cacheSoundTemplate(onOffObject->m_ambientSoundTemplateName.c_str());
		}
	}

	//-- events
	if (m_eventToEffectMap)
	{
		EventToEffectMap::const_iterator end = m_eventToEffectMap->end ();
		for (EventToEffectMap::const_iterator i = m_eventToEffectMap->begin (); i != end; ++i)
		{
			const CrcLowerString& name = i->second;
			const char* const string = name.getString ();
			if (string)
			{
				switch (string[0])
				{
					case 'c':
						cacheClientEffectTemplate(name.getString());
						break;

					case 'v':
					case 's':
						cacheSoundTemplate(name.getString());
						break;

					default:
						DEBUG_WARNING(true, ("Effect name does not begin with c or s: %s", string));
						break;
				}
			}
		}
	}

	//-- interpolated sound
	if (m_interpolatedSound != NULL && !m_interpolatedSound->m_soundTemplateName.empty())
		cacheSoundTemplate(m_interpolatedSound->m_soundTemplateName.c_str());

	//-- preload client-baked wearables
	if (m_wearableList)
	{
		WearableList::const_iterator const endIt = m_wearableList->end();
		for (WearableList::const_iterator it = m_wearableList->begin(); it != endIt; ++it)
		{
			Wearable const *const wearable = *it;
			if (wearable)
				wearable->preloadAssets();
		}
	}

	//-- preload the flag list.
	if (m_flagList)
		std::for_each(m_flagList->begin(), m_flagList->end(), VoidMemberFunction(&Flag::preloadAssets));

	//-- preload the banner list.
	if (m_bannerList)
		std::for_each(m_bannerList->begin(), m_bannerList->end(), VoidMemberFunction(&Banner::preloadAssets));

	//-- vehicle thruster data
	if (m_vehicleThrusterDataList)
	{
		VehicleThrusterDataList::const_iterator end = m_vehicleThrusterDataList->end();
		for (VehicleThrusterDataList::const_iterator iter = m_vehicleThrusterDataList->begin (); iter != end; ++iter)
		{
			VehicleThrusterData const * const vehicleThrusterData = *iter;
			if (vehicleThrusterData)
			{
				cacheSoundTemplate(vehicleThrusterData->m_soundData.m_idle.c_str());
				cacheSoundTemplate(vehicleThrusterData->m_soundData.m_accel.c_str());
				cacheSoundTemplate(vehicleThrusterData->m_soundData.m_decel.c_str());
				cacheSoundTemplate(vehicleThrusterData->m_soundData.m_run.c_str());
				cacheSoundTemplate(vehicleThrusterData->m_soundData.m_damageIdle.c_str());
				cacheSoundTemplate(vehicleThrusterData->m_soundData.m_damageAccel.c_str());
				cacheSoundTemplate(vehicleThrusterData->m_soundData.m_damageDecel.c_str());
				cacheSoundTemplate(vehicleThrusterData->m_soundData.m_damageRun.c_str());
			}
		}
	}
}

//-------------------------------------------------------------------

void ClientDataFile::apply (Object* const object) const
{
	if (!object)
	{
		DEBUG_WARNING (true, ("called ClientDataFile::apply () on a NULL object, ignoring."));
		return;
	}

	//-- add customization variables.
	applyCustomizationVariableOverrides(*object);

	//-- attach transform child objects
	if (m_transformChildObjectList)
	{
		TransformChildObjectList::const_iterator end = m_transformChildObjectList->end ();
		for (TransformChildObjectList::const_iterator i = m_transformChildObjectList->begin (); i != end; ++i)
		{
			TransformChildObject* const transformChildObject = (*i);
			NOT_NULL (transformChildObject);

			Object* objectToAdd = 0;
			if (transformChildObject->m_objectTemplate)
			{
				objectToAdd = transformChildObject->m_objectTemplate->createObject ();
				safe_cast<ClientObject*> (objectToAdd)->endBaselines ();
			}
			else
			{
				objectToAdd = new Object ();

				if (transformChildObject->m_appearanceTemplate)
					objectToAdd->setAppearance (transformChildObject->m_appearanceTemplate->createAppearance ());
			}

			if (transformChildObject->m_useHardpoint)
			{
				bool hardpointExists = false;
				if (object->getAppearance())
				{
					Transform hardpointTransform;
					if (object->getAppearance()->findHardpoint(ConstCharCrcString(transformChildObject->m_hardpointName.c_str()), hardpointTransform))
					{
						objectToAdd->setTransform_o2p(hardpointTransform);
						hardpointExists = true;
					}
				}

				DEBUG_WARNING(!hardpointExists, ("ClientDataFile: [%s] - missing hardpoint [%s] on object %s", getName(), transformChildObject->m_hardpointName.c_str(), object->getDebugInformation(true).c_str()));
			}
			else
			{
				objectToAdd->setPosition_p(transformChildObject->m_position);
				objectToAdd->yaw_o(transformChildObject->m_orientation.x);
				objectToAdd->pitch_o(transformChildObject->m_orientation.y);
				objectToAdd->roll_o(transformChildObject->m_orientation.z);
			}

			RenderWorld::addObjectNotifications (*objectToAdd);

			if (transformChildObject->m_rotationsPerSecond.magnitude () > 0.f)
			{
				objectToAdd->setDynamics (new RotationDynamics (objectToAdd, transformChildObject->m_rotationsPerSecond));
			}
			else
				if (transformChildObject->m_seesawsPerSecond.magnitude () > 0.f)
				{
					SeesawDynamics::Type type = SeesawDynamics::T_roll;
					if (transformChildObject->m_seesawsPerSecond.x > 0.f)
						type = SeesawDynamics::T_yaw;
					else
						if (transformChildObject->m_seesawsPerSecond.y > 0.f)
							type = SeesawDynamics::T_pitch;

					objectToAdd->setDynamics (new SeesawDynamics (objectToAdd, type, transformChildObject->m_seesawsPerSecond.magnitude (), transformChildObject->m_seesawMagnitude));
				}
				else
					if (transformChildObject->m_springOffset.magnitude () > 0.f)
					{
						Vector direction = transformChildObject->m_springOffset;
						IGNORE_RETURN (direction.normalize ());

						objectToAdd->setDynamics (new SpringDynamics (objectToAdd, transformChildObject->m_position, direction, transformChildObject->m_springOffset.magnitude (), transformChildObject->m_springsPerSecond));
					}
					else
						if (transformChildObject->m_rotation != 0.f)
						{
							objectToAdd->setDynamics(new WingDynamics(objectToAdd, transformChildObject->m_rotation, transformChildObject->m_rotationTime, transformChildObject->m_soundTemplate));
						}

			object->addChildObject_o (objectToAdd);
		} //lint !e429 // custodial pointer 'objectToAdd' has not been freed or returned // it's okay, owned by object.
	}

	//-- attach hardpoint child objects
	if (m_hardpointChildObjectList)
	{
		HardpointChildObjectList::const_iterator end = m_hardpointChildObjectList->end ();
		for (HardpointChildObjectList::const_iterator i = m_hardpointChildObjectList->begin (); i != end; ++i)
		{
			HardpointChildObject* const hardpointChildObject = (*i);
			NOT_NULL (hardpointChildObject);

			Object* const objectToAdd = new HardpointObject (CrcLowerString (hardpointChildObject->m_hardpointName.c_str ()));
			objectToAdd->setDebugName("CdfHpo");

			if (hardpointChildObject->m_appearanceTemplate)
				objectToAdd->setAppearance (hardpointChildObject->m_appearanceTemplate->createAppearance ());

			RenderWorld::addObjectNotifications (*objectToAdd);

			object->addChildObject_o (objectToAdd);
		}
	}

	//-- attach initial hardpoint child objects
	if (m_initialHardpointChildObjectList)
	{
		InitialHardpointChildObjectList::const_iterator end = m_initialHardpointChildObjectList->end ();
		for (InitialHardpointChildObjectList::const_iterator i = m_initialHardpointChildObjectList->begin (); i != end; ++i)
		{
			InitialHardpointChildObject * const hardpointChildObject = NON_NULL(*i);
			CrcLowerString hardpointName(hardpointChildObject->m_hardpointName.c_str());

			Transform hardpointTransform(Transform::IF_none);
			Object * const hardpointParent = findHardpointParent(*object, hardpointName, hardpointTransform);
			if (hardpointParent)
			{
				Object * const objectToAdd = new Object();

				if (hardpointChildObject->m_appearanceTemplate)
					objectToAdd->setAppearance(hardpointChildObject->m_appearanceTemplate->createAppearance ());

				if (hardpointChildObject->m_recoilDistance)
				{
					objectToAdd->setDynamics(
						new ShipWeaponBarrelDynamics(
							objectToAdd,
							hardpointTransform,
							hardpointChildObject->m_recoilDistance,
							hardpointChildObject->m_recoilRate,
							hardpointChildObject->m_recoilReturnRate));
				}

				RenderWorld::addObjectNotifications(*objectToAdd);

				objectToAdd->setTransform_o2p(hardpointTransform);
				hardpointParent->addChildObject_o(objectToAdd);
			} //lint !e429 // hardpointParent takes ownership of objectToAdd
		}
	}

	//-- attach transform light objects
	if (m_transformLightObjectList)
	{
		TransformLightObjectList::const_iterator end = m_transformLightObjectList->end ();
		for (TransformLightObjectList::const_iterator i = m_transformLightObjectList->begin (); i != end; ++i)
		{
			TransformLightObject* const transformLightObject = (*i);
			NOT_NULL (transformLightObject);

			Light* const light = createLight (*transformLightObject);
			if (light)
			{
				light->setPosition_p (transformLightObject->m_position);
				light->yaw_o (transformLightObject->m_orientation.x);
				light->pitch_o (transformLightObject->m_orientation.y);
				light->roll_o (transformLightObject->m_orientation.z);
				object->addChildObject_o (light);
			}
		}
	}

	//-- attach hardpoint light objects
	if (m_hardpointLightObjectList)
	{
		HardpointLightObjectList::const_iterator end = m_hardpointLightObjectList->end ();
		for (HardpointLightObjectList::const_iterator i = m_hardpointLightObjectList->begin (); i != end; ++i)
		{
			HardpointLightObject* const hardpointLightObject = (*i);
			NOT_NULL (hardpointLightObject);

			Light* const light = createLight (*hardpointLightObject);
			if (light)
			{
				Object* const objectToAdd = new HardpointObject (CrcLowerString (hardpointLightObject->m_hardpointName.c_str ()));
				objectToAdd->setDebugName("CdfLight");
				objectToAdd->addChildObject_o (light);

				object->addChildObject_o (objectToAdd);
			}
		}
	}

	//-- sun tracking object
	if (m_sunTrackingObjectData && m_sunTrackingObjectData->m_valid)
	{
		const Appearance* const baseAppearance = object->getAppearance ();

		if (baseAppearance)
		{
			SunTrackingObject* const sunTrackingObject = new SunTrackingObject (m_sunTrackingObjectData->m_sunTrackingYawMaximumRadiansPerSecond);
			sunTrackingObject->setAppearance (m_sunTrackingObjectData->m_sunTrackingAppearanceTemplate->createAppearance ());
			RenderWorld::addObjectNotifications (*sunTrackingObject);

			Transform hardpointTransform;
			if (!baseAppearance->findHardpoint (m_sunTrackingObjectData->m_sunTrackingHardpointName, hardpointTransform))
				DEBUG_WARNING (true, ("ClientDataFile: [%s] - missing hardpoint [%s] on object [%s]", getName(), m_sunTrackingObjectData->m_sunTrackingHardpointName.getString (), baseAppearance->getAppearanceTemplate()->getName ()));

			sunTrackingObject->setTransform_o2p (hardpointTransform);
			object->addChildObject_o (sunTrackingObject);

			Object* const barrelObject = new Object ();
			barrelObject->setAppearance (m_sunTrackingObjectData->m_barrelAppearanceTemplate->createAppearance ());
			RenderWorld::addObjectNotifications (*barrelObject);

			if (!sunTrackingObject->getAppearance ()->findHardpoint (m_sunTrackingObjectData->m_barrelHardpointName, hardpointTransform))
				DEBUG_WARNING (true, ("ClientDataFile: [%s] - missing hardpoint [%s] on object [%s]", getName(), m_sunTrackingObjectData->m_barrelHardpointName.getString (), m_sunTrackingObjectData->m_sunTrackingAppearanceTemplateName.c_str ()));

			barrelObject->setTransform_o2p (hardpointTransform);
			sunTrackingObject->setBarrel (barrelObject, m_sunTrackingObjectData->m_barrelPitchMinimumRadians, m_sunTrackingObjectData->m_barrelPitchMaximumRadians, m_sunTrackingObjectData->m_barrelPitchMaximumRadiansPerSecond);
			sunTrackingObject->addChildObject_o (barrelObject);
		} //lint !e429 // custodial pointer 'barrelObject' and 'sunTrackingObject' have not been freed or returned // it's okay, sunTrackingObject owns barrelObject, object owns sunTrackingObject.
	}

	//-- turrets
	if (m_turretObjectData && m_turretObjectData->m_valid)
	{
		const Appearance* const baseAppearance = object->getAppearance ();

		if (baseAppearance)
		{
			TurretObject* const turretObject = new TurretObject (m_turretObjectData->m_turretYawMaximumRadiansPerSecond);
			turretObject->setAppearance (m_turretObjectData->m_turretAppearanceTemplate->createAppearance ());
			RenderWorld::addObjectNotifications (*turretObject);

			Transform hardpointTransform;
			if (!baseAppearance->findHardpoint (m_turretObjectData->m_turretHardpointName, hardpointTransform))
				DEBUG_WARNING (true, ("ClientDataFile: [%s] - missing hardpoint [%s] on object [%s]", getName(), m_turretObjectData->m_turretHardpointName.getString (), baseAppearance->getAppearanceTemplate()->getName ()));

			turretObject->setTransform_o2p (hardpointTransform);
			object->addChildObject_o (turretObject);

			Object* const barrelObject = new Object ();
			barrelObject->setAppearance (m_turretObjectData->m_barrelAppearanceTemplate->createAppearance ());
			RenderWorld::addObjectNotifications (*barrelObject);

			if (!turretObject->getAppearance ()->findHardpoint (m_turretObjectData->m_barrelHardpointName, hardpointTransform))
				DEBUG_WARNING (true, ("ClientDataFile: [%s] - missing hardpoint [%s] on object [%s]", getName(), m_turretObjectData->m_barrelHardpointName.getString (), m_turretObjectData->m_turretAppearanceTemplateName.c_str ()));

			barrelObject->setTransform_o2p (hardpointTransform);
			turretObject->setBarrel (barrelObject, m_turretObjectData->m_barrelPitchMinimumRadians, m_turretObjectData->m_barrelPitchMaximumRadians, m_turretObjectData->m_barrelPitchMaximumRadiansPerSecond);
			turretObject->addChildObject_o (barrelObject);

			if (!barrelObject->getAppearance ()->findHardpoint (m_turretObjectData->m_muzzleHardpointName, hardpointTransform))
				DEBUG_WARNING (true, ("ClientDataFile: [%s] - missing hardpoint [%s] on object [%s]", getName(), m_turretObjectData->m_muzzleHardpointName.getString (), m_turretObjectData->m_barrelAppearanceTemplateName.c_str ()));

			turretObject->setWeapon (m_turretObjectData->m_weaponObjectTemplate, hardpointTransform, m_turretObjectData->m_speed, m_turretObjectData->m_expirationTime);
		} //lint !e429 // custodial pointer 'barrelObject' and 'turrentObject' have not been freed or returned // it's okay, turretObject owns barrelObject, object owns turretObject.
	}

	//-- add the clear flora notification
	if (m_clearFloraEntryList && !m_clearFloraEntryList->empty ())
		object->addNotification (ClearNonCollidableFloraNotification::getInstance ());
	
	if (m_damageLevelList && !m_damageLevelList->empty())
	{
		if (NULL == object->getProperty(DamageProperty::getClassPropertyId()))
		{
			//-- apply necessary damage objects (starts in 0 state)
			bool const wasInWorld = object->isInWorld();
			if (wasInWorld)
				object->removeFromWorld();
			
			object->addProperty (*(new DamageProperty (*object)));
			
			if (wasInWorld)
				object->addToWorld();
		}
	}

	applyDamage (object, 0);

	//-- object starts in the off state, so apply the off objects
	bool const on = object->asClientObject() && object->asClientObject()->asTangibleObject() && object->asClientObject()->asTangibleObject()->hasCondition(TangibleObject::C_onOff);
	applyOnOff (object, on);

	//-- apply client-baked wearables
	if (hasWearables())
	{
		if (object->getAppearance() && object->getAppearance()->asSkeletalAppearance2())
		{
			WearableList::const_iterator const endIt = m_wearableList->end();
			for (WearableList::const_iterator it = m_wearableList->begin(); it != endIt; ++it)
			{
				Wearable const *const wearable = *it;
				if (wearable)
				{
					if (!wearable->apply(object))
					{ 
						WARNING(true, ("ClientDataFile %s failed to apply wearable", getName()));
					}
				}
			}

			if (!m_wearableList->empty())
			{
				//-- Conclude handles locking the appearance wearables so they cannot be modified once client-baked wearables are specified.
				Wearable::concludeApply(object);
			}
		}
		else
			DEBUG_WARNING(true, ("ClientDataFile::apply: object %s has wearables but no skeletal appearance", object->getDebugInformation(true)));
	}

	//-- apply flags.
	if (m_flagList)
		std::for_each(m_flagList->begin(), m_flagList->end(), VoidBindSecond(VoidMemberFunction(&Flag::apply), object));

	//-- apply banners.
	if (m_bannerList)
		std::for_each(m_bannerList->begin(), m_bannerList->end(), VoidBindSecond(VoidMemberFunction(&Banner::apply), object));

	applyGlows(*object, false);
}

//----------------------------------------------------------------------

/**
* @param skipSelfGlows use this to skip all glows which can be applied to the object itself, without ascending the parents.  This is used when this method is called manually after an object has been added to an object heirarchy, and the self glows can be assumed to have been pre-established
*/

void ClientDataFile::applyGlows(Object & object, bool skipSelfGlows) const
{
	//-- apply glow objects
	if (NULL == m_glowDataVector)
		return;
	
	Appearance const * const baseAppearance = object.getAppearance ();
	Object const * const parent = object.getParent();
	
	for (GlowDataVector::const_iterator it = m_glowDataVector->begin(); it != m_glowDataVector->end(); ++it)
	{
		GlowData const * const glowData = NON_NULL(*it);				
		std::string const & hardpointName = glowData->m_hardpointName;
		
		if (!hardpointName.empty())
		{
			TemporaryCrcString const hardpointCrcName(hardpointName.c_str(), true);
			Transform hardpointTransform;

			if (NULL == baseAppearance || !baseAppearance->findHardpoint(hardpointCrcName, hardpointTransform))
			{
				//-- no appearance andor no hardpoint on the appearance.
				//-- skip this glow if there is no parent onto which to ascend
				if (NULL == parent)
					continue;
			}
			else if (skipSelfGlows)
				continue;
			
			GlowAppearance * const glowAppearance = glowData->createAppearance();
			
			if (glowAppearance == NULL)
			{
				WARNING(true, ("ClientDataFile [%s] failed to create glow appearance", getName()));
				continue;
			}
			
			HardpointObject * const hardpointObject = new HardpointObject(hardpointCrcName);
			if (!glowData->m_name.empty())
				hardpointObject->setDebugName(glowData->m_name.c_str());
			else
				hardpointObject->setDebugName("unnamed_glow");

			hardpointObject->setAppearance(glowAppearance);
			RenderWorld::addObjectNotifications(*hardpointObject);
						
			if (!addHardpointObjectToFirstAppropriateParent(*hardpointObject, object))
			{
				WARNING(true, ("ClientDataFile [%s] invalid glow hardpoint [%s]", getName(), hardpointName.c_str()));
				object.addChildObject_o(hardpointObject);
			}
		}
		else if (!skipSelfGlows)
		{
			Object * const nonHardpointObject = new Object;
			
			GlowAppearance * const glowAppearance = glowData->createAppearance();
			
			if (glowAppearance == NULL)
			{
				WARNING(true, ("ClientDataFile [%s] failed to create glow appearance", getName()));
				continue;
			}
			
			if (!glowData->m_name.empty())
				nonHardpointObject->setDebugName(glowData->m_name.c_str());
			nonHardpointObject->setAppearance(glowAppearance);
			RenderWorld::addObjectNotifications(*nonHardpointObject);
			object.addChildObject_o(nonHardpointObject);
			nonHardpointObject->move_p(glowData->m_position);
			float const theta = glowData->m_position.theta();
			nonHardpointObject->yaw_o(theta);
		} //lint !e429 //custodial nonHardpointObject
	}
}

//-------------------------------------------------------------------

void ClientDataFile::applyOnOff (Object* const object, const bool on) const
{
	applyAmbientSounds (object, on);

	//-- scan all child objects and turn them on/off
	{
		const int numberOfChildObjects = object->getNumberOfChildObjects();
		for (int i = 0; i < numberOfChildObjects; ++i)
		{
			Object* const childObject = object->getChildObject (i);

			//-- SunTrackingObject
			{
				SunTrackingObject* const sunTrackingObject = dynamic_cast<SunTrackingObject*> (childObject);

				if (sunTrackingObject)
					sunTrackingObject->setState (on);
			}

			//-- StateDynamics
			{
				StateDynamics* const stateDynamics = dynamic_cast<StateDynamics*> (childObject->getDynamics ());
				if (stateDynamics)
					stateDynamics->setState (on);
			}
		}
	}

	if (!m_onOffObjectList || m_onOffObjectList->empty ())
		return;

	//-- remove objects
	removeChildObjects (object, "off");
	removeChildObjects (object, "on");

	if (on)
	{
		//-- add on objects
		uint i;
		for (i = 0; i < m_onOffObjectList->size (); ++i)
		{
			const OnOffObject* const onOffObject = (*m_onOffObjectList) [i];

			if (!onOffObject->m_on)
				continue;

			Object* objectToAdd = 0;
			if (onOffObject->m_useHardpoint)
				objectToAdd = new HardpointObject (CrcLowerString (onOffObject->m_hardpointName.c_str ()));
			else
			{
				objectToAdd = new Object ();
				objectToAdd->setPosition_p (onOffObject->m_position);
				objectToAdd->yaw_o (onOffObject->m_orientation.x);
				objectToAdd->pitch_o (onOffObject->m_orientation.y);
				objectToAdd->roll_o (onOffObject->m_orientation.z);
			}

			if (onOffObject->m_appearanceTemplate)
				objectToAdd->setAppearance (onOffObject->m_appearanceTemplate->createAppearance ());

			RenderWorld::addObjectNotifications (*objectToAdd);
			objectToAdd->setDebugName ("on");

			object->addChildObject_o (objectToAdd);
		} //lint !e429 // custodial pointer 'objectToAdd' has not been freed or returned // it's okay, object owns it.
	}
	else
	{
		//-- add off objects
		uint i;
		for (i = 0; i < m_onOffObjectList->size (); ++i)
		{
			const OnOffObject* const onOffObject = (*m_onOffObjectList) [i];

			if (onOffObject->m_on)
				continue;

			Object* objectToAdd = 0;
			if (onOffObject->m_useHardpoint)
				objectToAdd = new HardpointObject (CrcLowerString (onOffObject->m_hardpointName.c_str ()));
			else
			{
				objectToAdd = new Object ();
				objectToAdd->setPosition_p (onOffObject->m_position);
				objectToAdd->yaw_o (onOffObject->m_orientation.x);
				objectToAdd->pitch_o (onOffObject->m_orientation.y);
				objectToAdd->roll_o (onOffObject->m_orientation.z);
			}

			if (onOffObject->m_appearanceTemplate)
				objectToAdd->setAppearance (onOffObject->m_appearanceTemplate->createAppearance ());

			RenderWorld::addObjectNotifications (*objectToAdd);
			objectToAdd->setDebugName ("off");

			object->addChildObject_o (objectToAdd);
		} //lint !e429 // custodial pointer 'objectToAdd' has not been freed or returned // it's okay, object owns it.
	}
}

//----------------------------------------------------------------------

void ClientDataFile::playDestructionEffect(Object const & object) const
{
	if (m_destructionEffect == NULL)
		return;

	ClientEffect * const clientEffect = m_destructionEffect->createClientEffect(object.getParentCell(), object.getPosition_c(), object.getObjectFrameK_p());
	clientEffect->execute();
	delete clientEffect;
}

//-------------------------------------------------------------------

void ClientDataFile::applyDamage (Object* const object, const bool on, float currentDamageLevel, float desiredDamageLevel) const
{
	UNREF (object);

	currentDamageLevel = clamp (0.f, currentDamageLevel, 1.f);
	desiredDamageLevel = clamp (0.f, desiredDamageLevel, 1.f);

	//-- check to see that the object can be damaged
	if (!m_damageLevelList || m_damageLevelList->empty ())
		return;

	//-- find the current damage level
	int currentDamageLevelIndex = static_cast<int> (m_damageLevelList->size ()) - 1;
	for (; currentDamageLevelIndex >= 0; --currentDamageLevelIndex)
	{
		const DamageLevel* const damageLevel = (*m_damageLevelList) [static_cast<uint> (currentDamageLevelIndex)];
		if (currentDamageLevel >= damageLevel->m_minimumDamageLevel)
			break;
	}

	//-- find the desired damage level
	int desiredDamageLevelIndex = static_cast<int> (m_damageLevelList->size ()) - 1;
	for (; desiredDamageLevelIndex >= 0; --desiredDamageLevelIndex)
	{
		const DamageLevel* const damageLevel = (*m_damageLevelList) [static_cast<uint> (desiredDamageLevelIndex)];
		if (desiredDamageLevel >= damageLevel->m_minimumDamageLevel)
			break;
	}

	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, currentDamageLevelIndex, static_cast<int> (m_damageLevelList->size ()));
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, desiredDamageLevelIndex, static_cast<int> (m_damageLevelList->size ()));

	//-- if they're the same, no change
	if (desiredDamageLevelIndex != currentDamageLevelIndex)
	{
		//-- figure out what needs to be added or subtracted
		const int direction = desiredDamageLevelIndex - currentDamageLevelIndex < 0 ? -1 : 1;

		while (currentDamageLevelIndex != desiredDamageLevelIndex)
		{
			if (direction > 0)
			{
				//-- we're adding
				currentDamageLevelIndex += direction;

				applyDamage (object, currentDamageLevelIndex);
			}
			else
			{
				//-- we're subtracting
				char buffer [10];
				sprintf (buffer, "damage%i", currentDamageLevelIndex);

				removeChildObjects (object, buffer);

				currentDamageLevelIndex += direction;
			}
		}

		DamageProperty* const damageProperty = safe_cast<DamageProperty*> (object->getProperty (DamageProperty::getClassPropertyId ()));
		if (damageProperty)
			damageProperty->setCurrentDamageLevelIndex (desiredDamageLevelIndex);

		applyAmbientSounds (object, on);
	}
}

//-------------------------------------------------------------------

void ClientDataFile::applyAmbientSounds (Object* const object, const bool on) const
{
	purgeAmbientSounds ();

	if (object != NULL)
	{
		const SoundId & soundId = findAmbientSoundForObject	(*object);
		if (soundId.isValid ())
		{
			Audio::stopSound (soundId, 1.0f);
			setAmbientSoundForObject (*object, SoundId::getInvalid());
		}
	}

	//-- if we have damage objects, apply the current damage level sound if we're on
	if (m_damageLevelList && !m_damageLevelList->empty ())
	{
		if (on && object)
		{
			const DamageProperty* const damageProperty = safe_cast<DamageProperty*> (object->getProperty (DamageProperty::getClassPropertyId ()));
			if (damageProperty)
			{
				const int currentDamageLevelIndex = damageProperty->getCurrentDamageLevelIndex ();
				attachSound (object, (*m_damageLevelList) [static_cast<DamageLevelList::size_type>(currentDamageLevelIndex)]->m_ambientSoundTemplateName);
			}
		}

		return;
	}

	//-- if we have on/off objects, only apply the ambient sound if we're on
	if (m_onOffObjectList && !m_onOffObjectList->empty ())
	{
		if (on)
		{
			if (m_ambientSoundFileName)
				attachSound (object, *m_ambientSoundFileName);
		}

		return;
	}

	//-- we have no damage objects and no on/off objects, so just attach the ambient sound
	if (m_ambientSoundFileName)
		attachSound (object, *m_ambientSoundFileName);
}

//-------------------------------------------------------------------

/** This function will return "" if the event isn't found for the object
 */
const CrcLowerString& ClientDataFile::getEffectForEvent(const CrcLowerString& event) const
{
	if (m_eventToEffectMap)
	{
		EventToEffectMap::iterator iter = m_eventToEffectMap->find (event);
		if (iter != m_eventToEffectMap->end ())
		{
			switch (iter->second.getString()[0])
			{
				case 'c':
					cacheClientEffectTemplate(iter->second.getString());
					break;

				case 's':
				case 'v':
					cacheSoundTemplate(iter->second.getString());
					break;
			}

			return iter->second;
		}

		//-- Fall back to generic footstep
		if (strstr (event.getString (), cms_footstep.getString ()) != 0)
		{
			iter = m_eventToEffectMap->find (cms_footstep);
			if (iter != m_eventToEffectMap->end ())
			{
				switch (iter->second.getString()[0])
				{
					case 'c':
						cacheClientEffectTemplate(iter->second.getString());
						break;

					case 's':
					case 'v':
						cacheSoundTemplate(iter->second.getString());
						break;
				}

				return iter->second;
			}
		}
	}

	return cms_empty;
}

//-------------------------------------------------------------------

ClientDataFile::ClearFloraEntryList const * ClientDataFile::getClearFloraEntryList () const
{
	return m_clearFloraEntryList;
}

//===================================================================
// PRIVATE ClientDataFile
//===================================================================

void ClientDataFile::release () const
{
	ClientDataTemplateList::release (*this);
}

//-------------------------------------------------------------------

void ClientDataFile::applyDamage (Object* const object, const int index) const
{
	if (!m_damageLevelList || m_damageLevelList->empty ())
		return;

	const DamageLevel* const damageLevel = (*m_damageLevelList) [static_cast<DamageLevelList::size_type>(index)];

	//-- signal the client effect event
	Vector clientEffectPosition_c = object->getPosition_c ();

	//-- add the appropriate child objects
	if (damageLevel->m_appearanceTemplateName.size () != 0)
	{
		Object* objectToAdd = 0;
		if (damageLevel->m_useHardpoint)
		{
			objectToAdd = new HardpointObject (CrcLowerString (damageLevel->m_hardpointName.c_str ()));
			objectToAdd->setDebugName("CdfDam");

			if (damageLevel->m_appearanceTemplate)
			{
				Appearance * const newAppearance = damageLevel->m_appearanceTemplate->createAppearance();

				if (newAppearance)
				{
					newAppearance->useRenderEffectsFlag(true);
				}

				objectToAdd->setAppearance(newAppearance);
			}

			RenderWorld::addObjectNotifications (*objectToAdd);
		}
		else
		{
			objectToAdd = new Object ();

			if (damageLevel->m_appearanceTemplate)
			{
				Appearance * const newAppearance = damageLevel->m_appearanceTemplate->createAppearance();

				if (newAppearance)
				{
					newAppearance->useRenderEffectsFlag(true);
				}

				objectToAdd->setAppearance(newAppearance);
			}

			objectToAdd->setPosition_p (damageLevel->m_position);
			objectToAdd->yaw_o (damageLevel->m_orientation.x);
			objectToAdd->pitch_o (damageLevel->m_orientation.y);
			objectToAdd->roll_o (damageLevel->m_orientation.z);
			RenderWorld::addObjectNotifications (*objectToAdd);
		}

		char buffer [10];
		sprintf (buffer, "damage%i", index);
		objectToAdd->setDebugName (buffer);

		object->addChildObject_o (objectToAdd);

		if (damageLevel->m_useHardpoint)
			safe_cast<HardpointObject *>(objectToAdd)->snapToPosition ();

		clientEffectPosition_c = objectToAdd->getPosition_c ();
	} //lint !e429 // custodial pointer 'objectToAdd' has not been freed or returned. // it's okay, owned by object.

	if (damageLevel->m_clientEffectTemplate)
	{
		ClientEffect* const clientEffect = damageLevel->m_clientEffectTemplate->createClientEffect (object->getParentCell(), clientEffectPosition_c, Vector::unitY);
		clientEffect->execute ();
		delete clientEffect;
	}
}

//-------------------------------------------------------------------

void ClientDataFile::attachSound (Object const* const object, const std::string& soundTemplateName) const
{
	if (object)
	{
		if (!soundTemplateName.empty ())
		{
			if (TreeFile::exists (soundTemplateName.c_str ()))
			{
				const SoundId soundId = Audio::attachSound (soundTemplateName.c_str (), object);
				setAmbientSoundForObject (*object, soundId);
				DEBUG_WARNING (!soundId.isValid () && Audio::isEnabled(), ("ClientDataFile: [%s] failed to attach sound [%s]", getName(), soundTemplateName.c_str ()));
			}
			else
				DEBUG_WARNING (true, ("ClientDataFile: [%s] sound template %s not found", getName(), soundTemplateName.c_str ()));
		}
	}
}

//-------------------------------------------------------------------

void ClientDataFile::load_0000 (Iff& iff)
{
	iff.enterForm (TAG_0000);

		while (iff.getNumberOfBlocksLeft ())
		{
			switch (iff.getCurrentName ())
			{
			case TAG_DAMA:
				{
					iff.enterForm (TAG_DAMA);

						DamageLevel* const damageLevel = new DamageLevel;

						iff.enterChunk (TAG_INFO);

							damageLevel->m_minimumDamageLevel = iff.read_float ();
							damageLevel->m_maximumDamageLevel = iff.read_float ();
							iff.read_string (damageLevel->m_clientEffectTemplateName);

							if (!damageLevel->m_clientEffectTemplateName.empty ())
							{
								const TemporaryCrcString clientEffectTemplateName (damageLevel->m_clientEffectTemplateName.c_str (), true);
								damageLevel->m_clientEffectTemplate = ClientEffectTemplateList::fetch (CrcLowerString (damageLevel->m_clientEffectTemplateName.c_str ()));
							}

						iff.exitChunk ();

						iff.enterChunk (TAG_APPR);

							iff.read_string (damageLevel->m_appearanceTemplateName);
							if (!damageLevel->m_appearanceTemplateName.empty ())
								damageLevel->m_appearanceTemplate = AppearanceTemplateList::fetch (damageLevel->m_appearanceTemplateName.c_str ());

						iff.exitChunk ();

						if (iff.getCurrentName () == TAG_PSOR)
						{
							iff.enterChunk (TAG_PSOR);

								damageLevel->m_useHardpoint = false;
								damageLevel->m_position = iff.read_floatVector ();
								damageLevel->m_orientation.x = convertDegreesToRadians(iff.read_float());
								damageLevel->m_orientation.y = convertDegreesToRadians(iff.read_float());
								damageLevel->m_orientation.z = convertDegreesToRadians(iff.read_float());

							iff.exitChunk ();
						}
						else
							if (iff.getCurrentName () == TAG_HARD)
							{
								iff.enterChunk (TAG_HARD);

									damageLevel->m_useHardpoint = true;
									iff.read_string (damageLevel->m_hardpointName);

								iff.exitChunk ();
							}

						iff.enterChunk (TAG_ASND);

							iff.read_string (damageLevel->m_ambientSoundTemplateName);

						iff.exitChunk (TAG_ASND);

						if (!m_damageLevelList)
							m_damageLevelList = new DamageLevelList;

						m_damageLevelList->push_back (damageLevel);

					iff.exitForm (TAG_DAMA);
				}
				break;

			case TAG_ASND:
				{
					if (!m_ambientSoundFileName)
					{
						iff.enterChunk(TAG_ASND);

							m_ambientSoundFileName = new std::string;
							*m_ambientSoundFileName = iff.read_stdstring();

						iff.exitChunk (TAG_ASND);
					}
					else
					{
						DEBUG_WARNING(true, ("ClientDataFile: [%s] has more than one ambient sound data specified, skipping all but the first one", getName()));
						iff.enterChunk(TAG_ASND);
						iff.exitChunk(true);
					}
				}
				break;

			case TAG_CEFT:
				{
					iff.enterChunk (TAG_CEFT);

						char* eventName  = iff.read_string();
						char* effectName = iff.read_string();
						const CrcLowerString event(eventName);
						const CrcLowerString effect(effectName);
						delete[] eventName;
						delete[] effectName;

						if (!m_eventToEffectMap)
							m_eventToEffectMap = new EventToEffectMap;

						(*m_eventToEffectMap)[event] = effect;

					iff.exitChunk (TAG_CEFT);
				}
				break;

			case TAG_CHLD:
				{
					iff.enterChunk (TAG_CHLD);

						TransformChildObject* const transformChildObject = new TransformChildObject;
						iff.read_string (transformChildObject->m_appearanceTemplateName);

						if (!transformChildObject->m_appearanceTemplateName.empty ())
							transformChildObject->m_appearanceTemplate = AppearanceTemplateList::fetch (transformChildObject->m_appearanceTemplateName.c_str ());

						transformChildObject->m_position = iff.read_floatVector ();
						transformChildObject->m_orientation.x = convertDegreesToRadians (iff.read_float ());
						transformChildObject->m_orientation.y = convertDegreesToRadians (iff.read_float ());
						transformChildObject->m_orientation.z = convertDegreesToRadians (iff.read_float ());

						transformChildObject->m_rotationsPerSecond = iff.read_floatVector ();
						transformChildObject->m_seesawsPerSecond = iff.read_floatVector ();
						transformChildObject->m_seesawMagnitude = convertDegreesToRadians (iff.read_float ());
						transformChildObject->m_springOffset = iff.read_floatVector ();
						transformChildObject->m_springsPerSecond = iff.read_float ();

						if (!m_transformChildObjectList)
							m_transformChildObjectList = new TransformChildObjectList;

						m_transformChildObjectList->push_back (transformChildObject);

					iff.exitChunk (TAG_CHLD);
				}
				break;

			case TAG_CHL2:
				{
					iff.enterChunk (TAG_CHL2);

						TransformChildObject* const transformChildObject = new TransformChildObject;
						iff.read_string (transformChildObject->m_objectTemplateName);

						if (!transformChildObject->m_objectTemplateName.empty ())
							transformChildObject->m_objectTemplate = ObjectTemplateList::fetch (transformChildObject->m_objectTemplateName.c_str ());

						transformChildObject->m_position = iff.read_floatVector ();
						transformChildObject->m_orientation.x = convertDegreesToRadians (iff.read_float ());
						transformChildObject->m_orientation.y = convertDegreesToRadians (iff.read_float ());
						transformChildObject->m_orientation.z = convertDegreesToRadians (iff.read_float ());

						transformChildObject->m_rotationsPerSecond = iff.read_floatVector ();
						transformChildObject->m_seesawsPerSecond = iff.read_floatVector ();
						transformChildObject->m_seesawMagnitude = convertDegreesToRadians (iff.read_float ());
						transformChildObject->m_springOffset = iff.read_floatVector ();
						transformChildObject->m_springsPerSecond = iff.read_float ();

						if (transformChildObject->m_objectTemplate)
						{
							if (!m_transformChildObjectList)
								m_transformChildObjectList = new TransformChildObjectList;

							m_transformChildObjectList->push_back (transformChildObject);
						}
						else
							delete transformChildObject;

					iff.exitChunk (TAG_CHL2);
				}
				break;

			case TAG_CNCF:
				{
					iff.enterChunk (TAG_CNCF);

						std::pair<Vector, float> clearFloraEntry;
						clearFloraEntry.first  = iff.read_floatVector ();
						clearFloraEntry.second = iff.read_float ();

						if (!m_clearFloraEntryList)
							m_clearFloraEntryList = new ClearFloraEntryList;

						m_clearFloraEntryList->push_back (clearFloraEntry);

					iff.exitChunk (TAG_CNCF);
				}
				break;

			case TAG_CSND:
				{
					iff.enterChunk (TAG_CSND);

						char* const eventName  = iff.read_string ();
						char* const effectName = iff.read_string ();
						const CrcLowerString event (eventName);
						const CrcLowerString effect (effectName);
						delete [] eventName;
						delete [] effectName;

						if (!m_eventToEffectMap)
							m_eventToEffectMap = new EventToEffectMap;

						(*m_eventToEffectMap) [event] = effect;

					iff.exitChunk (TAG_CSND);
				}
				break;

			case TAG_EVNT:
				{
					iff.enterChunk (TAG_EVNT);

						char* eventName  = iff.read_string();
						char* effectName = iff.read_string();
						const CrcLowerString event(eventName);
						const CrcLowerString effect(effectName);
						delete[] eventName;
						delete[] effectName;

						if (!m_eventToEffectMap)
							m_eventToEffectMap = new EventToEffectMap;

						(*m_eventToEffectMap)[event] = effect;

					iff.exitChunk (TAG_EVNT);
				}
				break;

			case TAG_HLOB:
				{
					iff.enterChunk (TAG_HLOB);

						HardpointLightObject* const hardpointLightObject = new HardpointLightObject;

						readLightObject (iff, *hardpointLightObject);
						iff.read_string (hardpointLightObject->m_hardpointName);

						if (!m_hardpointLightObjectList)
							m_hardpointLightObjectList = new HardpointLightObjectList;

						m_hardpointLightObjectList->push_back (hardpointLightObject);

					iff.exitChunk (TAG_HLOB);
				}
				break;

			case TAG_HOBJ:
				{
					iff.enterChunk (TAG_HOBJ);

						HardpointChildObject* const hardpointChildObject = new HardpointChildObject;
						hardpointChildObject->load (iff);

						if (!m_hardpointChildObjectList)
							m_hardpointChildObjectList = new HardpointChildObjectList;

						m_hardpointChildObjectList->push_back (hardpointChildObject);

					iff.exitChunk (TAG_HOBJ);
				}
				break;

			case TAG_IHOB:
				{
					iff.enterChunk (TAG_IHOB);

						InitialHardpointChildObject * const hardpointChildObject = new InitialHardpointChildObject;
						hardpointChildObject->load (iff);

						if (!m_initialHardpointChildObjectList)
							m_initialHardpointChildObjectList = new InitialHardpointChildObjectList;

						m_initialHardpointChildObjectList->push_back (hardpointChildObject);

					iff.exitChunk (TAG_IHOB);
				}
				break;

			case TAG_LOBJ:
				{
					iff.enterChunk (TAG_LOBJ);

						TransformLightObject* const transformLightObject = new TransformLightObject;

						readLightObject (iff, *transformLightObject);
						transformLightObject->m_position = iff.read_floatVector ();

						if (!m_transformLightObjectList)
							m_transformLightObjectList = new TransformLightObjectList;

						m_transformLightObjectList->push_back (transformLightObject);

					iff.exitChunk ();
				}
				break;

			case TAG_ONOF:
				{
					iff.enterForm (TAG_ONOF);

						OnOffObject* const onOffObject = new OnOffObject;

						iff.enterChunk (TAG_INFO);
							onOffObject->m_on = iff.read_int32 ();
						iff.exitChunk (TAG_INFO);

						iff.enterChunk (TAG_APPR);

							iff.read_string (onOffObject->m_appearanceTemplateName);
							if (!onOffObject->m_appearanceTemplateName.empty ())
								onOffObject->m_appearanceTemplate = AppearanceTemplateList::fetch (onOffObject->m_appearanceTemplateName.c_str ());

						iff.exitChunk ();

						if (iff.getCurrentName () == TAG_PSOR)
						{
							iff.enterChunk (TAG_PSOR);

								onOffObject->m_useHardpoint = false;
								onOffObject->m_position = iff.read_floatVector ();
								onOffObject->m_orientation.x = convertDegreesToRadians(iff.read_float());
								onOffObject->m_orientation.y = convertDegreesToRadians(iff.read_float());
								onOffObject->m_orientation.z = convertDegreesToRadians(iff.read_float());

							iff.exitChunk ();
						}
						else
							if (iff.getCurrentName () == TAG_HARD)
							{
								iff.enterChunk (TAG_HARD);

									onOffObject->m_useHardpoint = true;
									iff.read_string (onOffObject->m_hardpointName);

								iff.exitChunk ();
							}

						if (!m_onOffObjectList)
							m_onOffObjectList = new OnOffObjectList;

						m_onOffObjectList->push_back (onOffObject);

					iff.exitForm (TAG_ONOF);
				}
				break;

			case TAG_SUNT:
				{
					if (!m_sunTrackingObjectData)
					{
						m_sunTrackingObjectData = new SunTrackingObjectData;

						iff.enterChunk (TAG_SUNT);

							char nameBuffer [1000];

							iff.read_string (m_sunTrackingObjectData->m_sunTrackingAppearanceTemplateName);
							m_sunTrackingObjectData->m_sunTrackingAppearanceTemplate = AppearanceTemplateList::fetch (m_sunTrackingObjectData->m_sunTrackingAppearanceTemplateName.c_str ());
							iff.read_string (nameBuffer, 1000);
							m_sunTrackingObjectData->m_sunTrackingHardpointName.setString (nameBuffer);
							m_sunTrackingObjectData->m_sunTrackingYawMaximumRadiansPerSecond = convertDegreesToRadians (iff.read_float ());

							iff.read_string (m_sunTrackingObjectData->m_barrelAppearanceTemplateName);
							m_sunTrackingObjectData->m_barrelAppearanceTemplate = AppearanceTemplateList::fetch (m_sunTrackingObjectData->m_barrelAppearanceTemplateName.c_str ());
							iff.read_string (nameBuffer, 1000);
							m_sunTrackingObjectData->m_barrelHardpointName.setString (nameBuffer);
							m_sunTrackingObjectData->m_barrelPitchMinimumRadians = convertDegreesToRadians (iff.read_float ());
							m_sunTrackingObjectData->m_barrelPitchMaximumRadians = convertDegreesToRadians (iff.read_float ());
							m_sunTrackingObjectData->m_barrelPitchMaximumRadiansPerSecond = convertDegreesToRadians (iff.read_float ());

							m_sunTrackingObjectData->m_valid = true;

						iff.exitChunk (TAG_SUNT);
					}
					else
					{
						DEBUG_WARNING(true, ("ClientDataFile: [%s] has more than one sun tracking data specified, skipping all but the first one", getName()));
						iff.enterForm(TAG_SUNT);
						iff.exitForm(true);
					}
				}
				break;

			case TAG_TURR:
				{
					if (!m_turretObjectData)
					{
						m_turretObjectData = new TurretObjectData;

						iff.enterChunk (TAG_TURR);

							char nameBuffer [1000];

							iff.read_string (m_turretObjectData->m_turretAppearanceTemplateName);
							m_turretObjectData->m_turretAppearanceTemplate = AppearanceTemplateList::fetch (m_turretObjectData->m_turretAppearanceTemplateName.c_str ());
							iff.read_string (nameBuffer, 1000);
							m_turretObjectData->m_turretHardpointName.setString (nameBuffer);
							m_turretObjectData->m_turretYawMaximumRadiansPerSecond = convertDegreesToRadians (iff.read_float ());

							iff.read_string (m_turretObjectData->m_barrelAppearanceTemplateName);
							m_turretObjectData->m_barrelAppearanceTemplate = AppearanceTemplateList::fetch (m_turretObjectData->m_barrelAppearanceTemplateName.c_str ());
							iff.read_string (nameBuffer, 1000);
							m_turretObjectData->m_barrelHardpointName.setString (nameBuffer);
							m_turretObjectData->m_barrelPitchMinimumRadians = convertDegreesToRadians (iff.read_float ());
							m_turretObjectData->m_barrelPitchMaximumRadians = convertDegreesToRadians (iff.read_float ());
							m_turretObjectData->m_barrelPitchMaximumRadiansPerSecond = convertDegreesToRadians (iff.read_float ());

							iff.read_string (m_turretObjectData->m_weaponObjectTemplateName);
							const ObjectTemplate* const objectTemplate = ObjectTemplateList::fetch (m_turretObjectData->m_weaponObjectTemplateName.c_str ());
							m_turretObjectData->m_weaponObjectTemplate = dynamic_cast<const ClientWeaponObjectTemplate*> (objectTemplate);
							iff.read_string (nameBuffer, 1000);
							m_turretObjectData->m_muzzleHardpointName.setString (nameBuffer);
							m_turretObjectData->m_speed = iff.read_float ();
							m_turretObjectData->m_expirationTime = iff.read_float ();

							if (objectTemplate && !m_turretObjectData->m_weaponObjectTemplate)
								objectTemplate->releaseReference ();

							m_turretObjectData->m_valid = true;

						iff.exitChunk (TAG_TURR);
					}
					else
					{
						DEBUG_WARNING(true, ("ClientDataFile: [%s] has more than one turret data specified, skipping all but the first one", getName()));
						iff.enterForm(TAG_TURR);
						iff.exitForm(true);
					}
				}
				break;

			case TAG_WEAR:
				{
					Wearable *const newWearable = Wearable::load_0000(iff);
					if (newWearable)
					{
						if (!m_wearableList)
							m_wearableList = new WearableList;

						m_wearableList->push_back(newWearable);
					}
				}
				break;

			case TAG_FLAG:
				{
					Flag *const newFlag = Flag::load_0000(iff, getName());
					if (newFlag)
					{
						// Create the list as necessary.
						if (!m_flagList)
							m_flagList = new FlagList;

						m_flagList->push_back(newFlag);
					}
				}
				break;

			case TAG_BANR:
				{
					Banner *const newBanner = Banner::load_0000(iff, getName());
					if (newBanner)
					{
						// Create the list as necessary.
						if (!m_bannerList)
							m_bannerList = new BannerList;

						m_bannerList->push_back(newBanner);
					}
				}
				break;

			case TAG_CSSI:
				{
					//-- load the data.
					iff.enterChunk (TAG_CSSI);

						char variableName[256];
						iff.read_string(variableName, sizeof(variableName) - 1);
						variableName[sizeof(variableName) - 1] = '\0';

						int const value = static_cast<int>(iff.read_int32());

					iff.exitChunk (TAG_CSSI);

					//-- create the entry.
					if (!m_customizationIntList)
						m_customizationIntList = new CustomizationIntList;

					m_customizationIntList->push_back (new CustomizationInt (variableName, value));
				}
				break;
				
			case TAG_VTHR:
				{
					VehicleThrusterData * const vtd = new VehicleThrusterData ();
					
					iff.enterForm (TAG_VTHR);
						vtd->load (iff);
					iff.exitForm (TAG_VTHR);
					
					if (!m_vehicleThrusterDataList)
						m_vehicleThrusterDataList = new VehicleThrusterDataList;
					
					m_vehicleThrusterDataList->push_back (vtd);
				}
				break;

			case TAG_VGEF:
				{
					VehicleGroundEffectData * const vged = new VehicleGroundEffectData ();
					
					iff.enterForm (TAG_VGEF);
						vged->load (iff);
					iff.exitForm (TAG_VGEF);
					
					if (!m_vehicleGroundEffectDataList)
						m_vehicleGroundEffectDataList = new VehicleGroundEffectDataList;
					
					m_vehicleGroundEffectDataList->push_back (vged);
				}
				break;

			case TAG_VLEF:
				{
					VehicleLightningEffectData * const lighningEffectData = new VehicleLightningEffectData ();

					iff.enterForm (TAG_VLEF);
						lighningEffectData->load (iff);
					iff.exitForm (TAG_VLEF);

					if (!m_vehicleLightningEffectDataList)
						m_vehicleLightningEffectDataList = new VehicleLightningEffectDataList;

					m_vehicleLightningEffectDataList->push_back (lighningEffectData);
				}
				break;

			case TAG_CONT:
				{
					ContrailData * const contrailData = new ContrailData;

					iff.enterForm(TAG_CONT);
						contrailData->load(iff);
					iff.exitForm(TAG_CONT);

					if (!m_contrailDataVector)
						m_contrailDataVector = new ContrailDataVector;

					m_contrailDataVector->push_back(contrailData);
				}
				break;

			case TAG_WING:
				{
					iff.enterForm(TAG_WING);

						TransformChildObject * const transformChildObject = new TransformChildObject;

						iff.enterChunk(TAG_DATA);

							iff.read_string(transformChildObject->m_objectTemplateName);

							if (!transformChildObject->m_objectTemplateName.empty ())
								transformChildObject->m_objectTemplate = ObjectTemplateList::fetch (transformChildObject->m_objectTemplateName.c_str ());


							transformChildObject->m_rotation = convertDegreesToRadians(iff.read_float());
							transformChildObject->m_rotationTime = iff.read_float();
							iff.read_string(transformChildObject->m_soundTemplateName);
							if (!transformChildObject->m_soundTemplateName.empty())
								transformChildObject->m_soundTemplate = SoundTemplateList::fetch(transformChildObject->m_soundTemplateName.c_str());

						iff.exitChunk(TAG_DATA);

						if (iff.getCurrentName () == TAG_PSOR)
						{
							iff.enterChunk (TAG_PSOR);

								transformChildObject->m_useHardpoint = false;
								transformChildObject->m_position = iff.read_floatVector ();
								transformChildObject->m_orientation.x = convertDegreesToRadians (iff.read_float ());
								transformChildObject->m_orientation.y = convertDegreesToRadians (iff.read_float ());
								transformChildObject->m_orientation.z = convertDegreesToRadians (iff.read_float ());

							iff.exitChunk ();
						}
						else
							if (iff.getCurrentName () == TAG_HARD)
							{
								iff.enterChunk (TAG_HARD);

									transformChildObject->m_useHardpoint = true;
									iff.read_string(transformChildObject->m_hardpointName);

								iff.exitChunk ();
							}

						if (!m_transformChildObjectList)
							m_transformChildObjectList = new TransformChildObjectList;

						m_transformChildObjectList->push_back(transformChildObject);

					iff.exitForm(TAG_WING);
				}
				break;

			case TAG_GLOW:
				{
					GlowData * const glowData = new GlowData;

					iff.enterForm(TAG_GLOW);
						glowData->load(iff);
					iff.exitForm(TAG_GLOW);

					if (!m_glowDataVector)
						m_glowDataVector = new GlowDataVector;

					m_glowDataVector->push_back(glowData);
				}
				break;

			case TAG_INTS:
				{
					if (!m_interpolatedSound)
					{
						m_interpolatedSound = new InterpolatedSound;

						iff.enterForm(TAG_INTS);
							m_interpolatedSound->load(iff);
						iff.exitForm(TAG_INTS);
					}
					else
					{
						DEBUG_WARNING(true, ("ClientDataFile: [%s] has more than one interpolated sound specified, skipping all but the first one", getName()));
						iff.enterForm(TAG_INTS);
						iff.exitForm(true);
					}
				}
				break;
				
				//-- engine sounds
			case TAG_ENGS:
				{
					if (NULL == m_interpolatedSoundVector)
						m_interpolatedSoundVector = new InterpolatedSoundVector;
					
					iff.enterForm(TAG_ENGS);
					{
						while (iff.enterForm(TAG_INTS, true))
						{
							InterpolatedSound is;
							is.load(iff);
							m_interpolatedSoundVector->push_back(is);
							iff.exitForm(TAG_INTS);
						}
					}
					iff.exitForm(TAG_ENGS);
				}
				break;

			case TAG_BRKP:
				{
					Breakpoint breakpoint;

					iff.enterForm(TAG_BRKP);
						breakpoint.load(iff);
					iff.exitForm(TAG_BRKP);

					if (!m_breakpointVector)
						m_breakpointVector = new BreakpointVector;

					m_breakpointVector->push_back(breakpoint);
				}
				break;

			case TAG_DSTR:
				{
					if (!m_destructionEffect)
					{
						iff.enterForm(TAG_DSTR);
						{
							iff.enterChunk(TAG_INFO);
							{
								std::string effectName;
								iff.read_string(effectName);

								if (!effectName.empty())
								{
									m_destructionEffect = ClientEffectTemplateList::fetch(ConstCharCrcLowerString(effectName.c_str()));
									WARNING(m_destructionEffect == NULL, ("ClientDataFile [%] failed to load destruction client effect [%s]", effectName.c_str()));
								}
							}
							iff.exitChunk(TAG_INFO);
						}
						iff.exitForm(TAG_DSTR);
					}
					else
					{
						DEBUG_WARNING(true, ("ClientDataFile: [%s] has more than one destruction effect specified, skipping all but the first one", getName()));
						iff.enterForm(TAG_DSTR);
						iff.exitForm(true);
					}
				}
				break;

			case TAG_DSEF:
				{
					if (!m_destructionSequence)
					{
						iff.enterForm(TAG_DSEF);
						{
							m_destructionSequence = new DestructionSequence();
							m_destructionSequence->load(iff);
							m_destructionSequence->preload();
						}
						iff.exitForm(TAG_DSEF);
					}
					else
					{
						DEBUG_WARNING(true, ("ClientDataFile: [%s] has more than one destruction sequence specified, skipping all but the first one", getName()));
						iff.enterForm(TAG_DSEF);
						iff.exitForm(true);
					}
				}
				break;

			default:
				{
#ifdef _DEBUG
					char tagString [5];
					ConvertTagToString (iff.getCurrentName (), tagString);
					DEBUG_WARNING (true, ("ClientDataFile: [%s] - unknown chunk type %s, expecting ASND, CEFT, CHLD, CSND, DAMA, HLOB, HOBJ, or LOBJ", getName(), tagString));
#endif

					if (iff.isCurrentForm ())
					{
						iff.enterForm ();
						iff.exitForm (true);
					}
					else
					{
						iff.enterChunk ();
						iff.exitChunk (true);
					}
				}
				break;
			}
		}

	iff.exitForm (TAG_0000);
}

//-------------------------------------------------------------------

void ClientDataFileNamespace::readLightObject (Iff& iff, LightObject& lightObject)
{
	lightObject.m_type = iff.read_int32 ();
	iff.read_string (lightObject.m_appearanceTemplateName);

	if (!lightObject.m_appearanceTemplateName.empty ())
		lightObject.m_appearanceTemplate = AppearanceTemplateList::fetch (lightObject.m_appearanceTemplateName.c_str ());

	lightObject.m_color1.a = 1.f;
	lightObject.m_color1.r = iff.read_float ();
	lightObject.m_color1.g = iff.read_float ();
	lightObject.m_color1.b = iff.read_float ();
	lightObject.m_color2.a = 1.f;
	lightObject.m_color2.r = iff.read_float ();
	lightObject.m_color2.g = iff.read_float ();
	lightObject.m_color2.b = iff.read_float ();
	lightObject.m_range1 = iff.read_float ();
	lightObject.m_range2 = iff.read_float ();
	lightObject.m_time1 = iff.read_float ();
	lightObject.m_time2 = iff.read_float ();
	lightObject.m_constantAttenuation = iff.read_float ();
	lightObject.m_linearAttenuation = iff.read_float ();
	lightObject.m_quadraticAttenuation = iff.read_float ();
	int32 stateFlags = iff.read_int32();
	lightObject.m_timeOfDayAware = (stateFlags & 0x01);
	lightObject.m_onOffAware = (stateFlags & 0x02);
}

//-------------------------------------------------------------------

Light* ClientDataFileNamespace::createLight (const LightObject& lightObject)
{
	GameLight* light = 0;

	switch (lightObject.m_type)
	{
	default:
	case 0:
		break;

	case 1:
		light = new GameLight ();
		light->setFlicker (false);
		light->setDiffuseColor (lightObject.m_color1);
		light->Light::setRange (lightObject.m_range1);
		break;

	case 2:
		light = new GameLight ();
		light->setFlicker (true);
		light->setColor (lightObject.m_color1, lightObject.m_color2);
		light->setRange (lightObject.m_range1, lightObject.m_range2);
		light->setTime (lightObject.m_time1, lightObject.m_time2);
		break;
	}

	if (light)
	{
		if (lightObject.m_appearanceTemplate)
			light->setAppearance (lightObject.m_appearanceTemplate->createAppearance ());

		light->setConstantAttenuation (lightObject.m_constantAttenuation);
		light->setLinearAttenuation (lightObject.m_linearAttenuation);
		light->setQuadraticAttenuation (lightObject.m_quadraticAttenuation);
		light->setDayNightAware (lightObject.m_timeOfDayAware);
		light->setOnOffAware (lightObject.m_onOffAware);
	}

	return light;
}

//-------------------------------------------------------------------

void ClientDataFileNamespace::removeChildObjects (Object* const object, const char* const debugName)
{
	int n = object->getNumberOfChildObjects ();
	int i = 0;
	while (i < n)
	{
		Object* const childObject = object->getChildObject (i);
		if (childObject->getDebugName () && _stricmp (childObject->getDebugName (), debugName) == 0)
		{
			delete childObject;
			--n;
		}
		else
			++i;
	}
}

//===================================================================
// class ClientDataFile: PRIVATE
// ======================================================================

void ClientDataFile::applyCustomizationVariableOverrides (Object &object) const
{
	//-- Stop if there's no customization variable overrides.
	if (!m_customizationIntList || m_customizationIntList->empty ())
		return;

	//-- Make the customization data property if one isn't already attached.
	CustomizationDataProperty *cdProperty = safe_cast<CustomizationDataProperty*> (object.getProperty (CustomizationDataProperty::getClassPropertyId ()));
	if (!cdProperty)
	{
		DEBUG_REPORT_LOG (s_logUndeclaredCustomizations, ("ClientDataFile::apply (): client data file specifies customization int values but object template [%s] does not declare any customization variables, creating the CustomizationDataProperty.\n", object.getObjectTemplateName ()));
		cdProperty = new CustomizationDataProperty (object);
		object.addProperty (*cdProperty);
	}

	//-- Fetch the CustomizationData from the property.
	NOT_NULL (cdProperty);
	CustomizationData *const customizationData = cdProperty->fetchCustomizationData ();
	if (!customizationData)
	{
		DEBUG_FATAL(true, ("CustomizationDataProperty::fetchCustomizationData() returned a NULL pointer."));
		return; //lint !e429 !e527 // custodial cdProperty has not been freed or returned // object owns it, it's okay // unreachable // reachable in release.
	}

#if CUSTOMIZATION_WARN_ON_OVERRIDE_NON_DEFAULT
	//-- Create a temporary CustomizationData for object that contains all the default artist values
	//   for each variable.  We'll use this later to see if we're about to change a value that has been
	//   changed from the default already.  If we hit this case, we want to issue a warning since most
	//   likely this is a case of a server script setting up customizations and the client accidentally
	//   overwriting them.

	// ... create the customization data for defaults.
	CustomizationData *const customizationDataWithDefaults = new CustomizationData(object);
	customizationDataWithDefaults->fetch();

	// ... get object's appearance template name.
	Appearance const *const baseAppearance = object.getAppearance();
	if (!baseAppearance)
	{
		DEBUG_WARNING(true, ("CustomizationDataProperty::applyCustomizationVariableOverrides: object %s doesn't have an appearance", object.getDebugInformation(true).c_str()));
		return;
	}

	char const *const appearanceTemplateName = baseAppearance->getAppearanceTemplateName();

	// ... set defaults.
	bool const skipSharedOwnerVariables = (dynamic_cast<CreatureObject*>(&object) == NULL);
	IGNORE_RETURN(AssetCustomizationManager::addCustomizationVariablesForAsset(ConstCharCrcString(appearanceTemplateName), *customizationDataWithDefaults, skipSharedOwnerVariables));

#endif

	//-- add/create each declared variable.
	CustomizationIntList::iterator const endIt = m_customizationIntList->end ();
	for (CustomizationIntList::iterator it = m_customizationIntList->begin (); it != endIt; ++it)
	{
		CustomizationInt const *const customizationInt = *it;
		if (customizationInt == NULL)
		 continue;

		// get variable.
		CustomizationVariable *const baseVariable = customizationData->findVariable (customizationInt->m_variableName);
		RangedIntCustomizationVariable *variable = dynamic_cast<RangedIntCustomizationVariable*> (baseVariable);
		if (!variable)
		{
			if (!baseVariable)
			{
				DEBUG_REPORT_LOG (s_logUndeclaredCustomizations, ("ClientDataFile::apply (): client data file specifies customization int value for variable [%s] but object template [%s] does not declare that variable, creating new variable declaration now.\n", customizationInt->m_variableName.c_str (), object.getObjectTemplateName ()));

				// create the customization variable.  We do the best we can here to keep things running --- no idea what range should be.
				variable = new BasicRangedIntCustomizationVariable (std::min (0, customizationInt->m_value), customizationInt->m_value, customizationInt->m_value + 1);
				customizationData->addVariableTakeOwnership (customizationInt->m_variableName, variable);
			}
			else
			{
				DEBUG_WARNING (true, ("ClientDataFile::apply (): client data file specifies customization int value for variable [%s] but object template [%s] declares this as a non-int variable, cannot create/replace!", customizationInt->m_variableName.c_str (), object.getObjectTemplateName ()));
				continue;
			}
		}

#if CUSTOMIZATION_WARN_ON_OVERRIDE_NON_DEFAULT
		// get the default value for this customization.
		CustomizationVariable *const defaultBaseVariable = customizationDataWithDefaults->findVariable (customizationInt->m_variableName);
		RangedIntCustomizationVariable *defaultVariable = dynamic_cast<RangedIntCustomizationVariable*> (defaultBaseVariable);

		if (defaultVariable && variable)
		{
			bool const initialValueNotDefault = (variable->getValue () != defaultVariable->getValue ());
			DEBUG_WARNING (initialValueNotDefault, ("ClientDataFile CustomizationSetInt warning: cdf=[%s], object id=[%s], shared object template=[%s], variable=[%s]: overriding existing non-default value [%d] already set with value [%d]. This CustomizationSetInt directive is highly suspect and probably shouldn't be here.",
				getName(),
				object.getNetworkId().getValueString().c_str(),
				object.getObjectTemplateName(),
				customizationInt->m_variableName.c_str(),
				variable->getValue(),
				customizationInt->m_value));
		}
#endif

		// set value.
		NOT_NULL (variable);
		if (!variable->setValue (customizationInt->m_value))
		{
			WARNING(true, ("CDF %s unable to set int variable %s", getName(), customizationInt->m_variableName.c_str ()));
		}
	}

	//-- set the appearance's customization data.
	Appearance *const appearance = object.getAppearance ();
	if (appearance)
		appearance->setCustomizationData (customizationData);

	//-- release local references.
	customizationData->release();

#if CUSTOMIZATION_WARN_ON_OVERRIDE_NON_DEFAULT
	customizationDataWithDefaults->release();
#endif
} //lint !e429 // custodial pointer 'cdProperty' has not been freed or returned // it's okay, object owns it.

//----------------------------------------------------------------------

int ClientDataFile::findVehicleThrusterIndex (float damageLevel) const
{
	if (m_vehicleThrusterDataList)
	{
		int index = static_cast<int>(m_vehicleThrusterDataList->size ()) - 1;
		for (VehicleThrusterDataList::reverse_iterator it = m_vehicleThrusterDataList->rbegin (); it != m_vehicleThrusterDataList->rend (); ++it, --index)
		{
			const VehicleThrusterData * const vtd = *it;
			if (damageLevel >= vtd->m_damageLevelMin)
				return index;
		}
	}

	return -1;
}

//----------------------------------------------------------------------

bool ClientDataFile::updateVehicleThrusters (Object & obj, float oldDamageLevel, float newDamageLevel, ObjectVector & thrusters, VehicleThrusterSoundData & sounds) const
{
	const int oldIndex = findVehicleThrusterIndex (oldDamageLevel);
	const int newIndex = findVehicleThrusterIndex (newDamageLevel);

	if (oldIndex != newIndex)
		return updateVehicleThrusters (obj, newIndex, thrusters, sounds);

	return false;
}

//----------------------------------------------------------------------

bool ClientDataFile::updateVehicleThrusters (Object & obj, int index, ObjectVector & thrusters, VehicleThrusterSoundData & sounds) const
{
	if (index < 0 || !m_vehicleThrusterDataList || index > static_cast<int>(m_vehicleThrusterDataList->size ()))
		return false;
	
	{
		for (ObjectVector::iterator it = thrusters.begin (); it != thrusters.end (); ++it)
		{
			Object * const child = *it;
			child->kill ();
		}
	}
	
	thrusters.clear ();
	
	const VehicleThrusterData * const vtd = (*m_vehicleThrusterDataList) [static_cast<size_t>(index)];
	
	thrusters.reserve (vtd->m_hardpointEffects->size ());
	
	for (HardpointChildObjectList::const_iterator it = vtd->m_hardpointEffects->begin (); it != vtd->m_hardpointEffects->end (); ++it)
	{
		const HardpointChildObject * const hardpointChildObject = *it;
		
		HardpointObject* const objectToAdd = new HardpointObject (CrcLowerString (hardpointChildObject->m_hardpointName.c_str ()));
		objectToAdd->setDebugName("thruster");
		
		if (hardpointChildObject->m_appearanceTemplate)
		{
			Appearance * const thrusterAppearance = hardpointChildObject->m_appearanceTemplate->createAppearance ();
			thrusterAppearance->useRenderEffectsFlag(true);
			objectToAdd->setAppearance (thrusterAppearance);
		}
		
		RenderWorld::addObjectNotifications (*objectToAdd);
		
		if (!addHardpointObjectToFirstAppropriateParent(*objectToAdd, obj))
		{
			obj.addChildObject_o(objectToAdd);
		}
		
		thrusters.push_back (objectToAdd);
	}

	sounds = vtd->m_soundData;

	return true;
}

//----------------------------------------------------------------------

bool ClientDataFile::initVehicleThrusters (Object & obj, ObjectVector & thrusters, VehicleThrusterSoundData & sounds) const
{
	return updateVehicleThrusters (obj, 0, thrusters, sounds);
}

//----------------------------------------------------------------------

ClientDataFile::ContrailDataVector const * ClientDataFile::getContrailDataVector() const
{
	return m_contrailDataVector;
}

//----------------------------------------------------------------------

InterpolatedSound const * ClientDataFile::getInterpolatedSound() const
{
	return m_interpolatedSound;
}

//----------------------------------------------------------------------

bool ClientDataFile::processBreakpoints(Object const & object, float probabilityDelta, PlaneVector & planeVector) const
{
	if (NULL == m_breakpointVector)
		return false;

	return processBreakpoints(object, *m_breakpointVector, probabilityDelta, planeVector);
}

//----------------------------------------------------------------------


bool ClientDataFile::processBreakpoints(Object const & object, BreakpointVector const & breakpoints, float probabilityDelta, PlaneVector & planeVector) const
{
	if (breakpoints.empty())
		return false;

	planeVector.reserve(breakpoints.size());
	
	StringVector stringVector;
	stringVector.reserve(breakpoints.size());

	Appearance const * const appearance = object.getAppearance();

	for (BreakpointVector::const_iterator it = breakpoints.begin(); it != breakpoints.end(); ++it)
	{
		Breakpoint const & breakpoint = *it;

		//-- a previously processed breakpoint is required to process this one
		if (!breakpoint.m_dependency.empty())
		{
			if (!std::binary_search(stringVector.begin(), stringVector.end(), breakpoint.m_dependency))
				continue;
		}

		//-- a previously processed breakpoint excludes this one
		if (!breakpoint.m_exclusion.empty())
		{
			if (std::binary_search(stringVector.begin(), stringVector.end(), breakpoint.m_exclusion))
				continue;
		}

		if ((Random::randomReal(0.0f, 1.0f)) <= (breakpoint.m_probability + probabilityDelta))
		{
			stringVector.push_back(breakpoint.m_name);

			Transform transform;

			if (appearance != NULL && !breakpoint.m_hardpointName.empty())
			{
				if (!appearance->findHardpoint(TemporaryCrcString(breakpoint.m_hardpointName.c_str(), true), transform))
				{
					WARNING(true, ("ClientDataFile [%s] breakpoint hardpoint [%s] not found", getName(), breakpoint.m_hardpointName.c_str()));
				}
			}
			else
			{
				transform = Transform::identity;
				transform.move_p(breakpoint.m_point);
				
				//-- if the normal is not pointint toward +y or -y, use +y as the j frame
				if (fabsf(breakpoint.m_normal.dot(Vector::unitY)) < 0.9f)
					transform.setLocalFrameKJ_p(breakpoint.m_normal, Vector::unitY);
				else if (fabsf(breakpoint.m_normal.dot(Vector::unitZ)) < 0.9f)
					transform.setLocalFrameKJ_p(breakpoint.m_normal, Vector::unitZ);
				else
					transform.setLocalFrameKJ_p(breakpoint.m_normal, Vector::unitX);
			}

			Vector const pointDelta(Random::randomReal(-breakpoint.m_pointDelta.x, breakpoint.m_pointDelta.x),
				Random::randomReal(-breakpoint.m_pointDelta.x, breakpoint.m_pointDelta.y),
				Random::randomReal(-breakpoint.m_pointDelta.z, breakpoint.m_pointDelta.z));

			transform.move_l(pointDelta);

			Vector const normalDelta(Random::randomReal(-breakpoint.m_normalDelta.x, breakpoint.m_normalDelta.x),
				Random::randomReal(-breakpoint.m_normalDelta.y, breakpoint.m_normalDelta.y),
				Random::randomReal(-breakpoint.m_normalDelta.z, breakpoint.m_normalDelta.z));

			transform.yaw_l(normalDelta.x);
			transform.pitch_l(normalDelta.y);

			Plane const plane(transform.getLocalFrameK_p(), transform.getPosition_p());
			planeVector.push_back(plane);
		}
	}

	if (planeVector.empty() && probabilityDelta < 1.0f)
	{
		IGNORE_RETURN(processBreakpoints(object, probabilityDelta + 0.25f, planeVector));
	}

	return true;
}

//----------------------------------------------------------------------

void ClientDataFile::getAllChildObjects(StringVector & results) const
{	
	if (NULL != m_transformChildObjectList)
	{
		for (TransformChildObjectList::const_iterator it = m_transformChildObjectList->begin(); it != m_transformChildObjectList->end(); ++it)
		{
			TransformChildObject const * const tco = NON_NULL(*it);
			if (!tco->m_objectTemplateName.empty())
				results.push_back(tco->m_objectTemplateName);
		}
	}
}

//----------------------------------------------------------------------

void ClientDataFile::getAllChildAppearances(StringVector & results) const
{	
	if (NULL != m_transformChildObjectList)
	{
		for (TransformChildObjectList::const_iterator it = m_transformChildObjectList->begin(); it != m_transformChildObjectList->end(); ++it)
		{
			TransformChildObject const * const tco = NON_NULL(*it);
			if (!tco->m_appearanceTemplateName.empty())
				results.push_back(tco->m_appearanceTemplateName);
		}
	}

	if (NULL != m_hardpointChildObjectList)
	{
		for (HardpointChildObjectList::const_iterator it = m_hardpointChildObjectList->begin(); it != m_hardpointChildObjectList->end(); ++it)
		{
			HardpointChildObject const * const hco = NON_NULL(*it);
			if (!hco->m_appearanceTemplateName.empty())
				results.push_back(hco->m_appearanceTemplateName);
		}
	}
}

//----------------------------------------------------------------------

DestructionSequence const * ClientDataFile::getDestructionSequence() const
{
	return m_destructionSequence;
}

// ----------------------------------------------------------------------

void ClientDataFile::cacheClientEffectTemplate(char const * const fileName) const
{
	if (!m_cachedClientEffectTemplateMap)
		m_cachedClientEffectTemplateMap = new CachedClientEffectTemplateMap;

	TemporaryCrcString const crcFileName(fileName, true);
	if (m_cachedClientEffectTemplateMap->find(&crcFileName) == m_cachedClientEffectTemplateMap->end())
	{
		ClientEffectTemplate const * const clientEffectTemplate = ClientEffectTemplateList::fetch(CrcLowerString(fileName));

		//-- If the template doesn't exist, we store a newed CrcString.  Otherwise, the CrcString is in the template.
		m_cachedClientEffectTemplateMap->insert(std::make_pair(clientEffectTemplate ? &clientEffectTemplate->getCrcName() : new CrcLowerString(fileName), clientEffectTemplate));
	}
}

// ----------------------------------------------------------------------

void ClientDataFile::cacheSoundTemplate(char const * const fileName) const
{
	if (!m_cachedSoundTemplateMap)
		m_cachedSoundTemplateMap = new CachedSoundTemplateMap;

	TemporaryCrcString const crcFileName(fileName, true);
	if (m_cachedSoundTemplateMap->find(&crcFileName) == m_cachedSoundTemplateMap->end())
	{
		SoundTemplate const * const soundTemplate = SoundTemplateList::fetch(fileName);

		WARNING(!soundTemplate, ("ClientDataFile [%s] unable to load sound template [%s]", getName(), fileName));

		//-- If the template doesn't exist, we store a newed CrcString.  Otherwise, the CrcString is in the template.
		m_cachedSoundTemplateMap->insert(std::make_pair(soundTemplate ? &soundTemplate->getCrcName() : new CrcLowerString(fileName), soundTemplate));
	}
}

//----------------------------------------------------------------------

ClientDataFile::InterpolatedSoundVector const * ClientDataFile::getInterpolatedSoundVector() const
{
	return m_interpolatedSoundVector;
}

// ======================================================================

