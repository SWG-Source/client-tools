//
// TCPQueue - handles buffering for incoming & outgoing TCP data
//
#ifndef TCPQUEUE_H
#define TCPQUEUE_H
#include <vector>

//
// Used to queue incoming and outgoing data efficiently
//
class TCPQueue
{
public:
            TCPQueue();
            ~TCPQueue();

    void    Clear( void );
    int     Length( void ) const;
    int     Remove_Head( uint8* buffer, int maxlen );

    int     Add_Tail( const uint8* buffer, int length );
    int     Add_Head( const uint8* buffer, int length );

    bool    Peek_Head( uint8** buffer, int* len ) const;

private:

    class   TCPVector
    {
    public:

        enum
        {
            MaxLength = 1024
        };

        TCPVector()
        {
            Buffer = new uint8[MaxLength]; Size = 0;
        };
        ~TCPVector()
        {
            delete[]( Buffer );
        }

        uint8*                       Buffer;
        int                         Size;
    };


    std::vector<TCPVector*>     Queue;
    int                         QueueLength;

    // Keep vectors we don't need here so we don't have to realloc them later
    std::vector<TCPVector*>     SpareVectors;
};

#endif
