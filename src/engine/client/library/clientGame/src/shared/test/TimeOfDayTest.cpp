//===================================================================
//
// TimeOfDayTest.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/TimeOfDayTest.h"

#include "clientGraphics/Graphics.h"
#include "clientGraphics/Light.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientObject/ObjectListCamera.h"
#include "clientObject/ShadowManager.h"
#include "clientObject/ShadowVolume.h"
#include "sharedFoundation/Timer.h"

//===================================================================

namespace
{
	bool    ms_debugReport = true;
	bool    ms_paused      = true;

	float   m_currentTime  = 0.f;
	float   m_cycleTime    = 5.f;
	Timer   m_timer (m_cycleTime);
	
	Object* m_mainLight    = 0;
	float   m_mainYaw      = 0.f;
	Object* m_fillLight    = 0;
	float   m_fillYaw      = 0.f;
	Object* m_bounceLight  = 0;
	float   m_bounceYaw    = 0.f;
}

//===================================================================

TimeOfDayTest::TimeOfDayTest () : 
	TestIoWin ("TimeOfDayTest")
{
	ShadowManager::setEnabled (true);
	ShadowManager::setMeshShadowsVolumetric (true);
	ShadowManager::setSkeletalShadowsVolumetric (true);

	m_mainLight   = new Object ();
	m_fillLight   = new Object ();
	m_bounceLight = new Object ();
}

//-------------------------------------------------------------------

TimeOfDayTest::~TimeOfDayTest ()
{
	delete m_mainLight;
	delete m_fillLight;
	delete m_bounceLight;
}

//-------------------------------------------------------------------

IoResult TimeOfDayTest::processEvent (IoEvent* event)
{
	const IoResult result = TestIoWin::processEvent (event);

	switch (event->type)
	{
	case IOET_WindowKill:
		return IOR_PassKillMe;

	case IOET_Character:
		{
			if (tolower (event->arg2) == 'p')
				ms_paused = !ms_paused;

			if (tolower (event->arg2) == 's')
				m_timer.reset ();
		}
		break;

	case IOET_Update:
		{
			const float elapsedTime = event->arg3;

			if (!ms_paused)
				m_timer.updateSubtract (elapsedTime);
	
			const float timeRatio  = m_timer.getElapsedRatio ();
			const float PI_OVER_6  = PI_OVER_3 * 0.5f;
			const float PI_OVER_8  = PI_OVER_4 * 0.5f;
			const float PI_OVER_16 = PI_OVER_4 * 0.25f;

			//-- jake wants the main light at 12 degrees from the y axis
			if (timeRatio <= 0.5f)
				m_mainYaw = -PI_OVER_2 + timeRatio * PI_TIMES_2;
			else
				m_mainYaw = -PI - PI_OVER_2 + timeRatio * PI_TIMES_2;

			Transform t;
			t.pitch_l (PI_OVER_2 - PI_OVER_16);
			t.yaw_l (m_mainYaw);

			const Vector k = t.getLocalFrameK_p ();
			Vector i = Vector::unitZ;
			Vector j = k.cross (i);
			i = j.cross(k);

			t.setLocalFrameIJK_p (i, j, k);
			t.reorthonormalize ();
			m_mainLight->setTransform_o2p (t);

			//-- fill is opposite of main, but rotates at a pitch of 30 degrees
			m_fillYaw   = -PI_OVER_2 + timeRatio * PI_TIMES_2 + PI + PI_OVER_4;
			m_fillLight->resetRotate_o2p ();
			m_fillLight->yaw_o (m_fillYaw);
			m_fillLight->pitch_o (-PI_OVER_6);

			//-- bounce is at half the yaw of main and fill and at a pitch of -60 degrees
			m_bounceYaw = -PI_OVER_2 + timeRatio * PI_TIMES_2 + PI_OVER_2 + PI_OVER_8;
			m_bounceLight->resetRotate_o2p ();
			m_bounceLight->yaw_o (m_bounceYaw);
			m_bounceLight->pitch_o (-PI_OVER_3);

#ifdef _DEBUG
			if (ms_debugReport)
			{
				int hour   = 6 + static_cast<int> (timeRatio * 24.f * 60.f) / 60;
				if (hour >= 24)
					hour -= 24;
				int minute = static_cast<int> (fmodf (timeRatio * 24.f * 60.f, 60.f));
				UNREF (minute);

				DEBUG_REPORT_PRINT (true, ("time:  %02i:%02i\n\n", hour, minute));

				DEBUG_REPORT_PRINT (true, ("Q      quit\n"));
				DEBUG_REPORT_PRINT (true, ("SPACE  stop animation\n"));
				DEBUG_REPORT_PRINT (true, ("LMB    rotate camera, wheel zooms\n"));
				DEBUG_REPORT_PRINT (true, ("RMB    rotate light, wheel zooms\n"));
			}
#endif

			ShadowVolume::setDirectionToLight (-m_light->getObjectFrameK_w ());
		}
		break;
	}

	return result;
}

//-------------------------------------------------------------------

static void renderLight (Object* const object)
{
	Transform t = object->getTransform_o2w ();
	t.move_l (Vector::unitZ * -3.f);
	t.yaw_l (PI);
	
	Graphics::setStaticShader (ShaderTemplateList::get3dVertexColorZStaticShader ());
	Graphics::setObjectToWorldTransformAndScale (t, Vector::xyz111);
	Graphics::drawFrame ();
}

//-------------------------------------------------------------------

void TimeOfDayTest::draw () const
{
	TestIoWin::draw ();

	renderLight (m_mainLight);
	renderLight (m_fillLight);
	renderLight (m_bounceLight);

	//-- render alpha polygon over screen
	ShadowVolume::renderShadowAlpha (m_camera);
}

//===================================================================

void TimeOfDayTest::createFloor ()
{
}

//===================================================================

