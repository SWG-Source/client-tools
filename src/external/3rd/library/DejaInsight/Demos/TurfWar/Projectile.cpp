//==============================================================================
//
//  Projectile.cpp
//
//==============================================================================
//  Copyright (C) DejaTools, LLC.  All rights reserved.
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "stdafx.h"
#include "Projectile.h"
#include "../TinyEngine/engine.h"
#include "../TinyEngine/graphics.h"
#include "../../DejaLib.h"

//==============================================================================
//  DEFINES
//==============================================================================

//==============================================================================
//  OBJECT SYSTEM HOOKS
//==============================================================================

object*     ProjectileFactoryFn( void ) { return( new projectile ); }
type_data   ProjectileTypeData( "projectile", ProjectileFactoryFn );

//==============================================================================
//  FUNCTIONS
//==============================================================================

void SystemPrep_Projectile( void )
{
    // Register the projectile class with the object manager.
    ObjMgr.RegisterType( ProjectileTypeData );
}

//==============================================================================

void DejaDescriptor( const projectile& N )
{
    DEJA_TYPE ( N, projectile );
    DEJA_BASE ( N, object );
}

//==============================================================================
//  FUNCTIONS - projectile
//==============================================================================

projectile::projectile( void )
:   m_State         ( PROJECTILE_STATE_NULL ),
    m_TrailCount    ( 0 ),
    m_TrailColor    (255,255,255)
{
}

//==============================================================================

projectile::~projectile( void )
{
}

//==============================================================================

void projectile::OnDejaPost( void )
{
    DEJA_POST( *this );
}

//==============================================================================

void projectile::AddToTrail( const vector3& Point )
{
    // Insert a new point at the head of the trail buffer.
    memmove( &m_Trail[1], &m_Trail[0], sizeof(m_Trail) - sizeof(m_Trail[0]) );
    m_Trail[0] = Point;

    // Increate the count of points but limit to the size of the buffer.
    m_TrailCount++;
    m_TrailCount = MIN( m_TrailCount, sizeof(m_Trail) / sizeof(m_Trail[0]) );
}

//==============================================================================

void projectile::RenderTrail( void )
{
    DEJA_CONTEXT( "projectile::RenderTrail" );

    // Do we have a trail to render?
    if( m_TrailCount >= 2 )
    {
        // Temporary storage for vertices.
        vertex_pc v[(sizeof(m_Trail)/sizeof(m_Trail[0])) * 2];

        // Iterate over the trail adding line segments between each pair of 
        // points.
        int iVert = 0;
        for( int i=0; i<(m_TrailCount-1); i++ )
        {
            // Get the segment color from the base color with Alpha to fade the 
            // trail out.
            color C = m_TrailColor;
            C.A = (unsigned char)(m_TrailColor.A * (1.0f - ((float)i / (float)(m_TrailCount-1))));

            unsigned long c = (C.A << 24) | (C.B << 16) | (C.G <<  8) | (C.R <<  0);

            // Add 2 vertices.
            v[iVert].Position = m_Trail[i];
            v[iVert].Color    = c;
            iVert++;
            v[iVert].Position = m_Trail[i+1];
            v[iVert].Color    = c;
            iVert++;
        }

        // Use an identity matrix because the trail is in world space.
        matrix4 M;
        M.Identity();
        Graphics->SetL2W( M );

        // Don't write to Z buffer and enable alpha blending.
        Graphics->DisableZWrite();
        Graphics->EnableAlpha();

        // Draw the lines.
        Graphics->DrawLines( iVert, v );

        // Restore default alpha and Z write states.
        Graphics->DisableAlpha();
        Graphics->EnableZWrite();
    }
}

//==============================================================================

void projectile::OnLogic( float Time )
{
    (void)Time;

    DEJA_CONTEXT( "projectile::OnLogic" );

    // This logic should only execute when the projectile is in it's finalize state.
    ASSERT( m_State == PROJECTILE_STATE_FINALIZE );

    // Reduce the number of points in the trail.
    if( m_TrailCount > 0 )
        m_TrailCount--;

    // Destroy the object if no trail left.
    if( m_TrailCount <= 1 )
    {
        // DO NOT ACCESS this AFTER THIS LINE.
        ObjMgr.RemoveDestroy( this );
    }
}

//==============================================================================
