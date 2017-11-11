// ============================================================================
//
// ClientAsteroidManager.cpp
// Copyright Sony Online Entertainment, Inc.
//
// ============================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientAsteroidManager.h"

#include "clientAudio/Audio.h"
#include "clientGame/AsteroidDynamics.h"
#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientEffect.h"
#include "clientGame/ClientEffectTemplate.h"
#include "clientGame/ClientEffectTemplateList.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/Game.h"
#include "clientGame/ShipObject.h"
#include "clientGraphics/Camera.h"
#include "clientGraphics/DebugPrimitive.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/RenderWorld.h"
#include "sharedCollision/BoxExtent.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedDebug/PerformanceTimer.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/Exitchain.h"
#include "sharedFoundation/Timer.h"
#include "sharedGame/AsteroidGenerationManager.h"
#include "sharedGame/ConfigSharedGame.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedMath/Sphere.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/Object.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/RotationDynamics.h"

#include "clientGame/ClientObject.h"

#include <algorithm>
#include <vector>

// ============================================================================

namespace ClientAsteroidManagerNamespace
{
	// Classes

	struct RadialAsteroidTracker
	{
		Watcher<Object>                    m_rock;
		float const                        m_spawnRange;
		float const                        m_deleteRange;
		Vector                             m_relativePosition; // position in the space of the player's ship/topmost container
		bool                               m_hitLastFrame;
		ClientEffectTemplate const * const m_hitEffect;

		RadialAsteroidTracker(Object *asteroid, float spawnRange, float deleteRange, const Vector & relativePosition, ClientEffectTemplate const *hitEffect);

	private:
		RadialAsteroidTracker(); // disable
		RadialAsteroidTracker & operator=(const RadialAsteroidTracker &); //disable
	};

	struct ClientAsteroidFieldData : public AsteroidGenerationManager::AsteroidFieldData
	{
		std::vector<AsteroidGenerationManager::AsteroidData> const m_asteroidData;
		std::vector<Watcher<ClientObject> >                        m_asteroidObjects;
		float                                                      m_percentInstantiated;
		std::vector<const ClientEffectTemplate*>                   m_radialHitEffectTemplates;

		ClientAsteroidFieldData  (AsteroidGenerationManager::AsteroidFieldData const & sharedData, std::vector<AsteroidGenerationManager::AsteroidData> const & asteroidData);
		~ClientAsteroidFieldData (); //lint !e1509 // base class destructor not virtual (don't want to push virtual functions into AsteroidGenerationManager::AsteroidFieldData)

	private:
		ClientAsteroidFieldData(); //disable
		ClientAsteroidFieldData & operator=(const ClientAsteroidFieldData &); //disable
	};

	// Typedefs

	typedef int                                  FieldHandle;
	typedef std::vector<RadialAsteroidTracker*>  RadialAsteroidsType;

	// Functions

	void  doCleanup              ();
	void  enterAsteroidField     (FieldHandle const fieldId);
	void  generateField          (FieldHandle const handle, AsteroidGenerationManager::AsteroidFieldData const &fieldData);
	float getDistanceToField     (FieldHandle const fieldId);
	float getDistanceToField     (Vector const & loc, AsteroidGenerationManager::AsteroidFieldData const & fieldData);
	float getExtentRadius        (uint32 sharedTemplateCrc);
	void  instantiateField       (FieldHandle const handle, float const percentToCreate);
	float isInField              (Vector const & position_w, AsteroidGenerationManager::AsteroidFieldData const & fieldData);
	void  leaveAsteroidField     ();
	void  makeNewRadialAsteroids (size_t howMany);
	void  moveRocks              ();

	// Data

