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
#include <Windows.h>    // For function Sleep().
#endif
#ifdef XBOX
#pragma comment( lib, "../../DejaLib.X360.lib" )
#include <xtl.h>        // For function Sleep().
#endif

#include <stdlib.h>     // For function rand().

//------------------------------------------------------------------------------
//
//  Adding context support is very easy.  Just use DEJA_CONTEXT:
//
//      DEJA_CONTEXT( Label )
//
//  Where:
//
//    - Label is a string literal.  (It can not be a const char*.)
//
//------------------------------------------------------------------------------
//
//  Simply place a DEJA_CONTEXT at the top of any function (or scope block) for 
//  which  you wish to capture context information.  Insight will record when 
//  execution encounters the DEJA_CONTEXT statement, and when execution leaves 
//  the corresponding scope.  Context information is captured independently per 
//  thread.
//
//  In general, when the context scope is a function, we recommend that you use
//  the function name (qualified by class if applicable) as the context label.
//
//  Note that since the current context is shown in the Trace Log, DEJA_CONTEXT 
//  should precede DEJA_TRACE when they are proximate.
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//  A little function just to kill time...
//------------------------------------------------------------------------------

void KillTime( int N )
{
    // Simulate execution time with sleep.
    Sleep( rand() % N );
}

//------------------------------------------------------------------------------
//  A global utility function...
//------------------------------------------------------------------------------

void GlobalFunction( void )
{
    DEJA_CONTEXT( "GlobalFunction" );
    DEJA_TRACE( "Enter", "GlobalFunction" );

    KillTime( 500 );

    DEJA_TRACE( "Exit", "GlobalFunction" );
}

//------------------------------------------------------------------------------
//  A global object with a constructor that executes before main()...
//------------------------------------------------------------------------------

class global
{
public:
    global( void );    
};

global::global( void )
{
    DEJA_CONTEXT( "global::global" );
    DEJA_TRACE( "Enter", "Global constructor (pre-main)" );
    
    KillTime( 500 );
    GlobalFunction();
    KillTime( 500 );

    DEJA_TRACE( "Exit", "Global constructor (pre-main)" );
}

global G;

//------------------------------------------------------------------------------
//  A class with a member function...
//------------------------------------------------------------------------------

class example
{
public:
    void MemberFunction( void );
};

void example::MemberFunction( void )
{
    DEJA_CONTEXT( "example::MemberFunction" );
    DEJA_TRACE( "Enter", "MemberFunction" );
    
    KillTime( 500 );
    GlobalFunction();
    KillTime( 500 );

    DEJA_TRACE( "Exit", "MemberFunction" );
}

//------------------------------------------------------------------------------
//
//  If you have a large function and wish to gather more detailed timing 
//  information within the function, you can place DEJA_CONTEXT statements 
//  within embedded scopes within the function.
// 
//------------------------------------------------------------------------------
//  A function with contexts within embedded scopes...
//------------------------------------------------------------------------------

void LoadLotsOfStuff( void )
{
    DEJA_CONTEXT( "LoadLotsOfStuff" );
    DEJA_TRACE( "Enter", "LoadLotsOfStuff" );

    // Load bitmaps...
    {
        DEJA_CONTEXT( "Load bitmaps" );
        KillTime( 500 );
        GlobalFunction();
    }

    // Load geometry data...
    {
        DEJA_CONTEXT( "Load geometry data" );
        KillTime( 500 );
        GlobalFunction();
    }

    // Load audio...
    {
        DEJA_CONTEXT( "Load audio" );
        KillTime( 500 );
        GlobalFunction();
    }

    DEJA_TRACE( "Exit", "LoadLotsOfStuff" );
}

//------------------------------------------------------------------------------
//  And a main to invoke the functions above...
//------------------------------------------------------------------------------

void main( void )
{
    DEJA_CONTEXT( "main" );
    DEJA_TRACE( "Enter", "main" );

    KillTime( 300 );

    LoadLotsOfStuff();

    example E;
    E.MemberFunction();

    KillTime( 300 );

    GlobalFunction();

    DEJA_TRACE( "Exit", "main" );
}

//------------------------------------------------------------------------------
