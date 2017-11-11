#include "clientGame/FirstClientGame.h"
#include "TCPManager.h"
#include "TCPConnection.h"
#include "VeTime.hpp"
#include <assert.h>

#pragma warning(disable: 4127) // conditional expression is constant

#ifdef _UNIX

#define USE_POLL
#ifdef USE_POLL
#include <sys/poll.h>
#endif
#endif

#undef DBGMSG
#define DBGMSG(X)



TCPManager::TCPManager()
{
    HandleSequence_ = 1;
}

TCPManager::~TCPManager()
{
    // close all the listener sockets.
    while( ListenArray_.size() )
    {
        ListenSocket ls( ListenArray_.back() );
        ::closesocket( ls.fd );
        ListenArray_.pop_back();
    }
}


//
// Add a listener socket to accept connections on a given port
//
bool TCPManager::addListener( uint32 ip, uint16 port, bool reuseAddr )
{
    SOCKET fd = createSocket( ip, port, reuseAddr );
    if ( fd == INVALID_SOCKET )
        return( false );

    listen( fd, 64 );   // Solaris needs lots of listen slots

    ListenSocket lsock;
    lsock.fd = fd;
    lsock.ip = ip;
    lsock.port = port;

    ListenArray_.push_back( lsock );

    return( true );
}

//
// Remove listener on a given ip/port 
//
bool TCPManager::removeListener( uint32 ip, uint16 port )
{
    ListenSocket* lptr;
    for ( int i = 0; i < int( ListenArray_.size() ); i++ )
    {
        lptr = &( ListenArray_[i] );
        if ( ( lptr->ip == ip ) && ( lptr->port == port ) )
        {
            ::closesocket( lptr->fd );
            ListenArray_.erase( ListenArray_.begin() + i );
            return( true );
        }
    }
    return( false );
}

//
// Get the socket for a given listener
//
bool TCPManager::getListener( uint32 ip, uint16 port, OUT SOCKET& outsock )
{
    ListenSocket* lptr;
    for ( unsigned int i = 0; i < ListenArray_.size(); i++ )
    {
        lptr = &( ListenArray_[i] );
        if ( ( ( lptr->ip == ip ) || ( ip == 0 ) ) && ( lptr->port == port ) )
        {
            outsock = lptr->fd;
            return( true );
        }
    }
    return( false );
}


//
// Enable/Disable buffered writes on a socket
//
bool TCPManager::setBufferedWrites( TCPConnection* con, bool enabled )
{
    TCPConnection* tempptr = NULL;

    // Check to see if this connection is already in our list
    for ( int i = 0; i < int( BufferedWriters_.size() ); i++ )
    {
        tempptr = BufferedWriters_.at( i );
        if ( tempptr == con )
        {
            con->setBufferedWrites( this, false );
            BufferedWriters_.erase( BufferedWriters_.begin() + i );
            break;
        }
    }
    if ( enabled )  // OK, now add to the list
    {
        con->setBufferedWrites( this, true );
        BufferedWriters_.push_back( con );
    }
    return( true );
}


//
// Let all the buffered writers send data
//
void TCPManager::pumpWriters( void )    // pump the buffered writer connections
{
    TCPConnection* conptr = NULL;

    // Check to see if this connection is already in our list
    for ( int i = 0; i < int( BufferedWriters_.size() ); i++ )
    {
        conptr = BufferedWriters_.at( i );
        conptr->pumpWrites();
    }
}



//
// Connect by hostname rather than IP
//
bool TCPManager::connect( const char* host, uint16 port, OUT uint32* handle )
{
    char hostName[129];
    struct hostent* hostStruct;
    struct in_addr* hostNode;

    if ( isdigit( host[0] ) )
        return ( connect( ntohl( inet_addr( host ) ), port, handle ) );

    strcpy( hostName, host );
    hostStruct = gethostbyname( host );
    if ( hostStruct == NULL )
        return ( 0 );
    hostNode = ( struct in_addr * ) hostStruct->h_addr;
    return ( connect( ntohl( hostNode->s_addr ), port, handle ) );
}



