// ======================================================================
//
// ObjectWatcherListCamera.h
// Portions copyright 1999 Bootprint Entertainment
// Portions copyright 2001, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ObjectWatcherListCamera_H
#define INCLUDED_ObjectWatcherListCamera_H

// ======================================================================

class ObjectWatcherList;
class Texture;

#include "clientGraphics/Camera.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"

// ======================================================================

class ObjectWatcherListCamera : public Camera
{
public:

	typedef stdvector<const ObjectWatcherList *>::fwd ObjectWatcherListList;

	             ObjectWatcherListCamera    ();
	            ~ObjectWatcherListCamera    ();

	void         setEnvironmentTexture      (const Texture *texture);

	void         addObjectWatcherList       (const ObjectWatcherList & owl);
	void         removeObjectWatcherList    (const ObjectWatcherList & owl);
	void         setExcludedObject          (const Object *excludedObject);

protected:

	void         beginScene     () const;
	void         drawScene      () const;
	void         endScene       () const;

private:

	                          ObjectWatcherListCamera (const ObjectWatcherListCamera &);
	ObjectWatcherListCamera & operator =              (const ObjectWatcherListCamera &);

private:

	ObjectWatcherListList * m_owls;
	ConstWatcher<Object>    excludedObjectWatcher;
	const Texture         * m_environmentTexture;
};

// ======================================================================

#endif
