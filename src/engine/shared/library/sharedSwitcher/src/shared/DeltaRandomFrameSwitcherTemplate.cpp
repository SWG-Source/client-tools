// ======================================================================
//
// DeltaRandomFrameSwitcherTemplate.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedSwitcher/FirstSharedSwitcher.h"
#include "sharedSwitcher/DeltaRandomFrameSwitcherTemplate.h"

#include "sharedSwitcher/DeltaRandomFrameSwitcher.h"
#include "sharedFile/Iff.h"

// ======================================================================

const Tag TAG_DRFS = TAG(D,R,F,S);

// ======================================================================

DeltaRandomFrameSwitcherTemplate::DeltaRandomFrameSwitcherTemplate(Iff &iff)
: SwitcherTemplate(),
	m_numberOfStates(0)
{
	iff.enterForm(TAG_DRFS);
		iff.enterChunk(TAG_0000);
			m_numberOfStates = iff.read_int32();
		iff.exitChunk(TAG_0000);
	iff.exitForm(TAG_DRFS);
}

// ----------------------------------------------------------------------

DeltaRandomFrameSwitcherTemplate::~DeltaRandomFrameSwitcherTemplate()
{
}

// ----------------------------------------------------------------------

Switcher *DeltaRandomFrameSwitcherTemplate::create() const
{
	return new DeltaRandomFrameSwitcher(m_numberOfStates);
}

// ======================================================================

