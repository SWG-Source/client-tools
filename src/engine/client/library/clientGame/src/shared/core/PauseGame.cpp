//
// PauseGame.cpp
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#include "clientGame/FirstClientGame.h"
#include "clientGame/PauseGame.h"

#include "clientAudio/Audio.h"
#include "clientGame/ClientWorld.h"
#include "clientObject/GameCamera.h"
#include "clientGame/Panorama.h"
#include "clientGraphics/Graphics.h"
#include "clientObject/SaveCameraParameters.h"
#include "clientObject/ShadowVolume.h"
#include "clientTerrain/ClientProceduralTerrainAppearance.h"
#include "sharedMath/Rectangle2d.h"
#include "sharedTerrain/TerrainObject.h"
#include "sharedObject/CellProperty.h"

#include <dinput.h>

//-------------------------------------------------------------------

PauseGame::PauseGame (const GameCamera* sourceCamera, bool newFreeMode) :
	IoWin ("PauseGame"),
	camera (0),
	yaw (0),
	pitch (0),
	mouseCursor (0, MouseCursor::S_relative),
	advanceFrame (false),
	freeMode (newFreeMode)
{
	const Transform& transform = sourceCamera->getTransform_o2p ();

	camera = new GameCamera ();
	camera->setActive (true);
	camera->setParentCell (const_cast<CellProperty*> (sourceCamera->getParentCell ()));

	CellProperty::setPortalTransitionsEnabled(false);
		camera->setPosition_p (transform.getPosition_p ());
	CellProperty::setPortalTransitionsEnabled(true);

	yaw    = transform.getLocalFrameK_p ().theta ();
	pitch  = transform.getLocalFrameK_p ().phi ();
	originalMousePosition.x = static_cast<real> (mouseCursor.getX ());
	originalMousePosition.y = static_cast<real> (mouseCursor.getY ());

	SaveCameraParameters s;
	s.save (sourceCamera);
	s.restore (camera);

	//--
	ClientWorld::addCamera (camera);

	int i;
	for (i = 0; i < K_COUNT; ++i)
		keys [i] = false;

	//-- 
	open ();
}

//-------------------------------------------------------------------

PauseGame::~PauseGame (void)
{
	//--
	ClientWorld::removeCamera (camera);

	delete camera;
	camera = 0;
}

//-------------------------------------------------------------------

IoResult PauseGame::processEvent (IoEvent* event)
{
	//-- handle the cursor
	mouseCursor.processEvent (event);

	switch (event->type)
	{
	case IOET_WindowKill:
		return IOR_PassKillMe;

	case IOET_KeyDown:
		{
			switch (event->arg2)
			{
			case DIK_UP:
				{
					keys [K_up] = true;
				}
				break;

			case DIK_DOWN:
				{
					keys [K_down] = true;
				}
				break;

			case DIK_LEFT:
				{
					keys [K_left] = true;
				}
				break;

			case DIK_RIGHT:
				{
					keys [K_right] = true;
				}
				break;

			case DIK_LSHIFT:
			case DIK_RSHIFT:
				{
					keys [K_button] = true;
				}
				break;

			case DIK_Q:
				{
					keys [K_rollLeft] = true;
				}
				break;

			case DIK_W:
				{
					keys [K_rollRight] = true;
				}
				break;
			}
	
			if (event->arg2 == DIK_SCROLL)
				new Panorama (camera, !freeMode);
		}
		break;

	case IOET_KeyUp:
		{
			switch (event->arg2)
			{
			case DIK_UP:
				{
					keys [K_up] = false;
				}
				break;

			case DIK_DOWN:
				{
					keys [K_down] = false;
				}
				break;

			case DIK_LEFT:
				{
					keys [K_left] = false;
				}
				break;

			case DIK_RIGHT:
				{
					keys [K_right] = false;
				}
				break;

			case DIK_LSHIFT:
			case DIK_RSHIFT:
				{
					keys [K_button] = false;
				}
				break;

			case DIK_Q:
				{
					keys [K_rollLeft] = false;
				}
				break;

			case DIK_W:
				{
					keys [K_rollRight] = false;
				}
				break;
			}

			if (event->arg2 == DIK_ESCAPE)
			{
				event->type = IOET_InputReset;

				return IOR_PassKillMe;
			}

			if (event->arg2 == DIK_SPACE)
				advanceFrame = true;
		}
		break;

	case IOET_Update:
		{
			const real elapsedTime = event->arg3;

			Rectangle2d rect;
			camera->getViewport (rect.x0, rect.y0, rect.x1, rect.y1);

			Vector2d delta(static_cast<real> (mouseCursor.getX ()), static_cast<real> (mouseCursor.getY ()));

			const real yawMod   = PI * delta.x  / rect.getWidth ();
			const real pitchMod = PI * delta.y  / rect.getHeight ();
			const real rollMod  = keys [K_rollLeft] ? -elapsedTime : (keys [K_rollRight] ? elapsedTime : 0);

			yaw   += yawMod;
			pitch += pitchMod;

			if (!freeMode)
				pitch = clamp (-PI_OVER_2, pitch, PI_OVER_2);

			const real amount = elapsedTime * (keys [K_button] ? CONST_REAL (30) : CONST_REAL (7));

			int i;
			for (i = 0; i < K_COUNT; ++i)
			{
				if (keys [i])
					switch (i)
					{
					case K_up:     camera->move_o ( Vector::unitZ * amount);  break;
					case K_down:   camera->move_o (-Vector::unitZ * amount);  break;
					case K_left:   camera->move_o (-Vector::unitX * amount);  break;
					case K_right:  camera->move_o ( Vector::unitX * amount);  break;
					}
			}

			if (freeMode)
			{
				camera->yaw_o (yawMod);
				camera->pitch_o (pitchMod);
				camera->roll_o (rollMod);
			}
			else
			{
				camera->resetRotate_o2p ();
				camera->yaw_o (yaw);
				camera->pitch_o (pitch);
			}

			camera->alter (elapsedTime);

			//Audio::alter(elapsedTime, camera);

			if (advanceFrame)
			{
				ClientWorld::updateIfYouKnowWhatYoureDoing (elapsedTime);
				advanceFrame = false;
			}
		}
		break;

	default:
		break;
	}

	return IOR_Block;
}

//-------------------------------------------------------------------

void PauseGame::draw (void) const
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

	Graphics::clearViewport(true, backgroundColor.asUint32(), true, 1.0f, true, 0);
	ClientWorld::addRenderHookFunctions(camera);
	camera->renderScene ();
	ClientWorld::removeRenderHookFunctions();

	//-- render shadow alpha
	ShadowVolume::renderShadowAlpha (camera);
}

//-------------------------------------------------------------------

