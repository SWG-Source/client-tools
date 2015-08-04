//
// TCPQueue - handles buffering for incoming & outgoing TCP data
//
#include "clientGame/FirstClientGame.h"
#include "TCPQueue.h"

#pragma warning(disable: 4127) // conditional expression is constant

#ifndef MIN
#define MIN(x,y) (((x)<(y))?(x):(y))
#endif

#ifndef MAX
#define MAX(x,y) (((x)>(y))?(x):(y))
#endif

#ifndef FAIL_IF
#define FAIL_IF(x)  if (x)
#endif // FAIL_IF


TCPQueue::TCPQueue() :
    QueueLength( 0 )
{
}

TCPQueue::~TCPQueue()
{
    Clear();
}

void TCPQueue::Clear( void )
{
    // Pull out big chunks until done
    while ( Remove_Head( NULL, 0xFFFFFF ) );

    // Free the spare vectors
    int sparelen = SpareVectors.size();
    for ( int i = 0; i < sparelen; i++ )
    {
        TCPVector* delvec = 0;
        delvec = SpareVectors.at( i );
        delete( delvec );
    }
    SpareVectors.clear();
}

int TCPQueue::Length( void ) const
{
    return( QueueLength );
}


//
// Peek at the data in the first vector
//
bool TCPQueue::Peek_Head( uint8** buffer, int* len ) const
{
    FAIL_IF( !buffer )
    {
        return false;
    }
    FAIL_IF( !len )
    {
        return false;
    }

    if ( QueueLength == 0 )
        return( false );

    TCPVector* vecptr = NULL;

    vecptr = Queue.front();
    *buffer = vecptr->Buffer;
    *len = vecptr->Size;
    return( true );
}

//
// Remove upto 'maxlen' bytes from the head of the queue.
// Return bytes dequeued
//
int TCPQueue::Remove_Head( uint8* buffer, int maxlen )
{
    FAIL_IF( maxlen < 0 )
    {
        return 0;
    }

    if ( ( maxlen == 0 ) || ( QueueLength == 0 ) )
        return 0;

    int retval = 0;

    TCPVector* vecptr = NULL;
    while ( 1 )
    {
        FAIL_IF( Queue.size() == 0 )
        {
            return 0;
        }

        vecptr = Queue.front();
        FAIL_IF( vecptr == 0 )
        {
            return 0;
        }

        int copylen = MIN( vecptr->Size, maxlen );
        if ( buffer )
        {
            memcpy( buffer, vecptr->Buffer, copylen );
            buffer += copylen;
        }
        retval += copylen;
        maxlen -= copylen;
        vecptr->Size -= copylen;
        QueueLength -= copylen;

        if ( vecptr->Size == 0 )
        {
            vecptr = Queue.front();
            Queue.erase( Queue.begin() );

            SpareVectors.push_back( vecptr );
            // Make sure our spare list doesn't get too big
            // The +5 ensures we always have a few spares
            while ( SpareVectors.size() > ( Queue.size() + 5 ) )
            {
                TCPVector* delvec = SpareVectors.back();
                SpareVectors.erase( SpareVectors.end() - 1 );
                delete( delvec );
            }
        }
        else
        {
            // bah, gotta do some shifting
            memmove( vecptr->Buffer, vecptr->Buffer + copylen, vecptr->Size );
        }

        if ( maxlen == 0 )
            break;
        if ( QueueLength == 0 )
            break;
    }
    return( retval );
}


//
// Add 'length' bytes to the head of the queue.
// Returns bytes queued.
//
int TCPQueue::Add_Head( const uint8* buffer_in, int length_in )
{
    FAIL_IF( !buffer_in )
    {
        return 0;
    }
    FAIL_IF( length_in < 0 )
    {
        return 0;
    }

    int retval = 0;

    if ( length_in <= 0 )
        return( retval );

    retval = length_in;

    TCPVector* vecptr = NULL;
    if ( Queue.size() )
    {
        vecptr = Queue.at( 0 );
    }

    // work back to front
    const uint8* buffer = buffer_in;
    buffer += ( length_in / TCPVector::MaxLength ) * TCPVector::MaxLength;
    int length = length_in % TCPVector::MaxLength;
    if ( length == 0 )
    {
        // OK, length_in is a multiple of the vector length.  If we didn't do this
        //   our first vector would be empty.
        buffer -= TCPVector::MaxLength;
        length = TCPVector::MaxLength;
    }


    for ( int i = 0; ; i++ )
    {
        int copylen = MIN( length, TCPVector::MaxLength );

        // See if we need to add a new vector
        if ( ( vecptr == NULL ) || ( ( vecptr->Size + copylen ) > TCPVector::MaxLength ) )
        {
            // Is there a spare vector laying about?
            if ( SpareVectors.size() )
            {
                vecptr = SpareVectors.at( 0 );
                SpareVectors.erase( SpareVectors.begin() );
            }
            else
            {
                vecptr = new TCPVector;
            }
            Queue.insert( Queue.begin(), vecptr );
        }

        // Move any old data out of the way
        if ( vecptr->Size )
            memmove( vecptr->Buffer + copylen, vecptr->Buffer, vecptr->Size );

        // copy in the new stuff
        memcpy( vecptr->Buffer, buffer, copylen );

        buffer -= TCPVector::MaxLength;
        QueueLength += copylen;
        length = TCPVector::MaxLength;
        vecptr->Size += copylen;
        vecptr = NULL;

        if ( buffer < buffer_in )
            break;
    }
    return( retval );
}

//
// Add 'length' bytes to the end of the queue.
// Returns bytes queued.
//
int TCPQueue::Add_Tail( const uint8* buffer, int length )
{
    FAIL_IF( !buffer )
    {
        return 0;
    }
    FAIL_IF( length < 0 )
    {
        return 0;
    }

    if ( length == 0 )
        return 0;

    int retval = length;

    TCPVector* vecptr = NULL;
    if ( Queue.size() )
    {
        vecptr = Queue.back();
    }

    while ( 1 )
    {
        int copylen = MIN( length, TCPVector::MaxLength );

        // If there's some extra space in the tail vector, use that before
        //  allocating a new one...
        if ( ( vecptr ) && ( vecptr->Size < TCPVector::MaxLength ) )
        {
            copylen = MIN( TCPVector::MaxLength - vecptr->Size, copylen );
        }

        // See if we need to add a new vector
        if ( ( vecptr == NULL ) || ( ( vecptr->Size + copylen ) > TCPVector::MaxLength ) )
        {
            // Is there a spare vector laying about?
            if ( SpareVectors.size() )
            {
                vecptr = SpareVectors.front();
                SpareVectors.erase( SpareVectors.begin() );
            }
            else
            {
                vecptr = new TCPVector;
            }
            Queue.push_back( vecptr );
        }

        memcpy( vecptr->Buffer + vecptr->Size, buffer, copylen );

        buffer += copylen;
        QueueLength += copylen;
        length -= copylen;
        vecptr->Size += copylen;
        vecptr = NULL;

        if ( length == 0 )
            break;
    }
    return( retval );
}
