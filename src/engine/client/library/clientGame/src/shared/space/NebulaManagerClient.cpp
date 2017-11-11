//======================================================================
//
// NebulaManagerClient.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/NebulaManagerClient.h"

#include "clientAudio/Audio.h"
#include "clientAudio/SoundId.h"
#include "clientAudio/SoundTemplate.h"
#include "clientDirectInput/ForceFeedbackEffectTemplate.h"
#include "clientDirectInput/ForceFeedbackEffectTemplateList.h"
#include "clientGame/ClientEffect.h"
#include "clientGame/ClientEffectTemplate.h"
#include "clientGame/ClientEffectTemplateList.h"
#include "clientGame/ClientNebula.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientGame/NebulaShellShaderPrimitive.h"
#include "clientGame/NebulaVisualQuad.h"
#include "clientGame/NebulaVisualQuadShaderGroup.h"
#include "clientGame/ShipObject.h"
#include "clientGraphics/Camera.h"
#include "clientGraphics/DebugPrimitive.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientObject/GameCamera.h"
#include "clientParticle/LightningAppearance.h"
#include "clientParticle/ParticleEffectAppearance.h"
#include "clientTerrain/ClientSpaceTerrainAppearance.h"
#include "clientTerrain/GroundEnvironment.h"
#include "clientTerrain/SpaceEnvironment.h"
#include "clientUserInterface/CuiPreferences.h"
#include "sharedCollision/CollideParameters.h"
#include "sharedCollision/CollisionInfo.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedGame/Nebula.h"
#include "sharedGame/NebulaManager.h"
#include "sharedMath/Plane.h"
#include "sharedMath/VectorArgb.h"
#include "sharedNetworkMessages/EnvironmentalHitData.h"
#include "sharedNetworkMessages/NebulaLightningData.h"
#include "sharedNetworkMessages/NebulaLightningHitData.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/CachedNetworkId.h"
#include "sharedObject/CellProperty.h"
#include "sharedRandom/Random.h"
#include "sharedTerrain/TerrainObject.h"
#include <map>
#include <vector>

#include "dpvsModel.hpp"
#include "dpvsObject.hpp"

//======================================================================

namespace NebulaManagerClientNamespace
{

	typedef stdmap<int, SoundId>::fwd NebulaSoundMap;
	NebulaSoundMap s_nebulaSoundMap;

	typedef Watcher<Object> ObjectWatcher;

	typedef stdmap<int, ObjectWatcher>::fwd NebulaObjectMap;
	NebulaObjectMap s_nebulaObjectMap;

	typedef std::pair<NebulaLightningData, ObjectWatcher> LightningObjectPair;
	typedef stdvector<LightningObjectPair>::fwd LightningObjectVector;

	LightningObjectVector s_lightningObjectVector;

	typedef stdmap<CachedNetworkId, float>::fwd NetworkIdFloatMap;
	NetworkIdFloatMap s_objectsRecentlyHitByLightning;

	ObjectWatcher s_worldRenderObject;
	ObjectWatcher s_worldEnvironmentRenderObject;

	bool s_showNebulaExtents = false;
	bool s_lightningEnabled = true;
	bool s_printMetrics = false;
	bool s_flagsInstalled = false;
	int s_numLightningsSpawnedThisFrame = 0;

	//-- lightning data lives for 10 seconds after the lightning expires
	uint32 const s_lightningLifetime = 10000;

	int s_playerEnteredNebulasCount = 0;
	ForceFeedbackEffectTemplate const * ms_inNebulaForceFeedbackEffect = NULL;

	//----------------------------------------------------------------------

	typedef stdmap<int, ClientNebula *>::fwd ClientNebulaMap;
	ClientNebulaMap s_clientNebulaMap;

	//----------------------------------------------------------------------

	ClientNebula const * getClientNebulaById(int const id)
	{
		ClientNebulaMap::iterator iter = s_clientNebulaMap.find(id);
		if (iter != s_clientNebulaMap.end())
			return iter->second;

		return 0;
	}

	//----------------------------------------------------------------------

	int s_currentNumShaderPrimitivesNear = 0;
	int s_currentNumShaderPrimitivesFar = 0;
	int s_currentNumQuadsNear = 0;
	int s_currentNumQuadsFar = 0;

	//----------------------------------------------------------------------

	Vector findVectorNearPoint(Vector const & v, float const radius)
	{
		Vector result;

		result.x = (v.x > 0.0f) ? std::max(0.0f, (v.x - radius)) : std::min(0.0f, (v.x + radius));
		result.y = (v.y > 0.0f) ? std::max(0.0f, (v.y - radius)) : std::min(0.0f, (v.y + radius));
		result.z = (v.z > 0.0f) ? std::max(0.0f, (v.z - radius)) : std::min(0.0f, (v.z + radius));

		return result;
	}

	//----------------------------------------------------------------------

	Vector findVectorFarPoint(Vector const & v, float const radius)
	{
		Vector result;

		result.x = (v.x > 0.0f) ? (v.x + radius) : (v.x - radius);
		result.y = (v.y > 0.0f) ? (v.y + radius) : (v.y - radius);
		result.z = (v.z > 0.0f) ? (v.z + radius) : (v.z - radius);

		return result;
	}

	//----------------------------------------------------------------------

	class ShaderGroupList
	{
	public:
		typedef stdmap<Shader const *, NebulaVisualQuadShaderGroup *>::fwd ShaderGroupMap;

	public:

		ShaderGroupList();
		~ShaderGroupList();

		void addShaderGroupData(NebulaVisualQuadShaderGroup const & shaderGroup);
		void addQuad(Shader const & shader, NebulaVisualQuad const * quad);
		NebulaVisualQuadShaderGroup * findOrCreateShaderGroup(Shader const & shader);

		ShaderGroupMap const & getShaderGroupMap() const;
		ShaderGroupMap & getShaderGroupMap();

	private:
		ShaderGroupMap m_shaderGroupMap;
	};

	//----------------------------------------------------------------------

	ShaderGroupList::ShaderGroupList() :
	m_shaderGroupMap()
	{

	}

	//----------------------------------------------------------------------

	ShaderGroupList::~ShaderGroupList()
	{
		std::for_each(m_shaderGroupMap.begin(), m_shaderGroupMap.end(), PointerDeleterPairSecond());
		m_shaderGroupMap.clear();
	}

	//----------------------------------------------------------------------

