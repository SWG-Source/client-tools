//===================================================================
//
// Projectile.h
// copyright 2000, verant interactive, inc.
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_Projectile_H
#define INCLUDED_Projectile_H

//===================================================================

class AppearanceTemplate;
class ClientEffectTemplate;

//===================================================================

#include "sharedObject/Object.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Timer.h"
#include "sharedMath/Vector.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"

//===================================================================

class Projectile : public Object
{
public:

	Projectile(const AppearanceTemplate* appearanceTemplate, const ClientEffectTemplate* fireClientEffectTemplate, const ClientEffectTemplate* missHitNothingClientEffectTemplate);
	virtual ~Projectile() = 0;

	virtual float       alter(float time);

	void                setFacing(const CellProperty* startCell, const Vector& startPosition_w, const Vector& endPosition_w);
	void                setExpirationTime(float expirationTime);
	void                setSpeed(float speed);
	void                setTarget(const Object* target);
	void                setStartPosition_w(Vector const &startPosition);
	void                setPaused(bool paused);
	bool                getPaused();
	float               getSpeed();

	enum HitType
	{
		HT_creature,
		HT_terrain,
		HT_metal,
		HT_stone,
		HT_wood,
		HT_other
	};

protected:

	ConstWatcher<Object> m_lastCellObject;
	const CellProperty*  m_lastCellProperty;
	Vector               m_lastPosition_w;
	ConstWatcher<Object> m_target;
	float                m_speed;
	bool                 m_firstAlter;
	Vector               m_startPosition_w;
	bool                 m_isPaused;

private:

	Projectile();
	Projectile(const Projectile& rhs);
	Projectile& operator=(const Projectile& rhs);

private:

	void updateVelocity();

private:

	Timer                       m_timer;
	bool                        m_createdFireClientEffect;
	const ClientEffectTemplate* m_fireClientEffectTemplate;
	const ClientEffectTemplate* m_missHitNothingClientEffectTemplate;
};

//===================================================================

#endif
