//
// SpaceDeath.cpp
// tford
//
// copyright 2004, sony online entertainment
//

//-------------------------------------------------------------------

#include "clientGame/FirstClientGame.h"
#include "clientGame/SpaceDeath.h"

#include "clientGame/ClientEffect.h"
#include "clientGame/ClientEffectTemplate.h"
#include "clientGame/ClientEffectTemplateList.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/CollisionCallbacks.h"
#include "clientGame/Game.h"
#include "clientGame/GameMusicManager.h"
#include "clientGame/PlayerShipController.h"
#include "clientGame/ShipObject.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientObject/SaveCameraParameters.h"
#include "clientObject/ShadowVolume.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedDebug/Profiler.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedIoWin/IoWinManager.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedMessageDispatch/Message.h"
#include "sharedObject/CellProperty.h"
#include "sharedRandom/Random.h"

namespace SpaceDeathNamespace
{
	namespace UnnamedMessages
	{
		const char * const ConnectionServerConnectionClosed = "ConnectionServerConnectionClosed";
	}
	
	char const * const m_iffFileName = "scene/spacedeath.iff";

	Tag const TAG_SDTH = TAG(S,D,T,H); // SpaceDeaTH

	// these are the lengths that each scene is played in seconds
	// until advancing to the next
	float ms_timeCutSceneAsPassenger = 0.0f;
	float ms_timeCutSceneInCockpit = 0.0f;
	float ms_timeCutSceneShipDestruction = 6.0f;
	float ms_timeCutSceneShipDestructionPadding = 1000.0f;
	float ms_timeToTransitionCameraTarget = 10.0f;

	ClientEffectTemplate const * ms_clientEffectTemplate = 0;

	int ms_minimumClientEffects = 4;
	int ms_maximumClientEffects = 8;

	bool ms_hideShipAfterExplosion = true; // should only be false if debugging

	// listener for scene change
	MessageDispatch::Callback s_callback;

	void BuildCameraPath1(WaveForm3D & waveForm, int const numberOfPoints, float const offset);
	void BuildCameraPath2(WaveForm3D & waveForm, int const numberOfPoints, float const offset);
	void BuildCameraPath3(WaveForm3D & waveForm, int const numberOfPoints, float const offset);
	void BuildCameraPath4(WaveForm3D & waveForm, int const numberOfPoints, float const offset);
	void BuildCameraPath5(WaveForm3D & waveForm, int const numberOfPoints, float const offset);
	void BuildCameraPath6(WaveForm3D & waveForm, int const numberOfPoints, float const offset);

	typedef void (*BuildCameraPathFunction)(WaveForm3D & waveForm, int const numberOfPoints, float const offset);

	int const mc_numberOfCameraPaths = 6;
	BuildCameraPathFunction cameraPathBuilders[mc_numberOfCameraPaths] = {
								BuildCameraPath1,
								BuildCameraPath2,
								BuildCameraPath3,
								BuildCameraPath4,
								BuildCameraPath5,
								BuildCameraPath6,
							};

	void BuildRandomCameraPath(WaveForm3D & waveForm, int const numberOfPoints, float const offset);

	void BuildShipDestructionEffectTimes(std::queue<float> & timeQueue);

	void load_0000(Iff & iff);
	void install();
	void remove();
}

//-------------------------------------------------------------------

void SpaceDeathNamespace::BuildCameraPath1(WaveForm3D & waveForm, int const numberOfPoints, float const offset)
{
	float const step = (1.0f / numberOfPoints);
	float time = 0.0f;
	float scale = 1.0f;
	float const scaleFactor = 1.10f;
	int const whenToScale = (numberOfPoints / 3);

	for (int i = 0; i <= numberOfPoints; ++i)
	{
		if (i > whenToScale)
		{
			scale *= scaleFactor;
		}

		float const x = cosf(time) * offset * scale;
		float const y = x * 0.75f;
		float const z = sinf(time) * offset * scale;

		waveForm.insert(time, Vector(x, y, z));

		time = std::max(0.0f, std::min(time + step, 1.0f));
	}
}

