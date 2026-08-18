//===================================================================
//
// FreeChaseCamera.cpp
// asommers 5-31-2000
//
// copyright 2000, verant interactive, inc.
//
//===================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/FreeChaseCamera.h"

#include "UnicodeUtils.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GroundScene.h"
#include "clientGame/PlayerCreatureController.h"
#include "clientGame/PlayerShipController.h"
#include "clientGame/ShipObject.h"
#include "clientGraphics/DebugPrimitive.h"
#include "clientGraphics/RenderWorld.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientUserInterface/CuiIoWin.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "sharedCollision/CollideParameters.h"
#include "sharedCollision/CollisionInfo.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/Profiler.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/MessageQueue.h"
#include "sharedGame/GameObjectTypes.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedMath/Transform.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/CellProperty.h"
#include "sharedTerrain/TerrainObject.h"
#include "sharedUtility/LocalMachineOptionManager.h"

#include <map>

//===================================================================

namespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	const float MAX_PITCH_FABS = PI_OVER_2 - PI / 64.0f;

	float ms_spinYawPerSecond = PI_OVER_4 * 0.5f;

	const Tag TAG_DEFH = TAG (D,E,F,H);
	const Tag TAG_FCHS = TAG (F,C,H,S);
	const Tag TAG_FRST = TAG (F,R,S,T);
	const Tag TAG_PSHM = TAG (P,S,H,M);
	const Tag TAG_PSTM = TAG (P,S,T,M);
	const Tag TAG_ZOOM = TAG (Z,O,O,M);
	const Tag TAG_INZM = TAG (I,N,Z,M);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifdef _DEBUG
	bool ms_debugReport;
	bool ms_renderPivotFrame;
#endif

	int   ms_cameraMode = 1;
	int   ms_currentSetting = -1;
	float ms_pitch;
	float ms_cameraZoomSpeed;
	bool  ms_cameraSimpleCollision;
	bool  ms_useCameraOffset;
	bool  ms_cameraOffsetChanged = false;
	float ms_defaultCameraHeight = 1.0f;

	bool s_installed = false;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	CreatureObject const * getCreatureObject (Object const * const object)
	{
		ClientObject const * const clientObject = object ? object->asClientObject() : 0;
		CreatureObject const * const creatureObject = clientObject ? clientObject->asCreatureObject() : 0;

		return creatureObject;
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool shouldBeInvisibleInFirstPerson(CreatureObject const * const mount)
	{
		// Note: this function is a hack to remove a bug with the jetpacks.  What this function
		// should really do is look up a boolean on the object template.
		return strstr(mount->getObjectTemplate()->getName(), "jetpack") != NULL;
	}

	static const float cs_cameraSnapBackSpeedModifier = 6.5f; // <-- this is arbitrary

	// Chase-camera collision / ground clearances. Added to stop the camera
	// dropping below the avatar and rendering the world's backfaces from under
	// the ground when the view is pitched up from behind the character.
	//  - backoff: keep the camera off any surface the collision rays hit, so the
	//    near plane never straddles it (the simple-collision path already does
	//    this; the diamond path used to leave the camera flush -> see-through).
	//  - groundClearance: how far the camera must stay above the ground/footing.
	//    The near plane is 1m, so this also accounts for the bottom of the near
	//    frustum (>= nearPlane * tan(halfVerticalFov)) staying above the ground.
	static const float cs_cameraCollisionBackoff = 0.25f;
	static const float cs_cameraGroundClearance = 0.50f;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
}

//----------------------------------------------------------------------

float        FreeChaseCamera::ms_vehicleCameraOffsetY = 0.0f;

//===================================================================
// PUBLIC FreeChaseCamera
//===================================================================

//lint -esym(1926, FreeChaseCamera::m_offsets) // (Note -- Symbol 'FreeChaseCamera::m_offsets's default constructor implicitly called -- Effective C++ #12) // C-style arrays cannot appear in constructor list.

