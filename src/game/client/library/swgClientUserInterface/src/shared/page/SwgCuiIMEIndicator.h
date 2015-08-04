//======================================================================
//
// SwgCuiIMEInput.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiIMEIndicator_H
#define INCLUDED_SwgCuiIMEIndicator_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"

//----------------------------------------------------------------------

class SwgCuiIMEIndicator :
public CuiMediator
{
public:

	SwgCuiIMEIndicator     (UIPage & page);
	
protected:

	~SwgCuiIMEIndicator    ();

private:

	SwgCuiIMEIndicator ();
	SwgCuiIMEIndicator (const SwgCuiIMEIndicator & rhs);
	SwgCuiIMEIndicator operator= (const SwgCuiIMEIndicator & rhs);
};

//======================================================================

#endif
