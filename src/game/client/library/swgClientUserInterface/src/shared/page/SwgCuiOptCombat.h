//======================================================================
//
// SwgCuiOptCombat.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiOptCombat_H
#define INCLUDED_SwgCuiOptCombat_H

#include "swgClientUserInterface/SwgCuiOptBase.h"

//======================================================================

class SwgCuiOptCombat : 
public SwgCuiOptBase
{
public:
	explicit SwgCuiOptCombat (UIPage & page);
	~SwgCuiOptCombat () {}

private:

	SwgCuiOptCombat & operator=(const SwgCuiOptCombat & rhs);
	SwgCuiOptCombat            (const SwgCuiOptCombat & rhs);
};

//======================================================================

#endif
