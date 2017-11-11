//======================================================================
//
// SwgCuiOptControls.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiOptControls_H
#define INCLUDED_SwgCuiOptControls_H

#include "swgClientUserInterface/SwgCuiOptBase.h"
#include "UIButton.h"

//======================================================================

class SwgCuiOptControls : 
public SwgCuiOptBase
{
public:
	explicit SwgCuiOptControls (UIPage & page);
	~SwgCuiOptControls () {}

	virtual void             OnButtonPressed    (UIWidget * context);

private:

	SwgCuiOptControls & operator=(const SwgCuiOptControls & rhs);
	SwgCuiOptControls            (const SwgCuiOptControls & rhs);

	UIButton*					  m_reaquireJoysticks;
};

//======================================================================

#endif