	ClientAsteroidFieldData const *                 ms_currentAsteroidFieldData;
	bool                                            ms_currentListenForServerAsteroidData = false;
	float const                                     ms_enterFieldRange = 100.0f; //how far from asteroid fields to start generating radial rocks
	std::map<FieldHandle, ClientAsteroidFieldData*> ms_fieldData;
	FieldHandle                                     ms_fieldsEnd = 0;
	FieldHandle const                               ms_fieldsStart = 0;
	bool                                            ms_inAsteroidField = false;
	bool                                            ms_installed = false;
	Timer                                           ms_instantiateTimer(2.0f);
	bool                                            ms_listenForServerAsteroidData = false;
	size_t const                                    ms_maxRadialCreatesPerFrame=20;
	float const                                     ms_minRadialFlyawaySpeed=10.0f;
	RadialAsteroidsType                             ms_radialAsteroids;
	std::vector<Sphere>                             ms_serverAsteroidData;
	size_t                                          ms_targetNumRadialAsteroids = 0;
	SoundId *                                       ms_currentSoundId = NULL;
	bool                                            ms_playingSound = false;
}

using namespace ClientAsteroidManagerNamespace;

// ============================================================================

void ClientAsteroidManager::install()
{
	InstallTimer const installTimer("ClientAsteroidManager::install");

	DEBUG_FATAL(ms_installed, ("ClientAsteroidManager::install() - Already installed."));

	ms_installed = true;

	ms_listenForServerAsteroidData = false;
	ms_currentListenForServerAsteroidData = false;

	AsteroidGenerationManager::registerGetExtentRadiusFunction(ClientAsteroidManagerNamespace::getExtentRadius);

	DebugFlags::registerFlag (ms_listenForServerAsteroidData,     "ClientGame/ClientAsteroidManager", "renderServerAsteroidData");

	ExitChain::add(ClientAsteroidManager::remove, "ClientAsteroidManager::remove", 0, false);

	ms_currentSoundId = new SoundId();
}

//-----------------------------------------------------------------------------

void ClientAsteroidManager::remove()
{
	delete ms_currentSoundId;
	ms_currentSoundId = NULL;
	DebugFlags::unregisterFlag (ms_listenForServerAsteroidData);
	doCleanup();
	ms_installed = false;
}

//-----------------------------------------------------------------------------

/**
 * Reset the ClientAsteroidManager to its initial state and remove any radial
 * asteroids that were waiting to be deleted.  Call this when changing scenes or
 * exiting.
 */
void ClientAsteroidManagerNamespace::doCleanup()
{
	ms_currentAsteroidFieldData = NULL;
	{
		for (std::map<FieldHandle, ClientAsteroidFieldData*>::iterator i=ms_fieldData.begin(); i!=ms_fieldData.end(); ++i)
		{
			delete i->second;
		}
		ms_fieldData.clear();
	}
	ms_fieldsEnd=0;
	ms_inAsteroidField=false;
	{
		for (RadialAsteroidsType::iterator i=ms_radialAsteroids.begin(); i!=ms_radialAsteroids.end(); ++i)
		{
			NOT_NULL(*i);
			if ((*i)->m_rock.getPointer())
			{
				delete ((*i)->m_rock.getPointer());
			}
			delete *i;
		}
		ms_radialAsteroids.clear();
	}
	ms_targetNumRadialAsteroids = 0;
}

//-----------------------------------------------------------------------------

/**
 * Create the data for an asteroid field and store its data
 */
void ClientAsteroidManagerNamespace::generateField(FieldHandle const handle, AsteroidGenerationManager::AsteroidFieldData const &fieldData)
{
	if(!ms_installed)
	{
		DEBUG_FATAL (true, ("ClientAsteroidManager not installed"));
		return; //lint !e527 unreachable (reachable in release)
	}

	std::vector<AsteroidGenerationManager::AsteroidData> asteroidData;
	bool const result = AsteroidGenerationManager::generateField(fieldData,asteroidData);

	if(!result)
	{
		DEBUG_FATAL(true, ("ClientAsteroidManager::generateField problem occured generating field"));
		return; //lint !e527 unreachable (reachable in release)
	}

	if(asteroidData.size() != static_cast<unsigned int>(fieldData.count))
	{
		DEBUG_FATAL(true, ("ClientAsteroidManager::generateField should have generated %d asteroids, but we have %d instead", fieldData.count, asteroidData.size()));
		return; //lint !e527 unreachable (reachable in release)
	}

	ms_fieldData[handle] = new ClientAsteroidFieldData(fieldData, asteroidData);
}

