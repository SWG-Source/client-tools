// ======================================================================
//
// ClientEffectManager.cpp
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientEffectManager.h"

#include "clientGame/CellObject.h"
#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientEffect.h"
#include "clientGame/ClientEffectTemplateList.h"
#include "clientGame/ClientWeaponObjectTemplate.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientGame/PlayerObject.h"
#include "clientGame/Projectile.h"
#include "clientGame/SaddleManager.h"
#include "clientGame/TrackingProjectile.h"
#include "clientGraphics/Light.h"
#include "clientGraphics/RenderWorld.h"
#include "clientObject/TrailAppearance.h"
#include "clientParticle/ParticleEffectAppearance.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiStringVariablesData.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/CollisionWorld.h"
#include "sharedCollision/Floor.h"
#include "sharedCollision/FloorLocator.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/MemoryBlockManagerMacros.h"
#include "sharedMath/Plane.h"
#include "sharedMessageDispatch/Emitter.h"
#include "sharedMessageDispatch/Receiver.h"
#include "sharedNetworkMessages/ClientEffectMessages.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedTerrain/TerrainObject.h"

#include "StringId.h"

#include <vector>

// ======================================================================

namespace ClientEffectManagerNamespace
{
	const ConstCharCrcLowerString cs_lootDisc("appearance/pt_loot_disc.prt");
	float const					  ms_maxAutoLootDistance = 16.0f;
	StringId                      ms_autoLootStringId("ui", "auto_loot_fail");

	//NOTE: We need a separate class to handle the Receiver, since it's non-static and
	class Listener : public MessageDispatch::Receiver
	{
	public:

		Listener()
		:	MessageDispatch::Receiver()
		{
			connectToMessage (PlayClientEffectObjectMessage::MESSAGE_TYPE);
			connectToMessage (PlayClientEffectObjectTransformMessage::MESSAGE_TYPE);
			connectToMessage (PlayClientEffectLocMessage::MESSAGE_TYPE);
			connectToMessage (StopClientEffectObjectByLabelMessage::MESSAGE_TYPE);
			connectToMessage (CreateClientProjectileMessage::MESSAGE_TYPE);
			connectToMessage (CreateClientProjectileObjectToObjectMessage::MESSAGE_TYPE);
			connectToMessage (CreateClientProjectileObjectToLocationMessage::MESSAGE_TYPE);
			connectToMessage (CreateClientProjectileLocationToObjectMessage::MESSAGE_TYPE);
		}
		
		//----------------------------------------------------------------------

