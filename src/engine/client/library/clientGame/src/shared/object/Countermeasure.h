//===================================================================
//
// Countermeasure.h
//
// copyright 2004, Sony Online Entertainment
//
//===================================================================

#ifndef INCLUDED_Countermeasure_H
#define INCLUDED_Countermeasure_H

//===================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Timer.h"
#include "sharedObject/Object.h"

//===================================================================

class AppearanceTemplate;
class ClientEffectTemplate;

//===================================================================

class Countermeasure : public Object
{
public:
	Countermeasure            (Vector const & sourceLocation, Vector const & initialDirection, AppearanceTemplate const * const appearance, float const velocity, float const maxScatterAngle);
	virtual ~Countermeasure   ();

	virtual float alter       (float elapsedTime);

private:

	Timer m_expirationTimer;
	float const m_velocity;
	
private:
	Countermeasure(const Countermeasure&); //disable
	Countermeasure &operator=(const Countermeasure&); //disable
};

//===================================================================

#endif