//-----------------------------------------------------------------------------

/**
 * Instantiate the specified percentage of the asteroids in the specifield field.
 * If a higher percentage had already been instantiated, delete some of them.
 */
void ClientAsteroidManagerNamespace::instantiateField(FieldHandle const handle, float const percentToCreate)
{
	NOT_NULL(ms_fieldData[handle]);
	std::vector<AsteroidGenerationManager::AsteroidData> const & asteroidData = ms_fieldData[handle]->m_asteroidData;
	std::vector<Watcher<ClientObject> > &asteroidList=ms_fieldData[handle]->m_asteroidObjects;

	DEBUG_FATAL(asteroidData.size() != asteroidList.size(),("Programmer bug:  in ClientAsteroidManager::instantiateField(%i, %f), asteroidData and asteroidList were not the same size.",handle, percentToCreate));

	Object * o = NULL;
	ClientObject * newAsteroid = NULL;
	RotationDynamics * rotationDynamics = NULL;

	size_t numToCreate = static_cast<size_t>(percentToCreate * static_cast<float>(asteroidData.size()));

	for(size_t i = 0; i != asteroidData.size(); ++i)
	{
		if (i < numToCreate)
		{
			if (!asteroidList[i].getPointer())
			{
				ConstCharCrcString const crcString = ObjectTemplateList::lookUp(asteroidData[i].templateCrc);
				o = ObjectTemplateList::createObject(crcString);
				newAsteroid = o ? o->asClientObject() : NULL;
				if(!newAsteroid)
					continue;

				newAsteroid->endBaselines();
				RenderWorld::addObjectNotifications(*newAsteroid);
				newAsteroid->addToWorld();
				newAsteroid->setPosition_p(asteroidData[i].position);
				newAsteroid->setScale(Vector(asteroidData[i].scale, asteroidData[i].scale, asteroidData[i].scale));
				newAsteroid->yaw_o(asteroidData[i].orientation.x);
				newAsteroid->pitch_o(asteroidData[i].orientation.y);
				newAsteroid->roll_o(asteroidData[i].orientation.z);

				rotationDynamics = new RotationDynamics (newAsteroid, asteroidData[i].rotationVector);
				rotationDynamics->setState (true);
				newAsteroid->setDynamics (rotationDynamics);

				asteroidList[i]=newAsteroid;
			}
		}
		else
		{
			// don't generate this asteroid, delete it if it already exists (e.g. client moved away from the field, so
			// the percentage to generate decreased)
			if (asteroidList[i].getPointer())
			{
				asteroidList[i]->kill();
				asteroidList[i]=NULL;
			}
		}
	}

}	//lint !e429 rotationDynamics not freed (we don't own it)

//-----------------------------------------------------------------------------

/**
 * Generate the static asteroid fields for a given scene
 */
void ClientAsteroidManager::setupStaticFields(std::string const & sceneName)
{
	doCleanup();

	PerformanceTimer timer;
	timer.start();

	AsteroidGenerationManager::loadStaticFieldDataForScene(sceneName, false);
	std::vector<AsteroidGenerationManager::AsteroidFieldData> const & data = AsteroidGenerationManager::getDataForScene(sceneName);
	for(std::vector<AsteroidGenerationManager::AsteroidFieldData>::const_iterator i = data.begin(); i != data.end(); ++i)
	{
		generateField(ms_fieldsEnd, *i);
		++ms_fieldsEnd;
	}

	timer.stop();
	DEBUG_REPORT_LOG(ConfigSharedGame::getSpamAsteroidGenerationData(),("Asteroid generation took %f",timer.getElapsedTime()));
}

//-----------------------------------------------------------------------------

/**
 * Instantiate/move/delete asteroids as needed.
 */
