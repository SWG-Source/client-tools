//======================================================================
//
// CuiIconManagerCallback.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiIconManagerCallback.h"

//======================================================================

void CuiIconManagerCallback::overrideCursor  (const ClientObject & , UICursor *& ) const
{
}

//----------------------------------------------------------------------

bool CuiIconManagerCallback::overrideDoubleClick (const UIWidget &) const
{
	return false;
}

//======================================================================
