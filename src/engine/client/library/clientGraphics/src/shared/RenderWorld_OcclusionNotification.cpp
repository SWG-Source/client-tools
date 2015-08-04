// ======================================================================
//
// RenderWorld_OcclusionNotification.cpp
// Copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/RenderWorld_OcclusionNotification.h"

#include "sharedObject/appearance.h"
#include "sharedObject/CellProperty.h"

#include "dpvs.hpp"

// ======================================================================

namespace RenderWorld_OcclusionNotificationNamespace
{
	const Vector &GetObjectScale(const Object &object);
}
using namespace RenderWorld_OcclusionNotificationNamespace;

// ======================================================================

const Vector &RenderWorld_OcclusionNotificationNamespace::GetObjectScale(const Object &object)
{
	const Appearance * appearance = object.getAppearance();
	if (appearance && appearance->asSkeletalAppearance2())
		return Vector::xyz111;

	return object.getScale();
}

// ======================================================================

RenderWorld::OcclusionNotification::OcclusionNotification()
: ObjectNotification()
{
}

// ----------------------------------------------------------------------

RenderWorld::OcclusionNotification::~OcclusionNotification()
{
}

// ----------------------------------------------------------------------

int RenderWorld::OcclusionNotification::getPriority() const
{
	return 5;
}

// ----------------------------------------------------------------------

void RenderWorld::OcclusionNotification::addToWorld(Object &object) const
{
	cellChanged(object, false);
}

// ----------------------------------------------------------------------

void RenderWorld::OcclusionNotification::removeFromWorld(Object &object) const
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

bool RenderWorld::OcclusionNotification::positionChanged(Object &object, bool dueToParentChange, const Vector& oldPosition) const
{
	UNREF(oldPosition);
	UNREF(dueToParentChange);
	transformChanged(object);
	return true;
}

// ----------------------------------------------------------------------

void RenderWorld::OcclusionNotification::rotationChanged(Object &object, bool dueToParentChange) const
{
	UNREF(dueToParentChange);
	transformChanged(object);
}

// ----------------------------------------------------------------------

bool RenderWorld::OcclusionNotification::positionAndRotationChanged(Object &object, bool dueToParentChange, const Vector& oldPosition) const
{
	UNREF(oldPosition);
	UNREF(dueToParentChange);
	transformChanged(object);
	return true;
}

// ----------------------------------------------------------------------

void RenderWorld::OcclusionNotification::cellChanged(Object &object, bool dueToParentChange) const
{
	UNREF(dueToParentChange);

	const Object::DpvsObjects *dpvsObjects = object.getDpvsObjects();
	if (dpvsObjects)
	{
		const CellProperty *cell = object.getParentCell();
		NOT_NULL(cell);
		DPVS::Cell *dpvsCell = cell->getDpvsCell();
		NOT_NULL(dpvsCell);

		DPVS::Matrix4x4 dpvsTransform;
		convertToDpvsTransform(object.getTransform_o2c(), GetObjectScale(object), dpvsTransform, object);

		const Object::DpvsObjects::const_iterator iEnd = dpvsObjects->end();
		for (Object::DpvsObjects::const_iterator i = dpvsObjects->begin(); i != iEnd; ++i)
		{
			DPVS::Object *dpvsObject = *i;
			dpvsObject->setCell(dpvsCell);
			dpvsObject->setObjectToCellMatrix(dpvsTransform);
		}
	}
}

// ----------------------------------------------------------------------

void RenderWorld::OcclusionNotification::extentChanged(Object &object) const
{
	transformChanged(object);
}

// ----------------------------------------------------------------------

void RenderWorld::OcclusionNotification::transformChanged(Object &object) const
{
	const Object::DpvsObjects *dpvsObjects = object.getDpvsObjects();
	if (dpvsObjects)
	{
		DPVS::Matrix4x4 dpvsTransform;
		convertToDpvsTransform(object.getTransform_o2c(), GetObjectScale(object), dpvsTransform, object);

		const Object::DpvsObjects::const_iterator iEnd = dpvsObjects->end();
		for (Object::DpvsObjects::const_iterator i = dpvsObjects->begin(); i != iEnd; ++i)
			(*i)->setObjectToCellMatrix(dpvsTransform);
	}
}

// ======================================================================
