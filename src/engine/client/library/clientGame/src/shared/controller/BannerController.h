// ======================================================================
//
// BannerController.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_BannerController_H
#define INCLUDED_BannerController_H

// ======================================================================

class Vector;

#include "sharedObject/Controller.h"

// ======================================================================

class BannerController: public Controller
{
public:

	BannerController(Object *owner, float windSpeedScale, float liftRadiansPerUnitSpeed, float maxAnimationSpeed);

protected:

	virtual float realAlter(float time);

private:

	float         getPitch_o(float elapsedTime, Vector const &windVelocity_o) const;
	float         getWindSpeed(float elapsedTime, Vector const &windVelocity_o) const;

	// Disabled.
	BannerController();
	BannerController(BannerController const&);
	BannerController &operator =(BannerController const&);

private:

	float m_windSpeedScale;
	float m_liftRadiansPerUnitSpeed;
	float m_maxAnimationSpeed;

};

// ======================================================================

#endif
