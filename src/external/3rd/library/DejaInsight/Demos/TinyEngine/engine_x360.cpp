//==============================================================================
// engine_x360.cpp
//==============================================================================
//  Copyright (C) DejaTools, LLC.  All rights reserved.
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "stdafx.h"
#include "engine.h"
#include "graphics_x360.h"

#include "../../DejaLib.h"

//==============================================================================
//  LIBS
//==============================================================================

#pragma comment( lib, "../../DejaLib.X360.lib" )

//==============================================================================
//  DEFINES
//==============================================================================

#define FPS_LIMIT       60

//==============================================================================
//  STORAGE
//==============================================================================

extern  app&            g_TheApp;

        engine*         Engine   = NULL;                        // The Engine.
        graphics*       Graphics = NULL;                        // The Graphics.

static  LARGE_INTEGER   s_TimeFreq;                             // Frequency from QueryPerformanceFreq.
static  LARGE_INTEGER   s_TimeBase;                             // Base time from QueryPerformanceCounter.
static  float           s_AppTime = 0.0f;                       // Time for the application.

//==============================================================================
// TinyEngineMain
//==============================================================================

int TinyEngineMain( void )
{
    // Create subsystems.
    Engine = (engine*)new engine;
    graphics_x360* GraphicsX360 = new graphics_x360;
    GraphicsX360->Init();
    Graphics = (graphics*)GraphicsX360;

    // Call application initialization.
    g_TheApp.Init();

    // Main loop.
    float RealStart = Engine->GetRealTime();
    for( ;; )
    {
        // Determine the delta time using "real" time.
        float NewRealStart = Engine->GetRealTime();
        float DeltaTime    = NewRealStart - RealStart;
        RealStart = NewRealStart;

        // If the delta time is too large, then we want to constrain it.
        // This prevents huge delta time values when the app is hung up on
        // a break point or is being dragged.
        if( DeltaTime > (5.0f/FPS_LIMIT) )
        {
            DeltaTime = (5.0f/FPS_LIMIT);
        }

        // Advance the application time.
        s_AppTime += DeltaTime;

        // Run logic.
        g_TheApp.Logic( DeltaTime );

        // Render.
        Graphics->BeginFrame();
        g_TheApp.Render();
        Graphics->EndFrame();
    }

    g_TheApp.Kill();

    // Teardown subsystems.
    Graphics->Kill();
    delete Graphics;
    Graphics = NULL;
    delete Engine;
    Engine = NULL;
}

//==============================================================================
// engine
//==============================================================================

engine::engine( void )
{
    // Initialize time.
    QueryPerformanceFrequency( &s_TimeFreq );
    QueryPerformanceCounter  ( &s_TimeBase );
}

//==============================================================================
// ~engine
//==============================================================================

engine::~engine( void )
{
}

//==============================================================================
// Exit
//==============================================================================

void engine::Exit( int ExitCode )
{
}

//==============================================================================
// GetAppTime
//==============================================================================

float engine::GetAppTime( void )
{
    return( s_AppTime );
}

//==============================================================================
// GetRealTime
//==============================================================================

float engine::GetRealTime( void )
{
    // Get time.
    LARGE_INTEGER Time;
    QueryPerformanceCounter( &Time );
    float Seconds = (float)((double)(Time.QuadPart - s_TimeBase.QuadPart)
                          / (double)(s_TimeFreq.QuadPart));

    return( Seconds );
}

//==============================================================================
// IsKeyPressed
//==============================================================================

bool engine::IsKeyPressed( int Key )
{
    return( false );
}

//==============================================================================
