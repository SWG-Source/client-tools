//==============================================================================
//
//  Basic Logging
//
//==============================================================================
//  Copyright (C) DejaTools, LLC.  All rights reserved.
//==============================================================================

#include "..\..\DejaLib.h"

#ifdef WIN32
#pragma comment( lib, "../../DejaLib.Win32.lib" )
#endif
#ifdef XBOX
#pragma comment( lib, "../../DejaLib.X360.lib" )
#endif

//------------------------------------------------------------------------------
//
//  Logging is accomplished via DEJA_TRACE.
//
//      void DEJA_TRACE( Channel, FormatString, ... )
//
//  Where:
//
//    - Channel is (preferably) a string literal or a const character pointer.
//      (Although const char pointers can be used, there are performance and
//      functionality penalties.  See the documentation for details.)
//    - FormatString is a string literal using enhanced printf style format 
//      codes.
//
//------------------------------------------------------------------------------

void BasicTrace( void )
{
    DEJA_TRACE( "Channel", "FormatString" );

    for( int i = 0; i < 10; i++ )
    {
        DEJA_TRACE( "Simple formatting", "Iteration: %d", i );
    }
}

//------------------------------------------------------------------------------
//
//  DEJA_TRACE format specifications should accept all ANSI compliant format
//  strings.
//
//  **NOTE**  The string conversion implementation is still under development.
//  Some standard ANSI options and fields may not be respected in the output.
//  In particular, the width and precision specifications are not operational.
//  All values displayed will be correct, even if not formatted exactly as
//  specified.
//
//  Type field specification characters eEgGaA currently revert to f.
//
//  Additional format specifciation extensions are discussed in later tutorials
//  and the documentation.  They include:
//      %$              - Object label.
//      %(enum_type)    - Enumerated type value strings.
//
//------------------------------------------------------------------------------

void FormatSpecifications( void )
{
    //
    // ANSI standard type specifications.
    //

    const char* pTools = "Tools";

    DEJA_TRACE( "%c and %s", "%c%c%s%s",  68, 'e', "ja", pTools );
    DEJA_TRACE( "%d and %i", "%d and %i", 200, 200 );
    DEJA_TRACE( "%o and %u", "%o and %u", 200, 200 );
    DEJA_TRACE( "%x and %X", "%x and %X", 200, 200 );
    DEJA_TRACE( "%e and %E", "%e and %E", 200.0f, 200.0f );
    DEJA_TRACE( "%f"       , "%f"       , 200.0f         );
    DEJA_TRACE( "%g and %G", "%g and %G", 200.0f, 200.0f );
    DEJA_TRACE( "%a and %A", "%a and %A", 200.0f, 200.0f );

    //
    // Extended type specifications.
    //

    void*       pVoid  = (void*)0x12345678;
    long        Long   = 200;
    long long   Longer = 200;

    DEJA_TRACE( "%p and %P", "%p and %P", pVoid, pVoid );
    DEJA_TRACE( "%p and %P", "%p and %P", &Long, &Longer );
    DEJA_TRACE( "%ld"      , "%ld"      , Long );
    DEJA_TRACE( "%lld"     , "%lld"     , Longer );
}

//------------------------------------------------------------------------------
//
//  DEJA_TRACE accepts both "narrow" (char) and "wide" (wchar_t) parameters.
//
//  Format specification extentions allow mixing of wide and narrow arguments:
//    - %c (lower case) = character of same     width as format string.
//    - %C (upper case) = character of opposite width as format string.
//    - %s (lower case) = string    of same     width as format string.
//    - %S (upper case) = string    of opposite width as format string.
//    - %hs  and  %hS   = narrow string.
//    - %ls  and  %lS   = wide   string.
//
//------------------------------------------------------------------------------