//
// Request a connection to a given address (all values in host byte order)
//
bool TCPManager::connect( uint32 ip, uint16 port, OUT uint32* handle )
{
    PendingConn pConn;
    if ( ( pConn.fd = createSocket( ( uint32 ) 0, ( uint16 ) 0, false ) ) == INVALID_SOCKET )
        return( false );
    pConn.ip = 0;
    pConn.port = 0;
    pConn.remoteIp = ip;
    pConn.remotePort = port;
    pConn.startTime = time( NULL );
    pConn.handle = HandleSequence_++;
    pConn.state = CLOSED;
    pConn.incoming = false;  // outgoing connection

    ConnectArray_.push_back( pConn );

    *handle = pConn.handle;
    return( true );
}

//
// Get an incoming connection (specify handle or 0 for any)
//
// Wait for upto 'wait_secs' seconds for the connection.
//
bool TCPManager::getOutgoingConnection( TCPConnection** conn, uint32 handle, int32 wait_secs )
{
    return( getConnection( conn, handle, 0, wait_secs, OUTGOING ) );
}


//
// Get an incoming connection (specify listen port or 0 for any) 
//
// Wait for upto 'wait_secs' seconds for the connection.
//
bool TCPManager::getIncomingConnection( TCPConnection** conn, uint16 port, int32 wait_secs )
{
    return( getConnection( conn, INVALID_HANDLE, port, wait_secs, INCOMING ) );
}


//
// cancel a pending outgoing connection
//
bool TCPManager::cancelConnection( uint32 handle )
{
    PendingConn* connPtr = NULL;
    for ( int i = 0; i < int( ConnectArray_.size() ); i++ )
    {
        if ( int( ConnectArray_.size() ) <= i )
        {
            continue;
        }
        connPtr = &( ConnectArray_.at( i ) );
        if ( connPtr == 0 )
        {
            continue;
        }

        if ( connPtr->handle == handle )
        {
            ::closesocket( connPtr->fd );
            ConnectArray_.erase( ConnectArray_.begin() + i );
            return( true );
        }
    }
    return( false );
}

#ifdef USE_POLL
//
// Return after there is data to read, or we've timed out.
//
int TCPManager::wait( uint32 sec, uint32 usec, SOCKET* sockets, int count, bool readMode )
{
    DBGMSG( "Waiting on " << count << " sockets" );

    struct pollfd temp_poll_fd;     // optimization: single socket waits avoid malloc
    struct pollfd* poll_array = 0;  // pointer to the array of poll structs
    if ( count == 1 )
    {
        poll_array = &temp_poll_fd;
    }
    else
    {
        poll_array = ( struct pollfd * ) malloc( count * sizeof( struct pollfd ) );
    }

    // pick the events that we are polling on
    short events = ( readMode == true ? POLLIN : POLLOUT );

    // set up the array of poll structs
    uint32 i;
    for ( i = 0; i < ( uint32 ) count; ++i )
    {
        poll_array[i].fd = sockets[i];
        poll_array[i].events = events;
        poll_array[i].revents = 0;
    }

    // compute end time for this call
    VeTime timenow;
    VeTime timedone( timenow );
    timedone.AddSec( sec );
    timedone.AddUsec( usec );

    bool noTimeout = false;
    if ( ( sec == -1 ) || ( usec == -1 ) )
        noTimeout = true;

    int timeout = -1;

    int retval = 0;
    bool done = false;
    while ( !done )
    {
        // if we're waiting for a specific interval, update timeout value 
        // based on end time for this call (in case it got interrupted)
        if ( noTimeout == false )
        {
            VeTime timediff = timedone - timenow;
            timeout = timediff.GetSec() * 1000 + timediff.GetUsec() / 1000;
        }

        // poll our sockets
        int retval = ::poll( poll_array, ( unsigned int ) count, timeout );

        DBGMSG( "Select wake" );

        // retval is number of sockets that were ready, 0 if call timed out, -1 if error occured
        if ( retval >= 0 )
        {
            done = true;
        }
        else if ( ( retval == SOCKET_ERROR ) && ( getStatus() == INTR ) )
        {
            // did the call get interrupted?
            // if we're waiting for a specific interval, check whether we're done.
            if ( noTimeout == false )
            {
                timenow.Update();
                if ( timenow >= timedone )
                {
                    done = true;
                }
            }
        }
        else
        {
            done = true;
        }
    }

    if ( count > 1 )
    {
        free( poll_array );
    }

    return ( retval );
}
#else
//
// Return after there is data to read, or we've timed out.
//
int TCPManager::wait( uint32 sec, uint32 usec, SOCKET* sockets, int count, bool readMode )
{
    VeTime timeout, timenow, timethen;
    fd_set givenSet;
    fd_set returnSet;
    fd_set backupSet;
    int givenMax = 0;
    bool noTimeout = false;
    int retval = 0;
    uint32 i;

    DBGMSG( "Waiting on " << count << " sockets" );

    FD_ZERO( &givenSet );
    for ( i = 0; i < ( uint32 ) count; i++ )
    {
        FD_SET( sockets[i], &givenSet );
    }

    timeval tv;
    timeval* tvPtr = NULL;
    returnSet = givenSet;
    backupSet = givenSet;

    if ( ( sec == -1 ) || ( usec == -1 ) )
        noTimeout = true;

    timeout.setDay( 0 );
    timeout.setMsec( (sec*1000) + (usec/1000) );
    timethen += timeout;

    for ( i = 0; i < ( uint32 ) count; i++ )
    {
        if ( sockets[i] > ( SOCKET ) givenMax )
            givenMax = int(sockets[i]);
    }

    bool done = false;
    while ( !done )
    {
        tvPtr = &tv;
        if ( noTimeout )
            tvPtr = NULL;
        else
        {
            tv.tv_usec = (timeout.getMsec() % 1000) * 1000;
            tv.tv_sec = timeout.getMsec() / 1000;
        }

        if ( readMode )  // can we read?
            retval = select( givenMax + 1, &returnSet, 0, 0, tvPtr );
        else                        // can we write?
            retval = select( givenMax + 1, 0, &returnSet, 0, tvPtr );

        DBGMSG( "Select wake" );

        if ( retval >= 0 )
            done = true;
        else if ( ( retval == SOCKET_ERROR ) && ( getStatus() == INTR ) )   // in case of signal
        {
            if ( noTimeout == false )
            {
                timenow.update();
                timeout = timethen - timenow;
            }
            if ( ( noTimeout == false ) && ( timeout.getDay() <= 0 ) && ( timeout.getMsec() <= 0 ) )
                done = true;
            else
                returnSet = backupSet;
        }
        else    // maybe out of memory?
        {
            done = true;
        }
    }
    return( retval );
}
#endif // USE_POLL


