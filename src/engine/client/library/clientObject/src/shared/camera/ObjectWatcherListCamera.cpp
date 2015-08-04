// ======================================================================
//
// ObjectWatcherListCamera.cpp
// Portions copyright 1999 Bootprint Entertainment
// Portions copyright 2001, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientObject/FirstClientObject.h"
#include "clientObject/ObjectWatcherListCamera.h"

#include "sharedObject/ObjectWatcherList.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/Texture.h"
#include <vector>
#include <algorithm>

// ======================================================================

ObjectWatcherListCamera::ObjectWatcherListCamera () :
Camera (),
m_owls (new ObjectWatcherListList),
m_environmentTexture(0)
{
}

// ----------------------------------------------------------------------

ObjectWatcherListCamera::~ObjectWatcherListCamera ()
{
	if (m_environmentTexture)
	{
		m_environmentTexture->release();
		m_environmentTexture = 0;
	}

	delete m_owls;
	m_owls = 0;
}

// ----------------------------------------------------------------------
/**
 * Add an object list to the Camera's render.
 * 
 * This routine will add the specified object list to the Camera's list of
 * object lists for rendering by the camera.  The camera will only render
 * object on object lists it knows about.
 * 
 * This routine will call Fatal in debug compiles if it is passed NULL.
 * 
 * This routine will call Fatal in debug compiles if the object list list
 * is already full.
 * 
 * @param objectListToAdd  ObjectList to add to the Camera's render
 */

void ObjectWatcherListCamera::addObjectWatcherList (const ObjectWatcherList & owl)
{
	m_owls->push_back (&owl);
}

// ----------------------------------------------------------------------
/**
 * Remove an object list from the Camera's render.
 * 
 * This routine will call Fatal in debug compiles if it is passed NULL.
 * 
 * This routine will call Fatal in debug compiles if the object list was
 * not found in the object list.
 * 
 * @param objectListToRemove  ObjectList to remove from the Camera's render
 */

void ObjectWatcherListCamera::removeObjectWatcherList (const ObjectWatcherList & owl)
{
	m_owls->erase (std::remove (m_owls->begin (), m_owls->end (), &owl), m_owls->end ());
}

// ----------------------------------------------------------------------

void ObjectWatcherListCamera::beginScene () const
{
	Camera::beginScene ();
	ShaderPrimitiveSorter::pushCell (m_environmentTexture, false, 0.0f, PackedArgb::solidWhite);
}

// ----------------------------------------------------------------------
/**
 * Add all objects to the 3d scene.
 * 
 * This function call is only valid between a beginScene () and an endScene () pair.
 * 
 * @see Camera::beginScene (), Camera::endScene ()
 */

void ObjectWatcherListCamera::drawScene () const
{
	const ObjectWatcherListList::const_iterator end = m_owls->end ();

	// @todo if we knew which lists had lights on them, we might be able to reduce the number of
	// lists (and therefore objects) that we call this function upon.
	{
		for (ObjectWatcherListList::const_iterator it = m_owls->begin (); it != end; ++it)
			(*it)->setRegionOfInfluenceEnabled (true);
	}

	{
		for (ObjectWatcherListList::const_iterator it = m_owls->begin (); it != end; ++it)
			(*it)->render (excludedObjectWatcher.getPointer ());
	}

	{
		for (ObjectWatcherListList::const_iterator it = m_owls->begin (); it != end; ++it)
			(*it)->setRegionOfInfluenceEnabled (false);
	}
}

// ----------------------------------------------------------------------

void ObjectWatcherListCamera::endScene () const
{
	ShaderPrimitiveSorter::popCell ();
	Camera::endScene ();
}

// ----------------------------------------------------------------------

void ObjectWatcherListCamera::setExcludedObject (const Object * excludedObject)
{
	excludedObjectWatcher = excludedObject;
}

// ----------------------------------------------------------------------

void ObjectWatcherListCamera::setEnvironmentTexture(Texture const * const texture)
{
	if (texture)
		texture->fetch();

	if (m_environmentTexture)
		m_environmentTexture->release();

	m_environmentTexture = texture;
}

// ======================================================================
