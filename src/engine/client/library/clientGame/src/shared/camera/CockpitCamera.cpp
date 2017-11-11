// ======================================================================
//
// CockpitCamera.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/CockpitCamera.h"

#include "clientGame/ClientHeadTracking.h"
#include "clientGame/ClientShipTargeting.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/DustAppearance.h"
#include "clientGame/ShipObject.h"
#include "clientGraphics/RenderWorld.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiPreferences.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/MessageQueue.h"
#include "sharedGame/SharedShipObjectTemplate.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/ContainedByProperty.h"
#include "sharedTerrain/SpaceTerrainAppearanceTemplate.h"
#include "sharedTerrain/TerrainObject.h"
#include "sharedUtility/CurrentUserOptionManager.h"
#include "sharedDebug/DebugFlags.h"

// ======================================================================

namespace CockpitCameraNamespace
{
	Tag const TAG_CPIT = TAG(C,P,I,T);
	Tag const TAG_FRAM = TAG(F,R,A,M);
	Tag const TAG_FRST = TAG(F,R,S,T);
	Tag const TAG_ZOOM = TAG(Z,O,O,M);
	Tag const TAG_3OFF = TAG(3,O,F,F);
	Tag const TAG_1OFF = TAG(1,O,F,F);
	Tag const TAG_ISPB = TAG(I,S,P,B);
	Tag const TAG_HYPR = TAG(H,Y,P,R);

	float const c_firstPersonMaximumYaw = PI;
	float const c_firstPersonMaximumPitch = PI_OVER_2 - PI / 64.0f;

	ConstCharCrcString const c_cameraHardpointName ("camera");

	// ======================================================================

	float const c_minElasticity = 0.01f;

	Vector m_3rdPersonCameraOffset;
	Vector m_1stPersonCameraOffset;
	Vector m_hyperspaceCameraOffset;
	float const s_cameraRecenterTime = 0.3f;
	float const s_cameraRecenterSpeed = 5.0f;

	//-- parameters controlling the pov hat pan speed
	float const s_povHatPanSpeedFactorStart = 0.5f;
	float const s_povHatPanSpeedFactorEnd = 6.0f;

	bool ms_showCockpit = true;

	bool ms_debugFlagCockpitZoom = false;
		
	//----------------------------------------------------------------------
	
	/**
	* angle/angleTarget must be within [-PI,PI]
	*/
	float calculateMinimumDeltaAngle(float angle, float angleTarget)
	{
		if (angle < -PI_OVER_2)
		{
			if (angleTarget > PI_OVER_2)
			{
				return -((PI_TIMES_2 - angleTarget) + angle);
			}
		}
		else if (angle > PI_OVER_2)
		{
			if (angleTarget < -PI_OVER_2)
			{
				return PI_TIMES_2 + (angleTarget - angle);
			}
		}
		
		return angleTarget - angle;
	}
	
	//----------------------------------------------------------------------
	
	float clampAngleToRangePiNegPi(float angle)
	{
		if (angle < -PI)
		{
			return PI_TIMES_2 + angle;
		}
		else if (angle > PI)
		{
			return -(PI_TIMES_2 - angle);
		}
		
		return angle;
	}

	float const ELASTICITY_MAX_DISTANCE = 50.0f;
	float const ELASTICITY_OVERRIDE_FACTOR = 0.5f;
	
}

using namespace CockpitCameraNamespace;

// ======================================================================

class CockpitCamera::CameraDampener
{
public:
	CameraDampener();

	void setDampen(bool const dampen);
	Transform update(Transform const & transform, float const elapsedTime);
	Transform getCurrentTransform() const;
	float getTimeWindow() const;

private:
	Transform m_currentTransform;
	float m_timeWindow;
	bool m_dampen;
	Vector m_lastUpdatePosition;
	float m_currentSpeed;
};

// ======================================================================

CockpitCamera::CameraDampener::CameraDampener() :
	m_currentTransform(),
	m_timeWindow(CuiPreferences::getSpaceCameraElasticity()),
	m_dampen(false),
	m_lastUpdatePosition(Vector::zero),
	m_currentSpeed(0.f)
{
}

// ----------------------------------------------------------------------

void CockpitCamera::CameraDampener::setDampen(bool const dampen)
{
	m_dampen = dampen;
}

// ----------------------------------------------------------------------

