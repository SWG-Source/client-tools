// ======================================================================
//
// DynamicDebrisObject.h
// jwatson
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_DynamicDebrisObject_H
#define INCLUDED_DynamicDebrisObject_H

// ======================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"
#include "sharedObject/Object.h"

class MemoryBlockManager;
class Plane;

// ======================================================================

class DynamicDebrisObject : public Object
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

	typedef ConstWatcher<Object> ConstObjectWatcher;

public:

	DynamicDebrisObject ();
	virtual ~DynamicDebrisObject ();

	virtual float alter (float elapsedTime);

	void setPhysicsParameters(float timeToLive, Vector const & initialVelocity_w, Vector const & rotationYawPitchRoll);

	static bool splitObjects(Object & objectToSplit, Plane const & plane, DynamicDebrisObject *& front, DynamicDebrisObject *& back, float debrisParticleThreshold);

	typedef stdvector<Plane>::fwd PlaneVector;
	typedef stdvector<DynamicDebrisObject *>::fwd DynamicDebrisObjectVector;

	static bool splitObjects(Object & objectToSplit, PlaneVector const & planes, DynamicDebrisObjectVector & results, float debrisParticleThreshold);

	Vector const & getCurrentVelocity_w() const;
	Vector const & getCurrentYawPitchRollRates() const;

	void setCurrentVelocity_w(Vector const & velocity_w);
	void setCurrentYawPitchRollRates(Vector const & yawPitchRoll);

	float getLifeTimer() const;
	void setKeepalive(bool keepalive);

	void setLifeTimerToStartFade();

	bool isKeepalive() const;

private:

	DynamicDebrisObject (const DynamicDebrisObject&);
	DynamicDebrisObject& operator= (const DynamicDebrisObject&);

	float alterFallTowardTarget(float elapsedTimeSecs);
	void alterFindFallTowardTarget();
	void alterCheckForPlayerShipCollision();

private:

	Vector m_currentVelocity_w;
	Vector m_rotationYawPitchRoll;
	Vector m_lastPositionVsPlayer_o;
	bool m_skipPlayerCollision;

	float m_lifeTimer;

	ConstObjectWatcher m_fallTowardObject;

	bool m_keepalive;
};

// ======================================================================

#endif
