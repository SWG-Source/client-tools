//===================================================================
//
// StructurePlacementCamera.h
// asommers 
//
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_StructurePlacementCamera_H
#define INCLUDED_StructurePlacementCamera_H

//===================================================================

#include "clientObject/GameCamera.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"
#include "sharedObject/LotType.h"

class Object;
class MessageQueue;
class Shader;
class StructureFootprint;

//===================================================================

class StructurePlacementCamera : public GameCamera
{
public:

	StructurePlacementCamera ();
	virtual ~StructurePlacementCamera ();

	virtual void  setActive (bool active);
	virtual float alter (float time);

	void          setMessageQueue (const MessageQueue* queue);
	void          setTarget (const Object* target);
	void          setMouseCoordinates (int x, int y);
	void          setStructureFootprint (const StructureFootprint* structureFootprint);
	void          setStructureObject (Object* structureObject);
	void          setRotation (RotationType rotationType);

	const Vector& getCreateLocation () const;

protected:

	virtual void  drawScene () const;

private:

	enum Keys
	{
		K_up,
		K_down,
		K_left,
		K_right,

		K_COUNT
	};

private:

	const MessageQueue*       m_queue;
	bool                      m_keys [K_COUNT];
	ConstWatcher<Object>      m_target;
	Vector                    m_pivot;
	float                     m_zoom;
	int                       m_mouseX;
	int                       m_mouseY;
	const StructureFootprint* m_structureFootprint;
	mutable Object*           m_structureObject;
	mutable Vector            m_createLocation;
	RotationType              m_rotationType;
	float                     m_lodThreshold;
	float                     m_highLodThreshold;
	float                     m_fov;
	Light* const              m_light;
	const Shader* const       m_lotOccupiedShader;
	const Shader* const       m_allowedFootprintShader;
	const Shader* const       m_disallowedFootprintShader;

private:

	StructurePlacementCamera (const StructurePlacementCamera&);
	StructurePlacementCamera& operator= (const StructurePlacementCamera&);
};

//===================================================================

#endif
