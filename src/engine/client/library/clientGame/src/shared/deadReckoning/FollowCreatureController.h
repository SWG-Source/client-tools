//===================================================================
//
// FollowCreatureController.h
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_FollowCreatureController_H
#define INCLUDED_FollowCreatureController_H

//===================================================================

#include "clientGame/RemoteCreatureController.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Timer.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"

//===================================================================

class FollowCreatureController : public RemoteCreatureController
{
public:
	
	FollowCreatureController (CreatureObject* owner, const CreatureObject* target);
	virtual ~FollowCreatureController ();

protected:

	virtual float realAlter (float elapsedTime);

private:

	ConstWatcher<CreatureObject> m_target;
	Timer                        m_timer;

private:

	FollowCreatureController ();
	FollowCreatureController (const FollowCreatureController&);
	FollowCreatureController& operator= (const FollowCreatureController&);
};

//===================================================================

#endif
