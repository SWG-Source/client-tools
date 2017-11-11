//==============================================================================
//
//  CapPoint.cpp
//
//==============================================================================
//  Copyright (C) DejaTools, LLC.  All rights reserved.
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "stdafx.h"
#include "CapPoint.h"
#include "Boid.h"
#include "Arena.h"
#include "TurfTeam.h"
#include "../TinyEngine/graphics.h"
#include "../TinyEngine/engine.h"
#include "../../DejaLib.h"

//==============================================================================
//  STORAGE
//==============================================================================

static vector3 s_CapPointModelVerts1[] =
{
    vector3(  0.7f,  0.7f,  0.0f ),
    vector3(  0.0f,  0.7f,  0.7f ),
    vector3( -0.7f,  0.7f,  0.0f ),
    vector3(  0.0f,  0.7f, -0.7f ),

    vector3(  0.7f,  0.0f,  0.7f ),
    vector3( -0.7f,  0.0f,  0.7f ),
    vector3( -0.7f,  0.0f, -0.7f ),
    vector3(  0.7f,  0.0f, -0.7f ),

    vector3(  0.7f, -0.7f,  0.0f ),
    vector3(  0.0f, -0.7f,  0.7f ),
    vector3( -0.7f, -0.7f,  0.0f ),
    vector3(  0.0f, -0.7f, -0.7f ),
};

static int s_CapPointIndices1[] =
{
    // Square faces (two triangles each).
    0,  2,  1,      0,  3,  2,
    0,  4,  8,      0,  8,  7,
    1,  5,  9,      1,  9,  4,
    2,  6, 10,      2, 10,  5,
    3,  7, 11,      3, 11,  6,
    8,  9, 10,      8, 10, 11,

    // Triangular corners.
    0,  1,  4,
    1,  2,  5,
    2,  3,  6,
    3,  0,  7,
    4,  9,  8,
    5, 10,  9,
    6, 11, 10,
    7,  8, 11,
};
                                               
static vertex_pn s_CapPointVerts1[ 60 ];    // 20 triangles, 3 verts each.

//==============================================================================

static vector3 s_CapPointModelVerts2[] =
{
    vector3(  0.0f,  1.0f,  0.0f ),
    vector3(  0.0f,  0.0f,  1.0f ),
    vector3(  1.0f,  0.0f,  0.0f ),
    vector3(  0.0f,  0.0f, -1.0f ),
    vector3( -1.0f,  0.0f,  0.0f ),
    vector3(  0.0f, -1.0f,  0.0f ),
};

static int s_CapPointIndices2[] =
{
    0, 1, 2, 
    0, 2, 3,
    0, 3, 4, 
    0, 4, 1,
    5, 2, 1,
    5, 3, 2,
    5, 4, 3,
    5, 1, 4,
};

static vertex_pn s_CapPointVerts2[ 8 * 3 ];   // 8 faces, 3 verts each.

//==============================================================================
//  OBJECT SYSTEM HOOKS
//==============================================================================

object*     CapPointFactoryFn( void ) { return( new cap_point ); }
type_data   CapPointTypeData( "cap_point", CapPointFactoryFn );

int         cap_point::m_nTeams = 0;
int         cap_point::m_TeamID[ CAP_POINT_MAX_TEAMS ] = { -1 };

//==============================================================================
//  FUNCTIONS
//==============================================================================

void SystemPrep_CapPoint( void )
{
    // Register the cap_point class with the object manager.
    ObjMgr.RegisterType( CapPointTypeData );

    // Prepare the render data.

    for( int i = 0; i < 20; i++ )
    {
        // Copy verts for the triangle.
        s_CapPointVerts1[ (i*3)+0 ].Position = s_CapPointModelVerts1[ s_CapPointIndices1[(i*3)+0] ];
        s_CapPointVerts1[ (i*3)+1 ].Position = s_CapPointModelVerts1[ s_CapPointIndices1[(i*3)+1] ];
        s_CapPointVerts1[ (i*3)+2 ].Position = s_CapPointModelVerts1[ s_CapPointIndices1[(i*3)+2] ];

        // Generate a normal for each triangle, and associate with each vert.
        vector3 V1 = (s_CapPointVerts1[(i*3)+1].Position - s_CapPointVerts1[(i*3)+0].Position);
        vector3 V2 = (s_CapPointVerts1[(i*3)+2].Position - s_CapPointVerts1[(i*3)+0].Position);
        vector3 N  = Cross( V1, V2 );
        N.Normalize();
        s_CapPointVerts1[ (i*3)+0 ].Normal = N;
        s_CapPointVerts1[ (i*3)+1 ].Normal = N;
        s_CapPointVerts1[ (i*3)+2 ].Normal = N;
    }

    for( int i = 0; i < 8; i++ )
    {
        // Copy verts for the triangle.
        s_CapPointVerts2[ (i*3)+0 ].Position = s_CapPointModelVerts2[ s_CapPointIndices2[(i*3)+0] ];
        s_CapPointVerts2[ (i*3)+1 ].Position = s_CapPointModelVerts2[ s_CapPointIndices2[(i*3)+1] ];
        s_CapPointVerts2[ (i*3)+2 ].Position = s_CapPointModelVerts2[ s_CapPointIndices2[(i*3)+2] ];

        // Generate a normal for each triangle, and associate with each vert.
        vector3 V1 = (s_CapPointVerts2[(i*3)+1].Position - s_CapPointVerts2[(i*3)+0].Position);
        vector3 V2 = (s_CapPointVerts2[(i*3)+2].Position - s_CapPointVerts2[(i*3)+0].Position);
        vector3 N  = Cross( V1, V2 );
        N.Normalize();
        s_CapPointVerts2[ (i*3)+0 ].Normal = N;
        s_CapPointVerts2[ (i*3)+1 ].Normal = N;
        s_CapPointVerts2[ (i*3)+2 ].Normal = N;
    }

    // Register the state enumeration with the DejaLib.
    DEJA_ENUM_ALIAS( cap_point::state, cap_point::CAP_POINT_NEUTRAL,     "NEUTRAL"      );
    DEJA_ENUM_ALIAS( cap_point::state, cap_point::CAP_POINT_SECURING,    "SECURING"     );
    DEJA_ENUM_ALIAS( cap_point::state, cap_point::CAP_POINT_NEUTRALIZING,"NEUTRALIZING" );    
}

