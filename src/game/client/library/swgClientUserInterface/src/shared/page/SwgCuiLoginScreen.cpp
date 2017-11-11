// ======================================================================
//
// SwgCuiLoginScreen.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiLoginScreen.h"

#include "UIButton.h"
#include "UIData.h"
#include "UIManager.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UITextbox.h"
#include "UnicodeUtils.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/ConnectionManager.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientGame/LoginConnection.h"
#include "clientUserInterface/CuiLoginManager.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiStringIdsServer.h"
#include "clientUserInterface/CuiTransition.h"
#include "sharedFoundation/ApplicationVersion.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/Production.h"
#include "sharedMessageDispatch/Emitter.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/ClientLoginMessages.h"
#include "sharedRandom/Random.h"
#include "swgClientUserInterface/SwgCuiAvatarCreationHelper.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"

// ======================================================================

namespace
{
	namespace UnnamedMessages
	{
		const char * const LoginClientToken      = "LoginClientToken";
	}
}

//----------------------------------------------------------------------

SwgCuiLoginScreen::SwgCuiLoginScreen (UIPage & page) :
CuiMediator       ("SwgCuiLoginScreen", page),
UIEventCallback   (),
MessageDispatch::Receiver(),
m_cancelButton    (0),
m_okButton        (0),
m_devButton       (0),
m_usernameTextbox (0),
m_passwordTextbox (0),
m_messageBox      (0),
m_autoConnected   (false),
m_callback        (new MessageDispatch::Callback),
m_proceed         (false),
m_pageSession     (0),
m_pageNormal      (0),
m_connecting      (false)
{
	getCodeDataObject (TUIButton,  m_devButton,    "LocalButton");
	getCodeDataObject (TUIButton,  m_okButton,     "FinishButton");
	getCodeDataObject (TUIButton,  m_cancelButton, "CancelButton");

	getCodeDataObject (TUITextbox, m_usernameTextbox, "UsernameTextbox");
	getCodeDataObject (TUITextbox, m_passwordTextbox, "PasswordTextbox");

	getCodeDataObject (TUIPage,    m_pageSession,     "pageSession");
	getCodeDataObject (TUIPage,    m_pageNormal,      "pageNormal");

	const char* const sessionId = CuiLoginManager::getSessionIdKey ();

	if (sessionId)
	{
		m_usernameTextbox->SetLocalText (Unicode::emptyString);
		m_passwordTextbox->SetLocalText (Unicode::narrowToWide (sessionId));
	}
	else
	{
		m_passwordTextbox->SetLocalText (Unicode::emptyString);

		if (ConfigClientGame::getLoginClientID ())
			m_usernameTextbox->SetLocalText (Unicode::narrowToWide (ConfigClientGame::getLoginClientID ()));
		else
			m_usernameTextbox->SetLocalText (Unicode::emptyString);
	}

	DEBUG_REPORT_LOG_PRINT (true, ("SwgCuiLoginScreen: username=%s, sessionId=%s\n", ConfigClientGame::getLoginClientID (), sessionId ? sessionId : "null"));

	registerMediatorObject (getPage (),       true);
	registerMediatorObject (*m_devButton,     true);
	registerMediatorObject (*m_okButton,      true);
	registerMediatorObject (*m_cancelButton,  true);
}

//-----------------------------------------------------------------

SwgCuiLoginScreen::~SwgCuiLoginScreen()
{
	delete m_callback;
	m_callback = 0;

	m_cancelButton    = 0;
	m_okButton        = 0;
	m_devButton       = 0;
	m_usernameTextbox = 0;
	m_passwordTextbox = 0;
	m_pageSession     = 0;
	m_pageNormal      = 0;
}

//-----------------------------------------------------------------

