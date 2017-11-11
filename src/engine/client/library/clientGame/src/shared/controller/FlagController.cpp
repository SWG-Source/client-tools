// ======================================================================
//
// FlagController.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/FlagController.h"

#include "clientGame/Game.h"
#include "clientGraphics/Graphics.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientTerrain/WeatherManager.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/Object.h"
#include "sharedRandom/Random.h"

// ======================================================================

namespace FlagControllerNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	Vector simulateWindVelocity_w(float elapsedTime);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	int    s_lastFrameEvaluated;

	float  s_time;

	float  s_baseYaw;
	float  s_yawAmplitude;
	float  s_yawTimeFactor;

	float  s_baseSpeed;
	float  s_speedAmplitude;
	float  s_speedTimeFactor;
}

using namespace FlagControllerNamespace;

// ======================================================================
// namespace FlagControllerNamespace
// ======================================================================

Vector FlagControllerNamespace::simulateWindVelocity_w(float elapsedTime)
{
	int const currentFrameNumber = Graphics::getFrameNumber();
	if (currentFrameNumber != s_lastFrameEvaluated)
	{
		if (s_lastFrameEvaluated == 0)
		{
			//-- One time initialization goes here.
			s_baseYaw       = -PI + Random::randomReal() * 2.0f * PI;
			s_yawAmplitude  = Random::randomReal() * 0.5f * PI;
			s_yawTimeFactor = 0.5f + 3.0f * Random::randomReal();

			s_baseSpeed       = 2.0f + Random::randomReal() * 6.0f;
			s_speedAmplitude  = s_baseSpeed;
			s_speedTimeFactor = 2.0f * PI + 15.0f * Random::randomReal();
		}

		//-- Per frame simulation goes here.
		s_lastFrameEvaluated  = currentFrameNumber;
		s_time               += elapsedTime;
	}

	//-- Calculate speed.
	float const speed = s_baseSpeed + s_speedAmplitude * sin(s_speedTimeFactor * s_time);

	//-- Evaluation based on current frame parameters goes here.
	float const angle = s_baseYaw + s_yawAmplitude * sin(s_yawTimeFactor * s_time);
	return Vector(speed * sin(angle), 0.0f, speed * cos(angle));
}

// ======================================================================
// class FlagController: PUBLIC
// ======================================================================

FlagController::FlagController(Object *owner, float windSpeedScale, float maxAnimationSpeed) :
	Controller(owner),
	m_windSpeedScale(windSpeedScale),
	m_maxAnimationSpeed(maxAnimationSpeed)
{
	NOT_NULL(owner);
}

// ======================================================================

float FlagController::realAlter(float elapsedTime)
{
	//-- Get wind velocity in object space.
	Object *const object = getOwner();
	if (!object)
	{
		// Nothing to do with no owner.
		return AlterResult::cms_kill;
	}

#if 1
	Vector const windVelocity_w = WeatherManager::getScaledWindVelocity_w();
#else
	Vector const windVelocity_w = (!Game::getSinglePlayer() ? WeatherManager::getScaledWindVelocity_w() : simulateWindVelocity_w(elapsedTime));
#endif
	Vector const windVelocity_o      = object->rotate_w2o(windVelocity_w);
	float const  rawWindSpeedSquared = windVelocity_o.magnitudeSquared();
	
	//-- Get wind speed contribution along direction of y-z plane.
	float const windSpeed = getWindSpeed(elapsedTime, windVelocity_o);

	//-- Yaw the flag to line up +z with wind direction.
	if (rawWindSpeedSquared > 0.0f)
	{
		float const deltaYaw = getDeltaYaw_o(elapsedTime, windVelocity_o);
		object->yaw_o(deltaYaw);
	}

	//-- Tell flag character about wind speed.
	Appearance *const appearance = object->getAppearance();
	if (appearance)
	{
		SkeletalAppearance2 *const skeletalAppearance = appearance->asSkeletalAppearance2();
		if (skeletalAppearance)
			skeletalAppearance->setDesiredVelocity(std::min(windSpeed, m_maxAnimationSpeed) * Vector::unitZ);
	}

	return AlterResult::cms_alterNextFrame;
}

// ======================================================================
// class FlagController: PRIVATE
// ======================================================================

float FlagController::getDeltaYaw_o(float /*elapsedTime*/, Vector const &windVelocity_o) const
{
	// The flag appearance is located along the +z axis.  When wind is going in 
	// theta = 0 = due north, flag object needs to be rotated to 0 degrees.
	return windVelocity_o.theta();
}

// ----------------------------------------------------------------------

float FlagController::getWindSpeed(float /*elapsedTime*/, Vector const &windVelocity_o) const
{
	//-- Find wind speed contribution projected into flag's x - z plane, then scale it.
	return Vector(windVelocity_o.x, 0.0f, windVelocity_o.z).magnitude() * m_windSpeedScale;
}

// ======================================================================
