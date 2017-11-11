//
// HyperspaceIoWin.cpp
// tford
//
// copyright 2004, sony online entertainment
//

//-------------------------------------------------------------------

#include "clientGame/FirstClientGame.h"
#include "clientGame/HyperspaceIoWin.h"

#include "clientAudio/Audio.h"
#include "clientAudio/SoundTemplateList.h"
#include "clientGame/Bloom.h"
#include "clientGame/ClientEffect.h"
#include "clientGame/ClientEffectTemplate.h"
#include "clientGame/ClientEffectTemplateList.h"
#include "clientGame/CockpitCamera.h"
#include "clientGame/CollisionCallbacks.h"
#include "clientGame/CutScene.h"
#include "clientGame/Game.h"
#include "clientGame/GroundScene.h"
#include "clientGame/PlayerShipController.h"
#include "clientGame/RemoteShipController.h"
#include "clientGame/ShipObject.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/PostProcessingEffectsManager.h"
#include "clientGraphics/RenderWorld.h"
#include "clientObject/SaveCameraParameters.h"
#include "clientObject/ShadowVolume.h"
#include "clientGraphics/PostProcessingEffectsManager.h"
#include "clientParticle/ParticleEffectAppearance.h"
#include "clientParticle/SetupClientParticle.h"
#include "clientTerrain/ClientSpaceTerrainAppearance.h"
#include "clientTerrain/SpaceEnvironment.h"
#include "clientUserInterface/CuiLoadingManager.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedGame/ShipDynamicsModel.h"
#include "sharedIoWin/IoWinManager.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/ContainedByProperty.h"
#include "sharedObject/MemoryBlockManagedObject.h"
#include "sharedObject/PortalProperty.h"
#include "sharedTerrain/TerrainObject.h"

// ==================================================================

namespace HyperspaceIoWinNamespace
{
	Tag const TAG_HYPR = TAG(H,Y,P,R);
	Tag const TAG_STG1 = TAG(S,T,G,1);
	Tag const TAG_STG2 = TAG(S,T,G,2);
	Tag const TAG_STG3 = TAG(S,T,G,3);

	char const * const cs_iffHyperspaceFileName = "scene/hyperspace.iff";
	char const * const cs_cockpitCellNames[] =
							{
								"cockpit",
								"bridge",
								"bridge66",
								0
							};

	// Stage data isn't directly associated with the stages because the data
	// is read in during install and we want to avoid loading these assets
	// during the ioWin construction

	struct CockpitStageData
	{
		float m_cockpitStageTime;
		std::string m_particleAppearancePath;
		std::string m_clientEffectTemplatePath;
		float m_particleOffset;
		std::string m_hyperspaceCockpitSoundPath;
	};

	struct ThirdPersonStageData
	{
		float m_thirdPersonStageTime;
		float m_hyperspaceSpeed;
		float m_hyperspaceRadianRollsPerSecond;
		std::string m_hyperspaceSoundPath;
	};

	CockpitStageData s_enterCockpitStageData;
	CockpitStageData s_leaveCockpitStageData;
	ThirdPersonStageData s_thirdPersonStageData;

	typedef std::vector<SoundTemplate const *> CachedSounds;
	CachedSounds s_cachedSounds;

	bool s_playHyperspaceBeforeSceneChange = false;
	float s_secondsToDisableOtherObjects = 5.0f;
	// direction from the ship that the camera is placed for the sling shot
	Vector const s_cameraOffsetDirection(0.0f, 0.1f, -1.0f);
	// listener for scene change
	MessageDispatch::Callback s_callback;

	typedef std::queue<HyperspaceIoWin::Stage *> StageQueue;

	void install();
	void remove();
	void load();
	void load_0000(Iff & iff);
	HyperspaceIoWin::Stage * popAndGetNextStage(StageQueue & m_stages);
	HyperspaceIoWin::Stage * getCurrentStage(StageQueue const & m_stages);
	CellProperty * const findShipCameraCell(ShipObject * shipObject);
	Object * findTopMostRootObject(Object * object);
	void deactivateSpaceEnvironmentObjects();
	void disableOtherObjectsThisFrame(ShipObject * playerShip, GameCamera const * currentCamera);
	void letterBoxCamera(GameCamera * camera);
}

