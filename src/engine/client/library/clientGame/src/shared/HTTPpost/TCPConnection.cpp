#include "clientGame/FirstClientGame.h"
#include "TCPConnection.h"
#include <time.h>
#include "VeTime.hpp"
#include <stdarg.h>

#pragma warning(disable: 4127) // conditional expression is constant

#ifndef MIN
#define MIN(x,y) (((x)<(y))?(x):(y))
#endif

#ifndef MAX
#define MAX(x,y) (((x)>(y))?(x):(y))
#endif

#define DEFAULT_TCP_DELAY 15


TCPConnection::TCPConnection( SOCKET sock ) :
    BufferedWrites_( false ),
    TCPManagerPtr_( NULL ),
    MaxReadBuffer_( 65536 ),
    ThreadSafe_( 0 ),
    RemoteIP_( 0 ),
    RemotePort_( 0 )
{
    Socket_ = sock;

    isConnected();  // This will set the internal connect status

    setInputDelay( DEFAULT_TCP_DELAY ); 
    setOutputDelay( DEFAULT_TCP_DELAY );    

    getRemoteAddr( &RemoteIP_, &RemotePort_ );
}

TCPConnection::~TCPConnection()
{
    //fprintf(stderr,"TCPCON destructor!!!!!\n");
    lock();

    TCPConnection::close();    // will try and flush the write queue

    ::closesocket( Socket_ );
    Socket_ = INVALID_SOCKET;

    if ( ( BufferedWrites_ ) && ( TCPManagerPtr_ ) )
        TCPManagerPtr_->setBufferedWrites( this, false );

    unlock();
}


void TCPConnection::setThreadsafe( bool threadsafe )
{
    ThreadSafe_ = threadsafe;
}


SOCKET TCPConnection::getFD( void )
{
    return( Socket_ );
}


void TCPConnection::close( bool force )
{
    lock();

    if ( ( BufferedWrites_ ) && ( !force ) )
    {
        while ( WriteQueue_.Length() )
        {
            VeTime::sleep( 1 );
            pumpWrites();
        }
    }
    // Don't close the same socket twice...
    if ( State_ != TCPManager::CLOSED )
    {
        shutdown( Socket_, 2 );  // sends and recvs
    }
    else
    {
        //fprintf(stderr," SOCKET %d already closed\n",Socket_);
    }

    State_ = TCPManager::CLOSED;
    WriteQueue_.Clear();

    unlock();
}


//
// Write data
//
// Returns 'n' bytes written, 0 if closed, or -1 for error.
//
int32 TCPConnection::write( const uint8* msg, uint32 len, int32 wait_secs )
{
    if ( State_ == TCPManager::CLOSED )
        return( 0 );

    if ( BufferedWrites_ )
    {
        lock();
        WriteQueue_.Add_Tail( msg, len );
        unlock();

        if ( pumpWrites() == 0 )
        {
            // send what I can
            return( 0 );    // endpoint closed, can't send anything...
        }
        else
            return( len );
    }
    else
    {
        return ( normalWrite( msg, len, wait_secs ) );
    }
}


//
// set buffered status 
//
void TCPConnection::setBufferedWrites( TCPManager* mgrptr, bool enabled )
{
    if ( enabled )
        BufferedWrites_ = true;
    else
    {
        lock();
        while ( WriteQueue_.Length() )
        {
            VeTime::sleep( 1 );
            pumpWrites();
        }
        unlock();

        BufferedWrites_ = false;
    }
    TCPManagerPtr_ = mgrptr;
}


//
// Try and send queued data (PRIVATE).  Return bytes sent, or 0 if closed, or -1 if can't send
//
int TCPConnection::pumpWrites( void )
{
    lock();

    if ( State_ == TCPManager::CLOSED )
        WriteQueue_.Clear();

    if ( WriteQueue_.Length() <= 0 )
    {
        unlock();
        if ( State_ == TCPManager::CLOSED )
            return( 0 );
        return( -1 );
    }

    int sendlen;
    int retval;
    uint8* bufptr = NULL;
    int total_sent = 0;
    bool endpoint_closed = false;

    while ( 1 )
    {
        if ( WriteQueue_.Peek_Head( &bufptr, &sendlen ) == false )
            break;
        if ( sendlen <= 0 )
            break;

        retval = normalWrite( ( uint8 * ) bufptr, sendlen, 0 );
        if ( retval > 0 )
            total_sent += retval;

        // Endpoint closed, remove all queued data
        if ( retval == 0 )
        {
            WriteQueue_.Clear();
            endpoint_closed = true;
        }

        if ( retval <= 0 )  // try again later
            break;

        WriteQueue_.Remove_Head( NULL, retval );

        // If we sent less than we wanted to, break out early...
        if ( retval < sendlen )
            break;
    }
    unlock();

    if ( endpoint_closed )
    {
        if ( total_sent )
            return( total_sent );
        return( 0 );
    }
    else
    {
        if ( total_sent )
            return( total_sent );
        return( -1 );
    }
}


