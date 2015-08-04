//===================================================================
//
// WanderController.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/WanderController.h"

#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/CellProperty.h"
#include "sharedRandom/Random.h"
#include "sharedTerrain/TerrainObject.h"

//===================================================================

namespace WanderControllerNamespace
{
	float cms_minimumYaw;
	float cms_maximumYaw;
	float cms_minimumSpeed;
	float cms_maximumSpeed;
	float cms_minimumTime;
	float cms_maximumTime;

	bool ms_pauseWanderController;

	void remove ();
}

using namespace WanderControllerNamespace;

//===================================================================
// PUBLIC STATIC WanderController
//===================================================================

void WanderController::install ()
{
	InstallTimer const installTimer("WanderController::install");

	DebugFlags::registerFlag (ms_pauseWanderController, "ClientGame", "pauseWanderController");
	ExitChain::add (remove, "WanderControllerNamespace::remove");

	cms_minimumYaw = convertDegreesToRadians (ConfigFile::getKeyFloat ("WanderController", "minimumYaw", -90.f));
	cms_maximumYaw = convertDegreesToRadians (ConfigFile::getKeyFloat ("WanderController", "maximumYaw", 90.f));
	cms_minimumSpeed = ConfigFile::getKeyFloat ("WanderController", "minimumSpeed", 2.f);
	cms_maximumSpeed = ConfigFile::getKeyFloat ("WanderController", "maximumSpeed", 5.f);
	cms_minimumTime = ConfigFile::getKeyFloat ("WanderController", "minimumTime", 3.f);
	cms_maximumTime = ConfigFile::getKeyFloat ("WanderController", "maximumTime", 5.f);
}

//===================================================================
// PUBLIC WanderController
//===================================================================

WanderController::WanderController (Object* const owner) :
	Controller (owner),
	m_timer (0.f),
	m_speed (0.f),
	m_goalPosition_w ()
{
}

//-------------------------------------------------------------------

WanderController::~WanderController ()
{
}

//===================================================================
// STATIC PRIVATE WanderController
//===================================================================

void WanderControllerNamespace::remove ()
{
	DebugFlags::unregisterFlag (ms_pauseWanderController);
}

//===================================================================
// PRIVATE WanderController
//===================================================================

float WanderController::realAlter (float elapsedTime)
{
	Controller::realAlter (elapsedTime);

	//-- face a new heading
	if (!ms_pauseWanderController)
	{
		if (m_timer.updateZero (elapsedTime))
		{
			getOwner ()->yaw_o (Random::randomReal (cms_minimumYaw, cms_maximumYaw));
			m_speed = Random::randomReal (cms_minimumSpeed, cms_maximumSpeed);
			m_timer.setExpireTime (Random::randomReal (cms_minimumTime, cms_maximumTime));
		}

		getOwner ()->move_o (Vector::unitZ * m_speed * elapsedTime);
	}

	//-- snap to the terrain
	if (getOwner ()->getParentCell () == CellProperty::getWorldCellProperty ())
	{
		Vector position_w = getOwner ()->getPosition_w ();
		const TerrainObject* const terrainObject = TerrainObject::getConstInstance ();
		if (terrainObject && terrainObject->getHeight (position_w, position_w.y))
			getOwner ()->setPosition_w (position_w);
	}

	// @todo figure out what this should really return.
	return AlterResult::cms_alterNextFrame;
}

//===================================================================