void ClientAsteroidManager::update (float elapsedTime)
{
	if(ms_listenForServerAsteroidData != ms_currentListenForServerAsteroidData) //lint !e731 // comparing bools
	{
		if(ms_listenForServerAsteroidData)
			IGNORE_RETURN(ClientCommandQueue::enqueueCommand("serverAsteroidDataListen", NetworkId::cms_invalid, Unicode::emptyString));
		else
			IGNORE_RETURN(ClientCommandQueue::enqueueCommand("serverasteroidDatatopListening", NetworkId::cms_invalid, Unicode::emptyString));

		ms_currentListenForServerAsteroidData = ms_listenForServerAsteroidData;
	}

#ifdef _DEBUG
	Camera const * const camera = Game::getCamera();
	for(std::vector<Sphere>::iterator i = ms_serverAsteroidData.begin(); i != ms_serverAsteroidData.end(); ++i)
	{
		camera->addDebugPrimitive(new SphereDebugPrimitive(UtilityDebugPrimitive::S_none, Transform::identity, i->getCenter(), i->getRadius(), 8, 8));
	}
#endif

	{
		if (ms_radialAsteroids.size() < ms_targetNumRadialAsteroids)
		{
			// create only a limited number of radials asteroid per frame, to limit stuttering as you enter an asteroid field
			makeNewRadialAsteroids(std::min(ms_maxRadialCreatesPerFrame, ms_targetNumRadialAsteroids - ms_radialAsteroids.size()));
		}

		if (ms_radialAsteroids.size() != 0)
			moveRocks();

		if (ms_instantiateTimer.updateZero(elapsedTime))
		{
			bool nowInAsteroidField=false;

			for (FieldHandle fieldId=ms_fieldsStart; fieldId!=ms_fieldsEnd; ++fieldId)
			{
				ClientAsteroidFieldData * const fieldData= ms_fieldData[fieldId];
				NOT_NULL(fieldData);

				float distance = getDistanceToField(fieldId);
				float instantiatePercent = 0;

				if (distance >= 0)
				{
					if (distance < ms_enterFieldRange)
						nowInAsteroidField = true;

					if (distance < fieldData->viewAllDistance)
						instantiatePercent = 1.0f;
					else if (distance > fieldData->maxViewableDistance)
						instantiatePercent = 0.0f;
					else
						instantiatePercent =1.0f - ((distance-fieldData->viewAllDistance) / (fieldData->maxViewableDistance- fieldData->viewAllDistance));
				}

				if ((instantiatePercent > fieldData->m_percentInstantiated) ||
					(abs (instantiatePercent - fieldData->m_percentInstantiated) > .25f) ||
					(instantiatePercent == 0.0f && fieldData->m_percentInstantiated != 0.0f))
				{
					// always add asteroids as the client gets closer.  Only remove them if the client
					// gets farther away by a large enough amount.  (The goal is to avoid creating lots
					// of objects in the same frame, and avoid deleting objects that might get re-created
					// a little while later.)
					instantiateField(fieldId, instantiatePercent);
					fieldData->m_percentInstantiated = instantiatePercent;
				}

				if (!ms_inAsteroidField && nowInAsteroidField)
					enterAsteroidField(fieldId);
			}

			if (ms_inAsteroidField && !nowInAsteroidField)
					leaveAsteroidField();
		}
	}
}

//-----------------------------------------------------------------------------

/**
 * Set the server asteroid data when received from the server
 */
void ClientAsteroidManager::setServerAsteroidData(std::vector<Sphere> const & spheres)
{
	ms_serverAsteroidData.clear();
	for(std::vector<Sphere>::const_iterator i = spheres.begin(); i != spheres.end(); ++i)
	{
		ms_serverAsteroidData.push_back(*i);
	}
}

//-----------------------------------------------------------------------------

float ClientAsteroidManagerNamespace::getExtentRadius(uint32 sharedTemplateCrc)
{
	float radius = 0.0f;
	ObjectTemplate const * const ot = ObjectTemplateList::fetch(sharedTemplateCrc);
	if(ot)
	{
		SharedObjectTemplate const * const sharedOT = ot->asSharedObjectTemplate();
		if(sharedOT)
		{
			AppearanceTemplate const * const at = AppearanceTemplateList::fetch(sharedOT->getAppearanceFilename().c_str());
			if(at)
			{
				Extent const * const extent = at->getCollisionExtent();
				if(extent)
				{
					radius = extent->getSphere().getRadius();
					if (radius==0.0f)
					{
						DEBUG_WARNING(true,("Data problem:  Object template %s has radius 0",
							ObjectTemplateList::lookUp(sharedTemplateCrc).getString()));
						radius=5.0f;
					}
				}
				AppearanceTemplateList::release(at);
			}
		}
		ot->releaseReference();
	}
	return radius;
}

