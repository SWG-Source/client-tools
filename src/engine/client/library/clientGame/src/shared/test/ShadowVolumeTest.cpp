//===================================================================
//
// ShadowVolumeTest.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ShadowVolumeTest.h"

#include "clientGraphics/Light.h"
#include "clientObject/ObjectListCamera.h"
#include "clientObject/ShadowManager.h"
#include "clientObject/ShadowVolume.h"

//===================================================================

namespace
{
	bool ms_debugReport = true;
}

//===================================================================

ShadowVolumeTest::ShadowVolumeTest () : 
	TestIoWin ("ShadowVolumeTest")
{
	ShadowManager::setEnabled (true);
	ShadowManager::setMeshShadowsVolumetric (true);
	ShadowManager::setSkeletalShadowsVolumetric (true);
}

//-------------------------------------------------------------------

ShadowVolumeTest::~ShadowVolumeTest ()
{
}

//-------------------------------------------------------------------

IoResult ShadowVolumeTest::processEvent (IoEvent* event)
{
	const IoResult result = TestIoWin::processEvent (event);

	switch (event->type)
	{
	case IOET_WindowKill:
		return IOR_PassKillMe;

	case IOET_Update:
		{
			if (ms_debugReport)
			{
				DEBUG_REPORT_PRINT (true, ("camera theta: %1.2f degrees\n", convertRadiansToDegrees (m_camera->getObjectFrameK_w ().theta ())));
				DEBUG_REPORT_PRINT (true, ("camera phi:   %1.2f degrees\n", convertRadiansToDegrees (m_camera->getObjectFrameK_w ().phi ())));
				DEBUG_REPORT_PRINT (true, ("camera zoom:  %1.2f\n", m_cameraZoom));
				DEBUG_REPORT_PRINT (true, ("light theta:  %1.2f degrees\n", convertRadiansToDegrees (m_light->getObjectFrameK_w ().theta ())));
				DEBUG_REPORT_PRINT (true, ("light phi:    %1.2f degrees\n", convertRadiansToDegrees (m_light->getObjectFrameK_w ().phi ())));
				DEBUG_REPORT_PRINT (true, ("light zoom:   %1.2f\n", m_lightZoom));

				DEBUG_REPORT_PRINT (true, ("Q      quit\n"));
				DEBUG_REPORT_PRINT (true, ("SPACE  stop animation\n"));
				DEBUG_REPORT_PRINT (true, ("LMB    rotate camera, wheel zooms\n"));
				DEBUG_REPORT_PRINT (true, ("RMB    rotate light, wheel zooms\n"));
			}

			ShadowVolume::setDirectionToLight (-m_light->getObjectFrameK_w ());
		}
		break;
	}

	return result;
}

//-------------------------------------------------------------------

void ShadowVolumeTest::draw () const
{
	TestIoWin::draw ();

	//-- render alpha polygon over screen
	ShadowVolume::renderShadowAlpha (m_camera);
}

//===================================================================