		void receiveMessage(const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message)
		{
			UNREF(source);
			if (message.isType (PlayClientEffectObjectMessage::MESSAGE_TYPE))
			{
				Archive::ReadIterator ri = dynamic_cast<const GameNetworkMessage &>(message).getByteStream().begin();

				PlayClientEffectObjectMessage pceom(ri);
				
				if( ( !CuiPreferences::getShowCorpseLootIcon() || CuiPreferences::getAutoLootCorpses() ) &&  CrcLowerString(pceom.getEffectName().c_str()) == cs_lootDisc )
				{
					if(CuiPreferences::getAutoLootCorpses())
					{
						Object * creature = NetworkIdManager::getObjectById(pceom.getObjectId());
						if(creature && Game::getPlayerCreature())
						{
							Vector distance = Game::getPlayerCreature()->getPosition_w() - creature->getPosition_w();
							
							if(distance.approximateMagnitude() < ms_maxAutoLootDistance)
							{
								ClientCommandQueue::enqueueCommand ("loot", pceom.getObjectId(), Unicode::emptyString);
							}
							else
							{
								Unicode::String result;
								CuiStringVariablesData stringData;
								stringData.target = creature->asClientObject();

								CuiStringVariablesManager::process(ms_autoLootStringId, stringData, result);

								CuiSystemMessageManager::sendFakeSystemMessage(result);
							}
						}
						
					}

					if(!CuiPreferences::getShowCorpseLootIcon())
						return;
				}
				ClientEffectManager::playLabeledClientEffect(CrcLowerString(pceom.getEffectName().c_str()), NetworkIdManager::getObjectById(pceom.getObjectId()), CrcLowerString(pceom.getHardpoint().c_str()), CrcLowerString(pceom.getLabel().c_str()));
			}
			else if (message.isType (PlayClientEffectObjectTransformMessage::MESSAGE_TYPE))
			{
				Archive::ReadIterator ri = dynamic_cast<const GameNetworkMessage &>(message).getByteStream().begin();

				PlayClientEffectObjectTransformMessage const pceotm(ri);

				if( (!CuiPreferences::getShowCorpseLootIcon() || CuiPreferences::getAutoLootCorpses() ) && CrcLowerString(pceotm.getEffectName().c_str()) == cs_lootDisc )
				{
					if(CuiPreferences::getAutoLootCorpses())
					{
						Object * creature = NetworkIdManager::getObjectById(pceotm.getObjectId());
						if(creature && Game::getPlayerCreature())
						{
							Vector distance = Game::getPlayerCreature()->getPosition_w() - creature->getPosition_w();

							if(distance.approximateMagnitude() < ms_maxAutoLootDistance)
							{
								ClientCommandQueue::enqueueCommand ("loot", pceotm.getObjectId(), Unicode::emptyString);
							}
							else
							{
								Unicode::String result;
								CuiStringVariablesData stringData;
								stringData.target = creature->asClientObject();

								CuiStringVariablesManager::process(ms_autoLootStringId, stringData, result);

								CuiSystemMessageManager::sendFakeSystemMessage(result);
							}
						}
					}

					if(!CuiPreferences::getShowCorpseLootIcon())
						return;
				}

				ClientEffectManager::playLabeledClientEffect(CrcLowerString(pceotm.getEffectName().c_str()), NetworkIdManager::getObjectById(pceotm.getObjectId()), pceotm.getTransform(), CrcLowerString(pceotm.getLabel().c_str()));
			}
			else if (message.isType (PlayClientEffectLocMessage::MESSAGE_TYPE))
			{
				Archive::ReadIterator ri = dynamic_cast<const GameNetworkMessage &>(message).getByteStream().begin();
				PlayClientEffectLocMessage pcelm(ri);

				Vector playLoc;

				CellProperty const * cellProperty = CellProperty::getWorldCellProperty ();

				//if interior, transform and display
				if(pcelm.getCell() != CellProperty::getWorldCellProperty()->getOwner().getNetworkId())
				{
					NetworkId const & networkId = pcelm.getCell ();
					Object const * const object = NetworkIdManager::getObjectById (networkId);
					if (!object)
					{
						DEBUG_WARNING (true, ("PlayClientEffectLocMessage: invalid cell %s", networkId.getValueString ().c_str ()));
						return;
					}

					cellProperty = object->getCellProperty();
					if (!cellProperty)
					{
						DEBUG_WARNING (true, ("PlayClientEffectLocMessage: invalid cell property %s", networkId.getValueString ().c_str ()));
						return;
					}

					Floor const * const floor = cellProperty->getFloor();
					if (!floor)
					{
						DEBUG_WARNING (true, ("PlayClientEffectLocMessage: object %s, cell %s does not have a floor", networkId.getValueString ().c_str (), cellProperty->getCellName ()));
						return;
					}

					FloorLocator outLoc;
					floor->dropTest(pcelm.getLocation(), outLoc);
					Vector ground = outLoc.getPosition_p();
					playLoc.x = pcelm.getLocation().x;
					playLoc.y = ground.y + pcelm.getTerrainDelta();
					playLoc.z = pcelm.getLocation().z;
				}
				//else exterior
				else
				{
					if(Game::isSpace())
					{
						playLoc = pcelm.getLocation();
					}
					else
					{
						const TerrainObject* const terrain = TerrainObject::getInstance();
						if(terrain)
						{
							float terrainHeight = 0;
							bool result = terrain->getHeight(pcelm.getLocation(), terrainHeight);
							UNREF(result);
							DEBUG_WARNING(!result, ("Failed to get terrain height for script-based client effect"));
							//adjust play location for height offset sent from script
							playLoc.x = pcelm.getLocation().x;
							playLoc.y = terrainHeight + pcelm.getTerrainDelta();
							playLoc.z = pcelm.getLocation().z;
						}
						else
						{
							DEBUG_WARNING(true, ("No terrain object for script-based client effect"));
						}

						// See if there's a floor to collide with above the terrain...
						FloorLocator floorLocator;
						bool floorFound = CollisionWorld::makeLocator(CellProperty::getWorldCellProperty(), pcelm.getLocation(), floorLocator);
						if (floorFound && floorLocator.getPosition_w().y > playLoc.y)
							playLoc = floorLocator.getPosition_w();
					}
				}

				ClientEffectManager::playLabeledClientEffect(CrcLowerString(pcelm.getEffectName().c_str()), cellProperty, playLoc, Vector::unitY, CrcLowerString(pcelm.getLabel().c_str()));
			}
			else if (message.isType (StopClientEffectObjectByLabelMessage::MESSAGE_TYPE))
			{
				Archive::ReadIterator ri = dynamic_cast<const GameNetworkMessage &>(message).getByteStream().begin();

				StopClientEffectObjectByLabelMessage const pceotm(ri);
				ClientEffectManager::removeAllClientEffectsForObjectByLabel(NetworkIdManager::getObjectById(pceotm.getObjectId()), CrcLowerString(pceotm.getLabel().c_str()), pceotm.getSoftTerminate());
			}
			else if (message.isType (CreateClientProjectileMessage::MESSAGE_TYPE))
			{
				Archive::ReadIterator ri = dynamic_cast<const GameNetworkMessage &>(message).getByteStream().begin();
				CreateClientProjectileMessage ccpm(ri);

				CellProperty const * cellProperty = CellProperty::getWorldCellProperty ();

				if (ccpm.getStartCell() != CellProperty::getWorldCellProperty()->getOwner().getNetworkId())
				{
					NetworkId const & networkId = ccpm.getStartCell ();
					Object const * const object = NetworkIdManager::getObjectById (networkId);
					if (!object)
					{
						DEBUG_WARNING (true, ("CreateClientProjectileMessage: invalid cell %s", networkId.getValueString ().c_str ()));
						return;
					}

					cellProperty = object->getCellProperty();
					if (!cellProperty)
					{
						DEBUG_WARNING (true, ("CreateClientProjectileMessage: invalid cell property %s", networkId.getValueString ().c_str ()));
						return;
					}
				}

				ClientEffectManager::createClientNonTrackingProjectile(ccpm.getWeaponObjectTemplateName(), cellProperty, ccpm.getStartLocation(), ccpm.getEndLocation(), ccpm.getSpeed(), ccpm.getExpiration(), ccpm.getTrail(), ccpm.getTrailArgb());
			}
			else if (message.isType(CreateClientProjectileObjectToObjectMessage::MESSAGE_TYPE))
			{
				Archive::ReadIterator ri = dynamic_cast<const GameNetworkMessage &>(message).getByteStream().begin();
				CreateClientProjectileObjectToObjectMessage ccpm(ri);

				CellProperty const * cellProperty = CellProperty::getWorldCellProperty ();

				if (ccpm.getStartCell() != CellProperty::getWorldCellProperty()->getOwner().getNetworkId())
				{
					NetworkId const & networkId = ccpm.getStartCell ();
					Object const * const object = NetworkIdManager::getObjectById (networkId);
					if (!object)
					{
						DEBUG_WARNING (true, ("CreateClientProjectileMessage: invalid cell %s", networkId.getValueString ().c_str ()));
						return;
					}

					cellProperty = object->getCellProperty();
					if (!cellProperty)
					{
						DEBUG_WARNING (true, ("CreateClientProjectileMessage: invalid cell property %s", networkId.getValueString ().c_str ()));
						return;
					}
				}
				
				Object const * const sourceObject = NetworkIdManager::getObjectById (ccpm.getSourceObject());
				Object const * const targetObject = NetworkIdManager::getObjectById (ccpm.getTargetObject());

				if(!sourceObject)
				{
					DEBUG_WARNING(true, ("CreateClientProjectileMessageObjectToObject: Source object was NULL. Effect skipped.\n"));
					return;
				}

				if(!targetObject)
				{
					DEBUG_WARNING(true, ("CreateClientProjectileMessageObjectToObject: Target object was NULL. Effect skipped.\n"));
					return;
				}

				ClientEffectManager::createClientTrackingProjectileObjectToObject(ccpm.getWeaponObjectTemplateName(), cellProperty, *sourceObject, ccpm.getSourceHardpointName(), *targetObject, ccpm.getTargetHardpointName(), ccpm.getSpeed(), ccpm.getExpiration(), ccpm.getTrail(), ccpm.getTrailArgb());
			}
			else if (message.isType(CreateClientProjectileObjectToLocationMessage::MESSAGE_TYPE))
			{
				Archive::ReadIterator ri = dynamic_cast<const GameNetworkMessage &>(message).getByteStream().begin();
				CreateClientProjectileObjectToLocationMessage ccpm(ri);

				CellProperty const * cellProperty = CellProperty::getWorldCellProperty ();

				if (ccpm.getStartCell() != CellProperty::getWorldCellProperty()->getOwner().getNetworkId())
				{
					NetworkId const & networkId = ccpm.getStartCell ();
					Object const * const object = NetworkIdManager::getObjectById (networkId);
					if (!object)
					{
						DEBUG_WARNING (true, ("CreateClientProjectileMessage: invalid cell %s", networkId.getValueString ().c_str ()));
						return;
					}

					cellProperty = object->getCellProperty();
					if (!cellProperty)
					{
						DEBUG_WARNING (true, ("CreateClientProjectileMessage: invalid cell property %s", networkId.getValueString ().c_str ()));
						return;
					}
				}

				Object const * const sourceObject = NetworkIdManager::getObjectById (ccpm.getSourceObject());
				
				if(!sourceObject)
				{
					DEBUG_WARNING(true, ("CreateClientProjectileMessageObjectToLocation: Source object was NULL. Effect skipped.\n"));
					return;
				}

				ClientEffectManager::createClientTrackingProjectileObjectToLocation(ccpm.getWeaponObjectTemplateName(), cellProperty, *sourceObject, ccpm.getSourceHardpointName(), ccpm.getTargetLocation(), ccpm.getSpeed(), ccpm.getExpiration(), ccpm.getTrail(), ccpm.getTrailArgb());
			}
			else if (message.isType(CreateClientProjectileLocationToObjectMessage::MESSAGE_TYPE))
			{
				Archive::ReadIterator ri = dynamic_cast<const GameNetworkMessage &>(message).getByteStream().begin();
				CreateClientProjectileLocationToObjectMessage ccpm(ri);

				CellProperty const * cellProperty = CellProperty::getWorldCellProperty ();

				if (ccpm.getStartCell() != CellProperty::getWorldCellProperty()->getOwner().getNetworkId())
				{
					NetworkId const & networkId = ccpm.getStartCell ();
					Object const * const object = NetworkIdManager::getObjectById (networkId);
					if (!object)
					{
						DEBUG_WARNING (true, ("CreateClientProjectileMessage: invalid cell %s", networkId.getValueString ().c_str ()));
						return;
					}

					cellProperty = object->getCellProperty();
					if (!cellProperty)
					{
						DEBUG_WARNING (true, ("CreateClientProjectileMessage: invalid cell property %s", networkId.getValueString ().c_str ()));
						return;
					}
				}

				Object const * const targetObject = NetworkIdManager::getObjectById (ccpm.getTargetObject());
				
				if(!targetObject)
				{
					DEBUG_WARNING(true, ("CreateClientProjectileMessageObjectToObject: Target object was NULL. Effect skipped.\n"));
					return;
				}

				ClientEffectManager::createClientTrackingProjectileLocationToObject(ccpm.getWeaponObjectTemplateName(), cellProperty, ccpm.getStartLocation(), *targetObject, ccpm.getTargetHardpointName(), ccpm.getSpeed(), ccpm.getExpiration(), ccpm.getTrail(), ccpm.getTrailArgb());
			}
		}
	};

