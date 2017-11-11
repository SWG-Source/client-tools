//==============================================================================
//  graphics_gl.h
//==============================================================================
//  Copyright (C) DejaTools, LLC.  All rights reserved.
//==============================================================================

#ifndef graphics_gl_h
#define graphics_gl_h

//==============================================================================
//  INCLUDES
//==============================================================================

#include "graphics.h"
#include <gl/gl.h>

//==============================================================================
//  graphics_gl
//==============================================================================

class graphics_gl : public graphics
{
protected:
        HWND        m_hWnd;                                                     // The window.
        HDC         m_hDC;                                                      // Device context for rendering.
        HGLRC       m_hRC;                                                      // GL render context.
        int         m_w;                                                        // Width of viewport.
        int         m_h;                                                        // Height of viewport.
        view        m_View;                                                     // View to render.
        matrix4     m_W2V;                                                      // World to view matrix.
        matrix4     m_L2W;                                                      // Local to World matrix.
        bool        m_ZWrite;                                                   // ZWrite enabled state.
        bool        m_Alpha;                                                    // Alpha enabled state.
        bool        m_Lighting;                                                 // Lighting enabled state.
        color       m_Ambient;                                                  // Ambient light color.

protected:
        void        InitGL                  ( void );                           // Init GL.
public:
                    graphics_gl             ( void );                           // Construct & attach to window
virtual            ~graphics_gl             ( void );

virtual void        Init                    ( void );                           // Initialize graphics system.
        void        Init                    ( HWND              hWnd );         // Initialize graphics system with hWnd.
virtual void        Kill                    ( void );                           // Kill graphics system.

                    //==========================================================
                    // Window & System functions
                    //==========================================================

virtual void        WindowResized           ( void );                           // Called when window is resized.
virtual void        BeginFrame              ( void );                           // Begin the frame.
virtual void        EndFrame                ( void );                           // End the frame.

                    //==========================================================
                    // Drawing functions
                    //==========================================================

virtual void        GetViewport             ( int& l, int& t,
                                              int& w, int& h );                 // Get the viewport.

virtual void        SetView                 ( const view&       View );         // Set the view.
virtual void        SetL2W                  ( const matrix4&    L2W );          // Set the L2W transform.

virtual void        EnableZWrite            ( void );                           // Enable Z Writes.
virtual void        DisableZWrite           ( void );                           // Disable Z Writes.

virtual void        EnableAlpha             ( void );                           // Enable Alpha blending.
virtual void        DisableAlpha            ( void );                           // Disable Alpha blending.

virtual void        EnableLighting          ( void );                           // Enable Lighting.
virtual void        DisableLighting         ( void );                           // Disable Lighting.
virtual color       GetAmbientLight         ( void );                           // Get ambient light color.
virtual void        SetAmbientLight         ( const color&      Color );        // Set ambient light color.

virtual void        Clear                   ( const color&      Color );        // Clear framebuffer.

virtual void        SetColor                ( const color&      Color );        // Set draw color.

virtual void        DrawGrid                ( const vector3&    Origin,
                                              const vector3&    Normal,
                                              const vector3&    Axis,
                                              const color&      Color,
                                                    int         Steps );        // Draw a grid of squares.

virtual void        DrawTriGrid             ( const vector3&    Origin,
                                              const vector3&    Normal,
                                              const vector3&    Axis,
                                              const color&      Color,
                                                    int         Steps );        // Draw a grid of triangles.

virtual void        DrawSphere              ( const vector3&    Origin,
                                              float             Radius,
                                              int               Precision,
                                              const color&      Color );        // Draw a sphere.

virtual void        DrawLine                ( const vector3&    v1,
                                              const vector3&    v2,
                                              const color&      Color );        // Draw a line.

virtual void        DrawLines               ( int               nVerts,
                                              const vertex_pc*  pVerts );       // Draw lines.

virtual void        DrawTriangles           ( int               nVerts,
                                              const vertex_pn*  pVerts,
                                              const color&      Color );        // Draw triangles.

virtual void        DrawTriangles           ( int               nVerts,
                                              const vertex_pc*  pVerts );       // Draw triangles.

virtual void        DrawText                ( const char*       pString,
                                              int               x,
                                              int               y,
                                              const color&      Color );        // Draw text.
};

//==============================================================================
#endif // graphics_gl_h
//==============================================================================
