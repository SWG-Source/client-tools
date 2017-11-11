// ======================================================================
//
// SwgCuiMediatorFactorySetup.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiMediatorFactorySetup_H
#define INCLUDED_SwgCuiMediatorFactorySetup_H

#include "UITypes.h"

// ======================================================================
class SwgCuiMediatorFactorySetup
{
public:

	static void install ();
	static void remove  ();

private:
	SwgCuiMediatorFactorySetup (const SwgCuiMediatorFactorySetup & rhs);
	SwgCuiMediatorFactorySetup & operator= (const SwgCuiMediatorFactorySetup & rhs);
};

// ======================================================================

#endif
