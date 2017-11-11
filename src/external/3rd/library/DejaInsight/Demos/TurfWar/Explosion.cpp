//==============================================================================
//
//  Explosion.cpp
//
//==============================================================================
//  Copyright (C) DejaTools, LLC.  All rights reserved.
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "stdafx.h"
#include "Explosion.h"
#include "../TinyEngine/graphics.h"
#include "../../DejaLib.h"

//==============================================================================
//  DEFINES
//==============================================================================

#define LIFE_TIME_MIN   0.8f
#define LIFE_TIME_MAX   1.5f

//==============================================================================
//  OBJECT SYSTEM HOOKS
//==============================================================================

object*     ExplosionFactoryFn( void ) { return( new explosion ); }
type_data   ExplosionTypeData( "explosion", ExplosionFactoryFn );

//==============================================================================
//  FUNCTIONS
//==============================================================================

void SystemPrep_Explosion( void )
{
    // Register the explosion class with the object manager.
    ObjMgr.RegisterType( ExplosionTypeData );
}

//==============================================================================

void DejaDescriptor( const explosion& N )
{
    DEJA_TYPE ( N, explosion    );
    DEJA_BASE ( N, object       );
    DEJA_FIELD( N.m_Smooth      );
    DEJA_FIELD( N.m_Color       );
    DEJA_FIELD( N.m_Size        );
    DEJA_FIELD( N.m_Growth      );
    DEJA_FIELD( N.m_GrowthDecay );
    DEJA_FIELD( N.m_Timer       );
    DEJA_FIELD( N.m_AlphaTime   );
    DEJA_FIELD( N.m_Velocity    );
    DEJA_FIELD( N.m_Spin        );
    DEJA_FIELD( N.m_Rotation    );
}

//==============================================================================
//  FUNCTIONS - explosion
//==============================================================================

explosion::explosion( void )
{
}

//==============================================================================

explosion::~explosion( void )
{
}

//==============================================================================

void explosion::OnDejaObjInit( void )
{
    DEJA_OBJECT_INIT( *this );
}

//==============================================================================

void explosion::OnDejaPost( void )
{
    DEJA_POST( *this );
}

//==============================================================================

void explosion::SpawnMissile( const vector3& Position, 
                              const vector3& Velocity, 
                                    float    Size, 
                              const color&   Color )
{
    SetPosition( Position );
    m_Velocity     = Velocity;
    m_Rotation     = vector3( frand(0,R_360),frand(0,R_360),frand(0,R_360) );
    m_Spin         = vector3( frand(-2,2),frand(-2,2),frand(-2,2) );
    m_Timer        = frand( LIFE_TIME_MIN, LIFE_TIME_MAX );
    m_AlphaTime    = m_Timer * 0.8f;
    m_Size         = Size;
    m_Growth       = 2.0f;
    m_GrowthDecay  = 0.0f;
    m_Color        = Color;
    m_Smooth       = false;
}

//==============================================================================

void explosion::SpawnBoid( const vector3& Position, 
                           const vector3& Velocity, 
                                 float    Size, 
                           const color&   Color )
{
    SetPosition( Position );
    m_Velocity     = Velocity;
    m_Rotation     = vector3( frand(0,R_360),frand(0,R_360),frand(0,R_360) );
    m_Spin         = vector3( frand(-2,2),frand(-2,2),frand(-2,2) );
    m_Timer        = frand( LIFE_TIME_MIN, LIFE_TIME_MAX );
    m_AlphaTime    = m_Timer * 0.8f;
    m_Size         = Size;
    m_Growth       = 2.0f;
    m_GrowthDecay  = 0.0f;
    m_Color        = Color;
    m_Smooth       = false;
}

//==============================================================================

void explosion::SpawnMortar( const vector3& Position, 
                             const vector3& Velocity, 
                                   float    Size, 
                             const color&   Color )
{
    SetPosition( Position );
    m_Velocity     = Velocity;
    m_Rotation     = vector3( frand(0,R_360),frand(0,R_360),frand(0,R_360) );
    m_Spin         = vector3( frand(-2,2),frand(-2,2),frand(-2,2) );
    m_Timer        = frand( LIFE_TIME_MIN, LIFE_TIME_MAX ) * 0.8f;
    m_AlphaTime    = m_Timer * 0.8f;
    m_Size         = Size;
    m_Growth       = 200.0f;
    m_GrowthDecay  = 12.00f;
    m_Color        = Color;
    m_Smooth       = true;
}

//==============================================================================

void explosion::OnLogic( float Time )
{
    DEJA_CONTEXT( "explosion::OnLogic" );

    // Move the explosion and scale velocity to decelerate the movement.
    SetPosition( GetPosition() + m_Velocity * Time );
    m_Velocity = m_Velocity * 0.98f;

    // Apply spin.
    m_Rotation += m_Spin * Time;
    m_Spin      = m_Spin * 0.98f;

    // Increate the size by the growth in this time step.
    m_Size += m_Growth * Time;

    // Decay the growth.  
    // This gives a rapid immediate expansion, then slows over time.
    m_Growth -= m_Growth * (m_GrowthDecay * Time);

    // Update the timer and destroy if out of time.
    m_Timer -= Time;
    if( m_Timer <= 0.0f )
    {
        // DO NOT ACCESS this AFTER THIS LINE.
        ObjMgr.RemoveDestroy( this );
    }
}

//==============================================================================

void explosion::OnRender( void )
{
    DEJA_CONTEXT( "explosion::OnRender" );

    // Compute color, alpha fades from 1 to 0 over time.
    color c = m_Color;
    c.A = (unsigned char)(c.A * MIN(m_AlphaTime, m_Timer) / m_AlphaTime);

    // Get our position.
    vector3 p = GetPosition();

    // Set Local to World with rotation based on position to give the explosion 
    // spin as it moves.
    matrix4 M;
    M.Identity();
    M.RotateX( m_Rotation.x );
    M.RotateY( m_Rotation.y );
    M.RotateZ( m_Rotation.z );
    M.Scale( m_Size );
    M.Translate( p );
    Graphics->SetL2W( M );

    // Don't write to Z buffer and enable alpha blending.
    Graphics->DisableZWrite();
    Graphics->EnableAlpha();

    // Render a sphere.
    Graphics->DrawSphere( c, m_Smooth );

    // Restore default alpha and Z write states.
    Graphics->DisableAlpha();
    Graphics->EnableZWrite();
}

//==============================================================================
