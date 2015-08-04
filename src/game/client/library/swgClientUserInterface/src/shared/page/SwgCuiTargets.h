//======================================================================
//
// SwgCuiTargets.h
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiTargets_H
#define INCLUDED_SwgCuiTargets_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"

#include "UIEventCallback.h"

//----------------------------------------------------------------------

class UIPage;
class UIScrollbar;
class UIVolumePage;
class SwgCuiMfdStatus;
class UIButton;

//----------------------------------------------------------------------

class SwgCuiTargets :
public CuiMediator,
public UIEventCallback
{
public:
	explicit            SwgCuiTargets (UIPage & page);

	void                update         (float deltaTimeSecs);

	void                OnButtonPressed (UIWidget *context);

protected:
	void                performActivate ();
	void                performDeactivate ();

private:
	                   ~SwgCuiTargets ();

	                    SwgCuiTargets ();
	                    SwgCuiTargets (const SwgCuiTargets &);
	SwgCuiTargets &     operator= (const SwgCuiTargets &);

private:	
	class SwgCuiTargetsAction;
	SwgCuiTargetsAction *  m_action;

	SwgCuiMfdStatus *      m_mfdStatus;

	UIButton *             m_buttonCollapse;
	UIButton *             m_buttonExpand;

	UIPage *               m_pageToggle;

	int m_sceneType; // Game::SceneType
};

//======================================================================

#endif
