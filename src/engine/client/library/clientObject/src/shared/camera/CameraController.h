// ======================================================================
//
// CameraController.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_CameraController_H
#define INCLUDED_CameraController_H

// ======================================================================

#include "sharedObject/Controller.h"

// ======================================================================
/**
 * This class provides a stock Controller augmented with the ability to
 * retrieve the Controller's associated MessageQueue.
 *
 * The realAlter() function will return a value indicating that this Controller
 * needs its associated owner Object to be updated every frame.  Do not use
 * this for any object other than a Camera.
 *
 * The initial need for this class arose when the client needed to feed
 * the camera controller's message queue into the input map.
 */

class CameraController: public Controller
{
public:

	explicit CameraController(Object *owner);

	MessageQueue const *getMessageQueue() const;
	MessageQueue       *getMessageQueue();

protected:

	virtual float realAlter(float time);

private:

	// Disabled.
	CameraController();
	CameraController(const CameraController &);
	CameraController &operator =(const CameraController &);


};

// ======================================================================

#endif
