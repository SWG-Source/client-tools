// ======================================================================
//
// DeltaPingPongSwitcher.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_DeltaPingPongSwitcher_H
#define INCLUDED_DeltaPingPongSwitcher_H

// ======================================================================

#include "sharedSwitcher/Switcher.h"

// ======================================================================

class DeltaPingPongSwitcher : public Switcher
{
public:

	DeltaPingPongSwitcher(float minimumFrameTime, float maximumFrameTime, int numberOfStates);
	virtual ~DeltaPingPongSwitcher();

	virtual bool needsAlter() const;
	virtual void alter(real time);
	virtual int  getState() const;
	virtual int  getNumberOfStates() const;

private:

	/// Disabled.
	DeltaPingPongSwitcher();

	/// Disabled.
	DeltaPingPongSwitcher(const DeltaPingPongSwitcher &);

	/// Disabled.
	DeltaPingPongSwitcher &operator =(const DeltaPingPongSwitcher &);

private:

	const float m_minimumFrameTime;
	const float m_maximumFrameTime;
	float       m_frameTime;
	float       m_currentTime;

	const int   m_numberOfStates;
	int         m_currentState;
    int         m_stateDelta;
};

// ======================================================================

#endif
