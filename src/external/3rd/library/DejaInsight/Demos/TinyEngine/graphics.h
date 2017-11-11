//==============================================================================
//  graphics.h
//==============================================================================
//  Copyright (C) DejaTools, LLC.  All rights reserved.
//==============================================================================

#ifndef graphics_h
#define graphics_h

//==============================================================================
//  INCLUDES
//==============================================================================

#include "view.h"
#include "color.h"

//==============================================================================
//  TYPES - vertex
//==============================================================================

struct vertex_pc
{
    vector3         Position;
    unsigned long   Color;
};

struct vertex_pn
{
    vector3         Position;
    vector3         Normal;
};

//==============================================================================
//  TYPES - graphics
//==============================================================================

class graphics
{
public:
                    graphics                ( void ) {};                        // Construct.
virtual            ~graphics                ( void ) {};                        // Destruct.

virtual void        Init                    ( void ) = 0;                       // Initialize graphics system.
virtual void        Kill                    ( void ) = 0;                       // Kill graphics system.

protected:
        void        InitShapes              ( void );                           // Initialize the shapes.

                    //==========================================================
                    // Window & System functions
                    //==========================================================
public:
virtual void        WindowResized           ( void ) = 0;                       // Called when window is resized.
virtual void        BeginFrame              ( void ) = 0;                       // Begin the frame.
virtual void        EndFrame                ( void ) = 0;                       // End the frame.

                    //==========================================================
                    // Drawing functions
                    //==========================================================

virtual void        GetViewport             ( int& l, int& t,
                                              int& w, int& h ) = 0;             // Get the viewport.

virtual void        SetView                 ( const view&       View ) = 0;     // Set the view.
virtual void        SetL2W                  ( const matrix4&    L2W ) = 0;      // Set the L2W transform.

virtual void        EnableZWrite            ( void ) = 0;                       // Enable Z Writes.
virtual void        DisableZWrite           ( void ) = 0;                       // Disable Z Writes.

virtual void        EnableAlpha             ( void ) = 0;                       // Enable Alpha blending.
virtual void        DisableAlpha            ( void ) = 0;                       // Disable Alpha blending.

virtual void        EnableLighting          ( void ) = 0;                       // Enable Lighting.
virtual void        DisableLighting         ( void ) = 0;                       // Disable Lighting.
virtual color       GetAmbientLight         ( void ) = 0;                       // Get ambient light color.
virtual void        SetAmbientLight         ( const color&      Color ) = 0;    // Set ambient light color.

virtual void        Clear                   ( const color&      Color ) = 0;    // Clear framebuffer.

virtual void        SetColor                ( const color&      Color ) = 0;    // Set draw color.

virtual void        DrawGrid                ( const vector3&    Origin,
                                              const vector3&    Normal,
                                              const vector3&    Axis,
                                              const color&      Color,
                                                    int         Steps ) = 0;    // Draw a grid (of squares).

virtual void        DrawTriGrid             ( const vector3&    Origin,
                                              const vector3&    Normal,
                                              const vector3&    Axis,
                                              const color&      Color,
                                                    int         Steps ) = 0;    // Draw a grid of triangles.

virtual void        DrawLine                ( const vector3&    v1,
                                              const vector3&    v2,
                                              const color&      Color ) = 0;    // Draw a line.

virtual void        DrawLines               ( int               nVerts,
                                              const vertex_pc*  pVerts ) = 0;   // Draw lines.

virtual void        DrawTriangles           ( int               nVerts,
                                              const vertex_pn*  pVerts,
                                              const color&      Color ) = 0;    // Draw triangles.

virtual void        DrawTriangles           ( int               nVerts,
                                              const vertex_pc*  pVerts ) = 0;   // Draw triangles.

virtual void        DrawText                ( const char*       pString,
                                              int               x,
                                              int               y,
                                              const color&      Color ) = 0;    // Draw text.

        void        DrawOctahedron          ( const color&      Color );        // Draw an octahedron.

        void        DrawSphere              ( const color&      Color,
                                              bool              HiRes = true ); // Draw a sphere.
};

//==============================================================================
//  STORAGE
//==============================================================================

extern graphics* Graphics;

//==============================================================================
#endif // graphics_h
//==============================================================================
