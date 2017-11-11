// ============================================================================
// 
// AsteroidDynamics.cpp
// Copyright Sony Online Entertainment, Inc.
//
// ============================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/AsteroidDynamics.h"

#include "sharedObject/AlterResult.h"
#include "sharedObject/Object.h"

// ============================================================================

AsteroidDynamics::AsteroidDynamics (Object* newOwner, const Vector & velocity_w, const Vector& rotationsPerSecond) :
	SimpleDynamics(newOwner),
	m_rotationsPerSecond(rotationsPerSecond),
	m_ignoreFirstFrame(true)
{
	setCurrentVelocity_w(velocity_w);
}

//-----------------------------------------------------------------------------

AsteroidDynamics::~AsteroidDynamics ()
{
}

//-----------------------------------------------------------------------------

float AsteroidDynamics::alter(float time)
{
	if (m_ignoreFirstFrame)
	{
		// this dynamics is attached to objects that haven't altered since they were created, so ignore the "catchup" 
		// first alter (which has "time"=the time since the object was created), and only do work beginning the
		// following frame
		m_ignoreFirstFrame=false; 
	}
	else
	{
		Object* const owner = getOwner ();
		if (!owner)
			return AlterResult::cms_alterNextFrame;

		IGNORE_RETURN(SimpleDynamics::alter(time));

		owner->yaw_o   (m_rotationsPerSecond.x * time);
		owner->pitch_o (m_rotationsPerSecond.y * time);
		owner->roll_o  (m_rotationsPerSecond.z * time);
	}

	return AlterResult::cms_alterNextFrame;
}

// ============================================================================
