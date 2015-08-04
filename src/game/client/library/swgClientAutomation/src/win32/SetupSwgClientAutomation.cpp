// ============================================================================
//
// SetupSwgClientAutomation.h
// copyright (c) 2005 Sony Online Entertainment
//
// ============================================================================

#include "swgClientAutomation/FirstSwgClientAutomation.h"
#include "swgClientAutomation/SetupSwgClientAutomation.h"
#include "ClientAutomation.h"

#include "sharedFoundation/ExitChain.h"


// ============================================================================
//
// SetupSwgClientAutomation
//
// ============================================================================


//-----------------------------------------------------------------------------

void SetupSwgClientAutomation::install()
{
	ExitChain::add(SetupSwgClientAutomation::remove, "SetupSwgClientAutomation::remove");

	ClientAutomation::install();
}

//-----------------------------------------------------------------------------
void SetupSwgClientAutomation::remove()
{
	ClientAutomation::remove();
}

// ============================================================================
