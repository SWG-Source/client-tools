// ============================================================================
// 
// AsteroidDynamics.h
// Copyright Sony Online Entertainment, Inc.
//
// ============================================================================

#ifndef INCLUDED_AsteroidDynamics_H
#define INCLUDED_AsteroidDynamics_H

//===================================================================

#include "sharedObject/SimpleDynamics.h"
#include "sharedMath/Vector.h"

//===================================================================

/**
 * A dynamics that causes an object to rotate while moving in a straight line
 */
class AsteroidDynamics : public SimpleDynamics
{
public:

	AsteroidDynamics          (Object* newOwner, const Vector & velocity_w, const Vector& rotationsPerSecond);
	virtual ~AsteroidDynamics ();

	virtual float alter (float time);
	
private:

	AsteroidDynamics ();
	AsteroidDynamics (const AsteroidDynamics&);
	AsteroidDynamics& operator= (const AsteroidDynamics&);

private:
	const Vector  m_rotationsPerSecond;
	bool          m_ignoreFirstFrame;
};

//===================================================================

#endif


