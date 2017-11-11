//
// FreeCamera.cpp
// asommers 12-16-2000
//
// copyright 2000, verant interactive, inc.
//

//-------------------------------------------------------------------

#include "clientGame/FirstClientGame.h"
#include "clientGame/FreeCamera.h"
#include "clientGame/FreeChaseCamera.h"
#include "clientGame/Game.h"
#include "clientGame/GroundScene.h"
#include "clientGame/ConfigClientGame.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/MessageQueue.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/CellProperty.h"

#include <algorithm>

//-----------------------------------------------------------------

FreeCamera::FreeCamera ()
: GameCamera (),
	m_queue (0),
	m_target (),
	m_mode (M_fly),
	m_info(),
	m_targetInfo(),
	m_interpolating (false),
	m_minPivotDistance (CONST_REAL (0.1)),
	m_lastSpeed (0.0f),
	m_currentCell(getParentCell()),
	m_initializeFromFreeChaseCamera(false),
	m_initializeFromFreeCamera(false)
{
	/*Don't let this camera get moved between cells unless we specifically do it ourselves
	 (since we want this camera to remain in the same cell as the avatar)*/
	CellProperty::removePortalCrossingNotification (*this);

	Zero (m_info);
	Zero (m_targetInfo);

	for (int i = 0; i < static_cast<int>(K_COUNT); i++)
		m_keys [i] = false;
}

//----------------------------------------------------------------------

FreeCamera::~FreeCamera (void)
{
	m_queue  = 0;
	m_target = 0;
	m_currentCell = NULL;
}

//-------------------------------------------------------------------

/** Return the pivot point in world space (though the camera lives in the same cell as its target)
 */
const Vector FreeCamera::getPivotPoint () const
{
	Vector result;
	if(m_currentCell != CellProperty::getWorldCellProperty())
	{
		//convert pivot point to world space
		result = m_currentCell->getOwner().rotateTranslate_o2w(m_info.translate);
	}
	else
		result = m_info.translate;
	return result;
}

//-------------------------------------------------------------------

/** Given a pivot point in world-coordinates, store it (and convert to cell-coordinates if needed
 */
void FreeCamera::setPivotPoint (const Vector & pt)
{
	if(m_currentCell != CellProperty::getWorldCellProperty())
	{
		m_info.translate = m_currentCell->getOwner().rotateTranslate_w2o(pt);
	}
	else
		m_info.translate = pt;
}

//-----------------------------------------------------------------

/** Given a pivot point in world-coordinates, store it (and convert to cell-coordinates if needed
 */
void FreeCamera::setTargetInfo (const FreeCamera::Info & info, real time)
{
	UNREF (time);
	if(m_currentCell != CellProperty::getWorldCellProperty())
	{
		//convert the rotation point to the new cell
		Vector newCellVector = m_currentCell->getOwner().rotateTranslate_w2o(info.translate);
		m_targetInfo = info;
		m_targetInfo.translate = newCellVector;
	}
	else
	{
		m_targetInfo = info;
	}
}

//-----------------------------------------------------------------

/** Return the pivot point in world space (though the camera lives in the same cell as its target)
 */
void FreeCamera::getTargetInfo (Info & info) const
{
	if(m_currentCell != CellProperty::getWorldCellProperty())
	{
		//convert the rotation point to the new cell
		Vector worldVector = m_currentCell->getOwner().rotateTranslate_o2w(m_targetInfo.translate);
		info = m_targetInfo;
		info.translate = worldVector;
	}
	else
		info = m_targetInfo;
}

//-----------------------------------------------------------------

