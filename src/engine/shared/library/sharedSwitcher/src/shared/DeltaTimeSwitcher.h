// ======================================================================
//
// DeltaTimeSwitcher.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_DeltaTimeSwitcher_H
#define INCLUDED_DeltaTimeSwitcher_H

// ======================================================================

#include "sharedSwitcher/Switcher.h"

// ======================================================================

class DeltaTimeSwitcher : public Switcher
{
public:

	DeltaTimeSwitcher(float time, int numberOfStates);
	virtual ~DeltaTimeSwitcher();

	virtual bool needsAlter() const;
	virtual void alter(float time);
	virtual int  getState() const;
	virtual int  getNumberOfStates() const;

private:

	/// Disabled.
	DeltaTimeSwitcher();

	/// Disabled.
	DeltaTimeSwitcher(const DeltaTimeSwitcher &);

	/// Disabled.
	DeltaTimeSwitcher &operator =(const DeltaTimeSwitcher &);

private:

	const float m_frameTime;
	float       m_currentTime;

	const int   m_numberOfStates;
	int         m_currentState;
};

// ======================================================================

#endif