FreeChaseCamera::FreeChaseCamera() : GameCamera(),
									 m_queue(0),
									 m_target(0),
									 m_currentPosture(Postures::Invalid),
									 m_currentStates(0),
									 m_currentFirstPerson(false),
									 m_desiredOffset(),
									 m_offset(),
									 m_yaw_w(0),
									 m_offsetYaw_w(0),
									 m_zoom(0),
									 m_zoomMultiplier(1.0f),
									 m_radius(0),
									 m_currentZoom(0),
									 m_firstPerson(false),
									 m_lastTurnRate(0.0f),
									 m_numberOfSettings(0),
									 m_settings(0),
									 m_firstPersonDistance(0.5f),
									 m_offsetDefault(),
									 m_thirdPersonXOffset(0.f),
									 m_offsetPostureMap(new PostureMap),
									 m_offsetPostureStateMap(new PostureStateMap),
									 m_modeCallback(0),
									 m_context(0),
									 m_currentMode(false),
									 m_spinning(false),
									 m_colliding(false),
									 m_firstPersonEyeHeight(1.7f)
{
#ifdef _DEBUG
	DebugFlags::registerFlag (ms_debugReport, "ClientGame/FreeChaseCamera", "debugReport");
	DebugFlags::registerFlag (ms_renderPivotFrame, "ClientGame/FreeChaseCamera", "renderPivotFrame");
#endif

	if (!s_installed)
	{
		ms_spinYawPerSecond = convertDegreesToRadians (ConfigFile::getKeyFloat ("ClientGame", "spinYawDegreesPerSecond", convertRadiansToDegrees (ms_spinYawPerSecond)));
		ms_cameraZoomSpeed = ConfigClientGame::getFreeChaseCameraZoomSpeed ();

		const char * const section = "ClientGame/FreeChaseCamera";
		LocalMachineOptionManager::registerOption (ms_cameraMode,      section, "cameraMode", 1);
		LocalMachineOptionManager::registerOption (ms_currentSetting,  section, "currentSetting");
		LocalMachineOptionManager::registerOption (ms_pitch,           section, "pitch");
		LocalMachineOptionManager::registerOption (ms_cameraZoomSpeed, section, "cameraZoomSpeed");
		LocalMachineOptionManager::registerOption (ms_cameraSimpleCollision, section, "cameraSimpleCollision");

		s_installed = true;
	}

	Iff iff;
	if (iff.open ("camera/freechasecamera.iff"))
	{
		iff.enterForm (TAG_FCHS);
			iff.enterForm (TAG_0000);

				iff.enterChunk (TAG_ZOOM);
				{
					m_numberOfSettings = iff.getChunkLengthTotal() / static_cast<int>(sizeof(float));
					m_settings = new float [static_cast<size_t> (m_numberOfSettings)];

					int i;
					for (i = 0; i < m_numberOfSettings; ++i)
						m_settings [i] = iff.read_float () * ConfigClientGame::getFreeChaseCameraMaximumZoom ();
				}
				iff.exitChunk (TAG_ZOOM);

				// INZM (initial zoom) is a post-NGE addition. The NGE-retail
				// freechasecamera.iff lacks it - the file goes straight from
				// ZOOM to FRST. Make INZM optional so we don't FATAL on the
				// older asset; default ms_currentSetting in that case.
				if (iff.enterChunk(TAG_INZM, true))
				{
					ms_currentSetting = iff.read_int32();
					iff.exitChunk(TAG_INZM);
				}

				iff.enterChunk (TAG_FRST);
					m_firstPersonDistance = iff.read_float ();
				iff.exitChunk (TAG_FRST);

				iff.enterChunk (TAG_DEFH);
					{
						int const defhTotal = iff.getChunkLengthTotal();
						int const defhFloatCount = defhTotal / static_cast<int>(sizeof(float));

						// The post-NGE freechasecamera.iff has a different
						// DEFH layout than the parser expects (we observed
						// shoulderY = 2.1e8 with the old "5 floats" parse).
						float vals[16];
						int valsToRead = (defhFloatCount > 16) ? 16 : defhFloatCount;
						for (int v = 0; v < valsToRead; ++v)
							vals[v] = iff.read_float();
						// Heuristic: pick the first sane value (|v| < 50) as the camera Y.
						// On NGE retail freechasecamera.iff:
						//   floats[0]=1.7 (x), floats[1]=2.1e8 (BAD), floats[2..4]=0
						// vals[0]=1.7 puts the camera permanently 1.7m to the side of the player
						// (player rendered off-center). Force the default X to 0 — the optional
						// shoulder offset is layered on at runtime via m_thirdPersonXOffset
						// when CuiPreferences::getOffsetCamera() is true.
						float const z = (defhFloatCount >= 3 && fabsf(vals[2]) < 50.0f) ? vals[2] : 0.0f;
						float const fileShoulderY = (defhFloatCount >= 2) ? vals[1] : 0.0f;
						float const fileCenterY = (defhFloatCount >= 5) ? vals[4] : 0.0f;

						m_reticleOffsetShoulder = (fabsf(fileShoulderY) < 50.0f && fileShoulderY > 0.001f) ? fileShoulderY : 1.4f;
						m_reticleOffsetCenter = (fabsf(fileCenterY) < 50.0f && fileCenterY > 0.001f) ? fileCenterY : 1.4f;
						m_thirdPersonXOffset = (defhFloatCount >= 4 && fabsf(vals[3]) < 50.0f) ? vals[3] : 0.0f;

						// DEFH's leading float is the nominal "default height" -- the avatar's
						// standing EYE LEVEL (~1.7m on retail). This post-NGE asset stores it as
						// a single float; the older "5 float" parse mistook it for a sideways (X)
						// offset and discarded it. Capture it as the first-person eye height; it
						// is scaled to the avatar's size at runtime. Default 1.7 if out of range.
						float const fileDefaultHeight = (defhFloatCount >= 1) ? vals[0] : 0.0f;
						m_firstPersonEyeHeight = (fileDefaultHeight > 0.5f && fileDefaultHeight < 3.0f) ? fileDefaultHeight : 1.7f;

						ms_defaultCameraHeight = m_reticleOffsetCenter;

						float const x = 0.0f;
						m_offsetDefault.set(x, (CuiPreferences::getOffsetCamera() ? m_reticleOffsetShoulder : m_reticleOffsetCenter), z);
					}
				iff.exitChunk (TAG_DEFH);

				iff.enterChunk (TAG_PSHM);
				{
					while (iff.getChunkLengthLeft ())
					{
						const int posture = iff.read_int32 ();
						const int state = iff.read_int32 ();
						const int key = (posture << 16) | state;
						const float rawY = iff.read_float();
						const Vector offset(0.f, rawY, 0.f);
						if (posture >= 0 && posture < Postures::NumberOfPostures && state >= 0 && state < States::NumberOfStates)
							m_offsetPostureStateMap->insert (std::make_pair (key, offset));
					}
				}
				iff.exitChunk (TAG_PSHM);

				iff.enterChunk (TAG_PSTM);
				{
					while (iff.getChunkLengthLeft ())
					{
						const int posture = iff.read_int32 ();
						const float rawY = iff.read_float();
						const Vector offset(0.f, rawY, 0.f);
						if (posture >= 0 && posture < Postures::NumberOfPostures)
							(*m_offsetPostureMap) [posture] = offset;
					}
				}
				iff.exitChunk (TAG_PSTM);

			iff.exitForm (TAG_0000);
		iff.exitForm (TAG_FCHS);
	}

	if (ms_currentSetting < 0 || ms_currentSetting >= m_numberOfSettings)
		ms_currentSetting = m_numberOfSettings / 2;
}

