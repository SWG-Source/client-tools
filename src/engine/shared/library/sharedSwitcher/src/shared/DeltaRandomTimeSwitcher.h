// ======================================================================
//
// DeltaRandomTimeSwitcher.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_DeltaRandomTimeSwitcher_H
#define INCLUDED_DeltaRandomTimeSwitcher_H

// ======================================================================

#include "sharedSwitcher/Switcher.h"

// ======================================================================

class DeltaRandomTimeSwitcher : public Switcher
{
public:

	DeltaRandomTimeSwitcher(float minimumFrameTime, float maximumFrameTime, int numberOfStates);
	virtual ~DeltaRandomTimeSwitcher();

	virtual bool needsAlter() const;
	virtual void alter(real time);
	virtual int  getState() const;
	virtual int  getNumberOfStates() const;

private:

	/// Disabled.
	DeltaRandomTimeSwitcher();

	/// Disabled.
	DeltaRandomTimeSwitcher(const DeltaRandomTimeSwitcher &);

	/// Disabled.
	DeltaRandomTimeSwitcher &operator =(const DeltaRandomTimeSwitcher &);

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
