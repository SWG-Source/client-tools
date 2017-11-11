//======================================================================
//
// SwgCuiOptTerrain.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiOptTerrain_H
#define INCLUDED_SwgCuiOptTerrain_H

#include "swgClientUserInterface/SwgCuiOptBase.h"

//======================================================================

class SwgCuiOptTerrain : 
public SwgCuiOptBase
{
public:
	explicit SwgCuiOptTerrain (UIPage & page);
	~SwgCuiOptTerrain () {}

private:

	SwgCuiOptTerrain & operator=(const SwgCuiOptTerrain & rhs);
	SwgCuiOptTerrain            (const SwgCuiOptTerrain & rhs);
};

//======================================================================

#endif
