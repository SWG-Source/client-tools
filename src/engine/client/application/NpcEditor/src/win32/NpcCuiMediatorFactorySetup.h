// ======================================================================
//
// NpcCuiMediatorFactorySetup.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_NpcCuiMediatorFactorySetup_H
#define INCLUDED_NpcCuiMediatorFactorySetup_H

#include "UITypes.h"

// ======================================================================
class NpcCuiMediatorFactorySetup
{
public:

	static void install ();
	static void remove  ();

private: //-- disabled

	NpcCuiMediatorFactorySetup(const NpcCuiMediatorFactorySetup & rhs);
	NpcCuiMediatorFactorySetup & operator=(const NpcCuiMediatorFactorySetup & rhs);
	NpcCuiMediatorFactorySetup();
};

// ======================================================================

#endif
