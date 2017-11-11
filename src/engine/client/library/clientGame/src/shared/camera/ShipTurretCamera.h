// ======================================================================
//
// ShipTurretCamera.h
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ShipTurretCamera_H
#define INCLUDED_ShipTurretCamera_H

// ======================================================================

#include "clientObject/GameCamera.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"

class MessageQueue;

// ======================================================================

class ShipTurretCamera: public GameCamera
{
public:

	ShipTurretCamera();
	virtual ~ShipTurretCamera();

	virtual void setActive(bool active);
	virtual float alter(float elapsedTime);

	void setMessageQueue(MessageQueue const *queue);
	void setTarget(Object const *object);
	Object const * getTarget() const;

private:
	ShipTurretCamera(ShipTurretCamera const &);
	ShipTurretCamera &operator=(ShipTurretCamera const &);

	void setDustEnabled(bool dustEnabled);
	
private:

	MessageQueue const *m_queue;
	ConstWatcher<Object> m_target;
	ConstWatcher<Object> m_turretTarget;
	Transform m_cameraTransform;
	Object *m_dustObject;

	typedef stdvector<float>::fwd ZoomSettingList;
	ZoomSettingList * const m_zoomSettingList;
	int m_zoomSetting;
	float m_zoom;
	float m_currentZoom;
};

//----------------------------------------------------------------------

inline Object const * ShipTurretCamera::getTarget() const
{
	return m_target;
}

// ======================================================================

#endif