//----------------------------------------------------------------------

FreeChaseCamera::~FreeChaseCamera ()
{
#ifdef _DEBUG
	DebugFlags::unregisterFlag (ms_debugReport);
	DebugFlags::unregisterFlag (ms_renderPivotFrame);
#endif

	m_queue  = 0;
	m_target = 0;

	delete [] m_settings;
	m_settings = 0;

	delete m_offsetPostureMap;
	delete m_offsetPostureStateMap;
}

//-------------------------------------------------------------------

void FreeChaseCamera::setActive (const bool active)
{
	if (active && !isActive () && m_target)
		setTarget (m_target);

	GameCamera::setActive (active);
}

//----------------------------------------------------------------------

void FreeChaseCamera::alterCheckPostureOffsets ()
{
	CreatureObject const * const creatureObject = getCreatureObject (m_target.getPointer ());
	if (creatureObject)
	{
		bool const ridingMount = creatureObject->getState (States::RidingMount);

		const Postures::Enumerator posture = creatureObject->getVisualPosture ();
		const uint64               states  = creatureObject->getStates ();
		float const scale = ridingMount ? 1.f : creatureObject->getScaleFactor ();
		const CreatureObject * const mountCreature = ridingMount ? creatureObject->getMountedCreature () : 0;
		float const cameraHeight = mountCreature ? mountCreature->getCameraHeight (): 0.f;

		if (m_currentPosture != posture || m_currentStates != states || m_currentFirstPerson != m_firstPerson || ms_cameraOffsetChanged)
		{
			m_currentPosture = posture;
			m_currentStates  = states;
			m_currentFirstPerson = m_firstPerson;

			bool set = false;
			const float playerCameraHeight = CuiPreferences::getPlayerCameraHeight();
			// 3rd person (centered OR offset/over-the-shoulder) rides at the
			// configurable player camera height; only the X shoulder shift (applied
			// below) differs for offset cam.
			//
			// First person rides at the avatar's EYE LEVEL: the nominal default-height
			// from the camera data file (DEFH, ~1.7m), scaled to the avatar's size by
			// the '* scale' in the branch logic below. This matches the retail eye
			// height and is STABLE. We deliberately do NOT track the live "head"
			// hardpoint: it is already in the avatar's *scaled* object space (so it got
			// double-scaled here and sank to mid-chest), it bobs every frame with the
			// breathing/locomotion animation, and it sits at the base of the skull
			// (below the eyes).
			m_offsetDefault.y = m_firstPerson ? m_firstPersonEyeHeight : playerCameraHeight;

			//-- is the posture/state combination in the state map?
			{
				PostureStateMap::iterator iter = m_offsetPostureStateMap->begin ();
				PostureStateMap::iterator end = m_offsetPostureStateMap->end ();
				for (; iter != end && !set; ++iter)
				{
					//-- for every posture
					if (iter->first >> 16 == posture)
					{
						//-- check the state
						if (creatureObject->getState (static_cast<int8> (iter->first & 0x0FFFF)))
						{
							m_desiredOffset = iter->second * scale;
							if (ridingMount)
							{
								m_desiredOffset += Vector::unitY * cameraHeight;
							}

							set = true;
						}
					}
				}
			}

			//-- check the posture map
			if (!set)
			{
				PostureStateMap::iterator iter = m_offsetPostureMap->find (posture);
				if (iter != m_offsetPostureMap->end () && !set)
				{
					m_desiredOffset = iter->second * scale;
					if (ridingMount)
					{
						m_desiredOffset += Vector::unitY * cameraHeight;
					}

					set = true;
				}
			}

			//-- use the default
			if (!set)
			{
				m_desiredOffset = m_offsetDefault * scale;
				if (ridingMount)
				{
					m_desiredOffset += Vector::unitY * cameraHeight;
				}
			}

			if (!m_firstPerson || m_colliding)
			{
				const float offsetToUse = CuiPreferences::getOffsetCamera() ? m_thirdPersonXOffset : 0.0f;
				m_desiredOffset.x += offsetToUse * scale;
			}

			//-- DEFENSIVE: clamp insane camera offsets that would put us in deep space.
			//   Observed bug: m_desiredOffset.y was ~2e8 with NGE freechasecamera.iff, putting
			//   camera 200M units above the ground. Clamp to a sane range and log.
			if (m_desiredOffset.y < -50.0f || m_desiredOffset.y > 50.0f ||
				m_desiredOffset.x < -50.0f || m_desiredOffset.x > 50.0f ||
				m_desiredOffset.z < -50.0f || m_desiredOffset.z > 50.0f)
			{
				WARNING(true, ("FreeChaseCamera: invalid desired offset (%.3f, %.3f, %.3f); clamping to (0, 1.6, 0)",
							   m_desiredOffset.x, m_desiredOffset.y, m_desiredOffset.z));
				m_desiredOffset.set(0.0f, 1.6f, 0.0f);
				// Also re-snap m_offset so it doesn't lerp through bad values
				if (m_offset.y < -50.0f || m_offset.y > 50.0f)
					m_offset = m_desiredOffset;
			}
		}

		ms_cameraOffsetChanged = false;
	}
	else
	{
		m_desiredOffset = m_target && m_target->getAppearance () ? m_target->getAppearanceSphereCenter () : Vector::zero;
	}
}