void FreeCamera::setActive (bool newActive)
{
	if (newActive && !isActive () && m_target)
	{
		if(m_initializeFromFreeChaseCamera)
		{
			GroundScene * const groundScene = safe_cast<GroundScene * const>(Game::getScene());
			FreeChaseCamera * freeChaseCamera = 0;
			if(groundScene != 0)
			{
				GameCamera * const gameCamera = groundScene->getCamera(GroundScene::CI_freeChase);
				freeChaseCamera = safe_cast<FreeChaseCamera * const>(gameCamera);
			}
			if (groundScene != 0 && freeChaseCamera != 0)
			{
				Object const * const target = m_target->getRootParent();
				//set the camera to be in the same cell as the target (and since cell notifications are off, it will stay there)
				CellProperty* currentCell = getParentCell();
				CellProperty* targetCell = target->getParentCell();
				if(currentCell != targetCell)
				{
					//convert the m_info to the new cell coordinate system
					convertPivotPoint(targetCell);
				}
				m_interpolating = false;
				m_info.yaw   = freeChaseCamera->getYaw();
				m_info.pitch = freeChaseCamera->getPitch();
				resetRotateTranslate_o2p ();

				setPosition_w(freeChaseCamera->getPosition_w());
				yaw_o   (m_info.yaw);
				pitch_o (m_info.pitch);
				m_info.translate = getPosition_p ();
				m_info.distance = (freeChaseCamera->getTarget()->getPosition_w() - getPosition_w()).magnitude();
			}
		}
		else if(!m_initializeFromFreeCamera)
		{
			Object const * const target = m_target->getRootParent();

			//set the camera to be in the same cell as the target (and since cell notifications are off, it will stay there)
			CellProperty* currentCell = getParentCell();
			CellProperty* targetCell = target->getParentCell();
			if(currentCell != targetCell)
			{
				//convert the m_info to the new cell coordinate system
				convertPivotPoint(targetCell);
			}
			m_interpolating = false;
			m_info.yaw   = target->getObjectFrameK_w ().theta ();
			m_info.pitch = PI_OVER_4;
			const real radius = target->getAppearanceSphereRadius ();
			resetRotateTranslate_o2p ();

			move_o  (target->getPosition_p () + (Vector::unitY * CONST_REAL (2) * radius));
			yaw_o   (m_info.yaw);
			pitch_o (m_info.pitch);
			move_o  (- Vector::unitZ * CONST_REAL (2) * radius);
			m_info.translate = getPosition_p ();
			m_info.distance = std::max (CONST_REAL (0.1), CONST_REAL (2) * radius);
		}		
	}

	if(!newActive)
		//set the camera into the world cell so that we'll have the correct cell transition if the target is inside a cell on the next activate
		convertPivotPoint(CellProperty::getWorldCellProperty());

	GameCamera::setActive (newActive);
	m_initializeFromFreeChaseCamera = false;
	m_initializeFromFreeCamera = false;
}

//-----------------------------------------------------------------

void FreeCamera::setMode (Mode mode)
{
	if (mode == m_mode)
		return;

	m_mode = mode;

	//-- pivot
	if (m_mode == M_pivot)
	{
		m_info.translate += rotate_o2w (Vector::unitZ * m_info.distance);

	}
	
	//-- fly
	else
	{
		m_info.translate -= rotate_o2w (Vector::unitZ * m_info.distance);
	}
}

//-------------------------------------------------------------------

namespace
{
	float angleDiff2Pi( float a, float b ) {
        float diff = a - b;
		
        if ( diff < -PI )
			diff += PI_TIMES_2;
        else if ( diff > PI )
			diff -= PI_TIMES_2;
		
        return diff;
	}
	
	float angleClamp2Pi (float a)
	{
		
		if (a < 0)
			return a + floorf (-a / PI_TIMES_2) * PI_TIMES_2 + PI_TIMES_2;
		
		if (a > PI_TIMES_2)
			return a - floorf (a / PI_TIMES_2) * PI_TIMES_2;
		
		return a;
	}
}

//-----------------------------------------------------------------

