// ======================================================================
//
// CuiMediator_MediatorEventCallback.cpp
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiMediator_MediatorEventCallback.h"

#include "UIButton.h"
#include "UICheckbox.h"
#include "clientUserInterface/CuiWorkspace.h"

// ======================================================================

CuiMediator::MediatorEventCallback::MediatorEventCallback (CuiMediator & mediator) :
UIEventCallback (),
m_mediator (mediator)
{		
}

//-----------------------------------------------------------------

void CuiMediator::MediatorEventCallback::OnButtonPressed (UIWidget *context)
{
	if (context == m_mediator.m_buttonClose)
	{
		m_mediator.closeThroughWorkspace ();
	}
	else if (context == m_mediator.m_buttonMaximizeRestore)
	{
		if (m_mediator.m_containingWorkspace)
		{
			if (m_mediator.isMaximized ())
				m_mediator.m_containingWorkspace->restore (m_mediator);
			else
				m_mediator.m_containingWorkspace->maximize (m_mediator);
		}
		else if (m_mediator.isMaximizable ())
		{
			m_mediator.maximizeThroughWorkspace ();
		}
	}
	else if (context == m_mediator.m_buttonMinimize)
	{
		if (m_mediator.m_containingWorkspace)
			m_mediator.m_containingWorkspace->iconify (m_mediator);
	}
}

//----------------------------------------------------------------------

void CuiMediator::MediatorEventCallback::OnCheckboxSet (UIWidget * context)
{
	if (context == m_mediator.m_checkboxPopupHelp)
	{
		m_mediator.startPopupHelp ();
	}
}

//----------------------------------------------------------------------

void CuiMediator::MediatorEventCallback::OnCheckboxUnset (UIWidget * context)
{
	if (context == m_mediator.m_checkboxPopupHelp)
	{
		m_mediator.stopPopupHelp ();
	}
}

// ======================================================================
