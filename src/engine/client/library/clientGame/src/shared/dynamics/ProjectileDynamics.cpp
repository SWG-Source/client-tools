// ======================================================================
//
// ProjectileDynamics.cpp
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ProjectileDynamics.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(ProjectileDynamics, true, 0, 0, 0);

// ======================================================================

ProjectileDynamics::ProjectileDynamics(Object * const owner) : 
	SimpleDynamics(owner)
{
}

// ----------------------------------------------------------------------

ProjectileDynamics::~ProjectileDynamics()
{
}

// ======================================================================
