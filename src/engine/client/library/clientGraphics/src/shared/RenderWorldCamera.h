// ======================================================================
//
// RenderWorldCamera.h
// Copyright 2001 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_RenderWorldCamera_H
#define INCLUDED_RenderWorldCamera_H

// ======================================================================

#include "clientGraphics/Camera.h"
#include <vector>

// ======================================================================

class RenderWorldCamera : public Camera
{
public:

	RenderWorldCamera();
	virtual ~RenderWorldCamera();

	void setExcludedObject(Object const * excludedObject);
	void addExcludedObject(Object const * excludedObject);
	void clearExcludedObjects();

private:

	// Disabled
	RenderWorldCamera(const RenderWorldCamera &);
	RenderWorldCamera &operator =(const RenderWorldCamera &);

protected:

	virtual void drawScene() const;

protected:

	typedef std::vector<ConstWatcher<Object> > ConstObjectWatcherList;
	ConstObjectWatcherList m_excludedObjectWatcherList;
};

// ======================================================================

#endif
