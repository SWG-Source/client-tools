// ======================================================================
//
// DeltaRandomFrameSwitcherTemplate.h
// jeff grills
//
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_DeltaRandomFrameSwitcherTemplate_H
#define INCLUDED_DeltaRandomFrameSwitcherTemplate_H

// ======================================================================

class Iff;
class Switcher;

#include "sharedSwitcher/SwitcherTemplate.h"

// ======================================================================

class DeltaRandomFrameSwitcherTemplate : public SwitcherTemplate
{
public:

	explicit DeltaRandomFrameSwitcherTemplate(Iff &iff);
	virtual ~DeltaRandomFrameSwitcherTemplate();

	virtual Switcher *create() const;

private:

	/// Disabled.
	DeltaRandomFrameSwitcherTemplate();

	/// Disabled.
	DeltaRandomFrameSwitcherTemplate(const DeltaRandomFrameSwitcherTemplate &);

	/// Disabled.
	DeltaRandomFrameSwitcherTemplate &operator =(const DeltaRandomFrameSwitcherTemplate &);

private:

	int m_numberOfStates;
};

// ======================================================================

#endif
