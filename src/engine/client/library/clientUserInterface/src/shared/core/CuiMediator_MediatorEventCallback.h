// ======================================================================
//
// CuiMediator_MediatorEventCallback.h
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CuiMediator_MediatorEventCallback_H
#define INCLUDED_CuiMediator_MediatorEventCallback_H

// ======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

//-----------------------------------------------------------------

class CuiMediator::MediatorEventCallback :
public UIEventCallback
{
public:

	MediatorEventCallback (CuiMediator & mediator);
	void OnButtonPressed (UIWidget *context);
	void OnCheckboxSet   (UIWidget *context);
	void OnCheckboxUnset (UIWidget *context);

private:

	MediatorEventCallback ();
	MediatorEventCallback (const MediatorEventCallback & rhs);
	MediatorEventCallback & operator= (const MediatorEventCallback & rhs);

	CuiMediator & m_mediator;
};

// ======================================================================

#endif
