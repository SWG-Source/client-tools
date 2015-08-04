//===================================================================
//
// WanderCreatureController.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/WanderCreatureController.h"

#include "clientGame/CreatureObject.h"
#include "sharedGame/SharedTangibleObjectTemplate.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/CellProperty.h"
#include "sharedTerrain/TerrainObject.h"

//===================================================================
// PUBLIC WanderCreatureController
//===================================================================

WanderCreatureController::WanderCreatureController (CreatureObject* const owner) :
	RemoteCreatureController (owner),
	m_timer (3.f)
{
	updateDeadReckoningModel (owner->getParentCell (), owner->getTransform_o2p (), 0.f);
}

//-------------------------------------------------------------------

WanderCreatureController::~WanderCreatureController ()
{
}

//-------------------------------------------------------------------

float WanderCreatureController::realAlter (float elapsedTime)
{
	if (m_timer.updateZero (elapsedTime))
	{
		m_timer.reset ();

		const CellProperty* const cell = CellProperty::getWorldCellProperty ();
		Transform transform_w = getOwner ()->getTransform_o2w ();
		transform_w.yaw_l (Random::randomReal (PI_OVER_2) - PI_OVER_4);
		transform_w.move_l (Vector::unitZ * (Random::randomReal (2.f) + 2.f));

		Vector position_w = transform_w.getPosition_p ();
		const TerrainObject* const terrainObject = TerrainObject::getConstInstance ();
		if (terrainObject && terrainObject->getHeight (transform_w.getPosition_p (), position_w.y))
			transform_w.setPosition_p (position_w);

		updateDeadReckoningModel (cell, transform_w, 0.f);
	}

	float const baseAlterResult = RemoteCreatureController::realAlter (elapsedTime);
	UNREF (baseAlterResult);

	return AlterResult::cms_alterNextFrame;
}

//===================================================================