	const float s_baseObjectHeightMultipler = 1.0 / 0.5f; // 1 / base value to be compared to object->getAppearanceSphereRadius() to determine effect scale (0.5 is the collision sphere radius for an average human)
	Listener* s_listener;

	float const s_maxAllowedTime = 30.f;   // max time the soft terminate can take before the particle is killed
	float const s_defaultEffectTime = 3.f; // default time for unlabeled particle effects

	std::string const cs_enterStealthEffect("appearance/pt_smoke_puff.prt");
	std::string const cs_exitStealthEffect("appearance/pt_sonic_pulse.prt");
}

using namespace ClientEffectManagerNamespace;

// ======================================================================

ClientEffectManager::LightList    ClientEffectManager::m_lights;
ClientEffectManager::ParticleList ClientEffectManager::m_particleSystems;
ClientEffectManager::ShaderList   ClientEffectManager::m_shaders;
bool                              ClientEffectManager::ms_installed;

// ======================================================================

struct ClientEffectManager::ManagedParticleSystem
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	Watcher<Object> particleSystem;
	Watcher<Object> owner;
	CrcLowerString  hardpoint;
	float           time;
	Transform       transform;
	bool            softParticleTerminate;
	bool            ignoreDuration;
	CrcLowerString  label; // used to remove client effects by name
};

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(ClientEffectManager::ManagedParticleSystem, true, 0, 0, 0);

//----------------------------------------------------------------------

struct ClientEffectManager::ManagedLight
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	Watcher<Light>  light;
	Watcher<Object> owner;
	CrcLowerString  hardpoint;
	float           time;
	Transform       transform;
	CrcLowerString  label; // used to remove client effects by name
};

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(ClientEffectManager::ManagedLight, true, 0, 0, 0);

// ======================================================================