//-----------------------------------------------------------------------------

/**
 * Get the distance between the player and the specified field.
 */
float ClientAsteroidManagerNamespace::getDistanceToField(const FieldHandle fieldId)
{
	bool found = false;

	Vector loc;

	if (Game::isViewFreeCamera())
	{
		Camera * const camera = Game::getCamera();
		if (NULL != camera)
		{
			found = true;
			loc = camera->getPosition_w();
		}
	}

	if (!found)
	{
		Object* player = Game::getPlayer();
		if(!player)
		{
			//this can legitimately happen (quit from a scene, etc.)
			return -1.0f;
		}

		loc = player->getPosition_w();
	}

	ClientAsteroidFieldData const * fieldData = ms_fieldData[fieldId];
	NOT_NULL(fieldData);

	return getDistanceToField(loc, *fieldData);
}

//-----------------------------------------------------------------------------

/**
 * Get the distance between a point and the edge of the specified field.
 * This is not exact for square fields, but close enough.  If we add different
 * shapes of fields, this may need to change.
 */
float ClientAsteroidManagerNamespace::getDistanceToField(Vector const & loc, AsteroidGenerationManager::AsteroidFieldData const & fieldData)
{
	float result = 0.0f;

	switch (fieldData.fieldType)
	{
		case AsteroidGenerationManager::AsteroidFieldData::FT_cube:
			result = loc.magnitudeBetween(fieldData.centerPosition) - fieldData.radius;
			break;

		case AsteroidGenerationManager::AsteroidFieldData::FT_spline:
		{
			// distance to field is considered to be the shortest distance to any of the segments
			result = 8000.0f;
			std::vector<Vector>::const_iterator current = fieldData.splineControlPoints.begin();
			std::vector<Vector>::const_iterator next = current;
			DEBUG_FATAL(next == fieldData.splineControlPoints.end(),("SplineControlPoints requires at least two entries (this should have already been checked by AsteroidGenerationManager."));
			++next;
			while (next != fieldData.splineControlPoints.end())
			{
				result = std::min(result, loc.distanceToLineSegment(*current, *next));
				current = next;
				++next;
			}
			result -= fieldData.radius; // result is distance to edge, not distance to center
			DEBUG_REPORT_LOG(ConfigSharedGame::getSpamAsteroidGenerationData(),("Distance to spline field:  %f\n",result));
			break;
		}
	}

	return result >= 0.0f ? result : 0.0f;
}

//-----------------------------------------------------------------------------

/**
 * Returns true if the specified point is inside an asteroid field.
 */
float ClientAsteroidManagerNamespace::isInField(Vector const & position_w, AsteroidGenerationManager::AsteroidFieldData const & fieldData)
{
	switch (fieldData.fieldType)
	{
		case AsteroidGenerationManager::AsteroidFieldData::FT_cube:
			return position_w.magnitudeBetweenSquared(fieldData.centerPosition) < (fieldData.radius * fieldData.radius);

		case AsteroidGenerationManager::AsteroidFieldData::FT_spline:
			return getDistanceToField(position_w, fieldData) <= 0.0f;

		default:
			return false;
	}
}

//-----------------------------------------------------------------------------

/**
 * Called when the client enters an asteroid field.  Sets up the data to start
 * creating radial asteroids.
 */
