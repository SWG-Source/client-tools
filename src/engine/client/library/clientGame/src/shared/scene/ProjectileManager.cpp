// ======================================================================
//
// ProjectileManager.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ProjectileManager.h"

#include "clientGame/ClientEffect.h"
#include "clientGame/ClientEffectTemplate.h"
#include "clientGame/ClientEffectTemplateList.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientGame/ProjectileBeam.h"
#include "clientGame/ShipObject.h"
#include "clientGame/ShipObjectAttachments.h"
#include "clientGame/ShipProjectile.h"
#include "clientGraphics/RenderWorld.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedGame/ShipChassisSlotType.h"
#include "sharedGame/ShipComponentWeaponManager.h"
#include "sharedMathArchive/TransformArchive.h"
#include "sharedNetworkMessages/CreateProjectileMessage.h"
#include "sharedNetworkMessages/MessageQueueGeneric.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/Controller.h"
#include "sharedObject/Hardpoint.h"
#include "sharedUtility/DataTable.h"
#include <algorithm>
#include <map>
#include <vector>

// ======================================================================

namespace ProjectileManagerNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	enum ColumnData
	{
		CD_index,
		CD_appearanceTemplateName,
		CD_fireClientEffectTemplateName,
		CD_hitOtherClientEffectTemplateName,
		CD_hitMetalClientEffectTemplateName,
		CD_hitStoneClientEffectTemplateName,
		CD_hitAcidClientEffectTemplateName,
		CD_hitIceClientEffectTemplateName,
		CD_hitMoltenClientEffectTemplateName,
		CD_hitObsidianClientEffectTemplateName,
		CD_hitShieldClientEffectTemplateName,
		CD_hitShieldShellClientEffectTemplateName,
		CD_hitFriendlyFireClientEffectTemplateName,
		CD_numLightningBolts
	};

	struct ProjectileData
	{
	public:

		ProjectileData();
		~ProjectileData();

		void preload();

		ShipProjectile::MissHitData const createMissHitData() const;

	public:

		int m_index;
		std::string m_appearanceTemplateName;
		std::string m_fireClientEffectTemplateName;
		std::string m_hitOtherClientEffectTemplateName;
		std::string m_hitMetalClientEffectTemplateName;
		std::string m_hitStoneClientEffectTemplateName;
		std::string m_hitAcidClientEffectTemplateName;
		std::string m_hitIceClientEffectTemplateName;
		std::string m_hitMoltenClientEffectTemplateName;
		std::string m_hitObsidianClientEffectTemplateName;
		std::string m_hitShieldClientEffectTemplateName;
		std::string m_hitShieldShellClientEffectTemplateName;
		std::string m_hitFriendlyFireClientEffectTemplateName;
		int m_numLightningBolts;

	public:

		AppearanceTemplate const * m_appearanceTemplate;
		ClientEffectTemplate const * m_fireClientEffectTemplate;
		ClientEffectTemplate const * m_hitOtherClientEffectTemplate;
		ClientEffectTemplate const * m_hitMetalClientEffectTemplate;
		ClientEffectTemplate const * m_hitStoneClientEffectTemplate;
		ClientEffectTemplate const * m_hitAcidClientEffectTemplate;
		ClientEffectTemplate const * m_hitIceClientEffectTemplate;
		ClientEffectTemplate const * m_hitMoltenClientEffectTemplate;
		ClientEffectTemplate const * m_hitObsidianClientEffectTemplate;
		ClientEffectTemplate const * m_hitShieldClientEffectTemplate;
		ClientEffectTemplate const * m_hitShieldShellClientEffectTemplate;
		ClientEffectTemplate const * m_hitFriendlyFireClientEffectTemplate;

	private:

		bool m_preloaded;
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	char const * const cs_projectileFileName = "datatables/projectile/projectile.iff";

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef ShipObjectAttachments::TransformObjectList TransformObjectList;
	typedef std::map<int, ProjectileData *> ProjectileDataMap;
	ProjectileDataMap ms_projectileDataMap;

	ProjectileData * ms_defaultProjectileData;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void remove();
	void createProjectile(ShipObject * ownerShip, int weaponIndex, ProjectileData const * projectileData, Transform const & transform_p, Object * attachToObject, float speed, float expireTime, bool beamWasActive);
	void findMuzzleHardpoints_o(Object * const object, Transform const & parentTransform_w, TransformObjectList & transformObjectList);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	static std::string const s_muzzleHardpointName = "muzzle";

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef Watcher<ProjectileBeam> ProjectileBeamWatcher;
	typedef stdvector<ProjectileBeamWatcher>::fwd ProjectileBeamVector;
	typedef std::pair<NetworkId /* ship id */, int /*weaponIndex */> ShipWeaponPair;

	typedef stdmap<ShipWeaponPair, ProjectileBeamVector>::fwd ProjectileBeamMap;

	ProjectileBeamMap s_projectileBeamMap;

	//----------------------------------------------------------------------

	bool hasBeamsForWeapon(ShipObject & ownerShip, int weaponIndex)
	{
		return s_projectileBeamMap.find(ShipWeaponPair(ownerShip.getNetworkId(), weaponIndex)) != s_projectileBeamMap.end();
	}

}

