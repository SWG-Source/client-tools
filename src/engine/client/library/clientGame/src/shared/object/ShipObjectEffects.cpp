//======================================================================
//
// ShipObjectEffects.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ShipObjectEffects.h"

#include "clientAudio/SoundTemplate.h"
#include "clientAudio/SoundTemplateList.h"
#include "clientGame/ClientDataFile.h"
#include "clientGame/ClientDataFile_ContrailData.h"
#include "clientGame/ClientDataFile_InterpolatedSound.h"
#include "clientGame/ClientDataFile_InterpolatedSoundRuntime.h"
#include "clientGame/ClientDataFile_VehicleThrusterSoundData.h"
#include "clientGame/ClientDataFile_VehicleThrusterSoundRuntimeData.h"
#include "clientGame/ClientEffect.h"
#include "clientGame/ClientEffectManager.h"
#include "clientGame/ClientEffectTemplate.h"
#include "clientGame/Game.h"
#include "clientGame/GlowAppearance.h"
#include "clientGame/PlayerShipController.h"
#include "clientGame/RemoteShipController.h"
#include "clientGame/ShipObject.h"
#include "clientGame/ShipObjectAttachments.h"
#include "clientGame/ShipTargetAppearanceManager.h"
#include "clientGame/TangibleObject.h"
#include "clientGraphics/Camera.h"
#include "clientGraphics/DebugPrimitive.h"
#include "clientGraphics/RenderWorld.h"
#include "clientObject/GameCamera.h"
#include "clientParticle/ParticleEffectAppearance.h"
#include "clientParticle/SwooshAppearance.h"
#include "clientUserInterface/CuiPreferences.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/CollisionWorld.h"
#include "sharedCollision/SpatialDatabase.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedGame/ShipChassis.h"
#include "sharedGame/ShipComponentAttachmentManager.h"
#include "sharedGame/ShipComponentType.h"
#include "sharedMath/Capsule.h"

#ifdef _DEBUG
#include <set>
#endif
#include <vector>

//======================================================================

namespace ShipObjectEffectsNamespace
{
	//----------------------------------------------------------------------
	
	Object * const findExistingContrail(Object const & object, CrcString const & hardpointName, ShipObject::WatcherVector & oldContrails)
	{
		for (ShipObject::WatcherVector::iterator it = oldContrails.begin (); it != oldContrails.end (); ++it)
		{
			ShipObject::ObjectWatcher & watcher = *it;
			
			Object * const contrailObject = watcher.getPointer();
			
			if (contrailObject != NULL)
			{
				Appearance const * const app = contrailObject->getAppearance();

				if (NULL != app)
				{
					SwooshAppearance const * const swoosh = app->asSwooshAppearance();

					if (swoosh)
					{
						if (contrailObject->getParent () == &object && swoosh->getHardPoint1() == hardpointName)
							return contrailObject;
					}
				}
			}
		}
		
		return NULL;
	}	
	
	//----------------------------------------------------------------------
	
	//so we don't create 1 per alter
	std::vector<CollisionProperty*> ms_colliderList;
	
	Sphere ms_flybyTestSphere;
	
	std::map<NetworkId, float> ms_flybyList;
	float const cms_flybyTestRadius = 50.0f;
	float const cms_flybyTestTime = 5.0f;
	float const cms_flybySphereLeadDistance = 25.0f;

	bool ms_showFlybySphere = false;

	bool s_installed = false;

#ifdef _DEBUG
	typedef std::set<std::pair<uint32, uint32> > StringPairSet;
	StringPairSet ms_warningSet;
#endif

	//-------------------------------------------------------------------------
	// Disabled by default because the art isn't setup.
	bool s_useTargetAppearanceEffects = false;

	//-------------------------------------------------------------------------
	void install()
	{
		if (s_installed)
			return;

		s_installed = true;

		DebugFlags::registerFlag(ms_showFlybySphere, "ClientGame/ShipObject", "showFlybySphere");

		ms_flybyTestSphere.setRadius(cms_flybyTestRadius);

		DebugFlags::registerFlag(s_useTargetAppearanceEffects, "ClientGame/ShipObject", "useTargetAppearanceEffects");
	}

	//----------------------------------------------------------------------
	float const s_baseShipTargetingEffectRadius = 7.5f;

	bool ms_hideAllTargetEffects = false;
}

using namespace ShipObjectEffectsNamespace;

//----------------------------------------------------------------------

class ShipObjectEffects::EngineEffects
{
public:
	WatcherVector visualEffects; //lint !e1925 //public data member
	VehicleThrusterSoundRuntimeData soundRuntime[2]; //lint !e1925 //public data member

	typedef std::pair<float, InterpolatedSoundRuntime *> FloatInterpolatedSoundRuntimePair;
	typedef stdvector<FloatInterpolatedSoundRuntimePair>::fwd FloatInterpolatedSoundRuntimePairVector;
	FloatInterpolatedSoundRuntimePairVector m_damageInterpolatedRuntimes;