TCPManager::STATUS TCPManager::getStatus( void )
{
#ifdef _WIN32
    int status = WSAGetLastError();
    if ( status == 0 )
        return( OK );
    else if ( status == WSAEINTR )
        return( INTR );
    else if ( status == WSAEINPROGRESS )
        return( INPROGRESS );
    else if ( status == WSAECONNREFUSED )
        return( CONNREFUSED );
    else if ( status == WSAEINVAL )
        return( INVAL );
    else if ( status == WSAEISCONN )
        return( ISCONN );
    else if ( status == WSAENOTSOCK )
        return( NOTSOCK );
    else if ( status == WSAETIMEDOUT )
        return( TIMEDOUT );
    else if ( status == WSAEALREADY )
        return( ALREADY );
    else if ( status == WSAEWOULDBLOCK )
        return( WOULDBLOCK );
    else if ( status == WSAEBADF )
        return( BADF );
    else if ( status == WSAECONNRESET )
        return( CONNRESET );
    else if ( status == WSAESHUTDOWN )
        return( CONNRESET );
    else if ( status == WSAENOTCONN )
        return( CONNRESET );
    else
        return( UNKNOWN );
#else
    int status = errno;
    if ( status == 0 )
        return( OK );
    else if ( status == EINTR )
        return( INTR );
    else if ( status == EINPROGRESS )
        return( INPROGRESS );
    else if ( status == ECONNREFUSED )
        return( CONNREFUSED );
    else if ( status == EINVAL )
        return( INVAL );
    else if ( status == EISCONN )
        return( ISCONN );
    else if ( status == ENOTSOCK )
        return( NOTSOCK );
    else if ( status == ETIMEDOUT )
        return( TIMEDOUT );
    else if ( status == EALREADY )
        return( ALREADY );
    else if ( status == EAGAIN )
        return( AGAIN );
    else if ( status == EWOULDBLOCK )
        return( WOULDBLOCK );
    else if ( status == ECONNRESET )
        return( CONNRESET );
    else if ( status == EPIPE )
        return( CONNRESET );
    else if ( status == ENOTCONN )
        return( CONNRESET );
    else if ( status == EBADF )
        return( BADF );
    else
        return( UNKNOWN );
#endif

}


/********************* BEGIN PRIVATE METHODS *********************************/

