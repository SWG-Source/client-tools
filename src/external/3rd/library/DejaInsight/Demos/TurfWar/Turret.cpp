//==============================================================================
//
//  Turret.cpp
//
//==============================================================================
//  Copyright (C) DejaTools, LLC.  All rights reserved.
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "stdafx.h"
#include "Turret.h"
#include "Missile.h"
#include "Mortar.h"
#include "Magnet.h"
#include "Boid.h"
#include "Arena.h"
#include "../TinyEngine/graphics.h"
#include "../../DejaLib.h"

//==============================================================================
//  DEFINES
//==============================================================================

#define BASE_RADIUS             6.0f        // Radius of the missile base triangle
#define MIN_INITIAL_DELAY       1.0f        // Min time before initial firing
#define MAX_INITIAL_DELAY       3.0f        // Max time before initial firing
#define MIN_MISSILE_FIRE_DELAY  3.0f        // Min time between firing
#define MAX_MISSILE_FIRE_DELAY  6.0f        // Max time between firing
#define MIN_MORTAR_FIRE_DELAY   5.0f        // Min time between firing
#define MAX_MORTAR_FIRE_DELAY   8.0f        // Max time between firing
#define MORTAR_CHANCE           0.2f        // Chance of firing a mortar 

//==============================================================================
//  STORAGE
//==============================================================================

static vertex_pn s_TurretVerts[ 10 * 3 ];      // 10 faces, 3 verts each.

//==============================================================================
//  OBJECT SYSTEM HOOKS
//==============================================================================

object*     TurretFactoryFn( void ) { return( new turret ); }
type_data   TurretTypeData( "turret", TurretFactoryFn );

//==============================================================================
//  FUNCTIONS
//==============================================================================

void SystemPrep_Turret( void )
{
    vector3 Vertices[7];

    Vertices[0] = vector3( 0.0f, 3.0f, 0.0f );

    Vertices[1] = vector3( 0.0f, 1.0f, 1.0f );
    Vertices[2] = vector3( 0.0f, 0.0f, 1.0f );
    Vertices[3] = vector3( 0.0f, 1.0f, 1.0f );
    Vertices[4] = vector3( 0.0f, 0.0f, 1.0f );
    Vertices[5] = vector3( 0.0f, 1.0f, 1.0f );
    Vertices[6] = vector3( 0.0f, 0.0f, 1.0f );

    Vertices[1].RotateY( R_0 );
    Vertices[2].RotateY( R_60 );
    Vertices[3].RotateY( R_120 );
    Vertices[4].RotateY( R_180 );
    Vertices[5].RotateY( R_240 );
    Vertices[6].RotateY( R_300 );

    int Indices[10*3] =
    {
        0, 1, 3,    0, 3, 5,    0, 5, 1,
        1, 2, 3,    3, 4, 5,    5, 6, 1,
        2, 1, 6,    4, 3, 2,    6, 5, 4,
        2, 6, 4
    };

    // For each triangle...
    for( int i = 0; i < 10; i++ )
    {
        // Copy verts for the triangle.
        s_TurretVerts[ (i*3)+0 ].Position = Vertices[ Indices[(i*3)+0] ];
        s_TurretVerts[ (i*3)+1 ].Position = Vertices[ Indices[(i*3)+1] ];
        s_TurretVerts[ (i*3)+2 ].Position = Vertices[ Indices[(i*3)+2] ];

        // Generate a normal for each triangle, and associate with each vert.
        vector3 V1 = (s_TurretVerts[(i*3)+1].Position - s_TurretVerts[(i*3)+0].Position);
        vector3 V2 = (s_TurretVerts[(i*3)+2].Position - s_TurretVerts[(i*3)+0].Position);
        vector3 N  = Cross( V1, V2 );
        N.Normalize();
        s_TurretVerts[ (i*3)+0 ].Normal = N;
        s_TurretVerts[ (i*3)+1 ].Normal = N;
        s_TurretVerts[ (i*3)+2 ].Normal = N;
    }

    // Register the turret class with the object manager.
    ObjMgr.RegisterType( TurretTypeData );

    // Register the turret type enumeration with the DejaLib.
    DEJA_ENUM_ALIAS( turret_type, turret::TYPE_MISSILE, "MISSILE" );
    DEJA_ENUM_ALIAS( turret_type, turret::TYPE_MORTAR , "MORTAR"  );
}

