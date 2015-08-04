// ======================================================================
//
// FlyByCamera.cpp
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/FlyByCamera.h"

#include "clientGraphics/RenderWorld.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedObject/AlterResult.h"
#include "sharedRandom/Random.h"

// ======================================================================
//
// FlyByCameraNamespace
//
// ======================================================================

namespace FlyByCameraNamespace
{
	float const s_targetNotMovedExpireTime = 6.0f;
	float const s_cameraTooFarExpireTime = 10.0f;
}

using namespace FlyByCameraNamespace;

// ======================================================================
//
// FlyByCamera
//
// ======================================================================

// ----------------------------------------------------------------------

FlyByCamera::FlyByCamera()
 : GameCamera()
 , m_lookAtObject(NULL)
 , m_lookAtObjectNotMovedTimer(s_targetNotMovedExpireTime)
 , m_cameraTooFarTimer(s_cameraTooFarExpireTime)
 , m_goalCameraPosition_w()
 , m_previousTargetPosition_w()
 , m_repositionCameraDistance(0.0f)
 , m_lookAtObjectMoving(true)
{
}

// ----------------------------------------------------------------------

FlyByCamera::~FlyByCamera()
{
}

// ----------------------------------------------------------------------

void FlyByCamera::setActive(bool const active)
{
	GameCamera::setActive(active);
}

// ----------------------------------------------------------------------

float FlyByCamera::alter(float const elapsedTime)
{
	if (   !isActive()
	    || !m_lookAtObject)
	{
		return AlterResult::cms_alterNextFrame;
	}

	Vector const & targetPosition_w = m_lookAtObject->getPosition_w();

	if (targetPosition_w.magnitudeBetweenSquared(m_previousTargetPosition_w) < sqr(1.0f))
	{
		// The target is NOT moving very much

		if (   m_lookAtObjectMoving
		    || m_lookAtObjectNotMovedTimer.updateNoReset(elapsedTime))
		{
			m_lookAtObjectNotMovedTimer.reset();
			m_lookAtObjectMoving = false;
			calculateCameraPosition();
		}

		// Swoop the camera around

		setPosition_w(getPosition_w() + (m_goalCameraPosition_w - getPosition_w()) * elapsedTime);
	}
	else
	{
		// The target IS moving

		m_lookAtObjectMoving = true;
		m_lookAtObjectNotMovedTimer.reset();

		// If the target is far away from the camera, calculate a new camera position

		if (   m_cameraTooFarTimer.updateNoReset(elapsedTime)
		    && targetPosition_w.magnitudeBetweenSquared(getPosition_w()) > sqr(m_repositionCameraDistance))
		{
			m_cameraTooFarTimer.reset();
			calculateCameraPosition();
			setPosition_w(m_goalCameraPosition_w);
		}
	}

	m_previousTargetPosition_w = targetPosition_w;

	// Point the camera at the target
	{
		Vector k(targetPosition_w - getPosition_w()); // forward
		k.normalize();
		Vector i(Vector::unitY.cross(k));
		i.normalize();
		Vector j(k.cross(i));
		j.normalize();

		setTransformIJK_o2p(i, j, k);
	}

	//-- Chain back up to parent
	float alterResult = GameCamera::alter(elapsedTime);
	AlterResult::incorporateAlterResult(alterResult, AlterResult::cms_alterNextFrame);
	return alterResult;
}

// ----------------------------------------------------------------------

void FlyByCamera::setTarget(Object const * const object)
{
	m_lookAtObject = object;
	m_lookAtObjectNotMovedTimer.reset();
	m_cameraTooFarTimer.reset();
	m_lookAtObjectMoving = false;
	calculateCameraPosition();
	setPosition_w(m_goalCameraPosition_w);
}

// ----------------------------------------------------------------------

void FlyByCamera::calculateCameraPosition()
{
	if (!m_lookAtObject)
	{
		return;
	}

	if (m_lookAtObjectMoving)
	{
		// Pick a new position in the direction the target is heading

		CollisionProperty const * const collisionProperty = m_lookAtObject->getCollisionProperty();
		float const collisionRadius = (collisionProperty != NULL) ? collisionProperty->getBoundingSphere_l().getRadius() : 5.0f;
		m_repositionCameraDistance = (collisionRadius * 16.0f) + Random::randomReal() * (collisionRadius * 8.0f);
		float const targetDiameter = 2.0f * collisionRadius;
		float const x = (Random::randomReal() * targetDiameter + targetDiameter) * ((rand() % 2) ? 1.0f : -1.0f);
		float const y = (Random::randomReal() * targetDiameter + targetDiameter) * ((rand() % 2) ? 1.0f : -1.0f);
		float const z = m_repositionCameraDistance * 0.9f;

		m_goalCameraPosition_w = m_lookAtObject->rotateTranslate_o2w(Vector(x, y, z));
	}
	else
	{
		// Pick a new position behind and above the target

		CollisionProperty const * const collisionProperty = m_lookAtObject->getCollisionProperty();
		float const collisionRadius = (collisionProperty != NULL) ? collisionProperty->getBoundingSphere_l().getRadius() : 5.0f;
		float const targetDiameter = 2.0f * collisionRadius;
		float const x = 0.0f;
		float const y = (targetDiameter * 2.0f);
		float const z = -(targetDiameter * 4.0f);

		m_goalCameraPosition_w = m_lookAtObject->rotateTranslate_o2w(Vector(x, y, z));
	}
}

// ======================================================================
