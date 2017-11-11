#include "FirstLagger.h"
#include "Packet.h"

using namespace std;

//---------------------------------------------------------------------------

#define W(func) #func, NULL,

//---------------------------------------------------------------------------

#define WRAP(func,n) __declspec( naked ) void WRAP_##func () \
	{ __asm { jmp dword ptr funcTable + n * 8 + 4 } }

//---------------------------------------------------------------------------
namespace
{
	//
	// define the function table structure
	//
	struct FuncTable
	{
		const char *name;
		void(*fptr)();
	};

	//---------------------------------------------------------------------------

	// declare the function table
	FuncTable funcTable[] =
	{
#include "w.h"
		0, 0
	};

	//---------------------------------------------------------------------------

	// define the logging object
	DebugLogger           s_log;

	//---------------------------------------------------------------------------

	// define the name of this DLL
	const char *     cs_dllName = "ws2_32.dll";

	//---------------------------------------------------------------------------

	double           s_recvLatency            = 0.000;   // input latency in seconds
	double           s_sendLatency            = 0.000;   // <- output latency in seconds
	const UINT       cs_defaultInputBufferSize = 2000;    // <- default input buffer size
	HANDLE           s_sendThreadHandle       = INVALID_HANDLE_VALUE;
	HANDLE           s_sendEvent              = INVALID_HANDLE_VALUE;
	CRITICAL_SECTION s_sendQueueLock;
	const int        cs_cpListenPort          = 2373;
	SOCKET           s_cpSocket               = INVALID_SOCKET;
	int              s_packetLoss             = 0;   // <- packet loss as a percentage

	typedef int(*func_recvfrom)(SOCKET,char *,int,int, sockaddr *,int *);
	#define RECVFROM ((func_recvfrom)funcTable[16].fptr)

	typedef int(*func_WSAStartup)(WORD,LPWSADATA);
	#define WSASTARTUP ((func_WSAStartup)funcTable[110].fptr)

	//---------------------------------------------------------------------------


	static map< SOCKET, Packet::Queue > s_recvqMap;
	static Packet::Queue                s_sendq;


	//---------------------------------------------------------------------------

	enum DataDirection
	{
		Dir_Input,
		Dir_Output
	};

	//---------------------------------------------------------------------------

	//
	// set the incoming or outgoing latency in milliseconds
	//
	void setLatency( DataDirection dir, double value )
	{
		switch ( dir )
		{
		case Dir_Input:
			s_recvLatency = value;
			break;
		case Dir_Output:
			s_sendLatency = value;
			break;
		}
	}


}

//---------------------------------------------------------------------------

void initControlPanelSocket();
void updateControlPanelSocket();

//---------------------------------------------------------------------------