//
// Create a bound socket
//
SOCKET TCPManager::createSocket( uint32 ip, uint16 port, bool reuseAddr )
{
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons( port );
    addr.sin_addr.s_addr = htonl( ip );

    SOCKET fd = socket( AF_INET, SOCK_STREAM, DEFAULT_PROTOCOL );
    if ( fd == -1 )
        return( INVALID_SOCKET );

    if ( setBlocking( fd, false ) == false )
        return( INVALID_SOCKET );

    if ( reuseAddr )
    {
        uint32 opval = 1;
        if ( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, ( char * ) &opval, sizeof( opval ) ) != 0 )
        {
            ::closesocket( fd );
            return( INVALID_SOCKET );
        }
    }
    if ( bind( fd, ( struct sockaddr * ) &addr, sizeof( addr ) ) == SOCKET_ERROR )
    {
        ::closesocket( fd );
        return( INVALID_SOCKET );
    }
    return( fd );
}


//
// Set the blocking mode of the socket 
//
bool TCPManager::setBlocking( SOCKET fd, bool block )
{
#ifdef _WIN32
    unsigned long flag = 1;
    if ( block )
        flag = 0;
    int retval;
    retval = ioctlsocket( fd, FIONBIO, &flag );
    if ( retval == SOCKET_ERROR )
        return( false );
    else
        return( true );
#else   // UNIX
    int flags = fcntl( fd, F_GETFL, 0 );
    if ( block == false )                   // set nonblocking
        flags |= O_NONBLOCK;
    else                                             // set blocking
        flags &= ~( O_NONBLOCK );

    if ( fcntl( fd, F_SETFL, flags ) < 0 )
    {
        return( false );
    }
    return( true );
#endif
}

//
// conn may be NULL if the connection failed
//
bool TCPManager::getConnection( TCPConnection** conn, uint32 handle, uint16 port, int32 wait_secs, DIRECTION dir )
{
    PendingConn* connPtr = NULL;
    time_t start = time( NULL );
    SOCKET fdArray[1024];

    for ( int i = 0; i < int( ConnectArray_.size() ); i++ )
    {
        connPtr = &( ConnectArray_.at( i ) );

        fdArray[i] = connPtr->fd;
    }

    while ( 1 )
    {
        pumpConnections();

        for ( int i = 0; i < int( ConnectArray_.size() ); i++ )
        {
            connPtr = &( ConnectArray_.at( i ) );

            if ( ( connPtr->state != CONNECTED ) && ( connPtr->state != CONN_ERROR ) )
                continue;
            if ( ( !( ( int ) dir & ( int ) INCOMING ) ) && ( connPtr->incoming == true ) )
                continue;
            if ( ( !( ( int ) dir & ( int ) OUTGOING ) ) && ( connPtr->incoming == false ) )
                continue;
            if ( ( handle != INVALID_HANDLE ) && ( handle != connPtr->handle ) )
                continue;
            if ( ( port != 0 ) && ( port != connPtr->port ) )
                continue;
            if ( connPtr->state != CONN_ERROR )
                *conn = new TCPConnection( connPtr->fd );
            else
                *conn = NULL;

            ConnectArray_.erase( ConnectArray_.begin() + i );

            return( true );
        }
        // Wait for socket activity for a bit
        int32 remaining_wait = static_cast<int32>(wait_secs - ( time( NULL ) - start ));
        if ( ( remaining_wait > 0 ) &&
             ( wait( remaining_wait, 0, fdArray, int(ConnectArray_.size()), false ) > 0 ) )
            continue;   // got something!

        if ( remaining_wait <= 0 )
            break;
    }
    return( false );
}





