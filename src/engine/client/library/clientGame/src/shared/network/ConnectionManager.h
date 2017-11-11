// ConnectionManager.h
// copyright 2001 Verant Interactive
// Author: Justin Randall

#ifndef	_INCLUDED_ConnectionManager_H
#define	_INCLUDED_ConnectionManager_H

//-----------------------------------------------------------------------

#include <map>

class Connection;
class GameNetworkMessage;
class NetworkScene;

//-----------------------------------------------------------------------

class ConnectionManager
{
public:
	
	typedef std::map<std::string, std::pair<std::string, unsigned short> > ServerMap_t;

	ConnectionManager();
	~ConnectionManager();

	void                   addClusterToList(const char * const newClusterName, const std::string & newAddr, const unsigned short port);
	void                   connectToLoginServer(const std::string &  ipAddress, const uint16 port);
	void                   disconnectLoginServer(void);
	void                   sendToConnectionServer(const GameNetworkMessage & msg, bool reliable) const;
	void                   setSceneChannel(NetworkScene * newSceneChannel);

	const ServerMap_t &    getConnectionServerMap(void) const;

protected:

	// why are these here?
	void               setCurrentConnectionServerConnection(const Connection * newConnectionServerConnection);
	void               setCurrentLoginConnection(const Connection * newLoginConnection);

private:

	ConnectionManager operator = (const ConnectionManager & rhs);
	ConnectionManager(const ConnectionManager & source);

private:

	Connection *        currentLoginConnection;
	Connection *        currentConnectionServerConnection;

	std::string         loginServerAddress;
	unsigned short      loginServerPort;
	std::string         connectionServerAddress;
	unsigned short      connectionServerPort;

	ServerMap_t *       connectionServers;
};

//-----------------------------------------------------------------

#endif	// _INCLUDED_ConnectionManager_H
