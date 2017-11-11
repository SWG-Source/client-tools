//==============================================================================
//
//  Arena.cpp
//
//==============================================================================
//  Copyright (C) DejaTools, LLC.  All rights reserved.
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "stdafx.h"

#include "../TinyEngine/engine.h"
#include "../TinyEngine/graphics.h"

#include "Arena.h"
#include "Turret.h"

#include "../../DejaLib.h"

//==============================================================================
//  DEFINES
//==============================================================================

#define HEX_SIDE            10.0f                   // Length of one side of the hex
#define TALL_PROBABILITY    (5.0f/100.0f)           // Probablity of a cell being tall
#define DELAY_TIME_MIN      8.0f                    // Min time to be in delay state
#define DELAY_TIME_MAX     10.0f                    // Max time to be in delay state
#define MOVE_TIME_MIN       1.0f                    // Min time to complete move
#define MOVE_TIME_MAX       3.0f                    // Max time to complete move
#define TALL_MIN           20.0f                    // Min height of a tall cell
#define TALL_MAX           50.0f                    // Max height of a tall cell
#define SHORT_MIN           1.0f                    // Min height of a short cell
#define SHORT_MAX           1.0f                    // Max height of a short cell

#define COLOR_NEUTRAL       color( 128, 128, 192 )
#define COLOR_DAMAGE        color( 224, 128, 128 )

//==============================================================================
//  OBJECT SYSTEM HOOKS
//==============================================================================

object*     ArenaFactoryFn( void ) { return( new arena ); }
type_data   ArenaTypeData( "arena", ArenaFactoryFn );

//==============================================================================
//  FUNCTIONS
//==============================================================================

void SystemPrep_Arena( void )
{
    // Register the boid class with the object manager.
    ObjMgr.RegisterType( ArenaTypeData );
}

//==============================================================================

void DejaDescriptor( const arena& A )
{
    DEJA_TYPE ( A, arena );
    DEJA_BASE ( A, object );
}

//==============================================================================

void ComputeCellVerts( cell& c )
{
    DEJA_CONTEXT( "arena::ComputeCellVerts" );

    // Get an offset vector with just height in y.
    vector3 Offset( 0.0f, c.m_Height, 0.0f );

    // Initialize the triangle and line indexes.
    int iTri  = 0;
    int iLine = 0;

    // Outline lines.
    for( int s=0; s<6; s++ )
    {
        c.m_Lines[iLine+0].Position = c.m_Base[1 + ((s+0)%6)] + Offset;
        c.m_Lines[iLine+1].Position = c.m_Base[1 + ((s+1)%6)] + Offset;
        c.m_Lines[iLine+0].Color    = 0xc0c0c0;
        c.m_Lines[iLine+1].Color    = 0xc0c0c0;
        iLine += 2;

        c.m_Lines[iLine+0].Position = c.m_Base[1+s] + Offset;
        c.m_Lines[iLine+1].Position = c.m_Base[1+s];
        c.m_Lines[iLine+0].Color    = 0xc0c0c0;
        c.m_Lines[iLine+1].Color    = 0xc0c0c0;
        iLine += 2;
    }

    // Top triangles.
    for( int s=0; s<6; s++ )
    {
        c.m_Triangles[iTri+0].Position = c.m_Base[0            ] + Offset;
        c.m_Triangles[iTri+1].Position = c.m_Base[1 + ((s+1)%6)] + Offset;
        c.m_Triangles[iTri+2].Position = c.m_Base[1 + ((s+0)%6)] + Offset;
        iTri += 3;
    }

    // Side triangles.
    for( int s=0; s<6; s++ )
    {
        c.m_Triangles[iTri+0].Position = c.m_Triangles[s*3 + 1].Position;
        c.m_Triangles[iTri+1].Position = c.m_Triangles[s*3 + 1].Position - Offset;
        c.m_Triangles[iTri+2].Position = c.m_Triangles[s*3 + 2].Position;
        iTri += 3;

        c.m_Triangles[iTri+0].Position = c.m_Triangles[s*3 + 2].Position;
        c.m_Triangles[iTri+1].Position = c.m_Triangles[s*3 + 1].Position - Offset;
        c.m_Triangles[iTri+2].Position = c.m_Triangles[s*3 + 2].Position - Offset;
        iTri += 3;
    }
}

//==============================================================================

