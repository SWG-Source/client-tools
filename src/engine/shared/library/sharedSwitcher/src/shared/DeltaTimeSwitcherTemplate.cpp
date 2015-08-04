// ======================================================================
//
// DeltaTimeSwitcherTemplate.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedSwitcher/FirstSharedSwitcher.h"
#include "sharedSwitcher/DeltaTimeSwitcherTemplate.h"

#include "sharedSwitcher/DeltaTimeSwitcher.h"
#include "sharedFile/Iff.h"

// ======================================================================

const Tag TAG_DTST = TAG(D,T,S,T);

// ======================================================================

DeltaTimeSwitcherTemplate::DeltaTimeSwitcherTemplate(Iff &iff)
: SwitcherTemplate(),
	m_frameTime(0),
	m_numberOfStates(0)
{
	iff.enterForm(TAG_DTST);
		iff.enterChunk(TAG_0000);
			m_numberOfStates = iff.read_int32();
			m_frameTime = iff.read_float();
		iff.exitChunk(TAG_0000);
	iff.exitForm(TAG_DTST);
}

// ----------------------------------------------------------------------

DeltaTimeSwitcherTemplate::~DeltaTimeSwitcherTemplate()
{
}

// ----------------------------------------------------------------------

Switcher *DeltaTimeSwitcherTemplate::create() const
{
	return new DeltaTimeSwitcher(m_frameTime, m_numberOfStates);
}

// ======================================================================

