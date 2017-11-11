// ======================================================================
//
// DebugPortalCamera.cpp
//
// copyright 2000-2002 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/DebugPortalCamera.h"

#include "clientGame/ConfigClientGame.h"
#include "clientGame/DeadReckoningModelOmniDirectional.h"
#include "clientGame/Game.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/PlayerCreatureController.h"
#include "clientGame/WorldSnapshot.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "sharedDebug/VTune.h"
#include "sharedDebug/PerformanceTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/MessageQueue.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/Production.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/CellProperty.h"

#include <algorithm>
#include <vector>

// ======================================================================

const Tag TAG_PATH = TAG(P,A,T,H);

// ======================================================================

DebugPortalCamera::DebugPortalCamera ()
: GameCamera (),
	m_queue (0),
	m_vtuneCounter(0),
	m_target (),
	m_mode (M_normal),
	m_transformList (new TransformList),
	m_pathLeg (0),
	m_pathTime (30.f),
	m_pathLegTimer (0.f),
	m_minimumFrameRate (1000.f),
	m_maximumFrameRate (0.f),
	m_frameRateList (new FloatList),
	m_frameTimeList (new FloatList),
	m_deadReckoningModel (new DeadReckoningModelOmniDirectional ()),
	m_playerCreatureController(NULL)
{
	for (int i = 0; i < static_cast<int>(K_COUNT); i++)
		m_keys [i] = false;
	m_frameRateList->reserve(8 * 1024);
	m_frameTimeList->reserve(8 * 1024);
}

// ----------------------------------------------------------------------

DebugPortalCamera::~DebugPortalCamera (void)
{
	m_queue  = 0;
	m_target = 0;

	delete m_transformList;
	m_transformList = 0;

	delete m_frameRateList;
	m_frameRateList = 0;

	delete m_frameTimeList;
	m_frameTimeList = 0;

	delete m_deadReckoningModel;
	m_deadReckoningModel = 0;

	m_playerCreatureController = 0;
}

// ----------------------------------------------------------------------

void DebugPortalCamera::setActive (bool newActive)
{
	if (newActive && !isActive () && m_target)
	{
		setParentCell(m_target->getParentCell());
		CellProperty::setPortalTransitionsEnabled(false);
		setTransform_o2p (m_target->getTransform_o2p ());
		CellProperty::setPortalTransitionsEnabled(true);
		move_o (Vector::unitY);
	}

	GameCamera::setActive (newActive);
}


// ----------------------------------------------------------------------