using namespace ProjectileManagerNamespace;

// ======================================================================
// PUBLIC ProjectileManager::ProjectileData
// ======================================================================

ProjectileData::ProjectileData() :
	m_index(0),
	m_appearanceTemplateName(),
	m_fireClientEffectTemplateName(),
	m_hitOtherClientEffectTemplateName(),
	m_hitMetalClientEffectTemplateName(),
	m_hitStoneClientEffectTemplateName(),
	m_hitAcidClientEffectTemplateName(),
	m_hitIceClientEffectTemplateName(),
	m_hitMoltenClientEffectTemplateName(),
	m_hitObsidianClientEffectTemplateName(),
	m_hitShieldClientEffectTemplateName(),
	m_hitShieldShellClientEffectTemplateName(),
	m_hitFriendlyFireClientEffectTemplateName(),
	m_appearanceTemplate(0),
	m_fireClientEffectTemplate(0),
	m_hitOtherClientEffectTemplate(0),
	m_hitMetalClientEffectTemplate(0),
	m_hitStoneClientEffectTemplate(0),
	m_hitAcidClientEffectTemplate(0),
	m_hitIceClientEffectTemplate(0),
	m_hitMoltenClientEffectTemplate(0),
	m_hitObsidianClientEffectTemplate(0),
	m_hitShieldClientEffectTemplate(0),
	m_hitShieldShellClientEffectTemplate(0),
	m_hitFriendlyFireClientEffectTemplate(0),
	m_preloaded(false)
{
}

// ----------------------------------------------------------------------

ProjectileData::~ProjectileData()
{
	if (m_appearanceTemplate)
	{
		AppearanceTemplateList::release(m_appearanceTemplate);
		m_appearanceTemplate = 0;
	}

	if (m_fireClientEffectTemplate)
	{
		m_fireClientEffectTemplate->release();
		m_fireClientEffectTemplate = 0;
	}

	if (m_hitOtherClientEffectTemplate)
	{
		m_hitOtherClientEffectTemplate->release();
		m_hitOtherClientEffectTemplate = 0;
	}

	if (m_hitMetalClientEffectTemplate)
	{
		m_hitMetalClientEffectTemplate->release();
		m_hitMetalClientEffectTemplate = 0;
	}

	if (m_hitStoneClientEffectTemplate)
	{
		m_hitStoneClientEffectTemplate->release();
		m_hitStoneClientEffectTemplate = 0;
	}

	if (m_hitAcidClientEffectTemplate)
	{
		m_hitAcidClientEffectTemplate->release();
		m_hitAcidClientEffectTemplate = 0;
	}

	if (m_hitIceClientEffectTemplate)
	{
		m_hitIceClientEffectTemplate->release();
		m_hitIceClientEffectTemplate = 0;
	}

	if (m_hitMoltenClientEffectTemplate)
	{
		m_hitMoltenClientEffectTemplate->release();
		m_hitMoltenClientEffectTemplate = 0;
	}

	if (m_hitObsidianClientEffectTemplate)
	{
		m_hitObsidianClientEffectTemplate->release();
		m_hitObsidianClientEffectTemplate = 0;
	}

	if (m_hitShieldClientEffectTemplate)
	{
		m_hitShieldClientEffectTemplate->release();
		m_hitShieldClientEffectTemplate = 0;
	}

	if (m_hitShieldShellClientEffectTemplate)
	{
		m_hitShieldShellClientEffectTemplate->release();
		m_hitShieldShellClientEffectTemplate = 0;
	}

	if (m_hitFriendlyFireClientEffectTemplate)
	{
		m_hitFriendlyFireClientEffectTemplate->release();
		m_hitFriendlyFireClientEffectTemplate = 0;
	}
}

