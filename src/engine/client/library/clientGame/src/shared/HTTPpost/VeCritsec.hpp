#ifndef VECritsecH
#define VECritsecH

#ifdef _WIN32

#pragma warning( disable : 4740 ) // disable warning C4740: flow in or out of inline asm code suppresses global optimization

// Explicit declaration to avoid having to #include <windows.h>
extern "C"
{
    extern __declspec( dllimport ) unsigned long __stdcall GetCurrentThreadId( void );
}

#else
#include <pthread.h>
#endif


class VeCritsec
{
public:
    VeCritsec() : m_iLock(0), m_uThreadID(0xFFFFFFFF), m_uLockCount(0) {}
        
    //
    // Try and get the lock
    //        
    inline bool trylock ( void )
    {
#ifdef _WIN32    
        //
        // This looks like a race condition, but it isn't.  Yes the value of m_uThreadID
        // can change during the comparison but it can't change to or from our threadID
        // and that's all we care about.
        //
        uint32 uCallingThread = GetCurrentThreadId();
        if ( uCallingThread == m_uThreadID )
        {
            ++m_uLockCount;  // we already have the lock...
            return( true );
        }

        volatile unsigned int* p_i_lock = &m_iLock;
        __asm 
        {
            mov esi,[p_i_lock]
            lock bts dword ptr [esi], 0
            jnc Locked
        }
        return( false );
        
        Locked:
        m_uThreadID = uCallingThread;
        m_uLockCount = 1;
        return( true );    
#else
        //
        // This looks like a race condition, but it isn't.  Yes the value of m_uThreadID
        // can change during the comparison but it can't change to or from our threadID
        // and that's all we care about.
        //
        uint32 uCallingThread = pthread_self();
        if ( uCallingThread == m_uThreadID )
        {
            ++m_uLockCount;  // we already have the lock...
            return( true );
        }
        
        int oldbit = 0;
        volatile void* p_i_lock = ( volatile void* )&m_iLock;

        __asm__ __volatile__( "lock ; "
                "btsl %2,%1\n\tsbbl %0,%0"
                :"=r" (oldbit),"=m" (*(volatile long *) p_i_lock)
                :"Ir" (0) : "memory");
       
        if ( oldbit )  // previously set, lock failed
            return( false );
            
        m_uThreadID = uCallingThread;
        m_uLockCount = 1;            
        return( true );
        
#endif    
    }

    //
    // Release the lock
    //
    inline void unlock(void)
    {
        --m_uLockCount;
        if ( m_uLockCount == 0 )
        {
            m_uThreadID = 0xFFFFFFFF;
            m_iLock = 0;  // This actually releases the lock
        }
    }
    
    //
    // Spin with yields until we get the lock.
    //
    inline void lock(void)
    {
        int failcount=0;
        while( ! trylock() ) 
        {
            ++failcount;
            // Set test to a higher value if you want to enable spin locks
            // Spin locks are only a win on multiprocessor machines
            if ( failcount == 1 )
            {
                yield_thread();            
                failcount=0;
            }
        }
    }
    
    void yield_thread(void);
    
private:
    volatile unsigned int m_iLock;
    volatile uint32 m_uThreadID;
    volatile uint32 m_uLockCount;
};


#endif

