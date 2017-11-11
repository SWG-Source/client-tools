// ======================================================================
//
// ProjectileDynamics.h
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_ProjectileDynamics_H
#define INCLUDED_ProjectileDynamics_H

// ======================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"
#include "sharedObject/SimpleDynamics.h"
#include "sharedMath/Vector.h"

// ======================================================================

class ProjectileDynamics : public SimpleDynamics
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	explicit ProjectileDynamics(Object * owner);
	virtual ~ProjectileDynamics();

private:

	ProjectileDynamics();
	ProjectileDynamics(ProjectileDynamics const &);
	ProjectileDynamics & operator=(ProjectileDynamics const &);
};

// ======================================================================

#endif
