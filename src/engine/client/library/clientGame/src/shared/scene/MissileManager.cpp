// ======================================================================
//
// MissileManager.cpp
//
// copyright 2004, Sony Online Entertainment
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/MissileManager.h"

#include "clientGame/ClientEffectTemplate.h"
#include "clientGame/ClientEffectTemplateList.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/Countermeasure.h"
#include "clientGame/Game.h"
#include "clientGame/GameMusicManager.h"
#include "clientGame/Missile.h"
#include "clientGame/ShipObject.h"
#include "clientGame/ShipObjectAttachments.h"
#include "clientGraphics/RenderWorld.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedRandom/Random.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"

#include <map>

// ======================================================================

bool MissileManager::ms_installed = false;
MissileManager::MissilesType * MissileManager::ms_missiles=NULL;
MissileManager::MissileTypeDataType * MissileManager::ms_missileTypeData=NULL;
MissileManager::CountermeasuresType * MissileManager::ms_countermeasures=NULL;
size_t MissileManager::ms_maxCountermeasures=50;
MissileManager::CountermeasureTypeDataType * MissileManager::ms_countermeasureTypeData=NULL;

namespace MissileManagerNamespace
{
	std::string const s_missileHardpointName = "missile";
	float const cs_defaultTargetAcquisitionTime = 2.0f;
	float const cs_defaultTargetAcquisitionAngle = 10.0f;

	// So to prevent players from hacking these client side only values, we're moving these internal.
	// These match to missile IDs in missiles.iff
	float ms_clientAquisitionTimes[] =
	{  
		2.0f, // Missile ID 0
		2.0f, // 1
		2.0f, // 2
		1.0f, // 3
		1.0f, // 4
		2.0f, // 5
		2.0f, // 6
		2.0f, // 7
		2.0f, // 8
		2.0f, // 9
		2.0f, // 10
		2.0f, // 11
		2.0f, // 12
		2.0f, // 13
		2.0f, // 14
		2.0f, // 15
		2.0f, // 16
		2.0f, // 17
		2.0f, // 18
		2.0f, // 19
		2.0f, // 20
	};

	int ms_totalLoadedAquisitions = sizeof(ms_clientAquisitionTimes) / sizeof(ms_clientAquisitionTimes[0]);
}

using namespace MissileManagerNamespace;

// ======================================================================

MissileManager::MissileTypeDataRecord::MissileTypeDataRecord() :
m_typeId(0),
m_missileAppearance(NULL),
m_trailAppearance(NULL),
m_fireEffect(NULL),
m_hitEffect(NULL),
m_countermeasureEffect(NULL),
m_speed(0.0f),
m_targetAcquisitionSeconds(cs_defaultTargetAcquisitionTime),
m_targetAcquisitionAngle(cs_defaultTargetAcquisitionAngle),
m_time(0.0),
m_missileAppearanceName(),
m_trailAppearanceName(),
m_fireEffectName(),
m_hitEffectName(),
m_countermeasureEffectName(),
m_preloaded(false)
{
}

//----------------------------------------------------------------------

