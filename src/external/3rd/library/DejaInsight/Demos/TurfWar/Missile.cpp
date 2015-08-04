//==============================================================================
//
//  Missile.cpp
//
//==============================================================================
//  Copyright (C) DejaTools, LLC.  All rights reserved.
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "stdafx.h"
#include "Missile.h"
#include "Explosion.h"
#include "Boid.h"
#include "Arena.h"
#include "../TinyEngine/graphics.h"
#include "../../DejaLib.h"

//==============================================================================
//  DEFINES
//==============================================================================

#define BASE_RADIUS             0.7f        // Radius of the missile base triangle.
#define TURN_LIMIT              4.0f        // Turn limit (radians per second).
#define MIN_SPEED              45.0f        // Min launch speed.
#define MAX_SPEED              75.0f        // Max launch speed.
#define MIN_LAUNCH_TIME         0.2f        // Min time in launch state before tracking.
#define MAX_LAUNCH_TIME         1.0f        // Max time in launch state before tracking.
#define MIN_FUEL               10.0f        // Min fuel timer.
#define MAX_FUEL               12.0f        // Max fuel timer.
#define MIN_MISSED              0.3f        // Min time in missed state after missing target.
#define MAX_MISSED              0.5f        // Max time in missed state after missing target.
#define DETONATION_RADIUS       5.0f        // Proximal detonation radius.
#define DAMAGE                100           // Amount of damage a missile does.
#define INNER_DAMAGE_RADIUS     5.0f        // The inner damage radius.
#define OUTER_DAMAGE_RADIUS    10.0f        // The outer damage radius.

//==============================================================================
//  STORAGE
//==============================================================================
//                       0 +Z
//                       *
//                      /|\
//                     / | \
//                    /  |  \
//                   /   |   \
//                  /    |    \ 
//                 /     *1 +Y \
//                /  ___/ \___  \
//               /__/         \__\
//              *-----------------*
//          +X 2                   3 -X
//==============================================================================

static vertex_pn s_MissileVerts[ 4 * 3 ];      // 4 faces, 3 verts each.

//==============================================================================
//  OBJECT SYSTEM HOOKS
//==============================================================================

object*     MissileFactoryFn( void ) { return( new missile ); }
type_data   MissileTypeData( "missile", MissileFactoryFn );

//==============================================================================
//  FUNCTIONS
//==============================================================================

void SystemPrep_Missile( void )
{
    // Setup some constants.
    float r      = BASE_RADIUS;
    float l      = BASE_RADIUS * 4.0f;
    float Sin120 =  0.866025404f;
    float Cos120 = -0.5f;

    // Define the 4 vertices that make up the missile.
    vector3 Vertices[4] =
    {
        vector3(      0.0f,     0.0f,    l ),
        vector3(      0.0f,        r, 0.0f ),
        vector3( -Sin120*r, Cos120*r, 0.0f ),
        vector3(  Sin120*r, Cos120*r, 0.0f )
    };

    // Define the 4 faces.
    int Indices[4*3] =   
    {
        0,1,2, 0,2,3, 0,3,1, 1,3,2
    };

    // For each triangle...
    for( int i=0; i<4; i++ )
    {
        // Copy verts for the triangle.
        s_MissileVerts[ (i*3)+0 ].Position = Vertices[ Indices[(i*3)+0] ];
        s_MissileVerts[ (i*3)+1 ].Position = Vertices[ Indices[(i*3)+1] ];
        s_MissileVerts[ (i*3)+2 ].Position = Vertices[ Indices[(i*3)+2] ];

        // Generate a normal for each triangle, and associate with each vert.
        vector3 V1 = (s_MissileVerts[(i*3)+1].Position - s_MissileVerts[(i*3)+0].Position);
        vector3 V2 = (s_MissileVerts[(i*3)+2].Position - s_MissileVerts[(i*3)+0].Position);
        vector3 N  = Cross( V1, V2 );
        N.Normalize();
        s_MissileVerts[ (i*3)+0 ].Normal = N;
        s_MissileVerts[ (i*3)+1 ].Normal = N;
        s_MissileVerts[ (i*3)+2 ].Normal = N;
    }

    // Register the missile class with the object manager.
    ObjMgr.RegisterType( MissileTypeData );

    // Register enumeration with the DejaLib.
    DEJA_ENUM_ALIAS( missile::state, missile::PROJECTILE_STATE_NULL    , "NULL"        );
    DEJA_ENUM_ALIAS( missile::state, missile::PROJECTILE_STATE_FINALIZE, "FINALIZE"    );
    DEJA_ENUM_ALIAS( missile::state, missile::MISSILE_STATE_LAUNCH     , "LAUNCH"      );
    DEJA_ENUM_ALIAS( missile::state, missile::MISSILE_STATE_TRACK      , "TRACK"       );
    DEJA_ENUM_ALIAS( missile::state, missile::MISSILE_STATE_MISSED     , "MISSED"      );
    DEJA_ENUM_ALIAS( missile::state, missile::MISSILE_STATE_OUT_OF_FUEL, "OUT_OF_FUEL" );
    DEJA_ENUM_ALIAS( missile::state, missile::MISSILE_STATE_EXPLODE    , "EXPLODE"     );
}

