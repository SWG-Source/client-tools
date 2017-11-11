// ======================================================================
//
// CameraController.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientObject/FirstClientObject.h"
#include "clientObject/CameraController.h"

#include "sharedObject/AlterResult.h"

// ======================================================================

CameraController::CameraController(Object *owner) :
	Controller(owner)
{
}

// ----------------------------------------------------------------------

MessageQueue const *CameraController::getMessageQueue() const
{
	return m_messageQueue;
}

// ----------------------------------------------------------------------

MessageQueue *CameraController::getMessageQueue()
{
	return m_messageQueue;
}

// ----------------------------------------------------------------------

float CameraController::realAlter(float time)
{
	//-- Chain up to parent class.
	float const baseAlterResult = Controller::realAlter(time);
	UNREF(baseAlterResult);

	//-- Indicate that this controller needs an update every frame.  We must
	//   do this because we allow access to the message queue.  Changes to
	//   the message queue require an alter to occur for the 
	//   Controller's associated owner Object.  This is the way we guarantee
	//   that each CameraController is safe from changes to the MessageQueue:
	//   we cause it to always alter regardless.  This isn't an efficient solution
	//   because we will be forcing an alter when we might not need one every
	//   frame.
	return AlterResult::cms_alterNextFrame;
}

// ======================================================================
