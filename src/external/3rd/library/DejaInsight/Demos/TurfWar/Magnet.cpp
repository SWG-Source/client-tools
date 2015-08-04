//==============================================================================
//
//  Magnet.cpp
//
//==============================================================================
//  Copyright (C) DejaTools, LLC.  All rights reserved.
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "stdafx.h"
#include "Magnet.h"
#include "../TinyEngine/graphics.h"
#include "../TinyEngine/engine.h"
#include "../../DejaLib.h"

//==============================================================================
//  OBJECT SYSTEM HOOKS
//==============================================================================

object*     MagnetFactoryFn( void ) { return( new magnet ); }
type_data   MagnetTypeData( "magnet", MagnetFactoryFn );

//==============================================================================
//  FUNCTIONS
//==============================================================================

void SystemPrep_Magnet( void )
{
    // Register the magnet class with the object manager.
    ObjMgr.RegisterType( MagnetTypeData );
}

//==============================================================================

void DejaDescriptor( const magnet& N )
{
    DEJA_TYPE ( N, magnet );
    DEJA_BASE ( N, object );
    DEJA_FIELD( N.m_Velocity );
}

//==============================================================================
//  FUNCTIONS - magnet
//==============================================================================

magnet::magnet( void )
:   m_Velocity(  0,  0,  0 ),
    m_RangeMin( -1, -1, -1 ),
    m_RangeMax( +1, +1, +1 ),
    m_nBoids  (  0 )
{
}

//==============================================================================

magnet::~magnet( void )
{
}

//==============================================================================

void magnet::OnDejaObjInit( void )
{
    DEJA_OBJECT_INIT( *this );
}

//==============================================================================

void magnet::OnDejaPost( void )
{
    DEJA_POST( *this );
}

//==============================================================================

void magnet::Spawn( const vector3& Position, const vector3& Velocity )
{
    SetPosition( Position );
    m_Velocity = Velocity;
}

//==============================================================================

void magnet::SetRange( const vector3& Min, const vector3& Max )
{
    ASSERT( Min.x < Max.x );
    ASSERT( Min.y < Max.y );
    ASSERT( Min.z < Max.z );

    m_RangeMin = Min;
    m_RangeMax = Max;
}

//==============================================================================

int magnet::GetBoidCount( void )
{
    return( m_nBoids );
}

//==============================================================================

void magnet::IncBoidCount( void )
{
    m_nBoids++;
}

//==============================================================================

void magnet::DecBoidCount( void )
{
    m_nBoids--;
}

//==============================================================================

vector3 magnet::GetPredictedPos( float Time )
{
    return( GetPosition() + (m_Velocity * Time) );
}

//==============================================================================

void magnet::OnLogic( float Time )
{
    vector3 Move     = m_Velocity * Time;
    vector3 Position = GetPosition() + Move;

    // Keep inside of allowed range.

    if( (Position.x > m_RangeMax.x) && (m_Velocity.x > 0.0f) )
    {
        Position.x   = (m_RangeMax.x * 2.0f) - Position.x;
        m_Velocity.x = -m_Velocity.x;
    }
    else
    if( (Position.x < m_RangeMin.x) && (m_Velocity.x < 0.0f) )
    {
        Position.x   = (m_RangeMin.x * 2.0f) - Position.x;
        m_Velocity.x = -m_Velocity.x;
    }

    if( (Position.y > m_RangeMax.y) && (m_Velocity.y > 0.0f) )
    {
        Position.y   = (m_RangeMax.y * 2.0f) - Position.y;
        m_Velocity.y = -m_Velocity.y;
    }
    else
    if( (Position.y < m_RangeMin.y) && (m_Velocity.y < 0.0f) )
    {
        Position.y   = (m_RangeMin.y * 2.0f) - Position.y;
        m_Velocity.y = -m_Velocity.y;
    }

    if( (Position.z > m_RangeMax.z) && (m_Velocity.z > 0.0f) )
    {
        Position.z   = (m_RangeMax.z * 2.0f) - Position.z;
        m_Velocity.z = -m_Velocity.z;
    }
    else
    if( (Position.z < m_RangeMin.z) && (m_Velocity.z < 0.0f) )
    {
        Position.z   = (m_RangeMin.z * 2.0f) - Position.z;
        m_Velocity.z = -m_Velocity.z;
    }

    SetPosition( Position );
}

//==============================================================================

void magnet::OnRender( void )
{
}

//==============================================================================

void magnet::OnRemove( void )
{
    ASSERT( m_nBoids == 0 );
}

//==============================================================================
