// Client.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstSwgLoadClient.h"
#include "Client.h"
#include "GameConnection.h"
#include "LoginConnection.h"
#include "sharedFoundation/Clock.h"
#include "sharedNetwork/NetworkSetupData.h"
#include "sharedNetworkMessages/ClientLoginMessages.h"

//-----------------------------------------------------------------------

Client::Client(const std::string & id) :
	m_gameConnection(0),
	m_loginConnection(0),
	m_loginId(id),
	m_loginClientToken(0)
{
}

//-----------------------------------------------------------------------

Client::~Client()
{
	delete m_gameConnection;
	delete m_loginConnection;
	delete m_loginClientToken;
}

//-----------------------------------------------------------------------

void Client::connectToGame(const std::string & a, const unsigned short p, const NetworkId & characterId)
{
	delete m_gameConnection;
	NetworkSetupData setupData;
	setupData.useTcp = false;
	m_gameConnection = new GameConnection(this, a, p, characterId, setupData);
}

//-----------------------------------------------------------------------

void Client::createCharacter(const std::string & a, const unsigned short p)
{
	delete m_gameConnection;
	NetworkSetupData setupData;
	setupData.useTcp = false;
	m_gameConnection = new GameConnection(this, a, p, NetworkId::cms_invalid, setupData);
}

//-----------------------------------------------------------------------

const LoginClientToken * Client::getLoginClientToken() const
{
	return m_loginClientToken;
}

//-----------------------------------------------------------------------

const std::string & Client::getLoginId() const
{
	return m_loginId;
}

//-----------------------------------------------------------------------

void Client::login(const std::string & a, const unsigned short p)
{
	delete m_loginClientToken;
	m_loginClientToken = 0;
	NetworkSetupData setupData;
	setupData.useTcp = false;
	m_loginConnection = new LoginConnection(this, m_loginId, a, p, setupData);
}

//-----------------------------------------------------------------------

void Client::update()
{
	if(m_gameConnection)
	{
		if(m_gameConnection->getReadyToSimulate())
		{
			m_gameConnection->update();
		}
	}
}

//-----------------------------------------------------------------------

void Client::setLoginClientToken(LoginClientToken *token)
{
	delete m_loginClientToken;
	m_loginClientToken = token;
}

//-----------------------------------------------------------------------

void Client::onConnectionClosed(Connection *connection)
{
	if (connection == m_gameConnection)
		m_gameConnection = 0;
	if (connection == m_loginConnection)
		m_loginConnection = 0;
}

//-----------------------------------------------------------------------