	EngineEffects() :
	visualEffects(),
	m_damageInterpolatedRuntimes()
	{
	}
	
	void cleanup()
	{
		//-- update visual effects
		{
			for (WatcherVector::iterator wv_it = visualEffects.begin(); wv_it != visualEffects.end(); ++wv_it)
			{
				ObjectWatcher & objectWatcher = *wv_it;
				Object * const effectObj = objectWatcher.getPointer();
				if (NULL != effectObj)
				{
					effectObj->kill();
				}
			}
		}
		
		soundRuntime[0].stopAllSounds();
		soundRuntime[1].stopAllSounds();
		
		visualEffects.clear();
		
		{
			for (FloatInterpolatedSoundRuntimePairVector::iterator it = m_damageInterpolatedRuntimes.begin(); it != m_damageInterpolatedRuntimes.end(); ++it)
			{
				FloatInterpolatedSoundRuntimePair & p = *it;
				InterpolatedSoundRuntime * const runtime = p.second;
				delete runtime;
			}
			m_damageInterpolatedRuntimes.clear();
		}
	}
};

//----------------------------------------------------------------------

ShipObjectEffects::ShipObjectEffects(ShipObject & ship) :
m_ship(&ship),
m_engineEffects(new EngineEffectsMap),
m_contrails(new WatcherVector),
m_boosterInterpolatedSoundRuntime(new InterpolatedSoundRuntime),
m_flyByTemplate(NULL),
m_targetAppearanceObjectFlag(false),
m_targetComponentAppearanceObjectSlot(static_cast<int>(ShipChassisSlotType::SCST_invalid)),
m_targetAppearanceUseParentOrientation(true),
m_targetAppearanceSilhouetteDistance(1.f),
m_targetAcquiredAppearanceObject(NULL),
m_targetAcquiringEffect(NULL),
m_glowsEngine(NULL),
m_glowsBooster(NULL),
m_targetAcquiringEffectTimer(1.0f)
#ifdef _DEBUG
, m_missingHardpoint0Warning(false)
, m_missingHardpoint1Warning(false)
#endif
{
	if (!s_installed)
		install();
}

//----------------------------------------------------------------------

ShipObjectEffects::~ShipObjectEffects()
{

	m_ship = NULL;

	removeThrusterEffects();

	delete m_engineEffects;
	m_engineEffects = NULL;

	delete m_contrails;
	m_contrails = NULL;

	delete m_boosterInterpolatedSoundRuntime;
	m_boosterInterpolatedSoundRuntime = NULL;

	if (NULL != m_flyByTemplate)
	{
		SoundTemplateList::release(m_flyByTemplate);
		m_flyByTemplate = NULL;
	}

	removeAllTargetingEffects();
	
	delete m_targetAcquiringEffect;
	m_targetAcquiringEffect = NULL;

	delete m_glowsBooster;
	m_glowsBooster = NULL;

	delete m_glowsEngine;
	m_glowsEngine = NULL;
}

//----------------------------------------------------------------------

void ShipObjectEffects::initialize()
{
	ShipChassis const * const chassis = ShipChassis::findShipChassisByCrc(m_ship->getChassisType());
	if (NULL != chassis)
	{
		std::string const & flyBySound = chassis->getFlyBySound();
		if (!flyBySound.empty())
			m_flyByTemplate = SoundTemplateList::fetch(flyBySound.c_str());
	}
}

//----------------------------------------------------------------------

void ShipObjectEffects::updateComponentState(TangibleObject * const tangibleObject, int const, float const oldDamageLevel, float const currentDamageLevel)
{
	ClientDataFile const * const cdf = tangibleObject->getClientData();
	
	if (NULL == cdf)
		return;

	{
		CachedNetworkId networkId(tangibleObject->getNetworkId());
		EngineEffectsMap::iterator const fit = m_engineEffects->find(networkId);

		if (fit != m_engineEffects->end())
		{
			EngineEffects & effects = (*fit).second;
			WatcherVector & engineEffectVector = effects.visualEffects;

			ClientDataFile::ObjectVector thrusterObjectVector;
			thrusterObjectVector.reserve(engineEffectVector.size());
			
			{
				for (WatcherVector::iterator eev_it = engineEffectVector.begin(); eev_it != engineEffectVector.end(); ++eev_it)
				{
					ObjectWatcher & engineEffectObjectWatcher = *eev_it;
					Object * const effectObject = engineEffectObjectWatcher.getPointer();
					if (NULL != effectObject)
						thrusterObjectVector.push_back(effectObject);
				}
			}
			
			VehicleThrusterSoundData vtsd;

			//-- if this method returns true, our thrusterObjectVector objects have been killed & replaced by new ones
			if (cdf->updateVehicleThrusters(*tangibleObject, oldDamageLevel, currentDamageLevel, thrusterObjectVector, vtsd))
			{
				effects.soundRuntime[0].setObject(m_ship);
				effects.soundRuntime[0].updateSoundData(vtsd, false);
				effects.soundRuntime[1].setObject(m_ship);
				effects.soundRuntime[1].updateSoundData(vtsd, true);
				
				//-- out with the old, in with the new
				engineEffectVector.clear();
				engineEffectVector.reserve(thrusterObjectVector.size());

				
				for (ClientDataFile::ObjectVector::const_iterator tit = thrusterObjectVector.begin(); tit != thrusterObjectVector.end(); ++tit)
				{
					Object * const effectObject = *tit;
					engineEffectVector.push_back(ObjectWatcher(effectObject));
				}
			}
			
			updateEngineInterpolatedSoundState(effects, currentDamageLevel);
		}
	}
}

