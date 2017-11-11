// ======================================================================
//
// RenderWorld_CellNotification.cpp
// Copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/RenderWorld_CellNotification.h"

#include "sharedObject/CellProperty.h"

#include "dpvs.hpp"

// ======================================================================

RenderWorld::CellNotification::CellNotification()
{
}

// ----------------------------------------------------------------------

RenderWorld::CellNotification::~CellNotification()
{
}

// ----------------------------------------------------------------------

void RenderWorld::CellNotification::addToWorld(Object &object) const
{
	transformChanged(object);

	const Object::DpvsObjects *dpvsObjects = object.getDpvsObjects();
	if (dpvsObjects)
	{
		const CellProperty *cell = object.getCellProperty();
		NOT_NULL(cell);
		DPVS::Cell *dpvsCell = cell->getDpvsCell();
		NOT_NULL(dpvsCell);

		const Object::DpvsObjects::const_iterator iEnd = dpvsObjects->end();
		for (Object::DpvsObjects::const_iterator i = dpvsObjects->begin(); i != iEnd; ++i)
		{
			DPVS::Object *dpvsObject = *i;
			dpvsObject->setCell(dpvsCell);
		}
	}
}

// ----------------------------------------------------------------------

void RenderWorld::CellNotification::removeFromWorld(Object &object) const
{
	const Object::DpvsObjects *dpvsObjects = object.getDpvsObjects();
	if (dpvsObjects)
	{
		const Object::DpvsObjects::const_iterator iEnd = dpvsObjects->end();
		for (Object::DpvsObjects::const_iterator i = dpvsObjects->begin(); i != iEnd; ++i)
			(*i)->setCell(NULL);
	}
}

// ----------------------------------------------------------------------

bool RenderWorld::CellNotification::positionChanged(Object &object, bool dueToParentChange, const Vector& oldPosition) const
{
	UNREF(oldPosition);
	UNREF(dueToParentChange);
	transformChanged(object);
	return true;
}

// ----------------------------------------------------------------------

void RenderWorld::CellNotification::rotationChanged(Object &object, bool dueToParentChange) const
{
	UNREF(dueToParentChange);
	transformChanged(object);
}

// ----------------------------------------------------------------------

bool RenderWorld::CellNotification::positionAndRotationChanged(Object &object, bool dueToParentChange, const Vector& oldPosition) const
{
	UNREF(oldPosition);
	UNREF(dueToParentChange);
	transformChanged(object);
	return true;
}

// ----------------------------------------------------------------------

void RenderWorld::CellNotification::transformChanged(Object &object) const
{
	const Transform &c2w = object.getTransform_o2w();
	DPVS::Matrix4x4 dpvsTransform;
	convertToDpvsTransform(c2w, Vector::xyz111, dpvsTransform, object);
	object.getCellProperty()->getDpvsCell()->setCellToWorldMatrix(dpvsTransform);
}

// ======================================================================
