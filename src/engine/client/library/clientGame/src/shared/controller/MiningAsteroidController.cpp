// ======================================================================
//
// MiningAsteroidController.cpp
// jwatson
//
// copyright 2005, sony online entertainment
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/MiningAsteroidController.h"

#include "clientGame/ShipObject.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedObject/AlterResult.h"

//----------------------------------------------------------------------

MiningAsteroidController::MiningAsteroidController(ShipObject * const owner) :
RemoteShipController(owner)
{
	
}

//----------------------------------------------------------------------

MiningAsteroidController::~MiningAsteroidController()
{
}

//----------------------------------------------------------------------

float MiningAsteroidController::realAlter(float elapsedTime)
{
	/*
	ShipObject * const shipOwner = NON_NULL(safe_cast<ShipObject *>(getOwner()));

	int const maxHitpoints = shipOwner->getMaxHitPoints();
	int const curHitpoints = maxHitpoints - shipOwner->getDamageTaken();

	if (maxHitpoints > 0)
	{
		float const scaleFactor = 0.5f + ((0.5f * curHitpoints) / maxHitpoints);
		shipOwner->setScale(Vector::xyz111 * scaleFactor);
	}
*/
	float alterResult = RemoteShipController::realAlter(elapsedTime);
	AlterResult::incorporateAlterResult(alterResult, AlterResult::cms_alterNextFrame);
	return alterResult;
}

//----------------------------------------------------------------------