//-------------------------------------------------------------------

void SpaceDeathNamespace::BuildCameraPath2(WaveForm3D & waveForm, int const numberOfPoints, float const offset)
{
	float const step = (1.0f / numberOfPoints);
	float time = 0.0f;
	float scale = 1.0f;
	float const scaleFactor = 1.05f;
	int const whenToScale = (numberOfPoints / 3);

	for (int i = 0; i <= numberOfPoints; ++i)
	{
		if (i > whenToScale)
		{
			scale *= scaleFactor;
		}

		float const x = cosf(time) * offset * scale;
		float const y = x * 0.5f;
		float const z = sinf(time) * offset * scale;

		waveForm.insert(time, Vector(x, y, z));

		time = std::max(0.0f, std::min(time + step, 1.0f));
	}
}

//-------------------------------------------------------------------

void SpaceDeathNamespace::BuildCameraPath3(WaveForm3D & waveForm, int const numberOfPoints, float const offset)
{
	float const step = (1.0f / numberOfPoints);
	float time = 0.0f;
	float scale = 1.0f;
	float const scaleFactor = 1.08f;
	int const whenToScale = (numberOfPoints / 3);

	for (int i = 0; i <= numberOfPoints; ++i)
	{
		if (i > whenToScale)
		{
			scale *= scaleFactor;
		}

		float const x = -cosf(time) * offset * scale;
		float const y = -x * 0.75f;
		float const z = sinf(time) * offset * scale;

		waveForm.insert(time, Vector(x, y, z));

		time = std::max(0.0f, std::min(time + step, 1.0f));
	}
}

//-------------------------------------------------------------------

void SpaceDeathNamespace::BuildCameraPath4(WaveForm3D & waveForm, int const numberOfPoints, float const offset)
{
	float const step = (1.0f / numberOfPoints);
	float time = 0.0f;
	float scale = 1.0f;
	float const scaleFactor = 1.07f;
	int const whenToScale = (numberOfPoints / 3);

	for (int i = 0; i <= numberOfPoints; ++i)
	{
		if (i > whenToScale)
		{
			scale *= scaleFactor;
		}

		float const x = -cosf(time) * offset * scale;
		float const y = -x * 0.5f;
		float const z = sinf(time) * offset * scale;

		waveForm.insert(time, Vector(x, y, z));

		time = std::max(0.0f, std::min(time + step, 1.0f));
	}
}

//-------------------------------------------------------------------

void SpaceDeathNamespace::BuildCameraPath5(WaveForm3D & waveForm, int const numberOfPoints, float const offset)
{
	float const step = (1.0f / numberOfPoints);
	float time = 0.0f;
	float scale = 1.0f;
	float const scaleFactor = 1.09f;
	int const whenToScale = (numberOfPoints / 3);

	for (int i = 0; i <= numberOfPoints; ++i)
	{
		if (i > whenToScale)
		{
			scale *= scaleFactor;
		}

		float const cosOfTime = cosf(time);
		float const x = cosOfTime * offset * 0.25f * scale;
		float const y = cosOfTime * offset * 0.5f;
		float const z = sinf(time) * offset * scale;

		waveForm.insert(time, Vector(x, y, z));

		time = std::max(0.0f, std::min(time + step, 1.0f));
	}
}

//-------------------------------------------------------------------

void SpaceDeathNamespace::BuildCameraPath6(WaveForm3D & waveForm, int const numberOfPoints, float const offset)
{
	float const step = (1.0f / numberOfPoints);
	float time = 0.0f;
	float scale = 1.0f;
	float const scaleFactor = 1.07f;
	int const whenToScale = (numberOfPoints / 3);

	for (int i = 0; i <= numberOfPoints; ++i)
	{
		if (i > whenToScale)
		{
			scale *= scaleFactor;
		}

		float const cosOfTime = cosf(time);
		float const x = -cosOfTime * offset * 0.25f * scale;
		float const y = cosOfTime * offset * 0.5f;
		float const z = sinf(time) * offset;

		waveForm.insert(time, Vector(x, y, z));

		time = std::max(0.0f, std::min(time + step, 1.0f));
	}
}

