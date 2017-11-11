// ======================================================================
//
// ClientController.h
// Copyright 2001, 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ClientController_H
#define INCLUDED_ClientController_H

// ======================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Timer.h"
#include "sharedMath/Transform.h"
#include "sharedObject/NetworkController.h"

// ======================================================================

class ClientCombatActionInfo;
class MessageQueueCombatAction;
class NetworkId;
class Object;

// ======================================================================
/**
 * The base class for client-side controllers.
 *
 */

class ClientController : public NetworkController
{
public:

	explicit ClientController (Object * newOwner);
	virtual ~ClientController ();

	virtual void endBaselines();
	virtual void          sendControllerMessage (const ObjControllerMessage& msg);
	virtual void          handleMessage         (int message, float value, const MessageQueue::Data* data, uint32 flags);
	virtual float         getCurrentSpeed       () const;

	bool                  isSettingBaselines    () const;
	void                  sendTransform         (const Transform& transform_p, const bool reliable = false);
	void                  sendTransformUsingParent(const Transform& transform_p, NetworkId const &parentId, const bool reliable = false);

	bool                  isOwnerAnimationDebuggerTarget() const;

	// Don't call these unless you know exactly what you're doing!
	void                  doClientHandleNetUpdateTransform (const MessageQueueDataTransform& message);
	void                  doClientHandleNetUpdateTransformWithParent (const MessageQueueDataTransformWithParent& message);

protected:

	virtual float         realAlter (float time);

	int                   getNextSequenceNumber ();

	virtual void          handleNetUpdateTransform (const MessageQueueDataTransform& message);
	virtual void          handleNetUpdateTransformWithParent (const MessageQueueDataTransformWithParent& message);

private:

	// Disabled.
	ClientController ();
	ClientController (const ClientController & other);
	ClientController& operator= (const ClientController & rhs);

private:

	bool m_initialized;
	int m_sequenceNumber;
};

//-----------------------------------------------------------------------

#endif