	NebulaVisualQuadShaderGroup * ShaderGroupList::findOrCreateShaderGroup(Shader const & shader)
	{
		NebulaVisualQuadShaderGroup * myShaderGroup = NULL;

		ShaderGroupMap::iterator const map_it = m_shaderGroupMap.find(&shader);
		if (map_it != m_shaderGroupMap.end())
			myShaderGroup = (*map_it).second;
		else
		{
			myShaderGroup = new NebulaVisualQuadShaderGroup(shader, 0, false);
			IGNORE_RETURN(m_shaderGroupMap.insert(std::make_pair(&shader, myShaderGroup)));
		}

		return NON_NULL(myShaderGroup);
	}

	//----------------------------------------------------------------------

	void ShaderGroupList::addShaderGroupData(NebulaVisualQuadShaderGroup const & rhsShaderGroup)
	{
		NebulaVisualQuadShaderGroup * const myShaderGroup = findOrCreateShaderGroup(rhsShaderGroup.getShader());
		myShaderGroup->addDataFromShaderGroup(rhsShaderGroup);
	}

	//----------------------------------------------------------------------

	ShaderGroupList::ShaderGroupMap const & ShaderGroupList::getShaderGroupMap() const
	{
		return m_shaderGroupMap;
	}

	//----------------------------------------------------------------------

	ShaderGroupList::ShaderGroupMap & ShaderGroupList::getShaderGroupMap()
	{
		return m_shaderGroupMap; //lint !e1536 //exposing low access member
	}

	//----------------------------------------------------------------------

	void ShaderGroupList::addQuad(Shader const & shader, NebulaVisualQuad const * const quad)
	{
		NebulaVisualQuadShaderGroup * const myShaderGroup = findOrCreateShaderGroup(shader);
		myShaderGroup->addQuad(quad);
	}

	//----------------------------------------------------------------------

	typedef stdmap<float, ShaderGroupList>::fwd ShaderGroupListShellMap;
	ShaderGroupListShellMap s_shaderGroupListShellMap;

	typedef stdvector<NebulaShellShaderPrimitive *>::fwd NebulaShellShaderPrimitiveVector;
	NebulaShellShaderPrimitiveVector s_nebulaShellShaderPrimitives;

	//----------------------------------------------------------------------

	class MyAppearance : public Appearance
	{
		//lint -e754 //not referenced

	public:
		explicit MyAppearance(bool far);
		~MyAppearance();
		virtual void render() const;

	private:

		DPVS::Object * getDpvsObject() const;

	private:

		DPVS::Object * m_dpvsObject;

		bool m_far;

		MyAppearance();
		MyAppearance(MyAppearance const & rhs); //lint !e754 //not referenced
		MyAppearance & operator=(MyAppearance const & rhs); //lint !e754 //not referenced
	};

	//----------------------------------------------------------------------

	MyAppearance::MyAppearance(bool const appearanceIsFar) :
	Appearance(NULL),
	m_dpvsObject(NULL),
	m_far(appearanceIsFar)
	{
		m_dpvsObject = RenderWorld::createUnboundedObject(this);
	}

	//----------------------------------------------------------------------

	MyAppearance::~MyAppearance()
	{
		IGNORE_RETURN(m_dpvsObject->release());
		m_dpvsObject = NULL;
	}

	//----------------------------------------------------------------------

	void MyAppearance::render() const
	{
		bool const renderNear = !m_far && NebulaManagerClient::Config::getRenderNear();
		bool const renderFar = m_far && NebulaManagerClient::Config::getRenderFar();

		//-- env rendering is FIFO, so render from back to front
		for (NebulaShellShaderPrimitiveVector::reverse_iterator rit = s_nebulaShellShaderPrimitives.rbegin(); rit != s_nebulaShellShaderPrimitives.rend(); ++rit)
		{
			NebulaShellShaderPrimitive * const shaderPrimitive = NON_NULL(*rit);

			if (renderFar)
			{
				if (shaderPrimitive->isOutsideFarPlane())
					ShaderPrimitiveSorter::add(*shaderPrimitive);
			}
			else if (renderNear)
			{
				if (!shaderPrimitive->isOutsideFarPlane())
					ShaderPrimitiveSorter::add(*shaderPrimitive);
			}
		}

	}

	//----------------------------------------------------------------------

	DPVS::Object * MyAppearance::getDpvsObject() const
	{
		return m_dpvsObject;
	} //lint !e1763 //return indirectly modifies class.

	//----------------------------------------------------------------------

	inline bool testVisibilityForNebulaSphere(Sphere const & sphere, Volume const & frustum)
	{
		Vector const & sphereCenter = sphere.getCenter();
		float const radius = sphere.getRadius();

		if (ShaderPrimitiveSorter::getUseClipRectangle())
		{
			if (radius > 0.0f)
			{
				Rectangle2d const shaderPrimitiveRectangle(sphereCenter.x - radius, sphereCenter.z - radius, sphereCenter.x + radius, sphereCenter.z + radius);
				if (!ShaderPrimitiveSorter::getClipRectangle().intersects(shaderPrimitiveRectangle))
				{
					return false;
				}
			}
		}

		for (int i = static_cast<int>(Camera::FP_Near); i < static_cast<int>(Camera::FP_Max); ++i)
		{
			//-- skip far plane test
			if (i == static_cast<int>(Camera::FP_Far))
				continue;

			Plane const & plane = frustum.getPlane(i);
			if (plane.computeDistanceTo(sphereCenter) > radius)
				return false;
		}

		return true;
	}

	//----------------------------------------------------------------------
}


using namespace NebulaManagerClientNamespace;

//----------------------------------------------------------------------

VectorArgb NebulaManagerClient::ms_currentNebulaCameraHue;

//----------------------------------------------------------------------

void NebulaManagerClient::install()
{
	InstallTimer const installTimer("NebulaManagerClient::install");

	NebulaShellShaderPrimitive::install();
	NebulaVisualQuad::install();

	ms_inNebulaForceFeedbackEffect = ForceFeedbackEffectTemplateList::fetch("forcefeedback/nebula_rumble.ffe", false);
	ExitChain::add(remove, "NebulaManagerClient");
}

//----------------------------------------------------------------------

void NebulaManagerClient::remove()
{
	if(ms_inNebulaForceFeedbackEffect)
		ForceFeedbackEffectTemplateList::release(ms_inNebulaForceFeedbackEffect);

	delete s_worldRenderObject;
	s_worldRenderObject = 0;

	delete s_worldEnvironmentRenderObject;
	s_worldEnvironmentRenderObject = 0;
}

//----------------------------------------------------------------------