//==============================================================================

void DejaDescriptor( const cap_point& C )
{
    DEJA_TYPE ( C, cap_point );
    DEJA_BASE ( C, object );
    DEJA_FIELD( C.m_Level );
    DEJA_FIELD( C.m_iOwner );
    DEJA_FIELD( C.m_iTarget );
}

//==============================================================================
//  FUNCTIONS - cap_point
//==============================================================================

cap_point::cap_point( void )
{
    m_iOwner  =  -1;
    m_iTarget =  -1;
    m_Level   =   0.0f;
    m_Range   =  40.0f;
    m_State   = CAP_POINT_NEUTRAL;
    m_Yaw     = R_0;
}

//==============================================================================

cap_point::~cap_point( void )
{
}

//==============================================================================

void cap_point::OnDejaObjInit( void )
{
    DEJA_OBJECT_INIT( *this );
}

//==============================================================================

void cap_point::OnDejaPost( void )
{
    DEJA_POST( *this );
}

//==============================================================================

void cap_point::OnLogic( float Time )
{
    DEJA_CONTEXT( "cap_point::OnLogic" );

    // If needed, initialize the static list of teams.
    if( m_TeamID[0] == -1 )
    {
        for( int i = 0; i < CAP_POINT_MAX_TEAMS; i++ )
            m_TeamID[i] = -1;

        m_nTeams = 0;
        object* pTeam;
        ObjMgr.ResetTypeLoop( TurfTeamTypeData.iType );
        while( (pTeam = ObjMgr.GetNextInType( TurfTeamTypeData.iType )) != NULL )
        {
            ASSERT( m_nTeams < CAP_POINT_MAX_TEAMS );
            m_TeamID[m_nTeams] = pTeam->GetID();
            m_nTeams++;
        }
    }

    // Increase influence range if there is an owner.
    if( m_InfluenceRange > 0.0f )
    {
        m_InfluenceRange += Time * 4.0f;
    }

    // Slow rotation.
    m_Yaw += (R_90 * Time);
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
            Pos.y = pCell->m_Height + 10;
            SetPosition( Pos );
        }
    }

    // Count the number of boids per team within range.
    
    int Boids[ CAP_POINT_MAX_TEAMS ] = { 0 };
    int Total = 0;

    float R2 = m_Range * m_Range;
    ObjMgr.ResetTypeLoop( BoidTypeData.iType );
    boid* pBoid = (boid*)ObjMgr.GetNextInType( BoidTypeData.iType );
    while( pBoid )
    {
        vector3 Span = GetPosition() - pBoid->GetPosition();
        float   D2   = Span.GetLengthSqr();

        if( D2 < R2 )
        {
            int TeamID = pBoid->GetTeamID();
            for( int i = 0; i < CAP_POINT_MAX_TEAMS; i++ )
            {
                if( TeamID == m_TeamID[i] )
                {
                    Boids[i]++;
                    Total++;
                    break;
                }
            }
        }

        pBoid = (boid*)ObjMgr.GetNextInType( BoidTypeData.iType );
    }

    //
    // Now for the capture logic.
    //

    switch( m_State )
    {
    case CAP_POINT_NEUTRAL:
        {
            // Currently dead neutral.  The team with the most boids within 
            // range gets to start capturing.  If there is a tie, then neither
            // gets it.
            if( Total > 0 )
            {
                // Consider each team...
                for( int i = 0; i < m_nTeams; i++ )
                {
                    // Assume it is a good pick, then refute if possible.
                    bool Valid = true;

                    // Consider all _other_ teams...
                    for( int j = 0; j < m_nTeams; j++ )
                    {
                        // If not same team, and it has fewer or equal boids
                        // in range, then it is not a good pick.
                        if( (i != j) && (Boids[i] <= Boids[j]) )
                            Valid = false;
                    }

                    // If it was a good pick, then it is our new target, and we
                    // can switch to SECURING.
                    if( Valid )
                    {
                        m_iTarget = i;
                        m_Level   = 0.0f;
                        m_State   = CAP_POINT_SECURING;
                        DEJA_TRACE( "cap_point::OnLogic",
                                    "State:%(cap_point::state) - Target:%d[%d]",
                                    m_State, m_iTarget, m_TeamID[m_iTarget] );
                    }
                }

            }
        }
        break;

    case CAP_POINT_SECURING:
        {
            // Currently one team is trying to capture the point.  Count the 
            // number of in-range boids on the target team, and the number on
            // other teams.  Apply influence based on these counts.  Also, the
            // capture point resists change, so give the "opposition" a little
            // edge.  Also, if there are no boids in range, this little edge
            // will cause the capture point to gradually drift back to neutral.

            float nAllied  = 0.00f;
            float nOpposed = 0.75f; // That's the "edge".

            for( int i = 0; i < m_nTeams; i++ )
            {
                if( i == m_iTarget )    nAllied  += Boids[i];
                else                    nOpposed += Boids[i];
            }

            m_Level += ((nAllied - nOpposed) * Time * 0.1f);

            if( m_Level <= 0.0f )
            {
                m_Level   =  0.0f;
                m_iTarget = -1;
                m_State   = CAP_POINT_NEUTRAL;
                DEJA_TRACE( "cap_point::OnLogic",
                            "State:%(cap_point::state) - Target:%d",
                            m_State, m_iTarget );
            }
            else
            if( m_Level >= 1.0f )
            {
                m_Level          =  1.0f;
                m_iOwner         =  m_iTarget;
                m_InfluenceRange = 0.5f;
                m_iTarget        = -1;
                m_State          = CAP_POINT_NEUTRALIZING;
                DEJA_TRACE( "cap_point::OnLogic",
                            "State:%(cap_point::state) - Target:%d",
                            m_State, m_iTarget );
            }
        }
        break;

    case CAP_POINT_NEUTRALIZING:
        {
            // One team has successfully secured the capture point.  Now the
            // other teams are trying to neutralize the point.  The owning team
            // gets the edge now.  Thus, if no boids are within range, the
            // capture point will drift towards loyalty.

            float nAllied  = 0.75f; // That's the "edge".
            float nOpposed = 0.00f;

            for( int i = 0; i < m_nTeams; i++ )
            {
                if( i == m_iOwner )     nAllied  += Boids[i];
                else                    nOpposed += Boids[i];
            }

            m_Level += ((nAllied - nOpposed) * Time * 0.1f);

            if( m_Level <= 0.0f )
            {
                m_Level          =  0.0f;
                m_iOwner         = -1;
                m_InfluenceRange = -1.0f;
                m_iTarget        = -1;
                m_State          = CAP_POINT_NEUTRAL;
                DEJA_TRACE( "cap_point::OnLogic",
                            "State:%(cap_point::state) - Target:%d",
                            m_State, m_iTarget );
            }
            else
            if( m_Level >= 1.0f )
            {
                m_Level   =  1.0f;
            }
        }
        break;
    }
}