// 
// TOFIX: don't forget about connect flooding on Win32
//
void TCPManager::pumpConnections( void )
{
    PendingConn* connPtr = NULL;
    STATUS status;
    int i;
    int retval = 0;

    // Outgoing connections
    for ( i = 0; i < int( ConnectArray_.size() ); i++ )
    {
        connPtr = &( ConnectArray_.at( i ) );

        retval = SOCKET_ERROR;

        if ( ( connPtr->state == CLOSED ) || ( connPtr->state == CONNECTING ) )
        {
            struct sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_port = htons( connPtr->remotePort );
            addr.sin_addr.s_addr = htonl( connPtr->remoteIp );

            if ( connPtr->state == CONNECTING )
            {
#ifdef USE_POLL
                struct pollfd poll;
                poll.fd = connPtr->fd;
                poll.events = POLLOUT | POLLERR;
                poll.revents = 0;

                ::poll( ( struct pollfd * ) &poll, 1u, 0 );
                if ( ( poll.revents & POLLERR ) != 0 )
                {
                    // OK, the exception flag is set, guess we couldn't connect....
                    //                    MESSAGE_INF( ( MSG_DEFAULT, "EXCEPTION SET:" ) );
                    ::closesocket( connPtr->fd );
                    connPtr->state = CONN_ERROR;
                }
                else if ( ( poll.revents & POLLOUT ) != 0 )
                {
                    // OK, the write flag is set, we're good to go...
                    //                    MESSAGE_INF( ( MSG_DEFAULT, "Poll says we're connected:" ) );
                    connPtr->state = CONNECTED;
                }
#else
                // Check to see if we're connected
                fd_set wset;
                fd_set eset;
                FD_ZERO( &wset );
                FD_ZERO( &eset );
                FD_SET( connPtr->fd, &wset );
                FD_SET( connPtr->fd, &eset );
                timeval tv;
                tv.tv_sec = 0;
                tv.tv_usec = 0;
                select( int( connPtr->fd ) + 1, 0, &wset, &eset, &tv );
                if ( FD_ISSET( connPtr->fd, &eset ) )
                {
                    // OK, the exception flag is set, guess we couldn't connect....

                    ::closesocket( connPtr->fd );
                    connPtr->state = CONN_ERROR;
                }
                else if ( FD_ISSET( connPtr->fd, &wset ) )
                {
                    // OK, the write flag is set, we're good to go...
                    connPtr->state = CONNECTED;
                }
#endif // USE_POLL

            }
            else
            {
                retval = ::connect( connPtr->fd, ( struct sockaddr * ) &addr, sizeof( addr ) );

                if ( retval == SOCKET_ERROR )
                {
                    status = getStatus();

                    if ( status == ISCONN )
                        retval = 0;
                    else if ( ( status == INPROGRESS ) ||
                              ( status == ALREADY ) ||
                              ( status == WOULDBLOCK ) )
                    {
                        connPtr->state = CONNECTING;
                        continue;    // Move on to next pending conn...
                    }
                    else if ( ( status == CONNREFUSED ) ||
                              ( status == TIMEDOUT ) ||
                              ( status == CONNRESET ) )
                    {
                        // can't connect
                        ::closesocket( connPtr->fd );
                        connPtr->state = CONN_ERROR;
                        continue;    // Move on to next pending conn...
                    }
                    else // doh, real problem
                    {
                        // DRM (08/13/02): this assert was killing the
                        // global server due to authd generating CONNRESET.
                        // TODO: evaluate all possible error messages, and
                        // determine which should be error states.
                        // assert(0);
                        ::closesocket( connPtr->fd );
                        connPtr->fd = createSocket( connPtr->ip, connPtr->port, false );
                    }
                }
            }

            if ( retval == 0 )
            {
                connPtr->state = CONNECTED;
            }
        }
    } // for ConnectArray_; 

    // Incoming connections
    ListenSocket* listenPtr;
    struct sockaddr_in clientAddr;
    socklen_t addrlen;
    for ( i = 0; i < int( ListenArray_.size() ); i++ )
    {
        listenPtr = &( ListenArray_.at( i ) );

        while ( 1 ) // accept all incoming on each socket
        {
            addrlen = sizeof( clientAddr );
            SOCKET newFD = accept( listenPtr->fd, ( struct sockaddr* ) &clientAddr, &addrlen );
            if ( newFD != INVALID_SOCKET )
            {
                setBlocking( newFD, false );

                DBGMSG( "Connection accepted" );

                PendingConn newConn;
                newConn.fd = newFD;
                newConn.ip = 0;
                newConn.port = (uint16)listenPtr->port;
                newConn.remoteIp = ntohl( clientAddr.sin_addr.s_addr );
                newConn.remotePort = ntohs( clientAddr.sin_port );
                newConn.handle = HandleSequence_++;
                newConn.state = CONNECTED;
                newConn.incoming = true;
                newConn.remoteIp = ntohl( clientAddr.sin_addr.s_addr );
                newConn.remotePort = ntohs( clientAddr.sin_port );

                ConnectArray_.push_back( newConn );
            }
            else
                break;
        }
    }
    return;
}






