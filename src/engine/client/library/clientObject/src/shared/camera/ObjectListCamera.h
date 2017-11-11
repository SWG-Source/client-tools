// ======================================================================
//
// ObjectListCamera.h
// Portions copyright 1999 Bootprint Entertainment
// Portions copyright 2001, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ObjectListCamera_H
#define INCLUDED_ObjectListCamera_H

// ======================================================================

class ObjectList;
class Texture;

#include "clientGraphics/Camera.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"

// ======================================================================

class ObjectListCamera : public Camera
{
	friend class BlueprintTextureRendererTemplate;
	friend class DrawTextureCommandElement;

public:

	explicit ObjectListCamera(int numberOfObjectLists);
	~ObjectListCamera(void);

	void setEnvironmentTexture(Texture const *texture);

	void addObjectList(const ObjectList *objectList);
	void removeObjectList(const ObjectList *objectList);

	void setExcludedObject(const Object *excludedObject);

protected:

	virtual void beginScene(void) const;
	virtual void drawScene(void) const;
	virtual void endScene(void) const;

private:

	ObjectListCamera(void);
	ObjectListCamera(const ObjectListCamera &);
	ObjectListCamera &operator =(const ObjectListCamera &);

private:

	int                       m_maxNumberOfObjectLists;
	int                       m_numberOfObjectLists;
	ObjectList const * *      m_objectList;
	ConstWatcher<Object>      m_excludedObjectWatcher;
	Texture const *           m_environmentTexture;
};

// ======================================================================

#endif