//==============================================================================

void DejaDescriptor( const missile& N )
{
    DEJA_TYPE ( N, missile );
    DEJA_BASE ( N, projectile );
    DEJA_ENUM ( N.m_State, missile::state );
    DEJA_FIELD( N.m_Timer         );
    DEJA_FIELD( N.m_Fuel          );
    DEJA_FIELD( N.m_Speed         );
    DEJA_FIELD( N.m_Heading       );
    DEJA_FIELD( N.m_TargetID      );
    DEJA_FIELD( N.m_TargetPos     );
    DEJA_FIELD( N.m_TargetPredPos );
    DEJA_FIELD( N.m_Locked        );
}

//==============================================================================
//  FUNCTIONS - missile
//==============================================================================

missile::missile( void )
{
}

//==============================================================================

missile::~missile( void )
{
}

//==============================================================================

void missile::OnDejaObjInit( void )
{
    DEJA_OBJECT_INIT( *this );
}

//==============================================================================

void missile::OnDejaPost( void )
{
    DEJA_POST( *this );
}

//==============================================================================

void missile::Spawn( const vector3& Position, 
                           int      OwnerID, 
                     const vector3& Heading, 
                           int      TargetID )
{
    SetPosition( Position );
    m_OwnerID    = OwnerID;
    m_State      = MISSILE_STATE_LAUNCH;
    m_Fuel       = frand( MIN_FUEL, MAX_FUEL );
    m_Timer      = frand( MIN_LAUNCH_TIME, MAX_LAUNCH_TIME );
    m_Speed      = frand( MIN_SPEED, MAX_SPEED );
    m_Heading    = Heading;
    m_TargetID   = TargetID;
    m_Locked     = false;
    m_TrailColor = color( 255,128,128 );

    DEJA_TRACE( "missile::Spawn", 
                "State:%(missile_state) - Target:[%d]",
                m_State, m_TargetID );
}

//==============================================================================

void missile::UpdateTargetPos( void )
{
    DEJA_CONTEXT( "missile::UpdateTargetPos" );

    // Clear target position & predicted position to origin in case of no target.
    m_TargetPos    .Set( 0.0f, 0.0f, 0.0f );
    m_TargetPredPos.Set( 0.0f, 0.0f, 0.0f );

    // Get the target if it still exists.
    boid* pTarget = (boid*)ObjMgr.GetObjByID( m_TargetID );
    if( pTarget )
    {
        // Get the target position.
        m_TargetPos = pTarget->GetPosition();

        // Calculate how long to intercept target (approximate).
        float t = (m_TargetPos - GetPosition()).GetLength() / m_Speed;

        // Get the predicted target position at time of intercept.  This won't
        // be accurate because we don't account for the additional time to 
        // reach this new location.
        m_TargetPredPos = pTarget->GetPredictedPos( t );
    }
    else
    {
        // The target no longer exists.  Enter the missed state.
        m_TargetID = -1;
        m_State    = MISSILE_STATE_MISSED;
        m_Timer    = frand( MIN_MISSED, MAX_MISSED );

        DEJA_TRACE( "missile::UpdateTargetPos", 
                    "State:%(missile_state) - Target:[%d]",
                    m_State, m_TargetID );
    }
}

