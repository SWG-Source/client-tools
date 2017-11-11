// LoginConnection.h
// copyright 2000, 2005 Verant Interactive
// Author: Justin Randall, JMatzen

#ifndef	_LoginConnection_H
#define	_LoginConnection_H

//-----------------------------------------------------------------------

#include "sharedNetwork/Connection.h"

//-----------------------------------------------------------------------

class LoginEnumCluster;
class ClusterSelectionDialog;

class LoginConnection : public Connection
{
public:

	struct Messages
	{
		static const char * const LoginConnectionOpened;
		static const char * const LoginConnectionClosed;
		static const char * const LoginIncorrectClientId;
	};

	LoginConnection(
		ClusterSelectionDialog *dlg, 
		const std::string & remoteAddress,
		const unsigned short remotePort,
		const NetworkSetupData & setupData,
		const std::string &username,
		const std::string &password );

	~LoginConnection();

	void                          onConnectionClosed      ();
	void                          onConnectionOpened      ();
	void                          onConnectionOverflowing (const unsigned int bytesPending);
	void                          onReceive(const Archive::ByteStream & message);
		
private:

	ClusterSelectionDialog * m_clusterSelectionDlg;
	
	LoginConnection(const LoginConnection &);

	LoginConnection & operator = (const LoginConnection &);

	const std::string m_username;
	const std::string m_password;

};

//-----------------------------------------------------------------------

#endif	// _LoginConnection_H