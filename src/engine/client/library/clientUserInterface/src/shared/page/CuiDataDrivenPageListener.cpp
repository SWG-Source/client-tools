// ======================================================================
//
// CuiDataDrivenPage.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiDataDrivenPageListener.h"

#include "UIButton.h"
#include "UIWidget.h"
#include "clientUserInterface/CuiDataDrivenPage.h"
#include "clientUserInterface/CuiMediator.h"
#include "sharedGame/SuiEventType.h"

// ======================================================================

CuiDataDrivenPageListener::CuiDataDrivenPageListener(CuiDataDrivenPage* owner, UIButton *okButton, UIButton *cancelButton, UIButton *closeButton) :
	m_owner(owner),
	m_okButton(okButton),
	m_cancelButton(cancelButton),
	m_closeButton(closeButton)
{
}

// ======================================================================

CuiDataDrivenPageListener::~CuiDataDrivenPageListener()
{
	m_owner = 0;
	m_okButton = 0;
	m_cancelButton = 0;
}

// ======================================================================

void CuiDataDrivenPageListener::OnShow( UIWidget *Context )
{
	m_owner->onEvent(SuiEventType::SET_onVisibilityChanged, Context);
}

// ======================================================================

void CuiDataDrivenPageListener::OnHide( UIWidget *Context )
{
	m_owner->onEvent(SuiEventType::SET_onVisibilityChanged, Context);
}

// ======================================================================

void CuiDataDrivenPageListener::OnEnable( UIWidget *Context )
{
	m_owner->onEvent(SuiEventType::SET_onEnabledChanged, Context);
}

// ======================================================================

void CuiDataDrivenPageListener::OnDisable( UIWidget *Context )
{
	m_owner->onEvent(SuiEventType::SET_onEnabledChanged, Context);
}

// ======================================================================

void CuiDataDrivenPageListener::OnButtonPressed( UIWidget *Context )
{
	bool doClose = false;
	if (Context == m_okButton)
	{
		if(!m_owner->isClosed())
		{
			m_owner->onEvent(SuiEventType::SET_onClosedOk, NULL);
			m_owner->setClosed(true);
			doClose = true;
		}
	}
	else if (Context == m_cancelButton || Context == m_closeButton)
	{
		if(!m_owner->isClosed())
		{
			m_owner->onEvent(SuiEventType::SET_onClosedCancel, NULL);
			m_owner->setClosed(true);
			doClose = true;
		}
	}
	
	if (doClose)
	{
		IGNORE_RETURN(m_owner->closeThroughWorkspace());
	}
	else
		m_owner->onEvent(SuiEventType::SET_onButton, Context);
}

// ======================================================================

void CuiDataDrivenPageListener::OnCheckboxSet( UIWidget *Context )
{
	m_owner->onEvent(SuiEventType::SET_onCheckbox, Context);
}

// ======================================================================

void CuiDataDrivenPageListener::OnCheckboxUnset( UIWidget *Context )
{
	m_owner->onEvent(SuiEventType::SET_onCheckbox, Context);
}

// ======================================================================

void CuiDataDrivenPageListener::OnSliderbarChanged( UIWidget *Context )
{
	m_owner->onEvent(SuiEventType::SET_onSliderbar, Context);
}

// ======================================================================

void CuiDataDrivenPageListener::OnTabbedPaneChanged (UIWidget * Context)
{
	m_owner->onEvent(SuiEventType::SET_onTabbedPane, Context);
}

// ======================================================================

void CuiDataDrivenPageListener::OnVolumePageSelectionChanged (UIWidget * context)
{
	m_owner->onEvent(SuiEventType::SET_onGenericSelection, context);
}

// ======================================================================

void CuiDataDrivenPageListener::OnGenericSelectionChanged (UIWidget * context)
{
	m_owner->onEvent(SuiEventType::SET_onGenericSelection, context);
}

// ======================================================================