//
// Non-buffered write (PRIVATE METHOD) 
//
// Returns 'n' bytes written, 0 if closed, or -1 for error (or can't send anything).
//
int32 TCPConnection::normalWrite( const uint8* msg, uint32 len, int32 wait_secs )
{
    if ( State_ == TCPManager::CLOSED )
    {
        // DBGMSG("Conn closed.");
        return( 0 );
    }

    if ( wait_secs < 0 )
        wait_secs = OutputDelay_;

    int32 retval = 0;
    int32 sendCount = 0;
    time_t start = 0;

    TCPManager::STATUS status;

    while ( 1 )
    {
        retval = send( Socket_, ( const char * ) ( msg + sendCount ), ( len - sendCount ), 0 );

        if ( retval == SOCKET_ERROR )
        {
            status = TCPManager::getStatus(); 
            if ( status == TCPManager::CONNRESET )
            {
                if ( sendCount )
                    return( sendCount );
                else
                {
                    //DBGMSG("Conn reset.");
                    return( 0 );
                }
            }
            if ( ( status != TCPManager::INTR ) &&
                 ( status != TCPManager::WOULDBLOCK ) &&
                 ( status != TCPManager::INPROGRESS ) &&
                 ( status != TCPManager::AGAIN ) )
            {
                if ( sendCount )
                    return( sendCount );
                else
                    return( -1 );
            }
        }
        else if ( retval > 0 )
        {
            sendCount += retval;

            if ( retval == int( len ) )
                break;
        }


        if ( wait_secs == 0 )  // bypass the sleep check below if wait_secs is 0
            break;

        time_t nowtime = time( 0 );
        if ( start == 0 )
            start = nowtime;

        int32 remaining_wait = int32(wait_secs) - (int32)( nowtime - start );
        if ( ( remaining_wait > 0 ) &&
             ( TCPManager::wait( remaining_wait, 0, &Socket_, 1, false ) > 0 ) )
            continue;   // I can write now.... 

        if ( remaining_wait <= 0 )
            break;
    }
    if ( sendCount == 0 )
        return( -1 );

    return( sendCount );
}

//
// Read data
//
// Returns 'n' bytes read, 0 for close, or -1 for error.
// This may return less than we asked for
//
int32 TCPConnection::read( uint8* msg, uint32 maxlen, int32 wait_secs )
{
    int32 retval = 0;
    time_t start = 0;
    char readBuffer[1024 + 1];  // +1 to NULL term in case we want to do a debug print

    if ( wait_secs < 0 )
        wait_secs = InputDelay_;

    lock();

    if ( ( State_ == TCPManager::CLOSED ) && ( ReadQueue_.Length() == 0 ) )
    {
        unlock();
        return( 0 );
    }

    TCPManager::STATUS status;

    while ( 1 )
    {
        // Do we even nead to read from the net?
        if ( ( ReadQueue_.Length() >= int( maxlen ) ) || ( State_ == TCPManager::CLOSED ) )
        {
            maxlen = MIN( maxlen, uint32( ReadQueue_.Length() ) );
            ReadQueue_.Remove_Head( msg, maxlen );
            unlock();
            return( maxlen );
        }

        do
        {
            retval = recv( Socket_, readBuffer, 1024, 0 );
            if ( retval > 0 )    // add to the tail of the list
            {
                readBuffer[retval] = 0;

                // Add to tail
                ReadQueue_.Add_Tail( ( uint8 * ) readBuffer, retval );
            }
        }
        while ( ( retval > 0 ) && ( ReadQueue_.Length() < int( maxlen ) ) );

        if ( ReadQueue_.Length() > 0 )  // OK, we'll take what we've got
        {
            int retcount = ReadQueue_.Remove_Head( msg, maxlen );
            unlock();
            return( retcount );
        }
        else if ( retval == 0 )
        {
            unlock();
            return( 0 );
        }
        else if ( retval == SOCKET_ERROR )
        {
                 status = TCPManager::getStatus(); 

            if ( status == TCPManager::CONNRESET )
            {
                unlock();
                return( 0 );
            }

            if ( ( status != TCPManager::INTR ) &&
                 ( status != TCPManager::WOULDBLOCK ) &&
                 ( status != TCPManager::INPROGRESS ) &&
                 ( status != TCPManager::AGAIN ) )
            {
                unlock();

                //fprintf(stderr,"UNKNOWN TCP STATUS: %d (%d)\n",status,errno);

                return( -1 );
            }
        }

        if ( wait_secs == 0 )  // bypass the sleep check below if wait_secs is 0
            break;

        time_t nowtime = time( 0 );
        if ( start == 0 )
            start = nowtime;

        int32 remaining_wait = static_cast<int32>(wait_secs - ( nowtime - start ));
        if ( ( remaining_wait > 0 ) && ( TCPManager::wait( remaining_wait, 0, &Socket_, 1, true ) > 0 ) )
            continue;   // I can read now.... 

        if ( remaining_wait <= 0 )
            break;
    }
    unlock();
    return( retval );
}