void ComputeCellNormals( cell& c )
{
    DEJA_CONTEXT( "arena::ComputeCellNormals" );

    // Iterate over the triangles in the cell.
    for( int iVert=0; iVert<(sizeof(c.m_Triangles)/sizeof(c.m_Triangles[0])); iVert+=3 )
    {
        // Get 2 edges of the triangle and cross them & normalize the result to get a normal.
        vector3 V1 = (c.m_Triangles[iVert+1].Position - c.m_Triangles[iVert+0].Position);
        vector3 V2 = (c.m_Triangles[iVert+2].Position - c.m_Triangles[iVert+0].Position);
        vector3 N  = Cross( V1, V2 );
        N.Normalize();

        // All verts get the same normal for a faceted look.
        c.m_Triangles[iVert+0].Normal = N;
        c.m_Triangles[iVert+1].Normal = N;
        c.m_Triangles[iVert+2].Normal = N;
    }
}

//==============================================================================

int HexDistance( int i1, int j1, int i2, int j2 )
{
    // Get the delta i and dela j values.
    int di = i2-i1;
    int dj = j2-j1;

    // In like signed quadrants, the distance is the sum of delta i and 
    // delta j.
    if( ((di >= 0) && (dj >= 0)) || ((di < 0) && (dj < 0)) )
    {
        return( abs(di) + abs(dj) );
    }
    else
    {
        // In the opposite signed quadrants, the distance is the max of delta i 
        // and delta j.
        di = abs(di);
        dj = abs(dj);
        return( MAX( di, dj ) );
    }
}

//==============================================================================
//  FUNCTIONS - arena
//==============================================================================

arena::arena( void )
:   m_pGrid( NULL )
{
}

//==============================================================================

arena::~arena( void )
{
    delete[] m_pGrid;
}

//==============================================================================

void arena::OnDejaObjInit( void )
{
    DEJA_OBJECT_INIT( *this );
}

//==============================================================================

void arena::OnDejaPost( void )
{
    DEJA_POST( *this );
}

//==============================================================================

void arena::Init( int ArenaSize )
{
    DEJA_CONTEXT( "arena::Init" );

    // Initialize.
    m_Size       = ArenaSize;
    m_GridSize   = m_Size*2 + 1;
    m_pGrid      = new cell*[m_GridSize*m_GridSize];

    // Setup some constants for the arena.
    float   Side    = HEX_SIDE;
    float   Sin30   = 0.5f;
    float   Cos30   = 0.866025403f;

    // Initialize the i and j axes.
    vector3 iAxis( Side*Sin30*3, 0, Side*Cos30   );
    vector3 jAxis( 0           , 0, Side*Cos30*2 );

    // Iterate over j.
    vector3 jOrigin = (iAxis + jAxis) * (float)-m_Size;
    for( int j=-m_Size; j<=m_Size; j++ )
    {
        // Iterate over i.
        vector3 iOrigin( jOrigin );
        for( int i=-m_Size; i<=m_Size; i++ )
        {
            // Valid address for a cell?
            if( ((i*j) <= 0) || (abs(j) <= (m_Size - abs(i))) )
            {
                cell* pCell = new cell;

                //     1       2
                //      *-----*
                //     / \   / \ 
                //    /   \ /   \
                // 6 *-----0-----* 3
                //    \   / \   /
                //     \ /   \ / 
                //      *-----*
                //     5       4

                // Setup base vertices from grid.
                pCell->m_Base[0] = iOrigin;
                pCell->m_Base[1] = iOrigin + vector3(  Side*Sin30  , 0,  Side*Cos30 );
                pCell->m_Base[2] = iOrigin + vector3( -Side*Sin30  , 0,  Side*Cos30 );
                pCell->m_Base[3] = iOrigin + vector3( -Side*Sin30*2, 0,  0          );
                pCell->m_Base[4] = iOrigin + vector3( -Side*Sin30  , 0, -Side*Cos30 );
                pCell->m_Base[5] = iOrigin + vector3(  Side*Sin30  , 0, -Side*Cos30 );
                pCell->m_Base[6] = iOrigin + vector3(  Side*Sin30*2, 0,  0          );

                // Initialize the cell.
                pCell->m_i                = i;
                pCell->m_j                = j;
                pCell->m_Radius           = Side;
                pCell->m_Height           = 1.0f;
                pCell->m_TargetHeight     = 1.0f;
                pCell->m_MoveState        = cell::STATE_DELAY;
                pCell->m_MoveTimer        = 0.0f;
                pCell->m_ColorTimer       = 0.0f;
                pCell->m_Color            = COLOR_NEUTRAL;
                pCell->m_SrcColor         = pCell->m_Color;
                pCell->m_DstColor         = pCell->m_Color;

                // Compute vertices and normals for the cell.
                ComputeCellVerts  ( *pCell );
                ComputeCellNormals( *pCell );

                // Add the cell into the grid.
                m_pGrid[(j+m_Size)*m_GridSize + i+m_Size] = pCell;
            }
            else
            {
                // Illegal cell (i,j) index, just flag grid as empty.
                m_pGrid[(j+m_Size)*m_GridSize + i+m_Size] = NULL;
            }

            // Advance position.
            iOrigin += iAxis;
        }

        // Advance position.
        jOrigin += jAxis;
    }
}

