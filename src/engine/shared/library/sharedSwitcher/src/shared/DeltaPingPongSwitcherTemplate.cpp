// ======================================================================
//
// DeltaPingPongSwitcherTemplate.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "sharedSwitcher/FirstSharedSwitcher.h"
#include "sharedSwitcher/DeltaPingPongSwitcherTemplate.h"

#include "sharedSwitcher/DeltaPingPongSwitcher.h"
#include "sharedFile/Iff.h"

// ======================================================================

const Tag TAG_DPPT = TAG(D,P,P,T);

// ======================================================================

DeltaPingPongSwitcherTemplate::DeltaPingPongSwitcherTemplate(Iff &iff)
: SwitcherTemplate(),
	m_numberOfStates(0),
    m_minimumTime(0.f),
	m_maximumTime(0.f)

{
	iff.enterForm(TAG_DPPT);
		iff.enterChunk(TAG_0000);
			m_numberOfStates = iff.read_int32();
			m_minimumTime = iff.read_float();
			m_maximumTime = iff.read_float();
		iff.exitChunk(TAG_0000);
	iff.exitForm(TAG_DPPT);
}

// ----------------------------------------------------------------------

DeltaPingPongSwitcherTemplate::~DeltaPingPongSwitcherTemplate()
{
}

// ----------------------------------------------------------------------

Switcher *DeltaPingPongSwitcherTemplate::create() const
{
	return new DeltaPingPongSwitcher(m_minimumTime, m_maximumTime, m_numberOfStates);
}

// ======================================================================

