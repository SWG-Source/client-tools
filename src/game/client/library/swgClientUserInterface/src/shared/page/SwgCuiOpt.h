// ======================================================================
//
// SwgCuiOpt.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiOpt_H
#define INCLUDED_SwgCuiOpt_H


#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

class UIPage;
class UIButton;
class SwgCuiOptBase;
class UITabbedPane;

// ======================================================================

class SwgCuiOpt :
public CuiMediator,
public UIEventCallback
{
public:

	static void              gameOptionChanged();

public:

	explicit                 SwgCuiOpt            (UIPage & page);

	//- PS UI support
	virtual void             OnButtonPressed      (UIWidget *Context);
	virtual void             OnTabbedPaneChanged  (UIWidget *Context);

	virtual void             performActivate      ();
	virtual void             performDeactivate    ();

	bool                     close                ();

private:
	virtual                 ~SwgCuiOpt            ();
	                         SwgCuiOpt            ();
	                         SwgCuiOpt            (const SwgCuiOpt & rhs);
	SwgCuiOpt &              operator=            (const SwgCuiOpt & rhs);

private:
	
	typedef stdmap<int, SwgCuiOptBase *>::fwd BaseMap;
	BaseMap *            m_optionPages;	
	UITabbedPane *       m_tabs;

	UIButton *           m_buttonOk;
	UIButton *           m_buttonCancel;

	bool                 m_keepSettings;
};

// ======================================================================

#endif

