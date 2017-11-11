//======================================================================
//
// SwgCuiIMEInput.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiIMEIndicatorSpace_H
#define INCLUDED_SwgCuiIMEIndicatorSpace_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"

//----------------------------------------------------------------------

class SwgCuiIMEIndicatorSpace :
public CuiMediator
{
public:

	SwgCuiIMEIndicatorSpace     (UIPage & page);
	
protected:

	~SwgCuiIMEIndicatorSpace    ();

private:

	SwgCuiIMEIndicatorSpace ();
	SwgCuiIMEIndicatorSpace (const SwgCuiIMEIndicatorSpace & rhs);
	SwgCuiIMEIndicatorSpace operator= (const SwgCuiIMEIndicatorSpace & rhs);
};

//======================================================================

#endif