WRAP( accept, 0 )
//WRAP( bind, 1 )
//WRAP( closesocket, 2 )
WRAP( connect, 3 )
WRAP( getpeername, 4 )
WRAP( getsockname, 5 )
WRAP( getsockopt, 6 )
WRAP( htonl, 7 )
//WRAP( htons, 8 )
//WRAP( ioctlsocket, 9 )
WRAP( inet_addr, 10 )
WRAP( inet_ntoa, 11 )
WRAP( listen, 12 )
WRAP( ntohl, 13 )
WRAP( ntohs, 14 )
WRAP( recv, 15 )
//WRAP( recvfrom, 16 )
WRAP( select, 17 )
WRAP( send, 18 )
//WRAP( sendto, 19 )
WRAP( setsockopt, 20 )
WRAP( shutdown, 21 )
//WRAP( socket, 22 )
WRAP( WSApSetPostRoutine, 23 )
WRAP( WPUCompleteOverlappedRequest, 24 )
WRAP( WSAAccept, 25 )
WRAP( WSAAddressToStringA, 26 )
WRAP( WSAAddressToStringW, 27 )
WRAP( WSACloseEvent, 28 )
WRAP( WSAConnect, 29 )
WRAP( WSACreateEvent, 30 )
WRAP( WSADuplicateSocketA, 31 )
WRAP( WSADuplicateSocketW, 32 )
WRAP( WSAEnumNameSpaceProvidersA, 33 )
WRAP( WSAEnumNameSpaceProvidersW, 34 )
WRAP( WSAEnumNetworkEvents, 35 )
WRAP( WSAEnumProtocolsA, 36 )
WRAP( WSAEnumProtocolsW, 37 )
WRAP( WSAEventSelect, 38 )
WRAP( WSAGetOverlappedResult, 39 )
WRAP( WSAGetQOSByName, 40 )
WRAP( WSAGetServiceClassInfoA, 41 )
WRAP( WSAGetServiceClassInfoW, 42 )
WRAP( WSAGetServiceClassNameByClassIdA, 43 )
WRAP( WSAGetServiceClassNameByClassIdW, 44 )
WRAP( WSAHtonl, 45 )
WRAP( WSAHtons, 46 )
WRAP( WSAInstallServiceClassA, 47 )
WRAP( WSAInstallServiceClassW, 48 )
WRAP( WSAIoctl, 49 )
WRAP( gethostbyaddr, 50 )
WRAP( gethostbyname, 51 )
WRAP( getprotobyname, 52 )
WRAP( getprotobynumber, 53 )
WRAP( getservbyname, 54 )
WRAP( getservbyport, 55 )
WRAP( gethostname, 56 )
WRAP( WSAJoinLeaf, 57 )
WRAP( WSALookupServiceBeginA, 58 )
WRAP( WSALookupServiceBeginW, 59 )
WRAP( WSALookupServiceEnd, 60 )
WRAP( WSALookupServiceNextA, 61 )
WRAP( WSALookupServiceNextW, 62 )
WRAP( WSANSPIoctl, 63 )
WRAP( WSANtohl, 64 )
WRAP( WSANtohs, 65 )
WRAP( WSAProviderConfigChange, 66 )
WRAP( WSARecv, 67 )
WRAP( WSARecvDisconnect, 68 )
WRAP( WSARecvFrom, 69 )
WRAP( WSARemoveServiceClass, 70 )
WRAP( WSAResetEvent, 71 )
WRAP( WSASend, 72 )
WRAP( WSASendDisconnect, 73 )
WRAP( WSASendTo, 74 )
WRAP( WSASetEvent, 75 )
WRAP( WSASetServiceA, 76 )
WRAP( WSASetServiceW, 77 )
WRAP( WSASocketA, 78 )
WRAP( WSASocketW, 79 )
WRAP( WSAStringToAddressA, 80 )
WRAP( WSAStringToAddressW, 81 )
WRAP( WSAWaitForMultipleEvents, 82 )
WRAP( WSCDeinstallProvider, 83 )
WRAP( WSCEnableNSProvider, 84 )
WRAP( WSCEnumProtocols, 85 )
WRAP( WSCGetProviderPath, 86 )
WRAP( WSCInstallNameSpace, 87 )
WRAP( WSCInstallProvider, 88 )
WRAP( WSCUnInstallNameSpace, 89 )
WRAP( WSCUpdateProvider, 90 )
WRAP( WSCWriteNameSpaceOrder, 91 )
WRAP( WSCWriteProviderOrder, 92 )
WRAP( freeaddrinfo, 93 )
WRAP( getaddrinfo, 94 )
WRAP( getnameinfo, 95 )
WRAP( WSAAsyncSelect, 96 )
WRAP( WSAAsyncGetHostByAddr, 97 )
WRAP( WSAAsyncGetHostByName, 98 )
WRAP( WSAAsyncGetProtoByNumber, 99 )
WRAP( WSAAsyncGetProtoByName, 100 )
WRAP( WSAAsyncGetServByPort, 101 )
WRAP( WSAAsyncGetServByName, 102 )
WRAP( WSACancelAsyncRequest, 103 )
WRAP( WSASetBlockingHook, 104 )
WRAP( WSAUnhookBlockingHook, 105 )
//WRAP( WSAGetLastError, 106 )
//WRAP( WSASetLastError, 107 )
WRAP( WSACancelBlockingCall, 108 )
WRAP( WSAIsBlocking, 109 )
//WRAP( WSAStartup, 110 )
WRAP( WSACleanup, 111 )
WRAP( __WSAFDIsSet, 112 )
WRAP( WEP, 113 )

//---------------------------------------------------------------------------

int WRAP_ioctlsocket( SOCKET s, long cmd, u_long *arg )
{
	return ((int(*)(SOCKET,long,u_long*))funcTable[9].fptr)(s,cmd,arg);
}

//---------------------------------------------------------------------------

int WRAP_WSAStartup( WORD version, WSADATA *data )
{
	int result = WSASTARTUP( version, data );

	initControlPanelSocket();

	return result;
}
//---------------------------------------------------------------------------

SOCKET WRAP_socket( int af, int type, int proto )
{
	return ((SOCKET(*)(int,int,int))funcTable[22].fptr)(af,type,proto);
}