//-------------------------------------------------------------------

void HyperspaceIoWinNamespace::install()
{
	load();
}

//-------------------------------------------------------------------

void HyperspaceIoWinNamespace::load()
{
	Iff iff;
	if (iff.open(cs_iffHyperspaceFileName, true))
	{
		iff.enterForm(TAG_HYPR);

		switch (iff.getCurrentName())
		{
			case TAG_0000:
				load_0000(iff);
				break;
			default:
				{
					char tagBuffer [5];
					ConvertTagToString(iff.getCurrentName(), tagBuffer);
					char buffer [128];
					iff.formatLocation(buffer, sizeof(buffer));
					DEBUG_FATAL(true,("unknown layer type %s/%s", buffer, tagBuffer));
				}
				break;
		}

		iff.exitForm();
	}
	else
	{
		DEBUG_WARNING(true,("Hyperspace::HyperspaceVisual::load unable to open file %s", cs_iffHyperspaceFileName));
	}

	if (!s_enterCockpitStageData.m_hyperspaceCockpitSoundPath.empty())
	{
		s_cachedSounds.push_back(SoundTemplateList::fetch(s_enterCockpitStageData.m_hyperspaceCockpitSoundPath.c_str()));
	}

	if (!s_leaveCockpitStageData.m_hyperspaceCockpitSoundPath.empty())
	{
		s_cachedSounds.push_back(SoundTemplateList::fetch(s_leaveCockpitStageData.m_hyperspaceCockpitSoundPath.c_str()));
	}

	if (!s_thirdPersonStageData.m_hyperspaceSoundPath.empty())
	{
		s_cachedSounds.push_back(SoundTemplateList::fetch(s_thirdPersonStageData.m_hyperspaceSoundPath.c_str()));
	}
}

//-------------------------------------------------------------------

void HyperspaceIoWinNamespace::remove()
{
	CachedSounds::const_iterator ii = s_cachedSounds.begin();
	CachedSounds::const_iterator iiEnd = s_cachedSounds.end();

	for (; ii != iiEnd; ++ii)
	{
		SoundTemplate const * soundTemplate = *ii;
		if (soundTemplate != 0)
		{
			SoundTemplateList::release(soundTemplate);
		}
	}

	s_cachedSounds.clear();
}

//-------------------------------------------------------------------

void HyperspaceIoWinNamespace::load_0000(Iff & iff)
{
	iff.enterForm(TAG_0000);
		iff.enterChunk(TAG_DATA);
			s_secondsToDisableOtherObjects = iff.read_float();
		iff.exitChunk();

		iff.enterChunk(TAG_STG1);
			s_enterCockpitStageData.m_cockpitStageTime = iff.read_float();
			s_enterCockpitStageData.m_particleAppearancePath = iff.read_stdstring();
			s_enterCockpitStageData.m_clientEffectTemplatePath = iff.read_stdstring();
			s_enterCockpitStageData.m_particleOffset = iff.read_float();
			s_enterCockpitStageData.m_hyperspaceCockpitSoundPath = iff.read_stdstring();
		iff.exitChunk();

		iff.enterChunk(TAG_STG2);
			s_thirdPersonStageData.m_thirdPersonStageTime = iff.read_float();
			s_thirdPersonStageData.m_hyperspaceSpeed = iff.read_float();
			s_thirdPersonStageData.m_hyperspaceRadianRollsPerSecond = iff.read_float();
			s_thirdPersonStageData.m_hyperspaceSoundPath = iff.read_stdstring();
		iff.exitChunk();

		iff.enterChunk(TAG_STG3);
			s_leaveCockpitStageData.m_cockpitStageTime = iff.read_float();
			s_leaveCockpitStageData.m_particleAppearancePath = iff.read_stdstring();
			s_leaveCockpitStageData.m_clientEffectTemplatePath = iff.read_stdstring();
			s_leaveCockpitStageData.m_particleOffset = iff.read_float();
			s_leaveCockpitStageData.m_hyperspaceCockpitSoundPath = iff.read_stdstring();
		iff.exitChunk();

	iff.exitForm(TAG_0000);
}

