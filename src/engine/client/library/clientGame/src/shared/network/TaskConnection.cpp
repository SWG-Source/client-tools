// TaskConnection.cpp
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "clientGame/FirstClientGame.h"
#include "Archive/ByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetwork/NetworkSetupData.h"
#include "TaskConnection.h"

//-----------------------------------------------------------------------

TaskConnection::TaskConnection(const std::string & address, const unsigned short port) :
Connection(address, port, NetworkSetupData())
{
}

//-----------------------------------------------------------------------

TaskConnection::~TaskConnection()
{
}

//-----------------------------------------------------------------------

void TaskConnection::onConnectionClosed()
{
}

//-----------------------------------------------------------------------

void TaskConnection::onConnectionOpened()
{
}

//-----------------------------------------------------------------------

void TaskConnection::onReceive(const Archive::ByteStream &)
{
}

//-----------------------------------------------------------------------

void TaskConnection::send(const GameNetworkMessage & message)
{
	static Archive::ByteStream bs;
	bs.clear();
	message.pack(bs);
	Connection::send(bs, true);
}

//-----------------------------------------------------------------------