void MissileManager::MissileTypeDataRecord::preloadResources()
{
	if (m_preloaded)
	{
		return;
	}

	m_preloaded = true;

	if (!m_missileAppearanceName.empty())
		m_missileAppearance = AppearanceTemplateList::fetch(m_missileAppearanceName.c_str());

	if (!m_trailAppearanceName.empty())
		m_trailAppearance = AppearanceTemplateList::fetch(m_trailAppearanceName.c_str());

	if (!m_fireEffectName.empty())
		m_fireEffect= ClientEffectTemplateList::fetch(CrcLowerString(m_fireEffectName.c_str()));

	if (!m_hitEffectName.empty())
		m_hitEffect = ClientEffectTemplateList::fetch(CrcLowerString(m_hitEffectName.c_str()));

	if (!m_countermeasureEffectName.empty())
		m_countermeasureEffect = ClientEffectTemplateList::fetch(CrcLowerString(m_countermeasureEffectName.c_str()));

	DEBUG_WARNING(!m_missileAppearance,("Missile type %i has invalid appearance \"%s\" specified.",m_typeId, m_missileAppearanceName.c_str()));
	DEBUG_WARNING(!m_trailAppearance,("Missile type %i has trail appearance \"%s\" specified.",m_typeId, m_trailAppearanceName.c_str()));
	DEBUG_WARNING(!m_fireEffect,("Missile type %i has invalid fire effect \"%s\" specified.",m_typeId, m_fireEffectName.c_str()));
	DEBUG_WARNING(!m_hitEffect,("Missile type %i has invalid hit effect \"%s\" specified.",m_typeId, m_hitEffectName.c_str()));
	DEBUG_WARNING(!m_countermeasureEffect,("Missile type %i has invalid countermeasure effect \"%s\" specified.",m_typeId, m_countermeasureEffectName.c_str()));
}

//----------------------------------------------------------------------

void MissileManager::MissileTypeDataRecord::releaseResources()
{
	if (m_missileAppearance != NULL)
		AppearanceTemplateList::release(m_missileAppearance);

	if (m_trailAppearance != NULL)
		AppearanceTemplateList::release(m_trailAppearance);

	if (m_fireEffect != NULL)
		m_fireEffect->release();

	if (m_hitEffect != NULL)
		m_hitEffect->release();

	if (m_countermeasureEffect != NULL)
		m_countermeasureEffect->release();

	m_missileAppearance = NULL;
	m_trailAppearance = NULL;
	m_fireEffect = NULL;
	m_hitEffect = NULL;
	m_countermeasureEffect = NULL;
}

//----------------------------------------------------------------------

MissileManager::CountermeasureTypeDataRecord::CountermeasureTypeDataRecord() :
m_typeId(0),
m_appearance(0),
m_appearanceName(),
m_minNumber(0),
m_maxNumber(0),
m_minSpeed(0.0f),
m_maxSpeed(0.0f),
m_maxScatterAngle(0.0f),
m_preloaded(false)
{
}

//----------------------------------------------------------------------

void MissileManager::CountermeasureTypeDataRecord::preloadResources()
{
	if (m_preloaded)
	{
		return;
	}

	m_preloaded = true;

	if (!m_appearanceName.empty())
		m_appearance = AppearanceTemplateList::fetch(m_appearanceName.c_str());

	DEBUG_WARNING(!m_appearance,("countermeasures.iff specified appearance %s, which could not be loaded", m_appearanceName.c_str()));
}

//----------------------------------------------------------------------

void MissileManager::CountermeasureTypeDataRecord::releaseResources()
{
	if (m_appearance != NULL)
		AppearanceTemplateList::release(m_appearance);

	m_appearance = 0;
}

//----------------------------------------------------------------------

