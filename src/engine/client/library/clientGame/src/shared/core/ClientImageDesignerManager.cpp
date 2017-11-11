// ======================================================================
//
// ClientImageDesignerManager.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientImageDesignerManager.h"

#include "clientGame/CreatureObject.h"
#include "clientGame/WorldSnapshot.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/Os.h"
#include "sharedGame/CustomizationManager.h"
#include "sharedGame/CustomizationManager_MorphParameter.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/CustomizationVariable.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/PaletteColorCustomizationVariable.h"
#include "sharedObject/RangedIntCustomizationVariable.h"

// ======================================================================

// ======================================================================

namespace ClientImageDesignerManagerNamespace
{
	class MyCallback : public MessageDispatch::Callback
	{
	public:
		MyCallback ()
		: MessageDispatch::Callback ()
		{
			connect (*this, &MyCallback::onScaleFactorChanged, static_cast<CustomizationManager::MorphParameter::Messages::ScaleFactorChanged *>(0));
		}

		void onScaleFactorChanged (CustomizationManager::MorphParameter::Messages::ScaleFactorChanged::Payload const & payload)
		{
			NetworkId const & nid = payload.first;
			Object * const o = NetworkIdManager::getObjectById(nid);
			ClientObject * const co = o ? o->asClientObject() : NULL;
			CreatureObject * const creature = co ? co->asCreatureObject() : NULL;
			if (creature)
			{
				ObjectTemplate const * const tmp = creature->getObjectTemplate();
				SharedObjectTemplate const * const tmpl = dynamic_cast<SharedObjectTemplate const *>(tmp);
				if(tmpl)
				{
					float const denormalized = CustomizationManager::MorphParameter::denormalizeScale (tmpl, payload.second);
					creature->setScaleFactor (denormalized);
				}
			}
		}//lint !e1762 function could be const (no it couldn't, overriding base function)
	};

	bool ms_installed = false;
	MyCallback * s_callback = 0;
}

using namespace ClientImageDesignerManagerNamespace;

// ======================================================================

void ClientImageDesignerManager::install ()
{
	InstallTimer const installTimer("ClientImageDesignerManager::install");

	DEBUG_FATAL(ms_installed, ("Already Installed"));
	ms_installed = true;
	s_callback = new MyCallback();
	ExitChain::add(ClientImageDesignerManager::remove, "ClientImageDesignerManager::remove", 0, false);
}

//----------------------------------------------------------------------

void ClientImageDesignerManager::remove ()
{
	delete s_callback;
	s_callback = NULL;
	DEBUG_FATAL(!ms_installed, ("Not Installed"));
	ms_installed = false;
}

//----------------------------------------------------------------------

void ClientImageDesignerManager::setMorphCustomization(CreatureObject & creature, std::string const & customizationName, float const value)
{
	if(!ms_installed)
	{
		DEBUG_FATAL(true, ("ClientImageDesignerManager not installed"));
		return; //lint !e527 unreachable (reachable in release)
	}

	std::string const & species_gender = CustomizationManager::getSharedSpeciesGender(creature);
	CustomizationManager::Customization customization;
	bool const result = CustomizationManager::getCustomization(species_gender, customizationName, customization);
	if(result)
	{
		CustomizationData * const customizationData = fetchCustomizationDataForCustomization(creature, customization);
		if(customizationData)
		{
			CustomizationManager::Customization customization;
			bool const result = CustomizationManager::getCustomization(CustomizationManager::getSharedSpeciesGender(creature), customizationName, customization);
			if(result)
			{
				CustomizationManager::MorphParameter param(*customizationData, customization);
				param.update(creature, value);
			}
			customizationData->release();
		}
	}
}

//----------------------------------------------------------------------

void ClientImageDesignerManager::setIndexCustomization(CreatureObject & creature, std::string const & customizationName, int const value)
{
	if(!ms_installed)
	{
		DEBUG_FATAL(true, ("ClientImageDesignerManager not installed"));
		return; //lint !e527 unreachable (reachable in release)
	}

	std::string const & species_gender = CustomizationManager::getSharedSpeciesGender(creature);
	CustomizationManager::Customization customization;
	bool const result = CustomizationManager::getCustomization(species_gender, customizationName, customization);
	if(result)
	{
		CustomizationData * const customizationData = fetchCustomizationDataForCustomization(creature, customization);
		if(customizationData)
		{
			CustomizationVariable * const cv = CustomizationManager::findVariable (*customizationData, customization.variables);
			PaletteColorCustomizationVariable * const paletteVariable = dynamic_cast<PaletteColorCustomizationVariable *>(cv);
			if(paletteVariable)
			{
				int rangeMin = 0;
				int rangeMax = 0;
				paletteVariable->getRange(rangeMin, rangeMax);
				int const finalValue = clamp(rangeMin, value, rangeMax);
				paletteVariable->setValue(finalValue);
			}
			customizationData->release();
		}
	}
}

