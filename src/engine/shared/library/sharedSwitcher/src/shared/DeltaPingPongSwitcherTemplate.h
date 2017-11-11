// ======================================================================
//
// DeltaPingPongSwitcherTemplate.h
//
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_DeltaPingPongSwitcherTemplate_H
#define INCLUDED_DeltaPingPongSwitcherTemplate_H

// ======================================================================

class Iff;
class Switcher;

#include "sharedSwitcher/SwitcherTemplate.h"

// ======================================================================

class DeltaPingPongSwitcherTemplate : public SwitcherTemplate
{
public:

	explicit DeltaPingPongSwitcherTemplate(Iff &iff);
	virtual ~DeltaPingPongSwitcherTemplate();

	virtual Switcher *create() const;

private:

	/// Disabled.
	DeltaPingPongSwitcherTemplate();

	/// Disabled.
	DeltaPingPongSwitcherTemplate(const DeltaPingPongSwitcherTemplate &);

	/// Disabled.
	DeltaPingPongSwitcherTemplate &operator =(const DeltaPingPongSwitcherTemplate &);

private:

	int m_numberOfStates;
	float m_minimumTime;
	float m_maximumTime;
};

// ======================================================================

#endif
