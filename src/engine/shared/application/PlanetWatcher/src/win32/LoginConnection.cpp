// LoginConnection.cpp
// copyright 2000 Verant Interactive
// Author: Justin Randall


//-----------------------------------------------------------------------

#include "FirstPlanetWatcher.h"
#include "ClusterSelectionDialog.h"
#include "LoginConnection.h"
#include "PlanetWatcherUtility.h"

#include "Archive/ByteStream.h"
#include "sharedNetworkMessages/ClientLoginMessages.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/LoginEnumCluster.h"
#include "sharedNetworkMessages/LoginClusterStatus.h"

//-----------------------------------------------------------------------

LoginConnection::LoginConnection(
								 ClusterSelectionDialog *dlg,
								 const std::string & a,
								 const unsigned short p,
								 const NetworkSetupData & setupData,
								 const std::string &username,
								 const std::string &password ) :
Connection(a, p, setupData),
m_clusterSelectionDlg( dlg ),
m_username( username ),
m_password( password )
{
	m_clusterSelectionDlg->setStatusText( QString().sprintf( "Status: connecting to %s:%d", a.c_str(), p ) );
}

//-----------------------------------------------------------------------

LoginConnection::~LoginConnection()
{
}

//-----------------------------------------------------------------------

void LoginConnection::onConnectionClosed()
{
	m_clusterSelectionDlg->loginServerConnectionStatusUpdate( false );

	PlanetWatcherUtility::report( QString().sprintf( "Connection to login server closed: %s", getDisconnectReason().c_str() ) );
}

//-----------------------------------------------------------------------

void LoginConnection::onConnectionOpened()
{
	m_clusterSelectionDlg->loginServerConnectionStatusUpdate( true );

	PlanetWatcherUtility::report( QString().sprintf( "Connection to login server opened." ) );


#if 0
	// Don't send the user name if using station authentication, unless 
	// using the admin login feature.  When using the admin login feature,
	// the name represents the account we want to use.  Don't send the 
	// name during regular operation, or the Login Server might think we're
	// trying to hack into the admin login feature.
	bool sendUserName = true;
	if (CuiLoginManager::getSessionIdKey() && !ConfigClientGame::getEnableAdminLogin())
		sendUserName = false;
#endif

//	LoginClientId	id(sendUserName ? GameNetwork::getUserName() : "", GameNetwork::getUserPassword());

	m_clusterSelectionDlg->setStatusText( "Status: authenticating..." );

	Archive::ByteStream s;
	LoginClientId( m_username.c_str(), m_password.c_str() ).pack(s);
	send( s, true );

}

//-----------------------------------------------------------------------

void LoginConnection::onConnectionOverflowing(const unsigned int bytesPending)
{
	Connection::onConnectionOverflowing(bytesPending);
}

//-----------------------------------------------------------------------

void LoginConnection::onReceive( const Archive::ByteStream &message )
{
	Archive::ReadIterator ri = message.begin();
	GameNetworkMessage m(ri);

	m_clusterSelectionDlg->setStatusText( "" );

	if ( m.isType( "LoginEnumCluster" ) )
	{
		Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&m))->getByteStream ().begin ();
		m_clusterSelectionDlg->onLoginMessage( LoginEnumCluster( ri ) );
	}
	else if ( m.isType( "LoginClusterStatus" ) )
	{
		Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&m))->getByteStream ().begin ();
		m_clusterSelectionDlg->onLoginMessage( LoginClusterStatus( ri ) );
	}

}

//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