void MissileManager::install()
{
	InstallTimer const installTimer("MissileManager::install");

	DEBUG_FATAL(ms_installed,("MissileManager::install was called more than once."));
	ms_missiles = new MissileManager::MissilesType;
	ms_missileTypeData = new MissileTypeDataType;
	ms_countermeasures = new CountermeasuresType;
	ms_countermeasureTypeData = new CountermeasureTypeDataType;
	ms_installed = true;

	ExitChain::add(&remove,"MissileManager::remove");

	{
		DataTable const * const missileDataTable = DataTableManager::getTable("datatables/space/missiles.iff", true);
		FATAL(!missileDataTable,("Missile data table could not be opened."));
		int const numRows = missileDataTable->getNumRows();
		for (int row=0; row<numRows; ++row)
		{
			MissileTypeDataRecord data;
			data.m_typeId = missileDataTable->getIntValue("missile_type_id",row);
			data.m_speed = missileDataTable->getFloatValue("client_speed",row);
			data.m_targetAcquisitionSeconds = missileDataTable->getFloatValue("target_acquisition_seconds",row);
			data.m_targetAcquisitionAngle = missileDataTable->getFloatValue("target_acquisition_angle",row);
			data.m_time = missileDataTable->getFloatValue("max_time", row);
			data.m_missileAppearanceName = missileDataTable->getStringValue("appearance",row);
			data.m_trailAppearanceName = missileDataTable->getStringValue("trail_appearance",row);
			data.m_fireEffectName = missileDataTable->getStringValue("fire_effect",row);
			data.m_hitEffectName = missileDataTable->getStringValue("hit_effect",row);
			data.m_countermeasureEffectName = missileDataTable->getStringValue("countermeasure_effect",row);

			(*ms_missileTypeData)[data.m_typeId] = data;
		}
		DataTableManager::close("datatables/space/missiles.iff");
	}

	// Get countermeasure appearances

	{
		DataTable const * const countermeasureDataTable = DataTableManager::getTable("datatables/space/countermeasures.iff", true);
		int const numRows = countermeasureDataTable->getNumRows();
		for (int row=0; row<numRows; ++row)
		{
			CountermeasureTypeDataRecord data;
			data.m_typeId = countermeasureDataTable->getIntValue("countermeasure_type_id", row);
			data.m_appearanceName = countermeasureDataTable->getStringValue("appearance",row);
			data.m_minNumber = countermeasureDataTable->getIntValue("min_number",row);
			data.m_maxNumber = countermeasureDataTable->getIntValue("max_number",row);
			data.m_minSpeed = countermeasureDataTable->getFloatValue("min_speed",row);
			data.m_maxSpeed = countermeasureDataTable->getFloatValue("max_speed",row);
			data.m_maxScatterAngle = countermeasureDataTable->getFloatValue("scatter_angle_degrees", row) * (PI_TIMES_2 / 360.0f);
			IGNORE_RETURN(ms_countermeasureTypeData->insert(std::make_pair(data.m_typeId, data)));
		}
		DataTableManager::close("datatables/space/countermeasures.iff");
	}
}

// ----------------------------------------------------------------------

void MissileManager::remove()
{
	DEBUG_FATAL(!ms_installed,("MissileManager not installed"));
	delete ms_missiles;
	ms_missiles = NULL;

	{
		for (MissileManager::MissileTypeDataType::iterator it = ms_missileTypeData->begin(); it != ms_missileTypeData->end(); ++it)
		{
			MissileTypeDataRecord & missileTypeDataRecord = (*it).second;
			missileTypeDataRecord.releaseResources();
		}
	}

	{
		for (MissileManager::CountermeasureTypeDataType::iterator it = ms_countermeasureTypeData->begin(); it != ms_countermeasureTypeData->end(); ++it)
		{
			CountermeasureTypeDataRecord & record = (*it).second;
			record.releaseResources();
		}
	}

	delete ms_missileTypeData;
	ms_missileTypeData=NULL;
	delete ms_countermeasures;
	ms_countermeasures=NULL;
	delete ms_countermeasureTypeData;
	ms_countermeasureTypeData=NULL;

	ms_installed = false;
}

// ----------------------------------------------------------------------

