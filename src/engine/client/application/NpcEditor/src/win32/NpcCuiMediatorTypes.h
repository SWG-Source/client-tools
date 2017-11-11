// ======================================================================
//
// NpcCuiMediatorTypes.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_NpcCuiMediatorTypes_H
#define INCLUDED_NpcCuiMediatorTypes_H

#include "clientUserInterface/CuiMediatorTypes.h"

// ======================================================================

#define MAKE_MEDIATOR_TYPE(a) Type const a = #a

namespace CuiMediatorTypes
{
	MAKE_MEDIATOR_TYPE (Viewer);
}

#undef MAKE_MEDIATOR_TYPE

// ======================================================================

#endif
