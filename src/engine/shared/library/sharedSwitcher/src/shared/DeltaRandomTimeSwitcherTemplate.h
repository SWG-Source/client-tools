// ======================================================================
//
// DeltaRandomTimeDeltaRandomTimeSwitcherTemplate.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_DeltaRandomTimeSwitcherTemplate_H
#define INCLUDED_DeltaRandomTimeSwitcherTemplate_H

// ======================================================================

class Iff;
class Switcher;

#include "sharedSwitcher/SwitcherTemplate.h"

// ======================================================================

class DeltaRandomTimeSwitcherTemplate : public SwitcherTemplate
{
public:

	explicit DeltaRandomTimeSwitcherTemplate(Iff &iff);
	virtual ~DeltaRandomTimeSwitcherTemplate();

	virtual Switcher *create() const;

private:

	/// Disabled.
	DeltaRandomTimeSwitcherTemplate();

	/// Disabled.
	DeltaRandomTimeSwitcherTemplate(const DeltaRandomTimeSwitcherTemplate &);

	/// Disabled.
	DeltaRandomTimeSwitcherTemplate &operator =(const DeltaRandomTimeSwitcherTemplate &);

private:

	float m_minimumTime;
	float m_maximumTime;
	int   m_numberOfStates;
};

// ======================================================================

#endif
