//==============================================================================
//
//  Boid.cpp
//
//==============================================================================
//  Copyright (C) DejaTools, LLC.  All rights reserved.
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "stdafx.h"
#include "Boid.h"
#include "Magnet.h"
#include "Explosion.h"
#include "Arena.h"
#include "../TinyEngine/engine.h"
#include "../TinyEngine/graphics.h"
#include "../../DejaLib.h"

//==============================================================================
//  STORAGE
//==============================================================================
//
//  Top-ish             [0](+Z)
//  View                /|\
//                     / | \
//                    /  |  \
//                   /   |   \
//                  /    |    \ 
//                 /     |(+Y) \
//                /    _[3]_    \
//               /  __/  |  \__  \
//              /__/  __[4]__  \__\
//        (+X)[1]____/  (-Y) \____[2](-X)
//
//==============================================================================

static vector3 s_BoidModelVerts[] =
{
    vector3(  0.0f,  0.0f,  1.0f ),     // Nose
    vector3(  1.0f,  0.0f, -1.0f ),     // Tail, Left
    vector3( -1.0f,  0.0f, -1.0f ),     // Tail, Right
    vector3(  0.0f,  0.5f, -0.7f ),     // Tail, Upper
    vector3(  0.0f, -0.5f, -0.7f ),     // Tail, Lower
};

static int s_BoidIndices[] =
{
    0, 1, 3,
    0, 3, 2,
    0, 2, 4,
    0, 4, 1,
    1, 4, 3,
    3, 4, 2,
};

static vertex_pn s_BoidVerts[ 6 * 3 ];      // 6 faces, 3 verts each

//==============================================================================
//  OBJECT SYSTEM HOOKS
//==============================================================================

object*     BoidFactoryFn( void ) { return( new boid ); }
type_data   BoidTypeData( "boid", BoidFactoryFn );

//==============================================================================
//  FUNCTIONS
//==============================================================================

void SystemPrep_Boid( void )
{
    //
    // Convert the Boid Model Verts and Indices into the form we render.
    //

    // For each triangle...
    for( int i = 0; i < 6; i++ )
    {
        // Set the scale of the boid model
        float Scale = 2.0f;

        // Copy verts for the triangle.
        s_BoidVerts[ (i*3)+0 ].Position = s_BoidModelVerts[ s_BoidIndices[(i*3)+0] ] * Scale;
        s_BoidVerts[ (i*3)+1 ].Position = s_BoidModelVerts[ s_BoidIndices[(i*3)+1] ] * Scale;
        s_BoidVerts[ (i*3)+2 ].Position = s_BoidModelVerts[ s_BoidIndices[(i*3)+2] ] * Scale;

        // Generate a normal for each triangle, and associate with each vert.
        vector3 V1 = (s_BoidVerts[(i*3)+1].Position - s_BoidVerts[(i*3)+0].Position);
        vector3 V2 = (s_BoidVerts[(i*3)+2].Position - s_BoidVerts[(i*3)+0].Position);
        vector3 N  = Cross( V1, V2 );
        N.Normalize();
        s_BoidVerts[ (i*3)+0 ].Normal = N;
        s_BoidVerts[ (i*3)+1 ].Normal = N;
        s_BoidVerts[ (i*3)+2 ].Normal = N;
    }

    // Register the boid class with the object manager.
    ObjMgr.RegisterType( BoidTypeData );
}

//==============================================================================

void DejaDescriptor( const boid& B )
{
    DEJA_TYPE ( B, boid );
    DEJA_BASE ( B, object );
    DEJA_FIELD( B.m_Yaw );
    DEJA_FIELD( B.m_DeltaYaw );
    DEJA_FIELD( B.m_Velocity );
    DEJA_FIELD( B.m_TeamID );
    DEJA_FIELD( B.m_MagnetID );
}

//==============================================================================
//  FUNCTIONS - boid
//==============================================================================

boid::boid( void )
:   m_Velocity  ( 0.0f ),
    m_Yaw       (  R_0 ),
    m_DeltaYaw  (  R_0 ),
    m_Color     ( 0, 0, 0 ),
    m_TeamID    (   -1 ),
    m_MagnetID  (   -1 ),
    m_Health    (  100 )
{
}

