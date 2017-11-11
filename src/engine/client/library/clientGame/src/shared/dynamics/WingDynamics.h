// ======================================================================
//
// WingDynamics.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_WingDynamics_H
#define INCLUDED_WingDynamics_H

// ======================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Timer.h"
#include "sharedObject/Dynamics.h"
#include "sharedMath/Transform.h"

class SoundTemplate;

// ======================================================================

class WingDynamics : public Dynamics
{
public:

	WingDynamics(Object * newOwner, float rotation, float rotationTime, SoundTemplate const * soundTemplate);
	virtual ~WingDynamics();

	virtual float alter(float elapsedTime);

private:

	WingDynamics();
	WingDynamics(WingDynamics const &);
	WingDynamics & operator=(WingDynamics const &);

private:

	bool m_currentState;
	bool m_desiredState;
	Transform const m_initialTransform_p;
	float m_rotation;
	Timer m_timer;
	bool m_transitioning;
	SoundTemplate const * const m_soundTemplate;
};

// ======================================================================

#endif
