// LoginConnection.h
// copyright 2000 Verant Interactive
// Author: Justin Randall

#ifndef	_LoginConnection_H
#define	_LoginConnection_H

//-----------------------------------------------------------------------

#include "clientGame/GameNetworkConnection.h"

//-----------------------------------------------------------------------

class LoginConnection : public GameNetworkConnection
{
public:

	struct Messages
	{
		static const char * const LoginConnectionOpened;
		static const char * const LoginConnectionClosed;
		static const char * const LoginIncorrectClientId;
	};

	LoginConnection(const std::string & remoteAddress, const unsigned short remotePort, const NetworkSetupData & setupData);
	~LoginConnection();
	void                          onConnectionClosed      ();
	void                          onConnectionOpened      ();
	void                          onConnectionOverflowing (const unsigned int bytesPending);
private:
	LoginConnection(const LoginConnection &);
	LoginConnection & operator = (const LoginConnection &);
};

//-----------------------------------------------------------------------

#endif	// _LoginConnection_H