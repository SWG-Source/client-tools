//======================================================================
//
// ShipComponentPowerEffectsManagerClient.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ShipComponentPowerEffectsManagerClient.h"

#include "clientAudio/Audio.h"
#include "clientAudio/SoundTemplate.h"
#include "clientGame/ShipObject.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedGame/ShipChassis.h"
#include "sharedGame/ShipChassisSlotType.h"
#include "sharedGame/ShipComponentType.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"
#include <map>

//----------------------------------------------------------------------

namespace ShipComponentPowerEffectsManagerClientNamespace
{
	typedef std::pair<std::string, std::string> DisableEnableSounds;
	typedef stdmap<int /* component type */, DisableEnableSounds>::fwd SoundComponentMap;
	typedef stdmap<uint32 /* ship sound type crc*/, SoundComponentMap>::fwd SoundShipComponentMap;

	SoundShipComponentMap s_soundShipComponentMap;

	bool s_installed = false;
	uint32 const s_defaultSoundGroupCrc = Crc::normalizeAndCalculate("default");

	//----------------------------------------------------------------------

	std::string const & findSoundNameForComponent(ShipObject const & ship, int const componentType, bool const disableEnable)
	{
		static const std::string empty;

		ShipChassis const * const shipChassis = ShipChassis::findShipChassisByCrc(ship.getChassisType());
		if (NULL == shipChassis)
		{
			WARNING(true, ("ShipComponentPowerEffectsManagerClient invalid ship chassis for ship [%s]", ship.getDebugInformation()));
			return empty;
		}

		uint32 const hitSoundGroupCrc = Crc::normalizeAndCalculate(shipChassis->getHitSoundGroup().c_str());

		SoundShipComponentMap::const_iterator it = s_soundShipComponentMap.find(hitSoundGroupCrc);
		if (it == s_soundShipComponentMap.end())
		{
			//-- fall back to default sound group
			it = s_soundShipComponentMap.find(s_defaultSoundGroupCrc);

			if (it == s_soundShipComponentMap.end())
			{
				WARNING(true, ("ShipComponentPowerEffectsManagerClient invalid soundShipSlotMap"));
				return empty;
			}
		}

		SoundComponentMap const & soundComponentMap = (*it).second;

		SoundComponentMap::const_iterator ssit = soundComponentMap.find(componentType);
		if (ssit == soundComponentMap.end())
		{
			//-- fall back to default component type
			ssit = soundComponentMap.find(static_cast<int>(ShipComponentType::SCT_num_types));

			if (ssit == soundComponentMap.end())
			{
				WARNING(true, ("ShipComponentPowerEffectsManagerClient invalid soundShipSlotMap"));
				return empty;
			}
		}

		DisableEnableSounds const & soundNames = (*ssit).second;

		if (disableEnable)
			return soundNames.first;
		else
			return soundNames.second;
	}

	//----------------------------------------------------------------------

	std::string const & findSoundNameForSlot(ShipObject const & ship, int const chassisSlot, bool const disableEnable)
	{
		int const componentType = ShipChassisSlotType::getComponentTypeForSlot(static_cast<ShipChassisSlotType::Type>(chassisSlot));
		return findSoundNameForComponent(ship, componentType, disableEnable);
	}

	//----------------------------------------------------------------------

	enum DataTableCol
	{
		DTC_type,
		DTC_default_disabled,
		DTC_default_enabled,
		DTC_weapon_disabled,
		DTC_weapon_enabled,
		DTC_booster_disabled,
		DTC_booster_enabled,
		DTC_engine_disabled,
		DTC_engine_enabled,
		DTC_reactor_disabled,
		DTC_reactor_enabled
	};

	//----------------------------------------------------------------------

}

using namespace ShipComponentPowerEffectsManagerClientNamespace;

//----------------------------------------------------------------------