//----------------------------------------------------------------------

void ShipObjectEffects::updateEngineInterpolatedSoundState(EngineEffects & effects, float const currentDamageLevel)
{
	//-- update interpolated sounds for engine
	
	if (!effects.m_damageInterpolatedRuntimes.empty())
	{
		bool found = false;
		for (EngineEffects::FloatInterpolatedSoundRuntimePairVector::reverse_iterator dit = effects.m_damageInterpolatedRuntimes.rbegin(); dit != effects.m_damageInterpolatedRuntimes.rend(); ++dit)
		{
			float runtimeDamageLevel = (*dit).first;
			InterpolatedSoundRuntime * const runtime = (*dit).second;
			
			if (!found && currentDamageLevel >= runtimeDamageLevel)
			{
				runtime->setActive(true);
				found = true;
			}
			else
				runtime->setActive(false);
		}
		
		//-- set the normal one active at all times
		effects.m_damageInterpolatedRuntimes.front().second->setActive(true);
	}
}

//----------------------------------------------------------------------

void ShipObjectEffects::updateEngineEffects(float const elapsedTime)
{
	//--
	//-- update engine effects
	//--
	
	float const currentSpeed = m_ship->getCurrentSpeed();

	//-- Engine speed max should not include the booster speed
	float const maxSpeed = m_ship->getEngineSpeedMaximum() * (m_ship->getChassisSpeedMaximumModifier() * m_ship->getComponentEfficiencyGeneral(ShipChassisSlotType::SCST_engine));

	float const speedPercent = clamp(0.0f, (maxSpeed > 0.0f) ? (currentSpeed / maxSpeed) : 0.0f, 1.0f);
	float const thrusterScale = linearInterpolate (0.1f, 1.0f, speedPercent);
	float desiredSpeedPercent = speedPercent;
	bool engineGlowsActive = false;

	if (m_ship->isSlotInstalled(ShipChassisSlotType::SCST_engine) && 
		!m_ship->isComponentDisabled(ShipChassisSlotType::SCST_engine) && 
		!m_ship->isComponentDemolished(ShipChassisSlotType::SCST_engine))
	{
		engineGlowsActive = true;

		ShipController const * const shipController = m_ship->getController() ? m_ship->getController()->asShipController() : 0;
		if (shipController)
		{
			Vector const & velocity_p = shipController->getVelocity_p();
			desiredSpeedPercent = clamp(0.f, (maxSpeed > 0.f) ? (velocity_p.magnitude() / maxSpeed) : 0.f, 1.f);
		}
	}
	
	if (NULL != m_glowsEngine)
	{
		for (WatcherVector::iterator it = m_glowsEngine->begin(); it != m_glowsEngine->end(); ++it)
		{
			ObjectWatcher & ow = *it;
			if (NULL != ow.getPointer())
			{
				ow->setActive(engineGlowsActive);
			}
		}
	}
	
	{
		for (EngineEffectsMap::iterator it = m_engineEffects->begin(); it != m_engineEffects->end(); ++it)
		{
			EngineEffects & effects = (*it).second;
			WatcherVector & engineEffectVector = effects.visualEffects;
			
			//-- update visual effects
			{
				for (WatcherVector::iterator wv_it = engineEffectVector.begin(); wv_it != engineEffectVector.end(); ++wv_it)
				{
					ObjectWatcher & objectWatcher = *wv_it;
					Object * const effectObj = objectWatcher.getPointer();
					if (effectObj == NULL)
						continue;
					ParticleEffectAppearance * const pea = dynamic_cast<ParticleEffectAppearance *>(effectObj->getAppearance());
					if (pea != NULL)
						pea->setUniformScale(thrusterScale);
				}
			}
			
			//-- update engine sound effects
			
			effects.soundRuntime[0].update(elapsedTime, desiredSpeedPercent);
			effects.soundRuntime[1].update(elapsedTime, desiredSpeedPercent);

			{
				for (EngineEffects::FloatInterpolatedSoundRuntimePairVector::iterator dit = effects.m_damageInterpolatedRuntimes.begin(); dit != effects.m_damageInterpolatedRuntimes.end(); ++dit)
				{
					InterpolatedSoundRuntime * const runtime = (*dit).second;

					if (runtime->isValid())
						runtime->update(true, desiredSpeedPercent, elapsedTime);
				}
			}
		}
	}
	
	//--
	//-- update booster sounds
	//--

	if (m_ship->isSlotInstalled(ShipChassisSlotType::SCST_booster))
	{
		bool const isBoosterActive = !m_ship->isComponentDemolished(ShipChassisSlotType::SCST_booster) && 
			!m_ship->isComponentDisabled(ShipChassisSlotType::SCST_booster) && 
			m_ship->isBoosterActive();

		if (m_boosterInterpolatedSoundRuntime->isValid())
		{
			bool const boosterActive = m_ship->isBoosterActive() && !m_ship->isComponentDisabled(ShipChassisSlotType::SCST_booster);
			float const desiredInterpolation = boosterActive ? speedPercent : 0.0f;

			m_boosterInterpolatedSoundRuntime->update(m_ship->isBoosterActive(), desiredInterpolation, elapsedTime);
		}

		if (NULL != m_glowsBooster)
		{
			for (WatcherVector::iterator it = m_glowsBooster->begin(); it != m_glowsBooster->end(); ++it)
			{
				ObjectWatcher & ow = *it;
				if (NULL != ow.getPointer())
				{
					ow->setActive(isBoosterActive);
					if (!isBoosterActive)
					{
						GlowAppearance * const glowAppearance = safe_cast<GlowAppearance *>(ow->getAppearance());
						if (NULL != glowAppearance)
							glowAppearance->reset();
					}
				}
			}
		}
		
		//-- update off/on contrails
		{
			for (WatcherVector::iterator it = m_contrails->begin(); it != m_contrails->end(); ++it)
			{
				ObjectWatcher & ow = *it;
				Object * const obj = ow.getPointer();
				if (NULL != obj)
				{
					if (NULL != obj->getDebugName() && !strncmp(obj->getDebugName(), "booster", 7))
					{
						if (isBoosterActive && !obj->isActive())
						{
							SwooshAppearance * const swoosh = safe_cast<SwooshAppearance *>(obj->getAppearance());
							swoosh->restart();
						}
						obj->setActive(isBoosterActive);
					}
				}
			}
		}
		
		if (isBoosterActive)
		{
			if (Game::getPlayerContainingShip() == m_ship)
			{
				GameCamera * const gameCamera = const_cast<GameCamera *>(dynamic_cast<GameCamera const *>(Game::getCamera()));			
				if (NULL != gameCamera)
				{
					if (CuiPreferences::getScreenShake())
						gameCamera->jitter(gameCamera->getObjectFrameK_w(), Random::randomReal(0.1f, 3.0f), 0.01f, 1.0f);
				}
			}
		}
	}
}

