// ======================================================================
//
// SwgCuiGameMenu.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiGameMenu.h"

#include "UIButton.h"
#include "UIData.h"
#include "UIPage.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientUserInterface/CuiCachedAvatarManager.h"
#include "clientUserInterface/CuiLoginManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "sharedFoundation/Production.h"
#include "swgClientUserInterface/SwgCuiHudFactory.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"

// ======================================================================

SwgCuiGameMenu::SwgCuiGameMenu (UIPage & page) :
CuiMediator        ("SwgCuiGameMenu", page),
UIEventCallback    (),
m_resumeButton     (0),
m_disconnectButton (0),
m_exitGameButton   (0),
m_locationsButton  (0),
m_buttonCredits    (0)
{
	getCodeDataObject (TUIButton,  m_resumeButton,         "ResumeButton");
	getCodeDataObject (TUIButton,  m_disconnectButton,     "DisconnectButton");
	getCodeDataObject (TUIButton,  m_exitGameButton,       "ExitButton");
	getCodeDataObject (TUIButton,  m_locationsButton,      "locationsButton");
	getCodeDataObject (TUIButton,  m_buttonCredits,        "buttonCredits");

	registerMediatorObject (*m_resumeButton,     true);
	registerMediatorObject (*m_exitGameButton,   true);
	registerMediatorObject (*m_buttonCredits,    true);
	registerMediatorObject (*m_disconnectButton, true);

#if PRODUCTION == 1
	m_locationsButton->SetVisible (false);
#else
	registerMediatorObject (*m_locationsButton,  true);
#endif
}

//-----------------------------------------------------------------

SwgCuiGameMenu::~SwgCuiGameMenu ()
{
	m_resumeButton     = 0;
	m_disconnectButton = 0;
	m_exitGameButton   = 0;
	m_locationsButton  = 0;
	m_buttonCredits    = 0;
}

//-----------------------------------------------------------------

void SwgCuiGameMenu::performActivate ()
{
	setPointerInputActive  (true);
//	setKeyboardInputActive (true);
	setInputToggleActive   (false);


	//-- todo: handle this
	if (Game::isGodClient ())
	{
		m_exitGameButton->SetEnabled (false);
	}
}

//-----------------------------------------------------------------

void SwgCuiGameMenu::performDeactivate ()
{
	setPointerInputActive  (false);
//	setKeyboardInputActive (false);
	setInputToggleActive   (true);	

	m_resumeButton->RemoveCallback     (this);
	m_disconnectButton->RemoveCallback (this);
	m_exitGameButton->RemoveCallback   (this);
	m_locationsButton->RemoveCallback   (this);
}

//----------------------------------------------------------------------

bool SwgCuiGameMenu::close ()
{
	SwgCuiHudFactory::setHudActive (true);
	deactivate ();
	return true;
}

//-----------------------------------------------------------------

void SwgCuiGameMenu::OnButtonPressed   (UIWidget *context)
{
	UNREF (context);

	NOT_NULL (context);

	if (context == m_resumeButton)
	{
		close ();
	}
	else if (context == m_exitGameButton)
	{
		if (!Game::isGodClient ())
		{
			CuiCachedAvatarManager::saveScenePlayer ();		
			Game::quit ();
		}
	}
	else if (context == m_disconnectButton)
	{
		if (Game::getScene  ())
		{
			CuiCachedAvatarManager::saveScenePlayer ();
			CuiLoginManager::disconnectFromCluster ();
			Game::cleanupScene();
		}

		CuiMediatorFactory::activate (CuiMediatorTypes::Backdrop);
		if (Game::getSinglePlayer ())
			CuiMediatorFactory::activate (CuiMediatorTypes::SceneSelection);
		else
		{
			GameNetwork::disconnectConnectionServer ();
			CuiMediatorFactory::activate (CuiMediatorTypes::AvatarSelection);
		}

		deactivate ();
	}
	else if (context == m_buttonCredits)
	{
		CuiMediatorFactory::activate (CuiMediatorTypes::Credits);
		deactivate ();
	}

	else if (context == m_locationsButton)
	{
		CuiMediatorFactory::activate (CuiMediatorTypes::Locations);
		deactivate ();
	}
}

// ======================================================================