// ----------------------------------------------------------------------

void ProjectileData::preload()
{
	if (m_preloaded)
		return;

	m_preloaded = true;

	if (!m_appearanceTemplateName.empty())
		m_appearanceTemplate = AppearanceTemplateList::fetch(m_appearanceTemplateName.c_str());

	if (!m_fireClientEffectTemplateName.empty())
	{
		m_fireClientEffectTemplate = ClientEffectTemplateList::fetch(CrcLowerString(m_fireClientEffectTemplateName.c_str()));
		WARNING(m_fireClientEffectTemplate == NULL, ("ProjectileManager::preload for [%d] failed to load FIRE CEF [%s]", m_index, m_fireClientEffectTemplateName.c_str()));
	}

	if (!m_hitOtherClientEffectTemplateName.empty())
	{
		m_hitOtherClientEffectTemplate = ClientEffectTemplateList::fetch(CrcLowerString(m_hitOtherClientEffectTemplateName.c_str()));
		WARNING(m_hitOtherClientEffectTemplate == NULL, ("ProjectileManager::preload for [%d] failed to load HIT OTHER CEF [%s]", m_index, m_hitOtherClientEffectTemplateName.c_str()));
	}

	if (!m_hitMetalClientEffectTemplateName.empty())
	{
		m_hitMetalClientEffectTemplate = ClientEffectTemplateList::fetch(CrcLowerString(m_hitMetalClientEffectTemplateName.c_str()));
		WARNING(m_hitMetalClientEffectTemplate == NULL, ("ProjectileManager::preload for [%d] failed to load HIT METAL CEF [%s]", m_index, m_hitMetalClientEffectTemplateName.c_str()));
	}

	if (!m_hitStoneClientEffectTemplateName.empty())
	{
		m_hitStoneClientEffectTemplate = ClientEffectTemplateList::fetch(CrcLowerString(m_hitStoneClientEffectTemplateName.c_str()));
		WARNING(m_hitStoneClientEffectTemplate == NULL, ("ProjectileManager::preload for [%d] failed to load HIT STONE CEF [%s]", m_index, m_hitStoneClientEffectTemplateName.c_str()));
	}

	if (!m_hitAcidClientEffectTemplateName.empty())
	{
		m_hitAcidClientEffectTemplate = ClientEffectTemplateList::fetch(CrcLowerString(m_hitAcidClientEffectTemplateName.c_str()));
		WARNING(m_hitAcidClientEffectTemplate == NULL, ("ProjectileManager::preload for [%d] failed to load HIT ACID CEF [%s]", m_index, m_hitAcidClientEffectTemplateName.c_str()));
	}

	if (!m_hitIceClientEffectTemplateName.empty())
	{
		m_hitIceClientEffectTemplate = ClientEffectTemplateList::fetch(CrcLowerString(m_hitIceClientEffectTemplateName.c_str()));
		WARNING(m_hitIceClientEffectTemplate == NULL, ("ProjectileManager::preload for [%d] failed to load HIT ICE CEF [%s]", m_index, m_hitIceClientEffectTemplateName.c_str()));
	}

	if (!m_hitMoltenClientEffectTemplateName.empty())
	{
		m_hitMoltenClientEffectTemplate = ClientEffectTemplateList::fetch(CrcLowerString(m_hitMoltenClientEffectTemplateName.c_str()));
		WARNING(m_hitMoltenClientEffectTemplate == NULL, ("ProjectileManager::preload for [%d] failed to load HIT MOLTEN CEF [%s]", m_index, m_hitMoltenClientEffectTemplateName.c_str()));
	}

	if (!m_hitObsidianClientEffectTemplateName.empty())
	{
		m_hitObsidianClientEffectTemplate = ClientEffectTemplateList::fetch(CrcLowerString(m_hitObsidianClientEffectTemplateName.c_str()));
		WARNING(m_hitObsidianClientEffectTemplate == NULL, ("ProjectileManager::preload for [%d] failed to load HIT OBSIDIAN CEF [%s]", m_index, m_hitObsidianClientEffectTemplateName.c_str()));
	}

	if (!m_hitShieldClientEffectTemplateName.empty())
	{
		m_hitShieldClientEffectTemplate = ClientEffectTemplateList::fetch(CrcLowerString(m_hitShieldClientEffectTemplateName.c_str()));
		WARNING(m_hitShieldClientEffectTemplate == NULL, ("ProjectileManager::preload for [%d] failed to load HIT SHIELD CEF [%s]", m_index, m_hitShieldClientEffectTemplateName.c_str()));
	}

	if (!m_hitShieldShellClientEffectTemplateName.empty())
	{
		m_hitShieldShellClientEffectTemplate = ClientEffectTemplateList::fetch(CrcLowerString(m_hitShieldShellClientEffectTemplateName.c_str()));
		WARNING(m_hitShieldShellClientEffectTemplate == NULL, ("ProjectileManager::preload for [%d] failed to load HIT SHIELD SHELL CEF [%s]", m_index, m_hitShieldShellClientEffectTemplateName.c_str()));
	}

	if (!m_hitFriendlyFireClientEffectTemplateName.empty())
	{
		m_hitFriendlyFireClientEffectTemplate = ClientEffectTemplateList::fetch(CrcLowerString(m_hitFriendlyFireClientEffectTemplateName.c_str()));
		WARNING(m_hitFriendlyFireClientEffectTemplate == NULL, ("ProjectileManager::preload for [%d] failed to load HIT FRIENDLY FIRE CEF [%s]", m_index, m_hitFriendlyFireClientEffectTemplateName.c_str()));
	}
}