//==============================================================================

void cap_point::OnRender( void )
{
    DEJA_CONTEXT( "cap_point::OnRender" );

    matrix4 M;

    M.Identity();
    M.RotateY( m_Yaw );
    M.Scale( 6.0f );
    M.Translate( GetPosition() );
    Graphics->SetL2W( M );

    color Cube( 191, 191, 191 );
    color Octo(   0,   0,   0 );

    if( m_State == CAP_POINT_SECURING )
    {
        int ID = m_TeamID[ m_iTarget ];
        turf_team* pTeam = (turf_team*)ObjMgr.GetObjByID( ID );
        Octo = pTeam->GetColor();
        Octo.R = (unsigned char)(Octo.R * m_Level);
        Octo.G = (unsigned char)(Octo.G * m_Level);
        Octo.B = (unsigned char)(Octo.B * m_Level);
    }

    if( m_State == CAP_POINT_NEUTRALIZING )
    {
        int ID = m_TeamID[ m_iOwner ];
        turf_team* pTeam = (turf_team*)ObjMgr.GetObjByID( ID );
        Cube = pTeam->GetColor();
        Octo.R = (unsigned char)(255 * m_Level);
        Octo.G = (unsigned char)(255 * m_Level);
        Octo.B = (unsigned char)(255 * m_Level);
    }

    Graphics->DrawTriangles( sizeof(s_CapPointVerts1)/sizeof(s_CapPointVerts1[0]), 
                             s_CapPointVerts1, 
                             Cube );
    Graphics->DrawTriangles( sizeof(s_CapPointVerts2)/sizeof(s_CapPointVerts2[0]), 
                             s_CapPointVerts2, 
                             Octo );
}

//==============================================================================

void cap_point::OnRemove( void )
{
}

//==============================================================================

int cap_point::GetOwnerID( void )
{
    if( m_iOwner != -1 )
        return( m_TeamID[ m_iOwner ] );
    else
        return( -1 );
}

//==============================================================================

float cap_point::GetInfluenceRange( void )
{
    return( m_InfluenceRange );
}

//==============================================================================