//-------------------------------------------------------------------

float FreeChaseCamera::alter (float elapsedTime)
{
	PROFILER_AUTO_BLOCK_DEFINE ("FreeChaseCamera::alter");

	if (!isActive ())
	{
		// @todo consider returning no alter, then have setActive put us on the alter scheduler.
		return AlterResult::cms_alterNextFrame;
	}

	if (!m_target) 
		return AlterResult::cms_alterNextFrame;

	const bool useModeless = CuiPreferences::getUseModelessInterface();

	// hack to force camera mode in modeless interface.
	if ( useModeless )
	{
		ms_cameraMode = CM_chase;
	}

	if ( ms_cameraMode != CM_chase )
	{
		m_yaw_w += m_offsetYaw_w;
		m_offsetYaw_w = 0.f;
	}

	alterCheckPostureOffsets ();
	
	//-- handle camera specific messages
	NOT_NULL (m_queue);
	
	CreatureObject* const creatureObject = const_cast<CreatureObject *>(getCreatureObject (m_target.getPointer ()));
	ShipObject* const shipObject         = const_cast<ShipObject *>(dynamic_cast<const ShipObject*>(m_target.getPointer ()));
	PlayerCreatureController* const playerCreatureController = dynamic_cast<PlayerCreatureController*> (creatureObject ? creatureObject->getController () : 0);
	const bool isTurnStrafe = CuiPreferences::isTurnStrafe ();

	float  turnRate           = 0.0f;
	float  currentSpeed       = 0.0f;
	
	if (playerCreatureController)
	{
		currentSpeed        = playerCreatureController->getCurrentSpeed ();
		turnRate = creatureObject->getMaximumTurnRate (currentSpeed);
	}

	const float maximumYawThisFrame = convertDegreesToRadians (turnRate) * elapsedTime;

	float turnTotal = 0.0f;

	bool shouldLock = false;
	float yawMod = 0.f;
	float pitchMod = 0.f;
	
	int i;
	for (i = 0; i < m_queue->getNumberOfMessages (); i++)
	{
		int   message = 0;
		float value = 0.0f;

		m_queue->getMessage (i, &message, &value);

		switch (message)
		{
		case CM_spinCamera:
			m_spinning = !m_spinning;
			break;

		case CM_chaseCamera:
			setCameraMode ( useModeless || ( ms_cameraMode == CM_free ) ? CM_chase : CM_free);
			m_spinning = false;
			break;

		case CM_cameraYawMouse:   
			yawMod += value; 
			m_spinning = false;
			break;

		case CM_cameraPitchMouse: 
			pitchMod += value;                
			m_spinning = false;
			break;

		case CM_cameraYaw:        
			yawMod += value * elapsedTime;
			m_spinning = false;
			break;

		case CM_cameraPitch:      
			pitchMod += value * elapsedTime;  
			m_spinning = false;
			break;
			
		case CM_turn:
			if (!isTurnStrafe)
				turnTotal += value;
			
			m_spinning = false;
			break;

		case CM_cameraZoom:
		case CM_mouseWheel:
			{
				if (value > 0)
					ms_currentSetting = std::max (ms_currentSetting - 1, 0);
				else
					ms_currentSetting = std::min (ms_currentSetting + 1, m_numberOfSettings - 1);
				
				if (m_settings)
					m_zoom = m_settings [ms_currentSetting];
			}
			break;
	
		case CM_cameraLock:
			{
				if (ms_cameraMode == CM_chase)
					shouldLock = true;

				m_spinning = false;
			}
			break;

		case CM_cameraReset:
			m_spinning = false;
			m_offsetYaw_w = 0.f;
			break;

		default:
			break;
		}
	}

	if (m_spinning)
		yawMod += ms_spinYawPerSecond * elapsedTime;

	const CuiIoWin::MouseLookState mouseLookState = CuiIoWin::getMouseLookState();

	if ( useModeless )
	{
		if ( mouseLookState == CuiIoWin::MouseLookState_Camera )
		{
			shouldLock = true;
		}
		else if ( mouseLookState == CuiIoWin::MouseLookState_Move || mouseLookState == CuiIoWin::MouseLookState_Avatar )
		{
			m_yaw_w += m_offsetYaw_w;
			m_offsetYaw_w = 0;
		}
		else
		{
			// slowly orbit the camera back to behind the avatar,
			// but only when he's moving
			float speed = playerCreatureController->getCurrentSpeed();

			if ( speed > 0.f )
			{
				m_offsetYaw_w *= 1.0f - std::min( elapsedTime * cs_cameraSnapBackSpeedModifier, 1.0f );
			}
		}
	}

	if (shouldLock)
	{
		m_offsetYaw_w += yawMod;
	}
	else
	{
		m_yaw_w += yawMod;
	}

	if ( m_offsetYaw_w > PI )
	{
		m_offsetYaw_w = m_offsetYaw_w - 2.0f * PI;
	}
	else if ( m_offsetYaw_w < -PI )
	{
		m_offsetYaw_w = m_offsetYaw_w + 2.0f * PI;
	}

	ms_pitch += pitchMod;	
	
	turnTotal = std::max (-maximumYawThisFrame, std::min (maximumYawThisFrame, turnTotal));

	if (turnTotal > 0.0f && m_lastTurnRate >= 0.0f && turnTotal > m_lastTurnRate)
	{
		turnTotal = linearInterpolate (m_lastTurnRate, turnTotal, elapsedTime);
	}
	else if (turnTotal < 0.0f && m_lastTurnRate <= 0.0f && turnTotal < m_lastTurnRate)
	{
		turnTotal = linearInterpolate (m_lastTurnRate, turnTotal, elapsedTime);
	}
	
	if ( playerCreatureController )
	{
		if ( !m_firstPerson && ms_cameraMode != CM_chase ) // <- this will always be false in modeless because ms_cameraMode is never CM_chase
		{
			turnTotal *= std::min (1.0f, currentSpeed);
		}

		m_yaw_w += turnTotal;

		if ( mouseLookState == CuiIoWin::MouseLookState_Camera )
		{
			m_offsetYaw_w -= turnTotal;
		}

		playerCreatureController->setDesiredYaw_w(
			m_yaw_w,
			ms_cameraMode == CM_chase && mouseLookState != CuiIoWin::MouseLookState_Camera  );

	}
	
	m_lastTurnRate = turnTotal;

	//-- clamp values to the acceptable range
	m_zoom = m_settings [ms_currentSetting];
	ms_pitch = clamp (m_firstPerson ? -MAX_PITCH_FABS : -1.f, ms_pitch, MAX_PITCH_FABS);
	if (m_settings)
	{
		if (isFirstPerson ())
			m_zoom = clamp (0.f, m_zoom, m_settings [m_numberOfSettings - 1]);
		else
			m_zoom = clamp (ms_pitch > 0.f ? 0.f : -ms_pitch, m_zoom, m_settings [m_numberOfSettings - 1]);
	}

	bool haveMount = creatureObject ? creatureObject->getState (States::RidingMount) : false;

	if ((haveMount || shipObject) && m_numberOfSettings > 1)
	{
		const float settingPercent = static_cast<float>(ms_currentSetting) / (m_numberOfSettings - 1);
		float zoomMultiplier = 1.0f + settingPercent * m_zoomMultiplier;
		m_zoom *= zoomMultiplier;
	}

	m_currentZoom = linearInterpolate (m_currentZoom, m_zoom, ms_cameraZoomSpeed);

	//-- move camera to the avatar's cell and position
	{
		const CellProperty* const targetCell = m_target->getParentCell ();
		if (getParentCell () != targetCell)
			setParentCell (const_cast<CellProperty*> (targetCell));

		//-- move camera up to the offset position
		CellProperty::setPortalTransitionsEnabled (false);

		const Object * targetObject = NULL;
		if(shipObject)
			targetObject = shipObject;
		else
			targetObject = creatureObject;

		if (creatureObject && (creatureObject->getState (States::RidingMount)))
		{
			const CreatureObject * const mountCreature = creatureObject->getMountedCreature ();
			if (mountCreature && !(mountCreature->getGameObjectType () == SharedObjectTemplate::GOT_vehicle_hover))
				targetObject = mountCreature;
		}

		setTransform_o2p (targetObject->getTransform_o2c ());

		CellProperty::setPortalTransitionsEnabled (true);

		//-- reset rotation (we're now in cell space)
		resetRotate_o2p ();

		//-- reverse yaw to get world space transform
		if (!Game::isSpace())
			yaw_o (-getObjectFrameK_w ().theta ());

		//-- yaw the world yaw amount
		yaw_o (m_yaw_w + (ms_cameraMode == CM_chase ? m_offsetYaw_w : 0.f));

		m_offset = Vector::linearInterpolate(m_offset, m_desiredOffset, elapsedTime);

		move_o(Vector(m_offset.x, m_offset.y + ms_vehicleCameraOffsetY, m_offset.z));

#ifdef _DEBUG
		if (ms_renderPivotFrame)
			addDebugPrimitive (new FrameDebugPrimitive (FrameDebugPrimitive::S_none,  getTransform_o2w (), 0.25f));
#endif

		//-- pitch the camera
		pitch_o (ms_pitch);
	}

	//-- don't show the m_target if zoom distance is less than first person distance
	const float scaledFirstPersonDistance = m_firstPersonDistance * (creatureObject ? creatureObject->getScaleFactor () : 1.f);
	m_colliding = false;
	
	// prevent shoulder cam from clipping geometry (due to near plane)
	if (creatureObject && m_currentZoom >= scaledFirstPersonDistance)
	{
		CollisionInfo result;
		Vector startPos_w = creatureObject->getPosition_w();
		startPos_w.y += m_desiredOffset.y;
		Vector endPos_w = startPos_w + (creatureObject->rotate_o2w(Vector::unitX) * (m_desiredOffset.x + 0.25f));
		if(ClientWorld::collide(getParentCell(),startPos_w,endPos_w,CollideParameters::cms_default,result,ClientWorld::CF_allCamera))
		{
			move_o(Vector(-m_offset.x * elapsedTime, 0.0f, 0.0f));
			m_offset.x = 0.0f;
		}
	}
			
	
	if (m_currentZoom < scaledFirstPersonDistance)
	{
		setFirstPerson (true);
	}
	else
	{
		setFirstPerson (false);

		if (ms_cameraSimpleCollision)
		{
			const Vector cameraZ_p = Vector::negativeUnitZ * m_zoom;
			const Vector start_w   = getPosition_w ();
			const Vector end_w     = start_w + rotate_o2w (cameraZ_p);

			CollisionInfo result;
			if (ClientWorld::collide (getParentCell (), start_w, end_w, CollideParameters::cms_default, result, ClientWorld::CF_allCamera))
			{
				const float lineDistance = start_w.magnitudeBetween (end_w);
				const float t = clamp (0.f, (start_w.magnitudeBetween (result.getPoint ()) / lineDistance) - (0.25f / lineDistance), 1.f);
				m_currentZoom = Vector::linearInterpolate (start_w, end_w, t).magnitudeBetween (start_w);
				m_colliding = true;
			}
		}
		else
		{
			//-- we're going to fire 4 rays in a diamond pattern to see what gets collided with
			const Vector offsets [4] =
			{
				Vector::unitX * 0.25f,
				Vector::negativeUnitX * 0.25f,
				Vector::unitY * 0.25f,
				Vector::negativeUnitY * 0.25f
			};

			const Vector cameraZ_p = Vector::negativeUnitZ * m_zoom;
			const Vector start_w   = getPosition_w ();
			const Vector end_w     = start_w + rotate_o2w (cameraZ_p);

			bool hit = false;
			CollisionInfo result;
			float minimumT = 1.f;

			int i;
			for (i = 0; i < 4; ++i)
			{
				const Vector interimStart_w = start_w + rotate_o2w (offsets [i]);
				const Vector interimEnd_w = end_w + rotate_o2w (offsets [i]);

				CollisionInfo interimResult;
				if (ClientWorld::collide (getParentCell (), interimStart_w, interimEnd_w, CollideParameters::cms_default, interimResult, ClientWorld::CF_allCamera))
				{
					const float t = clamp (0.f, interimStart_w.magnitudeBetween (interimResult.getPoint ()) / interimStart_w.magnitudeBetween (interimEnd_w), 1.f);

					if (t < minimumT)
					{
						hit = true;
						minimumT = t;
						result = interimResult;
						m_colliding = true;
					}
				}
			}

			//SPACEHACK don't pull camera in when flying in a ship
			if(!shipObject)
			{
				if (hit)
				{
					// Back the camera off the collision point so its near plane
					// doesn't straddle the surface. The simple-collision path
					// above does the same; the diamond path used to place the
					// camera flush, which let you see through whatever it hit.
					float const hitDistance = Vector::linearInterpolate(start_w, end_w, minimumT).magnitudeBetween(start_w);
					m_currentZoom = std::max(0.0f, hitDistance - cs_cameraCollisionBackoff);
				}
			}
		}

		//-- GROUND FLOOR: never let the chase camera descend below the ground /
		//   footing. When the view is pitched up from behind, the camera swings
		//   down-and-back; if the world camera-collision misses it (or only pulls
		//   it flush with the terrain) the camera ends up under the world and
		//   renders the backfaces of distant meshes from below. Pull the camera
		//   in along its own view ray so it lands no lower than a small clearance
		//   above the ground. Reducing the zoom (rather than just clamping Y) keeps
		//   the avatar framed and degrades gracefully into first person up close.
		if (creatureObject && !shipObject)
		{
			const Vector pivot_w = getPosition_w();						// pivot at camera height (pre-pullback)
			const Vector viewDir_w = rotate_o2w(Vector::negativeUnitZ); // unit dir the camera pulls back along
			if (viewDir_w.y < -0.01f)									// only when the camera would move downward
			{
				float floorY = creatureObject->getPosition_w().y; // the avatar's footing (origin is at the feet)
				if (getParentCell() == CellProperty::getWorldCellProperty())
				{
					const TerrainObject *const terrain = TerrainObject::getConstInstance();
					float terrainHeight = 0.0f;
					if (terrain && terrain->getHeight(pivot_w, terrainHeight))
						floorY = std::max(floorY, terrainHeight);
				}
				floorY += cs_cameraGroundClearance;

				if (pivot_w.y > floorY)
				{
					const float maxZoomToFloor = (floorY - pivot_w.y) / viewDir_w.y; // both negative -> positive distance
					if (m_currentZoom > maxZoomToFloor)
						m_currentZoom = maxZoomToFloor;
				}
			}
		}

		if (m_currentZoom < scaledFirstPersonDistance)
			setFirstPerson (true);
		else
			move_o (Vector::negativeUnitZ * m_currentZoom);
	}

	//-- CONSULT-65 round 2 (2026-07-06): derive the camera's CELL from its own FINAL
	//   position instead of leaving it force-copied from the target (the copy at the
	//   top of this function). The chase camera lags meters behind the player, so
	//   running through a doorway leaves a window where the camera is TAGGED in the
	//   player's new cell while its eye is still in the old one -- dPVS then roots
	//   visibility at the wrong cell for the eye (the doorway portal backface-culls)
	//   and whole cells, including the avatar, vanish until the camera physically
	//   catches up. Signature: stable while standing still, cuts in/out on movement
	//   through portals; the camera collision ray sees no obstruction (open doorway)
	//   so no pull-in fires. Walk the short player->camera segment through the portal
	//   graph -- the same loop CellProperty::Notification::positionChanged uses -- so
	//   the player's authoritative cell anchors the derivation and the camera's
	//   per-frame teleports cannot desync it. The camera is never MOVED by this
	//   block, only re-tagged -- it does not change camera feel.
	if (m_target)
	{
		CellProperty * currentCell = const_cast<CellProperty *>(m_target->getParentCell ());
		if (currentCell)
		{
			Vector start = m_target->getPosition_w ();
			Vector end = getPosition_w ();

			//-- lift the segment slightly so doorway portals sitting on the terrain
			//   are not missed (same trick as the portal-crossing notification)
			start.y += 0.1f;
			end.y += 0.1f;

			CellProperty * lastCell = NULL;
			CellProperty * derivedCell = currentCell;

			while (currentCell)
			{
				Vector const cellStart = currentCell->getOwner ().rotateTranslate_w2o (start);
				Vector const cellEnd = currentCell->getOwner ().rotateTranslate_w2o (end);

				float time = 0.0f;
				CellProperty * const nextCell = currentCell->getDestinationCell (cellStart, cellEnd, time);

				if (time == 1.0f || !nextCell || nextCell == currentCell || nextCell == lastCell)
					break;

				//-- CONSULT-65 fix 5: knife-edge hysteresis. If the portal plane sits
				//   within 20cm of the camera eye (the crossing happens essentially AT
				//   the segment end), do NOT take the crossing -- keep the near-side
				//   (player-side) cell. Re-tagging exactly at the plane made the
				//   boundary portal backface-cull from the far root (probe signature
				//   tested:1/bf:1) and the whole neighbor cell vanished. Stateless, so
				//   there is no dangling-cell bookkeeping across zone changes.
				Vector const crossing_w = Vector::linearInterpolate (start, end, time);
				if (crossing_w.magnitudeBetween (end) < 0.20f)
					break;

				lastCell = currentCell;
				currentCell = nextCell;
				derivedCell = currentCell;
				start = crossing_w;
			}

			if (derivedCell != getParentCell ())
			{
				//-- preserve the camera's world pose across the cell re-tag
				Transform const cameraToWorld (getTransform_o2w ());
				CellProperty::setPortalTransitionsEnabled (false);
				setParentCell (derivedCell);
				setTransform_o2w (cameraToWorld);
				CellProperty::setPortalTransitionsEnabled (true);
			}
		}
	}

	//-- handle the mode callback
	if (m_modeCallback)
	{
		if (m_currentMode != isFirstPerson ())
		{
			m_modeCallback (m_context);

			m_modeCallback = 0;
			m_context = 0;
		}
	}

#ifdef _DEBUG
	DEBUG_REPORT_PRINT (ms_debugReport, ("firstPerson = %s\n", isFirstPerson () ? "yes" : "no"));
	DEBUG_REPORT_PRINT (ms_debugReport, ("mode        = %s\n", ms_cameraMode ? "chase" : "free"));
	DEBUG_REPORT_PRINT (ms_debugReport, ("yaw         = %1.2f\n", m_yaw_w));
	DEBUG_REPORT_PRINT (ms_debugReport, ("offsetYaw   = %1.2f\n", m_offsetYaw_w));
	DEBUG_REPORT_PRINT (ms_debugReport, ("pitch       = %1.2f\n", ms_pitch));
	DEBUG_REPORT_PRINT (ms_debugReport, ("zoom        = %1.2f\n", m_currentZoom));
	DEBUG_REPORT_PRINT (ms_debugReport, ("desiredZoom = %1.2f\n", m_zoom));
	DEBUG_REPORT_PRINT (ms_debugReport, ("near        = %1.2f\n", getNearPlane ()));
	DEBUG_REPORT_PRINT (ms_debugReport, ("far         = %1.2f\n", getFarPlane ()));
#endif

	IGNORE_RETURN (GameCamera::alter (elapsedTime));
	return AlterResult::cms_alterNextFrame;
}