//---------------------------------------------------------------------------

int WRAP_closesocket( SOCKET s )
{
	return ((int(*)(SOCKET))funcTable[2].fptr)(s);
}

//---------------------------------------------------------------------------

u_short WRAP_htons( u_short value )
{
	return ((u_short(*)(u_short))funcTable[8].fptr)(value);
}

//---------------------------------------------------------------------------

int WRAP_bind( SOCKET s, const sockaddr *name, int namelen )
{
	return ((int(*)(SOCKET,const sockaddr *,int))funcTable[1].fptr)(s,name,namelen);
}

//---------------------------------------------------------------------------

void WRAP_WSASetLastError( int error )
{
	((void(*)(int))funcTable[107].fptr)(error);
}

//---------------------------------------------------------------------------

int WRAP_WSAGetLastError()
{
	return ((int(*)())funcTable[106].fptr)();
}

//---------------------------------------------------------------------------

int WRAP_sendto( SOCKET s, const char *buf, int len, int flags, const sockaddr *addr, int addrlen )
{
	if ( len > 0 )
	{
		Packet p( buf, len, s, addr, addrlen, flags );
		EnterCriticalSection( &s_sendQueueLock );
		s_sendq.push( p );
		LeaveCriticalSection( &s_sendQueueLock );
	}

	SetEvent( s_sendEvent );

	return len;
}
//---------------------------------------------------------------------------


//
// receive some data from a socket
//

int WRAP_recvfrom( SOCKET s, char *buf, int /*len*/, int flags, sockaddr *from, int *fromlen )
{
	updateControlPanelSocket();

	auto_ptr< char > tmpbuf( new char[ cs_defaultInputBufferSize ] );

	int count = RECVFROM(
		s,
		tmpbuf.get(),
		cs_defaultInputBufferSize,
		flags,
		from,
		fromlen);

	if ( count == 0 )
	{
		// socket was closed
		return 0;
	}

	// if there was data on the socket, then we need to queue it up
	if ( count > 0 )
	{
		Packet p(
			tmpbuf.get(),
			count,
			s,
			from,
			*fromlen );

		s_recvqMap[ s ].push( p );
	}

	// look for any data that we need to send to the client
	// (note we can only send one packet at a time back!)

	while ( s_recvqMap[ s ].size() && s_recvqMap[ s ].front().getAge() >= s_recvLatency )
	{
		const Packet p( s_recvqMap[s].front() );

		s_recvqMap[s].pop();

		if ( rand() % 100 >= s_packetLoss )
		{

			const UINT dataSize = p.getDataSize();

			// copy data from packet into recv buffer
			memcpy( buf, p.getData(), dataSize );

			// copy address data into address buffer
			*fromlen = min( *fromlen, (int)p.getAddressSize() );
			memcpy( from, p.getAddress(), *fromlen );

			// return the number of bytes copied
			return dataSize;
		}
	}

	// nothing to return back to the client.

	WRAP_WSASetLastError( WSAEWOULDBLOCK );
	return -1;
}

//---------------------------------------------------------------------------

void initControlPanelSocket()
{

	if ( s_cpSocket == INVALID_SOCKET )
	{
		// create a listener socket for the control panel
		s_cpSocket = WRAP_socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP );

		// set the socket to non-blocking
		u_long socketoptValue = 1;
		WRAP_ioctlsocket( s_cpSocket, FIONBIO, &socketoptValue );

		// bind listener socket
		sockaddr_in sin;
		sin.sin_family       = AF_INET;
		sin.sin_port         = WRAP_htons( cs_cpListenPort );
		sin.sin_addr.s_addr  = INADDR_ANY;


		const int bindResult = WRAP_bind( s_cpSocket, (sockaddr *)&sin, sizeof( sockaddr_in ) );

		if ( bindResult == SOCKET_ERROR )
		{
			s_log.write( "%s: ERROR: unable to bind control panel listener socket (socket=%d error=%d)!",
				cs_dllName,
				s_cpSocket,
				WRAP_WSAGetLastError() );
		}
	}

}

//---------------------------------------------------------------------------

