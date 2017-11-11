//=============================================================================
//
// SwgCuiSpaceButtonBar.h
// copyright (c) 2004 Sony Online Entertainment
//
//=============================================================================

#ifndef INCLUDED_SwgCuiSpaceButtonBar_H
#define INCLUDED_SwgCuiSpaceButtonBar_H

//=============================================================================

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

//-----------------------------------------------------------------------------

class SwgCuiSpaceButtonBar :
public CuiMediator,
public UIEventCallback
{
public:
	SwgCuiSpaceButtonBar(UIPage & page);

protected:
	virtual ~SwgCuiSpaceButtonBar();

private:
	SwgCuiSpaceButtonBar();
	SwgCuiSpaceButtonBar(const SwgCuiSpaceButtonBar &);
	SwgCuiSpaceButtonBar & operator=(const SwgCuiSpaceButtonBar &);
};

//=============================================================================

#endif
