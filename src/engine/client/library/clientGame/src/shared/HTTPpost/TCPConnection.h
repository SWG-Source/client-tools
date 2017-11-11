#ifndef TCPCON_HEADER
#define TCPCON_HEADER

#include "TCPManager.h"
#include <vector>
#include "VeCritsec.hpp"
#include "TCPQueue.h"

class TCPManager;

class TCPConnection
{
public:
            TCPConnection( SOCKET sock );
            ~TCPConnection();

    void    setThreadsafe( bool threadsafe );

    SOCKET  getFD( void );

    void    close( bool force = false ); 
    int32   write( const uint8* msg, uint32 len, int32 wait_secs = -1 );
    int32   read( uint8* msg, uint32 maxlen, int32 wait_secs = -1 );
    int32   queueIncoming( void );  // buffer incoming data


    bool    unread( uint8* data, int length );
    bool    getRemoteAddr( uint32* ip, uint16* port );
    int32   printf( const char* format, ... );
    bool    isConnected( void );

    int getReadQueueLength( void )
    {
        return( ReadQueue_.Length() );
    }
    int getWriteQueueLength( void )
    {
        return( WriteQueue_.Length() );
    }

    bool setInputDelay( int32 delay )
    {
        InputDelay_ = delay; return( true );
    };
    bool setOutputDelay( int32 delay )
    {
        OutputDelay_ = delay; return( true );
    };

    bool setMaxReadBuffer( int32 buflen )
    {
        MaxReadBuffer_ = buflen; return( true );
    }

    int             pumpWrites( void );  // for buffered mode

private:
    friend class    TCPManager;

    // If we're in threadsafe mode these actually do locks, else no-ops
    inline void lock()
    {
        if ( ThreadSafe_ )
        {
            CritSec_.lock();
        }
    };
    inline void unlock()
    {
        if ( ThreadSafe_ )
        {
            CritSec_.unlock();
        }
    };

    void                setBufferedWrites( TCPManager* mgrptr, bool enabled );

    int32               normalWrite( const uint8* msg, uint32 len, int32 wait_secs = -1 );

    int32               InputDelay_;  // default max time for input
    int32               OutputDelay_; // default max time for output

    TCPManager::CONN_STATE  State_;

    SOCKET              Socket_;

    TCPQueue            ReadQueue_;            // reads are buffered
    int                 MaxReadBuffer_;    // max bytes of incoming data to buffer


    bool                BufferedWrites_;  // T/F buffer writes?

    TCPQueue            WriteQueue_;        // writes _can_ be buffered

    TCPManager*         TCPManagerPtr_;        // pointer to my manager object

    VeCritsec           CritSec_;            // lock for this object
    bool                ThreadSafe_;        // do we do the locking?

    uint32               RemoteIP_;            // cached endpoint IP & port, just for debugging...
    uint16               RemotePort_;
};

#endif