//----------------------------------------------------------------------

void ShipObjectEffects::resetEngineEffects(ClientObject * const child, int const chassisSlot)
{
	ClientDataFile const * const cdf = child->getClientData();

	if (NULL == cdf)
		return;
	
	//--
	//-- find the thrusters if applicable
	//--
	
	if (ShipChassisSlotType::getComponentTypeForSlot(static_cast<ShipChassisSlotType::Type>(chassisSlot)) == ShipComponentType::SCT_engine)
	{
		ClientDataFile::ObjectVector thrusterObjectVector;		
		VehicleThrusterSoundData vtsd;

		CachedNetworkId networkId(child->getNetworkId());
		EngineEffects & effects = (*m_engineEffects)[networkId];
		
		if (cdf->initVehicleThrusters(*child, thrusterObjectVector, vtsd))
		{			
			if (!effects.visualEffects.empty())
			{
				DEBUG_WARNING(true, ("ShipObjectEffects::resetEngineEffects() found non empty visual effects for attachment."));
				effects.visualEffects.clear();
			}			
			
			effects.soundRuntime[0].setObject(m_ship);
			effects.soundRuntime[0].updateSoundData(vtsd, false);
			effects.soundRuntime[1].setObject(m_ship);
			effects.soundRuntime[1].updateSoundData(vtsd, true);
			
			effects.visualEffects.reserve(thrusterObjectVector.size());
			
			for (ClientDataFile::ObjectVector::const_iterator tit = thrusterObjectVector.begin(); tit != thrusterObjectVector.end(); ++tit)
			{
				Object * const effectObject = *tit;
				effects.visualEffects.push_back(ObjectWatcher(effectObject));
			}
		}
		
		
		//--- create interpolated sound runtimes for engine
		
		ClientDataFile::InterpolatedSoundVector const * const interpolatedSounds = cdf->getInterpolatedSoundVector();
		if (NULL != interpolatedSounds)			
		{
			int const numRuntimes = static_cast<int>(interpolatedSounds->size());
			effects.m_damageInterpolatedRuntimes.clear();
			effects.m_damageInterpolatedRuntimes.reserve(numRuntimes);
			
			{
				int soundIndex = 0;
				for (ClientDataFile::InterpolatedSoundVector::const_iterator sit = interpolatedSounds->begin(); sit != interpolatedSounds->end(); ++sit, ++soundIndex)
				{
					InterpolatedSound const & interpolatedSound = *sit;
					float const damageLevel = static_cast<float>(soundIndex) / numRuntimes;
					
					InterpolatedSoundRuntime * const runtime = new InterpolatedSoundRuntime;
					runtime->setObject(*child, interpolatedSound);
					
					effects.m_damageInterpolatedRuntimes.push_back(EngineEffects::FloatInterpolatedSoundRuntimePair(damageLevel, runtime));
				}
			}

			TangibleObject const * const tangible = child->asTangibleObject();
			if (NULL != tangible)
				updateEngineInterpolatedSoundState(effects, tangible->getDamageLevel());
		}
	}
	
	if (ShipChassisSlotType::getComponentTypeForSlot(static_cast<ShipChassisSlotType::Type>(chassisSlot)) == ShipComponentType::SCT_booster)
	{
		m_boosterInterpolatedSoundRuntime->setObject(child);
	}
}