//==============================================================================

void arena::OnLogic( float Time )
{
    DEJA_CONTEXT( "arena::OnLogic" );

    // Iterate over all the cells.
    int nCells = m_GridSize * m_GridSize;
    for( int i=0; i<nCells; i++ )
    {
        // Get the cell, skip for empty grid locations.
        cell* pCell = m_pGrid[i];
        if( !pCell )
            continue;

        // Get the cell as a reference.
        cell& c = *pCell;

        // Need to animate color?
        if( c.m_ColorTimer > 0.0f )
        {
            // Update the timer limiting it to a minimum value of 0.
            c.m_ColorTimer -= Time;
            c.m_ColorTimer = MAX( c.m_ColorTimer, 0.0f );

            // Compute a parametric 't', then blend the Src and Dst colors.
            float t = 1.0f - (c.m_ColorTimer / c.m_ColorAnimTime);
            c.m_Color = Blend( c.m_SrcColor, c.m_DstColor, t );
        }

        // Advance movement.
        switch( c.m_MoveState )
        {
        case cell::STATE_MOVING:
            {
                // Check for height animation.
                c.m_Height += c.m_DeltaHeight * Time;
                if( c.m_DeltaHeight < 0.0f )
                {
                    // Check for reaching target height.
                    if( c.m_Height <= c.m_TargetHeight )
                    {
                        // Fix height at target height and move to delay state.
                        c.m_Height    = c.m_TargetHeight;
                        c.m_MoveState = cell::STATE_DELAY;
                        c.m_MoveTimer = frand( DELAY_TIME_MIN, DELAY_TIME_MAX );
                    }
                }
                else
                {
                    // Check for reaching target height.
                    if( c.m_Height >= c.m_TargetHeight )
                    {
                        // Fix height at target height and move to delay state.
                        c.m_Height    = c.m_TargetHeight;
                        c.m_MoveState = cell::STATE_DELAY;
                        c.m_MoveTimer = frand( DELAY_TIME_MIN, DELAY_TIME_MAX );
                    }
                }

                // Recompute the vertices after a height change (normals don't
                // change).
                ComputeCellVerts( c );
            }
            break;

        case cell::STATE_DELAY:
            {
                // Decrease delay time remaining.
                c.m_MoveTimer -= Time;
                if( c.m_MoveTimer <= 0.0f )
                {
                    // Get a new target height.
                    if( frand( 0.0f, 1.0f ) < TALL_PROBABILITY )
                        c.m_TargetHeight = frand( TALL_MIN, TALL_MAX );
                    else
                        c.m_TargetHeight = frand( SHORT_MIN, SHORT_MAX );

                    // Determine the rate to move to the new height.
                    float t = frand( MOVE_TIME_MIN, MOVE_TIME_MAX );
                    c.m_DeltaHeight = (c.m_TargetHeight - c.m_Height) / t;

                    // Set new state.
                    c.m_MoveState = cell::STATE_MOVING;
                }
            }
            break;
        }
    }
}

//==============================================================================

void arena::OnRender( void )
{
    DEJA_CONTEXT( "arena::OnRender" );

    // Set Local to World.
    matrix4 M;
    M.Identity();
    Graphics->SetL2W( M );

    // Iterate over all the cells.
    int nCells = m_GridSize * m_GridSize;
    for( int i=0; i<nCells; i++ )
    {
        // Get the cell, skip for empty grid locations.
        cell* pCell = m_pGrid[i];
        if( !pCell )
            continue;

        // Render triangles and lines for the cell.
        Graphics->DrawTriangles( 18*3, pCell->m_Triangles, pCell->m_Color );
        Graphics->DrawLines( 12*2, pCell->m_Lines );
    }
}

//==============================================================================