void NebulaManagerClient::render()
{
	Camera const * const camera = Game::getCamera();
	if (camera == NULL)
		return;

	if (s_showNebulaExtents)
	{
		static NebulaManager::NebulaVector nebulaVector;

		nebulaVector.clear();
		NebulaManager::getNebulasInSphere(camera->getPosition_w(), camera->getFarPlane () * 2.0f, nebulaVector);

		Graphics::setStaticShader(ShaderTemplateList::get3dVertexColorAZStaticShader());
		Graphics::setObjectToWorldTransformAndScale(Transform::identity, Vector::xyz111);

		for (NebulaManager::NebulaVector::const_iterator it = nebulaVector.begin(); it != nebulaVector.end(); ++it)
		{
			Nebula const * nebula = NON_NULL(*it);
			Vector const & pos_w = nebula->getSphere().getCenter();
			float const radius = nebula->getSphere().getRadius();

			float const density = nebula->getDensity();
			if (density <= 0.0f)
				continue;

			Graphics::drawSphere2(pos_w, radius, 10, 24, 24, nebula->getColor());
		}
	}
}

//----------------------------------------------------------------------

void NebulaManagerClient::updateCameraHue()
{
	ms_currentNebulaCameraHue = VectorArgb::solidBlack;
	ms_currentNebulaCameraHue.a = 0.0f;
}

//----------------------------------------------------------------------

void NebulaManagerClient::loadScene(std::string const & sceneId)
{
	if (!s_flagsInstalled)
	{
		DebugFlags::registerFlag(s_showNebulaExtents, "ClientGame/NebulaManagerClient", "showNebulaExtents");
		DebugFlags::registerFlag(s_lightningEnabled, "ClientGame/NebulaManagerClient", "lightningEnabled");
		DebugFlags::registerFlag(s_printMetrics, "ClientGame/NebulaManagerClient", "printMetrics");
		s_flagsInstalled = true;
	}

	NebulaManager::setClearFunction(&NebulaManagerClient::clear);
	NebulaManager::loadScene(sceneId);

	if (sceneId.empty())
		return;

	NebulaManager::NebulaVector const & nebulaVector = NebulaManager::getNebulaVector();
	for (NebulaManager::NebulaVector::const_iterator it = nebulaVector.begin(); it != nebulaVector.end(); ++it)
	{
		Nebula const * const nebula = NON_NULL(*it);
		int const nebulaId = nebula->getId();
		IGNORE_RETURN(s_clientNebulaMap.insert(std::make_pair(nebulaId, new ClientNebula(nebulaId))));
	}

	TerrainObject * const terrainObject = TerrainObject::getInstance();
	if (terrainObject == NULL)
	{
		WARNING(true, ("NebulaManagerClient::loadScene() [%s] with no terrain object", sceneId.c_str()));
		return;
	}

	ClientSpaceTerrainAppearance * const clientSpaceTerrainAppearance = dynamic_cast<ClientSpaceTerrainAppearance *>(terrainObject->getAppearance());
//	if (clientSpaceTerrainAppearance == NULL)
//	{
//		return;
//	}

//	SpaceEnvironment & spaceEnvironment = clientSpaceTerrainAppearance->getSpaceEnvironment();

	if (s_worldEnvironmentRenderObject.getPointer() == NULL)
	{
		Object * const object = new Object;
		object->setDebugName("NebulaManagerClient far appearance");
		MyAppearance * const myAppearance = new MyAppearance(true);
		object->setAppearance(myAppearance);

		if (clientSpaceTerrainAppearance)
		{
			clientSpaceTerrainAppearance->getSpaceEnvironment().addEnvironmentObject(*object);
		}
		else
			RenderWorld::addWorldEnvironmentObject(object);

		s_worldEnvironmentRenderObject = object;
	} //lint !e429 //custodial pointer object owned by world

	if (s_worldRenderObject.getPointer() == NULL)
	{
		Object * const nearObject = new Object;
		nearObject->setDebugName("NebulaManagerClient near appearance");
		MyAppearance * const myAppearance = new MyAppearance(false);
		nearObject->setAppearance(myAppearance);
		RenderWorld::addObjectNotifications(*nearObject);
		nearObject->addToWorld();
		nearObject->scheduleForAlter();
		s_worldRenderObject = nearObject;
	} //lint !e429 //custodial pointer nearObject owned by world
}

//----------------------------------------------------------------------

void NebulaManagerClient::clear()
{
	s_shaderGroupListShellMap.clear();

	//----------------------------------------------------------------------

	std::for_each(s_clientNebulaMap.begin(), s_clientNebulaMap.end(), PointerDeleterPairSecond());
	s_clientNebulaMap.clear();

	//----------------------------------------------------------------------

	std::for_each(s_nebulaShellShaderPrimitives.begin(), s_nebulaShellShaderPrimitives.end(), PointerDeleter());
	s_nebulaShellShaderPrimitives.clear();

	//----------------------------------------------------------------------

	s_objectsRecentlyHitByLightning.clear();

	{
		for (NebulaObjectMap::iterator it = s_nebulaObjectMap.begin(); it != s_nebulaObjectMap.end(); ++it)
		{
			ObjectWatcher & objectWatcher = (*it).second;
			delete objectWatcher.getPointer();
		}

		s_nebulaObjectMap.clear();
	}

	{
		for (LightningObjectVector::iterator it = s_lightningObjectVector.begin(); it != s_lightningObjectVector.end(); ++it)
		{
			ObjectWatcher & objectWatcher = (*it).second;
			delete objectWatcher.getPointer();
		}

		s_lightningObjectVector.clear();
	}

	{
		for (NebulaSoundMap::iterator it = s_nebulaSoundMap.begin(); it != s_nebulaSoundMap.end (); ++it)
		{
			SoundId const & soundId = (*it).second;
			Audio::stopSound(soundId);
		}
	}

	s_nebulaSoundMap.clear();
}

//----------------------------------------------------------------------

void NebulaManagerClient::handlePlayerEnter(int id)
{
	Nebula const * const nebula = NebulaManager::getNebulaById(id);
	if (nebula == NULL)
		return;

	if (nebula->getDensity() <= 0.0f)
		return;

	++s_playerEnteredNebulasCount;

	std::string const & ambientSound = nebula->getAmbientSound();
	if (!ambientSound.empty())
	{
		NebulaSoundMap::iterator const it = s_nebulaSoundMap.find(id);

		if (it == s_nebulaSoundMap.end() || !Audio::isSoundValid((*it).second))
		{
			SoundId const & soundId = Audio::playSound(nebula->getAmbientSound().c_str(), NULL);
			s_nebulaSoundMap[id] = soundId;
			Audio::setSoundVolume(soundId, 0.0f);
		}
	}
}

//----------------------------------------------------------------------

void NebulaManagerClient::handlePlayerExit(int id)
{
	Nebula const * const nebula = NebulaManager::getNebulaById(id);
	if (nebula == NULL)
		return;

	--s_playerEnteredNebulasCount;
	NebulaSoundMap::iterator const it = s_nebulaSoundMap.find(id);

	if (it != s_nebulaSoundMap.end())
	{
		SoundId const soundId = (*it).second;
		Audio::stopSound(soundId);
		s_nebulaSoundMap.erase(it);
	}
}