//----------------------------------------------------------------------

void ShipObjectEffects::removeThrusterEffects()
{
	{
		for (EngineEffectsMap::iterator it = m_engineEffects->begin(); it != m_engineEffects->end(); ++it)
		{
			EngineEffects & effects = (*it).second;
			effects.cleanup();
		}
	}

	m_engineEffects->clear();
}

//----------------------------------------------------------------------

void ShipObjectEffects::removeBoosterEffect()
{
	m_boosterInterpolatedSoundRuntime->setObject(NULL);
}


//----------------------------------------------------------------------

void ShipObjectEffects::clearContrails()
{
	for (WatcherVector::iterator it = m_contrails->begin (); it != m_contrails->end (); ++it)
	{
		ObjectWatcher & watcher = *it;

		Object * const obj    = watcher.getPointer ();
		if (obj != NULL)
			obj->kill();
	}

	m_contrails->clear();
}

//----------------------------------------------------------------------

void ShipObjectEffects::resetContrails()
{
	WatcherVector oldContrails = *m_contrails;
	StringSet overrides;
	m_contrails->clear();
	addContrailsFor(*m_ship, overrides, oldContrails);

	//-- cleanup the leftovers
	{
		for (WatcherVector::iterator it = oldContrails.begin(); it != oldContrails.end(); ++it)
		{
			ObjectWatcher const & objectWatcher = *it;

			WatcherVector::iterator cur_it = std::find(m_contrails->begin(), m_contrails->end(), objectWatcher);
			if (cur_it == m_contrails->end())
			{
				Object * const obj = objectWatcher.getPointer();
				if (obj != NULL)
					obj->kill();
			}
		}
	}
}

//----------------------------------------------------------------------

