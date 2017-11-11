//======================================================================
//
// CombatEffectManager.cpp
// Copyright 2004, 2005 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/CombatEffectsManager.h"

#include "clientGame/ClientEffectTemplate.h"
#include "clientGame/ClientEffectTemplateList.h"
#include "clientGame/ClientWeaponObjectTemplate.h"
#include "clientGame/ContainerInterface.h"

#include "clientGraphics/RenderWorld.h"

#include "clientParticle/ParticleEffectAppearance.h"
#include "clientParticle/LightningAppearance.h"

#include "clientAudio/Audio.h"

#include "sharedCollision/CollisionInfo.h"
#include "sharedCollision/CollideParameters.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedMath/Hsv.h"
#include "sharedMath/Transform.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/CustomizationDataProperty.h"
#include "sharedObject/PaletteColorCustomizationVariable.h"
#include "sharedObject/SlotIdManager.h"
#include "sharedObject/SlottedContainer.h"
#include "clientParticle/SwooshAppearance.h"
#include "clientParticle/SwooshAppearanceTemplate.h"
#include "clientParticle/ParticleEffectAppearanceTemplate.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/Object.h"
#include "sharedObject/MemoryBlockManagedObject.h"
#include "sharedCollision/BoxExtent.h"

#include "sharedUtility/DataTable.h"

#include <algorithm>

//======================================================================
//
// CombatEffectsManagerNamespace
//
//======================================================================

namespace CombatEffectsManagerNamespace
{
	bool s_installed = false;

	const ConstCharCrcLowerString   cs_genericWeaponHardpointName("weapon");
	const ConstCharCrcLowerString   cs_objectLocationName("loc");

	const ConstCharCrcLowerString   cs_defaultWeaponSlotName("default_weapon");
	SlotId                         s_defaultWeaponSlotId(SlotId::invalid);

	const ConstCharCrcLowerString  cs_holdRightSlotName("hold_r");
	SlotId                         s_holdRightSlotId(SlotId::invalid);

	void remove();

	struct AppearanceData
	{
		AppearanceData()
		: m_name()
		, m_template(0)
		, m_applyHue(false)
		{
		}

		void fetch()
		{
			if (!m_name.empty())
			{
				m_template = AppearanceTemplateList::fetch (m_name.c_str());
				DEBUG_WARNING(!m_template || strcmp(m_template->getName(), m_name.c_str()), ("CombatEffectsManager::install: Unable to create AppearanceTemplate (%s) for hit effect object", m_name.c_str()));
			}
		}

		void release()
		{
			if (m_template != 0) 
				AppearanceTemplateList::release(m_template);
		}

		bool isEmpty() const
		{
			return m_template == 0;
		}

		std::string                m_name;
		const AppearanceTemplate * m_template;
		bool                       m_applyHue;
	};

	struct SwooshAppearanceData
	{
		SwooshAppearanceData()
		: m_name()
		, m_template(0)
		, m_applyHue(false)
		{
		}

		void fetch()
		{
			if (!m_name.empty())
			{
				AppearanceTemplate const * temp = AppearanceTemplateList::fetch (m_name.c_str());

				if (temp)
				{
					m_template = dynamic_cast<SwooshAppearanceTemplate const *>(temp);

					if (m_template == 0)
					{
						DEBUG_WARNING(true, ("CombatEffectsManager::install: Unable to create swooshAppearanceTemplate (%s) for hit effect object", m_name.c_str()));
						AppearanceTemplateList::release(temp);
					}
				}
			}
		}

		void release()
		{
			if (m_template != 0) 
				AppearanceTemplateList::release(m_template);
		}

		bool isEmpty() const
		{
			return m_template == 0;
		}

		std::string                      m_name;
		const SwooshAppearanceTemplate * m_template;
		bool                             m_applyHue;
	};

	struct SoundData
	{
		SoundData()
		: m_name()
		, m_pitch(0.0f)
		{
		}

		bool isEmpty() const
		{
			return m_name.empty();
		}

		std::string m_name;
		float m_pitch;
	};

	//
	// combined appearance & sound EffectPair
	//
	struct EffectPair
	{
		EffectPair()
			: m_appearance()
			, m_sound()
		{
		}

		void fetch()
		{
			m_appearance.fetch();
		}

		void release()
		{
			m_appearance.release();
		}

		bool isEmpty() const
		{
			return m_appearance.isEmpty() && m_sound.isEmpty();
		}

		AppearanceData m_appearance;
		SoundData m_sound;
	};


	//
	// Special Attack Data
	//
	struct SpecialAttackEffectData
	{
		SpecialAttackEffectData()
		 : m_trailAppearance()
		{
		}

		void fetch()
		{
			m_trailAppearance.fetch();
			m_attackEffect.fetch();
			m_hitEffect.fetch();
		}

		void release()
		{
			m_trailAppearance.release();
			m_attackEffect.release();
			m_hitEffect.release();
		}

		SwooshAppearanceData m_trailAppearance;

		EffectPair           m_attackEffect;
		std::string          m_attackEffectHardpointName;
		EffectPair           m_hitEffect;
	};

	std::map<uint32, SpecialAttackEffectData> s_idToSpecialAttackEffectDataMap;
	typedef std::map<uint32, SpecialAttackEffectData>::iterator SpecialAttackTableItr;
	const SpecialAttackEffectData s_unknownSpecialAttackEffect;

	enum CombatEffectsSpecialAttackColumns
	{
		CESAC_attack = 0,             // NOTE: these must match the order of the data table
		CESAC_attackName,             // because these are used to index the table
		CESAC_trailAppearance,
		CESAC_trailAppearanceApplyHue,
		CESAC_attackAppearance,
		CESAC_attackAppearanceHardpointName,
		CESAC_attackSound,
		CESAC_attackSoundPitch,
		CESAC_hitAppearance,
		CESAC_hitSound,
		CESAC_hitSoundPitch

	};

	//
	// Melee Weapon Data
	//
	struct MeleeWeaponEffectData
	{
		MeleeWeaponEffectData()
		 : m_swooshAppearance()
		 , m_attackSound()
		 , m_useWeaponHue(false)
		{
		}


		void fetch()
		{
			m_hitEffect.fetch();
			m_swooshAppearance.fetch();
		}

		void release()
		{
			m_hitEffect.release();
			m_swooshAppearance.release();
		}

		EffectPair     m_hitEffect;
		SwooshAppearanceData m_swooshAppearance;
		SoundData      m_attackSound;
		bool           m_useWeaponHue;
	};

	std::map<std::string, MeleeWeaponEffectData> s_idToMeleeEffectDataMap;
	typedef std::map<std::string, MeleeWeaponEffectData>::iterator MeleeTableItr;
	const MeleeWeaponEffectData s_unknownMeleeWeaponEffect;

	enum CombatEffectsMeleeColumns
	{
		CEMC_weapon = 0,            // NOTE: these must match the order of the data table
		CEMC_swooshAppearance,     // because these are used to index the table
		CEMC_swooshAppearanceApplyHue,
		CEMC_attackSound,
		CEMC_attackSoundPitch,
		CEMC_hitAppearance,
		CEMC_hitAppearanceApplyHue,
		CEMC_hitSound,
		CEMC_hitSoundPitch,
		CEMC_useWeaponHue
	};

	//
	// Ranged Weapon Data
	//
	struct RangedWeaponEffectData
	{
		RangedWeaponEffectData()
			: m_hitEffectScale(1.f)
			, m_addDamageBaseHit(false)
			, m_addDamageElementalHit(false)
		{
		}
	
		void fetch()
		{
			m_muzzleEffect.fetch();
			m_hitTargetEffect.fetch();
			m_hitNothingEffect.fetch();
			m_hitRicochetEffect.fetch();
			m_boltAppearance.fetch();
		}

		void release()
		{
			m_muzzleEffect.release();
			m_hitTargetEffect.release();
			m_hitNothingEffect.release();
			m_hitRicochetEffect.release();
			m_boltAppearance.release();
		}

