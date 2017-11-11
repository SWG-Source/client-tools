//===================================================================
//
// ClearNonCollidableFloraNotification.h
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_ClearNonCollidableFloraNotification_H
#define INCLUDED_ClearNonCollidableFloraNotification_H

//===================================================================

#include "sharedObject/ObjectNotification.h"

//===================================================================

class ClearNonCollidableFloraNotification : public ObjectNotification
{
public:

	static ClearNonCollidableFloraNotification& getInstance ();

public:

	ClearNonCollidableFloraNotification ();
	virtual ~ClearNonCollidableFloraNotification ();

	virtual void addToWorld (Object& object) const;
	virtual void removeFromWorld (Object& object) const;

	virtual bool positionChanged (Object& object, bool dueToParentChange, const Vector& oldPosition) const;
	virtual bool positionAndRotationChanged (Object& object, bool dueToParentChange, const Vector& oldPosition) const;

private:

	ClearNonCollidableFloraNotification (const ClearNonCollidableFloraNotification&);
	ClearNonCollidableFloraNotification& operator= (const ClearNonCollidableFloraNotification&);

private:

	static ClearNonCollidableFloraNotification ms_instance;
};

//===================================================================

#endif