void MissileManager::addMissile(int const missileId, NetworkId const &source, NetworkId const &target, Vector const & sourceLocation, Vector const & targetLocation, int const impactTime, int const missileTypeId, int const weaponIndex, int const targetComponent)
{
	DEBUG_FATAL(!ms_installed,("MissileManager not installed"));

	Object * const targetObject = NetworkIdManager::getObjectById(target);
	TangibleObject * const targetTangibleObject = (targetObject != NULL) ? targetObject->asClientObject()->asTangibleObject() : NULL;
	Object * const sourceObject = NetworkIdManager::getObjectById(source);
	ShipObject * const sourceShipObject = (sourceObject != NULL) ? sourceObject->asClientObject()->asShipObject() : NULL;

	Transform clientSourceLocation;
	clientSourceLocation.setPosition_p(sourceLocation);
	bool foundLauncher = false;
	
	if (sourceObject)
	{
		if (sourceShipObject)
		{
			// use the client's idea of where the source object is, if we can find the object
			ShipObjectAttachments::TransformObjectList transformObjectList;

			ShipChassisSlotType::Type const shipChassisSlotType = static_cast<ShipChassisSlotType::Type>(static_cast<int>(ShipChassisSlotType::SCST_weapon_0) + weaponIndex);
			if (!sourceShipObject->getShipObjectAttachments().findAttachmentHardpoints_o(shipChassisSlotType, s_missileHardpointName, Transform::identity, transformObjectList))
				WARNING (true, ("MissileManager::addMissile tried to create missile for weapon [%d] which doesnt exist", weaponIndex));

			//-- if no points returned, just get all muzzles for the entire ship
			if (transformObjectList.empty())
				IGNORE_RETURN(sourceShipObject->getShipObjectAttachments().findAttachmentHardpoints_o(-1, s_missileHardpointName, Transform::identity, transformObjectList));

			if (transformObjectList.size() > 1)
			{
				size_t launcherIndex = static_cast<size_t>(sourceShipObject->getNextMissileLauncher());
				if (launcherIndex >= transformObjectList.size())
					launcherIndex = 0;
				clientSourceLocation = transformObjectList[launcherIndex].first;
				sourceShipObject->setNextMissileLauncher(static_cast<int>(++launcherIndex));
				foundLauncher = true;
			}
			else if (transformObjectList.size() == 1)
			{
				clientSourceLocation = transformObjectList[0].first;
				foundLauncher = true;
			}
		}

		if (!foundLauncher)
			clientSourceLocation = sourceObject->getTransform_o2w();
	}

	// get the information about this type of missile
	MissileTypeDataType::iterator const i=ms_missileTypeData->find(missileTypeId);
	WARNING(i==ms_missileTypeData->end(),("MissileManager::addMissile:  Received request to add missile with type id %i (launched from object %s), which isn't in the data table.",missileTypeId, source.getValueString().c_str()));
	if (i==ms_missileTypeData->end())
		return;
	MissileTypeDataRecord & data = i->second;
	data.preloadResources();

	// Create missile
	Missile * const missile = new Missile(missileId, missileTypeId, targetObject, source, clientSourceLocation,
		targetLocation, impactTime, data.m_speed, data.m_missileAppearance, data.m_trailAppearance, data.m_fireEffect, data.m_hitEffect, data.m_countermeasureEffect, foundLauncher, targetComponent);
	(*ms_missiles)[missileId] = missile;

	//-- Start combat music if the player is getting shot at
	if (targetTangibleObject == Game::getPlayerContainingShip())
	{
		if (   (sourceShipObject != NULL)
			&& sourceShipObject->isEnemy())
		{
			GameMusicManager::startCombatMusic(sourceShipObject->getNetworkId());
		}
	}

	//-- Start combat music if the player is shooting at someone
	if (sourceShipObject == Game::getPlayerContainingShip())
	{
		if (   (targetTangibleObject != NULL)
			&& targetTangibleObject->isEnemy())
		{
			GameMusicManager::startCombatMusic(targetTangibleObject->getNetworkId());
		}
	}

	// set up notifications
	RenderWorld::addObjectNotifications(*missile);
	missile->addNotification(ClientWorld::getIntangibleNotification());

	missile->addToWorld();
}

// ----------------------------------------------------------------------

void MissileManager::removeMissile(int missileId)
{
	DEBUG_FATAL(!ms_installed,("MissileManager not installed"));

	IGNORE_RETURN(ms_missiles->erase(missileId));
}

// ----------------------------------------------------------------------

