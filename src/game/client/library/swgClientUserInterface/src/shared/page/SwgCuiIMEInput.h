//======================================================================
//
// SwgCuiIMEInput.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiIMEInput_H
#define INCLUDED_SwgCuiIMEInput_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"

#include "UIEventCallback.h"

class UIList;

//----------------------------------------------------------------------

class SwgCuiIMEInput :
public CuiMediator,
UIEventCallback
{
public:

	SwgCuiIMEInput     (UIPage & page);

	virtual bool OnMessage ( UIWidget *, const UIMessage & msg );
	
protected:

	~SwgCuiIMEInput    ();

	void                         performActivate   ();
	void                         performDeactivate ();

private:

	SwgCuiIMEInput ();
	SwgCuiIMEInput (const SwgCuiIMEInput & rhs);
	SwgCuiIMEInput operator= (const SwgCuiIMEInput & rhs);

	UIList * m_listBox;
};

//======================================================================

#endif
