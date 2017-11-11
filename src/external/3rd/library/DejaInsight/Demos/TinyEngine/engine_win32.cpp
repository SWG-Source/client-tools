//==============================================================================
//  engine_win32.cpp
//==============================================================================
//  Copyright (C) DejaTools, LLC.  All rights reserved.
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "stdafx.h"
#include "engine.h"
#include "graphics_gl.h"

#include "../../DejaLib.h"

//==============================================================================
//  LIBS
//==============================================================================

#pragma comment( lib, "../../DejaLib.Win32.lib" )

//==============================================================================
//  DEFINES
//==============================================================================

#define FPS_LIMIT       60
#define WINDOW_CLASS    _T( "TinyEngineClass" )

//==============================================================================
//  STORAGE
//==============================================================================

extern  app&            g_TheApp;
extern  wchar_t*        g_pWindowTitle;

        engine*         Engine   = NULL;                        // The Engine.
        graphics*       Graphics = NULL;                        // The Graphics.

static  HINSTANCE       s_hInstance;                            // Process instance handle.
static  HWND            s_hWnd;                                 // The window.

static  LARGE_INTEGER   s_TimeFreq;                             // Frequency from QueryPerformanceFreq.
static  LARGE_INTEGER   s_TimeBase;                             // Base time from QueryPerformanceCounter.
static  float           s_AppTime = 0.0f;                       // Time for the application.

//==============================================================================
// WndProc
//==============================================================================

LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    switch( message )
    {
    case WM_SIZE:
        Graphics->WindowResized();
        break;

    case WM_ERASEBKGND:
        return( 0 );
        break;

    case WM_PAINT:
        Graphics->BeginFrame();
        g_TheApp.Render();
        Graphics->EndFrame();
        break;

    case WM_DESTROY:
        PostQuitMessage( 0 );
        break;

    default:
        return( DefWindowProc( hWnd, message, wParam, lParam ) );
    }

    return( 0 );
}

//==============================================================================
// MyRegisterClass
//==============================================================================

ATOM MyRegisterClass( HINSTANCE hInstance )
{
    WNDCLASSEX wcex;

    memset( &wcex, 0, sizeof(wcex) );
    wcex.cbSize         = sizeof( WNDCLASSEX );
    wcex.style			= CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc	= WndProc;
    wcex.cbClsExtra		= 0;
    wcex.cbWndExtra		= 0;
    wcex.hInstance		= hInstance;
    wcex.hCursor		= LoadCursor( NULL, IDC_ARROW );
    wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszClassName	= WINDOW_CLASS;

    return( RegisterClassEx( &wcex ) );
}

//==============================================================================
// TinyEngineMain
//==============================================================================

int TinyEngineMain( HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    LPTSTR    lpCmdLine,
                    int       nCmdShow )
{
    DEJA_CONTEXT( "TinyEngineMain" );

    UNREFERENCED_PARAMETER( hPrevInstance );
    UNREFERENCED_PARAMETER( lpCmdLine );

    // Save process handle.
    s_hInstance = hInstance;

    // Register the window class.
    MyRegisterClass( hInstance );

    // Create the window.
    s_hWnd = CreateWindow( WINDOW_CLASS, g_pWindowTitle, 
                           WS_OVERLAPPEDWINDOW,CW_USEDEFAULT, 
                           0, CW_USEDEFAULT, 0,
                           NULL, NULL, hInstance, NULL );

    // Check for failure to create window and exit if it failed.
    if( !s_hWnd )
        return FALSE;

    // Create subsystems.
    Engine = (engine*)new engine;
    graphics_gl* GraphicsGL = new graphics_gl;
    GraphicsGL->Init( s_hWnd );
    Graphics = (graphics*)GraphicsGL;

    // Initialize graphics viewport.
    Graphics->WindowResized();

    // Call application initialization.
    g_TheApp.Init();

    // Call Logic with a 10ms time step to fully initialize.
    g_TheApp.Logic( 0.01f );

    //** By forcing a flush, we trigger the DejaLib to go ahead and connect with
    //** Insight.  You wouldn't normally do this, but, since the connection
    //** process takes a moment, it was causing a framerate hitch.  Applications
    //** which have a non-trivial start up phase (unlike most demos using the
    //** TinyEngine) will likely generate enough DejaLib traffic so as to
    //** automatically connect before the main render loop.
    DEJA_FLUSH();

    // Show the window and cause a first update.
    nCmdShow = SW_SHOWMAXIMIZED;
    ShowWindow  ( s_hWnd, nCmdShow );
    UpdateWindow( s_hWnd );

    // Main loop.
    MSG msg = { 0 };
    float RealStart = Engine->GetRealTime();
    while( msg.message != WM_QUIT )
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

        // Dispatch windows messages.
        while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &msg );
            DispatchMessage ( &msg );
        }

        // Run logic.
        g_TheApp.Logic( DeltaTime );

        // Cause a paint.
        RedrawWindow( s_hWnd, NULL, NULL, RDW_UPDATENOW | RDW_INVALIDATE );
    }

    g_TheApp.Kill();

    // Teardown subsystems.
    Graphics->Kill();
    delete Graphics;
    Graphics = NULL;
    delete Engine;
    Engine = NULL;

    return( (int)msg.wParam );
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
    PostQuitMessage( ExitCode );
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
    if( GetFocus() == s_hWnd )
        return( (GetAsyncKeyState( Key ) & 0x8000) ? true : false );
    else
        return( false );
}

//==============================================================================
