// ======================================================================
//
// EnvironmentMapGenerator.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/EnvironmentMapGenerator.h"

#include "clientGame/ClientWorld.h"
#include "clientGraphics/Graphics.h"
#include "clientObject/SaveCameraParameters.h"
#include "clientObject/ShadowVolume.h"
#include "sharedIoWin/IoWinManager.h"
#include "sharedObject/CellProperty.h"
#include "sharedTerrain/TerrainObject.h"

#include <cstdio>
#include <dinput.h>

// ======================================================================
// EnvironmentMapGeneratorNamespace
// ======================================================================

namespace EnvironmentMapGeneratorNamespace
{
	float const cms_yaw [6] =
	{
		0.f,
		PI_OVER_2,
		PI,
		-PI_OVER_2,
		0.f,
		0.f
	};

	float const cms_pitch [6] =
	{
		0.f,
		0.f,
		0.f,
		0.f,
		-PI_OVER_2,
		PI_OVER_2
	};

	char const * const cms_names [6] =
	{
		"front",
		"right",
		"back",
		"left",
		"up",
		"down"
	};
}

using namespace EnvironmentMapGeneratorNamespace;

// ======================================================================
// PUBLIC EnvironmentMapGenerator
// ======================================================================

EnvironmentMapGenerator::EnvironmentMapGenerator (GameCamera const * const sourceCamera, Object const * const excludedObject) :
	IoWin ("EnvironmentMapGenerator"),
	m_camera (new GameCamera ()),
	m_state (0)
{
	SaveCameraParameters s;
	s.save (sourceCamera);
	s.restore (m_camera);

	m_camera->setViewport (0, 0, 256, 256);
	m_camera->setHorizontalFieldOfView (PI_OVER_2);
	if (excludedObject)
		m_camera->addExcludedObject (excludedObject);
	m_camera->setParentCell (const_cast<CellProperty *> (sourceCamera->getParentCell ()));
	CellProperty::setPortalTransitionsEnabled (false);
		m_camera->setTransform_o2p (sourceCamera->getTransform_o2p ());
	CellProperty::setPortalTransitionsEnabled (true);

	ClientWorld::addCamera (m_camera);

	open ();
}

// ----------------------------------------------------------------------

EnvironmentMapGenerator::~EnvironmentMapGenerator ()
{
	ClientWorld::removeCamera (m_camera);

	delete m_camera;

	IoWinManager::queueInputReset();
	IoWinManager::discardUserInputUntilNextProcessEvents();
}

// ----------------------------------------------------------------------

IoResult EnvironmentMapGenerator::processEvent (IoEvent * const event)
{
	static int currentScreenShot = 0;

	switch (event->type)
	{
	case IOET_WindowKill:
		return IOR_PassKillMe;

	case IOET_KeyUp:
		{
			if (event->arg2 == DIK_ESCAPE)
				return IOR_Block;
		}
		break;

	case IOET_Update:
		{
			if (m_state % 2 == 0)
			{
				m_camera->resetRotate_o2p ();
				m_camera->yaw_o (cms_yaw [m_state / 2]);
				m_camera->pitch_o (cms_pitch [m_state / 2]);
			}
			else
			{
				//-- screenshot
				char buffer [256];
				sprintf(buffer, "env_%02d_%s", currentScreenShot, cms_names [(m_state - 1) / 2]);
				Graphics::screenShot (buffer);
			}

			if (++m_state >= 6 * 2)
			{
				++currentScreenShot;
				return IOR_BlockKillMe;
			}
		}
		break;

	default:
		break;
	}

	return IOR_Block;
}

// ----------------------------------------------------------------------

void EnvironmentMapGenerator::draw () const
{
	PackedRgb const backgroundColor = TerrainObject::getInstance () ? TerrainObject::getInstance ()->getClearColor () : PackedRgb::solidBlack;

	Graphics::clearViewport (true, backgroundColor.asUint32 (), true, 1.f, true, 0);
	ClientWorld::addRenderHookFunctions (m_camera);
	m_camera->renderScene ();
	ClientWorld::removeRenderHookFunctions ();

	//-- render shadow alpha
	ShadowVolume::renderShadowAlpha (m_camera);
}

// ======================================================================