		EffectPair     m_muzzleEffect;
		EffectPair     m_hitTargetEffect;
		EffectPair     m_hitNothingEffect;
		EffectPair     m_hitRicochetEffect;
		AppearanceData m_boltAppearance;
		float          m_hitEffectScale;
		bool           m_addDamageBaseHit;
		bool           m_addDamageElementalHit;
	};
	
	std::map<std::string, RangedWeaponEffectData> s_idToRangedEffectDataMap;
	typedef std::map<std::string, RangedWeaponEffectData>::iterator RangedTableItr;
	const RangedWeaponEffectData s_unknownRangedWeaponEffect;

	enum CombatEffectsRangedColumns
	{
		CERC_weapon = 0,            // NOTE: these must match the order of the data table
		CERC_muzzleAppearance,     // because these are used to index the table
		CERC_muzzleApplyHue,
		CERC_muzzleSound,
		CERC_muzzleSoundPitch,
		CERC_boltAppearance,
		CERC_boltApplyHue,
		CERC_hitTargetAppearance,	  
		CERC_hitTargetApplyHue,
		CERC_hitNothingAppearance,
		CERC_hitNothingApplyHue,
		CERC_hitRicochetAppearance,
		CERC_hitRicochetApplyHue,
		CERC_hitTargetSound,
		CERC_hitTargetSoundPitch,
		CERC_hitNothingSound,
		CERC_hitNothingSoundPitch,
		CERC_hitRicochetSound,
		CERC_hitRicochetSoundPitch,
		CERC_hitEffectScale,
		CERC_hitAddDamageBaseHit,
		CERC_hitAddDamageElementalHit
	};

	//
	// Damage Type Data
	//
	struct DamageTypeEffectData
	{
		DamageTypeEffectData()
			: m_color()
		{
		}

		void fetch()
		{
			m_hitTargetEffect.fetch();
			m_hitNothingEffect.fetch();
			m_hitRicochetEffect.fetch();
		}

		void release()
		{
			m_hitTargetEffect.release();
			m_hitNothingEffect.release();
			m_hitRicochetEffect.release();
		}

		VectorArgb m_color;
		EffectPair m_hitTargetEffect;
		EffectPair m_hitNothingEffect;
		EffectPair m_hitRicochetEffect;

		SoundData  m_flyBySound;
	};

	std::map<int, DamageTypeEffectData> s_damageTypeToEffectDataMap;
	typedef std::map<int, DamageTypeEffectData>::iterator DamageTypeTableItr;
	const DamageTypeEffectData s_unknownDamageTypeEffect;

	enum CombatEffectsDamageColumns
	{
		CEDC_type = 0,             // NOTE: these must match the order of the data table
		CEDC_bitValue,
		CEDC_red,
		CEDC_green,
		CEDC_blue,
		CEDC_hitTargetAppearance,	  
		CEDC_hitTargetApplyHue,
		CEDC_hitNothingAppearance,
		CEDC_hitNothingApplyHue,
		CEDC_hitRicochetAppearance,
		CEDC_hitRicochetApplyHue,
		CEDC_hitSound,
		CEDC_hitSoundPitch,
		CEDC_flyBySound,
		CEDC_flyBySoundPitch
	};

	enum HitEffectHitType
	{
		HEHT_target = 0,
		HEHT_ricochet,
		HEHT_terrain,
		HEHT_nothing
	};

	VectorArgb getObjectColor(Object const &object);
	Object* const createParticleObject(AppearanceData const & effectAppearanceData, const VectorArgb & damageTypeColor, float scale, CellProperty * cell);
	void generateHitEffect(const RangedWeaponEffectData & effectData, HitEffectHitType const & hitType, CellProperty & cell, Vector const & position_l, Vector const & normal_l, WeaponObject::DamageType baseDamageType, WeaponObject::DamageType elementalDamageType, int actionNameCrc );
	void removePathFromFilename(const std::string &fullPath, std::string & nameOnly);
	void applyEffectPairAtLocation(EffectPair const * effect, CellProperty & cell, Vector const & position_l, Vector const & normal_l, VectorArgb const & damageColor, float effectScale );
	void applyEffectPairAtHardpoint(EffectPair const * effect, Object * hardpointObject, Transform const & hardpointTransform, CrcLowerString const & hardpointName, VectorArgb const & damageColor, float effectScale );
	MeleeWeaponEffectData const & getMeleeWeaponEffect(WeaponObject const & weaponObject);
	RangedWeaponEffectData const & getRangedWeaponEffect(std::string const & weaponTemplateName, WeaponObject::WeaponType weaponType );
	RangedWeaponEffectData const & getRangedWeaponEffect(WeaponObject const & weaponObject);
	DamageTypeEffectData const & getDamageTypeEffect( int damageType );
	SpecialAttackEffectData const & getSpecialAttackEffect( int damageType );

	WeaponObject *getDefaultWeaponObject(Object *holder);
	WeaponObject *getWeaponObject(Object *holder);


	enum CombatEffectsHitLocation // This is synced with base_class.java, search for HIT_LOCATION_
	{
		HIT_LOCATION_BODY  = 0,
		HIT_LOCATION_HEAD  = 1,
		HIT_LOCATION_R_ARM = 2,
		HIT_LOCATION_L_ARM = 3,
		HIT_LOCATION_R_LEG = 4,
		HIT_LOCATION_L_LEG = 5,
		HIT_LOCATION_NUM_LOCATIONS
	};

	enum CombatEffectsHitLocationColumns
	{
		CEHLC_location = 0,    // NOTE: these must match the order of the data table
		CEHLC_hardpoint        // because these are used to index the table
	};

	typedef std::vector<CrcLowerString *> CrcLowerStringList;
	std::map<int, CrcLowerStringList> s_hitLocationToHardpointMap; 

	struct SwooshHueRange
	{
		SwooshHueRange(float const min, float const max, float const red, float const green, float const blue)
		 : m_min(min)
		 , m_max(max)
		 , m_red(red)
		 , m_green(green)
		 , m_blue(blue)
		{
		}

		float m_min;
		float m_max;
		float m_red;
		float m_green;
		float m_blue;

	private:

		// Disabled
		SwooshHueRange();
	};

	typedef std::vector<SwooshHueRange>               HueRangeList;

	HueRangeList s_hueRangeList;

	std::string const s_bladeColorVariableName("/private/index_color_blade");
}

using namespace CombatEffectsManagerNamespace;

//-------------------------------------------------------------------
void CombatEffectsManagerNamespace::remove()
{
	DEBUG_FATAL(!s_installed, ("Not installed"));

	std::map<int, CrcLowerStringList>::iterator locationMapItr = s_hitLocationToHardpointMap.begin();
	while (locationMapItr != s_hitLocationToHardpointMap.end())
	{
		CrcLowerStringList & hardpointList = (*locationMapItr).second;

		std::for_each(hardpointList.begin(), hardpointList.end(), PointerDeleter());

		++locationMapItr;
	}
	s_hitLocationToHardpointMap.clear();

	{
		DamageTypeTableItr itr;
		for (itr = s_damageTypeToEffectDataMap.begin(); itr != s_damageTypeToEffectDataMap.end(); ++itr)
			itr->second.release();
		s_damageTypeToEffectDataMap.clear();
	}

	{
		RangedTableItr itr;
		for (itr = s_idToRangedEffectDataMap.begin(); itr != s_idToRangedEffectDataMap.end(); ++itr)
			itr->second.release();
		s_idToRangedEffectDataMap.clear();
	}
	{
		MeleeTableItr itr;
		for (itr = s_idToMeleeEffectDataMap.begin(); itr != s_idToMeleeEffectDataMap.end(); ++itr)
			itr->second.release();
		s_idToMeleeEffectDataMap.clear();
	}
	{
		SpecialAttackTableItr itr;
		for (itr = s_idToSpecialAttackEffectDataMap.begin(); itr != s_idToSpecialAttackEffectDataMap.end(); ++itr)
			itr->second.release();
		s_idToSpecialAttackEffectDataMap.clear();
	}
}

//-----------------------------------------------------------------------------

