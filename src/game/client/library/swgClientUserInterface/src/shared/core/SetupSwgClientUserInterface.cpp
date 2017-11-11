// ============================================================================
//
// SetupSwgClientUserInterface.h
// copyright (c) 2004 Sony Online Interactive
//
// ============================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SetupSwgClientUserInterface.h"

#include "swgClientUserInterface/ConfigSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiSpaceConversation.h"
#include "swgClientUserInterface/SwgCuiToolbar.h"
#include "swgClientUserInterface/SwgCuiMahjongDataTable.h"
#include "sharedFoundation/ExitChain.h"

// ============================================================================
//
// SetupSwgClientUserInterface
//
// ============================================================================

//-----------------------------------------------------------------------------
void SetupSwgClientUserInterface::install()
{
	ConfigSwgClientUserInterface::install();
	SwgCuiToolbar::install();
	SwgCuiSpaceConversation::install();
	SwgCuiMahjongDataTable::install();
	ExitChain::add(SetupSwgClientUserInterface::remove, "SetupSwgClientUserInterface::remove");
}

//-----------------------------------------------------------------------------
void SetupSwgClientUserInterface::remove()
{
}

// ============================================================================