// ----------------------------------------------------------------------

ShipProjectile::MissHitData const ProjectileData::createMissHitData() const
{
	ShipProjectile::MissHitData data;
	data.m_missHitOtherClientEffectTemplate = m_hitOtherClientEffectTemplate;
	data.m_missHitMetalClientEffectTemplate = m_hitMetalClientEffectTemplate;
	data.m_missHitStoneClientEffectTemplate = m_hitStoneClientEffectTemplate;
	data.m_missHitAcidClientEffectTemplate = m_hitAcidClientEffectTemplate;
	data.m_missHitIceClientEffectTemplate = m_hitIceClientEffectTemplate;
	data.m_missHitMoltenClientEffectTemplate = m_hitMoltenClientEffectTemplate;
	data.m_missHitObsidianClientEffectTemplate = m_hitObsidianClientEffectTemplate;
	data.m_missHitShieldClientEffectTemplate = m_hitShieldClientEffectTemplate;
	data.m_missHitShieldShellClientEffectTemplate = m_hitShieldShellClientEffectTemplate;
	data.m_missHitFriendlyFireClientEffectTemplate = m_hitFriendlyFireClientEffectTemplate;

	return data;
}

// ======================================================================
// PUBLIC ProjectileManagerNamespace
// ======================================================================

void ProjectileManagerNamespace::remove()
{
	std::for_each(ms_projectileDataMap.begin(), ms_projectileDataMap.end(), PointerDeleterPairSecond());
	ms_projectileDataMap.clear();
}

//----------------------------------------------------------------------

bool ProjectileManager::stopBeam(Object & object, int weaponIndex, bool notifyServer)
{
	bool found = false;
	ShipWeaponPair const shipWeaponPair(object.getNetworkId(), weaponIndex);
	ProjectileBeamMap::iterator it = s_projectileBeamMap.find(shipWeaponPair);
	if (it == s_projectileBeamMap.end())
	{
		WARNING(notifyServer, ("ProjectileManager::stopBeam [%s] [%d] attempt to stop non-existent beam", 
			object.getNetworkId().getValueString().c_str(), weaponIndex));
	}
	else
	{
		ProjectileBeamVector & pbv = (*it).second;
		
		{
			for (ProjectileBeamVector::iterator it = pbv.begin(); it != pbv.end(); ++it)
			{
				ProjectileBeamWatcher & projectileBeamWatcher = *it;
				if (projectileBeamWatcher.getPointer())
					projectileBeamWatcher->kill();

				found = true;
			}
		}
		
		s_projectileBeamMap.erase(it);
	}

	if (notifyServer)
	{
		typedef MessageQueueGenericValueType<int> MessageType;
		
		MessageType * const msg = new MessageType(weaponIndex);
		object.getController()->appendMessage(static_cast<int>(CM_shipStopFiring), 0.f, msg, 
		GameControllerMessageFlags::SEND | GameControllerMessageFlags::DEST_SERVER | GameControllerMessageFlags::RELIABLE);
	}

	return found;
}