void ShipObjectEffects::addContrailsFor(Object & object, StringSet & overrides, WatcherVector & oldContrails)
{
	//-- Recurse to children
	for (int i = 0; i < object.getNumberOfChildObjects(); ++i)
	{
		Object * const childObject = object.getChildObject(i);
		if (childObject && !childObject->getKill())
			addContrailsFor(*childObject, overrides, oldContrails);
	}

	Appearance * const appearance = object.getAppearance();
	if (NULL != appearance)
	{
		ClientObject * const clientObject = object.asClientObject();
		ClientDataFile const * const clientDataFile = clientObject != NULL ? clientObject->getClientData() : NULL;
		ClientDataFile::ContrailDataVector const * const contrailDataVector = clientDataFile != NULL ? clientDataFile->getContrailDataVector() : NULL;

		if (contrailDataVector != NULL)
		{
			Transform transform;
			for (ClientDataFile::ContrailDataVector::const_iterator it = contrailDataVector->begin(); it != contrailDataVector->end(); ++it)
			{
				NOT_NULL(clientDataFile);
				ContrailData const * const contrailData = NON_NULL(*it);

				ConstCharCrcString const hardpointName0(contrailData->m_hardpointName0.c_str());
				ConstCharCrcString const hardpointName1(contrailData->m_hardpointName1.c_str());

				if (!appearance->findHardpoint(hardpointName0, transform))
				{
#ifdef _DEBUG
					if (!m_missingHardpoint0Warning)
					{
						m_missingHardpoint0Warning = true;

						std::pair<uint32, uint32> warning(Crc::calculate(contrailData->m_name.c_str()), Crc::calculate(contrailData->m_hardpointName0.c_str()));
						if (ms_warningSet.find(warning) == ms_warningSet.end())
						{
							DEBUG_WARNING(true, ("ShipObject contrail [%s] for CDF [%s] specifies invalid hardpoint0 [%s]", contrailData->m_name.c_str(), clientDataFile->getName(), contrailData->m_hardpointName0.c_str()));
							IGNORE_RETURN(ms_warningSet.insert(warning));
						}
					}
#endif
					continue;
				}
				else
				{
					if (!hardpointName1.isEmpty())
					{
						if (!appearance->findHardpoint(hardpointName1, transform))
						{
#ifdef _DEBUG
							if (!m_missingHardpoint1Warning)
							{
								m_missingHardpoint1Warning = true;

								std::pair<uint32, uint32> warning(Crc::calculate(contrailData->m_name.c_str()), Crc::calculate(contrailData->m_hardpointName1.c_str()));
								if (ms_warningSet.find(warning) == ms_warningSet.end())
								{
									DEBUG_WARNING(true, ("ShipObject contrail [%s] for CDF [%s] specifies invalid hardpoint0 [%s]", contrailData->m_name.c_str(), clientDataFile->getName(), contrailData->m_hardpointName1.c_str()));
									IGNORE_RETURN(ms_warningSet.insert(warning));
								}
							}
#endif

							continue;
						}
					}
				}

				//-- override other contrails with the same name
				if (!contrailData->m_overrideName.empty())
					IGNORE_RETURN(overrides.insert(contrailData->m_overrideName));

				//-- this contrail has been overridden by one of the child objects' contrails
				if (overrides.find(contrailData->m_name) != overrides.end())
					continue;

				Object * const existingContrailObject  = findExistingContrail(object, hardpointName0, oldContrails);

				if (existingContrailObject == NULL)
				{
					SwooshAppearance * const contrailAppearance = contrailData->createAppearance(*appearance);
					if (contrailAppearance != NULL)
					{
						Object * const hpo = new Object();
						hpo->setDebugName(contrailData->m_name.c_str());
						hpo->setAppearance(contrailAppearance);
						RenderWorld::addObjectNotifications(*hpo);
						object.addChildObject_o(hpo);
						contrailAppearance->setWidth(contrailData->m_width);
						contrailAppearance->useRenderEffectsFlag(true);
						m_contrails->push_back(ObjectWatcher(hpo));
					} //lint !e429 //custodial hpo
				}
				else
				{
					ObjectWatcher const objectWatcher(static_cast<Object *>(existingContrailObject));
					IGNORE_RETURN(oldContrails.erase(std::remove(oldContrails.begin(), oldContrails.end(), objectWatcher)));
					m_contrails->push_back(objectWatcher);
				}
			}
		}
	}
}

//----------------------------------------------------------------------

void ShipObjectEffects::updateFlybySounds(float const elapsedTime)
{
	//tick down, remove items from list
	{
		for(std::map<NetworkId, float>::iterator i = ms_flybyList.begin(); i != ms_flybyList.end();)
		{
			if((i->second - elapsedTime) < 0.0f)
				ms_flybyList.erase(i++);
			else
			{
				i->second -= elapsedTime;
				++i;
			}
		}
	}

	SpatialDatabase const * const sd = CollisionWorld::getDatabase();
	if(sd)
	{
		ms_colliderList.clear();

		Vector const & delta = m_ship->getObjectFrameK_w() * cms_flybySphereLeadDistance;
		Vector const & finalPos = m_ship->getPosition_w() + delta;

#ifdef _DEBUG
		if(ms_showFlybySphere)
		{
			Camera const * const camera = Game::getCamera();
			if(camera)
				camera->addDebugPrimitive(new SphereDebugPrimitive(UtilityDebugPrimitive::S_z, Transform::identity, finalPos, cms_flybyTestRadius, 8, 8));
		}
#endif

		ms_flybyTestSphere.setCenter(finalPos);
		Capsule const flybyTestCapsule(ms_flybyTestSphere, Vector::zero);
		sd->queryFor(static_cast<int>(SpatialDatabase::Q_Dynamic), m_ship->getParentCell(), true, flybyTestCapsule, ms_colliderList);
		for(std::vector<CollisionProperty*>::const_iterator i = ms_colliderList.begin(); i != ms_colliderList.end(); ++i)
		{
			if((*i)->isShip())
			{
				Object const & ship = (*i)->getOwner();
				ClientObject const * const clientShip = ship.asClientObject();
				ShipObject const * const shipObject = clientShip ? clientShip->asShipObject() : NULL;
				if(shipObject && shipObject != m_ship)
				{
					std::map<NetworkId, float>::const_iterator const pos = ms_flybyList.find(shipObject->getNetworkId());
					//if not already in the map
					if(pos == ms_flybyList.end())
					{
						bool playSound = false;
						//case 1 to play sound: ship is flying "towards" and is at 60% of more of it's max speed
						{
							if(m_ship->getObjectFrameK_w().dot(shipObject->getObjectFrameK_w()) < -0.5f)
								if(shipObject->getCurrentSpeed() > (shipObject->getShipActualSpeedMaximum() * 0.6f))
									playSound = true;
						}
						//case 2 to play sound: we are close to stationary, and ship is at 60% of more of it's max speed
						{
							if(m_ship->getCurrentSpeed() < (m_ship->getShipActualSpeedMaximum() * 0.25f))
								if(shipObject->getCurrentSpeed() > (shipObject->getShipActualSpeedMaximum() * 0.6f))
									playSound = true;
						}
						if(playSound)
						{
							ShipChassis const * const chassis = ShipChassis::findShipChassisByCrc(shipObject->getChassisType());
							if(chassis && !chassis->getFlyBySound().empty())
								IGNORE_RETURN(Audio::attachSound(chassis->getFlyBySound().c_str(), shipObject));
							//add to map
							ms_flybyList[shipObject->getNetworkId()] = cms_flybyTestTime;
						}
					}
				}
			}
		}
	}
} //lint !e1762 //not const

