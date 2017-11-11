//======================================================================
//
// ShipTargetAppearanceManager.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ShipTargetAppearanceManager.h"

#include "clientAudio/Audio.h"
#include "clientGame/ClientEffectTemplate.h"
#include "clientGame/ClientEffectTemplateList.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedGame/ShipChassis.h"
#include "sharedGame/ShipComponentDescriptor.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedUtility/DataTable.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/ExitChain.h"

#include <algorithm>
#include <map>
#include <set>

//======================================================================


namespace ShipTargetAppearanceManagerNamespace
{
	//-------------------------------------------------------------------------
	enum DatatableColumns
	{
		DC_chassisType,
		DC_targetAppearance,
		DC_targetAppearanceEnemy,
		DC_targetComponentAppearance,
		DC_targetComponentAppearanceEnemy,
		DC_silhouette,
		DC_useparentorientation,
		DC_scale,
		DC_hardpoint,
		DC_activateEffect,
		DC_activateEffectEnemy,
		DC_activateSound,
		DC_deactivateEffect,
		DC_deactivateEffectEnemy,
		DC_deactivateSound,
		DC_targetAcquiring,
		DC_targetAcquiringSound,
		DC_targetAcquired,
		DC_targetAcquiredSound
	};

	//-------------------------------------------------------------------------
	struct Data
	{
	public:

		std::string chassisType;
		std::string targetAppearanceName;
		std::string targetAppearanceEnemyName;
		std::string targetComponentAppearanceName;
		std::string targetComponentAppearanceEnemyName;
		float silhouette;
		bool useparentorientation;
		float scale;
		CrcLowerString hardpoint;
		std::string activateEffectName;
		std::string activateEffectEnemyName;
		std::string activateSoundName;

		std::string deactivateEffectName;
		std::string deactivateEffectEnemyName;
		std::string deactivateSoundName;

		// Used for targeting for missiles.
		std::string targetAcquiringName;
		std::string targetAcquiringSoundName;

		std::string targetAcquiredName;
		std::string targetAcquiredSoundName;

	public:

		AppearanceTemplate const * targetAppearance;
		AppearanceTemplate const * targetAppearanceEnemy;
		AppearanceTemplate const * targetComponentAppearance;
		AppearanceTemplate const * targetComponentAppearanceEnemy;
		ClientEffectTemplate const * activateEffect;
		ClientEffectTemplate const * activateEffectEnemy;
		ClientEffectTemplate const * deactivateEffect;
		ClientEffectTemplate const * deactivateEffectEnemy;
		ClientEffectTemplate const * targetAcquiringEffect;
		AppearanceTemplate const * targetAcquiredAppearance;

	public:

		Data();
		~Data();
	};

	//-------------------------------------------------------------------------
	typedef std::map<uint32 /*crc*/, Data *> DataMap;

	std::string const filename = "datatables/space/ship_target_appearance.iff";
	bool s_installed = false;
	DataMap s_data;
	char const * const c_defaultChassis = "default";
	uint32 const c_defaultChassisCrc = Crc::normalizeAndCalculate(c_defaultChassis);
	float const c_defaultScale = 1.0f;
	float const c_defaultSilhouette = 0.0f;
	bool const c_defaultUseParentOrientation = false;

	//-------------------------------------------------------------------------
	// Find data.
	Data * internalFindData(uint32 key)
	{
		Data * data = NULL;

		DataMap::iterator itData = s_data.find(key);
		if (s_data.end() != itData)
			data = itData->second;

		return data;
	}

#ifdef _DEBUG
	std::set<uint32> beenWarned;
#endif

	Data * findData(uint32 const chassisType)
	{
		Data * data = internalFindData(chassisType);

		if (NULL == data)
		{
#ifdef _DEBUG
			std::pair<std::set<uint32>::iterator, bool> check = beenWarned.insert(chassisType);
			if(check.second)
			{
				ShipChassis const * const shipChassis = ShipChassis::findShipChassisByCrc(chassisType);
				DEBUG_WARNING(shipChassis == NULL, ("ShipTargetAppearanceManager: Chassis is NULL"));
			}
#endif

			data = internalFindData(c_defaultChassisCrc);
		}

		return data;
	}



