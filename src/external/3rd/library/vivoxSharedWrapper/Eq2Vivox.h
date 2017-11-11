#ifndef EQ2VIVOX_H
#define EQ2VIVOX_H

#include "Platform.hpp"
#include "Vivox.h"
#include "SharedString.hpp"
#include "VeLog.hpp"
#include "EqLog.h"
#include "VeUtil.hpp"
#include "Verify.hpp"

struct Eq2VivoxGlue
{
    static void log( VivoxLogSeverity severity, const char* pattern, ... )
    {
        va_list valist;
        va_start( valist, pattern );
        unsigned uFlags = 0;
        switch ( severity )
        {
        case VLS_CRITICAL: // Maps to error
        case VLS_ERROR: uFlags = EqLog::ERR; break;
        case VLS_INFO:  uFlags = EqLog::INF; break;
        case VLS_WARN:  uFlags = EqLog::WRN; break;
        }
        VeLog::log_va( "VIVOX", uFlags, pattern, valist );
        va_end( valist );
    }

    static uns64 getTimeMS()
    {
        return VeUtil::getGameTimeMilliseconds();
    }

    static void sleep( unsigned ms )
    {
        VeUtil::sleep( ms );
    }

    static void warning( const char* sFile, int iLine, const char* pattern, ... )
    {
        va_list valist;
        va_start( valist, pattern );
        char buffer[1024];
        _vsnprintf( buffer, sizeof(buffer), pattern, valist );
        buffer[sizeof(buffer) - 1] = '\0';
        Verify( sFile, iLine ).alrt( buffer );
        va_end( valist );
    }

    static void fatal( const char* sFile, int iLine, const char* pattern, ... )
    {
        va_list valist;
        va_start( valist, pattern );
        char buffer[1024];
        _vsnprintf( buffer, sizeof(buffer), pattern, valist );
        buffer[sizeof(buffer) - 1] = '\0';
        Verify( sFile, iLine ).ver( false, buffer );
        va_end( valist );
    }

    static const char* getGameName()
    {
        return "EverQuest2";
    }

    static int getLogLevel()
    {
        // From Eq2VivoxClient.cpp
        extern int vivox_loglevel;
        return vivox_loglevel;
    }
};

typedef Vivox< CharString, Eq2VivoxGlue > Eq2Vivox;

#endif