//-------------------------------------------------------------------

void FreeChaseCamera::setMessageQueue (const MessageQueue* const queue)
{
	m_queue = queue;
}

//-------------------------------------------------------------------

void FreeChaseCamera::setTarget (const Object* const target, bool force, bool overwriteYaw)
{
	if (target != m_target.getPointer() || force)
	{
		m_target         = target;
		m_radius         = m_target && m_target->getAppearance () ? m_target->getAppearanceSphereRadius () : 0.f;
 		
		// allow yaw forcing to be turned off while forcing the rest
		// This is needed to allow the camera to remain in place when
		// exiting a vehicle or mount
 		if (target != m_target.getPointer () || overwriteYaw)
 		{
			m_yaw_w      = m_target ? m_target->getObjectFrameK_w ().theta () : 0.f;
		}
		
		m_currentPosture = Postures::Invalid;
		m_currentStates  = 0;
		m_desiredOffset  = m_offsetDefault;
		m_offset         = m_offsetDefault;
	}
}

//-------------------------------------------------------------------

const Object* FreeChaseCamera::getTarget () const
{
	return m_target;
}

//-------------------------------------------------------------------

const Vector& FreeChaseCamera::getOffset () const
{
	return m_offset;
}

//-------------------------------------------------------------------

float FreeChaseCamera::getPitch () const
{
	return ms_pitch;
}

