#ifndef TCPMGR_HEADER
#define TCPMGR_HEADER

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#include <vector>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <io.h>

#else  //UNIX
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <limits.h>

typedef int SOCKET;
#define SOCKET_ERROR   -1
#define INVALID_SOCKET -1
#define closesocket    close

#endif

#ifdef AIX
#include <sys/select.h>
#endif

#ifdef _WIN32
typedef int socklen_t;
#endif

#if defined( __GCC__ ) && !defined( OUT )
#define OUT
#endif

class TCPConnection;


class TCPManager
{
public:

    enum CONN_STATE
    {
        CONN_ERROR          = -1,
        CLOSED              = 0,
        CONNECTING          = 1,
        CONNECTED           = 2
    };
    
    enum CONSTANTS
    {
        INVALID_HANDLE = 0,
        DEFAULT_PROTOCOL = 0
    };

    // These defines specify a system independent way to
    //   get error codes for socket services.
    enum STATUS
    {
        OK                                  = 0,      // Everything's cool
        UNKNOWN                             = -1,        // There was an error of unknown type
        ISCONN                              = -2,         // The socket is already connected
        INPROGRESS                          = -3,         // The socket is non-blocking and the operation
        //   isn't done yet
        ALREADY                             = -4,        // The socket is already attempting a connection
        //   but isn't done yet
        AGAIN                               = -5,      // Try again.
        ADDRINUSE                           = -6,      // Address already in use
        ADDRNOTAVAIL                        = -7,       // That address is not available on the remote host
        BADF                                = -8,       // Not a valid FD
        CONNREFUSED                         = -9,        // Connection was refused
        INTR                                = -10,      // Operation was interrupted
        NOTSOCK                             = -11,       // FD wasn't a socket
        PIPE                                = -12,      // That operation just made a SIGPIPE
        WOULDBLOCK                          = -13,        // That operation would block
        INVAL                               = -14,        // Invalid
        TIMEDOUT                            = -15,     // Timeout
        CONNRESET                           = -16      // Connection reset
    };

    enum DIRECTION
    {
        INCOMING            = 1,
        OUTGOING            = 2,
        EITHER              = 3
    };

                    TCPManager();
                    ~TCPManager();

    bool            addListener( uint32 ip, uint16 port, bool reuseAddr );
    bool            removeListener( uint32 ip, uint16 port );
    bool            getListener( uint32 ip, uint16 port, OUT SOCKET& outsock );

    bool            connect( const char* address, uint16 port, uint32* handle );
    bool            connect( uint32 ip, uint16 port, OUT uint32* handle );

    bool            getOutgoingConnection( TCPConnection** conn, uint32 handle, int32 wait_secs );
    bool            getIncomingConnection( TCPConnection** conn, uint16 port, int32 wait_secs );

    bool            cancelConnection( uint32 handle );  // cancel outgoing connection attempt

    bool            setBufferedWrites( TCPConnection* con, bool enabled );
    void            pumpWriters( void );  // pump the buffered writer connections

    // Static methods
    static int      wait( uint32 sec, uint32 usec, SOCKET* sockets, int count, bool readMode = true );
    static STATUS   getStatus( void );

private:

    SOCKET          createSocket( uint32 ip, uint16 port, bool reuseAddr = true );
    bool            setBlocking( SOCKET fd, bool block );
    bool            getConnection( TCPConnection** conn,
                                   uint32 handle,
                                   uint16 port,
                                   int32 wait_secs,
                                   DIRECTION dir );
    void            pumpConnections( void );

    struct ListenSocket
    {
        SOCKET                      fd;
        uint32                       ip;
        uint32                       port;
    };

    std::vector<ListenSocket>   ListenArray_;

    std::vector<TCPConnection*>        BufferedWriters_;  // need to be pumped

    struct PendingConn
    {
        SOCKET                      fd;
        uint32                       ip;
        uint16                       port;
        time_t                      startTime;
        uint32                       handle;
        CONN_STATE                  state;
        bool                        incoming;
        uint32                       remoteIp;
        uint16                       remotePort;
    };

    std::vector<PendingConn>    ConnectArray_;
    uint32                       HandleSequence_;
};

#endif