//-------------------------------------------------------------------

HyperspaceIoWin::Stage * HyperspaceIoWinNamespace::popAndGetNextStage(StageQueue & m_stages)
{
	if (!m_stages.empty())
	{
		m_stages.pop();
		return getCurrentStage(m_stages);
	}
	return 0;
}

//-------------------------------------------------------------------

HyperspaceIoWin::Stage * HyperspaceIoWinNamespace::getCurrentStage(StageQueue const & m_stages)
{
	if (!m_stages.empty())
	{
		return m_stages.front();
	}
	return 0;
}

//-------------------------------------------------------------------

CellProperty * const HyperspaceIoWinNamespace::findShipCameraCell(ShipObject * const shipObject)
{
	NOT_NULL(shipObject);

	PortalProperty * const portalProperty = shipObject->getPortalProperty();
	if (portalProperty != 0)
	{
		for (int i = 0; cs_cockpitCellNames[i] != 0; ++i)
		{
			CellProperty * const cellProperty = portalProperty->getCell(cs_cockpitCellNames[i]);
			if (cellProperty != 0)
			{
				return cellProperty;
			}
		}
	}

	return shipObject->getParentCell();
}

//-------------------------------------------------------------------

Object * HyperspaceIoWinNamespace::findTopMostRootObject(Object * const object)
{
	if (object != 0)
	{
		Object * topObject = object;

		do
		{
			{
				Object * parentObject = topObject->getParent();

				for (; parentObject != 0; parentObject = topObject->getParent())
				{
					topObject = parentObject;
				}
			}

			ContainedByProperty * containedByProperty = topObject->getContainedByProperty();
			while (containedByProperty != 0)
			{
				Object * const containerObject = containedByProperty->getContainedBy();

				if (containerObject != 0)
				{
					topObject = containerObject;

					containedByProperty = topObject->getContainedByProperty();
				}
				else
				{
					containedByProperty = 0;
				}
			}
		} while (topObject->getParent() != 0);

		return topObject;
	}

	return 0;
}

//-------------------------------------------------------------------

void HyperspaceIoWinNamespace::deactivateSpaceEnvironmentObjects()
{
	// don't render the distance objects
	TerrainObject * const terrainObject = TerrainObject::getInstance();
	Appearance * const appearance = (terrainObject != 0) ? terrainObject->getAppearance() : 0;
	ClientSpaceTerrainAppearance * const clientSpaceTerrainAppearance = dynamic_cast<ClientSpaceTerrainAppearance *>(appearance);
	if (clientSpaceTerrainAppearance != 0)
	{
		SpaceEnvironment & spaceEnvironment = clientSpaceTerrainAppearance->getSpaceEnvironment();
		spaceEnvironment.disableEnvironmentForHyperspace();
	}
}

//-------------------------------------------------------------------

void HyperspaceIoWinNamespace::disableOtherObjectsThisFrame(ShipObject * const playerShip, GameCamera const * const currentCamera)
{
	CellProperty * const targetCellProperty_p = (playerShip != 0) ? findShipCameraCell(playerShip) : 0;

	int begin = static_cast<int>(WOL_Tangible);
	int end = static_cast<int>(WOL_TangibleNotTargetable) + 1;
	// don't render other obects in the world
	for (int i = begin; i < end; ++i)
	{
		int const numberOf = World::getNumberOfObjects(i);

		for (int j = 0; j < numberOf; ++j)
		{
			Object const * const object = findTopMostRootObject(World::getObject(i, j));

			if (object == playerShip)
			{
				continue;
			}

			if (object == currentCamera)
			{
				continue;
			}

			if ((object != 0) && (object->getParentCell() == targetCellProperty_p))
			{
				continue;			
			}

			RenderWorld::recursivelyDisableDpvsObjectsForThisRender(object);
		}
	}
}

//-------------------------------------------------------------------