// ----------------------------------------------------------------------

void ProjectileManagerNamespace::createProjectile(ShipObject *ownerShip, int weaponIndex, ProjectileData const * const projectileData, Transform const & transform_p, Object * const attachToObject, float const speed, float const expireTime, bool beamWasActive)
{
	NOT_NULL(projectileData);

	UNREF(attachToObject);

	if (ownerShip->isBeamWeapon(weaponIndex))
	{
		ProjectileBeam * const projectileBeam = new ProjectileBeam(ownerShip, weaponIndex, speed * expireTime, projectileData->m_appearanceTemplate, projectileData->m_numLightningBolts, projectileData->createMissHitData());

		RenderWorld::addObjectNotifications (*projectileBeam);

		if (NULL != attachToObject)
			attachToObject->addChildObject_o(projectileBeam);
		else
			ownerShip->addChildObject_o(projectileBeam);

		Transform const & transform_o2p = projectileBeam->getParent()->getTransform_o2w().rotateTranslate_p2l(transform_p);
		projectileBeam->setTransform_o2p(transform_o2p);
		ShipWeaponPair const shipWeaponPair(ownerShip->getNetworkId(), weaponIndex);
		s_projectileBeamMap[shipWeaponPair].push_back(ProjectileBeamWatcher(projectileBeam));
	}
	else
	{
		ShipProjectile * const projectile = new ShipProjectile(projectileData->m_appearanceTemplate, projectileData->createMissHitData());
		projectile->setExpirationTime(expireTime);
		projectile->setTransform_o2p(transform_p);
		projectile->setSpeed(speed);
		projectile->setOwnerShip(ownerShip);
		projectile->setTravelDistance(speed * expireTime);
		projectile->setWeaponIndex(weaponIndex);
		projectile->addToWorld();
	}

	//-- Rather than pass the fireClientEffectTemplate to the projectile, we are creating it here to be able to attach the sound to the owner
	if (!beamWasActive && projectileData->m_fireClientEffectTemplate && ownerShip)
	{
		Transform const & muzzleTransform_o = ownerShip->getTransform_o2w().rotateTranslate_p2l(transform_p);
		ClientEffect * const clientEffect = projectileData->m_fireClientEffectTemplate->createClientEffect(ownerShip, muzzleTransform_o);
		clientEffect->execute();
		delete clientEffect;
	}
}

//----------------------------------------------------------------------

void ProjectileManagerNamespace::findMuzzleHardpoints_o(Object * const object, Transform const & parentTransform_w, TransformObjectList & transformObjectList)
{
	//-- Finds all hardpoints that start with "muzzle" in object space for an object
	Appearance const * const appearance = object->getAppearance();
	if (appearance)
	{
		AppearanceTemplate const * const appearanceTemplate = appearance->getAppearanceTemplate();
		if (appearanceTemplate)
		{
			for (int i = 0; i < appearanceTemplate->getHardpointCount(); ++i)
			{
				Hardpoint const & hardpoint = appearanceTemplate->getHardpoint(i);
				if (strncmp(hardpoint.getName().getString(), "muzzle", 6) == 0)
				{
					Transform const & t = parentTransform_w.rotateTranslate_p2l(object->getTransform_o2w().rotateTranslate_l2p(hardpoint.getTransform()));
					transformObjectList.push_back(ShipObjectAttachments::TransformObjectPair(t, object));
				}
			}
		}
	}

	//-- Recurse to children
	for (int i = 0; i < object->getNumberOfChildObjects(); ++i)
	{
		Object * const childObject = object->getChildObject(i);
		if (childObject)
			findMuzzleHardpoints_o(childObject, parentTransform_w, transformObjectList);
	}
}