//-------------------------------------------------------------------

void SpaceDeathNamespace::BuildRandomCameraPath(WaveForm3D & waveForm, int const numberOfPoints, float const offset)
{
	int index = Random::random(0, mc_numberOfCameraPaths - 1);
	SpaceDeathNamespace::cameraPathBuilders[index](waveForm, numberOfPoints, offset);
}

//-------------------------------------------------------------------

void SpaceDeathNamespace::BuildShipDestructionEffectTimes(std::queue<float> & timeQueue)
{
	int const numberOfSteps = Random::random(ms_minimumClientEffects, ms_maximumClientEffects);

	float const step = (ms_timeCutSceneShipDestruction / numberOfSteps);
	float time = 0.0f;
	float scale = 1.0f;

	for (int i = 0; i < numberOfSteps; ++i)
	{
		time += (step * scale);
		timeQueue.push(time);
		scale *= 0.9f;
	}
}

//-------------------------------------------------------------------

void SpaceDeathNamespace::load_0000(Iff & iff)
{
	iff.enterForm(TAG_0000);
		iff.enterChunk(TAG_DATA);

			ms_timeCutSceneAsPassenger = iff.read_float();

			ms_timeCutSceneInCockpit = iff.read_float();

			ms_timeCutSceneShipDestruction = iff.read_float();
			if (ms_timeCutSceneShipDestruction < 1.f)
				ms_timeCutSceneShipDestruction = 1.f;

			ms_timeCutSceneShipDestructionPadding = iff.read_float();

			ms_minimumClientEffects = iff.read_int32();
			ms_maximumClientEffects = iff.read_int32();
			if (ms_minimumClientEffects > ms_maximumClientEffects)
				std::swap(ms_minimumClientEffects, ms_maximumClientEffects);

		iff.exitChunk();
	iff.exitForm(TAG_0000);
}

//-------------------------------------------------------------------

