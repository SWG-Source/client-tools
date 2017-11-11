//
// ScreenShot360.cpp
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#include "clientGame/FirstClientGame.h"
#include "clientGame/ScreenShot360.h"

#include "clientGame/Bloom.h"
#include "clientGame/ClientWorld.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientObject/SaveCameraParameters.h"
#include "clientObject/ShadowVolume.h"
#include "clientTerrain/ClientTerrainSorter.h"
#include "sharedIoWin/IoWinManager.h"
#include "sharedObject/CellProperty.h"
#include "sharedTerrain/TerrainObject.h"

#include <cstdio>
#include <dinput.h>

//-------------------------------------------------------------------

ScreenShot360::ScreenShot360 (const GameCamera* sourceCamera, const CellProperty* const cellProperty, const Transform& newTransform, const Vector& newOffset, real newPitch, real newYaw, real newZoom, real newRadius) :
	IoWin ("ScreenShot360"),
	camera (0),
	state (0),
	transform (newTransform),
	offset (newOffset),
	pitch (newPitch),
	yaw (newYaw),
	zoom (newZoom),
	radius (newRadius)
{
	camera = new GameCamera ();
	camera->setParentCell (const_cast<CellProperty*> (cellProperty));

	SaveCameraParameters s;
	s.save (sourceCamera);
	s.restore (camera);

	//--
	ClientWorld::addCamera (camera);

	//--
	open ();
}

//-------------------------------------------------------------------

ScreenShot360::~ScreenShot360 (void)
{
	//--
	ClientWorld::removeCamera (camera);

	delete camera;
	camera = 0;

	IoWinManager::queueInputReset();
	IoWinManager::discardUserInputUntilNextProcessEvents();
}

//-------------------------------------------------------------------

IoResult ScreenShot360::processEvent (IoEvent* event)
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
			if (state > 36 * 2)
			{
				++currentScreenShot;
				return IOR_BlockKillMe;
			}

			if (state % 2 == 0)
			{
				camera->setTransform_o2p (transform);
				camera->move_o (offset);
				camera->yaw_o (yaw);
				camera->pitch_o (pitch);
				camera->move_o (Vector::negativeUnitZ * zoom * radius);

				yaw += convertDegreesToRadians (10.f);

				++state;
			}
			else
			{
				//-- screenshot
				char buffer [256];
				sprintf(buffer, "screenShot360_%04d_%02d.tga", currentScreenShot, (state - 1) / 2);
				Graphics::screenShot (buffer);

				++state;
			}
		}
		break;

	default:
		break;
	}

	return IOR_Block;
}

//-------------------------------------------------------------------

void ScreenShot360::draw (void) const
{
	PackedRgb backgroundColor = PackedRgb::solidBlack;

	TerrainObject* terrainObject = TerrainObject::getInstance();

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