VectorArgb CombatEffectsManagerNamespace::getObjectColor(Object const &object)
{
	VectorArgb color(VectorArgb::solidWhite);

	const CustomizationDataProperty *const property = dynamic_cast<const CustomizationDataProperty *const>(object.getProperty(CustomizationDataProperty::getClassPropertyId()));

	if (property != NULL)
	{
		const CustomizationData *const customizationData = property->fetchCustomizationData();
		NOT_NULL(customizationData);

		PaletteColorCustomizationVariable const *const variable = dynamic_cast<PaletteColorCustomizationVariable const*>(customizationData->findConstVariable(s_bladeColorVariableName));
		if (variable != NULL)
		{
			color = VectorArgb(variable->getValueAsColor());
		}

		customizationData->release();

		Vector hsv;

		Hsv::rgbToHsv(color, hsv);

		// Use the hue value to determine the final swoosh color

		HueRangeList::const_iterator iterHueRangeList = s_hueRangeList.begin();

		for (; iterHueRangeList != s_hueRangeList.end(); ++iterHueRangeList)
		{
			float const hueMin = iterHueRangeList->m_min;
			float const hueMax = iterHueRangeList->m_max;

			if (   (hsv.x >= hueMin)
				&& (hsv.x <= hueMax))
			{
				color.r = iterHueRangeList->m_red;
				color.g = iterHueRangeList->m_green;
				color.b = iterHueRangeList->m_blue;
				break;
			}
		}

	}

	return color;
}

// ----------------------------------------------------------------------

WeaponObject *CombatEffectsManagerNamespace::getDefaultWeaponObject(Object *holder)
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

	Object* objectInSlot = container->getObjectInSlot(s_defaultWeaponSlotId, tmp).getObject();

	if ( objectInSlot && objectInSlot->asClientObject() )
	{
		return objectInSlot->asClientObject()->asWeaponObject();
	}

	return 0;
}

// ----------------------------------------------------------------------

WeaponObject *CombatEffectsManagerNamespace::getWeaponObject(Object *holder)
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
	if ( holdRightObject && holdRightObject->asClientObject() )
	{
		return holdRightObject->asClientObject()->asWeaponObject();
	}

	// No object in the right hand.  Return the default weapon.
	return getDefaultWeaponObject(holder);
}


//-------------------------------------------------------------------
Object* const CombatEffectsManagerNamespace::createParticleObject(AppearanceData const & effectAppearanceData, const VectorArgb & damageTypeColor, float scale, CellProperty * cell)
{
	DEBUG_FATAL(!s_installed, ("Not installed"));

	Object* object = NULL;

	if (effectAppearanceData.m_template)
	{
		Appearance * effectAppearance = effectAppearanceData.m_template->createAppearance();

		if (!effectAppearance)
		{
			DEBUG_WARNING(true, ("createParticleObject: effectAppearance is NULL, using default appearance"));
			effectAppearance = AppearanceTemplateList::createAppearance(AppearanceTemplateList::getDefaultAppearanceTemplateName());
		}

		//-- create the object
		object = new MemoryBlockManagedObject();
		object->setAppearance(effectAppearance);

		if (cell)
		{
			//this MUST MUST MUST MUST MUST MUST be reset to true before returning!!!
			CellProperty::setPortalTransitionsEnabled(false);
			object->setParentCell(cell);
		}

		RenderWorld::addObjectNotifications(*object);
		object->addNotification(ClientWorld::getIntangibleNotification());
		object->addToWorld();

		//-- scale the appearance
		if (scale != 1.f)
		{
			effectAppearance->setScale(Vector(scale, scale, scale));
		}

		ParticleEffectAppearance* const particleEffectAppearance = effectAppearance->asParticleEffectAppearance();
		if (particleEffectAppearance)
		{
			//-- change the color
			if (effectAppearanceData.m_applyHue)
			{
				particleEffectAppearance->setColorModifier(damageTypeColor);
			}
		}
		else
		{
			DEBUG_WARNING(true, ("createParticleObject: appearance (%s) is not a particleEffectAppearance", effectAppearance->getAppearanceTemplateName()));
		}

		if (cell)
		{
			CellProperty::setPortalTransitionsEnabled(true);
		}
	}

	return object;
}

//-------------------------------------------------------------------

void CombatEffectsManagerNamespace::applyEffectPairAtLocation(  EffectPair const * effect
															  , CellProperty & cell
															  , Vector const & position_l
															  , Vector const & normal_l
															  , VectorArgb const & damageColor
															  , float effectScale )
{
	DEBUG_FATAL(!s_installed, ("Not installed"));

	if (effect != 0)
	{
		if (!effect->m_appearance.m_name.empty())
		{
			Object* const object = createParticleObject(effect->m_appearance, damageColor, effectScale, &cell);
			if (object)
			{
				object->setPosition_w(position_l);
				object->lookAt_o(normal_l);
			}
		}

		if (!effect->m_sound.m_name.empty())
		{
			SoundId soundId = Audio::playSound(effect->m_sound.m_name.c_str(), position_l, &cell);
			Audio::setSoundPitchDelta(soundId, effect->m_sound.m_pitch);
		}
	}


}

//-------------------------------------------------------------------

void CombatEffectsManagerNamespace::applyEffectPairAtHardpoint(  EffectPair const * effect
															   , Object * hardpointObject
															   , Transform const & hardpointTransform
															   , CrcLowerString const & hardpointName
															   , VectorArgb const & damageColor
															   , float effectScale )
{
	DEBUG_FATAL(!s_installed, ("Not installed"));

	if (effect != 0)
	{
		Object* const object = createParticleObject(effect->m_appearance, damageColor, effectScale, NULL);

		if (object && hardpointObject)
		{
			object->attachToObject_p(hardpointObject, true);
			object->setTransform_o2p(hardpointTransform);
		}

		if (!effect->m_sound.m_name.empty())
		{
			SoundId soundId = Audio::attachSound(effect->m_sound.m_name.c_str(), hardpointObject, hardpointName.getString());
			Audio::setSoundPitchDelta(soundId, effect->m_sound.m_pitch);
		}
	}
}

//-------------------------------------------------------------------

MeleeWeaponEffectData const & CombatEffectsManagerNamespace::getMeleeWeaponEffect(WeaponObject const & weaponObject)
{
	WeaponObject::WeaponType const weaponType = weaponObject.getWeaponType();

	std::string weaponMapKey;
	removePathFromFilename(weaponObject.getObjectTemplateName(), weaponMapKey);

	MeleeTableItr meleeItr = s_idToMeleeEffectDataMap.find(weaponMapKey);
	if (meleeItr == s_idToMeleeEffectDataMap.end())
	{
		// lookup weapon by type
		weaponMapKey = WeaponObject::getWeaponTypeString(weaponType);
		meleeItr = s_idToMeleeEffectDataMap.find(weaponMapKey);

		if (meleeItr == s_idToMeleeEffectDataMap.end())
		{
			DEBUG_WARNING(true, ("CombatEffectsManagerNamespace::getMeleeWeaponEffect: invalid weapon specified (%s) [type (%s)]", weaponObject.getObjectTemplateName(), weaponMapKey.c_str()));
			return s_unknownMeleeWeaponEffect;
		}
	}

	return meleeItr->second;
}

//-------------------------------------------------------------------

RangedWeaponEffectData const & CombatEffectsManagerNamespace::getRangedWeaponEffect(std::string const & weaponTemplateName, WeaponObject::WeaponType weaponType )
{
	// lookup weapon by template name first
	std::string weaponMapKey;
	removePathFromFilename(weaponTemplateName, weaponMapKey);

	RangedTableItr rangedItr = s_idToRangedEffectDataMap.find(weaponMapKey);
	if (rangedItr == s_idToRangedEffectDataMap.end())
	{
		// lookup weapon by type
		weaponMapKey = WeaponObject::getWeaponTypeString(weaponType);
		rangedItr = s_idToRangedEffectDataMap.find(weaponMapKey);
		if (rangedItr == s_idToRangedEffectDataMap.end())
		{
			DEBUG_WARNING(true, ("getRangedWeaponEffect: invalid weapon specified (%s) [type (%s)]", weaponTemplateName, weaponMapKey.c_str()));
			return s_unknownRangedWeaponEffect;
		}
	}

	// Get the appearance from the weapon type
	return rangedItr->second;
}