//----------------------------------------------------------------------

void NebulaManagerClient::update(float elapsedTime)
{
	PROFILER_AUTO_BLOCK_DEFINE("NebulaManagerClient::update");

	UNREF(elapsedTime);

	CreatureObject const * const player = Game::getPlayerCreature();

	if (player == NULL)
		return;

	updateCameraHue();

	//--
	//-- update client nebula data
	//--

	Camera const * const camera = Game::getCamera();
	if (camera != NULL)
	{
		updateClientNebulaData(camera->getPosition_w(), CuiPreferences::getGlobalNebulaRange());
	}
	Vector const & pos_w = player->findPosition_w();

	//--
	//-- handle nebula ambient sounds
	//--

	{
		for (NebulaSoundMap::iterator it = s_nebulaSoundMap.begin(); it != s_nebulaSoundMap.end (); )
		{
			int const nebulaId = (*it).first;
			SoundId const & soundId = (*it).second;
			Nebula const * const nebula = NebulaManager::getNebulaById(nebulaId);
			if (nebula == NULL || !Audio::isSoundValid(soundId))
			{
				Audio::stopSound(soundId);
				s_nebulaSoundMap.erase(it++);
				continue;
			}

			float const density = nebula->getDensity();
			if (density > 0.0f)
			{
				float const radius = nebula->getSphere().getRadius();

				if (radius > 0.0f)
				{
					float const distanceFromCenterSquared = pos_w.magnitudeBetweenSquared(nebula->getSphere().getCenter());
					float const engulfment = 1.0f - (distanceFromCenterSquared / sqr(radius));
					float const volume = clamp(0.0f, density * (engulfment * nebula->getAmbientSoundVolume()), 1.0f);
					Audio::setSoundVolume(soundId, volume);
				}
			}

			++it;
		}
	}

	//--
	//-- alter the nebulas
	//--

	{
		for (NebulaObjectMap::iterator it = s_nebulaObjectMap.begin(); it != s_nebulaObjectMap.end(); ++it)
		{
			ObjectWatcher & objectWatcher = (*it).second;
			NON_NULL(objectWatcher.getPointer())->scheduleForAlter();
		}
	}

	handleEnqueuedLightningEvents();

	//--
	//-- handle nebula camera jitter
	//--

	ShipObject const * const playerShipObject = player->getPilotedShip();

	if (playerShipObject != NULL)
	{
		GameCamera * const gameCamera = const_cast<GameCamera *>(dynamic_cast<GameCamera const *>(Game::getCamera()));

		ShipObject::IntVector const & shipNebulaIntVector = playerShipObject->getNebulas();
		if (gameCamera != NULL)
		{
			if (!gameCamera->isJittering())
			{
				float maxJitterAngle = 0.0f;

				for (ShipObject::IntVector::const_iterator it = shipNebulaIntVector.begin(); it != shipNebulaIntVector.end(); ++it)
				{
					int const nebulaId = *it;

					Nebula const * const nebula = NebulaManager::getNebulaById(nebulaId);
					if (nebula == NULL)
						continue;

					float const cameraJitterAngle = nebula->getCameraJitterAngle();

					if (cameraJitterAngle > 0.0f)
					{
						float const nebulaRadius = nebula->getSphere().getRadius();

						if (nebulaRadius > 0.0f)
						{
							float const density = nebula->getDensity();
							if (density <= 0.0f)
								continue;

							float const engulfment = 1.0f - (pos_w.magnitudeBetweenSquared(nebula->getSphere().getCenter()) / sqr(nebulaRadius));
							float const desiredJitterAngle = density * engulfment * cameraJitterAngle;

							maxJitterAngle = std::max(maxJitterAngle, desiredJitterAngle);
						}
					}
				}

				if (maxJitterAngle > 0.0f)
				{
					gameCamera->jitter(gameCamera->getObjectFrameK_w(), Random::randomReal(0.1f, 3.0f), maxJitterAngle, 1.0f);
				}
			}
		}
	}

	//-- do some testing simulation

	if (Game::getSinglePlayer())
		generateTestLightningEvents(elapsedTime);
}

//----------------------------------------------------------------------

void NebulaManagerClient::enqueueLightning(NebulaLightningData const & nebulaLightningData)
{
	if (!s_lightningEnabled)
		return;

	Nebula const * const nebula = NebulaManager::getNebulaById(nebulaLightningData.nebulaId);
	if (nebula == NULL)
	{
		WARNING(true, ("NebulaManagerClient::enqueueLightning invalid nebula [%d]", nebulaLightningData.nebulaId));
		return;
	}

	std::string const & lightningAppearance = nebula->getLightningAppearance();

	if (lightningAppearance.empty())
	{
		WARNING(true, ("NebulaManagerClient::enqueueLightning nebula [%d] has no lighting appearance", nebulaLightningData.nebulaId));
		return;
	}

	s_lightningObjectVector.push_back(LightningObjectPair(nebulaLightningData, ObjectWatcher(NULL)));
	++s_numLightningsSpawnedThisFrame;
}

//----------------------------------------------------------------------

/*
* handle enqueued lightning events
*/