//-------------------------------------------------------------------

float FreeChaseCamera::getYaw () const
{
	return m_yaw_w;
}

//-------------------------------------------------------------------

float FreeChaseCamera::getRadius () const
{
	return m_radius;
}

//-------------------------------------------------------------------

float FreeChaseCamera::getZoom () const
{
	return m_zoom;
}

//----------------------------------------------------------------------

bool FreeChaseCamera::isFirstPerson () const
{
	return m_firstPerson;
}

//----------------------------------------------------------------------

void FreeChaseCamera::setModeCallback (ModeCallback modeCallback, void* const context)
{
	m_modeCallback = modeCallback;
	m_context = context;
	m_currentMode = isFirstPerson ();
}

//----------------------------------------------------------------------

void FreeChaseCamera::setCameraMode (const CameraMode mode)
{
	ms_cameraMode = mode;
	if (ms_cameraMode)
		CuiSystemMessageManager::sendFakeSystemMessage (CuiStringIds::chase_camera_on.localize ());
	else
		CuiSystemMessageManager::sendFakeSystemMessage (CuiStringIds::chase_camera_off.localize ());
}

//----------------------------------------------------------------------

FreeChaseCamera::CameraMode  FreeChaseCamera::getCameraMode ()
{
	return static_cast<CameraMode>(ms_cameraMode);
}