float DebugPortalCamera::alter (float elapsedTime)
{
	if (!isActive ())
	{
		// @todo consider returning "no alter" and having activate put us on the alter queue.
		return AlterResult::cms_alterNextFrame;
	}

	NOT_NULL (m_queue);

	int i;
	for (i = 0; i < static_cast<int>(K_COUNT); i++)
		m_keys [i] = false;

	//-- handle camera specific messages
	float yaw = 0;
	float pitch = 0;
	for (i = 0; i < m_queue->getNumberOfMessages (); i++)
	{
		int   message;
		float value;

		m_queue->getMessage (i, &message, &value);

		switch (message)
		{
		case CM_cameraYawMouse:    yaw    += value;            break;
		case CM_cameraPitchMouse:  pitch  += value;            break;
		case CM_walk:
		case CM_mouseWalk:			
			m_keys [K_up]     = true;
			break;
		case CM_down:         m_keys [K_down]   = true;   break;
		case CM_left:         m_keys [K_left]   = true;   break;
		case CM_right:        m_keys [K_right]  = true;   break;
		case CM_toggleRunOn:  m_keys [K_button] = true;   break;

		case CM_cameraPathNormal:
			{
				m_mode = M_normal;
			}
			break;

		case CM_cameraPathAddWaypoint:
			{
				m_mode = M_record;
				m_transformList->push_back (getTransform_o2w ());
			}
			break;

		case CM_cameraPathDeleteLastWaypoint:
			{
				m_mode = M_record;
				if (!m_transformList->empty())
					m_transformList->pop_back ();
			}
			break;

		case CM_cameraPathPlayback:
			{
				{
					Object *player = Game::getPlayer();
					m_playerCreatureController = dynamic_cast<PlayerCreatureController *>(player->getController());
					DEBUG_REPORT_LOG(true, ("Camera path playback beginning\n"));
				}

				if (m_transformList->size () > 1)
				{
					m_mode = M_playback;
					m_pathLeg  = 0;
					m_pathLegTimer.setExpireTime (m_pathTime / (m_transformList->size () - 1));
					m_pathLegTimer.reset();
					m_frameRateList->clear ();
					m_frameTimeList->clear ();
					m_minimumFrameRate = 1000.f;
					m_maximumFrameRate = 0.f;
#if 1
#if PRODUCTION == 0
					if (ConfigClientGame::getDebugPortalCameraPathConstantStepRate() != 0.0f)
						VTune::resumeNextFrame();
#endif
#else
					PerformanceTimer t;

					VTune::resume();

					t.start();

						while (m_mode != M_normal)
						{
							if (m_pathLeg >= m_transformList->size () - 1)
								m_mode = M_normal;
							else
							{
								float const updateTime = 1.0f / ConfigClientGame::getDebugPortalCameraPathConstantStepRate();

								if (m_pathLegTimer.updateZero (updateTime))
									++m_pathLeg;

								m_deadReckoningModel->set ((*m_transformList) [m_pathLeg], Vector::zero, (*m_transformList) [m_pathLeg + 1], Vector::zero);

								float const t = m_pathLegTimer.getElapsedRatio ();

								Transform transform;
								m_deadReckoningModel->computeTransform (t, transform);
								WorldSnapshot::update(transform.getPosition_p());
							}
						}

					t.stop();

					VTune::pause();
					t.logElapsedTime("path run: ");
#endif

				}
			}
			break;

		case CM_vtuneCounter:
#if PRODUCTION == 0
			m_vtuneCounter = 200;
			VTune::resume();
#endif
			break;

		default:
			break;
		}
	}

	if (m_mode == M_normal || m_mode == M_record)
	{
		//-- 
		yaw_o (yaw);
		pitch_o (pitch);
		
		//-- move
		const float translationAmount = elapsedTime * (m_keys [K_button] ? CONST_REAL (30) : CONST_REAL (7));

		Vector direction;
		if (m_keys [K_up])
			direction += Vector::unitZ * translationAmount;

		if (m_keys [K_down])
			direction += -Vector::unitZ * translationAmount;

		if (m_keys [K_left])
			direction += -Vector::unitX * translationAmount;

		if (m_keys [K_right])
			direction += Vector::unitX * translationAmount;

		move_o(direction);

		// keep the up vector pointed up
		Vector up = rotate_w2o(Vector::unitY);
		const float roll = -atan2(up.x, up.y);
		roll_o(roll);
	}
	else
	{
		if (m_transformList->empty ())
		{
			m_mode = M_normal;
		}
		else
		{
			const float frameRate = Clock::framesPerSecond ();
			m_minimumFrameRate = std::min (m_minimumFrameRate, frameRate);
			m_maximumFrameRate = std::max (m_maximumFrameRate, frameRate);
			m_frameRateList->push_back (frameRate);
			m_frameTimeList->push_back (Clock::frameTime());

			if (m_pathLeg >= m_transformList->size () - 1)
			{
#if PRODUCTION == 0
				if (ConfigClientGame::getDebugPortalCameraPathConstantStepRate() != 0.0f)
					VTune::pauseNextFrame();
#endif

				DEBUG_REPORT_LOG(true, ("Camera path playback finished\n"));

				static int size = m_frameRateList->size();
				for (int i = 0; i < size; ++i)
				{
					DEBUG_REPORT_LOG_PRINT(true, ("path %4d %6.5f %5.2f\n", i, (*m_frameTimeList)[i], (*m_frameRateList)[i]));
				}

				m_mode = M_normal;
			}
			else
			{
				float updateTime = elapsedTime;

				if (ConfigClientGame::getDebugPortalCameraPathConstantStepRate() != 0.0f)
					updateTime = 1.0f / ConfigClientGame::getDebugPortalCameraPathConstantStepRate();

				if (m_pathLegTimer.updateZero (updateTime))
					++m_pathLeg;

				m_deadReckoningModel->set ((*m_transformList) [m_pathLeg], Vector::zero, (*m_transformList) [m_pathLeg + 1], Vector::zero);

				const float t = m_pathLegTimer.getElapsedRatio ();

				Transform transform;
				m_deadReckoningModel->computeTransform (t, transform);

				setTransform_o2p (transform);
				if (ConfigClientGame::getDebugPortalCameraPathDragPlayer())
					m_playerCreatureController->warpClient(transform);
			}
		}
	}

#if PRODUCTION == 0
	if (m_vtuneCounter && --m_vtuneCounter <= 0)
		VTune::pause();
#endif

	return GameCamera::alter (elapsedTime);
}