void SpaceDeathNamespace::install()
{
	Iff iff;
	if (iff.open(m_iffFileName, true))
	{
		iff.enterForm(TAG_SDTH);

		switch (iff.getCurrentName())
		{
			case TAG_0000:
				SpaceDeathNamespace::load_0000(iff);
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
		DEBUG_WARNING(true,("SpaceDeathNamespace::install unable to open file %s", m_iffFileName));
	}

	CrcLowerString const playerDeathEffect("clienteffect/combat_player_ship_hit_death.cef");
	SpaceDeathNamespace::ms_clientEffectTemplate = ClientEffectTemplateList::fetch(playerDeathEffect);
	DebugFlags::registerFlag(ms_hideShipAfterExplosion, "ClientGame/SpaceDeath", "hideShipAfterExplosion");
}

//-------------------------------------------------------------------

void SpaceDeathNamespace::remove()
{
	if (ms_clientEffectTemplate != 0)
	{
		SpaceDeathNamespace::ms_clientEffectTemplate->release();
		SpaceDeathNamespace::ms_clientEffectTemplate = 0;
	}
	DebugFlags::unregisterFlag(ms_hideShipAfterExplosion);
}

// ==================================================================

using namespace SpaceDeathNamespace;

//-------------------------------------------------------------------

// this is a small class that will allow the camera to interpolate between a
// queue of objects over a given amount of time for each object.  This was
// originally built to follow pieces of debris as they fall away from the
// ship as it explodes.  Currently it's only being used to have the camera
// look at the origin over a long amount of time to give additional movement
// to the screen after the player ship has exploded and been removed.

// This assumes that all objects are in the same cell space!!!

class SpaceDeath::CameraTarget
{
public:

	explicit CameraTarget(Object * toLookAtObject);
	~CameraTarget();

	// all objects are in the same cell space!!!
	void setNextLookAt(Object * toLookAtObject, float timeToTransition);

	// tickle the transition once a frame
	void update(float elapsedTime);

	// where the camera should be pointing at
	Vector calculateCurrentLookAt();

private:

	struct NextObject
	{
	public:

		NextObject(Object * toLookAtObject, float timeToTransition);

	public:

		Watcher<Object> m_object;
		float m_timeToTransition;

	private:

		NextObject();
	};

private:

	CameraTarget();
	CameraTarget(CameraTarget const &);
	CameraTarget & operator = (CameraTarget const &);

private:

	Watcher<Object> m_lookAtFromObject;
	Watcher<Object> m_lookAtToObject;

	typedef std::queue<NextObject *> NextObjectQueue;
	NextObjectQueue m_nextObjects;

	Vector m_lookAtFromLocation;
	Vector m_lookAtToLocation;
	Vector m_lookAt;

	float m_totalElapsedTime;
	float m_timeToTransition;
	float m_fractionalTime;
};

//-------------------------------------------------------------------

SpaceDeath::CameraTarget::NextObject::NextObject(Object * toLookAtObject, float const timeToTransition)
: m_object(toLookAtObject)
, m_timeToTransition(timeToTransition)
{
}

//-------------------------------------------------------------------

SpaceDeath::CameraTarget::CameraTarget(Object * toLookAtObject)
: m_lookAtFromObject(toLookAtObject)
, m_lookAtToObject(toLookAtObject)
, m_nextObjects()
, m_lookAtFromLocation()
, m_lookAtToLocation()
, m_lookAt()
, m_totalElapsedTime(0.0f)
, m_timeToTransition(0.0f)
, m_fractionalTime(0.f)
{
}

//-------------------------------------------------------------------

SpaceDeath::CameraTarget::~CameraTarget()
{
	while (!m_nextObjects.empty())
	{
		CameraTarget::NextObject * nextObject = m_nextObjects.front();
		m_nextObjects.pop();
		delete nextObject;
	}
}

//-------------------------------------------------------------------

void SpaceDeath::CameraTarget::setNextLookAt(Object * toLookAtObject, float const timeToTransition)
{
	m_nextObjects.push(new CameraTarget::NextObject(toLookAtObject, timeToTransition));
}

//-------------------------------------------------------------------

void SpaceDeath::CameraTarget::update(float const elapsedTime)
{
	m_totalElapsedTime += elapsedTime;

	m_fractionalTime = m_totalElapsedTime / m_timeToTransition;

	if (m_fractionalTime > 1.0f)
	{
		// once we've passed 1.0f then we can start transitioning
		// to the next object in the queue
		if (!m_nextObjects.empty())
		{
			CameraTarget::NextObject * nextObject = m_nextObjects.front();
			m_nextObjects.pop();

			m_lookAtFromObject = m_lookAtToObject;
			m_lookAtToObject = nextObject->m_object;
			m_timeToTransition = nextObject->m_timeToTransition;
			m_totalElapsedTime = 0.0f;
			m_fractionalTime = 0.0f;

			delete nextObject;
		}
		else
		{
			m_fractionalTime = 1.0f;
		}
	}
}

//-------------------------------------------------------------------

Vector SpaceDeath::CameraTarget::calculateCurrentLookAt()
{
	// assumes that m_lookAtFromObject and m_lookAtToObject are in the same cell space

	if (m_lookAtFromObject != 0)
	{
		m_lookAtFromLocation = m_lookAtFromObject->getTransform_o2c().getPosition_p();
	}

	if (m_lookAtToObject != 0)
	{
		m_lookAtToLocation = m_lookAtToObject->getTransform_o2c().getPosition_p();
		m_lookAt = m_lookAtToLocation;
	}

	// interpolate between the 2 values based off of m_fractionalTime
	if (m_lookAtFromObject != m_lookAtToObject)
	{
		m_lookAt = Vector::linearInterpolate(m_lookAtFromLocation, m_lookAtToLocation, m_fractionalTime);
	}

	return(m_lookAt);
}

// ==================================================================

SpaceDeath::SpaceDeath(GameCamera const * const sourceCamera, ShipObject * const playerShip, float const destructionSeverity, bool const testing)
: IoWin("SpaceDeath")
, MessageDispatch::Receiver()
, m_camera(new GameCamera)
, m_playerShip(playerShip)
, m_destructionSeverity(destructionSeverity)
, m_testing(testing)
, m_waveForm3D()
, m_cameraOffset_p()
, m_currentCutSceneTime(0.0f)
, m_state(S_cutSceneShipDestruction)
, m_shipDestructionEffectTimes()
, m_cameraTarget(new SpaceDeath::CameraTarget(playerShip))
{
	s_callback.connect(*this, &SpaceDeath::onSceneChanged, static_cast<Game::Messages::SceneChanged*>(0));
	connectToMessage(UnnamedMessages::ConnectionServerConnectionClosed);

	DEBUG_REPORT_LOG(true, ("SpaceDeath::SpaceDeath\n"));

	PlayerShipController * const controller = dynamic_cast<PlayerShipController * const>(m_playerShip->getController());
	if (controller != 0)
	{
		// keep the controller from responding to any input from the mouse, etc.
		controller->lockInputState(true);
	}

	// we don't want to smack into asteroids after the ship has exploded
	CollisionCallbacks::setIgnoreCollision(true);

	GameMusicManager::setMusicTrackOverride("sound/music_space_player_death.snd");

	// data used by CutSceneShipDestruction
	BuildRandomCameraPath(m_waveForm3D, 24, m_playerShip->getAppearanceSphereRadius() * 2.0f);
	BuildShipDestructionEffectTimes(m_shipDestructionEffectTimes);

	// camera location
	Transform const & transform = sourceCamera->getTransform_o2w();
	m_camera->setTransform_o2w(transform);
	m_camera->setActive(true);

	SaveCameraParameters s;
	s.save(sourceCamera);
	s.restore(m_camera);

	ClientWorld::addCamera(m_camera);

	open();
}

//-------------------------------------------------------------------

SpaceDeath::~SpaceDeath()
{
	s_callback.disconnect(*this, &SpaceDeath::onSceneChanged, static_cast<Game::Messages::SceneChanged*>(0));

	PlayerShipController * const controller = dynamic_cast<PlayerShipController * const>(m_playerShip->getController());
	if (controller != 0)
	{
		controller->lockInputState(false);
	}

	CollisionCallbacks::setIgnoreCollision(false);

	ClientWorld::removeCamera(m_camera);

	delete m_camera;
	delete m_cameraTarget;

	IoWinManager::queueInputReset();
	IoWinManager::discardUserInputUntilNextProcessEvents();
}

//-------------------------------------------------------------------

void SpaceDeath::install()
{
	InstallTimer const installTimer("SpaceDeath::install");

	SpaceDeathNamespace::install();

	ExitChain::add(remove, "SpaceDeathNamespace::remove");
}

//-------------------------------------------------------------------

void SpaceDeath::onSceneChanged(Game::Messages::SceneChanged::Payload const &)
{
	IoWinManager::close(this);
}

//-------------------------------------------------------------------

void SpaceDeath::receiveMessage(const MessageDispatch::Emitter & , const MessageDispatch::MessageBase & message)
{	
	if (message.isType(UnnamedMessages::ConnectionServerConnectionClosed))
	{
		IoWinManager::close(this);
	}
}

//-------------------------------------------------------------------

IoResult SpaceDeath::processEvent(IoEvent* event)
{
	switch (event->type)
	{
		case IOET_WindowKill:
		case IOET_WindowClose:
			return IOR_PassKillMe;

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
			{
#if _DEBUG
				if (m_testing && (m_state == S_cutSceneShipDestructionPadding || m_state == S_done))
					return IOR_BlockKillMe;
#endif

				return IOR_Block;
			}

		case IOET_Update:
			{
				// update state information
				float const elapsedTime = event->arg3;
				m_currentCutSceneTime += elapsedTime;

				switch(m_state)
				{
					case S_cutSceneAsPassenger :
						updateCutSceneAsPassenger(elapsedTime);
						nextCutSceneIfReady(ms_timeCutSceneAsPassenger);
						break;
					case S_cutSceneInCockpit :
						updateCutSceneInCockpit(elapsedTime);
						nextCutSceneIfReady(ms_timeCutSceneInCockpit);
						break;
					case S_cutSceneShipDestruction :
						updateCutSceneShipDestruction(elapsedTime);
						nextCutSceneIfReady(ms_timeCutSceneShipDestruction);
						break;
					case S_cutSceneShipDestructionPadding :
						updateCutSceneShipDestructionPadding(elapsedTime);
						nextCutSceneIfReady(ms_timeCutSceneShipDestructionPadding);
						break;
					case S_done:
					default:
						return IOR_BlockKillMe;
				}
			}
			break;

		default:
			break;
	}

	return IOR_Pass;
}

//-------------------------------------------------------------------

void SpaceDeath::draw() const
{
	switch(m_state)
	{
		case S_cutSceneAsPassenger :
			drawCutSceneAsPassenger();
			break;
		case S_cutSceneInCockpit :
			drawCutSceneInCockpit();
			break;
		case S_cutSceneShipDestruction :
			drawCutSceneShipDestruction();
			break;
		case S_cutSceneShipDestructionPadding :
			drawCutSceneShipDestructionPadding();
			break;
		case S_done:
		default:
			break;
	}

	// render the scene
	PackedRgb backgroundColor = PackedRgb::solidBlack;
	Graphics::clearViewport(true, backgroundColor.asUint32(), true, 1.0f, true, 0);
	ClientWorld::addRenderHookFunctions(m_camera);
	m_camera->renderScene();
	ClientWorld::removeRenderHookFunctions();

	// render shadow alpha
	ShadowVolume::renderShadowAlpha(m_camera);
}

//-------------------------------------------------------------------

// simple method of when to jump to next screen...
void SpaceDeath::nextCutSceneIfReady(float const maxTimeThisCutScene)
{
	if (m_currentCutSceneTime > maxTimeThisCutScene)
	{
		m_currentCutSceneTime = 0.0f;
		m_state = static_cast<State>(static_cast<int>(m_state) + 1);
	}
}

//-------------------------------------------------------------------

float const SpaceDeath::getCurrentCutSceneTime() const
{
	return(m_currentCutSceneTime);
}

//-------------------------------------------------------------------

void SpaceDeath::updateCutSceneAsPassenger(float const /*elapsedTime*/)
{
}

//-------------------------------------------------------------------

void SpaceDeath::drawCutSceneAsPassenger() const
{
}

//-------------------------------------------------------------------

void SpaceDeath::updateCutSceneInCockpit(float const /*elapsedTime*/)
{
}

//-------------------------------------------------------------------

void SpaceDeath::drawCutSceneInCockpit() const
{
}

//-------------------------------------------------------------------

void SpaceDeath::updateCutSceneShipDestruction(float const elapsedTime)
{
	// check that it's time to play an effect
	if (!m_shipDestructionEffectTimes.empty())
	{
		float const effectTime = m_shipDestructionEffectTimes.front();
		if (getCurrentCutSceneTime() > effectTime)
		{
			// it's time for an effect to be played
			m_shipDestructionEffectTimes.pop();

			CellProperty * cellProperty_p = m_playerShip->getParentCell();

			float const radius = m_playerShip->getAppearanceSphereRadius();
			float const radiusOver2 = radius * 0.5f;

			float const randomX = Random::randomReal(-radiusOver2, radiusOver2);
			float const randomY = Random::randomReal(0.0f, radiusOver2);
			float const randomZ = Random::randomReal(radiusOver2, radius);

			Vector const randomOffset(randomX, randomY, randomZ);
			Vector const position_p(m_playerShip->getPosition_p() + randomOffset);
			Vector const up_p(Vector::unitY);

			if (ms_clientEffectTemplate != 0)
			{
				ClientEffect * const clientEffect = ms_clientEffectTemplate->createClientEffect(cellProperty_p, position_p, up_p);
				clientEffect->execute();
				delete clientEffect;
			}

			// if this is the last effect that we just played then
			// blow up the ship
			if (m_shipDestructionEffectTimes.empty())
			{
				if (ms_hideShipAfterExplosion)
				{
					m_camera->setExcludedObject(m_playerShip);
				}

				ShipObject::DestructionDebrisList debrisList;

				NP_PROFILER_AUTO_BLOCK_DEFINE("SpaceDeath::updateCutSceneShipDestruction");

				m_playerShip->handleFinalShipDestruction(m_destructionSeverity, &debrisList);

#if 0 // ability to track debris objects with the camera -- disabled for now
				ShipObject::DestructionDebris::const_iterator ii = debris.begin();
				ShipObject::DestructionDebris::const_iterator iiEnd = debris.end();

				// assign the camera target to the largest piece of debris
				Object * largestDebrisObject = 0;
				float largestDebrisRadius = 0.0f;

				for (; ii != iiEnd; ++ii)
				{
					Object * const debrisObject = *ii;
					float const debrisRadius = debrisObject->getAppearanceSphereRadius();
					if (largestDebrisRadius < debrisRadius)
					{
						largestDebrisRadius = debrisRadius;
						largestDebrisObject = debrisObject;
					}
				}

				if (largestDebrisObject != 0)
				{
					// toss in another piece of debris for fun
					Object * debrisObject = 0;

					for (ii = debris.begin(); ((ii != iiEnd) && (debrisObject == 0)); ++ii)
					{
						if (*ii != largestDebrisObject)
						{
							debrisObject = *ii;
						}
					}

					if (debrisObject != 0)
					{
						float const shortTime = m_timeToTransitionCameraTarget * 0.5f;
						m_cameraTarget->setNextLookAt(debrisObject, shortTime);
					}

					m_cameraTarget->setNextLookAt(largestDebrisObject, m_timeToTransitionCameraTarget);
				}
#endif
			}
		}
	}

	m_cameraTarget->update(elapsedTime);

	float const time = std::max(0.0f, std::min((getCurrentCutSceneTime() / ms_timeCutSceneShipDestruction), 1.0f));

	// move the camera along the waveform spline
	Vector pointAtTime;
	m_waveForm3D.getValue(time, pointAtTime);

	m_cameraOffset_p.resetRotateTranslate_l2p();
	m_cameraOffset_p.setPosition_p(pointAtTime);
}

//-------------------------------------------------------------------

void SpaceDeath::drawCutSceneShipDestruction() const
{
	// place the camera...
	CellProperty * cellProperty_p = m_playerShip->getParentCell();
	Transform const shipTransform_p(m_playerShip->getTransform_o2c());

	Vector const toLookAt_p = m_cameraTarget->calculateCurrentLookAt();

	Transform targetTransform_p;
	targetTransform_p.multiply(shipTransform_p, m_cameraOffset_p);

	m_camera->setParentCell(cellProperty_p);
	CellProperty::setPortalTransitionsEnabled(false);
	m_camera->setTransform_o2p(targetTransform_p);
	m_camera->lookAt_p(toLookAt_p);
	CellProperty::setPortalTransitionsEnabled(true);
}

//-------------------------------------------------------------------

void SpaceDeath::updateCutSceneShipDestructionPadding(float const elapsedTime)
{
	m_cameraTarget->update(elapsedTime);
}

//-------------------------------------------------------------------

void SpaceDeath::drawCutSceneShipDestructionPadding() const
{
	drawCutSceneShipDestruction();
}