//----------------------------------------------------------------------

void ShipObjectEffects::setTargetAppearanceActive(bool active)
{
	// If the component targeting code is active, do not set the target appearance for the ship.
	bool const isTargetingComponent = m_targetComponentAppearanceObjectSlot != static_cast<int>(ShipChassisSlotType::SCST_invalid);

	if (!boolEqual(m_targetAppearanceObjectFlag, active))
	{

		// Kick off transitional effect.
		if (s_useTargetAppearanceEffects)
		{
			ClientEffectTemplate const * const clientEffectTemplate = ShipTargetAppearanceManager::getClientEffectTemplate(m_ship->getChassisType(), active, m_ship->isEnemy());
			if (clientEffectTemplate)
			{
				CrcLowerString hardpointName = ShipTargetAppearanceManager::getHardpointName(m_ship->getChassisType());
				
				ClientEffect * const clientEffect = clientEffectTemplate->createClientEffect(m_ship, hardpointName);
				
				//-- Don't show this object in the UI.
				clientEffect->setUseRenderEnableFlags(true);
				
				//-- Get the scale.
				float const baseScale = isTargetingComponent ? 1.0f : m_ship->getAppearanceSphereRadius() / s_baseShipTargetingEffectRadius;
				float const scale = ShipTargetAppearanceManager::getScale(m_ship->getChassisType()) * baseScale;
				if (scale > 0.0f)
				{
					clientEffect->setUniformScale(scale);
				}
				
				//-- Create the effect data.
				clientEffect->execute();
				
				//-- Delete the effect.
				delete clientEffect;
			}
		}

		//-- Play sound.
		if(active)
		{
			ShipTargetAppearanceManager::playActivateSound(m_ship->getChassisType());
		}

		//-- If successful, set flag.
		m_targetAppearanceObjectFlag = active;
	}

}

//----------------------------------------------------------------------

bool ShipObjectEffects::getTargetAppearanceActive() const
{
	return m_targetAppearanceObjectFlag;
}

//----------------------------------------------------------------------

void ShipObjectEffects::updateTargetAppearance(float const elapsedTime)
{
	Appearance const * const appearance = m_ship->getAppearance();

	if (appearance != NULL && appearance->getRenderedThisFrame())
	{
		Vector const & shipPosition_w = ShipObject::getTargetInterceptPosition_w(0.f, *m_ship);

		if (m_targetAcquiredAppearanceObject)
		{
			m_targetAcquiredAppearanceObject->setPosition_w(shipPosition_w);
			if(ms_hideAllTargetEffects)
				removeAllTargetingEffects();
			
		}
	}

	IGNORE_RETURN(m_targetAcquiringEffectTimer.updateNoReset(elapsedTime));
}

//----------------------------------------------------------------------

void ShipObjectEffects::setTargetAcquiringActive(bool const active, float const acquisitionTime)
{
	bool const isTargetingComponent = m_targetComponentAppearanceObjectSlot != static_cast<int>(ShipChassisSlotType::SCST_invalid);

	//-- Construct effect.
	if (active && !ms_hideAllTargetEffects)
	{
		if (m_targetAcquiringEffectTimer.isExpired() && !m_targetAcquiringEffect && !isTargetingComponent) 
		{
			ClientEffectTemplate const * const clientEffectTemplate = ShipTargetAppearanceManager::getAcquiringEffectTemplate(m_ship->getChassisType());
			if (clientEffectTemplate)
			{
				CrcLowerString hardpointName = ShipTargetAppearanceManager::getHardpointName(m_ship->getChassisType());
				
				m_targetAcquiringEffect = clientEffectTemplate->createClientEffect(m_ship, hardpointName);
				
				//-- Don't show this object in the UI.
				m_targetAcquiringEffect->setUseRenderEnableFlags(true);
				
				// m_targetAcquiringEffect->setTheDurationMang(duration);
				
				//-- Get the scale.
				float const baseScale = m_ship->getAppearanceSphereRadius() / s_baseShipTargetingEffectRadius;
				float const scale = ShipTargetAppearanceManager::getScale(m_ship->getChassisType()) * baseScale;
				if (scale > 0.0f)
				{
					m_targetAcquiringEffect->setUniformScale(scale);
				}
				
				//-- Create the effect data.
				m_targetAcquiringEffect->execute();

				//-- Reset the acquiring interface.
				m_targetAcquiringEffectTimer.reset();
			}
		}

		//-- Play sound.
		ShipTargetAppearanceManager::playTargetAcquiringSoundName(m_ship->getChassisType());

		// Set the expire time here.
		m_targetAcquiringEffectTimer.setExpireTime(acquisitionTime);
	}
	else
	{
		if(m_targetAcquiringEffect)
		{
			delete m_targetAcquiringEffect;
			m_targetAcquiringEffect = NULL;
		}
	}
}