//==============================================================================

void missile::OnLogic( float Time )
{
    DEJA_CONTEXT( "missile::OnLogic" );

    // If this is a base projectile state then call the projectile OnLogic.
    if( m_State < PROJECTILE_STATE_MAX )
    {
        return( projectile::OnLogic( Time ) );
    }

    // Move the missile.
    SetPosition( GetPosition() + m_Heading * (m_Speed * Time) );

    // Update trail.
    AddToTrail( GetPosition() );

    // Update fuel timer and switch to Out of Fuel if necessary.
    if( m_State != MISSILE_STATE_OUT_OF_FUEL )
    {
        m_Fuel -= Time;
        if( m_Fuel <= 0.0f )
        {
            m_State = MISSILE_STATE_OUT_OF_FUEL;
            m_Timer = 4.0f;

            DEJA_TRACE( "missile::OnLogic", 
                        "State:%(missile_state) - Target:[%d]",
                        m_State, m_TargetID );
        }
    }

    // Check for arena collision.
    ObjMgr.ResetTypeLoop( ArenaTypeData.iType );
    arena* pArena = (arena*)ObjMgr.GetNextInType( ArenaTypeData.iType );
    if( pArena && pArena->Intersects( GetPosition(), 1.0f ) )
    {
        // Collided!  Clear heading and speed.  Explode!
        m_Heading.Set( 0.0f, 0.0f, 0.0f );
        m_Speed = 0.0f;
        m_State = MISSILE_STATE_EXPLODE;

        DEJA_TRACE( "missile::OnLogic", 
                    "State:%(missile_state) - Target:[%d]",
                    m_State, m_TargetID );
    }

    // Run state logic.
    switch( m_State )
    {
    case MISSILE_STATE_LAUNCH:
        LogicLaunch( Time );
        break;

    case MISSILE_STATE_TRACK:
        LogicTrack( Time );
        break;

    case MISSILE_STATE_MISSED:
        LogicMissed( Time );
        break;

    case MISSILE_STATE_OUT_OF_FUEL:
        LogicOutOfFuel( Time );
        break;
    }

    // Check for explosion.
    if( m_State == MISSILE_STATE_EXPLODE )
    {
        // Apply damage through object manager.
        ObjMgr.ApplyDamage( GetPosition(), m_OwnerID, DAMAGE, 
                            INNER_DAMAGE_RADIUS, OUTER_DAMAGE_RADIUS );

        // Create explosion.
        explosion* pExplosion = (explosion*)ObjMgr.CreateObject( "explosion" );
        if( pExplosion )
        {
            pExplosion->SpawnMissile( GetPosition(), vector3(0,0,0), 
                                      1.0f, color( 255, 255, 0 ) );
            ObjMgr.AddObject( pExplosion );
        }

        // Enter the finalize state.
        m_State = PROJECTILE_STATE_FINALIZE;
        DEJA_TRACE( "missile::OnLogic", "State:%(missile_state)", m_State );
    }
}

//==============================================================================

void missile::LogicLaunch( float Time )
{
    DEJA_CONTEXT( "missile::LogicLaunch" );

    // Check for transition to Track state after timeout.
    m_Timer -= Time;
    if( m_Timer <= 0.0f )
    {
        m_State = MISSILE_STATE_TRACK;
        UpdateTargetPos();

        DEJA_TRACE( "missile::LogicLaunch", 
                    "State:%(missile_state) - Target:[%d]",
                    m_State, m_TargetID );
    }
}

//==============================================================================

