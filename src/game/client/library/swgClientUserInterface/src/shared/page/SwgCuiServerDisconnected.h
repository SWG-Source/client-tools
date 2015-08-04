//======================================================================
//
// SwgCuiServerDisconnected.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiServerDisconnected_H
#define INCLUDED_SwgCuiServerDisconnected_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"

//----------------------------------------------------------------------

class SwgCuiServerDisconnected :
public CuiMediator
{
public:

	SwgCuiServerDisconnected     (UIPage & page);

protected:

	~SwgCuiServerDisconnected    ();

	void                         performActivate   ();
	void                         performDeactivate ();

private:

	SwgCuiServerDisconnected ();
	SwgCuiServerDisconnected (const SwgCuiServerDisconnected & rhs);
	SwgCuiServerDisconnected operator= (const SwgCuiServerDisconnected & rhs);
};

//======================================================================

#endif