//==============================================================================

void DejaDescriptor( const turret& N )
{
    DEJA_TYPE ( N, turret );
    DEJA_BASE ( N, object );
    DEJA_ENUM ( N.m_Type, turret_type );
    DEJA_FIELD( N.m_Yaw );
    DEJA_FIELD( N.m_MinRange );
    DEJA_FIELD( N.m_MaxRange );
    DEJA_FIELD( N.m_Timer );
    DEJA_FIELD( N.m_ShotsFired );
    DEJA_FIELD( N.m_Kills );
}

//==============================================================================
//  FUNCTIONS - turret
//==============================================================================

turret::turret( void )
{
}

//==============================================================================

turret::~turret( void )
{
}

//==============================================================================

void turret::Spawn( const vector3& Position, int Type, float MinRange, float MaxRange )
{
    DEJA_TRACE( "turret::Spawn",
                "Position:%g,%g,%g - Type:%(turret_type) - Range:%g-%g",
                Position.x, Position.y, Position.z, Type, MinRange, MaxRange );

    SetPosition( Position );
    m_Type       = Type;
    m_Yaw        = frand( R_0, R_360 );
    m_MinRange   = MinRange;
    m_MaxRange   = MaxRange;
    m_Timer      = frand( MIN_INITIAL_DELAY, MAX_INITIAL_DELAY );
    m_ShotsFired = 0;
    m_Kills      = 0;
}

//==============================================================================

void turret::OnDejaObjInit( void )
{
    DEJA_OBJECT_INIT( *this );
}

//==============================================================================

void turret::OnLogic( float Time )
{
    DEJA_CONTEXT( "turret::OnLogic" );
    DEJA_SCOPE_OBJECT( *this );

    // Rotate.
    m_Yaw += Time / 2.0f;
    if( m_Yaw > R_360 )
        m_Yaw -= R_360;

    // Update position to track the arena cell under the turret.
    ObjMgr.ResetTypeLoop( ArenaTypeData.iType );
    arena* pArena = (arena*)ObjMgr.GetNextInType( ArenaTypeData.iType );
    if( pArena )
    {
        vector3 Pos = GetPosition();
        cell* pCell = pArena->GetCell( Pos );
        if( pCell )
        {
            Pos.y = pCell->m_Height;
            SetPosition( Pos );
        }
    }

    // Run state logic.
    switch( m_Type )
    {
    case TYPE_MISSILE:
        OnLogicMissile( Time );
        break;

    case TYPE_MORTAR:
        OnLogicMortar( Time );
        break;
    }
}

//==============================================================================

void turret::OnLogicMissile( float Time )
{
    DEJA_CONTEXT( "turret::OnLogicMissile" );

    // Update timer and check if it's time to fire yet.
    m_Timer -= Time;
    if( m_Timer <= 0.0f )
    {
        // Get turret position.
        vector3 Pos = GetPosition();

        // Find a boid to shoot at within the range limits.
        vector3 TargetPos( 0, 0, 0 );
        int TargetID = -1;
        ObjMgr.ResetTypeLoop( BoidTypeData.iType );
        boid* pBoid;
        while( (pBoid = (boid*)ObjMgr.GetNextInType( BoidTypeData.iType )) != NULL )
        {
            float Distance = (pBoid->GetPosition() - Pos).GetLength();
            if( (Distance >= m_MinRange) && (Distance <= m_MaxRange) )
            {
                TargetPos   = pBoid->GetPosition();
                TargetID    = pBoid->GetID();
                break;
            }
        }

        // Got a target?
        if( TargetID != -1 )
        {
            DEJA_TRACE( "turret::OnLogicMissile", 
                        "FIRE MISSILE - Target:%$", 
                        ObjMgr.GetObjByID( TargetID ) );

            // Launch missile.
            missile* pMissile = (missile*)ObjMgr.CreateObject( "missile" );
            pMissile->Spawn( GetPosition()+vector3(0,9,0), GetID(), vector3(0,1,0), TargetID );
            ObjMgr.AddObject( pMissile );
            m_Timer += frand( MIN_MISSILE_FIRE_DELAY, MAX_MISSILE_FIRE_DELAY );

            // Switch type.
            m_Type = (frand(0,1) < MORTAR_CHANCE) ? TYPE_MORTAR : TYPE_MISSILE;

            // Update states.
            m_ShotsFired++;
        }
    }
}

