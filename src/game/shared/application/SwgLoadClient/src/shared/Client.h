// Client.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_Client_H
#define	_INCLUDED_Client_H

//-----------------------------------------------------------------------

#include <string>

class Connection;
class GameConnection;
class LoginClientToken;
class LoginConnection;
class NetworkId;

//-----------------------------------------------------------------------

class Client
{
public:
	explicit Client(const std::string & id);
	~Client();

	void                      connectToGame        (const std::string & address, const unsigned short port, const NetworkId &characterId);
	void                      createCharacter      (const std::string & address, const unsigned short port);
	void                      setLoginClientToken  (LoginClientToken *token);
	const LoginClientToken *  getLoginClientToken  () const;
	const std::string &       getLoginId           () const;
	void                      login                (const std::string & adress, const unsigned short port);
	void                      update               ();
	void                      onConnectionClosed   (Connection *connection);

private:
	Client & operator = (const Client & rhs);
	Client(const Client & source);

	GameConnection *   m_gameConnection;
	LoginConnection *  m_loginConnection;
	std::string        m_loginId;
	LoginClientToken * m_loginClientToken;
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_Client_H
