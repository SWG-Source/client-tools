// LoginConnection.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstSwgLoadClient.h"
#include "Archive/ByteStream.h"
#include "Client.h"
#include "ConfigSwgLoadClient.h"
#include "LoginConnection.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/ClientLoginMessages.h"
#include "sharedNetworkMessages/ClientCentralMessages.h"
#include "sharedNetworkMessages/ErrorMessage.h"
#include "sharedNetworkMessages/LoginClusterStatus.h"
#include "sharedNetworkMessages/LoginEnumCluster.h"
#include "SwgLoadClient.h"
#include "UnicodeUtils.h"

//-----------------------------------------------------------------------

LoginConnection::LoginConnection(Client * o, const std::string & i, const std::string & a, const unsigned short p, const NetworkSetupData &setupData) :
LoadConnection(a, p, setupData),
	m_createCharacter(false),
	m_clusterId(0),
	m_gameAddress(""),
	m_gamePort(0),
	m_loginId(i),
	m_owner(o)
{
	REPORT_LOG(true, ("[%s] logging into LoginServer at %s:%i\n", i.c_str(), a.c_str(), p));
}

//-----------------------------------------------------------------------

LoginConnection::~LoginConnection()
{
	m_owner->onConnectionClosed(this);
}

//-----------------------------------------------------------------------

void LoginConnection::onConnectionClosed()
{
}

//-----------------------------------------------------------------------

void LoginConnection::onConnectionOpened()
{
	LoginClientId id(m_loginId, "LOADCLIENT");
	send(id, true);
	REPORT_LOG(true, ("[%s] sent LoginClientId message\n", m_loginId.c_str()));
}

//-----------------------------------------------------------------------

void LoginConnection::onReceive(const Archive::ByteStream & data)
{
	Archive::ReadIterator ri(data);
	GameNetworkMessage base(ri);

	ri = data.begin();

	if(base.isType("LoginClientToken"))
	{
		m_owner->setLoginClientToken(new LoginClientToken(ri));
		REPORT_LOG(true, ("[%s] received LoginClientToken\n", m_loginId.c_str()));
	}
	else if(base.isType("LoginEnumCluster"))
	{
		LoginEnumCluster ec(ri);

		REPORT_LOG(true, ("[%s] received LoginEnumCluster message... ", m_loginId.c_str()));
		// is the cluster we are targeting in the list?
		std::vector<LoginEnumCluster::ClusterData>::const_iterator i;
		bool clusterOnline = false;
		
		for(i = ec.getData().begin(); i != ec.getData().end(); ++i)
		{
			REPORT_LOG(true, ("\"%s\" ", (*i).m_clusterName.c_str()));
			if((*i).m_clusterName == ConfigSwgLoadClient::getClusterName())
			{
				// got the cluster! select it!
				clusterOnline = true;
				m_clusterId = (*i).m_clusterId;
				REPORT_LOG(true, ("<--***OUR CLUSTER*** "));
			}
		}
		REPORT_LOG(true, ("\n"));
		if(! clusterOnline)
		{
			REPORT_LOG(true, ("Galaxy %s is not available for load testing, stopping load client\n", ConfigSwgLoadClient::getClusterName()));
			SwgLoadClient::quit();
		}
	}
	else if(base.isType("LoginClusterStatus"))
	{
		REPORT_LOG(true, ("[%s] received LoginClusterStatus message\n", m_loginId.c_str()));

		LoginClusterStatus lcs(ri);
		const std::vector<LoginClusterStatus::ClusterData> & d = lcs.getData();
		std::vector<LoginClusterStatus::ClusterData>::const_iterator i;
		REPORT_LOG(true, ("Galaxies: "));
		bool connectionServerOnline = false;
		for(i = d.begin(); i != d.end(); ++i)
		{
			REPORT_LOG(true, ("\"%ld\" (%s:%d)  ", (*i).m_clusterId, (*i).m_connectionServerAddress.c_str(), (*i).m_connectionServerPort));
			if((*i).m_clusterId == m_clusterId)
			{
				connectionServerOnline = true;
				m_gameAddress = (*i).m_connectionServerAddress;
				m_gamePort = (*i).m_connectionServerPort;
				REPORT_LOG(true, ("*** found our galaxy ***"));
				break;
			}
		}
		if(! connectionServerOnline)
		{
			REPORT_LOG(true, ("[%s] a connection server for our galaxy is not online. Aborting\n", m_loginId.c_str()));
			SwgLoadClient::quit();
		}
		else
		{
			REPORT_LOG(true, ("\n"));
		}
	}
	else if(base.isType("EnumerateCharacterId"))
	{
		REPORT_LOG(true, ("[%s] received EnumerateCharacterId message\n", m_loginId.c_str()));
		EnumerateCharacterId ec(ri);
		if(ec.getData().empty())
		{
			REPORT_LOG(true, ("no characters on galaxy, going to create one...\n"));
			m_owner->createCharacter(m_gameAddress, m_gamePort);
			disconnect();
		}
		else
		{
			REPORT_LOG(true, ("Available characters: "));
			// is our character already available?
			std::vector<EnumerateCharacterId::Chardata>::const_iterator i;
			for(i = ec.getData().begin(); i != ec.getData().end(); ++i)
			{
				REPORT_LOG(true, ("[%s] ", Unicode::wideToNarrow((*i).m_name).c_str()));
				if((*i).m_name == Unicode::narrowToWide(m_loginId))
				{
					REPORT_LOG(true, ("<--***OUR CHARACTER***"));
					m_owner->connectToGame(m_gameAddress, m_gamePort, (*i).m_networkId);
 					disconnect();
					break;
				}
			}
			REPORT_LOG(true, ("\n"));
		}
	}
	else if(base.isType("ErrorMessage"))
	{
		ErrorMessage m(ri);
		REPORT_LOG(true, ("[%s] received ErrorMessage %s : %s\nQUITTING\n", m_loginId.c_str(), m.getErrorName().c_str(), m.getDescription().c_str()));
		SwgLoadClient::quit();
	}
}

//-----------------------------------------------------------------------