//----------------------------------------------------------------------

float ClientImageDesignerManager::getMorphCustomization(CreatureObject & creature, std::string const & customizationName)
{
	if(!ms_installed)
	{
		DEBUG_FATAL(true, ("ClientImageDesignerManager not installed"));
		return 0.0f; //lint !e527 unreachable (reachable in release)
	}

	CustomizationManager::Customization customization;
	bool const result = CustomizationManager::getCustomization(CustomizationManager::getSharedSpeciesGender(creature), customizationName, customization);
	if(result)
	{
		CustomizationData * const customizationData = fetchCustomizationDataForCustomization(creature, customization);
		if(customizationData)
		{
			CustomizationVariable * const cv = CustomizationManager::findVariable (*customizationData, customization.variables);
			RangedIntCustomizationVariable * const rangedIntVariable = dynamic_cast<RangedIntCustomizationVariable *>(cv);
			if(rangedIntVariable)
				return rangedIntVariable->getNormalizedFloatValue();
			customizationData->release();
		}
	}
	return 0.0f;
}

//----------------------------------------------------------------------

int ClientImageDesignerManager::getIndexCustomization(CreatureObject & creature, std::string const & customizationName)
{
	if(!ms_installed)
	{
		DEBUG_FATAL(true, ("ClientImageDesignerManager not installed"));
		return 0; //lint !e527 unreachable (reachable in release)
	}

	std::string const & species_gender = CustomizationManager::getSharedSpeciesGender(creature);
	CustomizationManager::Customization customization;
	bool const result = CustomizationManager::getCustomization(species_gender, customizationName, customization);
	if(result)
	{
		CustomizationData * const customizationData = fetchCustomizationDataForCustomization(creature, customization);
		if(customizationData)
		{
			CustomizationVariable * const cv = CustomizationManager::findVariable (*customizationData, customization.variables);
			PaletteColorCustomizationVariable * const paletteVariable = dynamic_cast<PaletteColorCustomizationVariable *>(cv);
			if(paletteVariable)
				return paletteVariable->getValue();
			customizationData->release();
		}
	}
	return 0;
}

//----------------------------------------------------------------------

SharedImageDesignerManager::SkillMods ClientImageDesignerManager::getSkillModsForDesigner(NetworkId const & designerId)
{
	SharedImageDesignerManager::SkillMods skillMods;

	if(!ms_installed)
	{
		DEBUG_FATAL(true, ("ClientImageDesignerManager not installed"));
		return skillMods; //lint !e527 unreachable (reachable in release)
	}

	Object const * const o = NetworkIdManager::getObjectById(designerId);
	ClientObject const * const co = o ? o->asClientObject() : NULL;
	CreatureObject const * const designer = co ? co->asCreatureObject() : NULL;
	if(designer)
	{
		skillMods.bodySkillMod = designer->getModValue(SharedImageDesignerManager::cms_bodySkillModName);
		skillMods.faceSkillMod = designer->getModValue(SharedImageDesignerManager::cms_faceSkillModName);
		skillMods.markingsSkillMod = designer->getModValue(SharedImageDesignerManager::cms_markingsSkillModName);
		skillMods.hairSkillMod = designer->getModValue(SharedImageDesignerManager::cms_hairSkillModName);
	}
	return skillMods;
}

//----------------------------------------------------------------------

/** TODO find a better place to put this
    This function returns a networkId not in use by the client cache system, and within the "client-only" range of 0 - 10,000,000
*/
NetworkId ClientImageDesignerManager::getNextClientNetworkId()
{
	static int lastUsedNetworkId = 1;
	static int const lastAvailableClientNetworkId = 10000000;
	while(WorldSnapshot::isClientCached(++lastUsedNetworkId) || NetworkIdManager::getObjectById(NetworkId(static_cast<int64>(lastUsedNetworkId))))
	{
		if(lastUsedNetworkId >= lastAvailableClientNetworkId)
			lastUsedNetworkId = static_cast<int64>(0);
	}
	return NetworkId(static_cast<int64>(lastUsedNetworkId));
}

//----------------------------------------------------------------------

CustomizationData * ClientImageDesignerManager::fetchCustomizationDataForCustomization(CreatureObject & creature, CustomizationManager::Customization const & customization)
{
	//use the hair if it's the one to be queried
	TangibleObject * objectToQuery = &creature;
	if(customization.isVarHairColor)
	{
		ClientObject * const hair = creature.getHairObject();
		TangibleObject * const tangibleHair = hair ? hair->asTangibleObject() : NULL;
		if(tangibleHair)
			objectToQuery = tangibleHair;
		else
			return NULL;
	}
	return objectToQuery->fetchCustomizationData();
}

// =====================================================================
