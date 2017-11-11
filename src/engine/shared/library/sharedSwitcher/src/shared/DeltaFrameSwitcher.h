// ======================================================================
//
// DeltaFrameSwitcher.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_DeltaFrameSwitcher_H
#define INCLUDED_DeltaFrameSwitcher_H

// ======================================================================

#include "sharedSwitcher/Switcher.h"

// ======================================================================

class DeltaFrameSwitcher : public Switcher
{
public:

	explicit DeltaFrameSwitcher(int numberOfStates);
	virtual ~DeltaFrameSwitcher();

	virtual bool needsAlter() const;
	virtual void alter(float time);
	virtual int  getState() const;
	virtual int  getNumberOfStates() const;

private:

	/// Disabled.
	DeltaFrameSwitcher();

	/// Disabled.
	DeltaFrameSwitcher(const DeltaFrameSwitcher &);

	/// Disabled.
	DeltaFrameSwitcher &operator =(const DeltaFrameSwitcher &);

private:

	const   int m_numberOfStates;
	mutable int m_currentState;
};

// ======================================================================

#endif
