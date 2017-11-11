// ======================================================================
//
// ObjectListCamera.cpp
// Portions copyright 1999 Bootprint Entertainment
// Portions copyright 2001, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientObject/FirstClientObject.h"
#include "clientObject/ObjectListCamera.h"

#include "sharedObject/ObjectList.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/Texture.h"

// ======================================================================

ObjectListCamera::ObjectListCamera(int maxObjectLists)
: Camera(),
	m_maxNumberOfObjectLists(maxObjectLists),
	m_numberOfObjectLists(0),
	m_objectList(new const ObjectList*[m_maxNumberOfObjectLists]),
	m_excludedObjectWatcher(NULL),
	m_environmentTexture(NULL)
{
	memset(m_objectList, 0, isizeof(*m_objectList) * m_maxNumberOfObjectLists);
}

// ----------------------------------------------------------------------

ObjectListCamera::~ObjectListCamera(void)
{
	if (m_environmentTexture)
		m_environmentTexture->release();

	delete [] m_objectList;
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

void ObjectListCamera::addObjectList(const ObjectList *objectListToAdd)
{
	DEBUG_FATAL(!objectListToAdd, ("Camera::addObjectList NULL"));
	DEBUG_FATAL(m_numberOfObjectLists >= m_maxNumberOfObjectLists, ("Camera::addObjectList full %d/%d", m_numberOfObjectLists, m_maxNumberOfObjectLists));
	m_objectList[m_numberOfObjectLists++] = objectListToAdd;
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

void ObjectListCamera::removeObjectList(const ObjectList *objectListToRemove)
{
	int i;

	DEBUG_FATAL(!objectListToRemove, ("Camera::removeObjectList NULL"));

	for (i = 0; i < m_numberOfObjectLists && m_objectList[i] != objectListToRemove; ++i)
		;

	if (i < m_numberOfObjectLists)
	{
		--m_numberOfObjectLists;
		m_objectList[i] = m_objectList[m_numberOfObjectLists];
		m_objectList[m_numberOfObjectLists] = NULL;
	}
	else
	{
		DEBUG_FATAL(true, ("Camera::removeObjectList not found %p/%d", objectListToRemove, m_numberOfObjectLists));
	}
}

// ----------------------------------------------------------------------

void ObjectListCamera::setEnvironmentTexture(Texture const *texture)
{
	if (texture)
		texture->fetch();

	if (m_environmentTexture)
		m_environmentTexture->release();

	m_environmentTexture = texture;
}

// ----------------------------------------------------------------------

void ObjectListCamera::beginScene() const
{
	Camera::beginScene();
	ShaderPrimitiveSorter::pushCell(m_environmentTexture, false, 0.0f, PackedArgb::solidWhite);
}

// ----------------------------------------------------------------------
/**
 * Add all objects to the 3d scene.
 * 
 * This function call is only valid between a beginScene() and an endScene() pair.
 * 
 * @see Camera::beginScene(), Camera::endScene()
 */

void ObjectListCamera::drawScene(void) const
{
	// @todo if we knew which lists had lights on them, we might be able to reduce the number of
	// lists (and therefore objects) that we call this function upon.
	{
		for (int i = 0; i < m_numberOfObjectLists; ++i)
			m_objectList[i]->setRegionOfInfluenceEnabled(true);
	}

	{
		for (int i = 0; i < m_numberOfObjectLists; ++i)
			m_objectList[i]->render(m_excludedObjectWatcher.getPointer());
	}

	{
		for (int i = 0; i < m_numberOfObjectLists; ++i)
			m_objectList[i]->setRegionOfInfluenceEnabled(false);
	}
}

// ----------------------------------------------------------------------

void ObjectListCamera::endScene() const
{
	ShaderPrimitiveSorter::popCell();
	Camera::endScene();
}

// ----------------------------------------------------------------------

void ObjectListCamera::setExcludedObject(const Object *excludedObject)
{
	m_excludedObjectWatcher = excludedObject;
}

// ======================================================================