void HyperspaceIoWinNamespace::letterBoxCamera(GameCamera * const camera)
{
	// currently not used and sits at the bottom
	int minimumX = 0, minimumY = 0;
	int maximumX = 0, maximumY = 0;

	camera->getViewport(minimumX, minimumY, maximumX, maximumY);

	int const originalWidth = maximumY - minimumX;
	int const originalHeight = maximumY - minimumX;

	int const newHeight = static_cast<int>(static_cast<float>(originalWidth) / 2.35f);

	int const heightDifferenceHalf = ((originalHeight - newHeight) / 2);

	camera->setViewport(minimumX, minimumY + heightDifferenceHalf, maximumX, maximumY - heightDifferenceHalf);	
}


//-------------------------------------------------------------------

using namespace HyperspaceIoWinNamespace;

// ==================================================================

class HyperspaceIoWin::Stage
{
public:
	virtual ~Stage();

	virtual void begin() = 0;
	virtual void alter(float time) = 0;
	virtual void draw() = 0;
	virtual bool isDone() const = 0;
	virtual void finish() = 0;

protected:
	Stage();

private:
	Stage(Stage const &);
	Stage & operator=(Stage const &);
};

// ==================================================================

class CockpitStage : public HyperspaceIoWin::Stage
{
public:
	CockpitStage(CockpitCamera * const sourceCamera, ShipObject * const playerShip);
	virtual ~CockpitStage();

	virtual void begin();
	virtual void alter(float time);
	virtual void draw();
	virtual bool isDone() const;
	virtual void finish();

private:
	CockpitStage();
	CockpitStage(CockpitStage const &);
	CockpitStage & operator=(CockpitStage const &);

	virtual CockpitStageData const & getStageData() const;
	virtual bool hideOtherObjects() const;

private:
	Watcher<CockpitCamera> m_sourceCamera;
	GameCamera * const m_camera;
	Watcher<ShipObject> m_playerShip;
	float m_elapsedTime;
	Watcher<Object> m_particleObject;
	bool m_deactivatedEnvironment;
};

// ==================================================================

class CockpitLeaveStage : public CockpitStage
{
public:
	CockpitLeaveStage(CockpitCamera * const sourceCamera, ShipObject * const playerShip);
	virtual ~CockpitLeaveStage();

private:
	CockpitLeaveStage();
	CockpitLeaveStage(CockpitLeaveStage const &);
	CockpitLeaveStage & operator=(CockpitLeaveStage const &);

	virtual CockpitStageData const & getStageData() const;
	virtual bool hideOtherObjects() const;
};

// ==================================================================

class ThirdPersonStage : public HyperspaceIoWin::Stage
{
public:
	ThirdPersonStage(CockpitCamera * const sourceCamera, ShipObject * const playerShip);
	virtual ~ThirdPersonStage();

	virtual void begin();
	virtual void alter(float time);
	virtual void draw();
	virtual bool isDone() const;
	virtual void finish();

private:
	ThirdPersonStage();
	ThirdPersonStage(ThirdPersonStage const &);
	ThirdPersonStage & operator=(ThirdPersonStage const &);

private:
	Watcher<CockpitCamera> m_sourceCamera;
	GameCamera * const m_camera;
	Watcher<ShipObject> m_playerShip;
	float m_elapsedTime;
};

// ==================================================================

HyperspaceIoWin::Stage::~Stage()
{
}

//-------------------------------------------------------------------

HyperspaceIoWin::Stage::Stage()
{
}

// ==================================================================

CockpitStage::CockpitStage(CockpitCamera * const sourceCamera, ShipObject * const playerShip)
: HyperspaceIoWin::Stage()
, m_sourceCamera(sourceCamera)
, m_camera(new GameCamera)
, m_playerShip(playerShip)
, m_elapsedTime(0.0f)
, m_particleObject()
, m_deactivatedEnvironment(false)
{
	NOT_NULL(sourceCamera);
	NOT_NULL(playerShip);
}

//-------------------------------------------------------------------

CockpitStage::~CockpitStage()
{
	delete m_camera;

	if (m_particleObject != 0)
	{
		delete m_particleObject.getPointer();
		m_particleObject = 0;
	}
}

//-------------------------------------------------------------------

CockpitStageData const & CockpitStage::getStageData() const
{
	return s_enterCockpitStageData;
}