void WideCharSupport( void )
{
    DEJA_TRACE(  "Narrow channel",  "Narrow message" );
    DEJA_TRACE( L"WIDE   channel",  "Narrow message" );
    DEJA_TRACE(  "Narrow channel", L"WIDE   message" );
    DEJA_TRACE( L"WIDE   channel", L"WIDE   message" );

    DEJA_TRACE(  "Mixed chars",     "Narrow format: %c and %C",   'n',      L'W' );
    DEJA_TRACE(  "Mixed chars",    L"WIDE   format: %C and %c",   'n',      L'W' );

    DEJA_TRACE(  "Mixed strings",   "Narrow format: %s and %S",   "narrow", L"WIDE" );
    DEJA_TRACE(  "Mixed strings",  L"WIDE   format: %S and %s",   "narrow", L"WIDE" );

    DEJA_TRACE(  "Mixed strings",   "Narrow format: %hs and %ls", "narrow", L"WIDE" );
    DEJA_TRACE(  "Mixed strings",  L"WIDE   format: %hs and %ls", "narrow", L"WIDE" );
    DEJA_TRACE(  "Mixed strings",   "Narrow format: %hS and %lS", "narrow", L"WIDE" );
    DEJA_TRACE(  "Mixed strings",  L"WIDE   format: %hS and %lS", "narrow", L"WIDE" );
}

//------------------------------------------------------------------------------
//
//  The Channel argument to DEJA_TRACE can be used to filter Insight's Trace Log 
//  view.  Using the Trace Filter view in Insight, channels can be hidden or 
//  disabled.
//
//  Channels are organized in the Trace filter view hierarchically using "::"
//  (the C++ scope operator) as a delimiter.  Multiple levels are permitted.
//
//  In general, we suggest using 'class_name::FunctionName' or
//  'SubSystem::FunctionName'.  This would produce a tree of channels including
//  all logged functions grouped under their respective classes or subsystems.
//
//------------------------------------------------------------------------------

void HierarchicalChannels( void )
{
    DEJA_TRACE( "One",             "One level deep" );
    DEJA_TRACE( "One::Two",        "Two levels deep" );
    DEJA_TRACE( "One::Two::Three", "Three levels deep" );

    DEJA_TRACE( "class::Function", "Associated with class::Function" );

    // Contrived examples.
    DEJA_TRACE( "object_mgr::CreateObject",  "Object created"            );
    DEJA_TRACE( "object_mgr::DestroyObject", "Object destroyed"          );
    DEJA_TRACE( "object_mgr::ApplyDamage",   "Applying damage to object" );
    DEJA_TRACE( "turret::SearchForTarget",   "Scanning for target"       );
    DEJA_TRACE( "turret::LockOnTarget",      "Target identified"         );
    DEJA_TRACE( "turret::TrackTarget",       "Tracking target"           );
    DEJA_TRACE( "turret::FirePrimary",       "Firing on target"          );

    // Of course, the examples above would be far more useful with arguments
    // identifying the object/turret/target.  We're just showing the channels
    // for now.
}

//------------------------------------------------------------------------------
//
//  In addition to DEJA_TRACE, you can use DEJA_WARNING, DEJA_ERROR, and
//  DEJA_BOOKMARK for logging.  All function exactly the same with the only
//  difference being how they are displayed in Insight.
//
//      DEJA_TRACE      - Log entry is white.
//      DEJA_WARNING    - Log entry is tinted yellow.
//      DEJA_ERROR      - Log entry is tinted red.
//      DEJA_BOOKMARK   - Log entry is automatically bookmarked (and thus, 
//                        tinted blue).
//
//------------------------------------------------------------------------------

void ErrorsWarningsBookmarks( void )
{
    DEJA_TRACE   ( "Severity::Trace",    "Normal log entry" );
    DEJA_WARNING ( "Severity::Warning",  "Warning log entry" );
    DEJA_ERROR   ( "Severity::Error",    "Error log entry" );
    DEJA_BOOKMARK( "Severity::Bookmark", "Programatically bookmarked entry" );

    DEJA_TRACE   ( "net_mgr::ProcessPacket", "Packet received" );
    DEJA_WARNING ( "net_mgr::ProcessPacket", "Out of order packet received" );
    DEJA_ERROR   ( "net_mgr::ProcessPacket", "Packet check sum failure" );
    DEJA_BOOKMARK( "net_mgr::ProcessPacket", "Connection confirmation" );
}

//------------------------------------------------------------------------------
//
//  And a simple main() to call each of the functions above...
//
//------------------------------------------------------------------------------

void main( void )
{
    BasicTrace();
    FormatSpecifications();
    WideCharSupport();
    HierarchicalChannels();
    ErrorsWarningsBookmarks();
}

//------------------------------------------------------------------------------
