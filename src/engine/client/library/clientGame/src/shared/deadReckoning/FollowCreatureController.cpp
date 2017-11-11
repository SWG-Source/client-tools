//===================================================================
//
// FollowCreatureController.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/FollowCreatureController.h"

#include "clientGame/CreatureObject.h"
#include "sharedObject/AlterResult.h"
#include "sharedRandom/Random.h"

//===================================================================
// PUBLIC FollowCreatureController
//===================================================================

FollowCreatureController::FollowCreatureController (CreatureObject* owner, const CreatureObject* target) :
	RemoteCreatureController (owner),
	m_target (target),
	m_timer (0.25f)
{
}

//-------------------------------------------------------------------

FollowCreatureController::~FollowCreatureController ()
{
}

//-------------------------------------------------------------------

float FollowCreatureController::realAlter (float elapsedTime)
{
	if (m_target && m_timer.updateZero (elapsedTime))
	{
		m_timer.setExpireTime (Random::randomReal (0.20f, 0.55f));
		m_timer.reset ();

		const ClientController* const clientController = safe_cast<const ClientController*> (m_target->getController ());
		if (clientController)
		{
			const CellProperty* cell        = m_target->getParentCell ();
			const Transform&    transform_p = m_target->getTransform_o2p ();
			const float         speed       = clientController->getCurrentSpeed ();

			updateDeadReckoningModel (cell, transform_p, speed);
		}
	}

	float const baseAlterResult = RemoteCreatureController::realAlter (elapsedTime);
	UNREF (baseAlterResult);

	return AlterResult::cms_alterNextFrame;
}

//===================================================================