void ClientAsteroidManagerNamespace::enterAsteroidField(FieldHandle const fieldId)
{
	ms_inAsteroidField = true;
	NOT_NULL(ms_fieldData[fieldId]);
	ms_currentAsteroidFieldData = ms_fieldData[fieldId];
	ms_targetNumRadialAsteroids = static_cast<size_t>(ms_currentAsteroidFieldData->radialCount);
	if (!ms_currentAsteroidFieldData->soundEffect.empty())
	{
		if (ms_playingSound)
			Audio::stopSound(*ms_currentSoundId, 5.0f);
		*ms_currentSoundId = Audio::playSound(ms_currentAsteroidFieldData->soundEffect.c_str(), NULL);
		ms_playingSound = true;
	}
}

//-----------------------------------------------------------------------------

/**
 * Spawn the specified number of radial asteroids.
 */
void ClientAsteroidManagerNamespace::makeNewRadialAsteroids(size_t howMany)
{
	NOT_NULL(ms_currentAsteroidFieldData);

	Object* player = Game::getPlayer();
	if(!player)
		return;	// wait until we have a player to do any of this

	Object* topmost = ContainerInterface::getTopmostContainer(*player, false);
	if(!topmost)
		return;

	ShipObject const * const ship = Game::getPlayerContainingShip();
	if (!ship)
		return;

	float const shipRadius = ship->getTangibleExtent().getRadius();

	for (; howMany != 0 ; --howMany)
	{
		float choice = Random::randomReal(0.0f, ms_currentAsteroidFieldData->radialLikelihoodSum);
		size_t choiceIndex = 0;
		for (std::vector<AsteroidGenerationManager::RadialAsteroidData>::const_iterator j=ms_currentAsteroidFieldData->radialData.begin();
			j!=ms_currentAsteroidFieldData->radialData.end();
			++j)
		{
			choice -= j->likelihood;
			if (choice <= 0)
			{
				Object *asteroid = new Object();

				float scale = Random::randomReal(j->scaleMin, j->scaleMax);
				asteroid->setScale(Vector(scale, scale, scale));

				{
					Appearance * newAppearance = 0;
					{
						const AppearanceTemplate *const appearanceTemplate = AppearanceTemplateList::fetch(j->appearance.c_str());
						newAppearance = appearanceTemplate->createAppearance();
						AppearanceTemplateList::release(appearanceTemplate);
					}
					asteroid->setAppearance(newAppearance);
					newAppearance=0;
				}

				// pick a random location around the player
				Vector playerLocation_w = player->getPosition_w();
				asteroid->setPosition_p(Vector(
					Random::randomReal(playerLocation_w.x - j->spawnRange, playerLocation_w.x + j->spawnRange),
					Random::randomReal(playerLocation_w.y - j->spawnRange, playerLocation_w.y + j->spawnRange),
					Random::randomReal(playerLocation_w.z - j->spawnRange, playerLocation_w.z + j->spawnRange)));

				// face a random direction
				asteroid->roll_o(Random::randomReal(-PI, PI));
				asteroid->pitch_o(Random::randomReal(0.0f, PI));

				RenderWorld::addObjectNotifications(*asteroid);
				asteroid->addNotification(ClientWorld::getTangibleNotTargetableNotification());

				if (isInField(asteroid->getPosition_w(), *ms_currentAsteroidFieldData))
					asteroid->addToWorld(); // only make visible if the asteroid is inside the field

				Vector const & relativePosition = topmost->rotateTranslate_w2o(asteroid->getPosition_w());
				ClientEffectTemplate const * const clientEffectTemplate = ms_currentAsteroidFieldData->m_radialHitEffectTemplates[choiceIndex];
				float const deleteRange = j->deleteRange + (shipRadius * 2);

				ms_radialAsteroids.push_back(new RadialAsteroidTracker(asteroid, j->spawnRange, deleteRange, relativePosition, clientEffectTemplate));
				break;
			}
			++choiceIndex;
		}
	}
}

//-----------------------------------------------------------------------------

/**
 * Called when the client leaves an asteroid field.  Starts the cleanup process
 * for radial asteroids.
 */
void ClientAsteroidManagerNamespace::leaveAsteroidField()
{
	ms_inAsteroidField = false;
	ms_targetNumRadialAsteroids = 0;
	ms_currentAsteroidFieldData = NULL;
	if (ms_playingSound)
	{
		Audio::stopSound(*ms_currentSoundId, 5.0f);
		*ms_currentSoundId = SoundId::getInvalid();
		ms_playingSound = false;
	}
}