void MissileManager::onServerMissileHitTarget(int missileId)
{
	DEBUG_FATAL(!ms_installed,("MissileManager not installed"));

	Missile * const missile = getMissile(missileId);
	if (missile)
		missile->hitTarget();
}

// ----------------------------------------------------------------------

void MissileManager::onServerMissileMissedTarget(int missileId)
{
	DEBUG_FATAL(!ms_installed,("MissileManager not installed"));

	Missile * const missile = getMissile(missileId);
	if (missile)
		missile->missTarget();
}

// ----------------------------------------------------------------------

void MissileManager::onServerMissileCountermeasured(int const missileId, int const countermeasureVisualType)
{
	DEBUG_FATAL(!ms_installed,("MissileManager not installed"));

	Missile * const missile = getMissile(missileId);
	if (missile)
	{
		Object const * const targetShip = missile->getTargetObject();
		if (targetShip)
		{
			Countermeasure * newCountermeasure = launchCountermeasure(*targetShip, countermeasureVisualType);
			missile->countermeasure(newCountermeasure);
		}
		else
			missile->countermeasure();
	}
}

// ----------------------------------------------------------------------

void MissileManager::onServerMissileCountermeasureFailed(NetworkId const & shipId, int const countermeasureVisualType)
{
	DEBUG_FATAL(!ms_installed,("MissileManager not installed"));

	Object * const sourceObject = NetworkIdManager::getObjectById(shipId);
	if (sourceObject)
	{
		IGNORE_RETURN(launchCountermeasure(*sourceObject, countermeasureVisualType));
	}
}

// ----------------------------------------------------------------------

Countermeasure * MissileManager::launchCountermeasure(Object const & sourceObject, int const countermeasureType)
{
	DEBUG_FATAL(!ms_installed,("MissileManager not installed"));

	if (ms_countermeasures->size() > ms_maxCountermeasures)
	{
		delete (*ms_countermeasures)[0];
	}

	CountermeasureTypeDataType::iterator typeData = ms_countermeasureTypeData->find(countermeasureType);
	if (typeData == ms_countermeasureTypeData->end())
	{
		DEBUG_WARNING(true,("The server specified countermeasure type # %i, which was not in the data table",countermeasureType));
		return 0;
	}

	CountermeasureTypeDataRecord & record = typeData->second;
	record.preloadResources();

	int const numberToCreate = Random::random(record.m_minNumber, record.m_maxNumber);
	Countermeasure * newCountermeasure = NULL;
	for (int i=0; i<numberToCreate; ++i)
	{
		newCountermeasure = new Countermeasure(sourceObject.getAppearanceSphereCenter_w() + sourceObject.getObjectFrameK_w() * -1.0f * sourceObject.getAppearanceSphereRadius() , sourceObject.getObjectFrameK_w() * -1.0f, record.m_appearance, Random::randomReal(record.m_minSpeed, record.m_maxSpeed), record.m_maxScatterAngle);

		newCountermeasure->addToWorld();
	}

	// return just one countermeasure object, which will be the one missiles home in on.
	// The other countermeasures in the group are just for visual effect
	return newCountermeasure;
}

// ----------------------------------------------------------------------

Missile * MissileManager::getMissile(int missileId)
{
	DEBUG_FATAL(!ms_installed,("MissileManager not installed"));

	MissilesType::const_iterator const i=ms_missiles->find(missileId);
	if (i!=ms_missiles->end())
		return i->second;
	else
		return NULL;
}

// ----------------------------------------------------------------------

void MissileManager::addCountermeasure(Countermeasure * const countermeasure)
{
	ms_countermeasures->push_back(countermeasure);
}

// ----------------------------------------------------------------------

void MissileManager::removeCountermeasure(Countermeasure const * const countermeasure)
{
	DEBUG_FATAL(!ms_installed,("MissileManager not installed"));

	IGNORE_RETURN(ms_countermeasures->erase(std::remove(ms_countermeasures->begin(), ms_countermeasures->end(), countermeasure), ms_countermeasures->end()));
}