void NebulaManagerClient::handleEnqueuedLightningEvents()
{
	s_numLightningsSpawnedThisFrame = 0;

	PROFILER_AUTO_BLOCK_DEFINE("NebulaManagerClient::handleEnqueuedLightningEvents");

	Camera const * const camera = Game::getCamera();
	Vector const & cameraPos_w = camera != NULL ? camera->getPosition_w() : Vector::zero;

	uint32 const syncStamp = Game::getSinglePlayer () ? static_cast<uint32>(Game::getElapsedTime() * 1000.0f) : GameNetwork::getServerSyncStampLong();

	for (LightningObjectVector::iterator it = s_lightningObjectVector.begin(); it != s_lightningObjectVector.end();)
	{
		NebulaLightningData const & nebulaLightningData = (*it).first;
		ObjectWatcher & objectWatcher = (*it).second;

		Nebula const * const nebula = NebulaManager::getNebulaById(nebulaLightningData.nebulaId);
		if (nebula == NULL)
		{
			it = s_lightningObjectVector.erase(it);
			continue;
		}

		std::string const & lightningAppearanceName = nebula->getLightningAppearance();

		if (lightningAppearanceName.empty())
		{
			it = s_lightningObjectVector.erase(it);
			continue;
		}

		//-- lightning occurs now or in the past
		if (nebulaLightningData.syncStampStart <= syncStamp)
		{
			//-- lightning is finished
			if (nebulaLightningData.syncStampEnd < syncStamp)
			{
				Object * const lightning = (*it).second;

				if (lightning != NULL)
				{
					Audio::detachSound(*lightning, 1.0f);
					delete lightning;
				}

				//-- see if the lightning lifetime is expired
				//-- we keep lightning data around a bit after it is finished so that
				//-- we can respond properly to server notifications concerning this lightning hitting ships

				if ((syncStamp - nebulaLightningData.syncStampEnd) > s_lightningLifetime)
				{
					it = s_lightningObjectVector.erase(it);
				}
				else
					++it;

				continue;
			}

			LightningAppearance * lightningAppearance = NULL;

			//-- lightning must be created
			if (objectWatcher.getPointer() == NULL)
			{
				lightningAppearance = LightningAppearance::asLightningAppearance(AppearanceTemplateList::createAppearance (lightningAppearanceName.c_str()));

				if (lightningAppearance == NULL)
				{
					WARNING(true, ("NebulaManagerClient unable to create lightning appearance [%s]", lightningAppearanceName.c_str()));
				}
				else
				{
					objectWatcher = new Object;
					objectWatcher->setAppearance (lightningAppearance);
					RenderWorld::addObjectNotifications(*objectWatcher);
					objectWatcher->addToWorld();

					float const boltLength = (nebulaLightningData.endpoint0 - nebulaLightningData.endpoint1).approximateMagnitude();
					static float const boltScaleFactor = RECIP(15.0f);
					float const boltScale = clamp(1.0f, boltLength * boltScaleFactor, 1024.0f);

					lightningAppearance->setUniformScale(boltScale);
					lightningAppearance->setColorModifier(VectorArgb::linearInterpolate(nebula->getLightningColor(), nebula->getLightningColorRamp(), Random::randomReal(0.0f, 1.0f)));

					int const numBolts = Random::random(2,LightningAppearance::getMaxLightningBoltCount());
					for (int i = 0; i < numBolts; ++i)
						lightningAppearance->setPosition_w (i, nebulaLightningData.endpoint0, nebulaLightningData.endpoint1);

					objectWatcher->move_o((nebulaLightningData.endpoint0 + nebulaLightningData.endpoint1) * 0.5f);

					std::string const & lightningSound = nebula->getLightningSound();
					if (!lightningSound.empty())
					{
						//-- Only play the lighting sound if the sound template can be heard at camera's position
						Vector const closestPointOnLightning = cameraPos_w.findClosestPointOnLineSegment(nebulaLightningData.endpoint0, nebulaLightningData.endpoint1);

						ClientNebula const * const clientNebula = getClientNebulaById(nebulaLightningData.nebulaId);
						if (clientNebula)
						{
							SoundTemplate const * const soundTemplate = clientNebula->getLightningSoundTemplate();
							if (soundTemplate)
							{
								float const maximumSoundDistance = Audio::getFallOffDistance(soundTemplate->getDistanceAtMaxVolume());

								if (cameraPos_w.magnitudeBetweenSquared(closestPointOnLightning) < sqr(maximumSoundDistance))
									IGNORE_RETURN(Audio::playSound(lightningSound.c_str(), closestPointOnLightning, NULL));
							}
						}
					}

					std::string const & lightningSoundLoop = nebula->getLightningSoundLoop();
					if (!lightningSoundLoop.empty())
						IGNORE_RETURN(Audio::attachSound(lightningSoundLoop.c_str(), objectWatcher, NULL));
				}
			}
			else
			{
				lightningAppearance = safe_cast<LightningAppearance *>(objectWatcher->getAppearance());
			}

			NOT_NULL(objectWatcher.getPointer());
			objectWatcher->scheduleForAlter();

			float const currentElapsedTime = Game::getElapsedTime();

			//-- test lightning for client-side scollisions
			if (lightningAppearance != NULL)
			{
				Vector startPosition_w = nebulaLightningData.endpoint0;
				Vector const & endPosition_w = nebulaLightningData.endpoint1;

				CollisionInfo result;
				Object const * excludeObject = NULL;

				uint16 const collisionFlags = ClientWorld::CF_tangible | ClientWorld::CF_childObjects | ClientWorld::CF_disablePortalCrossing;
				Vector unitLightningRay = (endPosition_w - startPosition_w);
				if (unitLightningRay.normalize())
				{
					static int const s_maxIterations = 10;

					CollideParameters collideParameters;
					collideParameters.setQuality(CollideParameters::Q_medium);

					for (int i = 0; i < s_maxIterations; ++i)
					{
						if (ClientWorld::collide(CellProperty::getWorldCellProperty(), startPosition_w, endPosition_w, collideParameters, result, collisionFlags, excludeObject))
						{
							excludeObject = result.getObject();

							if (excludeObject != NULL)
							{
								Object const * const rootParent = excludeObject->getRootParent();
								ClientObject const * const clientRootParent = rootParent->asClientObject();

								if (clientRootParent != NULL && clientRootParent->asShipObject() != NULL)
								{
									NetworkIdFloatMap::const_iterator const oit = s_objectsRecentlyHitByLightning.find(CachedNetworkId(*rootParent));
									if (oit == s_objectsRecentlyHitByLightning.end() || (*oit).second < currentElapsedTime)
									{
										ClientNebula const * const clientNebula = getClientNebulaById(nebulaLightningData.nebulaId);
										if (clientNebula)
										{
											ClientEffectTemplate const * const clientEffectTemplate = clientNebula->getClientHitLightningClientEffectTemplate();
											if (clientEffectTemplate)
											{
												Vector const position_w = result.getPoint();

												ClientEffect * const clientEffect = clientEffectTemplate->createClientEffect(CellProperty::getWorldCellProperty(), position_w, Vector::unitY);
												clientEffect->execute();
												delete clientEffect;
											}

											s_objectsRecentlyHitByLightning[CachedNetworkId(*rootParent)] = currentElapsedTime + 2.0f;
										}
									}
								}

								Vector const & hitPoint = result.getPoint();
								float const sphereRadius = rootParent->getAppearanceSphereRadius();

								//-- advance the start position, but not past the end position
								if (sqr(sphereRadius) < endPosition_w.magnitudeBetweenSquared(hitPoint))
								{
									startPosition_w = hitPoint + (unitLightningRay * sphereRadius);
									excludeObject = rootParent;
									continue;
								}
							}
						}

						//-- fell through because of no valid intersections or no portion of the line segment remaining
						break;
					}
				}
			}
		}

		++it;
	}
}
//----------------------------------------------------------------------

