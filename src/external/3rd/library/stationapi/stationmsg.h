#ifndef _STATIONMSG_H_
#define _STATIONMSG_H_

#include "stationapi.h"

typedef struct _msg_t
{
  uint16 msg;
} msg_t;

// messages from client to server
#define CLIENT_DISCONNECT					0x1001 
#define	CLIENT_CONNECT		    			0x1002

#define	CLIENT_REQUEST_LOGIN                0x1011
#define	CLIENT_REQUEST_SESSION              0x1012

// messages from server to client
#define SERVER_VERSION                      0x2001 

#define SERVER_LOGIN                        0x2011
#define SERVER_SESSION                      0x2012

#define SAPI_ENCRYPT_KEY    "#$DI8de3Dywe82}[-*&"

// structures from client to server
struct client_login
{
    StationAPITrack nTrack;     // tracking number assigned by client
    StationAPIRequest nRequest;
	char sStationName[SMALL_LENGTH];
	char sStationPassword[SMALL_LENGTH];
    StationAPIIP nIP;
    StationAPIProduct nProduct;
    char sParentSession[StationAPISession::SESSION_ID_LENGTH];
};
struct encrypt_client_login
{
	char buffer[(sizeof(client_login)+7)&0x0fff8];	// upped to nearest multiple of 8
};

struct client_session
{
    StationAPITrack nTrack;     // tracking number assigned by client
    StationAPIRequest nRequest;
    char sSession[StationAPISession::SESSION_ID_LENGTH];
};


// structures from server to client
struct server_login
{
    StationAPITrack nTrack;     // tracking number assigned by client
    StationAPIResult nResult;   // result of operation
    StationAPIUID nUid;
    StationAPIAccountStatus nAccountStatus;
    StationAPIGameStatus nGameStatus;
    char sSession[StationAPISession::SESSION_ID_LENGTH];
};

struct server_session
{
    StationAPITrack nTrack;     // tracking number assigned by client
    StationAPIResult nResult;   // result of operation
    StationAPIUID nUid;
};
#endif
