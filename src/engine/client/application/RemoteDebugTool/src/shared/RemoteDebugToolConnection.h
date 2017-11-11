// ======================================================================
//
// RemoteDebugToolConnection.h
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef RemoteDebugToolConnection_H
#define RemoteDebugToolConnection_H

//-----------------------------------------------------------------------

#include "FirstRemoteDebugTool.h"
#include "sharedNetwork/Connection.h"

class UdpConnecton;

class RemoteDebugToolConnection : public Connection//, public MessageDispatch::Emitter
{
public:
	RemoteDebugToolConnection(const std::string & remoteAddress, const unsigned short remotePort);
	RemoteDebugToolConnection(UdpConnection *);
	virtual void onConnectionClosed();
	virtual void onConnectionOpened();
	virtual void onConnectionOverflowing(const unsigned int bytesPending);
	virtual void onReceive(const Archive::ByteStream & message);

private:
	RemoteDebugToolConnection();
	RemoteDebugToolConnection(const RemoteDebugToolConnection &);
	RemoteDebugToolConnection & operator = (const RemoteDebugToolConnection &);
};

//-----------------------------------------------------------------------

#endif // RemoteDebugToolConnection_H
