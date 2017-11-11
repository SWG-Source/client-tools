// TaskConnection.h
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_TaskConnection_H
#define	_INCLUDED_TaskConnection_H

//-----------------------------------------------------------------------

#include "sharedNetwork/Connection.h"

class GameNetworkMessage;

//-----------------------------------------------------------------------

class TaskConnection : public Connection
{
public:
	TaskConnection(const std::string & address, const uint16 port);
	~TaskConnection();
	virtual void  onConnectionClosed  ();
	virtual void  onConnectionOpened  ();
	virtual void  onReceive           (const Archive::ByteStream & bs);
	virtual void  send                (const GameNetworkMessage & message);

private:
	TaskConnection & operator = (const TaskConnection & rhs);
	TaskConnection(const TaskConnection & source);

};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_TaskConnection_H