//-------------------------------------------------------------------

bool CockpitStage::hideOtherObjects() const
{
	return true;
}

//-------------------------------------------------------------------

void CockpitStage::begin()
{
	if (m_sourceCamera == 0)
	{
		return;
	}

	if (m_playerShip == 0)
	{
		return;
	}

	IGNORE_RETURN(Audio::playSound(getStageData().m_hyperspaceCockpitSoundPath.c_str()));

	// camera location
	CellProperty * const targetCellProperty_p = findShipCameraCell(m_playerShip);
	m_camera->setParentCell(targetCellProperty_p);
	CellProperty::setPortalTransitionsEnabled(false);
	m_camera->setTransform_o2p(m_sourceCamera->getHyperspaceCameraOffset());
	CellProperty::setPortalTransitionsEnabled(true);
	m_camera->setActive(true);

	SaveCameraParameters s;
	s.save(m_sourceCamera);
	s.restore(m_camera);

	ClientWorld::addCamera(m_camera);

	// if non POB, hide the player
	if (m_playerShip->getPortalProperty() == 0)
	{
		m_camera->addExcludedObject(Game::getConstPlayer());
	}

	{
		// add the particle to the world
		Appearance * appearance = AppearanceTemplateList::createAppearance(getStageData().m_particleAppearancePath.c_str());
		if (appearance != 0)
		{
			ParticleEffectAppearance * const particleEffectAppearance = ParticleEffectAppearance::asParticleEffectAppearance(appearance);

			if (particleEffectAppearance != 0)
			{
				m_particleObject = new MemoryBlockManagedObject();
				m_particleObject->setAppearance(particleEffectAppearance);

				particleEffectAppearance->setEnabled(true);

				RenderWorld::addObjectNotifications(*m_particleObject);
				SetupClientParticle::addNotifications(*m_particleObject);

				m_particleObject->addToWorld();
				m_particleObject->scheduleForAlter();

				Transform transform;
				transform.move_l(Vector(0.0f, 0.0f, getStageData().m_particleOffset));
				m_particleObject->setTransform_o2p(transform);
				m_camera->addChildObject_o(m_particleObject);
			}

			if (particleEffectAppearance == 0)
			{
				delete appearance;
			}
		}
	}

	{
		// add the client effect to the world and play it
		CrcLowerString const name(getStageData().m_clientEffectTemplatePath.c_str());
		ClientEffectTemplate const * const clientEffectTemplate = ClientEffectTemplateList::fetch(name);

		if (clientEffectTemplate != 0)
		{
			Vector const effectPosition_p(m_camera->getPosition_p());
			ClientEffect * const clientEffect = clientEffectTemplate->createClientEffect(targetCellProperty_p, effectPosition_p, Vector::unitY);
			clientEffect->execute();
			delete clientEffect;
			clientEffectTemplate->release();
		}
	}
}

//-------------------------------------------------------------------

void CockpitStage::alter(float time)
{
	m_elapsedTime += time;
}

//-------------------------------------------------------------------

void CockpitStage::draw()
{
	if (m_sourceCamera == 0)
	{
		return;
	}

	if (m_playerShip == 0)
	{
		return;
	}

	if (hideOtherObjects())
	{
		if (m_elapsedTime > s_secondsToDisableOtherObjects)
		{
			if (!m_deactivatedEnvironment)
			{
				deactivateSpaceEnvironmentObjects();
				m_deactivatedEnvironment = true;
			}
			disableOtherObjectsThisFrame(m_playerShip, m_camera);
		}
	}

	PostProcessingEffectsManager::preSceneRender();
	Bloom::preSceneRender();

	// camera location
	m_camera->setParentCell(findShipCameraCell(m_playerShip));
	CellProperty::setPortalTransitionsEnabled(false);
	m_camera->setTransform_o2p(m_sourceCamera->getHyperspaceCameraOffset());
	CellProperty::setPortalTransitionsEnabled(true);
	m_camera->setActive(true);
	
	// render the scene
	PackedRgb backgroundColor = PackedRgb::solidBlack;

	Graphics::clearViewport(false, backgroundColor.asUint32(), true, 1.0f, true, 0);
	ClientWorld::addRenderHookFunctions(m_camera);
	m_camera->renderScene();
	ClientWorld::removeRenderHookFunctions();

	// render shadow alpha
	ShadowVolume::renderShadowAlpha(m_camera);

	Bloom::postSceneRender();
	PostProcessingEffectsManager::postSceneRender();

}

