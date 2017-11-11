// ======================================================================
//
// RemoteDebugToolConnection.cpp
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstRemoteDebugTool.h"
#include "RemoteDebugToolConnection.h"

#include "Archive/ByteStream.h"
#include "sharedDebug/RemoteDebug_inner.h"
#include "RemoteDebugToolStatic.h"

//-----------------------------------------------------------------------

RemoteDebugToolConnection::RemoteDebugToolConnection(const std::string & a, const unsigned short p) :
Connection(a, p)
{
}

//-----------------------------------------------------------------------

RemoteDebugToolConnection::RemoteDebugToolConnection(UdpConnection * u)
: Connection(u)
{
	RemoteDebugToolStatic::connectionCreated(this);
}

//-----------------------------------------------------------------------

void RemoteDebugToolConnection::onConnectionClosed()
{
	RemoteDebugClient::close();
}

//-----------------------------------------------------------------------

void RemoteDebugToolConnection::onConnectionOpened()
{
	RemoteDebugClient::isReady();
}

//-----------------------------------------------------------------------

void RemoteDebugToolConnection::onConnectionOverflowing(const unsigned int bytesPending)
{
	UNREF(bytesPending);
	//TODO: throttle network send rates or throughput, advise the system it's overflowing
}

//-----------------------------------------------------------------------

void RemoteDebugToolConnection::onReceive(const Archive::ByteStream & message)
{
	RemoteDebugClient::receive(message.getBuffer(), message.getSize());
}

//-----------------------------------------------------------------------
