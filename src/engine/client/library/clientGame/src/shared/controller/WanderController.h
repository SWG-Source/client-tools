//===================================================================
//
// WanderController.h
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_WanderController_H
#define INCLUDED_WanderController_H

//===================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Timer.h"
#include "sharedObject/Controller.h"
#include "sharedMath/Vector.h"

//===================================================================

class WanderController : public Controller
{
public:

	static void install ();

public:
	
	WanderController (Object* owner);
	virtual ~WanderController ();

protected:

	virtual float realAlter (float elapsedTime);

private:

	Timer  m_timer;
	float  m_speed;
	Vector m_goalPosition_w;

private:

	WanderController ();
	WanderController (const WanderController&);
	WanderController& operator= (const WanderController&);
};

//===================================================================

#endif