// ======================================================================
// STATIC PUBLIC ProjectileManager
// ======================================================================

void ProjectileManager::install()
{
	InstallTimer const installTimer("ProjectileManager::install");

	DEBUG_FATAL(!ms_projectileDataMap.empty(), ("ProjectileManager::install - already installed"));
	ExitChain::add(ProjectileManagerNamespace::remove, "ProjectileManagerNamespace::remove");

	//-- Load the projectile data table
	{
		ms_projectileDataMap.clear();

		Iff iff;
		if (iff.open(cs_projectileFileName, true))
		{
			DataTable dataTable;
			dataTable.load(iff);

			int const numberOfRows = dataTable.getNumRows();
			for (int row = 0; row < numberOfRows; ++row)
			{
				int const index = dataTable.getIntValue(static_cast<int>(CD_index), row);
				if (ms_projectileDataMap.find(index) == ms_projectileDataMap.end())
				{
					//-- Extract data from datatable
					ProjectileData * const projectileData = new ProjectileData;
					projectileData->m_index = index;
					projectileData->m_appearanceTemplateName = dataTable.getStringValue(static_cast<int>(CD_appearanceTemplateName), row);
					projectileData->m_fireClientEffectTemplateName = dataTable.getStringValue(static_cast<int>(CD_fireClientEffectTemplateName), row);
					projectileData->m_hitOtherClientEffectTemplateName = dataTable.getStringValue(static_cast<int>(CD_hitOtherClientEffectTemplateName), row);
					projectileData->m_hitMetalClientEffectTemplateName = dataTable.getStringValue(static_cast<int>(CD_hitMetalClientEffectTemplateName), row);
					projectileData->m_hitStoneClientEffectTemplateName = dataTable.getStringValue(static_cast<int>(CD_hitStoneClientEffectTemplateName), row);
					projectileData->m_hitAcidClientEffectTemplateName = dataTable.getStringValue(static_cast<int>(CD_hitAcidClientEffectTemplateName), row);
					projectileData->m_hitIceClientEffectTemplateName = dataTable.getStringValue(static_cast<int>(CD_hitIceClientEffectTemplateName), row);
					projectileData->m_hitMoltenClientEffectTemplateName = dataTable.getStringValue(static_cast<int>(CD_hitMoltenClientEffectTemplateName), row);
					projectileData->m_hitObsidianClientEffectTemplateName = dataTable.getStringValue(static_cast<int>(CD_hitObsidianClientEffectTemplateName), row);
					projectileData->m_hitShieldClientEffectTemplateName = dataTable.getStringValue(static_cast<int>(CD_hitShieldClientEffectTemplateName), row);
					projectileData->m_hitShieldShellClientEffectTemplateName = dataTable.getStringValue(static_cast<int>(CD_hitShieldShellClientEffectTemplateName), row);
					projectileData->m_hitFriendlyFireClientEffectTemplateName = dataTable.getStringValue(static_cast<int>(CD_hitFriendlyFireClientEffectTemplateName), row);
					projectileData->m_numLightningBolts = dataTable.getIntValue(static_cast<int>(CD_numLightningBolts), row);

					//-- Add to projectile data map
					std::pair<ProjectileDataMap::iterator, bool> result = ms_projectileDataMap.insert(std::make_pair(index, projectileData));
					DEBUG_FATAL(!result.second, ("ProjectileManager::install: Failed to insert %i", index));

					//-- Moved preloading to where the projectile is created to allow the client to be started faster
				}
				else
					DEBUG_WARNING(true, ("ProjectileManager::install: file %s specifies duplicate index %i", cs_projectileFileName, index));
			}

			DEBUG_FATAL(ms_projectileDataMap.empty(), ("ProjectileManager::install: no projectile data found in file %s", cs_projectileFileName));

			ProjectileDataMap::iterator iter = ms_projectileDataMap.find(0);
			if (iter != ms_projectileDataMap.end())
				ms_defaultProjectileData = iter->second;
			else
				DEBUG_FATAL(true, ("ProjectileManager::install: file %s does not specify projectile type 0", cs_projectileFileName));
		}
		else
			DEBUG_FATAL(true, ("ProjectileManager::install: could not open file %s", cs_projectileFileName));
	}
}

