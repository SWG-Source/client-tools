// ======================================================================
//
// BannerController.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/BannerController.h"

#include "clientGame/Game.h"
#include "clientGraphics/Graphics.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientTerrain/WeatherManager.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/Object.h"
#include "sharedRandom/Random.h"

// ======================================================================

namespace BannerControllerNamespace
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

using namespace BannerControllerNamespace;

// ======================================================================
// namespace BannerControllerNamespace
// ======================================================================

Vector BannerControllerNamespace::simulateWindVelocity_w(float elapsedTime)
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
// class BannerController: PUBLIC
// ======================================================================

BannerController::BannerController(Object *owner, float windSpeedScale, float liftRadiansPerUnitSpeed, float maxAnimationSpeed) :
	Controller(owner),
	m_windSpeedScale(windSpeedScale),
	m_liftRadiansPerUnitSpeed(liftRadiansPerUnitSpeed),
	m_maxAnimationSpeed(maxAnimationSpeed)
{
	NOT_NULL(owner);
}

// ======================================================================

float BannerController::realAlter(float elapsedTime)
{
	//-- Get wind velocity in parent object (attachment) space.
	Object *const bannerObject = getOwner();
	if (!bannerObject)
	{
		// Nothing to do with no owner.
		return AlterResult::cms_kill;
	}

	Object *const parentObject = bannerObject->getParent();
	if (!parentObject)
	{
		// Nothing to do with no parent object.  This is unexpected.
		DEBUG_WARNING(true, ("Banner object does not have a parent, seems to be improperly constructed, logic error, killing banner."));
		return AlterResult::cms_kill;
	}

#if 1
	Vector const windVelocity_w = WeatherManager::getScaledWindVelocity_w();
#else
	Vector const windVelocity_w = (!Game::getSinglePlayer() ? WeatherManager::getScaledWindVelocity_w() : simulateWindVelocity_w(elapsedTime));
#endif
	Vector const windVelocity_o = parentObject->rotate_w2o(windVelocity_w);
	
	//-- Pitch the banner around X axis to simulate lift caused by wind blowing straight at the face (broad side) of the banner.
	float const pitch = getPitch_o(elapsedTime, windVelocity_o);
	
	bannerObject->resetRotate_o2p();
	bannerObject->pitch_o(pitch);

	//-- Get wind speed.
	float const windSpeed = getWindSpeed(elapsedTime, windVelocity_o);

	//-- Tell banner character about wind speed.
	Appearance *const appearance = bannerObject->getAppearance();
	if (appearance)
	{
		SkeletalAppearance2 *const skeletalAppearance = appearance->asSkeletalAppearance2();
		if (skeletalAppearance)
			skeletalAppearance->setDesiredVelocity(std::min(windSpeed, m_maxAnimationSpeed) * Vector::unitZ);
	}

	return AlterResult::cms_alterNextFrame;
}

// ======================================================================
// class BannerController: PRIVATE
// ======================================================================

float BannerController::getPitch_o(float /*elapsedTime*/, Vector const &windVelocity_o) const
{
	//-- Find wind speed contribution along +z axis (blowing directly against broad side of banner).
	float const effectiveWindSpeed = windVelocity_o.dot(Vector::unitZ) * m_windSpeedScale;
	float const deltaPitch         = std::max(-0.5f * PI, std::min(0.5f * PI, - effectiveWindSpeed * m_liftRadiansPerUnitSpeed));

	return deltaPitch;
}

// ----------------------------------------------------------------------

float BannerController::getWindSpeed(float /*elapsedTime*/, Vector const &windVelocity_o) const
{
	//-- Apply all wind velocity to the banner's animation speed, regardless of direction.
	return windVelocity_o.magnitude() * m_windSpeedScale;
}

// ======================================================================
