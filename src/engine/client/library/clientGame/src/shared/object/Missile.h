//===================================================================
//
// Missile.h
//
// copyright 2004, Sony Online Entertainment
//
//===================================================================

#ifndef INCLUDED_Missile_H
#define INCLUDED_Missile_H

//===================================================================

#include "sharedObject/CachedNetworkId.h"
#include "sharedObject/Object.h"

//===================================================================

class AppearanceTemplate;
class ClientEffectTemplate;
class Countermeasure;

//===================================================================

class Missile : public Object
{
public:
	Missile               (const int missileId, const int missileTypeId, Object * const target, NetworkId const & source, const Transform & sourceLocation, const Vector & targetLocation, const int timeToTarget, const float minSpeed, const AppearanceTemplate * const appearance, const AppearanceTemplate * const trailAppearanceTemplate, const ClientEffectTemplate * const fireEffect, const ClientEffectTemplate * const hitEffect, const ClientEffectTemplate * const countermeasureEffect, const bool foundLauncher, const int targetComponent);
	virtual ~Missile      ();

	virtual float alter   (float elapsedTime);

	void hitTarget        () const;
	void missTarget       ();
	void countermeasure   (Countermeasure * const targetCountermeasure);
	void countermeasure   ();
	Object const * const getTargetObject () const;
	
private:
	enum MissileState {MS_launch, MS_seek, MS_miss, MS_countermeasured};

private:
	bool   avoidObstacles              (Vector & velocity, float const speed) const;
	void   blowupCountermeasured       (bool useHitEffect);
	void   detachTrail                 ();
	void   getVelocitySpiralModel      (Vector & resultVelocity, float & resultSpeed) const;
	bool   isWithinHitDistance         () const;
	void   playCountermeasureEffect    () const;
	void   playHitEffect               () const;
	bool   checkForCollisions          (float elapsedTime) const;

private:
	const int                          m_missileId;
	const int                          m_missileTypeId;
	MissileState                       m_state;
	Watcher<Object>                    m_target;
	Vector                             m_targetLocation;
	float                              m_remainingTime;
	const int                          m_timeToTarget;
	float                              m_initialTurnTimer;
	const float                        m_minSpeed;
	Vector                             m_velocity;
	Vector                             m_initialVelocity;
	const float                        m_maxAcceleration;
	Watcher<Object>                    m_trail;
	const ClientEffectTemplate * const m_hitEffect;
	const ClientEffectTemplate * const m_countermeasureEffect;
	const Vector                       m_deflectVector;
	const int                          m_targetComponent;
	const CachedNetworkId              m_source;

	static const float                 ms_initialTurnTime;
	static const float                 ms_countermeasureDistance;
	static const float                 ms_countermeasureDistanceSquared;
	static const float                 ms_hitDistance;
	static const float                 ms_hitDistanceSquared;

private:
	Missile(const Missile&); //disable
	Missile &operator=(const Missile&); //disable
};

//===================================================================

#endif