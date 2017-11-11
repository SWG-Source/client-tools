// ======================================================================
//
// DeltaRandomFrameSwitcher.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_DeltaRandomFrameSwitcher_H
#define INCLUDED_DeltaRandomFrameSwitcher_H

// ======================================================================

#include "sharedSwitcher/Switcher.h"

// ======================================================================

class DeltaRandomFrameSwitcher : public Switcher
{
public:

	explicit DeltaRandomFrameSwitcher(int numberOfStates);
	virtual ~DeltaRandomFrameSwitcher();

	virtual bool needsAlter() const;
	virtual void alter(float time);
	virtual int  getState() const;
	virtual int  getNumberOfStates() const;

private:

	/// Disabled.
	DeltaRandomFrameSwitcher();

	/// Disabled.
	DeltaRandomFrameSwitcher(const DeltaRandomFrameSwitcher &);

	/// Disabled.
	DeltaRandomFrameSwitcher &operator =(const DeltaRandomFrameSwitcher &);

private:

	const   int m_numberOfStates;
	mutable int m_currentState;
};

// ======================================================================

#endif
