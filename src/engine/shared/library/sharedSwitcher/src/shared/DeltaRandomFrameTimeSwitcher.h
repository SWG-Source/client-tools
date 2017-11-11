// ======================================================================
//
// DeltaRandomFrameTimeSwitcher.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_DeltaRandomFrameTimeSwitcher_H
#define INCLUDED_DeltaRandomFrameTimeSwitcher_H

// ======================================================================

#include "sharedSwitcher/Switcher.h"

// ======================================================================

class DeltaRandomFrameTimeSwitcher : public Switcher
{
public:

	DeltaRandomFrameTimeSwitcher(float minimumFrameTime, float maximumFrameTime, int numberOfStates);
	virtual ~DeltaRandomFrameTimeSwitcher();

	virtual bool needsAlter() const;
	virtual void alter(real time);
	virtual int  getState() const;
	virtual int  getNumberOfStates() const;

private:

	/// Disabled.
	DeltaRandomFrameTimeSwitcher();

	/// Disabled.
	DeltaRandomFrameTimeSwitcher(const DeltaRandomFrameTimeSwitcher &);

	/// Disabled.
	DeltaRandomFrameTimeSwitcher &operator =(const DeltaRandomFrameTimeSwitcher &);

private:

	const float m_minimumFrameTime;
	const float m_maximumFrameTime;
	float       m_frameTime;
	float       m_currentTime;

	const int   m_numberOfStates;
	int         m_currentState;
};

// ======================================================================

#endif