Transform CockpitCamera::CameraDampener::update(Transform const & transform, float const elapsedTime)
{
	Vector const newPosition(transform.getPosition_p());
	float newSpeed = (newPosition-m_lastUpdatePosition).magnitude()/elapsedTime;

	// check if the preference slider has changed.  If it has then ease into the new value
	float const desiredTimeWindow = CuiPreferences::getSpaceCameraElasticity();
	float const desiredMaxDistance = ELASTICITY_MAX_DISTANCE * desiredTimeWindow;
	if (desiredTimeWindow < m_timeWindow)
	{
		m_timeWindow = clamp(desiredTimeWindow, m_timeWindow - elapsedTime, m_timeWindow);
	}
	else if (desiredTimeWindow > m_timeWindow)
	{
		m_timeWindow = clamp(m_timeWindow, m_timeWindow + elapsedTime, desiredTimeWindow);
	}

	if (!m_dampen || m_timeWindow < c_minElasticity)
	{
		m_currentTransform = transform;
		m_currentSpeed = newSpeed;
	}
	else
	{
		float const timeTranslation = clamp(0.f, elapsedTime / m_timeWindow, 1.f);
		float const timeRotation = clamp(0.f, timeTranslation * 2.0f, 1.f);

		m_currentSpeed = linearInterpolate(m_currentSpeed, newSpeed, timeTranslation);

		Vector const previousPosition(m_currentTransform.getPosition_p());

		Vector offsetDirection(previousPosition-newPosition);

		if (offsetDirection.normalize())
			m_currentTransform.setPosition_p(newPosition + offsetDirection * m_currentSpeed * 0.2f);
		else
			m_currentTransform.setPosition_p(newPosition);
		
		Vector difference(newPosition - m_currentTransform.getPosition_p());
		float const overrun = difference.magnitudeSquared() - (desiredMaxDistance * desiredMaxDistance);
		if(overrun > 0)
		{
			difference.normalize();
			m_currentTransform.setPosition_p(m_currentTransform.getPosition_p() + difference * sqrt(overrun) * ELASTICITY_OVERRIDE_FACTOR);
		}

		Vector const newJ(transform.getLocalFrameJ_p());
		Vector const previousJ(m_currentTransform.getLocalFrameJ_p());

		Vector const newK(transform.getLocalFrameK_p());
		Vector const previousK(m_currentTransform.getLocalFrameK_p());

		Vector finalJ(Vector::linearInterpolate(previousJ, newJ, timeRotation));
		Vector finalK(Vector::linearInterpolate(previousK, newK, timeRotation));

		if (finalJ.normalize() && finalK.normalize())
		{
			m_currentTransform.setLocalFrameKJ_p(finalK, finalJ);
			m_currentTransform.reorthonormalize();
		}
	}

	m_lastUpdatePosition = newPosition;

	return m_currentTransform;
}

// ----------------------------------------------------------------------

Transform CockpitCamera::CameraDampener::getCurrentTransform() const
{
	return(m_currentTransform);
}

// ----------------------------------------------------------------------

float CockpitCamera::CameraDampener::getTimeWindow() const
{
	return(m_timeWindow);
}

// ======================================================================

void CockpitCamera::install()
{
	InstallTimer const installTimer("CockpitCamera::install");

	CurrentUserOptionManager::registerOption(ms_showCockpit, "ClientGame/CockpitCamera", "showCockpit");

	DebugFlags::registerFlag(ms_debugFlagCockpitZoom, "ClientGame/CockpitCamera", "debugFlagCockpitZoom");
}

// ----------------------------------------------------------------------

void CockpitCamera::setShowCockpit(bool const showCockpit)
{
	ms_showCockpit = showCockpit;
}

// ----------------------------------------------------------------------

bool CockpitCamera::getShowCockpit()
{
	return ms_showCockpit;
}

// ======================================================================

CockpitCamera::CockpitCamera() :
	GameCamera(),
	m_queue(0),
	m_target(0),
	m_shipTarget(0),
	m_yaw(0.f),
	m_pitch(0.f),
	m_locked(false),
	m_dustObject(0),
	m_frameAppearanceTemplate(0),
	m_cameraOffset_p(),
	m_hyperspaceOffset_p(),
	m_zoomSettingList(new ZoomSettingList),
	m_zoomSetting(0),
	m_zoom(0.f),
	m_currentZoom(0.f),
	m_firstPersonDistance(0.f),
	m_cameraDampener(new CameraDampener()),
	m_timerUntilCameraRecenter(s_cameraRecenterTime),
	m_useTimerForCameraRecenter(false),
	m_yawTarget(0.0f),
	m_pitchTarget(0.0f),
	m_povHatActive(false),
	m_povHatPanLastSpeedFactor(0.0f),
	m_cameraLockTarget(false),
	m_rearView(false),
	m_wasShowCockpitBeforeRearView(false),
	m_isInHyperspace(false)
{
	m_zoomSettingList->push_back(0.f);
}

// ----------------------------------------------------------------------

CockpitCamera::~CockpitCamera()
{
	m_queue = 0;

	delete m_zoomSettingList;

	m_dustObject = 0;

	_setFrameAppearanceTemplate(0);

	delete m_cameraDampener;
}

// ----------------------------------------------------------------------