float FreeCamera::alter (float time)
{
	if (!isActive ())
	{
		// @todo consider returning no alter, then have activate put us on the alter scheduler.
		return AlterResult::cms_alterNextFrame;
	}

	m_targetInfo.distance = std::max (m_minPivotDistance, m_targetInfo.distance);
	m_info.distance       = std::max (m_minPivotDistance, m_info.distance);

	//-- update the interpolation if necessary
	if (m_interpolating)
	{
		int done = 0;
		
		m_targetInfo.yaw = angleClamp2Pi (m_targetInfo.yaw);
		
		if (m_targetInfo.pitch < -PI)
			m_targetInfo.pitch = -PI;
		else if (m_targetInfo.pitch > PI)
			m_targetInfo.pitch = PI;

		static const real threshold = CONST_REAL (0.001);
		static const real fraction  = CONST_REAL (0.3);

		{
			if (m_info.translate.magnitudeBetweenSquared (m_targetInfo.translate) < threshold)
			{
				m_info.translate = m_targetInfo.translate;
				++done;
			}
			else
				m_info.translate = Vector::linearInterpolate (m_info.translate, m_targetInfo.translate, fraction);
		}
			
		{
			if (fabs (m_info.pitch - m_targetInfo.pitch) < threshold)
			{
				m_info.pitch = m_targetInfo.pitch;
				++done;
			}
			else
				m_info.pitch += (m_targetInfo.pitch - m_info.pitch) * fraction;					
		}
		
		{
			const real ydiff = angleDiff2Pi (m_targetInfo.yaw, m_info.yaw);
			
			if (fabs (ydiff) < threshold)
			{
				m_info.yaw = m_targetInfo.yaw;
				++done;
			}
			else
				m_info.yaw += (ydiff) * fraction;
		}
		
		{
			if (fabs (m_info.distance - m_targetInfo.distance) < threshold)
			{
				m_info.distance = m_targetInfo.distance;
				++done;
			}
			else
				m_info.distance += (m_targetInfo.distance - m_info.distance) * fraction;			
		}
		
		if (done == 4)
			m_interpolating = false;
	}
	
	m_info.yaw = angleClamp2Pi (m_info.yaw);
	
	if (m_info.pitch < -PI)
		m_info.pitch = -PI;
	else if (m_info.pitch > PI)
		m_info.pitch = PI;		

	NOT_NULL (m_queue);

	int i;
	for (i = 0; i < static_cast<int>(K_COUNT); i++)
		m_keys [i] = false;

	//-- handle camera specific messages
	for (i = 0; i < m_queue->getNumberOfMessages (); i++)
	{
		int  message;
		real value;

		m_queue->getMessage (i, &message, &value);

		//-- stop interpolating on inputmap messages
		m_interpolating = false;

		switch (message)
		{
		case CM_cameraYawMouse:
			setMode (M_fly);
			m_info.yaw            += value;
			break;
			
		case CM_cameraPitchMouse:
			setMode (M_fly);
			m_info.pitch          += value;
			break;

		case CM_walk:
			m_keys [K_up]     = true;
			break;
		case CM_down:           m_keys [K_down]   = true;   break;
		case CM_left:           m_keys [K_left]   = true;   break;
		case CM_right:          m_keys [K_right]  = true;   break;
		case CM_toggleRunOn:    m_keys [K_button] = true;   break;
		case CM_turn:
			{
				if (value < 0.0f)
					m_keys[K_left] = true;
				else if (value > 0.0f)
					m_keys[K_right] = true;
			}
			break;

		case CM_cameraPivotYaw:
			setMode (M_pivot);
			m_info.yaw      += value;
			break;
			
		case CM_cameraPivotPitch:
			setMode (M_pivot);
			m_info.pitch  += value;
			break;
			
		case CM_cameraPivotTranslateX:
			setMode (M_pivot);
			m_info.translate += rotate_o2p (Vector::unitX * value) * std::max (CONST_REAL (10), m_info.distance);
			break;
			
		case CM_cameraPivotTranslateY:
			setMode (M_pivot);
			m_info.translate += rotate_o2p (Vector::unitY * value) * std::max (CONST_REAL (10), m_info.distance);
			break;

		case CM_cameraPivotZoom:
			setMode (M_pivot);
			m_info.distance = std::max (CONST_REAL (0), m_info.distance);
			m_info.distance = std::min (m_info.distance + value * sqrt (std::max (CONST_REAL (10), m_info.distance)), CONST_REAL (8192));

			if (m_info.distance < CONST_REAL (0.3))
			{
				const real diff = CONST_REAL (0.3) - m_info.distance;
				m_info.translate += getObjectFrameK_p () * diff;
			}
			break;

		case CM_cameraTranslateX:
			setMode (M_fly);
			m_info.translate.x += value;
			break;

		case CM_cameraTranslateY:
			setMode (M_fly);
			m_info.translate.y += value;
			break;

		case CM_cameraTranslateZ:
			setMode (M_fly);
			m_info.translate.z += value;
			break;

		default:
			break;
		}
	}
	
	const float speedFast = ConfigClientGame::getFreeCameraSpeedFast ();
	const float speedSlow = ConfigClientGame::getFreeCameraSpeedSlow ();

	if (m_mode == M_fly)
	{
		//-- move
		real translationAmount = time;
		
		if (m_keys [K_button])
			m_lastSpeed = std::min (speedFast, m_lastSpeed + speedFast * time);
		else
			m_lastSpeed = speedSlow;

		translationAmount *= m_lastSpeed;
		
		if (m_keys [K_up])
		{
			Vector worldTrans = rotate_o2w ( Vector::unitZ * translationAmount);
			Vector cellTrans = m_currentCell->getOwner().rotate_w2o(worldTrans);
			m_info.translate += cellTrans;
		}
		if (m_keys [K_down])
		{
			Vector worldTrans = rotate_o2w (-Vector::unitZ * translationAmount);
			Vector cellTrans = m_currentCell->getOwner().rotate_w2o(worldTrans);
			m_info.translate += cellTrans;
		}
			
		if (m_keys [K_left])
		{
			Vector worldTrans = rotate_o2w (-Vector::unitX * translationAmount);
			Vector cellTrans = m_currentCell->getOwner().rotate_w2o(worldTrans);
			m_info.translate += cellTrans;
		}
		
		if (m_keys [K_right])
		{
			Vector worldTrans = rotate_o2w ( Vector::unitX * translationAmount);
			Vector cellTrans = m_currentCell->getOwner().rotate_w2o(worldTrans);
			m_info.translate += cellTrans;
		}
	}

	if(m_info.yaw > PI)
		m_info.yaw -= 2.0f * PI;
	if(m_info.yaw < -PI)
		m_info.yaw += 2.0f * PI;



	resetRotateTranslate_o2p ();
	move_o (m_info.translate);

	m_info.pitch = clamp (-PI_OVER_2, m_info.pitch, PI_OVER_2);

	yaw_o   (m_info.yaw);
	pitch_o (m_info.pitch);

	if (m_mode == M_pivot)
	{
		move_o (Vector (CONST_REAL (0), CONST_REAL (0), -m_info.distance));
	}
	
	return GameCamera::alter (time);
}

