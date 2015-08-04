//==============================================================================
//
//  TurfWar.cpp
//
//==============================================================================
//  Copyright (C) DejaTools, LLC.  All rights reserved.
//==============================================================================

#include "stdafx.h"

#include "../TinyEngine/engine.h"
#include "../TinyEngine/graphics.h"

#include "CapPoint.h"
#include "TurfTeam.h"
#include "Magnet.h"
#include "Boid.h"
#include "Arena.h"
#include "Turret.h"
#include "Projectile.h"
#include "Missile.h"
#include "Mortar.h"
#include "Explosion.h"

#include "../../DejaLib.h"

//==============================================================================
//  DEFINES
//==============================================================================

#define ARENA_SIZE          9           // The size of the arena in hex rings.

//==============================================================================
//  TYPES
//==============================================================================

class turf_war : public app
{
public:
virtual void    Init            ( void );
virtual void    Kill            ( void );
virtual void    Logic           ( float DeltaTime );
virtual void    Render          ( void );

        void    CreateTurret    ( int i, int j, int Type, float MinRange, float MaxRange );
        void    CreateCapPoint  ( int i, int j );

protected:
        int     m_TurfTeamID[3];
};

//==============================================================================
//  STORAGE
//==============================================================================

view s_View;

//==============================================================================
//  FUNCTIONS
//==============================================================================

void turf_war::Init( void )
{
    DEJA_CONTEXT( "turf_war::Init" );

    // Register and initialize the various subsystems.
    SystemPrep_CapPoint();
    SystemPrep_TurfTeam();
    SystemPrep_Magnet();
    SystemPrep_Boid();
    SystemPrep_Arena();
    SystemPrep_Turret();
    SystemPrep_Projectile();
    SystemPrep_Missile();
    SystemPrep_Mortar();
    SystemPrep_Explosion();

    // Create the arena.
    arena* pArena = (arena*)ObjMgr.CreateObject( "arena" );
    if( pArena )
    {
        pArena->Init( ARENA_SIZE );
        ObjMgr.AddObject( pArena );
    }

    // Create the three turf war teams.
    color TeamColor[3] = 
    { 
        color( 255, 255, 191 ),
        color( 255, 191, 255 ),
        color( 191, 255, 255 ),
    };
    for( int i = 0; i < 3; i++ )
    {
        turf_team* pTeam = (turf_team*)ObjMgr.CreateObject( "turf_team" );
        pTeam->SetColor( TeamColor[i] );
        ObjMgr.AddObject( pTeam );
        m_TurfTeamID[i] = pTeam->GetID();
    }

    // Create capture points.
    CreateCapPoint( -(ARENA_SIZE-2),               0 );
    CreateCapPoint(               0,  (ARENA_SIZE-2) );
    CreateCapPoint(  (ARENA_SIZE-2), -(ARENA_SIZE-2) );

    // Create turrets.
    CreateTurret(  ARENA_SIZE,           0, turret::TYPE_MISSILE,  40, 200 );
    CreateTurret(           0, -ARENA_SIZE, turret::TYPE_MISSILE,  40, 200 );
    CreateTurret( -ARENA_SIZE,  ARENA_SIZE, turret::TYPE_MISSILE,  40, 200 );
    CreateTurret(           0,           0, turret::TYPE_MORTAR , 100, 200 );
}

//==============================================================================

void turf_war::CreateTurret( int i, int j, int Type, float MinRange, float MaxRange )
{
    ObjMgr.ResetTypeLoop( ArenaTypeData.iType );
    arena* pArena = (arena*)ObjMgr.GetNextInType( ArenaTypeData.iType );
    if( pArena )
    {
        // Get the cell.
        cell* pCell = pArena->GetCell( i, j );
        if( pCell )
        {
            turret* pTurret = (turret*)ObjMgr.CreateObject( "turret" );
            ASSERT( pTurret );
            pTurret->Spawn( pCell->m_Base[0], Type, MinRange, MaxRange  );
            ObjMgr.AddObject( pTurret );
        }
    }
}

//==============================================================================

void turf_war::CreateCapPoint( int i, int j )
{
    ObjMgr.ResetTypeLoop( ArenaTypeData.iType );
    arena* pArena = (arena*)ObjMgr.GetNextInType( ArenaTypeData.iType );
    if( pArena )
    {
        // Get the cell.
        cell* pCell = pArena->GetCell( i, j );
        if( pCell )
        {
            cap_point* pCapPoint = (cap_point*)ObjMgr.CreateObject( "cap_point" );
            pCapPoint->SetPosition( pCell->m_Base[0] + vector3( 0, 10, 0 ) );
            ObjMgr.AddObject( pCapPoint );
        }
    }
}

//==============================================================================

