//======================================================================
//
// CuiWorkspaceIcon.h
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiWorkspaceIcon_H
#define INCLUDED_CuiWorkspaceIcon_H

//======================================================================
#include "UIPage.h"
#include "UIEventCallback.h"

class CuiWorkspace;
class CuiMediator;

class CuiWorkspaceIcon : 
public UIPage, 
public UIEventCallback
{
public:
	struct PropertyName
	{
	};
	
	virtual UIBaseObject *   Clone( void ) const { return 0; }
	virtual	UIStyle *        GetStyle( void ) const;

	virtual bool             ProcessMessage( const UIMessage & );
	
	                         CuiWorkspaceIcon (CuiMediator * mediator);
	virtual                  ~CuiWorkspaceIcon ();

	void                     setWorkspace (CuiWorkspace * workspace);
	void                     OnShow (UIWidget *Context);

private:
	CuiWorkspaceIcon ();
	                         CuiWorkspaceIcon (const CuiWorkspaceIcon & rhs);
	CuiWorkspaceIcon &       operator=    (const CuiWorkspaceIcon & rhs);

	CuiMediator *            m_mediator;
	CuiWorkspace *           m_workspace;

	bool            m_mouseDown;
	UIPoint         m_mouseDownLocation;
	UIPoint         m_originalLocation;

private:

};
//-----------------------------------------------------------------

inline UIStyle * CuiWorkspaceIcon::GetStyle( void ) const
{
	return 0;
}
//----------------------------------------------------------------------

inline void CuiWorkspaceIcon::setWorkspace (CuiWorkspace * workspace)
{
	m_workspace = workspace;
}
//======================================================================

#endif
