// ============================================================================
//
// ClusterSelectionDialog.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstPlanetWatcher.h"
#include "ClusterSelectionDialog.h"
#include "ClusterSelectionDialog.moc"
#include "LoginConnection.h"
#include "PlanetWatcherUtility.h"
#include "sharedNetwork/NetworkSetupData.h"
#include "sharedNetworkMessages/LoginClusterStatus.h"
#include "sharedNetworkMessages/LoginEnumCluster.h"

#include <vector>
#include <fstream>
#include <memory>

using namespace std;

namespace
{
	enum Column
	{
		Column_Name,
		Column_Population,
		Column_Status,
		Column_Version
	};

	struct ClusterInfo
	{
		LoginClusterStatus::ClusterData m_data;
		QListViewItem*                  m_item;
	};

	typedef map< int, ClusterInfo > ClusterInfoMap;

	ClusterInfoMap s_clusterInfo;

	const UINT s_registryQueryDefaultBufferSize = 1024;
	const char *s_registryKey = "Software\\Sony Online Entertainment\\StarWarsGalaxies\\Planetwatcher";
	const ushort s_defaultLoginServerPort = 44453;


	// --------------------------------------------------------------------------

	// save username and server name in win32 registry
	void saveSettings( const string &user, const string &server )
	{
		HKEY key = 0;

		LONG result = RegCreateKey(
			HKEY_CURRENT_USER,
			s_registryKey,
			&key );

		if ( key && result == ERROR_SUCCESS )
		{
			RegSetValueEx(
				key,
				"Username",
				0,
				REG_SZ,
				(uchar *)user.c_str(),
				user.size() );

			RegSetValueEx(
				key,
				"LoginServer",
				0,
				REG_SZ,
				(uchar *)server.c_str(),
				server.size() );

			RegCloseKey( key );

		}
	}

	// --------------------------------------------------------------------------

	// load username and server name from win32 registry
	void loadSettings( string &user, string &server )
	{
		HKEY key = 0;

		LONG result = RegOpenKey(
			HKEY_CURRENT_USER,
			s_registryKey,
			&key );

		if ( result == ERROR_SUCCESS && key )
		{
			char *buffer = new char[ s_registryQueryDefaultBufferSize ];
			DWORD bufsiz = s_registryQueryDefaultBufferSize;
			DWORD type   = REG_SZ;

			result = RegQueryValueEx(
				key,
				"Username",
				0,
				&type,
				reinterpret_cast< uchar * >( buffer ),
				&bufsiz );
			
			user = buffer;

			type   = REG_SZ;
			bufsiz = s_registryQueryDefaultBufferSize;

			result = RegQueryValueEx(
				key,
				"LoginServer",
				0,
				&type,
				reinterpret_cast< uchar * >( buffer ),
				&bufsiz );

			server = buffer;

			RegCloseKey( key );

			delete [] buffer;
		}
	}

}
///////////////////////////////////////////////////////////////////////////////
//
// ClusterSelectionDialog
//
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
ClusterSelectionDialog::ClusterSelectionDialog(QWidget *parent, char const *name)
 : BaseClusterSelectionDialog(parent, name),
 m_loginConnection( 0 )
{
	connect( m_connectButton,      SIGNAL( clicked () ), this, SLOT( slotConnectButtonClicked()));
	connect( m_connectLoginServer, SIGNAL( clicked () ), this, SLOT( slotConnectLoginServer()));
	connect( m_cancelButton,       SIGNAL( clicked () ), this, SLOT( close() ) );

	string user;
	string server;
	loadSettings( user, server );

	m_username->setText( user.c_str() );
	m_loginAddress->setText( server.c_str() );
	
}

//-----------------------------------------------------------------------------

ClusterSelectionDialog::~ClusterSelectionDialog()
{
	m_loginConnection->disconnect();
}

//-----------------------------------------------------------------------------

void ClusterSelectionDialog::slotConnectLoginServer()
{
	NetworkSetupData setupData;
	setupData.useTcp = false;

	// if we have an active connection, attempt to disconnect it 
	
	if ( m_loginConnection ) 
	{
		m_loginConnection->disconnect();
		m_loginConnection = 0;
	}

	// attempt to connect to the login server

	// parse the login server address

	string address( m_loginAddress->text() );

	// determine if a port is specified

	string::size_type colon = address.find(":");
	ushort port = s_defaultLoginServerPort;

	// if there is a port, get the port out

	if ( colon != string::npos )
	{
		port = atoi( address.substr( colon + 1 ).c_str() );
		address = address.substr( 0, colon );
	}

	m_loginConnection = new LoginConnection(
		this,
		address.c_str(),
		port,
		setupData,
		string( m_username->text() ),
		string( m_password->text() ) );

	// serialize the provided username

	saveSettings( string( m_username->text() ), string( m_loginAddress->text() ) );

}

