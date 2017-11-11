// ======================================================================
//
// DeltaRandomFrameTimeSwitcherTemplate.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "sharedSwitcher/FirstSharedSwitcher.h"
#include "sharedSwitcher/DeltaRandomFrameTimeSwitcherTemplate.h"

#include "sharedSwitcher/DeltaRandomFrameTimeSwitcher.h"
#include "sharedFile/Iff.h"

// ======================================================================

const Tag TAG_DRFT = TAG(D,R,F,T);

// ======================================================================

DeltaRandomFrameTimeSwitcherTemplate::DeltaRandomFrameTimeSwitcherTemplate(Iff &iff)
: SwitcherTemplate(),
	m_numberOfStates(0),
	m_minimumTime(0.f),
	m_maximumTime(0.f)

{
	iff.enterForm(TAG_DRFT);
		iff.enterChunk(TAG_0000);
			m_numberOfStates = iff.read_int32();
			m_minimumTime = iff.read_float();
			m_maximumTime = iff.read_float();
		iff.exitChunk(TAG_0000);
	iff.exitForm(TAG_DRFT);
}

// ----------------------------------------------------------------------

DeltaRandomFrameTimeSwitcherTemplate::~DeltaRandomFrameTimeSwitcherTemplate()
{
}

// ----------------------------------------------------------------------

Switcher *DeltaRandomFrameTimeSwitcherTemplate::create() const
{
	return new DeltaRandomFrameTimeSwitcher(m_minimumTime, m_maximumTime, m_numberOfStates);
}

// ======================================================================