void ClientEffectManager::install()
{
	InstallTimer const installTimer("ClientEffectManager::install");

	DEBUG_FATAL(ms_installed, ("double install"));
	s_listener = new Listener;
	ms_installed = true;

	ClientEffectManager::ManagedParticleSystem::install();
	ClientEffectManager::ManagedLight::install();

	ExitChain::add (remove, "ClientEffectManager::remove");
}

// ----------------------------------------------------------------------

void ClientEffectManager::remove()
{
	//remove any lights, shaders, or particle sytems that we're currently managing the lifetimes of
	for(LightList::iterator i = m_lights.begin(); i != m_lights.end(); ++i)
	{
			delete (*i);
	}

	for(ParticleList::iterator i2 = m_particleSystems.begin(); i2 != m_particleSystems.end(); ++i2)
	{
			delete (*i2);
	}

	delete s_listener;

	DEBUG_FATAL(!ms_installed, ("not installed"));
	ms_installed = false;
}

// ----------------------------------------------------------------------

/** Play the given effect on the given object.
 */
bool ClientEffectManager::playClientEffect(const CrcLowerString& clientEffectName, Object* object, const CrcLowerString& hardpoint)
{
	return playLabeledClientEffect(clientEffectName, object, hardpoint, CrcLowerString::empty);
}

bool ClientEffectManager::playLabeledClientEffect(const CrcLowerString& clientEffectName, Object* object, const CrcLowerString& hardpoint, CrcLowerString const & label)
{
	DEBUG_FATAL(!ms_installed, ("not installed"));

	if (!object)
	{
		DEBUG_WARNING (true, ("ClientEffectManager::playClientEffect [%s]: NULL object specified", clientEffectName.getString()));
		return false;
	}

	// don't play client effects on characters that are not visible on this client
	ClientObject const * const clientObject = object ? object->asClientObject () : 0;
	CreatureObject const * const creatureObject = clientObject ? clientObject->asCreatureObject () : 0;			
	if (creatureObject 
		&& (creatureObject != Game::getPlayer())
		&& !creatureObject->getCoverVisibility() 
		&& !PlayerObject::isAdmin() 
		&& !creatureObject->isPassiveRevealPlayerCharacter(Game::getPlayerNetworkId())
		&& !isStealthEffect(clientEffectName.getString())
	)
	{
		return false;
	}

	ClientEffect* effect = ClientEffectTemplateList::createClientEffect(clientEffectName, object, hardpoint);
	if(effect)
	{
		effect->setLabel(label);

		CollisionProperty const * const collisionProperty = object->getCollisionProperty();
		if (collisionProperty)
		{
			float const collisionRadius = collisionProperty->getBoundingSphere_l().getRadius();
			float const collisionScale = collisionRadius * s_baseObjectHeightMultipler;
			float const effectScale = 0.5f + (0.5f * collisionScale); 
	
			effect->setUniformScale(effectScale);
		}

		effect->execute();
		delete effect;
		return true;
	}
	else
	{
		DEBUG_WARNING(true, ("could not create ClientEffect for file %s, bad filename maybe?", clientEffectName.getString()));
		return false;
	}
}

//----------------------------------------------------------------------

bool ClientEffectManager::playClientEffect(const CrcLowerString& clientEffectName, Object* object, Transform const & transform)
{
	return playLabeledClientEffect(clientEffectName, object, transform, CrcLowerString::empty);
}

bool ClientEffectManager::playLabeledClientEffect(const CrcLowerString& clientEffectName, Object* object, Transform const & transform, CrcLowerString const & label)
{
	DEBUG_FATAL(!ms_installed, ("not installed"));

	if (!object)
	{
		DEBUG_WARNING (true, ("ClientEffectManager::playClientEffect [%s]: NULL object specified", clientEffectName.getString()));
		return false;
	}

// don't play client effects on characters that are not visible on this client
	ClientObject const * const clientObject = object ? object->asClientObject () : 0;
	CreatureObject const * const creatureObject = clientObject ? clientObject->asCreatureObject () : 0;			
	if (creatureObject 
		&& (creatureObject != Game::getPlayer())
		&& !creatureObject->getCoverVisibility() 
		&& !PlayerObject::isAdmin() 
		&& !creatureObject->isPassiveRevealPlayerCharacter(Game::getPlayerNetworkId())
		&& !isStealthEffect(clientEffectName.getString())
	)
	{
		return false;
	}


	ClientEffect* effect = ClientEffectTemplateList::createClientEffect(clientEffectName, object, transform);
	if(effect)
	{
		effect->setLabel(label);

		CollisionProperty const * const collisionProperty = object->getCollisionProperty();
		if (collisionProperty)
		{
			float const collisionRadius = collisionProperty->getBoundingSphere_l().getRadius();
			float const collisionScale = collisionRadius * s_baseObjectHeightMultipler;
			float const effectScale = 0.5f + (0.5f * collisionScale); 
	
			effect->setUniformScale(effectScale);
		}

		effect->execute();
		delete effect;
		return true;
	}
	else
	{
		DEBUG_WARNING(true, ("could not create ClientEffect for file %s, bad filename maybe?", clientEffectName.getString()));
		return false;
	}
}

// ----------------------------------------------------------------------

/** Play the given effect at the given point in space.
 */
bool ClientEffectManager::playClientEffect(const CrcLowerString& clientEffectName, const CellProperty* cell, const Vector& location, const Vector& up)
{
	return playLabeledClientEffect(clientEffectName, cell, location, up, CrcLowerString::empty);
}

bool ClientEffectManager::playLabeledClientEffect(const CrcLowerString& clientEffectName, const CellProperty* cell, const Vector& location, const Vector& up, CrcLowerString const & label)
{
	DEBUG_FATAL(!ms_installed, ("not installed"));

	if (!cell)
	{
		DEBUG_WARNING (true, ("ClientEffectManager::playClientEffect [%s]: NULL cell specified", clientEffectName.getString()));
		return false;
	}

	ClientEffect* effect = ClientEffectTemplateList::createClientEffect(clientEffectName, cell, location, up);
	if(effect)
	{
		effect->setLabel(label);
		effect->execute();
		delete effect;
		return true;
	}
	else
	{
		DEBUG_WARNING(true, ("could not create ClientEffect for file %s, bad filename maybe?", clientEffectName.getString()));
		return false;
	}
}