RangedWeaponEffectData const & CombatEffectsManagerNamespace::getRangedWeaponEffect(WeaponObject const & weaponObject)
{
	return getRangedWeaponEffect(weaponObject.getObjectTemplateName(), weaponObject.getWeaponType());
}
	
//-------------------------------------------------------------------

DamageTypeEffectData const & CombatEffectsManagerNamespace::getDamageTypeEffect( int damageType )
{
	DamageTypeTableItr damageItr = s_damageTypeToEffectDataMap.find(damageType);

	if (damageItr == s_damageTypeToEffectDataMap.end())
	{
		DEBUG_WARNING(true, ("getDamageTypeEffect: invalid damage type specified (%d) for hit effect object", damageType));
		return s_unknownDamageTypeEffect;
	}

	return damageItr->second;
}


//-------------------------------------------------------------------

SpecialAttackEffectData const & CombatEffectsManagerNamespace::getSpecialAttackEffect(int attackCrc)
{
	if (attackCrc != 0)
	{
		SpecialAttackTableItr specialAttackItr = s_idToSpecialAttackEffectDataMap.find(attackCrc);

		if (specialAttackItr != s_idToSpecialAttackEffectDataMap.end())
		{
			return (*specialAttackItr).second;
		}
	}

	return s_unknownSpecialAttackEffect;

}


//-------------------------------------------------------------------
void CombatEffectsManagerNamespace::generateHitEffect(const RangedWeaponEffectData & effectData, HitEffectHitType const & hitType, CellProperty & cell, Vector const & position_l, Vector const & normal_l, WeaponObject::DamageType baseDamageType, WeaponObject::DamageType elementalDamageType, int actionNameCrc)
{
	DEBUG_FATAL(!s_installed, ("Not installed"));

	VectorArgb damageColor = CombatEffectsManager::getWeaponDamageTypeColor(baseDamageType, elementalDamageType);

	// pick the hit effect based on the hit type
	EffectPair const * effectPair = 0;

	switch (hitType)
	{
		case HEHT_target:
			effectPair = &effectData.m_hitTargetEffect;
			break;
		case HEHT_ricochet:
			effectPair = &effectData.m_hitRicochetEffect;
			break;
		case HEHT_terrain:
		case HEHT_nothing:
			effectPair = &effectData.m_hitNothingEffect;
			break;
		default:
			DEBUG_WARNING(true, ("generateHitEffect: invalid hit type specified (%d) for hit effect object", hitType));
			break;
	}

	applyEffectPairAtLocation(effectPair, cell, position_l, normal_l, damageColor, effectData.m_hitEffectScale);

	// now add the base damage type effect if specified
	if (effectData.m_addDamageBaseHit)
	{
		effectPair = 0;

		DamageTypeTableItr damageItr = s_damageTypeToEffectDataMap.find(baseDamageType);
		if (damageItr == s_damageTypeToEffectDataMap.end())
		{
			DEBUG_WARNING(true, ("generateHitEffect: invalid damage type specified (%d) for hit effect object", baseDamageType));
		}
		else
		{
			// Modify the effects based on the damage type
			DamageTypeEffectData damageTypeData = (*damageItr).second;			
			
			switch (hitType)
			{
				case HEHT_target:
					effectPair = &damageTypeData.m_hitTargetEffect;
					break;
				case HEHT_ricochet:
					effectPair = &damageTypeData.m_hitRicochetEffect;
					break;
				case HEHT_terrain:
				case HEHT_nothing:
					effectPair = &damageTypeData.m_hitNothingEffect;
					break;
				default:
					DEBUG_WARNING(true, ("generateHitEffect: invalid hit type specified (%d) for hit effect object", hitType));
					break;
			}

			applyEffectPairAtLocation( effectPair, cell, position_l, normal_l, damageColor, effectData.m_hitEffectScale );
		}
	}

	// now add the elemental damage type effect if specified
	if (effectData.m_addDamageElementalHit && elementalDamageType != WeaponObject::DT_none)
	{
		effectPair = 0;

		DamageTypeTableItr damageItr = s_damageTypeToEffectDataMap.find(elementalDamageType);
		if (damageItr == s_damageTypeToEffectDataMap.end())
		{
			DEBUG_WARNING(true, ("generateHitEffect: invalid damage type specified (%d) for hit effect object", elementalDamageType));
		}
		else
		{
			// Modify the effects based on the damage type
			DamageTypeEffectData damageTypeData = (*damageItr).second;			
			
			switch (hitType)
			{
				case HEHT_target:
					effectPair = &damageTypeData.m_hitTargetEffect;
					break;
				case HEHT_ricochet:
					effectPair = &damageTypeData.m_hitRicochetEffect;
					break;
				case HEHT_terrain:
				case HEHT_nothing:
					effectPair = &damageTypeData.m_hitNothingEffect;
					break;
				default:
					DEBUG_WARNING(true, ("generateHitEffect: invalid hit type specified (%d) for hit effect object", hitType));
					break;
			}

			applyEffectPairAtLocation( effectPair, cell, position_l, normal_l, damageColor, effectData.m_hitEffectScale );
		}
	}

	// now add the special attack effect, if specified
	if (actionNameCrc != 0)
	{
		SpecialAttackEffectData const & effectData = getSpecialAttackEffect( actionNameCrc );

		applyEffectPairAtLocation( &effectData.m_hitEffect, cell, position_l, normal_l, damageColor, 1.f );
	}
}

//-------------------------------------------------------------------
void CombatEffectsManagerNamespace::removePathFromFilename(const std::string &fullPath, std::string & nameOnly)
{
	if (fullPath.rfind("/") != std::string::npos)
	{
		IGNORE_RETURN(nameOnly.assign(fullPath.begin() + (fullPath.rfind("/") + 1), fullPath.end()));
	}
	else if (fullPath.rfind("\\") != std::string::npos)
	{
		IGNORE_RETURN(nameOnly.assign(fullPath.begin() + (fullPath.rfind("\\") + 1), fullPath.end()));
	}
	else
	{
		//-- There is no separator character, copy the whole thing.
		nameOnly = fullPath;
	}
}


//======================================================================
//
// CombatEffectsManager
//
//======================================================================

//-------------------------------------------------------------------