// ----------------------------------------------------------------------

void ProjectileManager::createClientProjectile(Object * const object, int const weaponIndex, int const projectileIndex, float const speed, float const expireTime)
{
	DEBUG_FATAL(expireTime <= 0.f, ("ProjectileManager::createClientProjectile: projectile expireTime <= 0.f"));
	if (!object)
		return;

	//-- Look up projectile data based on projectile index
	ProjectileData * projectileData = ms_defaultProjectileData;
	ProjectileDataMap::iterator iter = ms_projectileDataMap.find(projectileIndex);
	if (iter != ms_projectileDataMap.end())
		projectileData = iter->second;

	//-- Moved preloading to where the projectile is created to allow the client to be started faster
	projectileData->preload();

	Transform const & objectTransform_p = object->getTransform_o2p();

	ClientObject * const clientObject = object->asClientObject();

	if (!clientObject)
		return;

	ShipObject * const shipObject = clientObject->asShipObject();

	if (!shipObject)
		return;

	if (shipObject->hasCondition(TangibleObject::C_docking))
	{
		DEBUG_REPORT_LOG(true, ("ProjectileManager::createClientProjectile() Rejecting projectile due to TangibleObject::C_docking condition.\n"));
		return;
	}

	//-- Find hardpoints to emit projectiles
	TransformObjectList transformObjectList;

	bool isMissile = false;
	bool isCountermeasure = false;

	ShipChassisSlotType::Type const shipChassisSlotType = static_cast<ShipChassisSlotType::Type>(static_cast<int>(ShipChassisSlotType::SCST_weapon_0) + weaponIndex);

	uint32 const componentCrc = shipObject->getComponentCrc(shipChassisSlotType);

	if (ShipComponentWeaponManager::isMissile(componentCrc))
		isMissile = true;
	else if (ShipComponentWeaponManager::isCountermeasure(componentCrc))
		isCountermeasure = true;
	else
	{
		shipObject->onLocalProjectileFired(weaponIndex);
		if (!shipObject->getShipObjectAttachments().findAttachmentHardpoints_o(shipChassisSlotType, s_muzzleHardpointName, object->getTransform_o2w(), transformObjectList))
			WARNING (true, ("ProjectileManager::createClientProjectile tried to create projectile for weapon [%d] which doesnt exist", weaponIndex));

		//-- if no points returned, just get all muzzles for the entire ship
		if (transformObjectList.empty())
			IGNORE_RETURN(shipObject->getShipObjectAttachments().findAttachmentHardpoints_o(-1, s_muzzleHardpointName, object->getTransform_o2w(), transformObjectList));
	}

	Vector serverShotPosition_p;
	//-- Put projectile transforms in parent space, assure we have at least one,
	//-- and determine an appropriate centerpoint for a single projectile
	//-- representing all the shots.
	if (transformObjectList.empty())
	{
		transformObjectList.push_back(ShipObjectAttachments::TransformObjectPair(objectTransform_p, object));
		serverShotPosition_p = objectTransform_p.getPosition_p();
	}
	else
	{
		for (TransformObjectList::iterator it = transformObjectList.begin(); it != transformObjectList.end(); ++it)
		{
			Transform & t = (*it).first;
			t.multiply(objectTransform_p, t);
			serverShotPosition_p += t.getPosition_p();
		}
		serverShotPosition_p /= static_cast<float>(transformObjectList.size());
	}

	//-- don't update existing beams
	if (!shipObject || !shipObject->isBeamWeapon(weaponIndex) || !hasBeamsForWeapon(*shipObject, weaponIndex))
	{
		//-- missiles and countermeasures don't generate projectiles
		if (!(isMissile || isCountermeasure))
		{
			//-- if the transform list is still empty, just use the identity transform
			for (TransformObjectList::const_iterator it = transformObjectList.begin (); it != transformObjectList.end (); ++it)
			{
				Transform const & t = (*it).first;
				Object * const attachToObj = (*it).second;
				ProjectileManagerNamespace::createProjectile(shipObject, weaponIndex, projectileData, t, attachToObj, speed, expireTime, false);
			}
		}
	}

	CreatureObject const * const playerCreature = Game::getPlayerCreature();
	ShipChassisSlotType::Type const slot = playerCreature->getLookAtTargetSlot();

	//-- Send message to the server to create a projectile
	//-- We shoot from the average of muzzle locations being shot from
	//-- at the orientation of the first muzzle
	Transform serverProjectileTransform_p(transformObjectList.front().first);
	serverProjectileTransform_p.setPosition_p(serverShotPosition_p);
	CreateProjectileMessage const createProjectileMessage(0, weaponIndex, projectileIndex, slot, serverProjectileTransform_p, GameNetwork::getServerSyncStampLong());
	GameNetwork::send(createProjectileMessage, true);
}