void ShipComponentPowerEffectsManagerClient::install()
{
	DEBUG_FATAL(s_installed, ("ShipComponentPowerEffectsManagerClient already installed"));
	s_installed = true;
	ExitChain::add(ShipComponentPowerEffectsManagerClient::remove, "ShipComponentPowerEffectsManagerClient::remove");
	
	//-- load the datatables up
	
	std::string const & filename = "datatables/space/ship_power_sounds.iff";
	
	DataTable * const dt = DataTableManager::getTable(filename, true);
	
	if (dt == NULL)
	{
		WARNING(true, ("ShipComponentPowerEffectsManagerClient no such datatable [%s]", filename.c_str()));
		return;
	}
	
	int numRows = dt->getNumRows();
	
	for (int row = 0; row < numRows; ++row)
	{
		std::string const & shipHitType = dt->getStringValue(0, row);

		SoundComponentMap & scm = s_soundShipComponentMap[Crc::normalizeAndCalculate(shipHitType.c_str())];

		DisableEnableSounds def(dt->getStringValue(DTC_default_disabled, row), dt->getStringValue(DTC_default_enabled, row));
		DisableEnableSounds weapon(dt->getStringValue(DTC_weapon_disabled, row), dt->getStringValue(DTC_weapon_enabled, row));
		DisableEnableSounds booster(dt->getStringValue(DTC_booster_disabled, row), dt->getStringValue(DTC_booster_enabled, row));
		DisableEnableSounds engine(dt->getStringValue(DTC_engine_disabled, row), dt->getStringValue(DTC_engine_enabled, row));
		DisableEnableSounds reactor(dt->getStringValue(DTC_reactor_disabled, row), dt->getStringValue(DTC_reactor_enabled, row));

		scm[static_cast<int>(ShipComponentType::SCT_num_types)] = def;
		scm[static_cast<int>(ShipComponentType::SCT_weapon)] = weapon;
		scm[static_cast<int>(ShipComponentType::SCT_booster)] = booster;
		scm[static_cast<int>(ShipComponentType::SCT_engine)] = engine;
		scm[static_cast<int>(ShipComponentType::SCT_reactor)] = reactor;

		//-- check exists
		if (!TreeFile::exists(def.first.c_str()))
			WARNING(true, ("ShipComponentPowerEffectsManagerClient invalid sound [%s]", def.first.c_str()));
		if (!TreeFile::exists(def.second.c_str()))
			WARNING(true, ("ShipComponentPowerEffectsManagerClient invalid sound [%s]", def.second.c_str()));
		if (!TreeFile::exists(weapon.first.c_str()))
			WARNING(true, ("ShipComponentPowerEffectsManagerClient invalid sound [%s]", weapon.first.c_str()));
		if (!TreeFile::exists(weapon.second.c_str()))
			WARNING(true, ("ShipComponentPowerEffectsManagerClient invalid sound [%s]", weapon.second.c_str()));
		if (!TreeFile::exists(booster.first.c_str()))
			WARNING(true, ("ShipComponentPowerEffectsManagerClient invalid sound [%s]", booster.first.c_str()));
		if (!TreeFile::exists(booster.second.c_str()))
			WARNING(true, ("ShipComponentPowerEffectsManagerClient invalid sound [%s]", booster.second.c_str()));
		if (!TreeFile::exists(engine.first.c_str()))
			WARNING(true, ("ShipComponentPowerEffectsManagerClient invalid sound [%s]", engine.first.c_str()));
		if (!TreeFile::exists(engine.second.c_str()))
			WARNING(true, ("ShipComponentPowerEffectsManagerClient invalid sound [%s]", engine.second.c_str()));
		if (!TreeFile::exists(reactor.first.c_str()))
			WARNING(true, ("ShipComponentPowerEffectsManagerClient invalid sound [%s]", reactor.first.c_str()));
		if (!TreeFile::exists(reactor.second.c_str()))
			WARNING(true, ("ShipComponentPowerEffectsManagerClient invalid sound [%s]", reactor.second.c_str()));
	};

	DataTableManager::close(filename);
}

//----------------------------------------------------------------------

void ShipComponentPowerEffectsManagerClient::remove()
{
	DEBUG_FATAL(!s_installed, ("ShipComponentPowerEffectsManagerClient not installed"));
	s_installed = false;
	s_soundShipComponentMap.clear();
}

//----------------------------------------------------------------------

void ShipComponentPowerEffectsManagerClient::playEnableEffect(ShipObject const & ship, int chassisSlot)
{
	bool const disableEnableFlag = false;
	std::string const & soundName = findSoundNameForSlot(ship, chassisSlot, disableEnableFlag);
	if (soundName.empty())
	{
		WARNING(true, ("ShipComponentPowerEffectsManagerClient::playEnableEffect() failed"));
		return;
	}

	Audio::attachSound(soundName.c_str(), &ship, NULL);
}

//----------------------------------------------------------------------

void ShipComponentPowerEffectsManagerClient::playDisableEffect(ShipObject const & ship, int chassisSlot)
{
	bool const disableEnableFlag = true;
	std::string const & soundName = findSoundNameForSlot(ship, chassisSlot, disableEnableFlag);
	if (soundName.empty())
	{
		WARNING(true, ("ShipComponentPowerEffectsManagerClient::playDisableEffect() failed"));
		return;
	}

	Audio::attachSound(soundName.c_str(), &ship, NULL);
}

//----------------------------------------------------------------------

