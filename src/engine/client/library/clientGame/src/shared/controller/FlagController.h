// ======================================================================
//
// FlagController.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_FlagController_H
#define INCLUDED_FlagController_H

// ======================================================================

class Vector;

#include "sharedObject/Controller.h"

// ======================================================================

class FlagController: public Controller
{
public:

	FlagController(Object *owner, float windSpeedScale, float maxAnimationSpeed);

protected:

	virtual float realAlter(float time);

private:

	float         getDeltaYaw_o(float elapsedTime, Vector const &windVelocity_o) const;
	float         getWindSpeed(float elapsedTime, Vector const &windVelocity_o) const;

	// Disabled.
	FlagController();
	FlagController(FlagController const&);
	FlagController &operator =(FlagController const&);

private:

	float m_windSpeedScale;
	float m_maxAnimationSpeed;

};

// ======================================================================

#endif