//-------------------------------------------------------------------

bool CockpitStage::isDone() const
{
	return (m_elapsedTime > getStageData().m_cockpitStageTime);
}

//-------------------------------------------------------------------

void CockpitStage::finish()
{
	ClientWorld::removeCamera(m_camera);

	if (m_particleObject != 0)
	{
		m_particleObject->removeFromWorld();
	}
}

// ==================================================================

CockpitLeaveStage::CockpitLeaveStage(CockpitCamera * const sourceCamera, ShipObject * const playerShip)
: CockpitStage(sourceCamera, playerShip)
{
}

//-------------------------------------------------------------------

CockpitLeaveStage::~CockpitLeaveStage()
{
}

//-------------------------------------------------------------------

CockpitStageData const & CockpitLeaveStage::getStageData() const
{
	return s_leaveCockpitStageData;
}

//-------------------------------------------------------------------

bool CockpitLeaveStage::hideOtherObjects() const
{
	return false;
}

// ==================================================================

ThirdPersonStage::ThirdPersonStage(CockpitCamera * const sourceCamera, ShipObject * const playerShip)
: HyperspaceIoWin::Stage()
, m_sourceCamera(sourceCamera)
, m_camera(new GameCamera)
, m_playerShip(playerShip)
, m_elapsedTime(0.0f)
{
	NOT_NULL(sourceCamera);
	NOT_NULL(playerShip);
}

//-------------------------------------------------------------------

ThirdPersonStage::~ThirdPersonStage()
{
	delete m_camera;
}

//-------------------------------------------------------------------

void ThirdPersonStage::begin()
{
	if (m_sourceCamera == 0)
	{
		return;
	}

	if (m_playerShip == 0)
	{
		return;
	}

	IGNORE_RETURN(Audio::playSound(s_thirdPersonStageData.m_hyperspaceSoundPath.c_str()));

	float const maximumZoom = m_sourceCamera->getMaximumZoomOutSetting();
	Vector const offset_p(s_cameraOffsetDirection * maximumZoom);

	CellProperty * const targetCellProperty_p = m_playerShip->getParentCell();
	Transform targetTransform_p(m_playerShip->getTransform_o2c());
	targetTransform_p.move_l(offset_p);

	// camera location
	m_camera->setParentCell(targetCellProperty_p);
	CellProperty::setPortalTransitionsEnabled(false);
	m_camera->setTransform_o2p(targetTransform_p);
	CellProperty::setPortalTransitionsEnabled(true);
	m_camera->setActive(true);

	SaveCameraParameters s;
	s.save(m_sourceCamera);
	s.restore(m_camera);

	ClientWorld::addCamera(m_camera);
	deactivateSpaceEnvironmentObjects();
}

//-------------------------------------------------------------------

void ThirdPersonStage::alter(float time)
{
	if (m_sourceCamera == 0)
	{
		return;
	}

	if (m_playerShip == 0)
	{
		return;
	}

	m_elapsedTime += time;

	float const distanceToMove = s_thirdPersonStageData.m_hyperspaceSpeed * time;
	float const radiansToRoll = s_thirdPersonStageData.m_hyperspaceRadianRollsPerSecond * time;

	Transform transform_o = m_playerShip->getTransform_o2p();
	transform_o.move_l(Vector(0.0f, 0.0f, distanceToMove));
	transform_o.roll_l(radiansToRoll);

	m_playerShip->setTransform_o2p(transform_o);

	ShipController * const controller = safe_cast<ShipController *>(m_playerShip->getController());
	if (controller != 0)
	{
		controller->setShipDynamicsModelTransform(transform_o);
	}
}

//-------------------------------------------------------------------

