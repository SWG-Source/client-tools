//======================================================================
//
// SwgCuiOptMisc.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiOptMisc_H
#define INCLUDED_SwgCuiOptMisc_H

#include "swgClientUserInterface/SwgCuiOptBase.h"

//======================================================================

class SwgCuiOptMisc : 
public SwgCuiOptBase
{
public:
	explicit SwgCuiOptMisc (UIPage & page);
	~SwgCuiOptMisc () {};

	virtual void update (float deltaTimeSecs);

protected:
	virtual void performActivate();
	virtual void performDeactivate();

private:
	SwgCuiOptMisc & operator=(const SwgCuiOptMisc & rhs);
	SwgCuiOptMisc            (const SwgCuiOptMisc & rhs);
};


//======================================================================

#endif
