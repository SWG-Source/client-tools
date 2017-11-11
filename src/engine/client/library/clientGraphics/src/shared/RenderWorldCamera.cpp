// ======================================================================
//
// RenderWorldCamera.cpp
// Copyright 2001 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/RenderWorldCamera.h"

#include "clientGraphics/RenderWorld.h"
#include "sharedObject/CellProperty.h"

// ======================================================================

RenderWorldCamera::RenderWorldCamera() :
	Camera(),
	m_excludedObjectWatcherList()
{
	CellProperty::addPortalCrossingNotification(*this);
}

// ----------------------------------------------------------------------

RenderWorldCamera::~RenderWorldCamera()
{
}

// ----------------------------------------------------------------------

void RenderWorldCamera::drawScene() const
{
	ConstObjectWatcherList::const_iterator iter;
	for (iter = m_excludedObjectWatcherList.begin(); iter != m_excludedObjectWatcherList.end(); ++iter)
	{
		Object const * const object = (*iter).getPointer();
		if (object)
			RenderWorld::recursivelyDisableDpvsObjectsForThisRender(object);
		else
			DEBUG_WARNING(true, ("RenderWorldCamera::drawScene: NULL object found in exclude object list"));
	}

	RenderWorld::drawScene(*this);
}

// ----------------------------------------------------------------------

void RenderWorldCamera::addExcludedObject(Object const * const excludedObject)
{
	NOT_NULL(excludedObject);
	if (excludedObject)
		m_excludedObjectWatcherList.push_back(ConstWatcher<Object>(excludedObject));
}

// ----------------------------------------------------------------------

void RenderWorldCamera::clearExcludedObjects()
{
	m_excludedObjectWatcherList.clear();
}

// ----------------------------------------------------------------------

void RenderWorldCamera::setExcludedObject(Object const * const excludedObject)
{
	clearExcludedObjects();

	if (excludedObject)
		addExcludedObject(excludedObject);
}

// ======================================================================