void CockpitCamera::setActive(bool const active)
{
	//-- If we're already active, we want to change our cockpit view state
	if (isActive() && active)
		ms_showCockpit = !ms_showCockpit;

	GameCamera::setActive(active);
	ClientShipTargeting::activateVariableReticle(active);

	//-- Extract cockpitFilename from SharedShipObjectTemplate
	if (m_target && isActive())
	{
		ClientObject const * const clientObject = m_target->asClientObject();
		if (clientObject)
		{
			CreatureObject const * const creatureObject = clientObject->asCreatureObject();
			if (creatureObject)
			{
				ShipObject const * const shipObject = ShipObject::getContainingShip(*creatureObject);

				if (shipObject)
				{
					m_shipTarget = shipObject;
					SharedShipObjectTemplate const * const sharedShipObjectTemplate = safe_cast<SharedShipObjectTemplate const *>(shipObject->getObjectTemplate());
					if (sharedShipObjectTemplate)
					{
						char const * const cockpitFileName = sharedShipObjectTemplate->getCockpitFilename().c_str();
						if (cockpitFileName && *cockpitFileName)
							load(cockpitFileName);

						if (shipObject->getCockpitFrame() == 0)
						{
							Object * newFrameObject = NULL;
							if (createFrame(newFrameObject, &m_cameraOffset_p))
							{
								const_cast<ShipObject *>(shipObject)->setCockpitFrame(newFrameObject);
							}

							else
							{
								// no cockpit so must be a POB ship
								m_hyperspaceOffset_p.setPosition_p(m_hyperspaceCameraOffset);
							}
						}
					}
				}
			}
		}
	}

	if (CuiPreferences::getActualUseModelessInterface()
		|| CuiManager::getPointerInputActive())
	{
		CuiManager::setPointerToggledOn(false);
	}
}

// ----------------------------------------------------------------------

bool CockpitCamera::calculatePilotCameraLocation(bool tryForFirstPerson, CellProperty * & targetCellProperty_p, Transform & targetTransform_p, Transform & targetTransform_w)
{
	bool firstPerson = false;

	targetCellProperty_p = m_shipTarget->getParentCell();
	Transform const shipTransform_p(m_shipTarget->getTransform_o2c());
	targetTransform_p.multiply(shipTransform_p, m_cameraOffset_p);
	Transform const saveShipTransform_w(targetTransform_p);

	targetTransform_w = targetTransform_p;

	//-- If in first person, check to see if the target creature is in a pob ship
	ShipObject const * const shipObject = dynamic_cast<ShipObject const *>(m_shipTarget.getPointer());

	if (tryForFirstPerson)
	{
		if (shipObject && m_target)
		{
			ClientObject const * const clientObject = m_target->asClientObject();
			if (clientObject)
			{
				CreatureObject const * const creatureObject = clientObject->asCreatureObject();
				if (creatureObject)
				{
					ContainedByProperty const * const containedByProperty = creatureObject->getContainedByProperty();
					if (containedByProperty)
					{
						Object const * const containedByObject = containedByProperty->getContainedBy();
						if (containedByObject && containedByObject->asClientObject() && !containedByObject->asClientObject()->asShipObject())
						{
							//-- The target creature is in a pob, so our target is the player
							targetCellProperty_p = creatureObject->getParentCell();
							targetTransform_p = creatureObject->getTransform_o2c();
							targetTransform_p.move_l(Vector::unitY * 1.2f);
							targetTransform_w.multiply(shipObject->getTransform_o2w(), targetTransform_p);
						}
					}
					firstPerson = true;
				}
			}
		}
	}

	bool const isPOB = ((shipObject != 0) && (shipObject->getPortalProperty() != 0));

	if (firstPerson)
	{
		// adjust the camera based off of the offset when in 1st person
		targetTransform_p.move_l(m_1stPersonCameraOffset);
		Vector const offset_w(targetTransform_w.rotate_l2p(targetTransform_p.rotate_p2l(m_1stPersonCameraOffset)));
		targetTransform_w.move_l(offset_w);

		if (!isPOB)
		{
			m_hyperspaceOffset_p = targetTransform_p;
		}
	}

	else
	{
		if (!isPOB)
		{
			// adjust the camera based off of the offset when in 1st person
			m_hyperspaceOffset_p = targetTransform_p;
			m_hyperspaceOffset_p.move_l(m_1stPersonCameraOffset);
		}
		else
		{
			// adjust the camera based off of the offset when in 3rd person
			Vector offset_p(0.0f, 0.0f, -m_currentZoom);
			Vector offset_w(targetTransform_w.rotate_l2p(targetTransform_p.rotate_p2l(offset_p)));
			targetTransform_w.move_l(offset_w);
		}		
	}

	return firstPerson;
}

// ----------------------------------------------------------------------