//----------------------------------------------------------------------

void FreeChaseCamera::setCameraZoomSpeed (float f)
{
	ms_cameraZoomSpeed = f;
}

//----------------------------------------------------------------------

float FreeChaseCamera::getCameraZoomSpeed ()
{
	return ms_cameraZoomSpeed;
}

//----------------------------------------------------------------------

void FreeChaseCamera::setCameraSimpleCollision (bool const cameraSimpleCollision)
{
	ms_cameraSimpleCollision = cameraSimpleCollision;
}

//----------------------------------------------------------------------

bool FreeChaseCamera::getCameraSimpleCollision ()
{
	return ms_cameraSimpleCollision;
}

//----------------------------------------------------------------------

bool FreeChaseCamera::getCameraSimpleCollisionDefault ()
{
	return false;
}

//----------------------------------------------------------------------

void FreeChaseCamera::setFirstPerson (bool const firstPerson)
{
	if (firstPerson)
	{
		m_firstPerson = true;
		if (!ConfigClientGame::getShowAttachmentsInFirstPerson())
		{
			// DBE - this is necessary because this routine is called every frame by
			// alter. In first person mode the target will be inserted into the exclusion list
			// repeatedly.  
			// TODO: The handling of the exclusion list is not clean.  
			// There is no remove call and no way to reject duplicates.  Either the exclusion
			// list needs to be cleared every frame and the list re-built, or each object
			// needs to be responsible for adding and removing entries.
			clearExcludedObjects(); 

			addExcludedObject (m_target);

			//-- This is to make jetpacks both untargetable and invisible when in first person.  
			//   There is a special case for jetpacks in that cannot be dismounted, they can only
			//   be stored and are immediately placed back into the player's inventory.
			ClientObject const * const targetAsClientPointer = m_target.getPointer()->asClientObject();
			CreatureObject const * const targetAsCreaturePointer = targetAsClientPointer ? targetAsClientPointer->asCreatureObject() : 0;			
			if (targetAsCreaturePointer)
			{
				CreatureObject const * const mount = targetAsCreaturePointer->getMountedCreature();
				if (mount && shouldBeInvisibleInFirstPerson(mount))
				{
					addExcludedObject(mount);
					mount->setUntargettableOverride(true);
				}
			}
		}
		else if (m_target)
		{
			//-- shut off the player mesh but continue to draw attachments (i.e. weapons)
			SkeletalAppearance2 *const appearance = const_cast<SkeletalAppearance2 *> (m_target->getAppearance () ? m_target->getAppearance ()->asSkeletalAppearance2 () : 0);
			if (appearance)
				appearance->setShowMesh (false);			
		}		
	}
	else
	{
		m_firstPerson = false;

		if (!ConfigClientGame::getShowAttachmentsInFirstPerson())
		{
			clearExcludedObjects();
			ClientObject const * const targetAsClientPointer = m_target.getPointer()->asClientObject();
			CreatureObject const * const targetAsCreaturePointer = targetAsClientPointer ? targetAsClientPointer->asCreatureObject() : 0;			
			if (targetAsCreaturePointer)
			{
				CreatureObject const * const mount = targetAsCreaturePointer->getMountedCreature();
				if (mount && shouldBeInvisibleInFirstPerson(mount))
				{
					mount->setUntargettableOverride(false);
				}
			}
		}
		else if (m_target)
		{
			ClientObject const * const clientTarget = m_target->asClientObject ();
			ClientObject const * const containedByObject = clientTarget != NULL ? clientTarget->getContainedBy() : NULL;
			if (!containedByObject || containedByObject->asShipObject() == NULL)
			{
				//-- ensure that the player mesh is rendered in addition to attachments.
				SkeletalAppearance2 *const appearance = const_cast<SkeletalAppearance2 *> (m_target->getAppearance () ? m_target->getAppearance ()->asSkeletalAppearance2 () : 0);
				if (appearance)
					appearance->setShowMesh (true);
			}
		}
	}
}

//----------------------------------------------------------------------

void FreeChaseCamera::setZoomMultiplier(float multiplier)
{
	if(multiplier < 0.0f)
		return;
	m_zoomMultiplier = multiplier;
}

//----------------------------------------------------------------------
// these two accessor methods are for the "offset camera" checkbox in the controls ui
void FreeChaseCamera::setOffsetCamera(bool b)
{
	ms_useCameraOffset = b;
	ms_cameraOffsetChanged = true;
	CuiPreferences::setOffsetCamera(ms_useCameraOffset);
}

//----------------------------------------------------------------------

bool FreeChaseCamera::getOffsetCamera()
{
	return CuiPreferences::getOffsetCamera();
}

void FreeChaseCamera::setCameraHeight(float f)
{
	CuiPreferences::setPlayerCameraHeight(f);
	ms_cameraOffsetChanged = true;
}

float FreeChaseCamera::getCameraHeight()
{
	return CuiPreferences::getPlayerCameraHeight();
}

float FreeChaseCamera::getCameraHeightDefault()
{
	return ms_defaultCameraHeight;
}



//===================================================================
