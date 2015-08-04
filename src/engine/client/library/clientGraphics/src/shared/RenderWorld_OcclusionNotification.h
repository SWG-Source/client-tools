// PRIVATE ==============================================================
//
// RenderWorld_OcclusionNotification.h
// Copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_RenderWorld_OcclusionNotification_H
#define INCLUDED_RenderWorld_OcclusionNotification_H

// ======================================================================

#include "sharedObject/ObjectNotification.h"
#include "clientGraphics/RenderWorld.h"

// ======================================================================

class RenderWorld::OcclusionNotification : public ObjectNotification
{
public:
	OcclusionNotification();
	virtual ~OcclusionNotification();

	virtual int  getPriority() const;

	virtual void addToWorld(Object &object) const;
	virtual void removeFromWorld(Object &object) const;

	virtual bool positionChanged(Object &object, bool dueToParentChange, const Vector& oldPosition) const;
	virtual void rotationChanged(Object &object, bool dueToParentChange) const;
	virtual bool positionAndRotationChanged(Object &object, bool dueToParentChange, const Vector& oldPosition) const;
	virtual void cellChanged(Object &object, bool dueToParentChange) const;
	virtual void extentChanged(Object &object) const;

private:

	void  transformChanged(Object &object) const;

private:

	OcclusionNotification(const OcclusionNotification &);
	OcclusionNotification &operator =(const OcclusionNotification &);
};

// ======================================================================

#endif