float CockpitCamera::alter(float const elapsedTime)
{
	if (!isActive())
	{
		setDustEnabled(false);
		return AlterResult::cms_alterNextFrame;
	}

	if (!m_shipTarget)
	{
		setDustEnabled(false);
		return AlterResult::cms_alterNextFrame;
	}

	setDustEnabled(!m_isInHyperspace);

	//-- interpolate the camera to its targets
	if (m_yaw != m_yawTarget) //lint !e777 //testing floats for inequality ... close values will get clamped in this block
	{
		float delta = calculateMinimumDeltaAngle(m_yaw, m_yawTarget);
		float yaw = m_yaw + delta * std::min(1.0f, elapsedTime * (s_cameraRecenterSpeed * CuiPreferences::getCockpitCameraSnapSpeed()));
		yaw = clampAngleToRangePiNegPi(yaw);

		if (WithinEpsilonInclusive(m_yawTarget, yaw, 0.01f))
			yaw = m_yawTarget;

		setYawAndYawTarget(yaw, m_yawTarget);
	}

	if (m_pitch != m_pitchTarget) //lint !e777 //testing floats for inequality ... close values will get clamped in this block
	{
		float delta = calculateMinimumDeltaAngle(m_pitch, m_pitchTarget);
		float pitch = m_pitch + delta * std::min(1.0f, elapsedTime * (s_cameraRecenterSpeed * CuiPreferences::getCockpitCameraSnapSpeed()));
		pitch = clampAngleToRangePiNegPi(pitch);

		if (WithinEpsilonInclusive(m_pitchTarget, pitch, 0.01f))
			pitch = m_pitchTarget;

		setPitchAndPitchTarget(pitch, m_pitchTarget);
	}

	if (m_useTimerForCameraRecenter)
	{
		if (m_timerUntilCameraRecenter.updateZero(elapsedTime))
		{
			setYawAndYawTarget(m_yaw, 0.0f);
			setPitchAndPitchTarget(m_pitch, 0.0f);
			m_useTimerForCameraRecenter = false;
		}
	}

	CellProperty * targetCellProperty = 0;
	Transform targetTransform_p(Transform::IF_none);
	Transform targetTransform_w(Transform::IF_none);

	float zoomPercent = 0.0f;
	if (m_zoomSetting > 0 && m_zoomSettingList->size() > 1) 
		zoomPercent = static_cast<float>(m_zoomSetting - 1) / static_cast<float>(m_zoomSettingList->size() - 1);

	float const zoomMultiplier = linearInterpolate(1.0f, CuiPreferences::getCockpitCameraZoomMultiplier(), zoomPercent);
	bool const firstPerson = m_currentZoom < m_firstPersonDistance;
	bool firstPersonTest = false;
	firstPersonTest = calculatePilotCameraLocation(firstPerson, targetCellProperty, targetTransform_p, targetTransform_w);
	DEBUG_WARNING((firstPerson && !firstPersonTest || !firstPerson && firstPersonTest), ("CockpitCamera::alter (firstPerson != firstPersonTest)"));

	setParentCell(targetCellProperty);

	CellProperty::setPortalTransitionsEnabled(false);

	if (firstPerson)
	{
		// even though we are in first person we need to update the damper
		// with the actual world location of the camera.  Other wise, the
		// camera will be updated from the cell transformation
		m_cameraDampener->setDampen(false);
		IGNORE_RETURN(m_cameraDampener->update(targetTransform_w, elapsedTime));
	}
	else
	{
		// dampen the camera when in 3rd person
		//It's important to note that this makes the _p and _w go out of sync, which would be bad if we did it in 1st person
		m_cameraDampener->setDampen(true);
		targetTransform_p = m_cameraDampener->update(targetTransform_p, elapsedTime);
	}

	setTransform_o2p(targetTransform_p);

	CellProperty::setPortalTransitionsEnabled(true);

	CuiPreferences::PilotMouseMode const pilotMouseMode = CuiPreferences::getPilotMouseModeAutomatic();

	bool const wasLocked = m_locked;
	m_locked = false;
	float yawMod = 0.f;
	float joystickYawMod = 0.0f;
	float pitchMod = 0.f;
	float joystickPitchMod = 0.0f;
	float frameYaw = 0.f;
	float framePitch = 0.f;
	float frameRoll = 0.f;
	bool joystickInputReceived = false;

	bool cameraInputReceived = false;
	bool shipInputReceived = false;

	CuiPreferences::PovHatMode const povHatMode = CuiPreferences::getPovHatMode();

	if (!m_povHatActive)
		m_povHatPanLastSpeedFactor = s_povHatPanSpeedFactorStart;

	bool const wasRearView = m_rearView;
	m_rearView = false;

	NOT_NULL(m_queue);
	for (int i = 0; i < m_queue->getNumberOfMessages(); ++i)
	{
		int message = 0;
		float value = 0.f;
		m_queue->getMessage (i, &message, &value);

		switch (message)
		{
		case CM_rearView:
			m_rearView = true;

			if (m_yaw >= 0.0f)
				setYawAndYawTarget(PI, PI);
			else
				setYawAndYawTarget(-PI, -PI);

			setPitchAndPitchTarget(0.0f, 0.0f);
			break;

		case CM_cameraYawMouse:
			if (CuiPreferences::PMM_cockpitCamera == pilotMouseMode || wasLocked)
			{
				yawMod += value;
				if (!WithinEpsilonInclusive(0.0f, value, 0.005f))
					cameraInputReceived = true;
			}
			else if (CuiPreferences::PMM_virtualJoystick == pilotMouseMode)
			{
				if (!WithinEpsilonInclusive(0.0f, value, 0.005f))
					shipInputReceived = true;
			}
			break;

		case CM_cameraPitchMouse:
			if (CuiPreferences::PMM_cockpitCamera == pilotMouseMode || wasLocked)
			{
				pitchMod += value;
				if (!WithinEpsilonInclusive(0.0f, value, 0.005f))
					cameraInputReceived = true;
			}
			else if (CuiPreferences::PMM_virtualJoystick == pilotMouseMode)
			{
				if (!WithinEpsilonInclusive(0.0f, value, 0.005f))
					shipInputReceived = true;
			}

			break;

		case CM_yawButton:
		case CM_pitchButton:
		case CM_rollButton:
			shipInputReceived = true;
			break;

		case CM_yaw:
			{
				if (!WithinEpsilonInclusive(0.0f, value, 0.5f))
					joystickInputReceived = true;

				if (value >= 0.0f)
					value = clamp(0.0f, sqr(value), 1.0f);
				else
					value = -clamp(0.0f, sqr(value), 1.0f);

				joystickYawMod = value * 0.1f;
			}
			break;

		case CM_pitch:
			{
				if (!WithinEpsilonInclusive(0.0f, value, 0.5f))
					joystickInputReceived = true;

				if (value >= 0.0f)
					value = clamp(0.0f, sqr(value), 1.0f);
				else
					value = -clamp(0.0f, sqr(value), 1.0f);

				if (!CuiPreferences::isJoystickInverted())
					value = -value;

				joystickPitchMod += value * 0.05f;
			}
			break;

		case CM_roll:
			{
				if (!WithinEpsilonInclusive(0.0f, value, 0.5f))
					joystickInputReceived = true;
			}
			break;

		case CM_cameraLock:
			m_locked = true;
			break;

		case CM_cameraReset:
			setYawAndYawTarget(m_yaw, 0.0f);
			setPitchAndPitchTarget(m_pitch, 0.0f);
			break;

		case CM_cameraZoom:
		case CM_mouseWheel:
			{
				if (m_zoomSettingList)
				{
					if (value > 0)
						m_zoomSetting = std::max (m_zoomSetting - 1, 0);
					else
						m_zoomSetting = std::min (m_zoomSetting + 1, static_cast<int>(m_zoomSettingList->size()) - 1);
				}

				m_useTimerForCameraRecenter = false;
				m_timerUntilCameraRecenter.reset();
			}
			break;

		case CM_shipDynamicsYaw:
			frameYaw += value;
			break;

		case CM_shipDynamicsPitch:
			framePitch += value;
			break;

		case CM_shipDynamicsRoll:
			frameRoll += value;
			break;

		case CM_joystickPovDirection:

			{
				if (CuiPreferences::PHM_snap == povHatMode)
				{
					float const yawTarget = sin(value) * CuiPreferences::getPovHatSnapAngle();
					float const pitchTarget = -cos(value) * CuiPreferences::getPovHatSnapAngle();

					setYawAndYawTarget(m_yaw, yawTarget);
					setPitchAndPitchTarget(m_pitch, pitchTarget);
				}
				else if (CuiPreferences::PHM_pan == povHatMode)
				{
					float const yawSpeed = sin(value) * CuiPreferences::getPovHatPanSpeed();
					float const pitchSpeed = -cos(value) * CuiPreferences::getPovHatPanSpeed();

					m_povHatPanLastSpeedFactor = linearInterpolate(m_povHatPanLastSpeedFactor, s_povHatPanSpeedFactorEnd, std::min(1.0f, elapsedTime));

					yawMod += yawSpeed * (m_povHatPanLastSpeedFactor * elapsedTime);
					pitchMod += pitchSpeed * (m_povHatPanLastSpeedFactor * elapsedTime);
				}
				m_povHatActive = true;
			}

			break;
		case CM_joystickPovCenter:
			if (CuiPreferences::PHM_snap == povHatMode)
			{
				setYawAndYawTarget(m_yaw, 0.0f);
				setPitchAndPitchTarget(m_pitch, 0.0f);
			}

			m_povHatActive = false;
			break;

		default:
			break;
		}
	}

	if (wasRearView && !m_rearView)
	{
		setYawAndYawTarget(0.0f, 0.0f);
		setPitchAndPitchTarget(0.0f, 0.0f);
		if (m_wasShowCockpitBeforeRearView)
			CockpitCamera::setShowCockpit(true);
	}
	else if (!wasRearView && m_rearView)
	{
		m_wasShowCockpitBeforeRearView = CockpitCamera::getShowCockpit();
		CockpitCamera::setShowCockpit(false);
	}

	ShipObject const * const shipObject = NON_NULL(NON_NULL(m_shipTarget->asClientObject())->asShipObject());

	if ((m_cameraLockTarget) && (shipObject != 0))
	{
		//TODO: is this what is intended, or should it use the player's target		
		Object const * const lookAtTargetObject = shipObject->getPilotLookAtTarget().getObject();
		
		if (NULL != lookAtTargetObject)
		{
			Vector const & vectorToLookAtTarget_o = targetTransform_w.rotateTranslate_p2l(lookAtTargetObject->getPosition_w());
			
			bool thetaOk = !WithinEpsilonExclusive(vectorToLookAtTarget_o.x, 0.0f, 0.01f) || !WithinEpsilonExclusive(vectorToLookAtTarget_o.z, 0.0f, 0.01f);
			float const theta = thetaOk ? vectorToLookAtTarget_o.theta() : 0.0f;
			float const phi = vectorToLookAtTarget_o.phi();
			
			setYawAndYawTarget(m_yaw, theta);
			setPitchAndPitchTarget(m_pitch, phi);
		}
	}
	
	UNREF(frameYaw);
	UNREF(framePitch);
	UNREF(frameRoll);
	
	if (m_locked)
	{
		yawMod += joystickYawMod;
		pitchMod += joystickPitchMod;
	}

	if (joystickInputReceived)
	{
		if (m_locked)
			cameraInputReceived = true;
		else
			shipInputReceived = true;
	}

	if (cameraInputReceived)
	{
		m_useTimerForCameraRecenter = false;
		m_timerUntilCameraRecenter.reset();
	}
	else if (shipInputReceived && !m_locked)
	{
		if (CuiPreferences::getCockpitCameraRecenterOnShipMovement())
			m_useTimerForCameraRecenter = true;
	}

	//-- clamp values to the acceptable range
	if (m_zoomSettingList)
	{
		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, m_zoomSetting, static_cast<int>(m_zoomSettingList->size()));
		m_zoom = clamp(0.f, (*m_zoomSettingList)[static_cast<size_t>(m_zoomSetting)], getMaximumZoomOutSetting());
		m_zoom *= zoomMultiplier;
	}

	//-- if we are trying to zoom out, clamp the zoom to the first person distance.
	if (m_zoom >= m_firstPersonDistance)
		m_currentZoom = std::max(m_firstPersonDistance, m_currentZoom);

	m_currentZoom = linearInterpolate (m_currentZoom, m_zoom, std::min(1.0f, elapsedTime * 5.0f));

	//-- release of the camera lock, recenter the camera
	if (!m_locked && wasLocked && CuiPreferences::getCockpitCameraRecenterOnShipMovement())
	{
		setYawAndYawTarget(m_yaw, 0.0f);
		setPitchAndPitchTarget(m_pitch, 0.0f);
	}
	//-- otherwise if lock or cockpit camera mouse mode is engaged, move the camera
	else if (m_locked || CuiPreferences::PMM_cockpitCamera == pilotMouseMode || CuiPreferences::PHM_pan == povHatMode)
	{
		if (ClientHeadTracking::getEnabled())
		{
			float yaw = 0.f;
			float pitch = 0.f;
			ClientHeadTracking::getYawAndPitch(yaw, pitch);
			yaw = clamp(-c_firstPersonMaximumYaw, yaw * c_firstPersonMaximumYaw, c_firstPersonMaximumYaw);
			setYawAndYawTarget(yaw, yaw);
			pitch = clamp(-c_firstPersonMaximumPitch, pitch * c_firstPersonMaximumPitch, c_firstPersonMaximumPitch);
			setPitchAndPitchTarget(pitch, pitch);

			yawMod = 0.f;
			pitchMod = 0.f;
		}
		else
		{
			if (!WithinEpsilonExclusive(0.0f, yawMod, 0.001f))
			{
				float yaw = m_yaw + yawMod;
				if (yaw < -PI)
					yaw += PI_TIMES_2;
				if (yaw > PI)
					yaw -= PI_TIMES_2;

				setYawAndYawTarget(yaw, yaw);
			}

			if (!WithinEpsilonExclusive(0.0f, pitchMod, 0.001f))
			{
				float const pitch = clamp(-c_firstPersonMaximumPitch, m_pitch + pitchMod, c_firstPersonMaximumPitch);
				setPitchAndPitchTarget(pitch, pitch);
			}
		}
	}

	yaw_o(m_yaw);
	pitch_o(m_pitch);

	//-- Clear the list of objects that we're planning on excluding
	clearExcludedObjects();

	//-- Don't ever show the player if we're in the cockpit camera
	if (m_target)
	{
		ClientObject const * const clientObject = m_target->asClientObject();
		if (clientObject)
		{
			CreatureObject const * const creatureObject = clientObject->asCreatureObject();
			if (creatureObject)
				addExcludedObject(creatureObject);
		}
	}

	ClientWorld::setCollideOverrideExcludeObject(0);

	if (firstPerson)
	{
		//-- If we want the cockpit enabled, only display the ship if we're in a pob ship
		if (!ms_showCockpit && targetCellProperty == CellProperty::getWorldCellProperty())
		{
			addExcludedObject(m_shipTarget);

			//-- If the cockpit is disabled, we disable collisions with the ship
			ClientWorld::setCollideOverrideExcludeObject(m_shipTarget);
		}
	}
	else
	{
		Object const * const cockpitFrame = (shipObject != 0) ? shipObject->getCockpitFrame() : 0;

		if (cockpitFrame != 0)
		{
			addExcludedObject(cockpitFrame);
		}

		pitch_o(PI_OVER_2 * 0.125f);

		Vector offset(Vector::negativeUnitZ * m_currentZoom);
		offset += Vector::unitY * CuiPreferences::getCockpitCameraYOffset();

		move_o(offset);

		DEBUG_REPORT_PRINT(ms_debugFlagCockpitZoom, ("CockpitZoom settting [%d], distance [%f] / [%f] = [%f]\n", m_zoomSetting, m_currentZoom, zoomMultiplier));
	}

	//-- Chain back up to parent
	float alterResult = GameCamera::alter(elapsedTime);
	AlterResult::incorporateAlterResult(alterResult, AlterResult::cms_alterNextFrame);
	return alterResult;
}

