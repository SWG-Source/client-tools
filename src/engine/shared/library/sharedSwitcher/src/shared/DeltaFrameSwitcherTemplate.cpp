// ======================================================================
//
// DeltaFrameSwitcherTemplate.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedSwitcher/FirstSharedSwitcher.h"
#include "sharedSwitcher/DeltaFrameSwitcherTemplate.h"

#include "sharedSwitcher/DeltaFrameSwitcher.h"
#include "sharedFile/Iff.h"

// ======================================================================

const Tag TAG_DFST = TAG(D,F,S,T);

// ======================================================================

DeltaFrameSwitcherTemplate::DeltaFrameSwitcherTemplate(Iff &iff)
: SwitcherTemplate(),
	m_numberOfStates(0)
{
	iff.enterForm(TAG_DFST);
		iff.enterChunk(TAG_0000);
			m_numberOfStates = iff.read_int32();
		iff.exitChunk(TAG_0000);
	iff.exitForm(TAG_DFST);
}

// ----------------------------------------------------------------------

DeltaFrameSwitcherTemplate::~DeltaFrameSwitcherTemplate()
{
}

// ----------------------------------------------------------------------

Switcher *DeltaFrameSwitcherTemplate::create() const
{
	return new DeltaFrameSwitcher(m_numberOfStates);
}

// ======================================================================