void missile::LogicTrack( float Time )
{
    DEJA_CONTEXT( "missile::LogicTrack" );

    // Get current position.
    vector3 p = GetPosition();

    // Update target position.
    UpdateTargetPos();

    // Get the steering axis by crossing our heading and target vector.
    vector3 To = m_TargetPredPos - p;
    To.Normalize();
    vector3 Normal = Cross( m_Heading, To );

    // Get the steering angle by dotting our heading and target vector.
    float Angle = acos( Dot( To, m_Heading ) );

    // Limit the rate of turn.
    float AngleLimit   = TURN_LIMIT * Time;
    float LimitedAngle = MINMAX( -AngleLimit, Angle, AngleLimit );

    // Rotate the current heading towards the desired heading.
    matrix4 M( Normal, LimitedAngle );
    m_Heading = M * m_Heading;
    m_Heading.Normalize();

    // Check for proximal detonation.
    if( (m_TargetPos - p).GetLength() <= DETONATION_RADIUS )
    {
        m_State = MISSILE_STATE_EXPLODE;

        DEJA_TRACE( "missile::LogicTrack", 
                    "State:%(missile_state) - Target:[%d]",
                    m_State, m_TargetID );
    }
    else
    {
        // Check for good lock on target.
        if( fabs(Angle) < R_80 )
            m_Locked = true;

        // Check for target overshoot after a lock.
        if( m_Locked && (fabs(Angle) > R_90) )
        {
            // Overshot the target, so go into MISSED state.
            m_TargetID = -1;
            m_State    = MISSILE_STATE_MISSED;
            m_Timer    = frand( MIN_MISSED, MAX_MISSED );
            DEJA_TRACE( "missile::LogicTrack", 
                        "State:%(missile_state) - Target:[%d]",
                        m_State, m_TargetID );
        }
    }
}

//==============================================================================

void missile::LogicMissed( float Time )
{
    DEJA_CONTEXT( "missile::LogicMissed" );

    // Wait for state timeout.
    m_Timer -= Time;
    if( m_Timer <= 0.0f )
    {
        // Find a new target.  This should be improved.
        m_TargetID = ObjMgr.GetClosest( this, "boid" );

        // Track this new target.
        m_Locked = false;
        m_State  = MISSILE_STATE_TRACK;
        UpdateTargetPos();
    }
}

//==============================================================================

void missile::LogicOutOfFuel( float Time )
{
    DEJA_CONTEXT( "missile::LogicOutOfFuel" );

    // Apply gravity to heading so missile starts to descend.
    vector3 g( 0, -GRAVITY * Time / 50.0f, 0 );
    vector3 v = m_Heading * (m_Speed * Time) + g;
    v.Normalize();
    m_Heading = v;

    // Explode if this state times out before colliding with anything.
    m_Timer -= Time;
    if( m_Timer <= 0.0f )
    {
        m_State = MISSILE_STATE_EXPLODE;
        DEJA_TRACE( "missile::LogicOutOfFuel", 
                    "State:%(missile_state) - Target:[%d]",
                    m_State, m_TargetID );
    }
}

//==============================================================================

void missile::OnRender( void )
{
    DEJA_CONTEXT( "missile::OnRender" );

    // Don't render if just waiting for the trail to disappear.
    if( m_State != PROJECTILE_STATE_FINALIZE )
    {
        // Generate the matrix to render the missile using Axis and Angle.
        vector3 n = Cross( vector3( 0.0f, 0.0f, 1.0f ), m_Heading );
        n.Normalize();
        float   a = acos( Dot( vector3( 0.0f, 0.0f, 1.0f ), m_Heading ) );
        matrix4 M( n, a );
        M.Translate( GetPosition() );
        Graphics->SetL2W( M );

        // Draw the triangles.
        Graphics->DrawTriangles( sizeof(s_MissileVerts)/sizeof(s_MissileVerts[0]), 
                                 s_MissileVerts, 
                                 color(255,128,128) );
    }

    // Render the trail.
    RenderTrail();
}

//==============================================================================
