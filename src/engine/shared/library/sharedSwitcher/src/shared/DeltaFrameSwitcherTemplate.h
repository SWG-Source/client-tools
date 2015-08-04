// ======================================================================
//
// DeltaFrameDeltaFrameSwitcherTemplate.h
// jeff grills
//
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_DeltaFrameSwitcherTemplate_H
#define INCLUDED_DeltaFrameSwitcherTemplate_H

// ======================================================================

class Iff;
class Switcher;

#include "sharedSwitcher/SwitcherTemplate.h"

// ======================================================================

class DeltaFrameSwitcherTemplate : public SwitcherTemplate
{
public:

	explicit DeltaFrameSwitcherTemplate(Iff &iff);
	virtual ~DeltaFrameSwitcherTemplate();

	virtual Switcher *create() const;

private:

	/// Disabled.
	DeltaFrameSwitcherTemplate();

	/// Disabled.
	DeltaFrameSwitcherTemplate(const DeltaFrameSwitcherTemplate &);

	/// Disabled.
	DeltaFrameSwitcherTemplate &operator =(const DeltaFrameSwitcherTemplate &);

private:

	int m_numberOfStates;
};

// ======================================================================

#endif