// ----------------------------------------------------------------------

void DebugPortalCamera::drawScene (void) const
{
	GameCamera::drawScene ();

	switch (m_mode)
	{
	case M_normal:
		break;

	case M_record:
		{
			//-- render path
			if (m_transformList->size () > 0)
			{
				Graphics::setStaticShader (ShaderTemplateList::get3dVertexColorZStaticShader ());

				const uint n = m_transformList->size ();

				uint i;
				for (i = 0; i < n; i++)
				{
					Graphics::setObjectToWorldTransformAndScale ((*m_transformList) [i], Vector::xyz111);
					Graphics::drawFrame ();
					Graphics::drawBox (Vector::xyz111 * -0.5f, Vector::xyz111 * 0.5f, VectorArgb::solidRed);
				}

				Graphics::setObjectToWorldTransformAndScale (Transform::identity, Vector::xyz111);

				for (i = 0; i < n - 1; ++i) 
					Graphics::drawLine ((*m_transformList) [i].getPosition_p (), (*m_transformList) [i + 1].getPosition_p (), VectorArgb::solidYellow);

				Graphics::drawLine (m_transformList->back ().getPosition_p (), getPosition_w () - Vector::unitY, VectorArgb::solidYellow);
			}
		}
		break;

	case M_playback:
		break;
	}
}

// ----------------------------------------------------------------------

bool DebugPortalCamera::loadPath (const char* filename)
{
	Iff iff;
	if (iff.open (filename, true))
	{
		iff.enterForm (TAG_PATH);
			iff.enterForm (TAG_0000);

				iff.enterChunk (TAG_INFO);
					m_pathTime = iff.read_float ();
				iff.exitChunk ();

				iff.enterChunk (TAG_PATH);

					m_transformList->clear ();

					const int n = iff.getChunkLengthLeft (sizeof (Transform));
					int i;
					for (i = 0; i < n; ++i)
						m_transformList->push_back (iff.read_floatTransform ());

				iff.exitChunk ();

			iff.exitForm ();
		iff.exitForm ();

		if (!m_transformList->empty ())
			setTransform_o2p (m_transformList->front ());

		return true;
	}

	return false;
}

// ----------------------------------------------------------------------

bool DebugPortalCamera::savePath (const char* filename)
{
	Iff iff (1024);
	iff.insertForm (TAG_PATH);
		iff.insertForm (TAG_0000);

			iff.insertChunk (TAG_INFO);
				iff.insertChunkData (m_pathTime);
			iff.exitChunk ();

			iff.insertChunk (TAG_PATH);

				uint i;
				for (i = 0; i < m_transformList->size (); ++i)
					iff.insertChunkData ((*m_transformList) [i]);

			iff.exitChunk ();

		iff.exitForm ();
	iff.exitForm ();

	return iff.write (filename);
}

// ----------------------------------------------------------------------

float DebugPortalCamera::getPathTime () const
{
	return m_pathTime;
}

// ----------------------------------------------------------------------

void DebugPortalCamera::setPathTime (float pathTime)
{
	m_pathTime = pathTime;
}

// ----------------------------------------------------------------------

float DebugPortalCamera::getMinimumFrameRate () const
{
	return m_minimumFrameRate;
}

// ----------------------------------------------------------------------

float DebugPortalCamera::getMaximumFrameRate () const
{
	return m_maximumFrameRate;
}

// ----------------------------------------------------------------------

float DebugPortalCamera::getAverageFrameRate () const
{
	if (!m_frameRateList->empty ())
	{
		float sum = 0.f;

		const uint n = m_frameRateList->size ();
		uint i;
		for (i = 0; i < n; ++i)
			sum += (*m_frameRateList) [i];

		return sum / static_cast<float> (m_frameRateList->size ());
	}

	return 0.f;
}


// ----------------------------------------------------------------------

void DebugPortalCamera::clearPath ()
{
	m_transformList->clear ();
}

// ======================================================================
