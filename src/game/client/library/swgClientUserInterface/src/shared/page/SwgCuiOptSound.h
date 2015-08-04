//======================================================================
//
// SwgCuiOptSound.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiOptSound_H
#define INCLUDED_SwgCuiOptSound_H

#include "swgClientUserInterface/SwgCuiOptBase.h"

//======================================================================

class SwgCuiOptSound : 
public SwgCuiOptBase
{
public:
	explicit SwgCuiOptSound (UIPage & page);
	~SwgCuiOptSound () {}

	static int  onComboSimGet      (const SwgCuiOptBase & base, const UIComboBox & box);
	static void onComboSimSet      (const SwgCuiOptBase & base, const UIComboBox & box, int index);

private:

	SwgCuiOptSound & operator=(const SwgCuiOptSound & rhs);
	SwgCuiOptSound            (const SwgCuiOptSound & rhs);
};

//======================================================================

#endif