	//-------------------------------------------------------------------------
}

using namespace ShipTargetAppearanceManagerNamespace;

//----------------------------------------------------------------------

Data::Data() :
chassisType(),
targetAppearanceName(),
targetAppearanceEnemyName(),
targetComponentAppearanceName(),
targetComponentAppearanceEnemyName(),
silhouette(1.0f),
useparentorientation(true),
scale(1.0f),
hardpoint(),
activateEffectName(),
activateEffectEnemyName(),
activateSoundName(),
deactivateEffectName(),
deactivateEffectEnemyName(),
deactivateSoundName(),
targetAcquiringName(),
targetAcquiringSoundName(),
targetAcquiredName(),
targetAcquiredSoundName(),
targetAppearance(0),
targetAppearanceEnemy(0),
targetComponentAppearance(0),
targetComponentAppearanceEnemy(0),
activateEffect(0),
activateEffectEnemy(0),
deactivateEffect(0),
deactivateEffectEnemy(0),
targetAcquiringEffect(0),
targetAcquiredAppearance(0)
{
}

//----------------------------------------------------------------------

Data::~Data()
{
	if (targetAppearance)
	{
		AppearanceTemplateList::release(targetAppearance);
		targetAppearance = 0;
	}

	if (targetAppearanceEnemy)
	{
		AppearanceTemplateList::release(targetAppearanceEnemy);
		targetAppearanceEnemy = 0;
	}

	if (targetComponentAppearance)
	{
		AppearanceTemplateList::release(targetComponentAppearance);
		targetComponentAppearance = 0;
	}

	if (targetComponentAppearanceEnemy)
	{
		AppearanceTemplateList::release(targetComponentAppearanceEnemy);
		targetComponentAppearanceEnemy = 0;
	}

	if (activateEffect)
	{
		activateEffect->release();
		activateEffect = 0;
	}

	if (activateEffectEnemy)
	{
		activateEffectEnemy->release();
		activateEffectEnemy = 0;
	}

	if (deactivateEffect)
	{
		deactivateEffect->release();
		deactivateEffect = 0;
	}

	if (deactivateEffectEnemy)
	{
		deactivateEffectEnemy->release();
		deactivateEffectEnemy = 0;
	}

	if (targetAcquiredAppearance)
	{
		AppearanceTemplateList::release(targetAcquiredAppearance);
		targetAcquiredAppearance = 0;
	}

	if (targetAcquiringEffect)
	{
		targetAcquiringEffect->release();
		targetAcquiringEffect = 0;
	}
}

//----------------------------------------------------------------------

