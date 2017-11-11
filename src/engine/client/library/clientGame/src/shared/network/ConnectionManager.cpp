// ConnectionManager.cpp
// copyright 2001 Verant Interactive
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "clientGame/FirstClientGame.h"
#include "clientGame/ConnectionManager.h"

#include "unicodeArchive/UnicodeArchive.h"
#include "clientGame/LoginConnection.h"
#include "sharedNetwork/NetworkSetupData.h"
#include "sharedNetworkMessages/ClientCentralMessages.h"

#include <algorithm>

//-----------------------------------------------------------------------

ConnectionManager::ConnectionManager() :
currentLoginConnection(0),
currentConnectionServerConnection(0),
loginServerAddress (),
connectionServerAddress (),
connectionServers (NON_NULL (new ServerMap_t))
{
}

//-----------------------------------------------------------------------

ConnectionManager::~ConnectionManager()
{
	delete connectionServers;
	connectionServers = 0;

	if (currentLoginConnection)
	{
		currentLoginConnection->disconnect ();
		currentLoginConnection = 0;
	}

	if (currentConnectionServerConnection)
	{
		currentConnectionServerConnection->disconnect ();
		currentConnectionServerConnection = 0;
	}
}

//-----------------------------------------------------------------------

void ConnectionManager::addClusterToList(const char * const newClusterName, const std::string & newAddr, const unsigned short port)
{
	std::pair<std::string, unsigned short> a(newAddr, port);
	(*connectionServers)[newClusterName] = a;
}

//-----------------------------------------------------------------------

void ConnectionManager::connectToLoginServer(const std::string & ipAddress, const uint16 port)
{
	NetworkSetupData setupData;
	setupData.useTcp = false;
	currentLoginConnection = new LoginConnection(ipAddress, port, setupData);
}

//-----------------------------------------------------------------------

void ConnectionManager::disconnectLoginServer(void)
{
	if (currentLoginConnection)
	{
		currentLoginConnection->disconnect ();
		currentLoginConnection = 0;
	}
}

//-----------------------------------------------------------------------

void ConnectionManager::sendToConnectionServer(const GameNetworkMessage & outboundMessage, const bool reliable) const
{
	Archive::ByteStream msg;
	outboundMessage.pack(msg);
	currentConnectionServerConnection->send(msg, reliable);
}

//-----------------------------------------------------------------------

void ConnectionManager::setSceneChannel(NetworkScene * newSceneChannel)
{
	UNREF(newSceneChannel);
	DEBUG_FATAL(true, ("Reimplement!\n"));
}  //lint !e1762  //function could be make const

//-----------------------------------------------------------------

const ConnectionManager::ServerMap_t & ConnectionManager::getConnectionServerMap(void) const
{
	return *connectionServers;
}

//-----------------------------------------------------------------