//----------------------------------------------------------------------

/** Since we maintain a point in space and yaw and pitch to move to each frame, we need to convert
    these values into new cell coordinate systems correctly.  This function converts the m_info value
		to new cell-oriented values, and sets the current cell to the given cellproperty.
*/
void    FreeCamera::convertPivotPoint (CellProperty* cell)
{
	//convert the rotation point to the new cell
	{
		Vector worldVector = m_currentCell->getOwner().rotateTranslate_o2w(m_info.translate);
		Vector newCellVector = cell->getOwner().rotateTranslate_w2o(worldVector);
		m_info.translate = newCellVector;

		//convert the yaw and pitch to the new cell
		const Vector& oldForward = m_currentCell->getOwner().getObjectFrameK_w();
		const Vector& newForward = cell->getOwner().getObjectFrameK_w();
		float oldYaw             = atan2(oldForward.x, oldForward.z);
		float newYaw             = atan2(newForward.x, newForward.z);
		float deltaYaw           = newYaw - oldYaw;

		//apply the difference to our stored value
		m_info.yaw -= deltaYaw;
	}

	//convert the target loc to the new cell
	{
		//convert the rotation point to the new cell
		Vector worldVector = m_currentCell->getOwner().rotateTranslate_o2w(m_targetInfo.translate);
		Vector newCellVector = cell->getOwner().rotateTranslate_w2o(worldVector);
		m_targetInfo.translate = newCellVector;

		//convert the yaw and pitch to the new cell
		const Vector& oldForward = m_currentCell->getOwner().getObjectFrameK_w();
		const Vector& newForward = cell->getOwner().getObjectFrameK_w();
		float oldYaw             = atan2(oldForward.x, oldForward.z);
		float newYaw             = atan2(newForward.x, newForward.z);
		float deltaYaw           = newYaw - oldYaw;

		//apply the difference to our stored value
		m_targetInfo.yaw -= deltaYaw;
	}

	//store the new cell
	m_currentCell = cell;
	setParentCell(m_currentCell);
}

//-------------------------------------------------------------------