//----------------------------------------------------------------------

void ShipObjectEffects::setTargetAcquiredActive(bool const active)
{
	bool const isTargetingComponent = m_targetComponentAppearanceObjectSlot != static_cast<int>(ShipChassisSlotType::SCST_invalid);

	if (active && !ms_hideAllTargetEffects)
	{
		if (!m_targetAcquiredAppearanceObject)
		{
			m_targetAcquiredAppearanceObject = new Object();

			AppearanceTemplate const * const appearanceTemplate = ShipTargetAppearanceManager::getAcquiredAppearanceTemplate(m_ship->getChassisType());
			if (appearanceTemplate)
			{
				Appearance * const effectAppearance = appearanceTemplate->createAppearance();
				m_targetAcquiredAppearanceObject->setAppearance(effectAppearance);

				//-- Observe the renderEnable flags for the list view.
				effectAppearance->useRenderEffectsFlag(true);

				//-- Setup the object scale.
				float const baseScale = isTargetingComponent ? 1.0f : m_ship->getAppearanceSphereRadius() / s_baseShipTargetingEffectRadius;
				float const scale = ShipTargetAppearanceManager::getScale(m_ship->getChassisType()) * baseScale;
				if (scale > FLT_MIN)
				{
					m_targetAcquiredAppearanceObject->setScale(Vector(scale, scale, scale));
				}

				m_targetAcquiredAppearanceObject->setPosition_w(ShipObject::getTargetInterceptPosition_w(0.0f, *m_ship));
			}

			RenderWorld::addObjectNotifications(*m_targetAcquiredAppearanceObject);
			m_targetAcquiredAppearanceObject->attachToObject_w(m_ship, true);
		}

		//-- Play sound.
		ShipTargetAppearanceManager::playTargetAcquiredSoundName(m_ship->getChassisType());
	}
	else
	{
		if (m_targetAcquiredAppearanceObject)
		{
			m_targetAcquiredAppearanceObject->kill();
			m_targetAcquiredAppearanceObject = NULL;
		}
	}
}

//----------------------------------------------------------------------

void ShipObjectEffects::removeAllTargetingEffects()
{
	setTargetAcquiringActive(false, 0.0f);
	setTargetAcquiredActive(false);
}

//----------------------------------------------------------------------

void ShipObjectEffects::updateGlowListsInternal(Object & object)
{
	char const * const name = object.getDebugName();
	
	if (NULL != name)
	{
		Appearance * const app = object.getAppearance();
		if (NULL != app)
		{
			GlowAppearance * const glow = dynamic_cast<GlowAppearance *>(app);
			if (NULL != glow)
			{
				if (!strcmp(name, "booster"))
				{
					if (NULL == m_glowsBooster)
						m_glowsBooster = new WatcherVector;
					m_glowsBooster->push_back(ObjectWatcher(&object));
				}
				
				if (!strcmp(name, "engine"))
				{
					if (NULL == m_glowsEngine)
						m_glowsEngine = new WatcherVector;
					m_glowsEngine->push_back(ObjectWatcher(&object));
				}
			}
		}
	}
	
	//-- Recurse to children
	for (int i = 0; i < object.getNumberOfChildObjects(); ++i)
	{
		Object * const childObject = object.getChildObject(i);
		if (childObject && !childObject->getKill())
			updateGlowListsInternal(*childObject);
	}
}

//----------------------------------------------------------------------

void ShipObjectEffects::updateGlowLists()
{
	if (NULL != m_glowsEngine)
		m_glowsEngine->clear();
	if (NULL != m_glowsBooster)
		m_glowsBooster->clear();

	updateGlowListsInternal(*m_ship);
}

//----------------------------------------------------------------------

void ShipObjectEffects::releaseEffectsForSlot(int chassisSlot)
{
	int const componentType = ShipChassisSlotType::getComponentTypeForSlot(static_cast<ShipChassisSlotType::Type>(chassisSlot));
	
	switch (componentType)
	{
	case ShipComponentType::SCT_engine:
		if (NULL != m_engineEffects)
			removeThrusterEffects();
		if (NULL != m_glowsEngine)
			m_glowsEngine->clear();
		break;
	case ShipComponentType::SCT_booster:
		if (NULL != m_glowsBooster)
			m_glowsBooster->clear();
		break;
	}
}

//----------------------------------------------------------------------

void ShipObjectEffects::setHideAllTargetEffects(bool b)
{
	ms_hideAllTargetEffects = b;
}

//----------------------------------------------------------------------