// ----------------------------------------------------------------------

void ProjectileManager::createServerProjectile(Object * const object, int const weaponIndex, int const projectileIndex, Transform const & transform_p, float const speed, float const expireTime)
{
	DEBUG_FATAL(expireTime <= 0.f, ("ProjectileManager::createServerProjectile: projectile expireTime <= 0.f"));
	if (!object)
		return;

	ClientObject * const clientObject = object->asClientObject();
	if (clientObject == NULL)
		return;
	ShipObject * const shipObject = clientObject->asShipObject();

	Transform const & objectTransform_p = object->getTransform_o2p();

	//-- Look up projectile data based on gun index
	ProjectileData * projectileData = ms_defaultProjectileData;
	ProjectileDataMap::iterator iter = ms_projectileDataMap.find(projectileIndex);
	if (iter != ms_projectileDataMap.end())
		projectileData = iter->second;

	//-- Moved preloading to where the projectile is created to allow the client to be started faster
	projectileData->preload();

	Vector startPosition_p;
	Vector endPosition_p;

	//don't base turret shots off the object location since the transform already accounts for the hardpoint offset
	if (shipObject && shipObject->isTurret(weaponIndex))
	{
		startPosition_p = transform_p.getPosition_p();
		endPosition_p = startPosition_p + (transform_p.getLocalFrameK_p() * speed * expireTime);
	}
	else
	{
		startPosition_p = objectTransform_p.getPosition_p();
		endPosition_p = transform_p.getPosition_p() + (transform_p.getLocalFrameK_p() * speed * expireTime);
	}

	float const travelDistance = (endPosition_p - startPosition_p).magnitude();
	float const projectileSpeed = travelDistance / expireTime;

	//-- Find hardpoints to emit projectiles
	TransformObjectList transformObjectList;

	if (shipObject != NULL)
	{
		shipObject->onRemoteProjectileFired(weaponIndex, endPosition_p);
		if (!shipObject->getShipObjectAttachments().findAttachmentHardpoints_o(static_cast<int>(ShipChassisSlotType::SCST_weapon_0) + weaponIndex, s_muzzleHardpointName, object->getTransform_o2w (), transformObjectList))
			WARNING (true, ("ProjectileManager::createServerProjectile tried to create projectile for weapon [%d] which doesnt exist", weaponIndex));
	}
	else
	{
		findMuzzleHardpoints_o(object, object->getTransform_o2w(), transformObjectList);
	}

	//-- if the transform list is still empty, just use the centerpoint
	if (transformObjectList.empty())
		transformObjectList.push_back(ShipObjectAttachments::TransformObjectPair(Transform::identity, NULL));

	if (shipObject && shipObject->isBeamWeapon(weaponIndex))
		shipObject->setWeaponFiring(weaponIndex);

	//-- don't update existing beams
	if (!shipObject || !shipObject->isBeamWeapon(weaponIndex) || !hasBeamsForWeapon(*shipObject, weaponIndex))
	{
		for (TransformObjectList::const_iterator it = transformObjectList.begin (); it != transformObjectList.end (); ++it)
		{
			Transform projectileTransform_p(objectTransform_p);
			projectileTransform_p.move_l((*it).first.getPosition_p());
			Vector endPosition_o(projectileTransform_p.rotateTranslate_p2l(endPosition_p));
			projectileTransform_p.yaw_l(endPosition_o.theta());
			projectileTransform_p.pitch_l(endPosition_o.phi());
			Object * const attachToObject = (*it).second;
			ProjectileManagerNamespace::createProjectile(shipObject, weaponIndex, projectileData, projectileTransform_p, attachToObject, projectileSpeed, expireTime, false);
		}
	}
}

// ======================================================================