void SwgCuiLoginScreen::performActivate ()
{
	SwgCuiAvatarCreationHelper::setCreatingJedi (false);

	GameNetwork::disconnectLoginServer ();

	m_connecting = false;
	m_callback->connect (*this, &SwgCuiLoginScreen::onAvatarListChanged,         static_cast<CuiLoginManager::Messages::AvatarListChanged *>     (0));

	m_proceed = false;

	setPointerInputActive  (true);
	setKeyboardInputActive (true);
	setInputToggleActive   (false);

#if PRODUCTION == 1
	m_devButton->SetVisible (false);
#endif

	connectToMessage (LoginConnection::Messages::LoginConnectionOpened);
	connectToMessage (LoginConnection::Messages::LoginConnectionClosed);
	connectToMessage (LoginConnection::Messages::LoginIncorrectClientId);

	m_usernameTextbox->SetSelected (true);
	m_usernameTextbox->SetFocus ();

	const char* const sessionId = CuiLoginManager::getSessionIdKey ();

	bool const showLoginPrompt = ((sessionId == 0) || ConfigClientGame::getEnableAdminLogin());

	m_pageNormal->SetVisible  (showLoginPrompt);
	m_pageSession->SetVisible (!showLoginPrompt);

	//-- always autoconnect if we are using session authentication
	//-- otherwise only autoconnect if it is the first time through this screen

	if ((!m_autoConnected && ConfigClientGame::getAutoConnectToLoginServer ()) || (sessionId != 0 && !ConfigClientGame::getEnableAdminLogin()))
	{
		m_autoConnected = true;
		ok ();
	}

	//-- todo: handle this and signal the godclient to close when the button is pressed
	if (Game::isGodClient ())
	{
		m_cancelButton->SetEnabled (false);
	}

	setIsUpdating (true);

	CuiTransition::signalTransitionReady (CuiMediatorTypes::LoginScreen);
}

//-----------------------------------------------------------------

void SwgCuiLoginScreen::performDeactivate ()
{
	setIsUpdating (false);

	if (m_messageBox != 0)
		m_messageBox->closeMessageBox ();

	m_callback->disconnect (*this, &SwgCuiLoginScreen::onAvatarListChanged,         static_cast<CuiLoginManager::Messages::AvatarListChanged *>     (0));

	// clear all connections
	disconnectAll();

	m_proceed = false;
}

//-----------------------------------------------------------------

void SwgCuiLoginScreen::OnButtonPressed( UIWidget *context )
{
	m_pageSession->SetVisible (false);

	if (context == m_devButton)
	{
		Game::setSinglePlayer (true);
		CuiTransition::startTransition (CuiMediatorTypes::LoginScreen, CuiMediatorTypes::SceneSelection);
	}
	else if (context == m_okButton)
	{
		ok ();
	}
	else if (context == m_cancelButton)
	{
		deactivate ();
		CuiManager::terminateIoWin ();
	}
}

//----------------------------------------------------------------------

void SwgCuiLoginScreen::ok ()
{
	UIString name;
	UIString passwd;
	
	m_usernameTextbox->GetLocalText (name);
	m_passwordTextbox->GetLocalText (passwd);
	
	const char* const sessionId = CuiLoginManager::getSessionIdKey ();

	if (name.empty () && !sessionId)
	{
		CuiMessageBox::createInfoBox (CuiStringIdsServer::server_err_no_username.localize ());
	}
	else
	{
		m_messageBox = CuiMessageBox::createMessageBox (CuiStringIdsServer::server_connecting_login.localize (), CuiMessageBox::GBT_Cancel);
		m_messageBox->setRunner (true);
		m_messageBox->connectToMessages (*this);
		
		// hook up to the loginserver
		GameNetwork::setUserName        (Unicode::wideToNarrow (name));
		GameNetwork::setUserPassword    (Unicode::wideToNarrow (passwd));

		std::vector<std::pair<std::string, uint16> > loginServerList;

		{
			char buffer [128];
			char const * address = 0;

			int i = 0;
			do
			{
				sprintf (buffer, "loginServerAddress%i", i);
				address = ConfigFile::getKeyString ("ClientGame", buffer, 0, 0);
				if (address)
				{
					sprintf (buffer, "loginServerPort%i", i);
					const uint16 port = static_cast<uint16> (ConfigFile::getKeyInt ("ClientGame", buffer, 0, 0));

					if (port != 0)
					{
						DEBUG_REPORT_LOG (true, ("Found login server: address=%s, port=%i\n", address, port));
						loginServerList.push_back (std::make_pair (std::string (address), port));
					}

					++i;
				}
			} 
			while (address);
		}

		if (loginServerList.size ())
		{
			const int choice = Random::random (loginServerList.size () - 1);
			DEBUG_REPORT_LOG (true, ("Connecting to login server: address=%s, port=%i\n", loginServerList [choice].first.c_str (), loginServerList [choice].second));

			//-- save the choices for later use
			ConfigClientGame::setLoginServerAddress (loginServerList [choice].first);
			ConfigClientGame::setLoginServerPort    (loginServerList [choice].second);

			GameNetwork::connectLoginServer (loginServerList [choice].first, loginServerList [choice].second);
		}
		else
			GameNetwork::connectLoginServer (ConfigClientGame::getLoginServerAddress (), ConfigClientGame::getLoginServerPort ());
		
		m_pageSession->SetVisible (false);

		m_connecting = true;
	}
}

