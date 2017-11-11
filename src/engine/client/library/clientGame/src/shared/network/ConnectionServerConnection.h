// ConnectionServerConnection.h
// copyright 2001 Verant Interactive


#ifndef	_INCLUDED_ConnectionServerConnection_H
#define	_INCLUDED_ConnectionServerConnection_H

//-----------------------------------------------------------------------

#include "clientGame/GameNetworkConnection.h"

#include <list>

//-----------------------------------------------------------------------

class ConnectionServerConnection : public GameNetworkConnection
{
public:
	ConnectionServerConnection(const std::string & remoteAddress, const unsigned short remotePort, const NetworkSetupData &);
	~ConnectionServerConnection();

	int           getConnectionServerLag  () const;
	int           getGameServerLag        () const;
	void          onConnectionClosed      ();
	void          onConnectionOpened      ();
	void          onConnectionOverflowing (const unsigned int bytesPending);
	virtual void  onReceive               (const Archive::ByteStream & message);
	void          updateLagStats          ();

	void          beginDeferringIncomingPackets();
	void          endDeferringIncomingPackets(bool emitDeferredPackets=true);
	bool          isDeferringIncomingPackets()                                 const { return m_deferredPackets!=0; }

private:
	ConnectionServerConnection & operator = (const ConnectionServerConnection & rhs);
	ConnectionServerConnection(const ConnectionServerConnection & source);
	void  sendLagReport();

	void  _deferIncomingPacket(const Archive::ByteStream & message);
	void  _onReceive(const Archive::ByteStream & message);

private:

	typedef std::list<Archive::ByteStream *> PacketList;

	int   m_lagRequestTime;
	int   m_connectionServerLag;
	int   m_gameServerLag;
	int   m_lagRequestDelay;
	bool  m_pendingConnectionServerLagResponse;
	bool  m_pendingGameServerLagResponse;
	PacketList *m_deferredPackets;
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ConnectionServerConnection_H
