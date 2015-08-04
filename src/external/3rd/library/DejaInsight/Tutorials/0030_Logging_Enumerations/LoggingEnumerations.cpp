//==============================================================================
//
//  Logging Enumerations
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
//  Define a couple of enumerated types.
//
//------------------------------------------------------------------------------

enum movement
{
    MOVE_FORWARD,
    MOVE_BACKWARD,
    MOVE_LEFT,
    MOVE_RIGHT,
    MOVE_UP,
    MOVE_DOWN,
};

enum compass
{
    COMPASS_NORTH = (1 << 0),
    COMPASS_SOUTH = (1 << 1),
    COMPASS_EAST  = (1 << 2),
    COMPASS_WEST  = (1 << 3),
};

//------------------------------------------------------------------------------
//
//  This function 'teaches' Insight about the above enumerations.
//
//  DEJA_ENUM_VALUE announces a value within an enumerated type.  When displayed
//  as an enumerated value, the identifier (a "stringized" version the second
//  argument) will be used.
//
//  DEJA_ENUM_ALIAS also announces an enumerated value, but adds an alias.  This
//  is handy when the enumeration value identifier is lengthy and you would 
//  rather display something shorter.
//
//  Once enumerations are presented to Insight, they are available to all 
//  DejaLib functions using formatted output.
//
//------------------------------------------------------------------------------

void AnnounceEnumerations( void )
{
    DEJA_ENUM_VALUE( movement, MOVE_FORWARD  );
    DEJA_ENUM_VALUE( movement, MOVE_BACKWARD );
    DEJA_ENUM_VALUE( movement, MOVE_LEFT     );
    DEJA_ENUM_VALUE( movement, MOVE_RIGHT    );
    DEJA_ENUM_VALUE( movement, MOVE_UP       );
    DEJA_ENUM_VALUE( movement, MOVE_DOWN     );

    DEJA_ENUM_ALIAS( compass, COMPASS_NORTH,  "N" );
    DEJA_ENUM_ALIAS( compass, COMPASS_SOUTH,  "S" );
    DEJA_ENUM_ALIAS( compass, COMPASS_EAST , L"East" );
    DEJA_ENUM_ALIAS( compass, COMPASS_WEST , L"West" );
}

//------------------------------------------------------------------------------
//
//  As an extension of formatted output, you can use %(enum_type_name) to
//  indicate that you want to display the identifier associated with the value
//  rather than the value itself.  If the enumerated type is unknown, or if the
//  value is not defined for the type, then the integral value is displayed.
//
//------------------------------------------------------------------------------

void DemonstrateEnumerations( void )
{
    //
    // Log variables of the enumeration types.
    //

    movement M = MOVE_FORWARD;
    compass  C = COMPASS_NORTH;

    DEJA_TRACE( "movement as int",  "M = %d",          M );
    DEJA_TRACE( "movement as enum", "M = %(movement)", M );

    DEJA_TRACE( "compass as int",   "C = %d",          C );
    DEJA_TRACE( "compass as enum",  "C = %(compass)",  C );

    //
    // Log integers (possibly) holding values from the enumerations.
    //

    int i;

    for( i = 0; i < 10; i++ )
    {
        // I'm using DEJA_WARNING on the first of this trio of messages just
        // to make it easier to pick out the grouping in Insight.

        DEJA_WARNING( "int as int",      "i = %d",          i );
        DEJA_TRACE  ( "int as movement", "i = %(movement)", i );
        DEJA_TRACE  ( "int as compass",  "i = %(compass)",  i );
    }
}

//------------------------------------------------------------------------------
//
//  And a simple main() to call the functions...
//
//------------------------------------------------------------------------------

void main( void )
{
    AnnounceEnumerations();
    DemonstrateEnumerations();
}

//------------------------------------------------------------------------------