//-----------------------------------------------------------------------------

/**
 * Update all the radial asteroids around the player.  Move or destroy them as
 * needed.
 */
void ClientAsteroidManagerNamespace::moveRocks()
{
	Object* player = Game::getPlayer();
	if (player)
	{
		Object* topmost = ContainerInterface::getTopmostContainer(*player, false);
		if(topmost)
		{
			float shipSpeed = 0.0f;
			if (topmost->asClientObject() && topmost->asClientObject()->asShipObject())
				shipSpeed = topmost->asClientObject()->asShipObject()->getCurrentSpeed();
			float flyawaySpeed = std::max(shipSpeed * 1.2f, ms_minRadialFlyawaySpeed); // speed with which asteroids that are hit fly away

			Vector playerLocation_w = topmost->getPosition_w();
			AxialBox const collisionBox = topmost->getTangibleExtent();
			BoxExtent const extent(collisionBox);

			for (RadialAsteroidsType::iterator i=ms_radialAsteroids.begin(); i!=ms_radialAsteroids.end(); )
			{
				NOT_NULL(*i);
				bool removeThisRock = false;

				Object *asteroid = (*i)->m_rock.getPointer();
				if (asteroid)
				{
					bool bounced = false;
					Vector newRelativePosition_o = topmost->rotateTranslate_w2o(asteroid->getPosition_w());
					if (asteroid->isInWorld())
					{
						float time;
						Vector normal_o;
						if (extent.intersect((*i)->m_relativePosition, newRelativePosition_o, &normal_o, &time))
						{
							if (!(*i)->m_hitLastFrame) // just in case the asteroid gets inside the ship somehow, ignore multiple hits in a row
							{
								// hit the rock.  Kablewy
								Vector const hitPosition_o = Vector::linearInterpolate((*i)->m_relativePosition, newRelativePosition_o, time);
								Vector const hitPosition_w = topmost->rotateTranslate_o2w(hitPosition_o);

								if ((*i)->m_hitEffect)
								{
									ClientEffect * const clientEffect = (*i)->m_hitEffect->createClientEffect(asteroid->getParentCell(), hitPosition_w, Vector::unitY);
									clientEffect->execute();
									delete clientEffect;
								}

								// and send it flying
								asteroid->setPosition_w(hitPosition_w);
								Vector newDirection_o = normal_o.reflectOutgoing((*i)->m_relativePosition - hitPosition_o);
								IGNORE_RETURN(newDirection_o.normalize());

								Vector spin (Random::randomReal(-2.0f, 2.0f),Random::randomReal(-2.0f, 2.0f),Random::randomReal(-2.0f, 2.0f)); // could base it on how you hit the rock, but that would be hard
								AsteroidDynamics *dyn = new AsteroidDynamics(asteroid, topmost->rotate_o2w(newDirection_o) * flyawaySpeed, spin);
								asteroid->setDynamics(dyn);

								asteroid->scheduleForAlter();
								newRelativePosition_o = hitPosition_o;
								bounced=true;
							} //lint !e429 // didn't delete dyn (asteroid takes ownership)
							(*i)->m_hitLastFrame = true;
						}
						else
							(*i)->m_hitLastFrame = false;
					}
					(*i)->m_relativePosition = newRelativePosition_o;

					if ((playerLocation_w - asteroid->getPosition_w()).magnitudeSquared() > ((*i)->m_deleteRange * (*i)->m_deleteRange))
					{
						if (ms_radialAsteroids.size() <= ms_targetNumRadialAsteroids)
						{
							// out of range - move back around in front of the player
							if (asteroid->isInWorld())
								asteroid->removeFromWorld();

							if (asteroid->getDynamics())
							{
								DEBUG_WARNING(bounced,("Programmer bug:  An asteroid bounced of the player ship and went out of range in the same frame.  This may indicate a bug in the collision math."));
								asteroid->setDynamics(NULL); // asteroid was one that was hit and sent flying, stop it from moving because we're pretending it's a new one
							}

							// move to the opposite side of player to keep the distribution even
							Vector adjustment_w(playerLocation_w - asteroid->getPosition_w());
							IGNORE_RETURN(adjustment_w.normalize());
							asteroid->setPosition_w(playerLocation_w);
							asteroid->move_p(adjustment_w * (*i)->m_spawnRange);

							// face a random direction
							asteroid->roll_o(Random::randomReal(-PI, PI));
							asteroid->pitch_o(Random::randomReal(0.0f, PI));

							if (isInField(asteroid->getPosition_w(), *ms_currentAsteroidFieldData))
								asteroid->addToWorld(); // only make visible if the asteroid is inside the field

							(*i)->m_relativePosition = topmost->rotateTranslate_w2o(asteroid->getPosition_w());
						}
						else
						{
							// Too many rocks already, remove this asteroid
							delete asteroid;
							removeThisRock = true;
						}
					}
				}
				else
				{
					// asteroid was deleted by something outside this system, remove it
					removeThisRock = true;
				}

				if (removeThisRock)
				{
					// remove this rock from the list by replacing it with the last one then popping the list
					delete (*i);
					(*i) = ms_radialAsteroids.back();
					ms_radialAsteroids.pop_back();
				}
				else
					++i;
			}
		}
	}
}

