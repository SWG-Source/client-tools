// PRIVATE ==============================================================
//
// RenderWorld_CellNotification.h
// Copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_RenderWorld_CellNotification_H
#define INCLUDED_RenderWorld_CellNotification_H

// ======================================================================

#include "sharedObject/ObjectNotification.h"
#include "clientGraphics/RenderWorld.h"

// ======================================================================

class RenderWorld::CellNotification : public ObjectNotification
{
public:
	CellNotification();
	virtual ~CellNotification();

	virtual void addToWorld(Object &object) const;
	virtual void removeFromWorld(Object &object) const;

	virtual bool positionChanged(Object &object, bool dueToParentChange, const Vector& oldPosition) const;
	virtual void rotationChanged(Object &object, bool dueToParentChange) const;
	virtual bool positionAndRotationChanged(Object &object, bool dueToParentChange, const Vector& oldPosition) const;

private:
	CellNotification(const CellNotification &);
	CellNotification &operator =(const CellNotification &);

	void transformChanged(Object &object) const;
};

// ======================================================================

#endif
