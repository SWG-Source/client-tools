//======================================================================
//
// CuiWorkspaceIcon.cpp
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiWorkspaceIcon.h"

#include "UIMessage.h"
#include "UIWidgetRectangleStyles.h"
#include "UnicodeUtils.h"
#include "clientUserInterface/CuiWorkspace.h"

//======================================================================

CuiWorkspaceIcon::CuiWorkspaceIcon (CuiMediator * mediator) :
UIPage (),
UIEventCallback (), //lint !e1769
m_mediator (NON_NULL (mediator)),
m_workspace (0),
m_mouseDown (false),
m_mouseDownLocation (),
m_originalLocation ()
{
}

//----------------------------------------------------------------------

CuiWorkspaceIcon::~CuiWorkspaceIcon ()
{
	m_mediator = 0;
	m_workspace = 0;
}

//----------------------------------------------------------------------

void CuiWorkspaceIcon::OnShow (UIWidget *)
{
	if (GetParent ())
		IGNORE_RETURN (SetProperty (UIWidgetRectangleStyles::PropertyName::Default, Unicode::narrowToWide ("/styles.icon.style")));
}

//----------------------------------------------------------------------

bool CuiWorkspaceIcon::ProcessMessage( const UIMessage & msg)
{
	if (!msg.IsMouseMessage ())
		return false;

	if (msg.Type == UIMessage::LeftMouseDown)
	{		
		m_mouseDown = true;
		m_mouseDownLocation = msg.MouseCoords;
		m_originalLocation = GetLocation ();
		UIBaseObject * const parent = NON_NULL (GetParent ());
		IGNORE_RETURN (parent->MoveChild (this, UIBaseObject::Top));

		return true;
	}
	else if (msg.Type == UIMessage::LeftMouseUp)
	{
		m_mouseDown = false;
		SetActivated (false);
	}
	else if (msg.Type == UIMessage::LeftMouseDoubleClick)
	{
		m_workspace->restoreFromIcon (this, *NON_NULL(m_mediator));
	}
	else if (m_mouseDown && msg.Type == UIMessage::MouseMove)
	{
		if (m_workspace)
		{ 
			UIPoint diff = msg.MouseCoords - m_mouseDownLocation;
			m_workspace->moveIcon (*this, GetLocation () + diff);
			return true;
		}

		return false;
	}
	
	return false;
}
//----------------------------------------------------------------------

//======================================================================
