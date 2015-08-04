//
// ListObject.cpp
// asommers 6-11-2000
//
// copyright 2000, verant interactive, inc.
//

//-------------------------------------------------------------------

#include "clientGame/FirstClientGame.h"
#include "clientGame/ListObject.h"

#include "sharedRandom/RandomGenerator.h"

//-------------------------------------------------------------------

ListObject::ListObject (real newYawRotation, real newPitchRotation, real newRollRotation) : 
	Object(),
	yawRotation (newYawRotation),
	pitchRotation (newPitchRotation),
	rollRotation (newRollRotation)
{
}

//-------------------------------------------------------------------
	
ListObject::~ListObject (void)
{
}

//-------------------------------------------------------------------

void ListObject::assignRandomRotations (RandomGenerator* randomGenerator)
{
	yawRotation   = randomGenerator->randomReal (PI_OVER_4 * CONST_REAL (0.5));
	pitchRotation = randomGenerator->randomReal (PI_OVER_4 * CONST_REAL (0.5));
	rollRotation  = randomGenerator->randomReal (PI_OVER_4 * CONST_REAL (0.5));

	yaw_o   (yawRotation);
	pitch_o (pitchRotation);
	roll_o  (rollRotation);
}

//-------------------------------------------------------------------

float ListObject::alter (float time)
{
	yaw_o   (yawRotation   * time);
	pitch_o (pitchRotation * time);
	roll_o  (rollRotation  * time);

	return Object::alter (time);
}

//-------------------------------------------------------------------

