//==============================================================================
//  app.cpp
//==============================================================================
//  Copyright (C) DejaTools, LLC.  All rights reserved.
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "stdafx.h"
#include "app.h"
#include "obj_mgr.h"
#include "../../DejaLib.h"

//==============================================================================
//  FUNCTIONS
//==============================================================================

void app::Logic( float DeltaTime )
{
    DEJA_CONTEXT( "app::Logic" );
    ObjMgr.Logic( DeltaTime );
}

//==============================================================================

void app::Render( void )
{
    DEJA_CONTEXT( "app::Render" );
    ObjMgr.Render();
}

//==============================================================================