//==============================================================================

void turret::OnLogicMortar( float Time )
{
    DEJA_CONTEXT( "turret::OnLogicMortar" );

    // Update timer and check if it's time to fire yet.
    m_Timer -= Time;
    if( m_Timer <= 0.0f )
    {
        // Get turret position.
        vector3 Pos = GetPosition();

        // Find a magnet to shoot at within the range limits.
        vector3 TargetPos( 0, 0, 0 );
        magnet* pTarget = NULL;
        ObjMgr.ResetTypeLoop( MagnetTypeData.iType );
        magnet* pMagnet;
        while( (pMagnet = (magnet*)ObjMgr.GetNextInType( MagnetTypeData.iType )) != NULL )
        {
            float Distance = (pMagnet->GetPosition() - Pos).GetLength();
            if( (Distance >= m_MinRange) && (Distance <= m_MaxRange) )
            {
                pTarget     = pMagnet;
                TargetPos   = pMagnet->GetPosition();
                break;
            }
        }

        // Got a target?
        if( pTarget )
        {
            // Pick launch angle.
            float Pitch = frand( R_30, R_60 );

            // Get distance to target.
            vector3 Delta = TargetPos - GetPosition();
            Delta.y = 0;
            float Distance = Delta.GetLength();

            // Compute the launch speed required to reach the target.
            float Speed = sqrt( (Distance * GRAVITY) / sin(2*Pitch) );

            // Compute how long the mortar will take to get to the target.
            float t = Distance / (cos(Pitch)*Speed);

            // Tweak aim & speed to account for movement of target over that time.
            vector3 PredictedDelta = pTarget->GetPredictedPos( t ) - GetPosition();
            PredictedDelta = PredictedDelta * 0.5f;
            PredictedDelta.y = 0;
            float Ratio = (PredictedDelta.GetLength() / Delta.GetLength());
            Ratio = 1.0f + (Ratio - 1.0f) * cos(Pitch);

            // Calculate initial velocity.
            float Yaw = atan2( PredictedDelta.x, PredictedDelta.z );
            vector3 Velocity(0,0,Speed * Ratio);
            Velocity.RotateX( -Pitch );
            Velocity.RotateY( Yaw );

            // Create the mortar.
            mortar* pMortar = (mortar*)ObjMgr.CreateObject( "mortar" );
            pMortar->Spawn( GetPosition()+vector3(0,9,0), GetID(), Velocity );
            ObjMgr.AddObject( pMortar );
            m_Timer += frand( MIN_MORTAR_FIRE_DELAY, MAX_MORTAR_FIRE_DELAY );

            // Switch type.
            m_Type = (frand(0,1) < MORTAR_CHANCE) ? TYPE_MORTAR : TYPE_MISSILE;

            // Update stats.
            m_ShotsFired++;

            DEJA_TRACE( "turret::OnLogicMortar", 
                        "FIRE MORTAR - Target:%$ - Projectile:%$", 
                        pMagnet, pMortar );
        }
    }
}

//==============================================================================

void turret::OnKillCredit( void )
{
    m_Kills++;
}

//==============================================================================

void turret::OnRender( void )
{
    DEJA_CONTEXT( "turret::OnRender" );

    // Generate the matrices to render the turret.
    matrix4 M1, M2;
    M1.Identity();                      M2.Identity();               
    M1.Scale( 6.0f );                   M2.Scale( 6.0f );
    M1.RotateY( m_Yaw );                M2.RotateY( -m_Yaw );
    M1.Translate( GetPosition() );      M2.Translate( GetPosition() );

    // Draw the triangles.
    Graphics->SetL2W( M1 );
    Graphics->DrawTriangles( sizeof(s_TurretVerts)/sizeof(s_TurretVerts[0]), 
                             s_TurretVerts, 
                             color(224,128,128) );

    Graphics->SetL2W( M2 );
    Graphics->DrawTriangles( sizeof(s_TurretVerts)/sizeof(s_TurretVerts[0]), 
                             s_TurretVerts, 
                             color(224,128,128) );
}

//==============================================================================

void turret::OnDejaPost( void )
{
    DEJA_POST( *this );
}

//==============================================================================