void ShipTargetAppearanceManager::install()
{
	InstallTimer const installTimer("ShipTargetAppearanceManager::install");

	DEBUG_FATAL(s_installed, ("ShipTargetAppearanceManager already installed"));
	s_installed = true;

	Iff iff;
	if (iff.open(filename.c_str(), true))
	{
		DataTable dataTable;
		dataTable.load(iff);

		const int numRows = dataTable.getNumRows();

		for (int row = 0; row < numRows; ++row)
		{
			Data * const newData = new Data;

			newData->chassisType = dataTable.getStringValue(static_cast<int>(DC_chassisType), row);
			newData->targetAppearanceName = dataTable.getStringValue(static_cast<int>(DC_targetAppearance), row);
			newData->targetAppearanceEnemyName = dataTable.getStringValue(static_cast<int>(DC_targetAppearanceEnemy), row);
			newData->targetComponentAppearanceName = dataTable.getStringValue(static_cast<int>(DC_targetComponentAppearance), row);
			newData->targetComponentAppearanceEnemyName = dataTable.getStringValue(static_cast<int>(DC_targetComponentAppearanceEnemy), row);
			newData->silhouette = dataTable.getFloatValue(static_cast<int>(DC_silhouette), row);
			newData->useparentorientation = dataTable.getIntValue(static_cast<int>(DC_useparentorientation), row) > 0;
			newData->scale = dataTable.getFloatValue(static_cast<int>(DC_scale), row);
			newData->hardpoint = CrcLowerString(dataTable.getStringValue(static_cast<int>(DC_hardpoint), row));

			newData->activateEffectName = dataTable.getStringValue(static_cast<int>(DC_activateEffect), row);
			newData->activateEffectEnemyName = dataTable.getStringValue(static_cast<int>(DC_activateEffectEnemy), row);
			newData->activateSoundName = dataTable.getStringValue(static_cast<int>(DC_activateSound), row);

			newData->deactivateEffectName = dataTable.getStringValue(static_cast<int>(DC_deactivateEffect), row);
			newData->deactivateEffectEnemyName = dataTable.getStringValue(static_cast<int>(DC_deactivateEffectEnemy), row);
			newData->deactivateSoundName = dataTable.getStringValue(static_cast<int>(DC_deactivateSound), row);

			newData->targetAcquiringName = dataTable.getStringValue(static_cast<int>(DC_targetAcquiring), row);
			newData->targetAcquiringSoundName = dataTable.getStringValue(static_cast<int>(DC_targetAcquiringSound), row);

			newData->targetAcquiredName = dataTable.getStringValue(static_cast<int>(DC_targetAcquired), row);
			newData->targetAcquiredSoundName = dataTable.getStringValue(static_cast<int>(DC_targetAcquiredSound), row);

			std::pair<DataMap::iterator, bool> result = s_data.insert(std::make_pair(Crc::normalizeAndCalculate(newData->chassisType.c_str()), newData));
			UNREF(result);
			DEBUG_WARNING(!result.second, ("ShipTargetAppearanceManager: [%s] chassis type %s already defined", filename.c_str(), newData->chassisType.c_str()));

			//-- Fetch the necessary templates
			if (!newData->targetAppearanceName.empty())
				newData->targetAppearance = AppearanceTemplateList::fetch(newData->targetAppearanceName.c_str());

			if (!newData->targetAppearanceEnemyName.empty())
				newData->targetAppearanceEnemy = AppearanceTemplateList::fetch(newData->targetAppearanceEnemyName.c_str());

			if (!newData->targetComponentAppearanceName.empty())
				newData->targetComponentAppearance = AppearanceTemplateList::fetch(newData->targetComponentAppearanceName.c_str());

			if (!newData->targetComponentAppearanceEnemyName.empty())
				newData->targetComponentAppearanceEnemy = AppearanceTemplateList::fetch(newData->targetComponentAppearanceEnemyName.c_str());

			if (!newData->activateEffectName.empty())
				newData->activateEffect = ClientEffectTemplateList::fetch(CrcLowerString(newData->activateEffectName.c_str()));

			if (!newData->activateEffectEnemyName.empty())
				newData->activateEffectEnemy = ClientEffectTemplateList::fetch(CrcLowerString(newData->activateEffectEnemyName.c_str()));

			if (!newData->deactivateEffectName.empty())
				newData->deactivateEffect = ClientEffectTemplateList::fetch(CrcLowerString(newData->deactivateEffectName.c_str()));

			if (!newData->deactivateEffectEnemyName.empty())
				newData->deactivateEffectEnemy = ClientEffectTemplateList::fetch(CrcLowerString(newData->deactivateEffectEnemyName.c_str()));

			if (!newData->targetAcquiredName.empty())
				newData->targetAcquiredAppearance = AppearanceTemplateList::fetch(newData->targetAcquiredName.c_str());

			if (!newData->targetAcquiringName.empty())
				newData->targetAcquiringEffect = ClientEffectTemplateList::fetch(CrcLowerString(newData->targetAcquiringName.c_str()));
		}
	}
	else
	{
		WARNING(true, ("ShipTargetAppearanceManager no such datatable [%s]", filename.c_str()));
	}

	ExitChain::add(ShipTargetAppearanceManager::remove, "ShipTargetAppearanceManager::remove");
}

//----------------------------------------------------------------------

void ShipTargetAppearanceManager::remove()
{
	s_installed = false;

	std::for_each(s_data.begin(), s_data.end(), PointerDeleterPairSecond());
	s_data.clear();
}

//----------------------------------------------------------------------

AppearanceTemplate const * ShipTargetAppearanceManager::getAppearanceTemplate(uint32 const chassisType, bool const isEnemy)
{
	Data const * const data = findData(chassisType);

	if (NULL != data)
	{
		if (isEnemy && data->targetAppearanceEnemy)
			return data->targetAppearanceEnemy;

		return data->targetAppearance;
	}

	return NULL;
}

