//==============================================================================
//
//  Mortar.cpp
//
//==============================================================================
//  Copyright (C) DejaTools, LLC.  All rights reserved.
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "stdafx.h"
#include "Mortar.h"
#include "Explosion.h"
#include "Arena.h"
#include "../TinyEngine/graphics.h"
#include "../../DejaLib.h"

//==============================================================================
//  DEFINES
//==============================================================================

#define DAMAGE                   400        // Damage a mortar causes.
#define INNER_DAMAGE_RADIUS    20.0f        // The inner damage radius.
#define OUTER_DAMAGE_RADIUS    25.0f        // The outer damage radius.

//==============================================================================
//  OBJECT SYSTEM HOOKS
//==============================================================================

object*     MortarFactoryFn( void ) { return( new mortar ); }
type_data   MortarTypeData( "mortar", MortarFactoryFn );

//==============================================================================
//  FUNCTIONS
//==============================================================================

void SystemPrep_Mortar( void )
{
    // Register the mortar class with the object manager.
    ObjMgr.RegisterType( MortarTypeData );

    // Register enumeration with the DejaLib.
    DEJA_ENUM_ALIAS( mortar::state, mortar::PROJECTILE_STATE_NULL    , "NULL"      );
    DEJA_ENUM_ALIAS( mortar::state, mortar::PROJECTILE_STATE_FINALIZE, "FINALIZE"  );
    DEJA_ENUM_ALIAS( mortar::state, mortar::MORTAR_STATE_BALLISTIC   , "BALLISTIC" );
}

//==============================================================================

void DejaDescriptor( const mortar& N )
{
    DEJA_TYPE ( N, mortar );
    DEJA_BASE ( N, projectile );
    DEJA_ENUM ( N.m_State, mortar::state );
    DEJA_FIELD( N.m_Velocity  );
}

//==============================================================================
//  FUNCTIONS - mortar
//==============================================================================

mortar::mortar( void )
{
}

//==============================================================================

mortar::~mortar( void )
{
}

//==============================================================================

void mortar::OnDejaObjInit( void )
{
    DEJA_OBJECT_INIT( *this );
}

//==============================================================================

void mortar::OnDejaPost( void )
{
    DEJA_POST( *this );
}

//==============================================================================

void mortar::Spawn( const vector3& Position, 
                          int      OwnerID, 
                    const vector3& Velocity )
{
    SetPosition( Position );
    m_OwnerID    = OwnerID;
    m_State      = MORTAR_STATE_BALLISTIC;
    m_Velocity   = Velocity;
    m_TrailColor = color(128,255,128);

    DEJA_TRACE( "mortar::Spawn", "State:%(mortar_state)", m_State );
}

//==============================================================================

void mortar::Explode( void )
{
    // Apply damage through object manager.
    ObjMgr.ApplyDamage( GetPosition() + vector3(0,15,0), m_OwnerID, DAMAGE, 
                        INNER_DAMAGE_RADIUS, OUTER_DAMAGE_RADIUS );

    // Create explosion.
    explosion* pExplosion = (explosion*)ObjMgr.CreateObject( "explosion" );
    if( pExplosion )
    {
        pExplosion->SpawnMortar( GetPosition(), vector3(0,0,0), 
                                 5.0f, color( 255, 255, 0, 224 ) );
        ObjMgr.AddObject( pExplosion );
    }

    // Finalize this object.
    m_State = PROJECTILE_STATE_FINALIZE;
    DEJA_TRACE( "mortar::Explode", "State:%(mortar_state)", m_State );
}

//==============================================================================

void mortar::OnLogic( float Time )
{
    DEJA_CONTEXT( "mortar::OnLogic" );

    // Run state logic.
    switch( m_State )
    {
    case MORTAR_STATE_BALLISTIC:
        LogicBallistic( Time );
        break;

    default:
        projectile::OnLogic( Time );
        break;
    }
}

//==============================================================================

void mortar::LogicBallistic( float Time )
{
    bool bExplode = false;

    // Move the mortar.
    SetPosition( GetPosition() + m_Velocity * Time );
    m_Velocity.y -= GRAVITY * Time;

    // Update trail.
    AddToTrail( GetPosition() );

    // Check for arena collision
    ObjMgr.ResetTypeLoop( ArenaTypeData.iType );
    arena* pArena = (arena*)ObjMgr.GetNextInType( ArenaTypeData.iType );
    if( pArena && pArena->Intersects( GetPosition(), 1.0f ) )
    {
        // Collided, explode.
        bExplode = true;
    }

    // Explode?
    if( bExplode )
        Explode();
}

//==============================================================================

void mortar::OnRender( void )
{
    DEJA_CONTEXT( "mortar::OnRender" );

    // Don't render if just waiting for the trail to disappear.
    if( m_State != PROJECTILE_STATE_FINALIZE )
    {
        // Generate the matrix to render the mortar.
        matrix4 M;
        M.Identity();
        M.Translate( GetPosition() );
        Graphics->SetL2W( M );

        // Draw an octahedron for the mortar.
        Graphics->DrawOctahedron( color(128,255,128) );
    }

    // Render the trail.
    RenderTrail();
}

//==============================================================================