//==============================================================================

boid::~boid( void )
{
}

//==============================================================================

void boid::OnDejaObjInit( void )
{
    DEJA_OBJECT_INIT( *this );
}

//==============================================================================

void boid::OnDejaPost( void )
{
    DEJA_POST( *this );
}

//==============================================================================

void boid::Spawn( const vector3& Position, radian Yaw )
{
    SetPosition( Position );
    m_Yaw      = Yaw;
    m_DeltaYaw = R_0;

    m_Offset.x = frand( -20, +20 );
    m_Offset.y = frand( -10, +10 );
    m_Offset.z = frand( -20, +20 );
}

//==============================================================================

void boid::SetVelocity( float Velocity )
{
    m_Velocity = Velocity;
}

//==============================================================================

void boid::SetTeam( int TeamID, color Color )
{
    m_TeamID = TeamID;
    m_Color  = Color;
}

//==============================================================================

void boid::SetMagnet( int MagnetID )
{
    m_MagnetID = MagnetID;
}

//==============================================================================

int boid::GetTeamID( void )
{
    return( m_TeamID );
}

//==============================================================================

int boid::GetMagnetID( void )
{
    return( m_MagnetID );
}

//==============================================================================

vector3 boid::GetPredictedPos( float Time )
{
    vector3 Pos( 0.0f, 0.0f, m_Velocity * Time );
    Pos.RotateY( m_Yaw );
    Pos += GetPosition();
    return Pos;
}

//==============================================================================

void boid::Explode( const vector3& Velocity )
{
    DEJA_TRACE( "boids::Explode", 
                "Position:%g,%g,%g - Velocity:%g,%g,%g",
                GetPosition().x, GetPosition().y, GetPosition().z, 
                Velocity.x, Velocity.y, Velocity.z );

    vector3 v( 0, 0, (m_Velocity * 0.5f) );
    v.RotateY( m_Yaw );
    v += Velocity;
    explosion* pExplosion = (explosion*)ObjMgr.CreateObject( "explosion" );
    if( pExplosion )
    {
        pExplosion->SpawnBoid( GetPosition(), v, 3.0f, color(255,255,0) );
        ObjMgr.AddObject( pExplosion );
    }

    // DO NOT ACCESS this AFTER THIS LINE.
    ObjMgr.RemoveDestroy( this );
}

//==============================================================================

void boid::OnRemove( void )
{
    magnet* pMagnet = (magnet*)ObjMgr.GetObjByID( m_MagnetID );
    if( pMagnet )
        pMagnet->DecBoidCount();
}

//==============================================================================

