//===================================================================
//
// ClientClearCollidableFloraNotification.h
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_ClientClearCollidableFloraNotification_H
#define INCLUDED_ClientClearCollidableFloraNotification_H

//===================================================================

#include "sharedObject/ObjectNotification.h"

//===================================================================

class ClientClearCollidableFloraNotification : public ObjectNotification
{
public:

	static ClientClearCollidableFloraNotification& getInstance ();

public:

	ClientClearCollidableFloraNotification ();
	virtual ~ClientClearCollidableFloraNotification ();

	virtual void addToWorld (Object& object) const;
	virtual void removeFromWorld (Object& object) const;

	virtual bool positionChanged (Object& object, bool dueToParentChange, const Vector& oldPosition) const;
	virtual bool positionAndRotationChanged (Object& object, bool dueToParentChange, const Vector& oldPosition) const;

private:

	ClientClearCollidableFloraNotification (const ClientClearCollidableFloraNotification&);
	ClientClearCollidableFloraNotification& operator= (const ClientClearCollidableFloraNotification&);

private:

	static ClientClearCollidableFloraNotification ms_instance;
};

//===================================================================

#endif