void NebulaManagerClient::generateTestLightningEvents(float elapsedTime)
{
	//----------------------------------------------------------------------
	//--
	//-- generate test lightning events
	//--

	if (Game::getSinglePlayer())
	{
		NebulaManager::NebulaVector const & nebulaVector = NebulaManager::getNebulaVector();
		for (NebulaManager::NebulaVector::const_iterator it = nebulaVector.begin(); it != nebulaVector.end(); ++it)
		{
			Nebula const * const nebula = NON_NULL(*it);

			float const density = nebula->getDensity();
			if (density <= 0.0f)
				continue;

			std::string const & lightningAppearance = nebula->getLightningAppearance();

			if (!lightningAppearance.empty())
			{
				float const lightningFrequency = nebula->getLightningFrequency();

				if (lightningFrequency > 0.0f)
				{
					float const averageLightningCountThisFrame = std::min(0.5f, elapsedTime * lightningFrequency);

					if (Random::randomReal(0.0f, 1.0f) < averageLightningCountThisFrame)
					{
						float const radius = nebula->getSphere().getRadius() * 0.3f;
						Vector const & center = nebula->getSphere().getCenter();

						float const lightningDurationMax = nebula->getLightningDurationMax();
						int const lightningDurationMaxMs = static_cast<int>(lightningDurationMax * 1000.0f);

						NebulaLightningData nebulaLightningData;
						nebulaLightningData.syncStampStart = static_cast<uint32>(Game::getElapsedTime() * 1000.0f) + 1000;
						nebulaLightningData.syncStampEnd = nebulaLightningData.syncStampStart + static_cast<uint32>(Random::random(lightningDurationMaxMs / 2, lightningDurationMaxMs));
						nebulaLightningData.nebulaId = nebula->getId();

						nebulaLightningData.endpoint0.x = Random::randomReal(center.x - radius, center.x + radius);
						nebulaLightningData.endpoint0.y = Random::randomReal(center.y - radius, center.y + radius);
						nebulaLightningData.endpoint0.z = Random::randomReal(center.z - radius, center.z + radius);

						nebulaLightningData.endpoint1.x = Random::randomReal(center.x - radius, center.x + radius);
						nebulaLightningData.endpoint1.y = Random::randomReal(center.y - radius, center.y + radius);
						nebulaLightningData.endpoint1.z = Random::randomReal(center.z - radius, center.z + radius);

						NebulaManagerClient::enqueueLightning(nebulaLightningData);
					}
				}
			}
		}
	}
}

//----------------------------------------------------------------------

NebulaLightningData const * NebulaManagerClient::findNebulaLightningData(uint16 lightningId)
{
	for (LightningObjectVector::iterator it = s_lightningObjectVector.begin(); it != s_lightningObjectVector.end(); ++it)
	{
		NebulaLightningData const & nebulaLightningData = (*it).first;
		if (nebulaLightningData.lightningId == lightningId)
			return &nebulaLightningData;
	}

	return NULL;
}

//----------------------------------------------------------------------

/**
* handleServerHit is called from the controller.  The lightning appearance will be created next frame.
* The scene will be rendered once, and the victim altered once before the lightning object will be placed and rendered.
*/

void NebulaManagerClient::handleServerHit(ClientObject const & victim, NebulaLightningHitData const & nebulaLightningHitData)
{
	float const currentElapsedTime = Game::getElapsedTime();

	ShipObject const * const shipVictim = victim.asShipObject();

	if (shipVictim == NULL)
	{
		WARNING(true, ("NebulaManagerClient::handleServerHit on non-ship [%s]", victim.getNetworkId().getValueString().c_str()));
		return;
	}

	NebulaLightningData const * const nebulaLightningData = NebulaManagerClient::findNebulaLightningData(nebulaLightningHitData.lightningId);

	int nebulaId = 0;

	Vector const & victimPos_w = victim.getPosition_w();
	if (nebulaLightningData == NULL)
	{
		float outMinDistance = 0.0f;
		float outMaxDistance = 0.0f;

		Nebula const * const nebula = NebulaManager::getClosestNebula(victimPos_w, 128.0f, outMinDistance, outMaxDistance);
		if (nebula != NULL)
			nebulaId = nebula->getId();
	}
	else
	{
		nebulaId = nebulaLightningData->nebulaId;
	}

	//-- if no nebula is nearby, just do nothing
	if (nebulaId == 0)
		return;

	float const currentSpeed = shipVictim->getCurrentSpeed();
	float const distancePerFrame = currentSpeed * Clock::frameTime();
	float const shipSphereRadius = shipVictim->getAppearanceSphereRadius();

	float offsetForward = 0.0f;

	//-- front
	if (nebulaLightningHitData.side == 0)
	{
		offsetForward = (shipSphereRadius * 0.5f) + distancePerFrame;
	}
	//-- back
	else
	{
		offsetForward = distancePerFrame;
	}

	//-- find out if the victim has been recently hit by lightning,
	//-- if not, play a lightning effect across the nose or center of the ship
	NetworkIdFloatMap::const_iterator const oit = s_objectsRecentlyHitByLightning.find(CachedNetworkId(victim));
	if (oit == s_objectsRecentlyHitByLightning.end() || (*oit).second > currentElapsedTime)
	{
		Vector startPosition_o (0.0f, shipSphereRadius * 2.0f, offsetForward);
		Vector endPosition_o (0.0f, -shipSphereRadius * 2.0f, offsetForward);
		Vector const & startPosition_w = victim.rotateTranslate_o2p(startPosition_o);
		Vector const & endPosition_w = victim.rotateTranslate_o2p(endPosition_o);

		NebulaLightningData newNebulaLightningData;
		newNebulaLightningData.lightningId = nebulaLightningHitData.lightningId;
		newNebulaLightningData.nebulaId = nebulaId;
		newNebulaLightningData.syncStampStart = GameNetwork::getServerSyncStampLong() - 1;
		newNebulaLightningData.syncStampEnd = newNebulaLightningData.syncStampStart + 2000;
		newNebulaLightningData.endpoint0 = startPosition_w;
		newNebulaLightningData.endpoint0 = endPosition_w;

		NebulaManagerClient::enqueueLightning(newNebulaLightningData);
	}

	//-- now play the damage effect
	ClientNebula const * const clientNebula = getClientNebulaById(nebulaId);
	if (clientNebula)
	{
		ClientEffectTemplate const * const clientEffectTemplate = clientNebula->getServerHitLightningClientEffectTemplate();
		if (clientEffectTemplate)
		{
			Vector position_w = victimPos_w;
			position_w.z += offsetForward;

			ClientEffect * const clientEffect = clientEffectTemplate->createClientEffect(CellProperty::getWorldCellProperty(), position_w, Vector::unitY);
			clientEffect->execute();
			delete clientEffect;
		}
	}
}

//----------------------------------------------------------------------