//
// read and parse any data from the control panel socket
//
void updateControlPanelSocket()
{
	if ( s_cpSocket == INVALID_SOCKET )
	{
		return;
	}

	char buffer[ 512 ];

	int bytesRead = RECVFROM( s_cpSocket, buffer, 512, 0, 0, 0 );

	if ( bytesRead > 0 )
	{
		int recvLat, sendLat, pktLoss;

		buffer[ bytesRead ] = 0; // <- null terminate string

		sscanf( buffer, "%d %d %d", &recvLat, &sendLat, &pktLoss );

		s_recvLatency = (double)recvLat * 0.001; // milliseconds
		s_sendLatency = (double)sendLat * 0.001; // milliseconds
		s_packetLoss  = pktLoss;                 // percentage

		s_log.write(
			"%s: setting recvLatency=%.3f sendLatency=%.3f packetLoss=%d%%\n",
			cs_dllName,
			s_recvLatency,
			s_sendLatency,
			s_packetLoss  );
	}
}

//---------------------------------------------------------------------------

DWORD WINAPI sendThreadProc( LPVOID )
{
	DWORD timeout;

	while ( s_sendThreadHandle != INVALID_HANDLE_VALUE  )
	{
		timeout = INFINITE;

		EnterCriticalSection( &s_sendQueueLock );

		double packetAge = 0;

		while ( s_sendq.size() && ( ( packetAge = s_sendq.front().getAge() ) > s_sendLatency ) )
		{
			const Packet &p = s_sendq.front();

			typedef int(*func_sendto)(SOCKET,const char *, int, int, const sockaddr *, int );


			if ( rand() % 100 >= s_packetLoss )
			{
				const int count = ((func_sendto)funcTable[19].fptr)(
					p.getSocket(),
					(const char *)( p.getData() ),
					p.getDataSize(),
					p.getSendFlags(),
					p.getAddress(),
					p.getAddressSize() );

				// check for a send error
				if ( count > 0 )
				{
					s_sendq.pop(); // if there is no send error, then we can remove this packet
				}
				else if ( count == SOCKET_ERROR )
				{
					const int error = WRAP_WSAGetLastError();

					if ( error == WSAEWOULDBLOCK )
					{
						timeout = 10;
					}
					else
					{
						s_sendq.pop();
					}
				}
			}
			else
			{
				// just remove the packet if it's packet loss
				s_sendq.pop();
			}


		}

		if ( s_sendq.size() )
		{
			timeout = (DWORD)( ( s_sendLatency - packetAge ) * 1000. );
			timeout = max( timeout, 1 );
		}

		LeaveCriticalSection( &s_sendQueueLock );

		WaitForSingleObject( s_sendEvent, timeout );
	}

	CloseHandle( s_sendEvent );
	s_sendEvent = INVALID_HANDLE_VALUE;

	s_log.write( "%s: sendThreadProc exit\n", cs_dllName );
	return 0;
}

BOOL WINAPI DllMain( HINSTANCE , DWORD reason, void * )
{
	if ( reason != DLL_PROCESS_ATTACH )
	{
		if ( reason == DLL_PROCESS_DETACH )
		{
			CloseHandle( s_sendThreadHandle );
			s_sendThreadHandle = INVALID_HANDLE_VALUE;
			SetEvent( s_sendEvent );
			DeleteCriticalSection( &s_sendQueueLock );

			if ( s_cpSocket != INVALID_SOCKET )
			{
				WRAP_closesocket( s_cpSocket );
			}
		}
		return TRUE;
	}

	s_log.write( "--------- WS2_32.DLL wrapper start\n" );

	Packet::install();

	//	g_log.open( "C:\\wsock.txt", ios::out);

	char systemPath[ MAX_PATH ];
	GetSystemDirectory( systemPath, MAX_PATH );
	strcat( systemPath, "\\" );
	strcat( systemPath, cs_dllName );

	HMODULE module = LoadLibrary( systemPath );

	if ( module == NULL )
	{
		s_log.write( "%s: unable to open %s\n", cs_dllName, systemPath );
		ExitProcess(0);
	}

	FuncTable *func = funcTable;

	for ( ; func->name; ++func )
	{
		func->fptr = (void(*)(void))GetProcAddress( module, func->name );
		s_log.write( "Binding function '%s' addr=0x%08x\n", func->name, func->fptr );
	}

	InitializeCriticalSection( &s_sendQueueLock );

	setLatency( Dir_Output, 0. );
	setLatency( Dir_Input,  0. );

	s_sendEvent        = CreateEvent( 0, FALSE, FALSE, 0 );
	DWORD threadId;
	s_sendThreadHandle = CreateThread( 0, 0, sendThreadProc, 0, CREATE_SUSPENDED, &threadId );
	ResumeThread( s_sendThreadHandle );

	s_log.write( "--------- WS2_32.DLL wrapper installed!\n" );

	return TRUE;
}