void ThirdPersonStage::draw()
{
	if (m_sourceCamera == 0)
	{
		return;
	}

	if (m_playerShip == 0)
	{
		return;
	}

	disableOtherObjectsThisFrame(m_playerShip, m_camera);

	PostProcessingEffectsManager::preSceneRender();
	Bloom::preSceneRender();

	CellProperty * const targetCellProperty_p = m_playerShip->getParentCell();
	Transform targetTransform_p(m_playerShip->getTransform_o2c());
	targetTransform_p.setPosition_p(m_camera->getTransform_o2c().getPosition_p());

	// camera location
	m_camera->setParentCell(targetCellProperty_p);
	CellProperty::setPortalTransitionsEnabled(false);
	m_camera->setTransform_o2p(targetTransform_p);
	CellProperty::setPortalTransitionsEnabled(true);
	m_camera->setActive(true);

	// render the scene
	PackedRgb backgroundColor = PackedRgb::solidBlack;

	Graphics::clearViewport(false, backgroundColor.asUint32(), true, 1.0f, true, 0);
	ClientWorld::addRenderHookFunctions(m_camera);
	m_camera->renderScene();
	ClientWorld::removeRenderHookFunctions();

	// render shadow alpha
	ShadowVolume::renderShadowAlpha(m_camera);

	Bloom::postSceneRender();
	PostProcessingEffectsManager::postSceneRender();
}

//-------------------------------------------------------------------

bool ThirdPersonStage::isDone() const
{
	return (m_elapsedTime > s_thirdPersonStageData.m_thirdPersonStageTime);
}

//-------------------------------------------------------------------

void ThirdPersonStage::finish()
{
	ClientWorld::removeCamera(m_camera);
}

// ==================================================================

void HyperspaceIoWin::install()
{
	InstallTimer const installTimer("HyperspaceIoWin::install");

	HyperspaceIoWinNamespace::install();
	ExitChain::add(remove, "HyperspaceIoWinNamespace::remove");
}

//-------------------------------------------------------------------

bool HyperspaceIoWin::getPlayHyperspaceBeforeSceneChange()
{
	return s_playHyperspaceBeforeSceneChange;
}

//-------------------------------------------------------------------

void HyperspaceIoWin::setPlayHyperspaceBeforeSceneChange(bool const playHyperspaceBeforeSceneChange)
{
	s_playHyperspaceBeforeSceneChange = playHyperspaceBeforeSceneChange;
}

//-------------------------------------------------------------------

HyperspaceIoWin::HyperspaceIoWin(CockpitCamera * const sourceCamera, ShipObject * const playerShip, State const state, bool testing)
: IoWin("HyperspaceIoWin")
, m_stages()
, m_sourceCamera(sourceCamera)
, m_playerShip(playerShip)
, m_currentStageToRender(0)
, m_cameraModeToRestore(0)
, m_showCockpit(CockpitCamera::getShowCockpit())
, m_fieldOfView(GroundScene::getCameraFieldOfViewDegrees())
, m_currentState(state)
, m_clampMovementToZoneVolume(ShipDynamicsModel::getClampMovementToZoneVolume())
{
	if (testing)
	{
		load();
	}

	s_callback.connect(*this, &HyperspaceIoWin::onSceneChanged, static_cast<Game::Messages::SceneChanged*>(0));

	GroundScene * const groundScene = safe_cast<GroundScene * const>(Game::getScene());
	if (groundScene != 0)
	{
		m_cameraModeToRestore = groundScene->getCurrentView();
		groundScene->setView(GroundScene::CI_cockpit);
	}

	NOT_NULL(m_sourceCamera);
	NOT_NULL(m_playerShip);

	m_sourceCamera->setIsInHyperspace(true);
	GroundScene::setCameraFieldOfViewDegrees(60.0f);

	// we don't want to smack into asteroids
	CollisionCallbacks::setIgnoreCollision(true);
	ShipDynamicsModel::setClampMovementToZoneVolume(false);

	if (m_playerShip != 0)
	{
		PlayerShipController * const controller = dynamic_cast<PlayerShipController * const>(m_playerShip->getController());
		if (controller != 0)
		{
			// lock the ship and keep the controller from responding to
			// any input from the mouse, etc.
			controller->setThrottlePosition(0.0f, true);
			controller->lockInputState(true);
		}
	}

	Audio::silenceAllNonBackgroundMusic();
	setPlayHyperspaceBeforeSceneChange(m_currentState == S_enter);

	if (m_currentState == S_enter)
	{
		m_stages.push(new CockpitStage(sourceCamera, m_playerShip));
		m_stages.push(new ThirdPersonStage(sourceCamera, m_playerShip));
	}
	else
	{
		m_stages.push(new CockpitLeaveStage(sourceCamera, m_playerShip));
	}

	open();
}