cell* arena::GetCell( int i, int j )
{
    cell* pCell = NULL;

    // Valid address for a cell?
    if( (abs(i) <= m_Size) && 
        (abs(j) <= m_Size) && 
        (((i*j) <= 0) || (abs(j) <= (m_Size - abs(i)))) )
    {
        // Return a pointer to the cell.
        pCell = m_pGrid[(j+m_Size)*m_GridSize + (i+m_Size)];
    }

    // Return the cell, or NULL if there is no cell at that location.
    return( pCell );
}

//==============================================================================

cell* arena::GetCell( const vector3& Point )
{
    // Setup some constants for the arena.
    float   Side     = HEX_SIDE;
    float   Sin30    = 0.5f;
    float   Cos30    = 0.866025403f;

    // Convert the Point x,z into i,j space.
    float iFloat = (Point.x / (Side*Sin30*3));
    float jFloat = ((Point.z - ((Point.x / (Side*Sin30*3))*Side*Cos30)) / (Side*Cos30*2));
    int   iBase  = (int)iFloat;
    int   jBase  = (int)jFloat;

    // Adjust loop ranges when in the negative quadrants.
    if( iFloat < 0 )
        iBase -= 1;
    if( jFloat < 0 )
        jBase -= 1;

    // Iterate over the 4 hex cells surrounding the point.
    float ClosestDist = Side*2.0f;
    cell* pClosest = NULL;
    for( int j=jBase; j<(jBase+2); j++ )
    {
        if( (j >= -m_Size) && (j <= m_Size) )
        {
            for( int i=iBase; i<(iBase+2); i++ )
            {
                if( (i >= -m_Size) && (i <= m_Size) )
                {
                    // Read the cell number from the grid.
                    cell *pCell = m_pGrid[ (j+m_Size)*m_GridSize + (i+m_Size) ];
                    if( pCell )
                    {
                        // Get the base position of the cell.
                        vector3& b = pCell->m_Base[0];

                        // Check radius of cell against point.
                        float dx   = Point.x - b.x;
                        float dz   = Point.z - b.z;
                        float Dist = sqrt( dx*dx + dz*dz );
                        if( (Dist <= pCell->m_Radius) && (Dist <= ClosestDist) )
                        {
                            // Save this cell as the closest.
                            ClosestDist = Dist;
                            pClosest    = pCell;
                        }
                    }
                }
            }
        }
    }

    // Return closest cell or NULL if no cells.
    return( pClosest );
}

//==============================================================================

bool arena::Intersects( const vector3& Point, float )
{
    // Setup some constants for the arena.
    float   Side     = HEX_SIDE;
    float   Sin30    = 0.5f;
    float   Cos30    = 0.866025403f;

    // Convert the Point x,z into i,j space.
    float iFloat = (Point.x / (Side*Sin30*3));
    float jFloat = ((Point.z - ((Point.x / (Side*Sin30*3))*Side*Cos30)) / (Side*Cos30*2));
    int   iBase  = (int)iFloat;
    int   jBase  = (int)jFloat;

    // Adjust loop ranges when in the negative quadrants.
    if( iFloat < 0 )
        iBase -= 1;
    if( jFloat < 0 )
        jBase -= 1;

    // Iterate over the 4 hex cells surrounding the point.
    for( int j=jBase; j<(jBase+2); j++ )
    {
        if( (j >= -m_Size) && (j <= m_Size) )
        {
            for( int i=iBase; i<(iBase+2); i++ )
            {
                if( (i >= -m_Size) && (i <= m_Size) )
                {
                    // Read the cell number from the grid.
                    cell* pCell = m_pGrid[ (j+m_Size)*m_GridSize + (i+m_Size) ];
                    if( pCell )
                    {
                        // Get the base position of the cell.
                        vector3& b = pCell->m_Base[0];

                        // Check height of cell against point.
                        float dy = Point.y - (b.y + pCell->m_Height);
                        if( dy < 0.0f )
                        {
                            // Check radius of cell against point.
                            float dx = Point.x - b.x;
                            float dz = Point.z - b.z;
                            if( (dx*dx + dz*dz) < (pCell->m_Radius*pCell->m_Radius) )
                            {
                                // Flag a hit on the cell with a color pulse.
                                pCell->m_SrcColor      = COLOR_DAMAGE;
                                pCell->m_ColorTimer    = 1.0f;
                                pCell->m_ColorAnimTime = 1.0f;
                                return( true );
                            }
                        }
                    }
                }
            }
        }
    }

    // No intersection.
    return( false );
}

//==============================================================================