//-----------------------------------------------------------------------

void SwgCuiLoginScreen::receiveMessage(const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message)
{

	//----------------------------------------------------------------------

	if(message.isType (LoginConnection::Messages::LoginConnectionOpened))
	{
		if (m_connecting)
		{
			if (m_messageBox != 0)
			{
				// login server established connection
				m_messageBox->setText (CuiStringIdsServer::server_login_cxn_opened.localize ());
				
				// listen for the token from the server, will disable message box when it's received
				connectToEmitter(source, UnnamedMessages::LoginClientToken);
			}
		}
	}

	//----------------------------------------------------------------------

	else if(message.isType (LoginConnection::Messages::LoginConnectionClosed))
	{
		if (m_connecting)
		{
			if (m_pageNormal->IsVisible ())
			{
				if (m_messageBox != 0)
				{
					m_messageBox->setText (CuiStringIdsServer::server_login_cxn_failed.localize ());
					m_messageBox->setRunner (false);
				}
			}
			else
			{
				if (m_messageBox)
					m_messageBox->closeMessageBox ();

				m_pageSession->SetVisible (true);
			}

			m_connecting = false;
		}
	}
	//----------------------------------------------------------------------

	else if (message.isType (LoginConnection::Messages::LoginIncorrectClientId))
	{
#if PRODUCTION != 1
		if (m_connecting)
		{
			if (m_messageBox != 0)
			{
				Archive::ReadIterator ri (NON_NULL (safe_cast<const GameNetworkMessage *> (&message))->getByteStream ().begin ());
				LoginIncorrectClientId errorMsg (ri);
				
				m_messageBox->setText (Unicode::narrowToWide ("Your network version does not match the server's network version.\n\nClient network version: " + GameNetworkMessage::NetworkVersionId + "\nClient application version: " + ApplicationVersion::getInternalVersion() + "\n\nServer network version: " + errorMsg.getServerId() + "\nServer application version: " + errorMsg.getServerApplicationVersion()));
				m_messageBox->setRunner (false);
			}

			m_connecting = false;
		}
#endif
	}

	//----------------------------------------------------------------------

	else if(message.isType(UnnamedMessages::LoginClientToken))
	{
		if (m_connecting)
		{
		}
	}
	
	//----------------------------------------------------------------------

	else
	{
		const CuiMessageBox::BoxMessage * const abm = dynamic_cast<const CuiMessageBox::BoxMessage *>(&message);
		
		if (abm && abm->getMessageBox () == m_messageBox)
		{
			const CuiMessageBox::CompletedMessage * const cm = dynamic_cast<const CuiMessageBox::CompletedMessage *>(abm);
			
			if (cm)
			{
				if (m_connecting)
					m_connecting = false;
			}
			else
			{
				if (message.isType (CuiMessageBox::Messages::CLOSED))
					m_messageBox = 0;
			}

			if (!m_pageNormal->IsVisible ())
				m_pageSession->SetVisible (true);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiLoginScreen::onAvatarListChanged (bool)
{
	if (m_connecting)
	{
		m_proceed = true;
	}
}

//----------------------------------------------------------------------

void SwgCuiLoginScreen::update (float deltaTimeSecs)
{
	CuiMediator::update (deltaTimeSecs);

	if (m_proceed)
	{
		m_proceed = false;
		Game::setSinglePlayer (false);
		CuiTransition::startTransition (CuiMediatorTypes::LoginScreen, CuiMediatorTypes::AvatarSelection);
	}
}

//-----------------------------------------------------------------
