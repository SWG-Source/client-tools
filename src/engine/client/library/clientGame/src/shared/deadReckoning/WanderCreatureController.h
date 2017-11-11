//===================================================================
//
// WanderCreatureController.h
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_WanderCreatureController_H
#define INCLUDED_WanderCreatureController_H

//===================================================================

#include "clientGame/RemoteCreatureController.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Timer.h"

//===================================================================

class WanderCreatureController : public RemoteCreatureController
{
public:
	
	WanderCreatureController (CreatureObject* owner);
	virtual ~WanderCreatureController ();

protected:

	virtual float realAlter (float elapsedTime);

private:

	Timer m_timer;

private:

	WanderCreatureController ();
	WanderCreatureController (const WanderCreatureController&);
	WanderCreatureController& operator= (const WanderCreatureController&);
};

//===================================================================

#endif
