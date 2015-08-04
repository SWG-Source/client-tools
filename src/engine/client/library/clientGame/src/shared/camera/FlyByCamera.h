// ======================================================================
//
// FlyByCamera.h
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_FlyByCamera_H
#define INCLUDED_FlyByCamera_H

// ======================================================================

#include "clientObject/GameCamera.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Timer.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"

class MessageQueue;

// ======================================================================

class FlyByCamera: public GameCamera
{
public:

	FlyByCamera();
	virtual ~FlyByCamera();

	virtual void setActive(bool active);
	virtual float alter(float elapsedTime);

	void setTarget(Object const * object);

private:

	void calculateCameraPosition();
	
	ConstWatcher<Object> m_lookAtObject;
	Timer m_lookAtObjectNotMovedTimer;
	Timer m_cameraTooFarTimer;
	Vector m_goalCameraPosition_w;
	Vector m_previousTargetPosition_w;
	float m_repositionCameraDistance;
	bool m_lookAtObjectMoving;

	// Disabled

	FlyByCamera(FlyByCamera const &);
	FlyByCamera &operator=(FlyByCamera const &);
};

// ======================================================================

#endif // INCLUDED_FlyByCamera_H
