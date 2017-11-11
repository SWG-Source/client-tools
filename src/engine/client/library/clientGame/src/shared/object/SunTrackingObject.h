//===================================================================
//
// SunTrackingObject.h
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_SunTrackingObject_H
#define INCLUDED_SunTrackingObject_H

//===================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"
#include "sharedObject/Object.h"

//===================================================================

class SunTrackingObject : public Object
{
public:

	explicit SunTrackingObject (float yawMaximumRadiansPerSecond);
	virtual ~SunTrackingObject ();

	virtual float alter (float elapsedTime);

	void          setBarrel (Object* object, float pitchMinimumRadians, float pitchMaximumRadians, float pitchMaximumRadiansPerSecond);
	void          setState (bool on);

private:

	SunTrackingObject ();
	SunTrackingObject (const SunTrackingObject&);
	SunTrackingObject& operator= (const SunTrackingObject&);

private:

	float                             m_yawMaximumRadiansPerSecond;
	Object*                           m_barrel;
	float                             m_pitchMinimumRadians;
	float                             m_pitchMaximumRadians;
	float                             m_pitchMaximumRadiansPerSecond;
	bool                              m_on;
};

//===================================================================

#endif
