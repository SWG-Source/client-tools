// ======================================================================
//
// DeltaRandomFrameTimeSwitcherTemplate.h
//
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_DeltaRandomFrameTimeSwitcherTemplate_H
#define INCLUDED_DeltaRandomFrameTimeSwitcherTemplate_H

// ======================================================================

class Iff;
class Switcher;

#include "sharedSwitcher/SwitcherTemplate.h"

// ======================================================================

class DeltaRandomFrameTimeSwitcherTemplate : public SwitcherTemplate
{
public:

	explicit DeltaRandomFrameTimeSwitcherTemplate(Iff &iff);
	virtual ~DeltaRandomFrameTimeSwitcherTemplate();

	virtual Switcher *create() const;

private:

	/// Disabled.
	DeltaRandomFrameTimeSwitcherTemplate();

	/// Disabled.
	DeltaRandomFrameTimeSwitcherTemplate(const DeltaRandomFrameTimeSwitcherTemplate &);

	/// Disabled.
	DeltaRandomFrameTimeSwitcherTemplate &operator =(const DeltaRandomFrameTimeSwitcherTemplate &);

private:

	int m_numberOfStates;
	float m_minimumTime;
	float m_maximumTime;
};

// ======================================================================

#endif
