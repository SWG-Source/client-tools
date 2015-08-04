// ======================================================================
//
// SwgCuiGameMenu.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiGameMenu_H
#define INCLUDED_SwgCuiGameMenu_H

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

class UIPage;
class UIButton;

// ======================================================================

class SwgCuiGameMenu :
public CuiMediator,
public UIEventCallback
{
public:

	explicit                 SwgCuiGameMenu (UIPage & page);

	virtual void             OnButtonPressed   (UIWidget *context);

	virtual void             performActivate   ();
	virtual void             performDeactivate ();

	bool                     close             ();

private:
	virtual                 ~SwgCuiGameMenu ();
	                         SwgCuiGameMenu ();
	                         SwgCuiGameMenu (const SwgCuiGameMenu & rhs);
	SwgCuiGameMenu &         operator=      (const SwgCuiGameMenu & rhs);

private:

	UIButton *               m_resumeButton;
	UIButton *               m_disconnectButton;
	UIButton *               m_exitGameButton;
	UIButton *               m_locationsButton;
	UIButton *               m_buttonCredits;
};

// ======================================================================

#endif
