//==============================================================================
//  graphics_common.cpp
//==============================================================================
//  Copyright (C) DejaTools, LLC.  All rights reserved.
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "stdafx.h"
#include "graphics.h"

#include "../../DejaLib.h"

//==============================================================================
//  STORAGE
//==============================================================================

// Six equidistant points lying on the unit sphere.
#define XMAX  vector3(  1.0f,  0.0f,  0.0f )
#define XMIN  vector3( -1.0f,  0.0f,  0.0f )
#define YMAX  vector3(  0.0f,  1.0f,  0.0f )
#define YMIN  vector3(  0.0f, -1.0f,  0.0f )
#define ZMAX  vector3(  0.0f,  0.0f,  1.0f )
#define ZMIN  vector3(  0.0f,  0.0f, -1.0f )

// Vertices of a unit octahedron.
vector3 s_Octahedron[] =
{
    XMAX, ZMAX, YMAX,
    YMAX, ZMAX, XMIN,
    XMIN, ZMAX, YMIN,
    YMIN, ZMAX, XMAX,
    XMAX, YMAX, ZMIN,
    YMAX, XMIN, ZMIN,
    XMIN, YMIN, ZMIN,
    YMIN, XMAX, ZMIN,
};

static vertex_pn s_OctahedronVerts[ 8 * 3         ];
static vertex_pn s_Sphere1Verts   [ 8 * 3 * 4     ];
static vertex_pn s_Sphere2Verts   [ 8 * 3 * 4 * 4 ];

//==============================================================================
//  FUNCTIONS
//==============================================================================

void GenerateNormals( vertex_pn* pVerts, int nTris, bool Facets = true )
{
    // Iterate over the triangles
    for( int i=0; i<nTris; i++ )
    {
        // Show facets?
        if( Facets )
        {
            // Get 2 edges of the triangle and cross them & normalize the result to get a normal
            vector3 V1 = (pVerts[(i*3)+1].Position - pVerts[(i*3)+0].Position);
            vector3 V2 = (pVerts[(i*3)+2].Position - pVerts[(i*3)+0].Position);
            vector3 N  = Cross( V1, V2 );
            N.Normalize();

            // All verts get the same normal for a faceted look
            pVerts[ (i*3)+0 ].Normal = N;
            pVerts[ (i*3)+1 ].Normal = N;
            pVerts[ (i*3)+2 ].Normal = N;
        }
        else
        {
            // For these shapes the normal is conveniently equal to the position
            pVerts[ (i*3)+0 ].Normal = pVerts[(i*3)+0].Position;
            pVerts[ (i*3)+1 ].Normal = pVerts[(i*3)+1].Position;
            pVerts[ (i*3)+2 ].Normal = pVerts[(i*3)+2].Position;
        }
    }
}

//==============================================================================

void Subdivide( vertex_pn* pSrc, vertex_pn* pDst, int nTris )
{
    //
    // Subdivision of a triangle, the input vertices are 0,1,2.
    // a,b,c are midpoints on the edges.
    //
    //           (0)
    //            *
    //           / \
    //          /   \
    //         /     \
    //     (a)*-------*(c)
    //       / \     / \
    //      /   \   /   \
    //     /     \ /     \
    // (1)*-------*-------*(2)
    //           (b)
    //

    // Iterate over triangles creating 4 new triangles for each one.
    int iOut = 0;
    for( int i=0; i<nTris; i++ )
    {
        // Get source vertices 0,1,2
        vector3 v0 = pSrc[ (i*3)+0 ].Position;
        vector3 v1 = pSrc[ (i*3)+1 ].Position;
        vector3 v2 = pSrc[ (i*3)+2 ].Position;

        // Compute midpoints a,b,c these must be normalize to lay on the unit sphere.
        vector3 va = (v0 + v1) * 0.5f;
        vector3 vb = (v1 + v2) * 0.5f;
        vector3 vc = (v2 + v0) * 0.5f;
        va.Normalize();
        vb.Normalize();
        vc.Normalize();

        // Output the 4 new triangles.

        pDst[iOut++].Position = v0;
        pDst[iOut++].Position = va;
        pDst[iOut++].Position = vc;

        pDst[iOut++].Position = va;
        pDst[iOut++].Position = v1;
        pDst[iOut++].Position = vb;

        pDst[iOut++].Position = va;
        pDst[iOut++].Position = vb;
        pDst[iOut++].Position = vc;

        pDst[iOut++].Position = vc;
        pDst[iOut++].Position = vb;
        pDst[iOut++].Position = v2;
    }
}

//==============================================================================
// InitShapes
//==============================================================================

void graphics::InitShapes( void )
{
    // For each triangle...
    for( int i=0; i<8; i++ )
    {
        // Copy verts for the triangle.
        s_OctahedronVerts[ (i*3)+0 ].Position = s_Octahedron[ (i*3)+0 ];
        s_OctahedronVerts[ (i*3)+1 ].Position = s_Octahedron[ (i*3)+2 ];
        s_OctahedronVerts[ (i*3)+2 ].Position = s_Octahedron[ (i*3)+1 ];
    }

    // Subdivide base octahedron to create sphere1, subdivide that for sphere2.
    Subdivide( s_OctahedronVerts, s_Sphere1Verts, 8   );
    Subdivide( s_Sphere1Verts   , s_Sphere2Verts, 8*4 );

    // Generate normals for all 3 shapes.
    GenerateNormals( s_OctahedronVerts, 8    , true );
    GenerateNormals( s_Sphere1Verts   , 8*4  , true );
    GenerateNormals( s_Sphere2Verts   , 8*4*4, true );
}

//==============================================================================
// DrawOctahedron
//==============================================================================

void graphics::DrawOctahedron( const color&   Color )
{
    Graphics->DrawTriangles( sizeof(s_OctahedronVerts)/sizeof(s_OctahedronVerts[0]),
                             s_OctahedronVerts,
                             Color );
}

//==============================================================================
// DrawSphere
//==============================================================================

void graphics::DrawSphere( const color& Color,
                           bool         HiRes )
{
    // Use the high resolution or low resolution sphere.
    if( HiRes )
    {
        Graphics->DrawTriangles( sizeof(s_Sphere2Verts)/sizeof(s_Sphere2Verts[0]),
                                 s_Sphere2Verts,
                                 Color );
    }
    else
    {
        Graphics->DrawTriangles( sizeof(s_Sphere1Verts)/sizeof(s_Sphere1Verts[0]),
                                 s_Sphere1Verts,
                                 Color );
    }
}

//==============================================================================