//============================================================================

ClientAsteroidManagerNamespace::RadialAsteroidTracker::RadialAsteroidTracker(Object *asteroid, float spawnRange, float deleteRange, const Vector & relativePosition, ClientEffectTemplate const *hitEffect) :
	m_rock(asteroid),
	m_spawnRange(spawnRange),
	m_deleteRange(deleteRange),
	m_relativePosition(relativePosition),
	m_hitLastFrame(false),
	m_hitEffect(hitEffect)
{
}

//============================================================================

ClientAsteroidManagerNamespace::ClientAsteroidFieldData::ClientAsteroidFieldData(AsteroidGenerationManager::AsteroidFieldData const & sharedData,std::vector<AsteroidGenerationManager::AsteroidData> const & asteroidData) :
	AsteroidGenerationManager::AsteroidFieldData(sharedData),
	m_asteroidData(asteroidData),
	m_asteroidObjects(static_cast<size_t>(sharedData.count)),
	m_percentInstantiated(0.0f),
	m_radialHitEffectTemplates()
{
	DEBUG_FATAL(m_asteroidObjects.size() != m_asteroidData.size(),("Programmer bug:  ClientAsteroidManager:  m_asteroidData and m_asteroidObjects were not the same size."));
	// Preload all the hit effects so that they won't stutter when played
	for (std::vector<AsteroidGenerationManager::RadialAsteroidData>::const_iterator i=radialData.begin(); i!=radialData.end(); ++i)
	{
		ClientEffectTemplate const * clientEffectTemplate = ClientEffectTemplateList::fetch(CrcLowerString(i->hitEffectName.c_str()));
		DEBUG_WARNING(clientEffectTemplate == NULL, ("Could not load client effect template for %s", i->hitEffectName.c_str()));
		m_radialHitEffectTemplates.push_back(clientEffectTemplate);
	}
}

//-----------------------------------------------------------------------------

ClientAsteroidManagerNamespace::ClientAsteroidFieldData::~ClientAsteroidFieldData()
{
	for (size_t ai = 0; ai != m_asteroidObjects.size(); ++ai)
	{
		// don't generate this asteroid, delete it if it already exists (e.g. client moved away from the field, so
		// the percentage to generate decreased)
		if (m_asteroidObjects[ai].getPointer())
		{
			m_asteroidObjects[ai]->kill();
			m_asteroidObjects[ai]=NULL;
		}
	}

	// Release the hit effect templates
	for (std::vector<const ClientEffectTemplate*>::const_iterator i=m_radialHitEffectTemplates.begin(); i!=m_radialHitEffectTemplates.end(); ++i)
	{
		if (*i)
			(*i)->release();
	}
}

//============================================================================