void NebulaManagerClient::handleServerEnvironmentalDamage(ClientObject const & victim, EnvironmentalHitData const & environmentalHitData)
{
	//-- play the damage effect
	ClientNebula const * const clientNebula = getClientNebulaById(environmentalHitData.nebulaId);
	if (clientNebula)
	{
		ClientEffectTemplate const * const clientEffectTemplate = clientNebula->getEnvironmentalDamageClientEffectTemplate();
		if (clientEffectTemplate)
		{
			Vector const position_w = victim.getPosition_w();

			ClientEffect * const clientEffect = clientEffectTemplate->createClientEffect(CellProperty::getWorldCellProperty(), position_w, Vector::unitY);
			clientEffect->execute();
			delete clientEffect;
		}
	}
}

//----------------------------------------------------------------------

void NebulaManagerClient::updateClientNebulaData(Vector const & center, float const range)
{
	const Sphere sphereToTest(center, range);

	ShaderGroupList masterShaderGroupList;

	Camera const * const camera = Game::getCamera();
	if (camera == NULL)
		return;

	Volume const & frustum = camera->getFrustumVolume();

	{
		bool const forceRegenerate = Config::getRegenerate();
		if (forceRegenerate)
			Config::setRegenerate(false);

		typedef stdmap<float, Shader const *>::fwd RangeShaderMap;
		RangeShaderMap rangeShaderMap;

		int const s_numRanges = std::max(1, Config::getNumShells());

		{
			float const farPlane = camera->getFarPlane();
			float const numRangesFactor = RECIP(static_cast<float>(s_numRanges));
			float const nominalIncrement = farPlane * numRangesFactor;
			float const increment = (farPlane + nominalIncrement) * numRangesFactor;

			for (int i = 0; i < s_numRanges; ++i)
			{
				float const rangeSquared = sqr(static_cast<float>(i * increment));
				IGNORE_RETURN(rangeShaderMap.insert(std::make_pair(rangeSquared, static_cast<Shader const *>(NULL))));
			}
		}


		{
			for (ClientNebulaMap::iterator it = s_clientNebulaMap.begin(); it != s_clientNebulaMap.end(); ++it)
			{
				ClientNebula * const clientNebula = NON_NULL((*it).second);
				int const nebulaId = clientNebula->getId();
				Nebula const * const nebula = NON_NULL(NebulaManager::getNebulaById(nebulaId));

				if (nebula->getDensity() <= 0.0f)
					continue;

				Sphere const & sphere = nebula->getSphere();

				if (sphereToTest.intersectsSphere(sphere))
				{
					//-- force the nebulas to regenerate
					if (forceRegenerate && clientNebula->isPopulated())
						clientNebula->clear();

					if (!clientNebula->isPopulated())
						clientNebula->populate();

					NebulaVisualQuadShaderGroup const * const shaderGroup = clientNebula->getNebulaVisualQuadShaderGroup();

					if (shaderGroup != NULL)
					{

						Vector sphereToCamera_w = center - sphere.getCenter();
						IGNORE_RETURN(sphereToCamera_w.approximateNormalize());
						Vector const & sphereRadiusToCamera = sphereToCamera_w * sphere.getRadius();

						Vector const & sphereNearEdge = sphere.getCenter() - sphereRadiusToCamera;
						Vector const & sphereFarEdge = sphere.getCenter() + sphereRadiusToCamera;

						float const rangeSquaredNear = sphereNearEdge.magnitudeBetweenSquared(center);
						float const rangeSquaredFar = sphereFarEdge.magnitudeBetweenSquared(center);

						IGNORE_RETURN(rangeShaderMap.insert(std::make_pair(rangeSquaredNear, &shaderGroup->getShader())));
						IGNORE_RETURN(rangeShaderMap.insert(std::make_pair(rangeSquaredFar, &shaderGroup->getShader())));
					}

					//-- test visisility vs. frustum
					Sphere const & sphere_c = camera->rotateTranslate_w2o(sphere);
					if (!testVisibilityForNebulaSphere(sphere_c, frustum))
						continue;

					if (shaderGroup != NULL)
						masterShaderGroupList.addShaderGroupData(*shaderGroup);
				}
				else if (clientNebula->isPopulated())
					clientNebula->clear();
			}
		}

		s_shaderGroupListShellMap.clear();

		{
			Shader const * lastShader = NULL;
			float lastRangeSquared = -10000.0f;

			for (RangeShaderMap::const_iterator it = rangeShaderMap.begin(); it != rangeShaderMap.end(); ++it)
			{
				float const rangeSquared = (*it).first;
				Shader const * const shader = (*it).second;

				if (shader != NULL && shader == lastShader)
					continue;

				float const rangeSquaredDiff = rangeSquared - lastRangeSquared;
				if (rangeSquaredDiff < 10.0f)
					continue;

				IGNORE_RETURN(s_shaderGroupListShellMap[rangeSquared]);

				lastShader = shader;
				lastRangeSquared = rangeSquared;
			}
		}

	}

	ShaderGroupList::ShaderGroupMap const & masterShaderGroupMap = masterShaderGroupList.getShaderGroupMap();

	for (ShaderGroupList::ShaderGroupMap::const_iterator sgm_it = masterShaderGroupMap.begin(); sgm_it != masterShaderGroupMap.end(); ++sgm_it)
	{
		Shader const * const shader = NON_NULL((*sgm_it).first);
		NebulaVisualQuadShaderGroup const * const shaderGroup = NON_NULL((*sgm_it).second);

		NebulaVisualQuadShaderGroup::NebulaVisualQuadVector const & quads = shaderGroup->getNebulaVisualQuads();

		for (NebulaVisualQuadShaderGroup::NebulaVisualQuadVector::const_iterator q_it = quads.begin(); q_it != quads.end(); ++q_it)
		{
			NebulaVisualQuad const & quad = **q_it;
			Sphere const & sphere = quad.getSphere();
			Sphere const & sphere_c = camera->rotateTranslate_w2o(sphere);
			if (!testVisibilityForNebulaSphere(sphere_c, frustum))
				continue;

			Vector sphereToCamera_w = center - sphere.getCenter();
			IGNORE_RETURN(sphereToCamera_w.approximateNormalize());

			Vector const & sphereNearEdge = sphere.getCenter() - (sphereToCamera_w * sphere.getRadius());

			float const rangeSquared = sphereNearEdge.magnitudeBetweenSquared(center);
			ShaderGroupListShellMap::iterator shell_it = s_shaderGroupListShellMap.lower_bound(rangeSquared);

			if (shell_it != s_shaderGroupListShellMap.begin())
				--shell_it;

			ShaderGroupList & shaderGroupList = (*shell_it).second;
			shaderGroupList.addQuad(*shader, &quad);
		}
	}

	generateShaderPrimitives();
}

//----------------------------------------------------------------------