void CombatEffectsManager::install()
{
	DEBUG_FATAL(s_installed, ("Already installed"));

	InstallTimer const installTimer("CombatEffectsManager::install");

	s_defaultWeaponSlotId = SlotIdManager::findSlotId(cs_defaultWeaponSlotName);
	s_holdRightSlotId     = SlotIdManager::findSlotId(cs_holdRightSlotName);


	Iff dataTableIff;

	//-- load the special attack data
	if (dataTableIff.open("datatables/weapon/combat_effects_special_attacks.iff", true))
	{
		DataTable specialAttackDataTable;

		specialAttackDataTable.load(dataTableIff);

		//-- Store all of the info from the data table

		int const rowCount = specialAttackDataTable.getNumRows();
		for (int row = 0; row < rowCount; ++row)
		{
			SpecialAttackEffectData currentData;

			int specialAttackCrc = specialAttackDataTable.getIntValue(CESAC_attack, row);

			// we only want to continue if this effect does not exist
			SpecialAttackTableItr itr =  s_idToSpecialAttackEffectDataMap.find(specialAttackCrc);
			if (itr == s_idToSpecialAttackEffectDataMap.end())
			{
				currentData.m_trailAppearance.m_name = specialAttackDataTable.getStringValue(CESAC_trailAppearance, row);
				currentData.m_trailAppearance.m_applyHue = specialAttackDataTable.getIntValue(CESAC_trailAppearanceApplyHue, row);
				currentData.m_attackEffect.m_appearance.m_name = specialAttackDataTable.getStringValue(CESAC_attackAppearance, row);
				currentData.m_attackEffectHardpointName = specialAttackDataTable.getStringValue(CESAC_attackAppearanceHardpointName, row);
				currentData.m_attackEffect.m_sound.m_name = specialAttackDataTable.getStringValue(CESAC_attackSound, row);
				currentData.m_attackEffect.m_sound.m_pitch = specialAttackDataTable.getFloatValue(CESAC_attackSoundPitch, row);

				currentData.m_hitEffect.m_appearance.m_name = specialAttackDataTable.getStringValue(CESAC_hitAppearance, row);
				currentData.m_hitEffect.m_sound.m_name = specialAttackDataTable.getStringValue(CESAC_hitSound, row);
				currentData.m_hitEffect.m_sound.m_pitch = specialAttackDataTable.getFloatValue(CESAC_hitSoundPitch, row);

				// preload the appearances
				currentData.fetch();

				IGNORE_RETURN(s_idToSpecialAttackEffectDataMap.insert(std::make_pair(specialAttackCrc, currentData)));
			}
			else
			{
				DEBUG_WARNING(true, ("CombatEffectsManager::install not creating special attack effect, it already exists: %s (%d)", specialAttackDataTable.getStringValue(CESAC_attackName, row), specialAttackCrc));
			}
 		}

		dataTableIff.close();
	}

	//-- load the ranged weapon data
	if (dataTableIff.open("datatables/weapon/combat_effects_melee.iff", true))
	{
		DataTable meleeDataTable;

		meleeDataTable.load(dataTableIff);

		//-- Store all of the info from the data table

		int const rowCount = meleeDataTable.getNumRows();
		for (int row = 0; row < rowCount; ++row)
		{
			MeleeWeaponEffectData currentWeaponData;

			std::string const &weaponEffect = meleeDataTable.getStringValue(CEMC_weapon, row);

			// we only want to continue if this effect does not exist
			MeleeTableItr itr = s_idToMeleeEffectDataMap.find(weaponEffect);
			if (itr == s_idToMeleeEffectDataMap.end())
			{
				currentWeaponData.m_swooshAppearance.m_name = meleeDataTable.getStringValue(CEMC_swooshAppearance, row);
				currentWeaponData.m_swooshAppearance.m_applyHue = meleeDataTable.getIntValue(CEMC_swooshAppearanceApplyHue, row);
				currentWeaponData.m_attackSound.m_name = meleeDataTable.getStringValue(CEMC_attackSound, row);
				currentWeaponData.m_attackSound.m_pitch = meleeDataTable.getFloatValue(CEMC_attackSoundPitch, row);
				currentWeaponData.m_hitEffect.m_appearance.m_name = meleeDataTable.getStringValue(CEMC_hitAppearance, row);
				currentWeaponData.m_hitEffect.m_appearance.m_applyHue = meleeDataTable.getIntValue(CEMC_hitAppearanceApplyHue, row);
				currentWeaponData.m_hitEffect.m_sound.m_name = meleeDataTable.getStringValue(CEMC_hitSound, row);
				currentWeaponData.m_hitEffect.m_sound.m_pitch = meleeDataTable.getFloatValue(CEMC_hitSoundPitch, row);
				currentWeaponData.m_useWeaponHue= meleeDataTable.getIntValue(CEMC_useWeaponHue, row);

				// preload the appearances
				currentWeaponData.fetch();

				IGNORE_RETURN(s_idToMeleeEffectDataMap.insert(std::make_pair(weaponEffect, currentWeaponData)));
			}
			else
			{
				DEBUG_WARNING(true, ("CombatEffectsManager::install not creating melee weapon effect, it already exists: %s", weaponEffect.c_str()));
			}
 		}

		dataTableIff.close();
	}

	//-- load the ranged weapon data
	if (dataTableIff.open("datatables/weapon/combat_effects_ranged.iff", true))
	{
		DataTable weaponRangedData;

		weaponRangedData.load(dataTableIff);

		//-- Store all of the info from the data table

		int const rowCount = weaponRangedData.getNumRows();
		for (int row = 0; row < rowCount; ++row)
		{
			RangedWeaponEffectData currentWeaponData;

			std::string const &weaponEffect = weaponRangedData.getStringValue(CERC_weapon, row);

			// we only want to continue if this effect does not exist
			RangedTableItr itr = s_idToRangedEffectDataMap.find(weaponEffect);
			if (itr == s_idToRangedEffectDataMap.end())
			{
				currentWeaponData.m_muzzleEffect.m_appearance.m_name = weaponRangedData.getStringValue(CERC_muzzleAppearance, row);
				currentWeaponData.m_muzzleEffect.m_appearance.m_applyHue = weaponRangedData.getIntValue(CERC_muzzleApplyHue, row);
				currentWeaponData.m_muzzleEffect.m_sound.m_name = weaponRangedData.getStringValue(CERC_muzzleSound, row);
				currentWeaponData.m_muzzleEffect.m_sound.m_pitch = weaponRangedData.getFloatValue(CERC_muzzleSoundPitch, row);

				currentWeaponData.m_boltAppearance.m_name = weaponRangedData.getStringValue(CERC_boltAppearance, row);
				currentWeaponData.m_boltAppearance.m_applyHue = weaponRangedData.getIntValue(CERC_boltApplyHue, row);

				currentWeaponData.m_hitTargetEffect.m_appearance.m_name = weaponRangedData.getStringValue(CERC_hitTargetAppearance, row);
				currentWeaponData.m_hitTargetEffect.m_appearance.m_applyHue = weaponRangedData.getIntValue(CERC_hitTargetApplyHue, row);
				currentWeaponData.m_hitNothingEffect.m_appearance.m_name = weaponRangedData.getStringValue(CERC_hitNothingAppearance, row);
				currentWeaponData.m_hitNothingEffect.m_appearance.m_applyHue = weaponRangedData.getIntValue(CERC_hitNothingApplyHue, row);
				currentWeaponData.m_hitRicochetEffect.m_appearance.m_name = weaponRangedData.getStringValue(CERC_hitRicochetAppearance, row);
				currentWeaponData.m_hitRicochetEffect.m_appearance.m_applyHue = weaponRangedData.getIntValue(CERC_hitRicochetApplyHue, row);

				currentWeaponData.m_hitTargetEffect.m_sound.m_name = weaponRangedData.getStringValue(CERC_hitTargetSound, row);
				currentWeaponData.m_hitTargetEffect.m_sound.m_pitch = weaponRangedData.getFloatValue(CERC_hitTargetSoundPitch, row);
				currentWeaponData.m_hitNothingEffect.m_sound.m_name = weaponRangedData.getStringValue(CERC_hitNothingSound, row);
				currentWeaponData.m_hitNothingEffect.m_sound.m_pitch = weaponRangedData.getFloatValue(CERC_hitNothingSoundPitch, row);
				currentWeaponData.m_hitRicochetEffect.m_sound.m_name = weaponRangedData.getStringValue(CERC_hitRicochetSound, row);
				currentWeaponData.m_hitRicochetEffect.m_sound.m_pitch = weaponRangedData.getFloatValue(CERC_hitRicochetSoundPitch, row);

				currentWeaponData.m_hitEffectScale = weaponRangedData.getFloatValue(CERC_hitEffectScale, row);

				currentWeaponData.m_addDamageBaseHit = weaponRangedData.getIntValue(CERC_hitAddDamageBaseHit, row);
				currentWeaponData.m_addDamageElementalHit = weaponRangedData.getIntValue(CERC_hitAddDamageElementalHit, row);

				// always preload the templates
				currentWeaponData.fetch();

				IGNORE_RETURN(s_idToRangedEffectDataMap.insert(std::make_pair(weaponEffect, currentWeaponData)));
			}
			else
			{
				DEBUG_WARNING(true, ("CombatEffectsManager::install not creating ranged weapon effect, it already exists: %s", weaponEffect.c_str()));
			}

		}

		dataTableIff.close();
	}

	//-- load the damage type data
	if (dataTableIff.open("datatables/weapon/combat_effects_damage.iff", true))
	{
		DataTable damageTypeData;
		damageTypeData.load(dataTableIff);

		int const rowCount = damageTypeData.getNumRows();

		for (int row = 0; row < rowCount; ++row)
		{
			DamageTypeEffectData damageData;

			const int bitValue = damageTypeData.getIntValue(CEDC_bitValue, row);
			
			// we only want to continue if this effect does not exist
			DamageTypeTableItr itr = s_damageTypeToEffectDataMap.find(bitValue);
			if (itr == s_damageTypeToEffectDataMap.end())
			{
				const int red =      damageTypeData.getIntValue(CEDC_red, row);
				const int green =    damageTypeData.getIntValue(CEDC_green, row);
				const int blue =     damageTypeData.getIntValue(CEDC_blue, row);
				damageData.m_color.set(1.f ,red/255.f, green/255.f, blue/255.f);

				damageData.m_hitTargetEffect.m_appearance.m_name = damageTypeData.getStringValue(CEDC_hitTargetAppearance, row);
				damageData.m_hitTargetEffect.m_appearance.m_applyHue = damageTypeData.getIntValue(CEDC_hitTargetApplyHue, row);
				damageData.m_hitNothingEffect.m_appearance.m_name = damageTypeData.getStringValue(CEDC_hitNothingAppearance, row);
				damageData.m_hitNothingEffect.m_appearance.m_applyHue = damageTypeData.getIntValue(CEDC_hitNothingApplyHue, row);
				damageData.m_hitRicochetEffect.m_appearance.m_name = damageTypeData.getStringValue(CEDC_hitRicochetAppearance, row);
				damageData.m_hitRicochetEffect.m_appearance.m_applyHue = damageTypeData.getIntValue(CEDC_hitRicochetApplyHue, row);

				damageData.m_hitTargetEffect.m_sound.m_name = damageTypeData.getStringValue(CEDC_hitSound, row);
				damageData.m_hitTargetEffect.m_sound.m_pitch = damageTypeData.getFloatValue(CEDC_hitSoundPitch, row);
				
				//-- NOTE: hit nothing and hit ricochet are the same as the hit target sound in this table
				damageData.m_hitNothingEffect.m_sound = damageData.m_hitTargetEffect.m_sound;
				damageData.m_hitRicochetEffect.m_sound = damageData.m_hitRicochetEffect.m_sound;

				damageData.m_flyBySound.m_name = damageTypeData.getStringValue(CEDC_flyBySound, row);
				damageData.m_flyBySound.m_pitch = damageTypeData.getFloatValue(CEDC_flyBySoundPitch, row);

				damageData.fetch();

				IGNORE_RETURN(s_damageTypeToEffectDataMap.insert(std::make_pair(bitValue, damageData)));
			}
			else
			{
				DEBUG_WARNING(true, ("CombatEffectsManager::install not creating damage effect, it already exists: %d (bitValue)", bitValue));
			}
		}
	}

	std::map<std::string, int> locationIndexToNameMap;
	locationIndexToNameMap["HIT_LOCATION_BODY"]  = HIT_LOCATION_BODY;
	locationIndexToNameMap["HIT_LOCATION_HEAD"]  = HIT_LOCATION_HEAD;
	locationIndexToNameMap["HIT_LOCATION_R_ARM"] = HIT_LOCATION_R_ARM;
	locationIndexToNameMap["HIT_LOCATION_L_ARM"] = HIT_LOCATION_L_ARM;
	locationIndexToNameMap["HIT_LOCATION_R_LEG"] = HIT_LOCATION_R_LEG;
	locationIndexToNameMap["HIT_LOCATION_L_LEG"] = HIT_LOCATION_L_LEG;

	//-- load the hit location data
	if (dataTableIff.open("datatables/weapon/combat_effects_hit_locations.iff", true))
	{
		DataTable hitLocationData;

		hitLocationData.load(dataTableIff);

		//-- Store all of the info from the data table

		int const rowCount = hitLocationData.getNumRows();
		for (int row = 0; row < rowCount; ++row)
		{
			std::string hitLocationName = hitLocationData.getStringValue(CEHLC_location, row);

			std::string hardpointStr = hitLocationData.getStringValue(CEHLC_hardpoint, row);
			CrcLowerString hardpointName(hardpointStr.c_str());

			std::map<std::string, int>::iterator locationNameItr = locationIndexToNameMap.find(hitLocationName);

			if (locationNameItr != locationIndexToNameMap.end())
			{
				const int hitLocation = (*locationNameItr).second;

				std::map<int, CrcLowerStringList>::iterator locationItr = s_hitLocationToHardpointMap.find(hitLocation);
				if (locationItr == s_hitLocationToHardpointMap.end())
				{
					CrcLowerStringList newHardpointList;
					newHardpointList.push_back(new CrcLowerString(hardpointName));
					s_hitLocationToHardpointMap[hitLocation] = newHardpointList;
				}
				else
				{

					CrcLowerStringList & hardpointList = (*locationItr).second;
					bool needToAddHardpoint = true;

					CrcLowerStringList::iterator itr = hardpointList.begin();
					while (itr != hardpointList.end())
					{
						if (hardpointName == *(*itr))
						{
							needToAddHardpoint = false;
							break;
						}
						++itr;
					}

					if (needToAddHardpoint)
					{
						hardpointList.push_back(new CrcLowerString(hardpointName));
					}
				}
			}
		}
	}

	{
		// Load the hue range to swoosh template mapping

		Iff iff("datatables/weapon/swoosh_hue.iff");
		DataTable dataTable;
		dataTable.load(iff);

		for (int row = 0; row < dataTable.getNumRows(); ++row)
		{
			float const hueMin = dataTable.getFloatValue("HueMin", row);
			float const hueMax = dataTable.getFloatValue("HueMax", row);
			float const red = static_cast<float>(dataTable.getIntValue("Red", row)) / 255.0f;
			float const green = static_cast<float>(dataTable.getIntValue("Green", row)) / 255.0f;
			float const blue = static_cast<float>(dataTable.getIntValue("Blue", row)) / 255.0f;

			SwooshHueRange swooshHueRange(hueMin, hueMax, red, green, blue);
			s_hueRangeList.push_back(swooshHueRange);
		}
	}


	ExitChain::add (remove, "CombatEffectsManagerNamespace::remove");

	s_installed = true;
}

