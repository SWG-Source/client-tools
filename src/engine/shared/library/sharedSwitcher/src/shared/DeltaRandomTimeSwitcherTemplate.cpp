// ======================================================================
//
// DeltaRandomTimeSwitcherTemplate.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedSwitcher/FirstSharedSwitcher.h"
#include "sharedSwitcher/DeltaRandomTimeSwitcherTemplate.h"

#include "sharedSwitcher/DeltaRandomTimeSwitcher.h"
#include "sharedFile/Iff.h"

// ======================================================================

const Tag TAG_DRTS = TAG(D,R,T,S);

// ======================================================================

DeltaRandomTimeSwitcherTemplate::DeltaRandomTimeSwitcherTemplate(Iff &iff)
: SwitcherTemplate(),
	m_minimumTime(0),
	m_maximumTime(0),
	m_numberOfStates(0)
{
	iff.enterForm(TAG_DRTS);
		iff.enterChunk(TAG_0000);
			m_numberOfStates = iff.read_int32();
			m_minimumTime = iff.read_float();
			m_maximumTime = iff.read_float();
		iff.exitChunk(TAG_0000);
	iff.exitForm(TAG_DRTS);
}

// ----------------------------------------------------------------------

DeltaRandomTimeSwitcherTemplate::~DeltaRandomTimeSwitcherTemplate()
{
}

// ----------------------------------------------------------------------

Switcher *DeltaRandomTimeSwitcherTemplate::create() const
{
	return new DeltaRandomTimeSwitcher(m_minimumTime, m_maximumTime, m_numberOfStates);
}

// ======================================================================