//----------------------------------------------------------------------

AppearanceTemplate const * ShipTargetAppearanceManager::getAppearanceTemplateForComponent(uint32 const chassisType, bool const isEnemy)
{
	Data const * const data = findData(chassisType);

	if (NULL != data)
	{
		if (isEnemy && data->targetComponentAppearanceEnemy)
			return data->targetComponentAppearanceEnemy;

		return data->targetComponentAppearance;
	}

	return NULL;
}

//----------------------------------------------------------------------

ClientEffectTemplate const * ShipTargetAppearanceManager::getClientEffectTemplate(uint32 const chassisType, bool const isActive, bool const isEnemy)
{
	Data const * const data = findData(chassisType);

	if (NULL != data)
	{
		if (isActive)
		{
			if (isEnemy && data->activateEffectEnemy)
				return data->activateEffectEnemy;

			return data->activateEffect;
		}
		else
		{
			if (isEnemy && data->deactivateEffectEnemy)
				return data->deactivateEffectEnemy;

			return data->deactivateEffect;
		}
	}

	return NULL;
}

//----------------------------------------------------------------------

float ShipTargetAppearanceManager::getScale(uint32 const chassisType)
{
	Data const * const data = findData(chassisType);

	return data ? data->scale : c_defaultScale;
}

//----------------------------------------------------------------------

float ShipTargetAppearanceManager::getSilhouetteDistance(uint32 const chassisType)
{
	Data const * const data = findData(chassisType);

	return data ? data->silhouette : c_defaultSilhouette;
}

//----------------------------------------------------------------------

bool ShipTargetAppearanceManager::useParentOrientation(uint32 const chassisType)
{
	Data const * const data = findData(chassisType);

	return data ? data->useparentorientation : c_defaultUseParentOrientation;
}

//----------------------------------------------------------------------

CrcLowerString const & ShipTargetAppearanceManager::getHardpointName(uint32 const chassisType)
{
	Data const * const data = findData(chassisType);

	return data ? (data->hardpoint.getString() ? data->hardpoint : CrcLowerString::empty) : CrcLowerString::empty;
}

//----------------------------------------------------------------------

AppearanceTemplate const * ShipTargetAppearanceManager::getAcquiredAppearanceTemplate(uint32 chassisType)
{
	Data const * const data = findData(chassisType);

	if (NULL != data)
	{
		return data->targetAcquiredAppearance;
	}

	return NULL;
}

//----------------------------------------------------------------------

ClientEffectTemplate const * ShipTargetAppearanceManager::getAcquiringEffectTemplate(uint32 chassisType)
{
	Data const * const data = findData(chassisType);

	if (NULL != data)
	{
		return data->targetAcquiringEffect;
	}

	return NULL;
}

//----------------------------------------------------------------------

void ShipTargetAppearanceManager::playActivateSound(uint32 chassisType)
{
	Data const * const data = findData(chassisType);

	if (NULL != data)
	{
		IGNORE_RETURN(Audio::playSound(data->activateSoundName.c_str(),NULL));
	}
}

//----------------------------------------------------------------------

void ShipTargetAppearanceManager::playDeactivateSoundName(uint32 chassisType)
{
	Data const * const data = findData(chassisType);

	if (NULL != data)
	{
		IGNORE_RETURN(Audio::playSound(data->deactivateSoundName.c_str(),NULL));
	}
}


//----------------------------------------------------------------------

void ShipTargetAppearanceManager::playTargetAcquiringSoundName(uint32 chassisType)
{
	Data const * const data = findData(chassisType);

	if (NULL != data)
	{
		IGNORE_RETURN(Audio::playSound(data->targetAcquiringSoundName.c_str(),NULL));
	}
}

//----------------------------------------------------------------------

void ShipTargetAppearanceManager::playTargetAcquiredSoundName(uint32 chassisType)
{
	Data const * const data = findData(chassisType);

	if (NULL != data)
	{
		IGNORE_RETURN(Audio::playSound(data->targetAcquiredSoundName.c_str(),NULL));
	}
}

//======================================================================

