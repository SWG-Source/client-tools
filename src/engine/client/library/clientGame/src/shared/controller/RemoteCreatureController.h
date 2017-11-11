//===================================================================
//
// RemoteCreatureController.h
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_RemoteCreatureController_H
#define INCLUDED_RemoteCreatureController_H

//===================================================================

#include "clientGame/CreatureController.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Timer.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"

class AlignToHardpointAction;
class CellProperty;
class CreatureObject;

//===================================================================

class RemoteCreatureController : public CreatureController
{
	friend class AlignToHardpointAction;

public:

	static void install ();

public:

	explicit RemoteCreatureController (CreatureObject* owner);
	virtual ~RemoteCreatureController ();

	virtual float       getCurrentSpeed () const;
	virtual void        endBaselines ();
	virtual void        pauseDeadReckoning (float pauseTime);
	virtual bool        shouldApplyAnimationDrivenLocomotion () const;
	virtual float       getDesiredSpeed () const;

	void                updateDeadReckoningModel (Transform const & transform_w);
	void                updateDeadReckoningModel (const CellProperty* cell, const Transform& transform_p, float speed);
	void                updateDeadReckoningModel (const CellProperty* cell, const Transform& transform_p, float speed, float lookAtYaw);

protected:

	virtual float       realAlter (float elapsedTime);

private:

	//-- information received from server
	int32               m_serverSequenceNumber;
	ConstWatcher<Object> m_serverCellObject;
	Transform           m_serverTransform_p;
	float               m_serverSpeed;

	//-- what dead reckoning will compute
	float               m_currentSpeed;
	float               m_currentAnimationSpeed;

	bool                m_pauseDeadReckoning;
	Timer               m_pauseDeadReckoningTimer;

	Vector              m_lastPosition_w;
	float               m_distanceToMoveLastFrame;

	float               m_stuckWarpTimer;
	float               m_desiredSpeed;

	float               m_serverLookAtYaw;

#ifdef _DEBUG
	typedef stdvector<Vector>::fwd VectorList;
	VectorList*         m_clientPath;
	VectorList*         m_serverPath;
#endif

private:

	static void remove ();

private:

	void         warpTransform (const Object* cellObject, const Transform& transform_p);

	virtual void handleNetUpdateTransformWithParent (const MessageQueueDataTransformWithParent& message);
	virtual void handleNetUpdateTransform (const MessageQueueDataTransform& message);

#ifdef _DEBUG
	void _reportTransforms();
#endif

private:

	RemoteCreatureController ();
	RemoteCreatureController (const RemoteCreatureController&);
	RemoteCreatureController& operator= (const RemoteCreatureController&);
};

//===================================================================

#endif
