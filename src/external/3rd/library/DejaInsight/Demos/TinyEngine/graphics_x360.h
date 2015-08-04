//==============================================================================
//  graphics_x360.h
//==============================================================================
//  Copyright (C) DejaTools, LLC.  All rights reserved.
//==============================================================================

#ifndef graphics_x360_h
#define graphics_x360_h

//==============================================================================
//  INCLUDES
//==============================================================================

#include "graphics.h"

//==============================================================================
//  graphics_x360
//==============================================================================

class graphics_x360 : public graphics
{
protected:
        IDirect3D9*                     m_pD3D;                                 // Used to create the D3D device.
        IDirect3DDevice9*               m_pd3dDevice;                           // The rendering device.
        XVIDEO_MODE                     m_VideoMode;                            // The video mode.
        D3DPRESENT_PARAMETERS           m_d3dpp;                                // Present parameters.
        IDirect3DVertexShader9*         m_VertexShader_pc;
        IDirect3DPixelShader9*          m_PixelShader_pc;
        IDirect3DVertexDeclaration9*    m_VertexDecl_pc;
        IDirect3DVertexShader9*         m_VertexShader_pn;
        IDirect3DPixelShader9*          m_PixelShader_pn;
        IDirect3DVertexDeclaration9*    m_VertexDecl_pn;

        view                            m_View;                                 // View to render.
        XMMATRIX                        m_WVP;                                  // World*View*Projection matrix.
        matrix4                         m_L2W;                                  // Local to World matrix.
        bool                            m_ZWrite;                               // ZWrite enabled state.
        bool                            m_Alpha;                                // Alpha enabled state.
        bool                            m_Lighting;                             // Lighting enabled state.
        color                           m_Ambient;                              // Ambient light color.

public:
                    graphics_x360           ( void );                           // Construct & attach to window
virtual            ~graphics_x360           ( void );

virtual void        Init                    ( void );                           // Initialize graphics system.
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
#endif // graphics_x360_h
//==============================================================================