void NebulaManagerClient::generateShaderPrimitives()
{
	s_currentNumShaderPrimitivesNear = 0;
	s_currentNumShaderPrimitivesFar = 0;
	s_currentNumQuadsNear = 0;
	s_currentNumQuadsFar = 0;

	Camera const * const camera = Game::getCamera();
	if (camera == NULL)
		return;

	static int s_maxQuadsPerBuffer = 1000;

	float const farPlaneSquared = sqr(camera->getFarPlane());
	Vector const & cameraPos_w = camera->getPosition_w();

	TerrainObject const * const terrainObject = TerrainObject::getInstance();
	bool farNebulasAllowed = true;
	ClientSpaceTerrainAppearance const * const clientSpaceTerrainAppearance = terrainObject ? dynamic_cast<ClientSpaceTerrainAppearance const *>(terrainObject->getAppearance()): NULL;
	if (!clientSpaceTerrainAppearance)
		farNebulasAllowed = false;

	std::for_each(s_nebulaShellShaderPrimitives.begin(), s_nebulaShellShaderPrimitives.end(), PointerDeleter());
	s_nebulaShellShaderPrimitives.clear();

	for (ShaderGroupListShellMap::iterator it = s_shaderGroupListShellMap.begin(); it != s_shaderGroupListShellMap.end();)
	{
		ShaderGroupList & shaderGroupList = (*it).second;

		ShaderGroupList::ShaderGroupMap & shaderGroupMap = shaderGroupList.getShaderGroupMap();

		++it;

		bool const isOutsideFarPlane = (it == s_shaderGroupListShellMap.end()) || ((*it).first > farPlaneSquared);

		if (isOutsideFarPlane && !farNebulasAllowed)
			continue;

		for (ShaderGroupList::ShaderGroupMap::iterator sgm_it = shaderGroupMap.begin(); sgm_it != shaderGroupMap.end(); ++sgm_it)
		{
			NebulaVisualQuadShaderGroup * const shaderGroup = NON_NULL((*sgm_it).second);

			shaderGroup->sortQuads(cameraPos_w);
			int const totalQuads = static_cast<int>(shaderGroup->getNebulaVisualQuads().size());

			NebulaVisualQuadShaderGroup::NebulaVisualQuadVector const & quads = shaderGroup->getNebulaVisualQuads();

			for (int quadBegin = 0; quadBegin < totalQuads; quadBegin += s_maxQuadsPerBuffer)
			{
				int const quadEnd = std::min(quadBegin + s_maxQuadsPerBuffer, totalQuads);

				NebulaVisualQuad const & endQuad = **(quads.begin() + (quadEnd - 1));
				float const endQuadRangeSquared = -endQuad.getSort();
				NebulaShellShaderPrimitive * const shaderPrimitive = new NebulaShellShaderPrimitive(*shaderGroup, quadBegin, quadEnd, endQuadRangeSquared, isOutsideFarPlane);
				s_nebulaShellShaderPrimitives.push_back(shaderPrimitive);

				if (isOutsideFarPlane)
				{
					++s_currentNumShaderPrimitivesFar;
					s_currentNumQuadsFar += (quadEnd - quadBegin);
				}
				else
				{
					++s_currentNumShaderPrimitivesNear;
					s_currentNumQuadsNear += (quadEnd - quadBegin);
				}
			}
		}
	}

	DEBUG_REPORT_PRINT(s_printMetrics, ("NebulaManagerClient primitives [%d/%d] near/far, quads [%d/%d] near/far, lightning [%d/%d] active/spawned\n",
		s_currentNumShaderPrimitivesNear, s_currentNumShaderPrimitivesFar, s_currentNumQuadsNear, s_currentNumQuadsFar, static_cast<int>(s_lightningObjectVector.size()), s_numLightningsSpawnedThisFrame));
}

//----------------------------------------------------------------------
//-- Config
//----------------------------------------------------------------------

float NebulaManagerClient::Config::ms_orientedPercent = 0.0f;
int NebulaManagerClient::Config::ms_numShells = 10;
bool NebulaManagerClient::Config::ms_regenerate = false;
int NebulaManagerClient::Config::ms_randomSeedOffset = 0;
VectorArgb NebulaManagerClient::Config::ms_colorVariance;
float NebulaManagerClient::Config::ms_quadGenerationRadius = 0.9f;
bool NebulaManagerClient::Config::ms_renderNear = true;
bool NebulaManagerClient::Config::ms_renderFar = true;

//----------------------------------------------------------------------

void NebulaManagerClient::Config::setOrientedPercent(float f)
{
	ms_orientedPercent = f;
}

//----------------------------------------------------------------------

void NebulaManagerClient::Config::setNumShells(int i)
{
	ms_numShells = i;
}

//----------------------------------------------------------------------

void NebulaManagerClient::Config::setRegenerate(bool b)
{
	ms_regenerate = b;
}

//----------------------------------------------------------------------

void NebulaManagerClient::Config::setRandomSeedOffset(int i)
{
	ms_randomSeedOffset = i;
}

//----------------------------------------------------------------------

void NebulaManagerClient::Config::setColorVariance(VectorArgb const & v)
{
	ms_colorVariance = v;
}

//----------------------------------------------------------------------

void NebulaManagerClient::Config::setQuadGenerationRadius(float f)
{
	ms_quadGenerationRadius = f;
}

//----------------------------------------------------------------------

void NebulaManagerClient::Config::setRenderNear(bool b)
{
	ms_renderNear = b;
}

//----------------------------------------------------------------------

void NebulaManagerClient::Config::setRenderFar(bool b)
{
	ms_renderFar = b;
}

//----------------------------------------------------------------------

float NebulaManagerClient::Config::getOrientedPercent()
{
	return ms_orientedPercent;
}

//----------------------------------------------------------------------

int NebulaManagerClient::Config::getNumShells()
{
	return ms_numShells;
}

//----------------------------------------------------------------------

bool NebulaManagerClient::Config::getRegenerate()
{
	return ms_regenerate;
}

//----------------------------------------------------------------------

int NebulaManagerClient::Config::getRandomSeedOffset()
{
	return ms_randomSeedOffset;
}

//----------------------------------------------------------------------

VectorArgb const & NebulaManagerClient::Config::getColorVariance()
{
	return ms_colorVariance;
}

//----------------------------------------------------------------------

float NebulaManagerClient::Config::getQuadGenerationRadius()
{
	return ms_quadGenerationRadius;
}

//----------------------------------------------------------------------

bool NebulaManagerClient::Config::getRenderNear()
{
	return ms_renderNear;
}

//----------------------------------------------------------------------

bool NebulaManagerClient::Config::getRenderFar()
{
	return ms_renderFar;
}

//======================================================================
