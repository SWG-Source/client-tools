// LoadConnection.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstSwgLoadClient.h"
#include "Archive/ByteStream.h"
#include "LoadConnection.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

LoadConnection::LoadConnection(const std::string & a, const unsigned short p, const NetworkSetupData &setupData) :
Connection(a, p, setupData)
{
}

//-----------------------------------------------------------------------

LoadConnection::~LoadConnection()
{
}

//-----------------------------------------------------------------------

void LoadConnection::send(const GameNetworkMessage & source, const bool reliable)
{
	static Archive::ByteStream bs;
	bs.clear();
	source.pack(bs);
	Connection::send(bs, reliable);
}

//-----------------------------------------------------------------------

