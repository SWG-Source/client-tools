//===================================================================
//
// Countermeasure.cpp
//
// Copyright 2004 Sony Online Entertainment
//
//===================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/Countermeasure.h"

#include "clientGraphics/RenderWorld.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/MissileManager.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedRandom/Random.h"

//===================================================================

namespace Countermeasure_namespace
{
	float const ms_expirationTime = 10.0f; // how long a countermeasure lasts
}

using namespace Countermeasure_namespace;

//===================================================================

Countermeasure::Countermeasure(Vector const & sourceLocation, Vector const & initialDirection, AppearanceTemplate const * const appearance, float const velocity, float const maxScatterAngle) :
	Object(),
	m_expirationTimer(ms_expirationTime),
	m_velocity(velocity)
{
	setPosition_w(sourceLocation);
	lookAt_o(initialDirection);

	// Add some randomness
	yaw_o(Random::randomReal(-maxScatterAngle, maxScatterAngle));
	pitch_o(Random::randomReal(-maxScatterAngle, maxScatterAngle));

	if (appearance)
		setAppearance(appearance->createAppearance());

	RenderWorld::addObjectNotifications(*this);
	addNotification(ClientWorld::getIntangibleNotification());
}

// ----------------------------------------------------------------------

Countermeasure::~Countermeasure()
{
	MissileManager::removeCountermeasure(this);
}

// ----------------------------------------------------------------------

float Countermeasure::alter(float elapsedTime)
{
	const float desiredMove = m_velocity * elapsedTime;
	move_o(Vector::unitZ * desiredMove);

	if (m_expirationTimer.updateZero(elapsedTime))
	{
		kill();
	}

	float alterResult = AlterResult::cms_alterNextFrame;
	AlterResult::incorporateAlterResult(alterResult, Object::alter(elapsedTime));
	return alterResult;
}

//===================================================================