//
// Read incoming data into the buffer
//
// Returns 'n' bytes read, 0 for close, or -1 for error.
//
int32 TCPConnection::queueIncoming( void )
{
    int32 retval = 0;
    char readBuffer[1024 + 1];  // +1 to NULL term for debug prints

    if ( State_ == TCPManager::CLOSED )
        return( 0 );

    // If we already have a bunch of data, don't queue any more
    lock();
    if ( ReadQueue_.Length() >= MaxReadBuffer_ )
    {
        unlock();
        return( 0 );
    }
    unlock();

    do
    {
        retval = recv( Socket_, readBuffer, 1024, 0 );
        if ( retval > 0 )    // add to the tail of the list
        {
            // Add to tail
            lock();
            ReadQueue_.Add_Tail( ( uint8 * ) readBuffer, retval );
            unlock();
        }
    }
    while ( retval > 0 );

    if ( retval == SOCKET_ERROR )
    {
        retval = -1;
        int status = TCPManager::getStatus();
        if ( status == TCPManager::CONNRESET )
            return( 0 );
    }
    return( retval );
}


//
// Push data back onto the read queue
//
bool TCPConnection::unread( uint8* data, int length )
{
    if ( length <= 0 )
        return( false );

    lock();
    ReadQueue_.Add_Head( data, length );
    unlock();

    return( true );
}


//
// Returns 0 on failure
// Returns IP in host byte order!
//
bool TCPConnection::getRemoteAddr( uint32* ip, uint16* port )
{
    // See if the cached versions are valid...
    if ( ( RemoteIP_ ) && ( RemotePort_ ) )
    {
        if ( ip )
            *ip = RemoteIP_;
        if ( port )
            *port = RemotePort_;
        return( true );
    }

    struct sockaddr_in sin;
    socklen_t sinSize = sizeof( sin );

    if ( getpeername( Socket_, ( sockaddr * ) &sin, &sinSize ) == 0 )
    {
        if ( ip )
            *ip = ntohl( sin.sin_addr.s_addr );
        if ( port )
            *port = ntohs( sin.sin_port );
        return( true );
    }
    return( false );
}

//
// only use for strings up to 4096 chars!
//
int32 TCPConnection::printf( const char* format, ... )
{
    va_list arg;
    char string[4097];
    int32 retval;
    va_start( arg, format );
    vsprintf( string, format, arg );
    va_end( arg );
    string[4096] = 0;

    retval = write( ( const uint8 * ) string, uint32(strlen( string )), OutputDelay_ );
    return( retval );
}


//
// Are we connected to the endpoint?
//
bool TCPConnection::isConnected( void )
{
    if ( Socket_ == INVALID_SOCKET )
    {
        State_ = TCPManager::CLOSED;
        return( false );
    }

    // Do a recv peek...
    char buffer[1];
    int32 retval = recv( Socket_, buffer, 1, MSG_PEEK );

    if ( retval == SOCKET_ERROR )
    {
        TCPManager::STATUS status = TCPManager::getStatus();
        switch ( status )
        {
        case TCPManager::CONNRESET:
        case TCPManager::NOTSOCK:
        case TCPManager::BADF:
        case TCPManager::INVAL:
            retval = 0;
            break;

        default:
            break;
        }
    }


    if ( retval == 0 )
    {
        State_ = TCPManager::CLOSED;
        shutdown( Socket_, 2 );  // sends and recvs
        return( false );
    }
    else
    {
        State_ = TCPManager::CONNECTED;
        return( true );
    }
}


