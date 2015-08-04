// ============================================================================
//
// NpcCuiManager.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ============================================================================

#include "FirstNpcEditor.h"
#include "NpcCuiManager.h"
#include "NpcCuiMediatorFactorySetup.h"

// ============================================================================

namespace
{
	bool s_installed;
}

// ----------------------------------------------------------------------------

void NpcCuiManager::install ()
{
	DEBUG_FATAL (s_installed, ("already installed\n"));

	NpcCuiMediatorFactorySetup::install ();

	s_installed = true;
}

// ----------------------------------------------------------------------------

void NpcCuiManager::remove ()
{
	DEBUG_FATAL (!s_installed, ("not installed\n"));

	NpcCuiMediatorFactorySetup::remove ();

	s_installed = false;
}

// ----------------------------------------------------------------------------

bool NpcCuiManager::test (std::string const & result)
{
	UNREF (result);

	return false;
}

// ----------------------------------------------------------------------------

void NpcCuiManager::update (float deltaTimeSecs)
{
	UNREF (deltaTimeSecs);
}

// ============================================================================