// ----------------------------------------------------------------------

/** Hand lifetime ownership of this light to the ClientEffectManager.  After <time> has elapsed, it will be cleaned up.
 */
void ClientEffectManager::addManagedLight(Light* light, Object* owner, const CrcLowerString& hardpoint, Transform const & transform, float time, CrcLowerString &label)
{
	DEBUG_FATAL(!ms_installed, ("not installed"));
	ManagedLight * const ml = new ManagedLight;
	ml->light        = light;
	ml->owner        = owner;
	ml->hardpoint    = hardpoint;
	ml->time         = time;
	ml->transform    = transform;
	ml->label        = label; // used to remove client effects by name
	m_lights.push_back(ml);
}

// ----------------------------------------------------------------------

/** Hand lifetime ownership of this particle system to the ClientEffectManager.  After <time> has elapsed, it will be cleaned up.
 */
void ClientEffectManager::addManagedParticleSystem(Object* particleSystem, Object* owner, const CrcLowerString& hardpoint, Transform const & transform, float time, bool softParticleTerminate, bool ignoreDuration, CrcLowerString &label)
{
	DEBUG_FATAL(!ms_installed, ("not installed"));
	ManagedParticleSystem * const mps = new ManagedParticleSystem;
	mps->particleSystem        = particleSystem;
	mps->owner                 = owner;
	mps->hardpoint             = hardpoint;
	mps->transform             = transform;
	mps->softParticleTerminate = softParticleTerminate;
	mps->label                 = label; // used to remove client effects by name
	mps->ignoreDuration        = mps->label == CrcLowerString::empty ? false : ignoreDuration; // if the name is empty and ignore is true, this particle would never be deleted until the owner was destroyed
	mps->time                  = (mps->label == CrcLowerString::empty && time <= 0.f) ? s_defaultEffectTime : time; // this is a workaround for one shot cef files that only contain a particle (and therefore no duration)
	m_particleSystems.push_back(mps);
}

// ----------------------------------------------------------------------

/** Hand lifetime ownership of this shader to the ClientEffectManager.  After <time> has elapsed, it will be cleaned up.
 */
void ClientEffectManager::addManagerShader(Shader* shader, float time)
{
	DEBUG_FATAL(!ms_installed, ("not installed"));
	m_shaders.push_back(std::make_pair(time, shader));
}

// ----------------------------------------------------------------------

/** Update loop for the ClientEffectManager. Countdown the elapsed time and delete any expired objects that we manage.
 */
void ClientEffectManager::sendHeartbeat(const float elapsedTime)
{
	DEBUG_FATAL(!ms_installed, ("not installed"));

	//manually update each particle system if attached to a hardpoint
	for(ParticleList::iterator i = m_particleSystems.begin(); i != m_particleSystems.end(); ++i)
	{
		ManagedParticleSystem const & mps = *NON_NULL(*i);
		if (!mps.owner || !mps.particleSystem)
			continue;

		//look for the hardpoint if needed
		Transform hardpointToObjectTransform;
		Appearance* app = mps.owner->getAppearance();
		bool hpFound = false;
		if(!mps.hardpoint.isEmpty())
		{
			hpFound = app->findHardpoint(mps.hardpoint, hardpointToObjectTransform);
			if (!hpFound)
				continue;
		}
		else
			hardpointToObjectTransform = mps.transform;

		mps.particleSystem->setPosition_p(Vector(0.0f, 0.0f, 0.0f));
		mps.particleSystem->setTransform_o2p(hardpointToObjectTransform);
		mps.particleSystem->setTransform_a2w(hardpointToObjectTransform);
	}

	//manually update each light if attached to a hardpoint
	for(LightList::iterator i2 = m_lights.begin(); i2 != m_lights.end(); ++i2)
	{
		ManagedLight const & ml = *NON_NULL(*i2);
		if (!ml.owner || !ml.light)
			continue;

		//look for the hardpoint if needed
		Transform hardpointToObjectTransform;
		Appearance* app = ml.owner->getAppearance();
		bool hpFound = false;
		if(!ml.hardpoint.isEmpty())
		{
			hpFound = app->findHardpoint(ml.hardpoint, hardpointToObjectTransform);

			//if we're given a hardpoint that we can't find, abort this effect
			if(!hpFound)
			{
				continue;
			}
		}
		else
			hardpointToObjectTransform = ml.transform;

		ml.light->setPosition_p(Vector(0.0f, 0.0f, 0.0f));
		ml.light->setTransform_o2p(hardpointToObjectTransform);
	}

	if (!m_lights.empty())
	{
		uint n = m_lights.size ();
		uint i = 0;
		while (i < n)
		{
			ManagedLight* const ml = m_lights [i];
			ml->time -= elapsedTime;

			if (!ml->light || ml->time <= 0.f)
			{
				if (ml->light && ml->light->isInWorld())
				{
					ClientWorld::removeLight(ml->light);
					delete ml->light;
				}

				delete ml;
				m_lights.erase (m_lights.begin () + i);

				--n;
			}
			else
				++i;
		}
	}

	if (!m_particleSystems.empty())
	{
		uint n = m_particleSystems.size ();
		uint i = 0;
		while (i < n)
		{
			ManagedParticleSystem* const mps = m_particleSystems [i];
			mps->time -= elapsedTime;

			bool shouldDelete = false;

			if ((mps->time <= 0.f) && (!mps->ignoreDuration))
			{
				if(mps->softParticleTerminate)
				{
					//New behavior - disable, then wait for terminate
					if(mps->particleSystem)
					{
						ParticleEffectAppearance *particleSystemAppearance = ParticleEffectAppearance::asParticleEffectAppearance(mps->particleSystem->getAppearance());
						if(particleSystemAppearance)
						{
							if(particleSystemAppearance->isEnabled())
								particleSystemAppearance->setEnabled(false);
							else
							{
								if (mps->time <= -s_maxAllowedTime)
								{
									if (particleSystemAppearance->isInfiniteLooping())
										DEBUG_WARNING(true, ("Particle system [%s] was killed by client effect because it was soft particle terminated but still infinite looping", particleSystemAppearance->getAppearanceTemplateName()));
									else 
										DEBUG_WARNING(true, ("Particle system [%s] was killed by client effect but still exists %1.0f seconds later", particleSystemAppearance->getAppearanceTemplateName(), s_maxAllowedTime));

									shouldDelete = true;
								}
								else 
									if(particleSystemAppearance->isDeletable())
										shouldDelete = true;
							}
						}
						else
							shouldDelete = true;
					}
					else
						shouldDelete = true;
				}
				else
					shouldDelete = true;
			}
			else if (mps->particleSystem && mps->particleSystem->getAppearance())
			{
				shouldDelete = !(mps->particleSystem->getAppearance()->isAlive());
			}

			if (shouldDelete)
			{
				delete mps->particleSystem;
				delete mps;
				m_particleSystems.erase (m_particleSystems.begin () + i);

				--n;
			}
			else
				++i;
		}
	}
}

