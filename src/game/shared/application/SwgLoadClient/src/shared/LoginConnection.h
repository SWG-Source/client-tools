// LoginConnection.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_LoginConnection_H
#define	_INCLUDED_LoginConnection_H

//-----------------------------------------------------------------------

#include "LoadConnection.h"

class Client;
class LoginClientToken;

//-----------------------------------------------------------------------

class LoginConnection : public LoadConnection
{
public:
	LoginConnection(Client * owner, const std::string & loginId, const std::string & address, const unsigned short port, const NetworkSetupData &setupData);
	~LoginConnection();

	void                      onConnectionClosed   ();
	void                      onConnectionOpened   ();
	void                      onReceive            (const Archive::ByteStream &);

private:
	LoginConnection & operator = (const LoginConnection & rhs);
	LoginConnection(const LoginConnection & source);
	
	bool                m_createCharacter;
	unsigned long       m_clusterId;
	std::string         m_gameAddress;
	unsigned short      m_gamePort;
	std::string         m_loginId;
	Client *            m_owner;
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_LoginConnection_H