void turf_war::Kill( void )
{
    for( int i = 0; i < 3; i++ )
    {
        ObjMgr.RemoveDestroy( m_TurfTeamID[i] );
    }

    // Destroy any remaining objects in the object manager.
    ObjMgr.RemoveDestroyAll();
}

//==============================================================================

void turf_war::Logic( float DeltaTime )
{
    DEJA_CONTEXT( "turf_war::Logic" );

    //
    // Input logic.
    //

#ifdef WIN32

    if( Engine->IsKeyPressed( VK_ESCAPE ) )
    {
        Engine->Exit( 0 );
    }

    if( Engine->IsKeyPressed( 'A' ) )   s_View.Translate( vector3( 3, 0, 0), view::VIEW );
    if( Engine->IsKeyPressed( 'D' ) )   s_View.Translate( vector3(-3, 0, 0), view::VIEW );
    if( Engine->IsKeyPressed( 'W' ) )   s_View.Translate( vector3( 0, 0, 3), view::VIEW );
    if( Engine->IsKeyPressed( 'S' ) )   s_View.Translate( vector3( 0, 0,-3), view::VIEW );

#endif // WIN32

    //
    // Object logic.
    //

    ObjMgr.Logic( DeltaTime );

    //
    // Camera logic.
    //

    static float t = 0;
    t += DeltaTime * 0.1f;
    vector3 Target (  0,  -40,   0 );
    vector3 Vantage( 200*cos(t), 250, 200*sin(t) );

    s_View.SetPosition( Vantage );
    s_View.LookTo( Target );
}

//==============================================================================

void turf_war::Render( void )
{
    DEJA_CONTEXT( "turf_war::Render" );

    // Setup the View from the Graphics viewport.
    int l, t, w, h;
    Graphics->GetViewport( l, t, w, h );
    s_View.SetViewport( l, t, w, h );

    // Clear the screen and depth buffer.
    Graphics->Clear( color( 0, 0, 0 ) );

    // Establish the view (camera).
    Graphics->SetView( s_View );

    // Enable lighting.
    Graphics->EnableLighting();

    // Test code to add some color to the background.
    {
        matrix4 L2W;
        L2W.Identity();
        Graphics->SetL2W( L2W );

        vector3 s[5], d[5], v[5];

        int l, t, r, b, w, h;
        s_View.GetViewport( l, t, w, h );
        r = l + w;
        b = t + h;

        s_View.PixelToRay( s[0], d[0], (l+r)/2, (t+b)/2 );
        s_View.PixelToRay( s[1], d[1],       l,       t );
        s_View.PixelToRay( s[2], d[2],       r,       t );
        s_View.PixelToRay( s[3], d[3],       r,       b );
        s_View.PixelToRay( s[4], d[4],       l,       b );

        unsigned long c[5];
        for( int i=0; i<5; i++ )
        {
            int r = (int)(ABS(d[i].x) * 255);
            int g = (int)(ABS(d[i].y) * 255);
            int b = (int)(ABS(d[i].z) * 255);

            c[i] = (r << 16) | (g << 8) | (b << 0);
            v[i] = s[i] + d[i] * 1000.0f;
        }

        vertex_pc Verts[4*3];
        for( int i=0; i<4; i++ )
        {
            int i0 = 0;
            int i1 = i+1;
            int i2 = (i+1)%4 + 1;

            Verts[i*3+0].Position = v[i0]; Verts[i*3+0].Color = c[i0];
            Verts[i*3+2].Position = v[i1]; Verts[i*3+2].Color = c[i1];
            Verts[i*3+1].Position = v[i2]; Verts[i*3+1].Color = c[i2];
        }
        Graphics->DisableZWrite();
        Graphics->DrawTriangles( 4*3, Verts );
        Graphics->EnableZWrite();
    }

    // Render all the objects.
    ObjMgr.Render();

    // Clear the L2W.
    matrix4 M;
    M.Identity();
    Graphics->SetL2W( M );

    // Render diagnostics.
    {
        if( !(DEJA_LIB_CONNECTED()) )
        {
            const char* pError = DEJA_LAST_ERROR();
            char        Buffer[256];
            if( *pError )
            {
                sprintf_s( Buffer, sizeof(Buffer), 
                           "Not Connected - DEJA_LAST_ERROR: \"%s\"",                    
                           pError );
            }
            else
            {
                sprintf_s( Buffer, sizeof(Buffer), "Disconnected" );
            }
            Graphics->DrawText( Buffer, 10, 20, color( 255, 255, 255, 255 ) );
        }
    }

    // End of frame.
    DEJA_NEXT_FRAME();
}

//==============================================================================
//  ENGINE ENTRY POINT
//==============================================================================

turf_war TurfWar;

TINY_ENGINE_MAIN( TurfWar, "Turf War" );

//==============================================================================
