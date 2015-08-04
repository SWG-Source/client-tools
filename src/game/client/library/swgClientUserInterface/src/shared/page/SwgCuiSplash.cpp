// ======================================================================
//
// SwgCuiSplash.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiSplash.h"

#include "UIClock.h"
#include "UIData.h"
#include "UIManager.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UIText.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/Game.h"
#include "clientGraphics/Graphics.h"
#include "clientUserInterface/ConfigClientUserInterface.h"
#include "clientUserInterface/CuiBackdrop.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "sharedFoundation/ApplicationVersion.h"
#include "sharedGame/PlatformFeatureBits.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"

// ======================================================================

SwgCuiSplash::SwgCuiSplash (UIPage & page) :
CuiMediator      ("SwgCuiSplash", page),
UIEventCallback  (),
UINotification   (),
m_currentPage    (0),
m_timeout        (0.0f)
{
	bool const useJapanese = UIManager::gUIManager().isLocaleJapanese();
#if 0
	bool hasCoa = (Game::getGameFeatureBits () & ClientGameFeature::CompleteOnlineAdventures) != 0;
	if(hasCoa)
	{
		getCodeDataObject(TUIPage, m_pages[0], "0COA");
	}
	else
	{
		//game screen, either japanese, swg, jtl, or ep3
		if(useJapanese)
		{
			getCodeDataObject (TUIPage, m_pages [0], "0JTLJapan");
		}
		else
		{
			getCodeDataObject (TUIPage, m_pages [0], "0SWG");
		}
	}
#else
	// everyone sees the anniversary logo
	getCodeDataObject(TUIPage, m_pages[0], "0ANV");
#endif

	//lucasarts
	getCodeDataObject (TUIPage, m_pages [1], "1");

	//soe
	getCodeDataObject (TUIPage, m_pages [2], "2");

	//only show esrb in english
	if(useJapanese)
	{
		m_pages [3] = NULL;
	}
	else
	{
		getCodeDataObject (TUIPage, m_pages [3], "3");
	}

	UIText * textVersion = 0;
	getCodeDataObject (TUIText, textVersion, "textVersion");

	textVersion->SetPreLocalized (true);
	textVersion->SetLocalText    (Unicode::narrowToWide (ApplicationVersion::getPublicVersion ()));
}

//-----------------------------------------------------------------

SwgCuiSplash::~SwgCuiSplash ()
{
	int i;
	for (i = 0; i < m_numberOfPages; ++i)
		m_pages [i] = 0;

	deactivate ();
}

//-----------------------------------------------------------------

void SwgCuiSplash::performActivate ()
{
	
	if (ConfigClientGame::getSkipSplash())
	{
		proceed();
		return;
	}

	getPage ().SetOpacity (1.0f);
	
	if(m_pages [m_currentPage])
	{
		m_pages [m_currentPage]->SetEnabled (false);
		m_pages [m_currentPage]->SetEnabled (true);
	}

	setPointerInputActive  (true);
	setKeyboardInputActive (true);
	setInputToggleActive   (false);

	getPage ().AddCallback (this);
	UIManager::gUIManager ().DrawCursor (false);

	m_timeout = Game::getElapsedTime () + static_cast<float>(ConfigClientUserInterface::getSplashTimeoutSeconds ());

	UIClock::gUIClock ().ListenPerFrame (this);
}

//-----------------------------------------------------------------

void SwgCuiSplash::performDeactivate ()
{
	setPointerInputActive     (false);
	setKeyboardInputActive    (false);
	setInputToggleActive      (false);

	getPage ().RemoveCallback (this);

	UIClock::gUIClock ().StopListening (this);
}

//----------------------------------------------------------------------

void SwgCuiSplash::Notify      (UINotificationServer *, UIBaseObject *, SwgCuiSplash::Code )
{
	// prevent the splash screen sequence from running while the a modal dialog box is open
	if (CuiMessageBox::getModalMessageBoxCount ())
	{
		m_timeout = Game::getElapsedTime () + static_cast<float>(ConfigClientUserInterface::getSplashTimeoutSeconds ());
		return;
	}

	if (Game::getElapsedTime () > m_timeout)
	{
		//-- reset time per page
		m_timeout = Game::getElapsedTime () + static_cast<float> (ConfigClientUserInterface::getSplashTimeoutSeconds ());

		if (++m_currentPage >= m_numberOfPages)
		{
			//-- if we're on the last page, disable page
			if(m_pages [m_numberOfPages - 1])
				m_pages [m_numberOfPages - 1]->SetEnabled (false);
		}
		else
		{
			//-- switch to the next page
			if(m_pages [m_currentPage])
				m_pages [m_currentPage]->SetEnabled (true);
		}
	}

	//-- wait for the last page to time out
	if (m_currentPage >= m_numberOfPages)
	{
		//if we have a final page, wait for it to fade before proceeding
		if(m_pages [m_numberOfPages - 1])
		{
			if (m_pages [m_numberOfPages - 1]->GetOpacity () == 0.f)
				proceed ();
		}
		else
			proceed ();
	}
}

//-----------------------------------------------------------------

bool SwgCuiSplash::OnMessage         (UIWidget *, const UIMessage & msg)
{
	// start the intro on user input
	if (msg.IsMouseDownCommand () || msg.IsKeyMessage ())
		m_timeout = 0.f;

	return true;
}

//----------------------------------------------------------------------

void SwgCuiSplash::proceed ()
{
	deactivate ();
	CuiMediatorFactory::deactivate (CuiMediatorTypes::Backdrop);

	// -------------------------------------------------------------------------

	Graphics::setViewport (0, 0, Graphics::getCurrentRenderTargetWidth (), Graphics::getCurrentRenderTargetHeight ());
	
	CuiBackdrop * backdrop = dynamic_cast<CuiBackdrop*>(CuiMediatorFactory::get (CuiMediatorTypes::Backdrop));
	NOT_NULL (backdrop);
	backdrop->setRenderData (0, 0);
	
	CuiMediatorFactory::activate (CuiMediatorTypes::Backdrop);

	CuiMediatorFactory::activate ("LoginScreen"); // SwgCuiMediatorTypes::LoginScreen;
	CuiManager::restartMusic ();
}

// ======================================================================
