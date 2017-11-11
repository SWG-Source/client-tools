//
// Panorama.cpp
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#include "clientGame/FirstClientGame.h"
#include "clientGame/Panorama.h"

#include "clientGame/Bloom.h"
#include "clientGame/ClientWorld.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientObject/SaveCameraParameters.h"
#include "clientObject/ShadowVolume.h"
#include "clientTerrain/ClientProceduralTerrainAppearance.h"
#include "sharedIoWin/IoWinManager.h"
#include "sharedTerrain/TerrainObject.h"
#include "sharedObject/CellProperty.h"

#include <dinput.h>
#include <cstdio>

//-------------------------------------------------------------------

Panorama::Panorama (const GameCamera* sourceCamera, bool align) :
	IoWin ("Panorama"),
	camera (0),
	state (0)
{
	const Transform& transform = sourceCamera->getTransform_o2c ();

	camera = new GameCamera ();
	camera->setParentCell(sourceCamera->getParentCell());

	CellProperty::setPortalTransitionsEnabled(false);

	if (align)
	{
		camera->setPosition_p (transform.getPosition_p ());
		camera->yaw_o (transform.getLocalFrameK_p ().theta ());
	}
	else
		camera->setTransform_o2p (transform);

	CellProperty::setPortalTransitionsEnabled(true);

	SaveCameraParameters s;
	s.save (sourceCamera);
	s.restore (camera);

	//--
	ClientWorld::addCamera (camera);

	//--
	open ();
}

//-------------------------------------------------------------------

Panorama::~Panorama (void)
{
	//--
	ClientWorld::removeCamera (camera);

	delete camera;
	camera = 0;

	IoWinManager::queueInputReset();
	IoWinManager::discardUserInputUntilNextProcessEvents();
}

//-------------------------------------------------------------------

IoResult Panorama::processEvent (IoEvent* event)
{
	static int currentScreenShot = 0;

	ClientProceduralTerrainAppearance::setReferenceCamera(camera);

	switch (event->type)
	{
	case IOET_WindowKill:
		return IOR_PassKillMe;

	case IOET_Update:
		{
			switch (state)
			{
			case 0:
				{
					camera->yaw_o (-camera->getHorizontalFieldOfView () * CONST_REAL (0.5));

					++state;
				}
				break;

			case 1:
				{
					//-- screenshot
					char buffer [256];
					sprintf(buffer, "swg_%04d_l.tga", currentScreenShot);
					Graphics::screenShot (buffer);

					++state;
				}
				break;

			case 2:
				{
					camera->yaw_o (camera->getHorizontalFieldOfView ());

					++state;
				}
				break;

			case 3:
				{
					//-- screenshot
					char buffer [256];
					sprintf(buffer, "swg_%04d_r.tga", currentScreenShot);
					Graphics::screenShot (buffer);

					++state;
				}
				break;

			default:
				{
					++currentScreenShot;
					return IOR_BlockKillMe;
				}
			}
		}
		break;

	default:
		break;
	}

	return IOR_Block;
}

//-------------------------------------------------------------------

void Panorama::draw (void) const
{
	PackedRgb backgroundColor = PackedRgb::solidBlack;

	TerrainObject* terrainObject = TerrainObject::getInstance ();

	//-- the terrain to be rendered
	if (terrainObject)
	{

		const ClientProceduralTerrainAppearance* cpta = dynamic_cast<const ClientProceduralTerrainAppearance*> (terrainObject->getAppearance ());
		if (cpta)
			backgroundColor = cpta->getClearColor ();
	}

	Bloom::preSceneRender();

		Graphics::clearViewport(true, backgroundColor.asUint32(), true, 1.0f, true, 0);
		ClientWorld::addRenderHookFunctions(camera);
			camera->renderScene ();
		ClientWorld::removeRenderHookFunctions();

		//-- render shadow alpha
		ShadowVolume::renderShadowAlpha (camera);

	Bloom::postSceneRender();
}

//-------------------------------------------------------------------