void boid::OnLogic( float Time )
{
    DEJA_CONTEXT( "boid::OnLogic" );

    //
    // Turning logic.  Try to avoid collisions with the arena, otherwise follow
    // assigned magnet.
    //

    bool Follow = true;

    // Get the arena.
    ObjMgr.ResetTypeLoop( ArenaTypeData.iType );
    arena* pArena = (arena*)ObjMgr.GetNextInType( ArenaTypeData.iType );
    
    if( pArena )
    {
        // Create collision test points ahead of the boid.

        float   Z = m_Velocity * 0.5f;
        vector3 TestC( 0, -10, Z );           // Center.
        vector3 TestL( 0, -10, Z );           // Left.
        vector3 TestR( 0, -10, Z );           // Right.

        TestC.RotateY( m_Yaw        );
        TestL.RotateY( m_Yaw + R_25 );
        TestR.RotateY( m_Yaw - R_25 );

        TestC += GetPosition();
        TestL += GetPosition();
        TestR += GetPosition();

        // Check each point against the arena.

        cell* pCell;
        bool  HitC;
        bool  HitL;
        bool  HitR;

        pCell = pArena->GetCell( TestC );
        HitC  = ( pCell && (pCell->m_Height > TestC.y ) );

        pCell = pArena->GetCell( TestL );
        HitL  = ( pCell && (pCell->m_Height > TestL.y ) );

        pCell = pArena->GetCell( TestR );
        HitR  = ( pCell && (pCell->m_Height > TestR.y ) );

        // Got any hits?
        if( HitC || HitL || HitR )
        {
            // We must avoid collision.  Do NOT follow the magnet.
            Follow = false;

            // How fast to turn?
            radian Swerve = R_360 * 2.0f * Time;

            // Which way to swerve?
            if( HitL && !HitR )
                m_DeltaYaw -= Swerve;
            else
            if( HitR && !HitL )
                m_DeltaYaw += Swerve;
            else
            {
                // If we are here, then we don't have a good idea which way to
                // swerve since we hit (a) both L+R, (b) Center, or (c) all 
                // three.  In this case, keep turning in the direction we are
                // already turning.
                if( m_DeltaYaw > R_0 )  m_DeltaYaw += Swerve; 
                else                    m_DeltaYaw -= Swerve;
            }
        }

        // Constrain the turn rate.
        m_DeltaYaw = MINMAX( -R_180, m_DeltaYaw, +R_180 );
    }

    // If no collision threat, then target the magnet's position.
    if( Follow )
    {    
        // Get the magnet we are following.
        magnet* pMagnet = (magnet*)ObjMgr.GetObjByID( m_MagnetID );
        vector3 Target  = pMagnet->GetPosition() + m_Offset;

        // Create a unit vector which points in the direction of the boid's 
        // left.
        vector3 Left( 1, 0, 0 );
        Left.RotateY( m_Yaw );

        // Create a unit vector which points in the direction of the target  
        // from the boid.
        vector3 ToTarget = Target - GetPosition();
        ToTarget.Normalize();

        // The sign of the dot product between these vectors tells if we want to
        // turn left or right.  We want to constrain the turn rate, too.  But we
        // don't want to simply clip the value (like above) since that could 
        // cause a pop as we transition from the avoidance functionality above 
        // to normal "follow the magnet" behavior.
        float DP = Dot( ToTarget, Left );
        if( DP > 0 )
        {               
            if( m_DeltaYaw < +R_90 )
                m_DeltaYaw += R_360 * Time;
        }
        else
        {
            if( m_DeltaYaw > -R_90 )
                m_DeltaYaw -= R_360 * Time;
        }
    }

    // And turn.
    m_Yaw += m_DeltaYaw * Time;

    //
    // Move forward.
    //

    vector3 Forward( 0, 0, m_Velocity * Time );
    Forward.RotateY( m_Yaw );
    SetPosition( GetPosition() + Forward );

    //
    // Check for arena collision.  (This is for destruction, not avoidance.)
    //

    if( pArena )
    {
        if( pArena->Intersects( GetPosition(), 1.0f ) )
        {
            DEJA_TRACE( "boid::OnLogic(fatal)", "EXPLODE - Arena impact" );

            m_Velocity = 0.0f;
            Explode( vector3(0,0,0) );
        }
    }
}

//==============================================================================

void boid::OnRender( void )
{
    DEJA_CONTEXT( "boid::OnRender" );

    matrix4 M;

    M.Identity();
    M.RotateZ( m_DeltaYaw * -0.50f );   // Add some roll based on turn rate.
    M.RotateY( m_Yaw );
    M.Translate( GetPosition() );
    Graphics->SetL2W( M );
    Graphics->DrawTriangles( sizeof(s_BoidVerts)/sizeof(s_BoidVerts[0]), 
                             s_BoidVerts, 
                             m_Color );
}

//==============================================================================

bool boid::OnDamage( const vector3& Position, int Damage )
{
    DEJA_CONTEXT( "boid::OnDamage" );
    DEJA_TRACE( "boid::OnDamage", 
                "Source:%g,%g,%g - Damage:%d",
                Position.x, Position.y, Position.z, Damage );

    bool Kill = false;

    // Apply damage to health.
    m_Health -= Damage;
    if( m_Health <= 0 )
    {
        DEJA_TRACE( "boid::OnDamage(fatal)", "EXPLODE - Damage" );

        // Create explosion and apply force to move away from damage point.
        vector3 v = GetPosition() - Position;
        v.Normalize();
        v = v * (10.0f * Damage / 100.0f);

        // DO NOT ACCESS this AFTER THIS LINE.
        Explode( v );

        // Return killed.
        Kill = true;
    }

    // Return true if the boid was killed.
    return( Kill );
}

//==============================================================================