// ----------------------------------------------------------------------

void ClientEffectManager::removeAllClientEffectsForObject(Object const * const object)
{
	DEBUG_FATAL(!ms_installed, ("not installed"));

	for(ParticleList::iterator i = m_particleSystems.begin(); i != m_particleSystems.end();i++)
	{
		ManagedParticleSystem * const mps = *i;

		if (mps && (mps->owner == object))
		{
			// don't immediately delete the particles
			// force them to soft terminate instead
			mps->time = 0.f;
			mps->softParticleTerminate = true;
			mps->ignoreDuration = false;
		}
	}

	for(LightList::iterator i2 = m_lights.begin(); i2 != m_lights.end();/**/)
	{
		ManagedLight const * ml = (*i2);
		if (ml && (ml->owner == object))
		{
			if (ml->light)
			{
				if ( ml->light->isInWorld())
				{
					ml->light->removeFromWorld();
				}
				delete ml->light;
			}

			delete ml;

			i2 = m_lights.erase(i2);
		}
		else
			++i2;
	}

}

// ----------------------------------------------------------------------

void ClientEffectManager::removeAllClientEffectsForObjectByLabel(Object const * const object, CrcLowerString const & label, bool softTerminate)
{
	DEBUG_FATAL(!ms_installed, ("not installed"));

	for(ParticleList::iterator i = m_particleSystems.begin(); i != m_particleSystems.end(); )
	{
		ManagedParticleSystem * const mps = *i;

		if (mps && (mps->owner == object) && (mps->label == label))
		{
			if (softTerminate)
			{
				// don't immediately delete the particles
				// force them to soft terminate instead
				mps->time = 0.f;
				mps->softParticleTerminate = true;
				mps->ignoreDuration = false;
			}
			else
			{
				if (mps->particleSystem)
				{
					if (mps->particleSystem->isInWorld())
						mps->particleSystem->removeFromWorld();

					delete mps->particleSystem;
				}

				delete mps;
				i = m_particleSystems.erase(i);

				continue;
			}
		}

		++i;
	}

	for(LightList::iterator i2 = m_lights.begin(); i2 != m_lights.end();/**/)
	{
		ManagedLight const * ml = (*i2);
		if (ml && (ml->owner == object))
		{
			if (ml->light && (ml->label == label))
			{
				if ( ml->light->isInWorld())
				{
					ml->light->removeFromWorld();
				}
				delete ml->light;
			}

			delete ml;

			i2 = m_lights.erase(i2);
		}
		else
			++i2;
	}
}

// ----------------------------------------------------------------------
void ClientEffectManager::removeNonStealthClientEffects(Object const * const object)
{
	DEBUG_FATAL(!ms_installed, ("not installed"));

	for(ParticleList::iterator i = m_particleSystems.begin(); i != m_particleSystems.end(); )
	{
		ManagedParticleSystem * const mps = *i;

		if (mps && (mps->owner == object))
		{
			if (mps->particleSystem)
			{

				ParticleEffectAppearance *particleSystemAppearance = ParticleEffectAppearance::asParticleEffectAppearance(mps->particleSystem->getAppearance());
				if(particleSystemAppearance && isStealthEffect(particleSystemAppearance->getAppearanceTemplateName()))
				{
					++i;
					continue;
				}

				if (mps->particleSystem->isInWorld())
					mps->particleSystem->removeFromWorld();

				delete mps->particleSystem;
			}

			delete mps;
			i = m_particleSystems.erase(i);

			continue;
		}

		++i;
	}

	for(LightList::iterator i2 = m_lights.begin(); i2 != m_lights.end();/**/)
	{
		ManagedLight const * ml = (*i2);
		if (ml && (ml->owner == object))
		{
			if (ml->light)
			{
				if ( ml->light->isInWorld())
				{
					ml->light->removeFromWorld();
				}
				delete ml->light;
			}

			delete ml;

			i2 = m_lights.erase(i2);
		}
		else
			++i2;
	}
}

// ----------------------------------------------------------------------

/**
* Transfer all client effects attached to a specified object.  The client effects are now owned by the new owner.
* If a partition is specified, only client effects which lie on the front of the partition are transfered.  This method
* reparents children, keeping the same o2p transform.
*/

