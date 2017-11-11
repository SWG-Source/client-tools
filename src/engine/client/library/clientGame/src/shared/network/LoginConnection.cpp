// LoginConnection.cpp
// copyright 2000 Verant Interactive
// Author: Justin Randall


//-----------------------------------------------------------------------

#include "clientGame/FirstClientGame.h"
#include "clientGame/LoginConnection.h"

#include "Archive/ByteStream.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/ConnectionManager.h"
#include "clientGame/GameNetwork.h"
#include "clientUserInterface/CuiLoginManager.h"
#include "sharedNetworkMessages/ClientLoginMessages.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"

//-----------------------------------------------------------------------

const char * const LoginConnection::Messages::LoginConnectionOpened  = "LoginConnectionOpened";
const char * const LoginConnection::Messages::LoginConnectionClosed  = "LoginConnectionClosed";
const char * const LoginConnection::Messages::LoginIncorrectClientId = "LoginIncorrectClientId";

//----------------------------------------------------------------------

LoginConnection::LoginConnection(const std::string & a, const unsigned short p, const NetworkSetupData & setupData) :
GameNetworkConnection(a, p, setupData)
{
}

//-----------------------------------------------------------------------

LoginConnection::~LoginConnection()
{
}

//-----------------------------------------------------------------------

void LoginConnection::onConnectionClosed()
{
	GameNetworkConnection::onConnectionClosed();

	// emit a message for Receiver objects specifically listening for
	// a LoginConnection to close
	static MessageConnectionCallback m("LoginConnectionClosed"); 
	emitMessage(m);
}

//-----------------------------------------------------------------------

void LoginConnection::onConnectionOpened()
{
	GameNetworkConnection::onConnectionOpened();

	CuiLoginManager::setOverrideSessionIdKey(std::string());

	// Don't send the user name if using station authentication, unless 
	// using the admin login feature.  When using the admin login feature,
	// the name represents the account we want to use.  Don't send the 
	// name during regular operation, or the Login Server might think we're
	// trying to hack into the admin login feature.
	bool sendUserName = true;
	if (CuiLoginManager::getSessionIdKey() && !ConfigClientGame::getEnableAdminLogin())
		sendUserName = false;

	LoginClientId	id(sendUserName ? GameNetwork::getUserName() : "", GameNetwork::getUserPassword());
	send(id, true);

#if PRODUCTION != 1
	GenericValueTypeMessage< int > msg( "RequestExtendedClusterInfo", 0 );
	send( msg, true );
#endif

	// emit a message for Receiver objects specifically listening for
	// a LoginConnection to open
	static MessageConnectionCallback m("LoginConnectionOpened"); 
	emitMessage(m);
}

//-----------------------------------------------------------------------

void LoginConnection::onConnectionOverflowing(const unsigned int bytesPending)
{
	GameNetworkConnection::onConnectionOverflowing(bytesPending);
}

//-----------------------------------------------------------------------
