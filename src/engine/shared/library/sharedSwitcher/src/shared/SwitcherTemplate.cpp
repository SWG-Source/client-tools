// ======================================================================
//
// SwitcherTemplate.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "sharedSwitcher/FirstSharedSwitcher.h"
#include "sharedSwitcher/SwitcherTemplate.h"

#include "sharedSwitcher/DeltaFrameSwitcherTemplate.h"
#include "sharedSwitcher/DeltaRandomFrameSwitcherTemplate.h"
#include "sharedSwitcher/DeltaRandomFrameTimeSwitcherTemplate.h"
#include "sharedSwitcher/DeltaRandomTimeSwitcherTemplate.h"
#include "sharedSwitcher/DeltaPingPongSwitcherTemplate.h"
#include "sharedSwitcher/DeltaTimeSwitcherTemplate.h"
#include "sharedFile/Iff.h"

// ======================================================================

const Tag TAG_DFST = TAG(D,F,S,T);
const Tag TAG_DTST = TAG(D,T,S,T);
const Tag TAG_DRFS = TAG(D,R,F,S);
const Tag TAG_DRTS = TAG(D,R,T,S);
const Tag TAG_DRFT = TAG(D,R,F,T);
const Tag TAG_DPPT = TAG(D,P,P,T);

// ======================================================================

SwitcherTemplate::SwitcherTemplate()
{
}

// ----------------------------------------------------------------------

SwitcherTemplate::~SwitcherTemplate()
{
}

// ----------------------------------------------------------------------

SwitcherTemplate *SwitcherTemplate::create(Iff &iff)
{
	switch (iff.getCurrentName())
	{
		case TAG_DFST:
			return new DeltaFrameSwitcherTemplate(iff);

		case TAG_DRFS:
			return new DeltaRandomFrameSwitcherTemplate(iff);

		case TAG_DRTS:
			return new DeltaRandomTimeSwitcherTemplate(iff);

		case TAG_DTST:
			return new DeltaTimeSwitcherTemplate(iff);

		case TAG_DRFT:
			return new DeltaRandomFrameTimeSwitcherTemplate(iff);

		case TAG_DPPT:
			return new DeltaPingPongSwitcherTemplate(iff);

		default:
			break;
	}

	char buffer[256];
	iff.formatLocation(buffer, sizeof(buffer));
	DEBUG_FATAL(true, ("Unknown SwitcherTemplate type %s", buffer));
	return NULL;
}

// ======================================================================

