// ======================================================================
//
// DeltaTimeDeltaTimeSwitcherTemplate.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_DeltaTimeSwitcherTemplate_H
#define INCLUDED_DeltaTimeSwitcherTemplate_H

// ======================================================================

class Iff;
class Switcher;

#include "sharedSwitcher/SwitcherTemplate.h"

// ======================================================================

class DeltaTimeSwitcherTemplate : public SwitcherTemplate
{
public:

	explicit DeltaTimeSwitcherTemplate(Iff &iff);
	virtual ~DeltaTimeSwitcherTemplate();

	virtual Switcher *create() const;

private:

	/// Disabled.
	DeltaTimeSwitcherTemplate();

	/// Disabled.
	DeltaTimeSwitcherTemplate(const DeltaTimeSwitcherTemplate &);

	/// Disabled.
	DeltaTimeSwitcherTemplate &operator =(const DeltaTimeSwitcherTemplate &);

private:

	float m_frameTime;
	int   m_numberOfStates;
};

// ======================================================================

#endif