//-------------------------------------------------------------------
const VectorArgb & CombatEffectsManager::getWeaponDamageTypeColor(WeaponObject const & weaponObject)
{
	return getWeaponDamageTypeColor(weaponObject.getDamageType(), weaponObject.getElementalType());
}

const VectorArgb & CombatEffectsManager::getWeaponDamageTypeColor(WeaponObject::DamageType baseDamageType, WeaponObject::DamageType elementalDamageType)
{
	DEBUG_FATAL(!s_installed, ("Not installed"));

	VectorArgb returnValue;

	int combinedDamageType = baseDamageType + elementalDamageType;

	DamageTypeTableItr damageItr = s_damageTypeToEffectDataMap.find(combinedDamageType);
	if (damageItr != s_damageTypeToEffectDataMap.end())
	{
		return (*damageItr).second.m_color;
	}
	else
	{
		DEBUG_WARNING(true, ("getDamageTypeColor: invalid damage type specified (%d)", combinedDamageType));

		return VectorArgb::solidWhite;
	}
}

//-------------------------------------------------------------------

const CrcLowerString & CombatEffectsManager::getHitLocationHardPointName(const Object & targetObject, const int hitLocation)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, hitLocation, (int) HIT_LOCATION_NUM_LOCATIONS);

	std::map<int, CrcLowerStringList>::iterator locationItr = s_hitLocationToHardpointMap.find(hitLocation);
	if (locationItr != s_hitLocationToHardpointMap.end())
	{
		Appearance const * const targetAppearance = targetObject.getAppearance();

		if (targetAppearance != NULL)
		{
			CrcLowerStringList & hardpointList = (*locationItr).second;

			CrcLowerStringList::iterator itr = hardpointList.begin();
			while (itr != hardpointList.end())
			{
				// see if this is a valid hardpoint for this object
				Transform hardPointTransform;
				if (targetAppearance->findHardpoint(*(*itr), hardPointTransform))
				{
					// found valid hardpoint, so return it
					return *(*itr);
				}
				++itr;
			}
		}
	}

	return CrcLowerString::empty;
}

