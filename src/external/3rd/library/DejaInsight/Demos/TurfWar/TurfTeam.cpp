//==============================================================================
//
//  TurfTeam.cpp
//
//==============================================================================
//  Copyright (C) DejaTools, LLC.  All rights reserved.
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "stdafx.h"
#include "TurfTeam.h"
#include "Magnet.h"
#include "Boid.h"
#include "../TinyEngine/graphics.h"
#include "../../DejaLib.h"

//==============================================================================
//  OBJECT SYSTEM HOOKS
//==============================================================================

object*     TurfTeamFactoryFn( void ) { return( new turf_team ); }
type_data   TurfTeamTypeData( "turf_team", TurfTeamFactoryFn );

//==============================================================================
//  FUNCTIONS
//==============================================================================

void SystemPrep_TurfTeam( void )
{
    // Register the turf team class with the object manager.
    ObjMgr.RegisterType( TurfTeamTypeData );
}

//==============================================================================

void DejaDescriptor( const turf_team& T )
{
    DEJA_TYPE ( T, turf_team );
    DEJA_BASE ( T, object );
}

//==============================================================================
//  FUNCTIONS - turf_team
//==============================================================================

turf_team::turf_team( void )
{
    for( int i = 0; i < MAGNETS_PER_TEAM; i++ )
        m_MagnetID[i] = -1;
}

//==============================================================================

turf_team::~turf_team( void )
{
}

//==============================================================================

void turf_team::SetColor( color& Color )
{
    m_Color = Color;
}

//==============================================================================

color turf_team::GetColor( void )
{
    return( m_Color );
}

//==============================================================================

void turf_team::OnDejaObjInit( void )
{
    DEJA_OBJECT_INIT( *this );
}

//==============================================================================

void turf_team::OnLogic( float Time )
{
    (void)Time;

    for( int i = 0; i < MAGNETS_PER_TEAM; i++ )
    {
        // If there is no magnet in the position, create a new one and give it
        // a fresh squadron of boids.
        if( m_MagnetID[i] == -1 )
        {
            vector3 SpawnPt( 0, 25, 100 );
            SpawnPt.RotateY( frand( R_0, R_360 ) );

            vector3 Velocity( 0, 0, 30 );
            Velocity.RotateY( frand( R_0, R_360 ) );

            magnet* pMagnet = (magnet*)ObjMgr.CreateObject( "magnet" );
            pMagnet->SetRange( vector3( -100,  +15, -100 ),
                               vector3( +100,  +25, +100 ) );
            pMagnet->Spawn( SpawnPt, Velocity );
            ObjMgr.AddObject( pMagnet );
            m_MagnetID[i] = pMagnet->GetID();

            for( int j = 0; j < BOIDS_PER_MAGNET; j++ )
            {
                boid* pBoid = (boid*)ObjMgr.CreateObject( "boid" );
                pBoid->SetTeam( GetID(), m_Color );
                pBoid->SetMagnet( m_MagnetID[i] );
                pBoid->SetVelocity( frand( 20, 40 ) );
                pBoid->Spawn( SpawnPt, frand( R_0, R_360 ) );
                ObjMgr.AddObject( pBoid );

                pMagnet->IncBoidCount();
            }
        }

        // If the magnet has lost all of its boids, then retire the magnet.
        else
        {
            magnet* pMagnet = (magnet*)ObjMgr.GetObjByID( m_MagnetID[i] );
            if( pMagnet->GetBoidCount() == 0 )
            {
                ObjMgr.RemoveDestroy( m_MagnetID[i] );
                m_MagnetID[i] = -1;
            }
        }
    }
}

//==============================================================================

void turf_team::OnRender( void )
{
}

//==============================================================================

void turf_team::OnRemove( void )
{
    for( int i = 0; i < MAGNETS_PER_TEAM; i++ )
    {
        if( m_MagnetID[i] != -1 )
        {
            boid*   pBoid;
            ObjMgr.ResetTypeLoop( BoidTypeData.iType );
            while( (pBoid = (boid*)ObjMgr.GetNextInType( BoidTypeData.iType )) != NULL )
            {
                if( pBoid->GetMagnetID() == m_MagnetID[i] )
                {
                    ObjMgr.RemoveDestroy( pBoid->GetID() );
                }
            }

            ObjMgr.RemoveDestroy( m_MagnetID[i] );
        }
    }
}

//==============================================================================

void turf_team::OnDejaPost( void )
{
    DEJA_POST( *this );
}

//==============================================================================