//-------------------------------------------------------------------

HyperspaceIoWin::~HyperspaceIoWin()
{
	s_callback.disconnect(*this, &HyperspaceIoWin::onSceneChanged, static_cast<Game::Messages::SceneChanged*>(0));
	
	CollisionCallbacks::setIgnoreCollision(false);
	ShipDynamicsModel::setClampMovementToZoneVolume(m_clampMovementToZoneVolume);

	if (m_playerShip != 0)
	{
		PlayerShipController * const controller = dynamic_cast<PlayerShipController * const>(m_playerShip->getController());
		if (controller != 0)
		{
			controller->lockInputState(false);
		}
	}

	IoWinManager::queueInputReset();
	IoWinManager::discardUserInputUntilNextProcessEvents();

	GroundScene::setCameraFieldOfViewDegrees(m_fieldOfView);
	GroundScene * const groundScene = safe_cast<GroundScene * const>(Game::getScene());
	if (groundScene != 0)
	{
		groundScene->setView(m_cameraModeToRestore);
	}

	CockpitCamera::setShowCockpit(m_showCockpit);

	if (m_sourceCamera != 0)
	{
		m_sourceCamera->setIsInHyperspace(false);
	}

	m_currentStageToRender = 0;

	Audio::unSilenceAllNonBackgroundMusic();
}

//-------------------------------------------------------------------

IoResult HyperspaceIoWin::processEvent(IoEvent* event)
{
	Stage * stage = getCurrentStage(m_stages);

	if (stage == 0)
	{
		return IOR_BlockKillMe;
	}

	switch (event->type)
	{
		case IOET_WindowClose:
		case IOET_WindowKill:
			return IOR_PassKillMe;
		case IOET_Update:
			{
				if (stage != 0)
				{
					if (m_currentStageToRender == 0)
					{
						stage->begin();
					}
				
					if (stage->isDone())
					{
						stage->finish();
						delete stage;

						stage = popAndGetNextStage(m_stages);
						if (stage != 0)
						{
							stage->begin();
						}
					}
				}

				m_currentStageToRender = stage;

				if (stage != 0)
				{
					stage->alter(event->arg3);
					return IOR_Pass;
				}

				return IOR_BlockKillMe;
			}
		case IOET_Character:
		case IOET_KeyDown:
		case IOET_KeyUp:
		case IOET_JoystickMove:
		case IOET_JoystickButtonDown:
		case IOET_JoystickButtonUp:
		case IOET_JoystickPovHat:
		case IOET_JoystickSlider:
		case IOET_MouseMove:
		case IOET_MouseButtonDown:
		case IOET_MouseButtonUp:
			return IOR_Block;
		case IOET_WindowOpen:
		case IOET_InputReset:
		case IOET_SetSystemMouseCursorPosition:
		case IOET_Count:
		case IOET_Prepare:
		default:
			break;
	}

	return IOR_Pass;
}

//-------------------------------------------------------------------

void HyperspaceIoWin::draw() const
{
	if (!CutScene::isRunning())
	{
		if (m_currentStageToRender != 0)
		{
			m_currentStageToRender->draw();
		}
	}
}

//-------------------------------------------------------------------

void HyperspaceIoWin::onSceneChanged(bool const &)
{
	Stage * stage = getCurrentStage(m_stages);

	while (stage != 0)
	{
		stage->finish();
		delete stage;

		stage = popAndGetNextStage(m_stages);
	}

	IoWinManager::close(this);
}

//-------------------------------------------------------------------