void ClientEffectManager::transferOwnerShipOfAllClientEffects(Object & previousOwner, Object & newOwner, Plane const * const partition, bool reparentObjects)
{
	if (&newOwner == &previousOwner)
		return;

	{
		for (ParticleList::iterator i = m_particleSystems.begin(); i != m_particleSystems.end(); ++i)
		{
			ManagedParticleSystem & mps = *NON_NULL(*i);
			if (&previousOwner != mps.owner.getPointer() || NULL == mps.particleSystem.getPointer())
				continue;

			if (NULL != partition)
			{
				Vector const & mpsPos_p = mps.transform.getPosition_p();
				float const distanceTo = partition->computeDistanceTo(mpsPos_p);

				if (distanceTo < 0.0f)
					continue;
			}

			mps.owner = &newOwner;

			if (reparentObjects)
			{

				Object * const childObject = mps.particleSystem.getPointer();
				if (NULL != childObject)
				{
					if (childObject->getParent() == &previousOwner)
					{
						ParticleEffectAppearance * const pea = ParticleEffectAppearance::asParticleEffectAppearance(childObject->getAppearance());
						if (NULL != pea)
							pea->setRestartOnRemoveFromWorld(false);

						previousOwner.removeChildObject(childObject, Object::DF_none);
						newOwner.addChildObject_o(childObject);

						if (NULL != pea)
							pea->setRestartOnRemoveFromWorld(true);
					}
				}
			}
		}
	}

	{
		for (LightList::iterator i = m_lights.begin(); i != m_lights.end(); ++i)
		{
			ManagedLight & ml = *NON_NULL(*i);
			if (&previousOwner != ml.owner.getPointer() || NULL == ml.light.getPointer())
				continue;

			if (NULL != partition)
			{
				Vector const & mlPos_p = ml.transform.getPosition_p();
				float const distanceTo = partition->computeDistanceTo(mlPos_p);

				if (distanceTo < 0.0f)
					continue;
			}

			ml.owner = &newOwner;

			if (reparentObjects)
			{
				Object * const childObject = ml.light.getPointer();
				if (NULL != childObject)
				{
					if (childObject->getParent() == &previousOwner)
					{
						previousOwner.removeChildObject(childObject, Object::DF_none);
						newOwner.addChildObject_o(childObject);
					}
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

bool ClientEffectManager::isStealthEffect(const char *effectName)
{
	if (effectName)
	{
		std::string name(effectName);
	
		if (name == cs_enterStealthEffect || name == cs_exitStealthEffect)
			return true;
	}

	return false;
}

// ----------------------------------------------------------------------

void ClientEffectManager::createClientNonTrackingProjectile(std::string const & weaponObjectTemplateName, CellProperty const * const cellProperty, Vector const & startLoc, Vector const & endLoc, float const speed, float const expiration, bool const hasTrail, uint32 const trailArgb)
{
	if (!cellProperty)
		return;

	ClientWeaponObjectTemplate const * weaponObjectTemplate = static_cast<ClientWeaponObjectTemplate const *>(ObjectTemplateList::fetch(weaponObjectTemplateName));

	if (!weaponObjectTemplate)
		return;

	Projectile * const projectile = weaponObjectTemplate->createProjectile(true, false);

	if (projectile)
	{
		projectile->setExpirationTime(expiration);
		projectile->setParentCell(const_cast<CellProperty*>(cellProperty));

		CellProperty::setPortalTransitionsEnabled(false);
		projectile->setPosition_p(cellProperty->getOwner().rotateTranslate_p2o(startLoc));
		projectile->lookAt_o(projectile->rotateTranslate_p2o(endLoc));
		CellProperty::setPortalTransitionsEnabled(true);

		projectile->setSpeed(speed);
		projectile->setTarget(Game::getPlayer());
		projectile->addToWorld();

		if (hasTrail)
		{
			Object* const trail = new Object ();
			trail->addNotification(ClientWorld::getIntangibleNotification());
			RenderWorld::addObjectNotifications(*trail);
			trail->setParentCell(const_cast<CellProperty*> (projectile->getParentCell()));
			trail->setTransform_o2p(projectile->getTransform_o2p ());
			trail->setAppearance(new TrailAppearance (projectile, 0.05f, PackedArgb(trailArgb)));
			trail->addToWorld();
		}
	}

	weaponObjectTemplate->releaseReference();
}

// ----------------------------------------------------------------------

void ClientEffectManager::createClientTrackingProjectileObjectToObject(std::string const & weaponObjectTemplateName, CellProperty const * const cellProperty, Object const & source, std::string const & sourceHardpoint, Object const & target, std::string const & targetHardpoint, float const speed, float const expiration, bool hasTrail, uint32 const trailArgb)
{
	if (!cellProperty)
		return;

	ClientWeaponObjectTemplate const * weaponObjectTemplate = static_cast<ClientWeaponObjectTemplate const *>(ObjectTemplateList::fetch(weaponObjectTemplateName));

	if (!weaponObjectTemplate)
		return;

	Projectile * const projectile = weaponObjectTemplate->createProjectile(false, false);

	if (projectile)
	{
		TrackingProjectile * const trackingProjectile = safe_cast<TrackingProjectile*>(projectile);

		trackingProjectile->setExpirationTime(expiration);
		trackingProjectile->setParentCell(const_cast<CellProperty*>(cellProperty));

		Object const * saddle = SaddleManager::getSaddleObjectFromMount(source);

		Vector startLoc;
		if(sourceHardpoint.empty())
			startLoc = saddle ? saddle->getPosition_p() : source.getPosition_p();
		else
		{
			Appearance const * const sourceAppearance = saddle ? saddle->getAppearance() : source.getAppearance();

			if (sourceAppearance != NULL)
			{
				Transform hardPointTransform;
				CrcLowerString const hardpointCrc(sourceHardpoint.c_str());
				if (sourceAppearance->findHardpoint(hardpointCrc, hardPointTransform))
				{
					startLoc = saddle ? saddle->rotateTranslate_o2w(hardPointTransform.getPosition_p()) : source.rotateTranslate_o2w(hardPointTransform.getPosition_p());		 
				}
			}
		}

		CellProperty::setPortalTransitionsEnabled(false);
		trackingProjectile->setPosition_p(cellProperty->getOwner().rotateTranslate_p2o(startLoc));
		Vector lookAt = target.getPosition_w();
		lookAt.y += target.getCollisionSphereExtent_w().getRadius();
		trackingProjectile->lookAt_o(projectile->rotateTranslate_p2o(lookAt));
		CellProperty::setPortalTransitionsEnabled(true);

		Object const * targetSaddle = SaddleManager::getSaddleObjectFromMount(target);

		trackingProjectile->setSpeed(speed);
		trackingProjectile->setTarget( targetSaddle ? targetSaddle : &target);
		
		if(!targetHardpoint.empty())
		{
			CrcLowerString const targetHardpointCrc(targetHardpoint.c_str());
			trackingProjectile->setTargetHardpoint(targetHardpointCrc);
		}

		trackingProjectile->addToWorld();

		if (hasTrail)
		{
			Object* const trail = new Object ();
			trail->addNotification (ClientWorld::getIntangibleNotification ());
			RenderWorld::addObjectNotifications (*trail);
			trail->setParentCell (const_cast<CellProperty*> (trackingProjectile->getParentCell ()));
			trail->setTransform_o2p (trackingProjectile->getTransform_o2p ());
			trail->setAppearance (new TrailAppearance (trackingProjectile, 0.05f, PackedArgb(trailArgb)));
			trail->addToWorld ();
			trackingProjectile->addChildObject_p(trail);
		}
	}

	weaponObjectTemplate->releaseReference();
}

// ----------------------------------------------------------------------

void ClientEffectManager::createClientTrackingProjectileObjectToLocation(const std::string &weaponObjectTemplateName, const CellProperty *const cellProperty, const Object &source, const std::string &sourceHardpoint, const Vector &endLoc, const float speed, const float expiration, const bool hasTrail, const uint32 trailArgb)
{
	if (!cellProperty)
		return;

	ClientWeaponObjectTemplate const * weaponObjectTemplate = static_cast<ClientWeaponObjectTemplate const *>(ObjectTemplateList::fetch(weaponObjectTemplateName));

	if (!weaponObjectTemplate)
		return;

	Projectile * const projectile = weaponObjectTemplate->createProjectile(true, false);

	if (projectile)
	{
		projectile->setExpirationTime(expiration);
		projectile->setParentCell(const_cast<CellProperty*>(cellProperty));

		Object const * saddle = SaddleManager::getSaddleObjectFromMount(source);

		Vector startLoc;
		if(sourceHardpoint.empty())
			startLoc = saddle ? saddle->getPosition_p() : source.getPosition_p();
		else
		{
			Appearance const * const sourceAppearance = saddle ? saddle->getAppearance() : source.getAppearance();

			if (sourceAppearance != NULL)
			{
				Transform hardPointTransform;
				CrcLowerString const hardpointCrc(sourceHardpoint.c_str());
				if (sourceAppearance->findHardpoint(hardpointCrc, hardPointTransform))
				{
					startLoc = saddle ? saddle->rotateTranslate_o2w(hardPointTransform.getPosition_p()) : source.rotateTranslate_o2w(hardPointTransform.getPosition_p());		 
				}
			}
		}

		CellProperty::setPortalTransitionsEnabled(false);
		projectile->setPosition_p(cellProperty->getOwner().rotateTranslate_p2o(startLoc));
		projectile->lookAt_o(projectile->rotateTranslate_p2o(endLoc));
		CellProperty::setPortalTransitionsEnabled(true);

		projectile->setSpeed(speed);
		projectile->setTarget(Game::getPlayer());
		projectile->addToWorld();

		if (hasTrail)
		{
			Object* const trail = new Object ();
			trail->addNotification(ClientWorld::getIntangibleNotification());
			RenderWorld::addObjectNotifications(*trail);
			trail->setParentCell(const_cast<CellProperty*> (projectile->getParentCell()));
			trail->setTransform_o2p(projectile->getTransform_o2p ());
			trail->setAppearance(new TrailAppearance (projectile, 0.05f, PackedArgb(trailArgb)));
			trail->addToWorld();
		}
	}

	weaponObjectTemplate->releaseReference();
}

// ----------------------------------------------------------------------

void ClientEffectManager::createClientTrackingProjectileLocationToObject(const std::string &weaponObjectTemplateName, const CellProperty *const cellProperty, const Vector &startLoc, const Object &target, const std::string &targetHardpoint, const float speed, const float expiration, const bool hasTrail, const uint32 trailArgb)
{
	if (!cellProperty)
		return;

	ClientWeaponObjectTemplate const * weaponObjectTemplate = static_cast<ClientWeaponObjectTemplate const *>(ObjectTemplateList::fetch(weaponObjectTemplateName));

	if (!weaponObjectTemplate)
		return;

	Projectile * const projectile = weaponObjectTemplate->createProjectile(false, false);

	if (projectile)
	{
		projectile->setExpirationTime(expiration);
		projectile->setParentCell(const_cast<CellProperty*>(cellProperty));
		
		TrackingProjectile * const trackingProjectile = safe_cast<TrackingProjectile*>(projectile);
		
		CellProperty::setPortalTransitionsEnabled(false);
		trackingProjectile->setPosition_p(cellProperty->getOwner().rotateTranslate_p2o(startLoc));
		Vector lookAt = target.getPosition_w();
		lookAt.y += target.getCollisionSphereExtent_w().getRadius();
		trackingProjectile->lookAt_o(projectile->rotateTranslate_p2o(lookAt));
		CellProperty::setPortalTransitionsEnabled(true);

		Object const * targetSaddle = SaddleManager::getSaddleObjectFromMount(target);

		trackingProjectile->setSpeed(speed);
		trackingProjectile->setTarget( targetSaddle ? targetSaddle : &target);

		if(!targetHardpoint.empty())
		{
			CrcLowerString const targetHardpointCrc(targetHardpoint.c_str());
			trackingProjectile->setTargetHardpoint(targetHardpointCrc);
		}

		trackingProjectile->addToWorld();

		if (hasTrail)
		{
			Object* const trail = new Object ();
			trail->addNotification(ClientWorld::getIntangibleNotification());
			RenderWorld::addObjectNotifications(*trail);
			trail->setParentCell(const_cast<CellProperty*> (trackingProjectile->getParentCell()));
			trail->setTransform_o2p(trackingProjectile->getTransform_o2p ());
			trail->setAppearance(new TrailAppearance (trackingProjectile, 0.05f, PackedArgb(trailArgb)));
			trail->addToWorld();
		}
	}

	weaponObjectTemplate->releaseReference();
}

// ======================================================================