//-----------------------------------------------------------------------------

void ClusterSelectionDialog::slotConnectButtonClicked()
{
	accept();
}

//-----------------------------------------------------------------------------

void ClusterSelectionDialog::slotCancelButtonClicked()
{
	close();
}

//-----------------------------------------------------------------------------

void ClusterSelectionDialog::keyPressEvent(QKeyEvent * /*keyEvent*/ )
{
}

//-----------------------------------------------------------------------------

void ClusterSelectionDialog::onLoginMessage( const LoginEnumCluster &lec )
{

	typedef vector<LoginEnumCluster::ClusterData> Data;

	const Data &data = lec.getData();

	m_clusterList->clear();

	for ( Data::const_iterator it = data.begin(); it != data.end(); ++it )
	{
		QListViewItem *item = new QListViewItem( m_clusterList, (*it).m_clusterName.c_str() );
		item->setText( Column_Status, "unknown" );
		item->setText( Column_Population, "?" );
		item->setText( Column_Version, "?" );

		ClusterInfo info;
		info.m_item = item;


		s_clusterInfo[ (*it).m_clusterId ] = info;
	}
	
}

//-----------------------------------------------------------------------------

void ClusterSelectionDialog::onLoginMessage( const LoginClusterStatus &lcs )
{
	typedef vector<LoginClusterStatus::ClusterData> Data;

	const Data &data = lcs.getData();

	for ( Data::const_iterator it = data.begin(); it != data.end(); ++it )
	{
		const LoginClusterStatus::ClusterData &cd = *it;

		if ( s_clusterInfo.find( cd.m_clusterId ) != s_clusterInfo.end() )
		{
			QListViewItem *item = s_clusterInfo[ cd.m_clusterId ].m_item;
			s_clusterInfo[ cd.m_clusterId ].m_data = cd;

			item->setText( Column_Population, QString().sprintf( "%d", cd.m_populationOnline ) );
			
			switch ( cd.m_status )
			{
			case LoginClusterStatus::ClusterData::S_down:
				item->setText( Column_Status, "down" );
				break;
			case LoginClusterStatus::ClusterData::S_loading:
				item->setText( Column_Status, "loading" );
				break;
			case LoginClusterStatus::ClusterData::S_up:
				item->setText( Column_Status, "up" );
				break;
			case LoginClusterStatus::ClusterData::S_locked:
				item->setText( Column_Status, "locked" );
				break;
			case LoginClusterStatus::ClusterData::S_restricted:
				item->setText( Column_Status, "restricted" );
				break;
			case LoginClusterStatus::ClusterData::S_full:
				item->setText( Column_Status, "full" );
				break;
			default:
				item->setText( Column_Status, "*unknown state*" );
				break;
			}
		}
	}
}

//-----------------------------------------------------------------------------

void ClusterSelectionDialog::setStatusText( const char * text )
{
	m_status->setText( text );
}

//-----------------------------------------------------------------------------

int ClusterSelectionDialog::getSelectedCluster() const
{
	ClusterInfoMap::const_iterator it = s_clusterInfo.begin();

	for ( ; it != s_clusterInfo.end(); ++it )
	{
		const QListViewItem *item = (*it).second.m_item;

		if ( item->isSelected() == true )
		{
			return (*it).first;
		}
	}

	return -1;
}

//-----------------------------------------------------------------------------

const LoginClusterStatus_ClusterData &ClusterSelectionDialog::getClusterData( int clusterId ) const
{
	ClusterInfoMap::const_iterator it = s_clusterInfo.find( clusterId );

	if ( it == s_clusterInfo.end() )
	{
		throw std::bad_exception( "unable to find cluster information" );
	}

	return (*it).second.m_data;

}

//-----------------------------------------------------------------------------

void ClusterSelectionDialog::loginServerConnectionStatusUpdate( bool /*connected*/ )
{
}

// ============================================================================