//-------------------------------------------------------------------
void CombatEffectsManager::getSwooshEffect(WeaponObject & weaponObject, std::string &outSwooshFilename, VectorArgb & outColor)
{
	DEBUG_FATAL(!s_installed, ("Not installed"));

	// Get the appearance from the weapon type
	MeleeWeaponEffectData const & effectData = getMeleeWeaponEffect(weaponObject);

	outSwooshFilename = effectData.m_swooshAppearance.m_name;
	outColor = VectorArgb::solidWhite;

	if (effectData.m_swooshAppearance.m_applyHue)
	{
		if (effectData.m_useWeaponHue)
		{
			 outColor = getObjectColor(weaponObject);
		}
		else
		{
			outColor = getWeaponDamageTypeColor(weaponObject);
		}
	}
}

//-------------------------------------------------------------------
bool CombatEffectsManager::actionHasSpecialAttackEffect(int actionNameCrc)
{
	if (actionNameCrc != 0)
	{
		SpecialAttackTableItr specialAttackItr = s_idToSpecialAttackEffectDataMap.find(actionNameCrc);
		if (specialAttackItr != s_idToSpecialAttackEffectDataMap.end())
			return true;
	}
	return false;
}


//-------------------------------------------------------------------
void CombatEffectsManager::createMeleeSwingEffect(WeaponObject & weaponObject)
{
	DEBUG_FATAL(!s_installed, ("Not installed"));

	// Get the appearance from the weapon type
	MeleeWeaponEffectData const & effectData = getMeleeWeaponEffect(weaponObject);

	Object * rootParent = ContainerInterface::getFirstParentInWorld(weaponObject);

	// play the attack sound
	SoundId soundId;
	if (!effectData.m_attackSound.m_name.empty())
	{
		soundId = Audio::attachSound(effectData.m_attackSound.m_name.c_str(), rootParent);
	}
}

//-------------------------------------------------------------------
void CombatEffectsManager::createMeleeHitEffect(WeaponObject & weaponObject, CrcLowerString const & weaponHardPoint, Object const * const targetObject, int actionNameCrc)
{
	DEBUG_FATAL(!s_installed, ("Not installed"));

	Transform hardPointTransform;
	Vector hitLocation(0.f, 0.f, 0.f);
	Vector hitNormal(0.f, 0.f, 1.f);

	Object * rootParent = ContainerInterface::getFirstParentInWorld(weaponObject);

	// Get the hard point transform
	if (rootParent != NULL)
	{
		Appearance const * const rootParentAppearance = rootParent->getAppearance();

		if (rootParentAppearance != NULL)
		{
			if (rootParentAppearance->findHardpoint(weaponHardPoint, hardPointTransform))
			{
				hitLocation = rootParent->rotateTranslate_o2w(hardPointTransform.getPosition_p());
				hitNormal = rootParent->rotateTranslate_o2w(hardPointTransform.getLocalFrameK_p());
			}
			else 
			{
				// no hardpoint / invalid hardpoint - so use the weapon location as the start of the ray
				hitLocation = weaponObject.getPosition_w();
				if (targetObject)
				{
					hitNormal = hitLocation - targetObject->getPosition_w();
					hitNormal.normalize();
				}

				DEBUG_WARNING((weaponHardPoint != CrcLowerString::empty), ("createMeleeHitEffect: Unable to find the hardpoint(%s) on the weaponObject(%s)", weaponHardPoint.getString(), weaponObject.getDebugInformation().c_str()));
			}

			if (targetObject)
			{
				Vector rayBegin        = hitLocation;
				Vector rayEnd          = targetObject->rotateTranslate_o2w(targetObject->getAppearanceSphereCenter()) ;

				CollisionInfo result;
				const uint16 collisionFlags = ClientWorld::CF_tangible | ClientWorld::CF_childObjects | ClientWorld::CF_skeletal | ClientWorld::CF_terrain;
				
				if (ClientWorld::collideObject(targetObject, rayBegin, rayEnd, CollideParameters::cms_default, result, collisionFlags))
				{
					hitLocation = result.getPoint();
					hitNormal = result.getNormal();
					hitNormal.normalize();
					hitLocation += hitNormal * 0.05f; // move the hit slightly outside the mesh
				}
			}
		}
		else
		{
			DEBUG_WARNING(true, ("createMeleeHitEffect: Unable to find the root parent appearance of the weapon object(%s)", weaponObject.getDebugInformation().c_str()));
		}
	}
	else
	{
		DEBUG_WARNING(true, ("createMeleeHitEffect: Unable to find the root parent of the weapon object(%s)", weaponObject.getDebugInformation().c_str()));
	}


	// Get the appearance from the weapon type
	MeleeWeaponEffectData const & effectData = getMeleeWeaponEffect(weaponObject);
	VectorArgb color(VectorArgb::solidWhite);

	if (effectData.m_swooshAppearance.m_applyHue)
	{
		if (effectData.m_useWeaponHue)
		{
			color = getObjectColor(weaponObject);
		}
		else
		{
			color = getWeaponDamageTypeColor(weaponObject);
		}
	}

	CellProperty & cell = rootParent ? *rootParent->getParentCell() : *CellProperty::getWorldCellProperty ();

	applyEffectPairAtLocation(&effectData.m_hitEffect, cell, hitLocation, hitNormal, color, 1.f );
	

	// apply damage type effect(s)
	if (!effectData.m_useWeaponHue)
	{
		if (weaponObject.getDamageType() != WeaponObject::DT_none)
		{
			DamageTypeEffectData const & damageTypeData = getDamageTypeEffect( weaponObject.getDamageType() );
			applyEffectPairAtLocation(&damageTypeData.m_hitTargetEffect, cell, hitLocation, hitNormal, color, 1.f );
		}

		if (weaponObject.getElementalType() != WeaponObject::DT_none)
		{
			DamageTypeEffectData const & elementalTypeData = getDamageTypeEffect( weaponObject.getElementalType() );
			applyEffectPairAtLocation(&elementalTypeData.m_hitTargetEffect, cell, hitLocation, hitNormal, color, 1.f );
		}
	}

	// now add the special attack effect, if specified
	if (actionNameCrc != 0)
	{
		SpecialAttackEffectData const & effectData = getSpecialAttackEffect( actionNameCrc );
		applyEffectPairAtLocation( &effectData.m_hitEffect, cell, hitLocation, hitNormal, color, 1.f );
	}

}

//-------------------------------------------------------------------
void CombatEffectsManager::createMuzzleFlash(WeaponObject & weaponObject, CrcLowerString const & hardPoint)
{
	DEBUG_FATAL(!s_installed, ("Not installed"));

	const RangedWeaponEffectData & effectData = getRangedWeaponEffect(weaponObject);

	Transform hardPointTransform;

	Object * rootParent = ContainerInterface::getFirstParentInWorld(weaponObject);

	// Get the hard point transform
	if (rootParent != NULL)
	{
		Appearance const * const rootParentAppearance = rootParent->getAppearance();

		if (rootParentAppearance != NULL)
		{
			if (!rootParentAppearance->findHardpoint(hardPoint, hardPointTransform))
			{
				DEBUG_WARNING(true, ("createMuzzleFlash: Unable to find the hardpoint(%s) on the weaponObject(%s)", hardPoint.getString(), weaponObject.getDebugInformation().c_str()));
			}
		}
		else
		{
			DEBUG_WARNING(true, ("createMuzzleFlash: Unable to find the root parent appearance of the weapon object(%s)", weaponObject.getDebugInformation().c_str()));
		}
	}
	else
	{
		DEBUG_WARNING(true, ("createMuzzleFlash: Unable to find the root parent of the weapon object(%s)", weaponObject.getDebugInformation().c_str()));
	}


	applyEffectPairAtHardpoint(&effectData.m_muzzleEffect, rootParent, hardPointTransform, hardPoint, getWeaponDamageTypeColor(weaponObject), 1.f );
}