// ----------------------------------------------------------------------

void CockpitCamera::setMessageQueue(MessageQueue const * const queue)
{
	m_queue = queue;
}

// ----------------------------------------------------------------------

void CockpitCamera::setTarget(Object const * const object)
{
	if (m_target.getPointer() != object)
		m_zoomSetting = 0;

	m_target = object;
}

// ----------------------------------------------------------------------

bool CockpitCamera::isFirstPerson() const
{
	return m_locked || (CuiPreferences::PMM_cockpitCamera == CuiPreferences::getPilotMouseModeAutomatic());
}

// ----------------------------------------------------------------------

float CockpitCamera::getMaximumZoomOutSetting() const
{
	NOT_NULL(m_zoomSettingList);
	return m_zoomSettingList->back();
}

// ======================================================================

void CockpitCamera::_setFrameAppearanceTemplate(AppearanceTemplate const *newTemplate)
{
	if (m_frameAppearanceTemplate)
	{
		AppearanceTemplateList::release(m_frameAppearanceTemplate);
		m_frameAppearanceTemplate = 0;
	}
	m_frameAppearanceTemplate=newTemplate;
}

// ----------------------------------------------------------------------

void CockpitCamera::load(char const * const fileName)
{
	Iff iff;
	if (iff.open(fileName, true))
	{
		iff.enterForm(TAG_CPIT);

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
		DEBUG_WARNING(true, ("CockpitCamera::load: cockpit %s not found", fileName));
}

// ----------------------------------------------------------------------

void CockpitCamera::load_0000(Iff & iff)
{
	iff.enterForm(TAG_0000);

		{
			iff.enterChunk(TAG_FRAM);

				std::string appearanceTemplateName;
				iff.read_string(appearanceTemplateName);

				if (!appearanceTemplateName.empty())
				{
					_setFrameAppearanceTemplate(AppearanceTemplateList::fetch(appearanceTemplateName.c_str()));
				}
				else
				{
					_setFrameAppearanceTemplate(0);
				}

			iff.exitChunk();
		}

		{
			float const zoomMultiplier = ConfigFile::getKeyFloat("ClientGame/CockpitCamera", "zoomMultiplier", 1.f);

			iff.enterChunk(TAG_ZOOM);
				NOT_NULL(m_zoomSettingList);
				m_zoomSettingList->clear();
				m_zoomSettingList->push_back(0.f);
				while(iff.getChunkLengthLeft(sizeof(float)))
					m_zoomSettingList->push_back(iff.read_float() * (zoomMultiplier >= 1.f ? zoomMultiplier : 1.f));
			iff.exitChunk(TAG_ZOOM);
		}

		{
			iff.enterChunk(TAG_FRST);
				m_firstPersonDistance = iff.read_float();
			iff.exitChunk();
		}

		{
			for (ZoomSettingList::iterator it = m_zoomSettingList->begin(); it != m_zoomSettingList->end(); )
			{
				float const zoom = *it;

				if (zoom < m_firstPersonDistance && it != m_zoomSettingList->begin())
				{
					//-- silently ignore zeros
					if (zoom != 0.0f)
					{
						char buffer [128];
						iff.formatLocation(buffer, sizeof(buffer));
						DEBUG_WARNING(true, ("CockpitCameraCockpitCamera [%s] has invalid zoom setting [%f], which is less than the first person distance [%f]", buffer, zoom, m_firstPersonDistance));
					}

					it = m_zoomSettingList->erase(it);
				}
				else
					++it;
			}
		}

		{
			iff.enterChunk(TAG_3OFF);
				m_3rdPersonCameraOffset.x = iff.read_float();
				m_3rdPersonCameraOffset.y = iff.read_float();
				m_3rdPersonCameraOffset.z = iff.read_float();
			iff.exitChunk();
		}

		{
			iff.enterChunk(TAG_1OFF);
				m_1stPersonCameraOffset.x = iff.read_float();
				m_1stPersonCameraOffset.y = iff.read_float();
				m_1stPersonCameraOffset.z = iff.read_float();
			iff.exitChunk();
		}

		{
			if (iff.enterForm(TAG_ISPB, true))
			{
				iff.enterChunk(TAG_HYPR);
					m_hyperspaceCameraOffset.x = iff.read_float();
					m_hyperspaceCameraOffset.y = iff.read_float();
					m_hyperspaceCameraOffset.z = iff.read_float();
				iff.exitChunk();
				iff.exitForm(TAG_ISPB);
			}
		}

	iff.exitForm(TAG_0000);
}

// ----------------------------------------------------------------------

void CockpitCamera::setDustEnabled(bool const dustEnabled)
{
	if (dustEnabled)
	{
		//-- Create the dust if it doesn't exist
		if (!m_dustObject)
		{
			TerrainObject const * const terrainObject = TerrainObject::getConstInstance();
			if (terrainObject && terrainObject->getAppearance())
			{
				SpaceTerrainAppearanceTemplate const * const spaceTerrainAppearanceTemplate = dynamic_cast<SpaceTerrainAppearanceTemplate const *>(terrainObject->getAppearance()->getAppearanceTemplate());
				if (spaceTerrainAppearanceTemplate)
				{
					m_dustObject = new Object;
					m_dustObject->setAppearance(new DustAppearance(m_shipTarget, spaceTerrainAppearanceTemplate->getNumberOfDust(), spaceTerrainAppearanceTemplate->getDustRadius()));
					RenderWorld::addObjectNotifications(*m_dustObject);
					addChildObject_o(m_dustObject);
				}
			}
		}
	}
	else
	{
		//-- Destroy the dust if it exists
		if (m_dustObject)
		{
			m_dustObject->removeFromWorld();

			delete m_dustObject;
			m_dustObject = 0;
		}
	}
}

// ----------------------------------------------------------------------

bool CockpitCamera::createFrame(Object * & object, Transform * const cameraOffset_p) const
{
	if (m_frameAppearanceTemplate && m_shipTarget)
	{
		Appearance * const frameAppearance = m_frameAppearanceTemplate->createAppearance();

		if (cameraOffset_p != 0)
		{
			if (!frameAppearance->findHardpoint(c_cameraHardpointName, *cameraOffset_p))
					DEBUG_WARNING(true, ("Could not find camera hardpoint %s in appearance template %s", c_cameraHardpointName.getString(), m_frameAppearanceTemplate->getName()));
		}

		object = new Object;
		object->setAppearance(frameAppearance);

		//-- this name is expected by the UI so it can exlude the cockpit frame from render
		object->setDebugName("ShipCockpitFrame");
		return true;
	}
	return false;
}

// ----------------------------------------------------------------------

void CockpitCamera::setYawAndYawTarget(float yaw, float yawTarget)
{
	m_yaw = yaw;
	m_yawTarget = yawTarget;
}

//----------------------------------------------------------------------

void CockpitCamera::setPitchAndPitchTarget(float pitch, float pitchTarget)
{
	m_pitch = pitch;
	m_pitchTarget = pitchTarget;
}

//----------------------------------------------------------------------

void CockpitCamera::setCameraLockTarget(bool b)
{
	if (m_cameraLockTarget && !b)
	{
		setYawAndYawTarget(m_yaw, 0.0f);
		setPitchAndPitchTarget(m_pitch, 0.0f);
	}

	m_cameraLockTarget = b;
}

//----------------------------------------------------------------------

bool CockpitCamera::getCameraLockTarget() const
{
	return m_cameraLockTarget;
}

//----------------------------------------------------------------------

Transform const & CockpitCamera::getHyperspaceCameraOffset() const
{
	return m_hyperspaceOffset_p;
}

//----------------------------------------------------------------------

void CockpitCamera::setIsInHyperspace(bool isInHyperspace)
{
	m_isInHyperspace = isInHyperspace;
}

//----------------------------------------------------------------------

bool CockpitCamera::isZoomWithinFirstPersonDistance() const
{
	return m_zoom < m_firstPersonDistance;
}


// ======================================================================
