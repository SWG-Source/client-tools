// LoadConnection.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_LoadConnection_H
#define	_INCLUDED_LoadConnection_H

//-----------------------------------------------------------------------

#include "sharedNetwork/Connection.h"

class GameNetworkMessage;

//-----------------------------------------------------------------------

class LoadConnection : public Connection
{
public:
	LoadConnection(const std::string & address, const unsigned short port, const NetworkSetupData &setupData);
	~LoadConnection();

	void          send                  (const GameNetworkMessage & message, const bool reliable);

private:
	LoadConnection & operator = (const LoadConnection & rhs);
	LoadConnection(const LoadConnection & source);

};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_LoadConnection_H