//-------------------------------------------------------------------
void CombatEffectsManager::attachProjectileAppearance(WeaponObject const & weaponObject, Object & parentProjectile)
{
	DEBUG_FATAL(!s_installed, ("Not installed"));

	// Get the appearance from the weapon type
	RangedWeaponEffectData const & effectData = getRangedWeaponEffect(weaponObject);

	if (effectData.m_boltAppearance.m_template)
	{
		Appearance * const projectileAppearance = effectData.m_boltAppearance.m_template->createAppearance();

		LightningAppearance * const lightningAppearance = LightningAppearance::asLightningAppearance(projectileAppearance);
		if (lightningAppearance)
		{
			if (effectData.m_boltAppearance.m_applyHue)
			{
				lightningAppearance->setColorModifier(getWeaponDamageTypeColor(weaponObject));
			}
			parentProjectile.setAppearance(lightningAppearance);
		}
		else
		{
			ParticleEffectAppearance* const particleEffectAppearance = ParticleEffectAppearance::asParticleEffectAppearance(projectileAppearance);
			if (particleEffectAppearance)
			{
				if (effectData.m_boltAppearance.m_applyHue)
				{
					particleEffectAppearance->setColorModifier(getWeaponDamageTypeColor(weaponObject));
				}
				parentProjectile.setAppearance(particleEffectAppearance);
			}
		}
	}
}

//-------------------------------------------------------------------
void CombatEffectsManager::addProjectileSpecialAttackEffect(Object &projectile, uint32 attackCrc, WeaponObject const &weaponObject)
{

	if (attackCrc)
	{

		const SpecialAttackEffectData & effectData = getSpecialAttackEffect(attackCrc);

		if (effectData.m_trailAppearance.m_template)
		{
			SwooshAppearance * swooshAppearance = new SwooshAppearance(effectData.m_trailAppearance.m_template, &projectile);

			if (swooshAppearance)
			{
				Object* const trail = new Object ();
				trail->addNotification (ClientWorld::getIntangibleNotification ());
				RenderWorld::addObjectNotifications (*trail);
				trail->setParentCell (const_cast<CellProperty*> (projectile.getParentCell ()));
				trail->setTransform_o2p (projectile.getTransform_o2p ());

				if (effectData.m_trailAppearance.m_applyHue)
				{
					VectorArgb damageColor = CombatEffectsManager::getWeaponDamageTypeColor(weaponObject);

					swooshAppearance->setColorModifier(damageColor);
				}

				trail->setAppearance (swooshAppearance);

				trail->addToWorld ();
			}
		}
	}
}

//-------------------------------------------------------------------
void CombatEffectsManager::createHitEffect(CellProperty & cell, Vector const & position_l, Vector const & normal_l, std::string & weaponObjectTemplateName, WeaponObject::WeaponType weaponType, WeaponObject::DamageType baseDamageType, WeaponObject::DamageType elementalDamageType, int actionNameCrc, bool ricochet)
{
	DEBUG_FATAL(!s_installed, ("Not installed"));

	const RangedWeaponEffectData & effectData = getRangedWeaponEffect(weaponObjectTemplateName, weaponType);

	generateHitEffect(effectData, (ricochet ? HEHT_ricochet : HEHT_target), cell, position_l, normal_l, baseDamageType, elementalDamageType, actionNameCrc);
}

//-------------------------------------------------------------------
void CombatEffectsManager::createMissEffect(CellProperty & cell, Vector const & position_l, Vector const & normal_l, std::string & weaponObjectTemplateName, WeaponObject::WeaponType weaponType, WeaponObject::DamageType baseDamageType, WeaponObject::DamageType elementalDamageType)
{
	DEBUG_FATAL(!s_installed, ("Not installed"));

	const RangedWeaponEffectData & effectData = getRangedWeaponEffect(weaponObjectTemplateName, weaponType);

	generateHitEffect(effectData, HEHT_terrain, cell, position_l, normal_l, baseDamageType, elementalDamageType, 0);
}


//-------------------------------------------------------------------
void CombatEffectsManager::createFlyByEffect(Object& projectile, std::string const & /*weaponObjectTemplateName*/, WeaponObject::WeaponType /*weaponType*/, WeaponObject::DamageType baseDamageType, WeaponObject::DamageType /*elementalDamageType*/)
{
	DEBUG_FATAL(!s_installed, ("Not installed"));

	DamageTypeEffectData const & damageTypeData = getDamageTypeEffect( baseDamageType );

	if (!damageTypeData.m_flyBySound.m_name.empty())
	{
		SoundId soundId = Audio::attachSound(damageTypeData.m_flyBySound.m_name.c_str(), &projectile);
		Audio::setSoundPitchDelta(soundId, damageTypeData.m_flyBySound.m_pitch);
	}
}

//-------------------------------------------------------------------
void CombatEffectsManager::createSpecialAttackEffect(Object& targetObject, int actionNameCrc)
{
	if (actionNameCrc)
	{
		const SpecialAttackEffectData & effectData = getSpecialAttackEffect(actionNameCrc);

		if (!effectData.m_attackEffect.isEmpty())
		{
			CrcLowerString hardpointName1(effectData.m_attackEffectHardpointName.c_str());
			CrcLowerString hardpointName2;

			Object *hardpointObject = &targetObject;

			if (hardpointName1.isEmpty())
			{
				hardpointName1.setString("root");
			}
			else if (hardpointName1 == cs_genericWeaponHardpointName)
			{
				WeaponObject* weaponObject = getWeaponObject(&targetObject);

				if (weaponObject)
				{
					switch( weaponObject->getWeaponType() )
					{

					case WeaponObject::WT_rifle:
					case WeaponObject::WT_lightRifle:
					case WeaponObject::WT_pistol:
					case WeaponObject::WT_heavyWeapon:
						hardpointObject = weaponObject;
						hardpointName1.setString("muzzle");
						break;

					case WeaponObject::WT_1handMelee:
					case WeaponObject::WT_2handMelee:
						hardpointObject = weaponObject;
						hardpointName1.setString("whp1");
						break;

					case WeaponObject::WT_unarmed:
						// do not set hardpointObject to the weapon. Use the attacker.
						hardpointName1.setString("hold_r");
						hardpointName2.setString("hold_l");
						break;

					case WeaponObject::WT_polearm:
						hardpointObject = weaponObject;
						hardpointName1.setString("whp1");
						hardpointName2.setString("whp2");
						break;

					default:
						// do nothing.
						hardpointName1.clear();
						break;

					}
				}
				else
					DEBUG_WARNING(true, ("createSpecialAttackEffect: no weapon found for object(%s)", targetObject.getDebugInformation().c_str()));
			}

			Transform hardpointTransform;

			Appearance const * const targetAppearance = hardpointObject->getAppearance();

			if (targetAppearance != NULL)
			{
				if (!hardpointName1.isEmpty())
				{
					if (hardpointName1 == cs_objectLocationName)
					{
						CellProperty & cell = *targetObject.getParentCell();
						const Transform& transform = targetObject.getTransform_o2c();

						applyEffectPairAtLocation( &effectData.m_attackEffect, cell, transform.getPosition_p(), transform.getLocalFrameK_p(), VectorArgb::solidWhite, 1.f );
					}
					else
					{
						if (targetAppearance->findHardpoint(hardpointName1, hardpointTransform))
						{
							applyEffectPairAtHardpoint( &effectData.m_attackEffect, hardpointObject, hardpointTransform, hardpointName1, VectorArgb::solidWhite, 1.f );
						}
						else
						{
							DEBUG_WARNING(true, ("createSpecialAttackEffect: Unable to find the hardpoint(%s) on the object(%s)", hardpointName1.getString(), hardpointObject->getDebugInformation().c_str()));
						}
					}
				}

				if (!hardpointName2.isEmpty())
				{
					if (targetAppearance->findHardpoint(hardpointName2, hardpointTransform))
					{
						applyEffectPairAtHardpoint( &effectData.m_attackEffect, hardpointObject, hardpointTransform, hardpointName2, VectorArgb::solidWhite, 1.f );
					}
					else
					{
						DEBUG_WARNING(true, ("createSpecialAttackEffect: Unable to find the hardpoint(%s) on the object(%s)", hardpointName2.getString(), hardpointObject->getDebugInformation().c_str()));
					}
				}
			}
		}
	}
}

//===================================================================
