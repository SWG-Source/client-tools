//==============================================================================
//
//  Hello World
//
//==============================================================================
//  Copyright (C) DejaTools, LLC.  All rights reserved.
//==============================================================================

//------------------------------------------------------------------------------
//
//  Execution instructions:
//    (1) Start the DejaLauncher application.
//    (2) Run this program.
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  All source files which are going to invoke Insight features must include the
//  file "DejaLib.h".
//
//  If you anticipate using various Insight features throughout your project, 
//  consider adding the DejaLib.h include to one of your own key header files.
//
//------------------------------------------------------------------------------
//
//  Assuming all files from the Insight distribution are in their original
//  locations, the DejaLib.h file should be found "two directories up" from
//  the location of this source file.
//
//------------------------------------------------------------------------------

#include "..\..\DejaLib.h"

//------------------------------------------------------------------------------
//
//  All applications which utilize Insight instrumentation must link with an
//  appropriate version of the DejaLib library.  The DejaLib library will be 
//  explored in subsequent tutorials.
//
//  There are various ways to link the DejaLib library into a project.  Consult
//  the documentation for more information on this.  For the purposes of this
//  particular tutorial, we'll use the simplest solution.
//
//------------------------------------------------------------------------------

#ifdef WIN32
#pragma comment( lib, "../../DejaLib.Win32.lib" )
#endif

#ifdef XBOX
#pragma comment( lib, "../../DejaLib.X360.lib" )
#endif

//------------------------------------------------------------------------------
//
//  And, finally, some code.
//
//------------------------------------------------------------------------------

void main( void )
{
    DEJA_TRACE( "Tutorial", "Hello, World!" );
}

//------------------------------------------------------------------------------