// ----------------------------------------------------------------------

float MissileManager::getTargetAcquisitionSeconds(int const missileTypeId)
{
	float timeSeconds = cs_defaultTargetAcquisitionTime;

	if(missileTypeId >= 0 && missileTypeId < ms_totalLoadedAquisitions)
		return ms_clientAquisitionTimes[missileTypeId];

	//-- Get the information about this type of missile
	MissileTypeDataType::const_iterator const i = ms_missileTypeData->find(missileTypeId);
	if (i == ms_missileTypeData->end())
	{
		WARNING(true,("MissileManager::getTargetAcquisitionSeconds:  Received request with type id %i, which isn't in the data table.", missileTypeId));
	}
	else
	{
		MissileTypeDataRecord const & data = i->second;
		timeSeconds = data.m_targetAcquisitionSeconds;
	}

	return timeSeconds;
}

// ----------------------------------------------------------------------

float MissileManager::getTargetAcquisitionAngle(int const missileTypeId)
{
	float angleDegrees = cs_defaultTargetAcquisitionAngle;

	//-- Get the information about this type of missile.
	MissileTypeDataType::const_iterator const i = ms_missileTypeData->find(missileTypeId);
	if (i == ms_missileTypeData->end())
	{
		WARNING(true,("MissileManager::getTargetAcquisitionData:  Received request with type id %i, which isn't in the data table.", missileTypeId));
	}
	else
	{
		MissileTypeDataRecord const & data = i->second;
		angleDegrees = data.m_targetAcquisitionAngle;
	}

	return angleDegrees;
}

// ----------------------------------------------------------------------

float MissileManager::getDefaultTargetAcquisitionSeconds()
{
	return cs_defaultTargetAcquisitionTime;
}

// ----------------------------------------------------------------------

float MissileManager::getDefaultTargetAcquisitionAngle()
{
	return cs_defaultTargetAcquisitionAngle;
}

// ----------------------------------------------------------------------

float MissileManager::getTime(int missileTypeId)
{
	float missileTime = 0.0f;
	
	//-- Get the information about this type of missile.
	MissileTypeDataType::const_iterator const i = ms_missileTypeData->find(missileTypeId);
	if (i == ms_missileTypeData->end())
	{
		WARNING(true,("MissileManager::getTime:  Received request with type id %i, which isn't in the data table.", missileTypeId));
	}
	else
	{
		MissileTypeDataRecord const & data = i->second;
		missileTime = data.m_time;
	}
	
	return missileTime;
}

// ----------------------------------------------------------------------

float MissileManager::getRange(int missileTypeId)
{
	float missileRange = 0.0f;
	
	//-- Get the information about this type of missile.
	MissileTypeDataType::const_iterator const i = ms_missileTypeData->find(missileTypeId);
	if (i == ms_missileTypeData->end())
	{
		WARNING(true,("MissileManager::getRange:  Received request with type id %i, which isn't in the data table.", missileTypeId));
	}
	else
	{
		MissileTypeDataRecord const & data = i->second;
		missileRange = data.m_time * data.m_speed;
	}
	
	return missileRange;
}



// ----------------------------------------------------------------------

float MissileManager::getSpeed(int missileTypeId)
{
	float missileSpeed = 0.0f;
	
	//-- Get the information about this type of missile.
	MissileTypeDataType::const_iterator const i = ms_missileTypeData->find(missileTypeId);
	if (i == ms_missileTypeData->end())
	{
		WARNING(true,("MissileManager::getSpeed:  Received request with type id %i, which isn't in the data table.", missileTypeId));
	}
	else
	{
		MissileTypeDataRecord const & data = i->second;
		missileSpeed = data.m_speed;
	}
	
	return missileSpeed;
}



// ======================================================================
