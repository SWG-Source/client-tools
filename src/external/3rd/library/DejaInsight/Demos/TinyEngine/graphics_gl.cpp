//==============================================================================
//  graphics_gl.cpp
//==============================================================================
//  Copyright (C) DejaTools, LLC.  All rights reserved.
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "stdafx.h"
#include "graphics_gl.h"
#include "glext.h"

#include "../../DejaLib.h"

//==============================================================================
//  LIBS
//==============================================================================

#pragma comment( lib, "OpenGL32.lib" )

//==============================================================================
//  STORAGE
//==============================================================================

int      s_FontDisplayListBase = -1;
int      s_GlyphWidths[256];
vector3* s_pVert  = NULL;
color*   s_pColor = NULL;
int      s_nVerts = 0;

//==============================================================================
// OpenGL extensions
//==============================================================================

typedef bool (APIENTRY* PFNWGLSWAPINTERVALPROC)(int);

#define GET_GL_EXT( n, t ) n = (t)wglGetProcAddress( #n );

static  PFNWGLSWAPINTERVALPROC              wglSwapIntervalEXT          = NULL;

static  PFNGLBINDBUFFERARBPROC              glBindBufferARB             = NULL;
static  PFNGLDELETEBUFFERSARBPROC           glDeleteBuffersARB          = NULL;
static  PFNGLGENBUFFERSARBPROC              glGenBuffersARB             = NULL;
static  PFNGLISBUFFERARBPROC                glIsBufferARB               = NULL;
static  PFNGLBUFFERDATAARBPROC              glBufferDataARB             = NULL;
static  PFNGLBUFFERSUBDATAARBPROC           glBufferSubDataARB          = NULL;
static  PFNGLGETBUFFERSUBDATAARBPROC        glGetBufferSubDataARB       = NULL;
static  PFNGLMAPBUFFERARBPROC               glMapBufferARB              = NULL;
static  PFNGLUNMAPBUFFERARBPROC             glUnmapBufferARB            = NULL;
static  PFNGLGETBUFFERPARAMETERIVARBPROC    glGetBufferParameterivARB   = NULL;
static  PFNGLGETBUFFERPOINTERVARBPROC       glGetBufferPointervARB      = NULL;

static  PFNGLDELETEOBJECTARBPROC            glDeleteObjectARB           = NULL;
static  PFNGLGETHANDLEARBPROC               glGetHandleARB              = NULL;
static  PFNGLDETACHOBJECTARBPROC            glDetachObjectARB           = NULL;
static  PFNGLCREATESHADEROBJECTARBPROC      glCreateShaderObjectARB     = NULL;
static  PFNGLSHADERSOURCEARBPROC            glShaderSourceARB           = NULL;
static  PFNGLCOMPILESHADERARBPROC           glCompileShaderARB          = NULL;
static  PFNGLCREATEPROGRAMOBJECTARBPROC     glCreateProgramObjectARB    = NULL;
static  PFNGLATTACHOBJECTARBPROC            glAttachObjectARB           = NULL;
static  PFNGLLINKPROGRAMARBPROC             glLinkProgramARB            = NULL;
static  PFNGLUSEPROGRAMOBJECTARBPROC        glUseProgramObjectARB       = NULL;
static  PFNGLVALIDATEPROGRAMARBPROC         glValidateProgramARB        = NULL;
static  PFNGLUNIFORM1FARBPROC               glUniform1fARB              = NULL;
static  PFNGLUNIFORM2FARBPROC               glUniform2fARB              = NULL;
static  PFNGLUNIFORM3FARBPROC               glUniform3fARB              = NULL;
static  PFNGLUNIFORM4FARBPROC               glUniform4fARB              = NULL;
static  PFNGLUNIFORM1IARBPROC               glUniform1iARB              = NULL;
static  PFNGLUNIFORM2IARBPROC               glUniform2iARB              = NULL;
static  PFNGLUNIFORM3IARBPROC               glUniform3iARB              = NULL;
static  PFNGLUNIFORM4IARBPROC               glUniform4iARB              = NULL;
static  PFNGLUNIFORM1FVARBPROC              glUniform1fvARB             = NULL;
static  PFNGLUNIFORM2FVARBPROC              glUniform2fvARB             = NULL;
static  PFNGLUNIFORM3FVARBPROC              glUniform3fvARB             = NULL;
static  PFNGLUNIFORM4FVARBPROC              glUniform4fvARB             = NULL;
static  PFNGLUNIFORM1IVARBPROC              glUniform1ivARB             = NULL;
static  PFNGLUNIFORM2IVARBPROC              glUniform2ivARB             = NULL;
static  PFNGLUNIFORM3IVARBPROC              glUniform3ivARB             = NULL;
static  PFNGLUNIFORM4IVARBPROC              glUniform4ivARB             = NULL;
static  PFNGLUNIFORMMATRIX2FVARBPROC        glUniformMatrix2fvARB       = NULL;
static  PFNGLUNIFORMMATRIX3FVARBPROC        glUniformMatrix3fvARB       = NULL;
static  PFNGLUNIFORMMATRIX4FVARBPROC        glUniformMatrix4fvARB       = NULL;
static  PFNGLGETOBJECTPARAMETERFVARBPROC    glGetObjectParameterfvARB   = NULL;
static  PFNGLGETOBJECTPARAMETERIVARBPROC    glGetObjectParameterivARB   = NULL;
static  PFNGLGETINFOLOGARBPROC              glGetInfoLogARB             = NULL;
static  PFNGLGETATTACHEDOBJECTSARBPROC      glGetAttachedObjectsARB     = NULL;
static  PFNGLGETUNIFORMLOCATIONARBPROC      glGetUniformLocationARB     = NULL;
static  PFNGLGETACTIVEUNIFORMARBPROC        glGetActiveUniformARB       = NULL;
static  PFNGLGETUNIFORMFVARBPROC            glGetUniformfvARB           = NULL;
static  PFNGLGETUNIFORMIVARBPROC            glGetUniformivARB           = NULL;
static  PFNGLGETSHADERSOURCEARBPROC         glGetShaderSourceARB        = NULL;

//==============================================================================
// SetPerspective
//==============================================================================

static void SetPerspective( GLfloat fov, GLfloat aspect, GLfloat near_plane, GLfloat far_plane )
{
	GLfloat range = near_plane * tanf( ( fov/2.0f ) * (float)M_PI / 180.0f );
	glFrustum( -range*aspect, range*aspect, -range, range, near_plane, far_plane );
}

//==============================================================================
// InitGL
//==============================================================================

void graphics_gl::InitGL( void )
{
    DEJA_CONTEXT( "graphics_gl::InitGL" );
    DEJA_TRACE( "InitGL", "Enter" );

        GLuint PixelFormat;
        static PIXELFORMATDESCRIPTOR pfd =
        {
        sizeof(PIXELFORMATDESCRIPTOR),      // Size Of This Pixel Format Descriptor
        1,                                  // Version Number (?)
        PFD_DRAW_TO_WINDOW |                // Format Must Support Window
        PFD_SUPPORT_OPENGL |                // Format Must Support OpenGL
        PFD_DOUBLEBUFFER,                   // Must Support Double Buffering
        PFD_TYPE_RGBA,                      // Request An RGBA Format
        24,                                 // Select A 24Bit Color Depth
        0, 0, 0, 0, 0, 0,                   // Color Bits Ignored (?)
        0,                                  // No Alpha Buffer
        0,                                  // Shift Bit Ignored (?)
        0,                                  // No Accumulation Buffer
        0, 0, 0, 0,                         // Accumulation Bits Ignored (?)
        24,                                 // 24Bit Z-Buffer (Depth Buffer)  
        0,                                  // No Stencil Buffer
        0,                                  // No Auxiliary Buffer (?)
        PFD_MAIN_PLANE,                     // Main Drawing Layer
        0,                                  // Reserved (?)
        0, 0, 0                             // Layer Masks Ignored (?)
    };

    HDC hDC = GetDC( m_hWnd );

    // Finds the closest match to the pixel format we set above.
    PixelFormat = ChoosePixelFormat( hDC, &pfd );
    if (!PixelFormat)
    {
        DEJA_ERROR( "OpenGL::InitGL", "Failed to choose a pixel format." );
        MessageBox( 0, _T("OpenGL failed to choose a pixel format."), _T("Error"), MB_OK|MB_ICONERROR );
        PostQuitMessage( 0 );
    }

    PIXELFORMATDESCRIPTOR pfd2;
    DescribePixelFormat( hDC, PixelFormat, sizeof(pfd2), &pfd2 );

    if( !SetPixelFormat( hDC, PixelFormat, &pfd) )
    {
        DEJA_ERROR( "OpenGL::InitGL", "Failed to set pixel format." );
        MessageBox( 0, _T("OpenGL failed to set pixel format."), _T("Error"), MB_OK|MB_ICONERROR );
        PostQuitMessage( 0 );
    }

    m_hRC = wglCreateContext( hDC );
    if( !m_hRC )
    {
        DEJA_ERROR( "OpenGL::InitGL", "Failed to create context." );
        MessageBox( 0, _T("OpenGL failed to create context."), _T("Error"), MB_OK|MB_ICONERROR );
        PostQuitMessage( 0 );
    }

    if( !wglMakeCurrent( hDC, m_hRC ) )
    {
        DEJA_ERROR( "OpenGL::InitGL", "Failed wglMakeCurrent." );
        MessageBox( 0, _T("OpenGL failed wglMakeCurrent."), _T("Error"), MB_OK|MB_ICONERROR );
        PostQuitMessage( 0 );
    }

    // Initialize extensions.
    GET_GL_EXT( wglSwapIntervalEXT          , PFNWGLSWAPINTERVALPROC              );

    // Initialize extensions VBO.
    GET_GL_EXT( glBindBufferARB             , PFNGLBINDBUFFERARBPROC              );
    GET_GL_EXT( glDeleteBuffersARB          , PFNGLDELETEBUFFERSARBPROC           );
    GET_GL_EXT( glGenBuffersARB             , PFNGLGENBUFFERSARBPROC              );
    GET_GL_EXT( glIsBufferARB               , PFNGLISBUFFERARBPROC                );
    GET_GL_EXT( glBufferDataARB             , PFNGLBUFFERDATAARBPROC              );
    GET_GL_EXT( glBufferSubDataARB          , PFNGLBUFFERSUBDATAARBPROC           );
    GET_GL_EXT( glGetBufferSubDataARB       , PFNGLGETBUFFERSUBDATAARBPROC        );
    GET_GL_EXT( glMapBufferARB              , PFNGLMAPBUFFERARBPROC               );
    GET_GL_EXT( glUnmapBufferARB            , PFNGLUNMAPBUFFERARBPROC             );
    GET_GL_EXT( glGetBufferParameterivARB   , PFNGLGETBUFFERPARAMETERIVARBPROC    );
    GET_GL_EXT( glGetBufferPointervARB      , PFNGLGETBUFFERPOINTERVARBPROC       );

    // Initialize extensions GLSL.
    GET_GL_EXT( glDeleteObjectARB           , PFNGLDELETEOBJECTARBPROC            );
    GET_GL_EXT( glGetHandleARB              , PFNGLGETHANDLEARBPROC               );
    GET_GL_EXT( glDetachObjectARB           , PFNGLDETACHOBJECTARBPROC            );
    GET_GL_EXT( glCreateShaderObjectARB     , PFNGLCREATESHADEROBJECTARBPROC      );
    GET_GL_EXT( glShaderSourceARB           , PFNGLSHADERSOURCEARBPROC            );
    GET_GL_EXT( glCompileShaderARB          , PFNGLCOMPILESHADERARBPROC           );
    GET_GL_EXT( glCreateProgramObjectARB    , PFNGLCREATEPROGRAMOBJECTARBPROC     );
    GET_GL_EXT( glAttachObjectARB           , PFNGLATTACHOBJECTARBPROC            );
    GET_GL_EXT( glLinkProgramARB            , PFNGLLINKPROGRAMARBPROC             );
    GET_GL_EXT( glUseProgramObjectARB       , PFNGLUSEPROGRAMOBJECTARBPROC        );
    GET_GL_EXT( glValidateProgramARB        , PFNGLVALIDATEPROGRAMARBPROC         );
    GET_GL_EXT( glUniform1fARB              , PFNGLUNIFORM1FARBPROC               );
    GET_GL_EXT( glUniform2fARB              , PFNGLUNIFORM2FARBPROC               );
    GET_GL_EXT( glUniform3fARB              , PFNGLUNIFORM3FARBPROC               );
    GET_GL_EXT( glUniform4fARB              , PFNGLUNIFORM4FARBPROC               );
    GET_GL_EXT( glUniform1iARB              , PFNGLUNIFORM1IARBPROC               );
    GET_GL_EXT( glUniform2iARB              , PFNGLUNIFORM2IARBPROC               );
    GET_GL_EXT( glUniform3iARB              , PFNGLUNIFORM3IARBPROC               );
    GET_GL_EXT( glUniform4iARB              , PFNGLUNIFORM4IARBPROC               );
    GET_GL_EXT( glUniform1fvARB             , PFNGLUNIFORM1FVARBPROC              );
    GET_GL_EXT( glUniform2fvARB             , PFNGLUNIFORM2FVARBPROC              );
    GET_GL_EXT( glUniform3fvARB             , PFNGLUNIFORM3FVARBPROC              );
    GET_GL_EXT( glUniform4fvARB             , PFNGLUNIFORM4FVARBPROC              );
    GET_GL_EXT( glUniform1ivARB             , PFNGLUNIFORM1IVARBPROC              );
    GET_GL_EXT( glUniform2ivARB             , PFNGLUNIFORM2IVARBPROC              );
    GET_GL_EXT( glUniform3ivARB             , PFNGLUNIFORM3IVARBPROC              );
    GET_GL_EXT( glUniform4ivARB             , PFNGLUNIFORM4IVARBPROC              );
    GET_GL_EXT( glUniformMatrix2fvARB       , PFNGLUNIFORMMATRIX2FVARBPROC        );
    GET_GL_EXT( glUniformMatrix3fvARB       , PFNGLUNIFORMMATRIX3FVARBPROC        );
    GET_GL_EXT( glUniformMatrix4fvARB       , PFNGLUNIFORMMATRIX4FVARBPROC        );
    GET_GL_EXT( glGetObjectParameterfvARB   , PFNGLGETOBJECTPARAMETERFVARBPROC    );
    GET_GL_EXT( glGetObjectParameterivARB   , PFNGLGETOBJECTPARAMETERIVARBPROC    );
    GET_GL_EXT( glGetInfoLogARB             , PFNGLGETINFOLOGARBPROC              );
    GET_GL_EXT( glGetAttachedObjectsARB     , PFNGLGETATTACHEDOBJECTSARBPROC      );
    GET_GL_EXT( glGetUniformLocationARB     , PFNGLGETUNIFORMLOCATIONARBPROC      );
    GET_GL_EXT( glGetActiveUniformARB       , PFNGLGETACTIVEUNIFORMARBPROC        );
    GET_GL_EXT( glGetUniformfvARB           , PFNGLGETUNIFORMFVARBPROC            );
    GET_GL_EXT( glGetUniformivARB           , PFNGLGETUNIFORMIVARBPROC            );
    GET_GL_EXT( glGetShaderSourceARB        , PFNGLGETSHADERSOURCEARBPROC         );

    // Set the swap interval if available.
    if( wglSwapIntervalEXT )
        wglSwapIntervalEXT( 1 );

    // Done with the window.
    ReleaseDC( m_hWnd, hDC );

    DEJA_TRACE( "InitGL", "Exit" );
}

//==============================================================================
// graphics_gl
//==============================================================================

graphics_gl::graphics_gl()
{
    m_hWnd = NULL;
}

//==============================================================================
// ~graphics_gl
//==============================================================================

graphics_gl::~graphics_gl( void )
{
}

//==============================================================================
// Init
//==============================================================================

void graphics_gl::Init( void )
{
}

//==============================================================================
// Init
//==============================================================================

void graphics_gl::Init( HWND hWnd )
{
    // Save the window handle.
    m_hWnd = hWnd;

    // Initialize OpenGL.
    InitGL();

    // Initialize the common shapes.
    InitShapes();
}

//==============================================================================
// Kill
//==============================================================================

void graphics_gl::Kill( void )
{
}

//==============================================================================
// WindowResized
//==============================================================================

void graphics_gl::WindowResized( void )
{
    RECT r;
    GetClientRect( m_hWnd, &r );

    // Prevent a divide by zero if the window is too small.
    m_w = r.right  - r.left;
    m_h = r.bottom - r.top;
    if( m_h < 1 )
        m_h = 1;
}

//==============================================================================
// BeginFrame
//==============================================================================

void graphics_gl::BeginFrame( void )
{
    DEJA_CONTEXT( "graphics_gl::BeginFrame" );

    // Setup.
    m_hDC = GetDC( m_hWnd );
    wglMakeCurrent( m_hDC, m_hRC );

    glViewport  ( 0, 0, m_w, m_h );             // Reset the current viewport.
    glDisable   ( GL_TEXTURE_2D );              // Disable texture mapping.
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );     // Set the clear color.
    glClearDepth( 1.0f );                       // Set the clear depth.
    glDepthFunc ( GL_LESS );                    // The type of depth test.
    glEnable    ( GL_DEPTH_TEST );              // Enables depth testing.
    glShadeModel( GL_SMOOTH );                  // Enables smooth color shading.

    // Reset The projection matrix.
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();

    // Setup perspective.
    RECT r;
    GetClientRect( m_hWnd, &r );
    SetPerspective( 45.0f, (GLfloat)(r.right-r.left) / (GLfloat)(r.bottom-r.top), 0.1f, 100.0f );

    // Z Write is enabled by default.
    glDepthMask( GL_TRUE );
    m_ZWrite = true;

    // Blending is disabled by default.
    glDisable( GL_BLEND );
    m_Alpha = false;

    // Culling.
    glFrontFace( GL_CCW );
    glCullFace( GL_BACK );
    glEnable( GL_CULL_FACE );

    // Polygon offset.
    glPolygonOffset( 1, 1 );
    glEnable( GL_POLYGON_OFFSET_FILL );

    // Set model matrix mode.
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    // Setup some ambient illumination.
    SetAmbientLight( color(128, 128, 128, 255) );

    // Light 0.
    {
        GLfloat LightDiffuse [] = { 0.5f, 0.5f, 0.5f, 1.0f };
        GLfloat LightPosition[] = { 100.0f, 100.0f, 100.0f, 1.0f };
        glLightfv( GL_LIGHT0, GL_DIFFUSE , LightDiffuse  );
        glLightfv( GL_LIGHT0, GL_POSITION, LightPosition );
        glEnable ( GL_LIGHT0 );
    }

    // Light 1.
    {
    //  GLfloat LightDiffuse [] = { 0.5f, 0.5f, 0.5f, 1.0f };
    //  GLfloat LightPosition[] = { -100.0f, 100.0f, 0.0f, 1.0f };
    //  glLightfv( GL_LIGHT1, GL_DIFFUSE , LightDiffuse  );
    //  glLightfv( GL_LIGHT1, GL_POSITION, LightPosition );
    //  glEnable ( GL_LIGHT1 );
    }

    // Enable normalization of normals to deal with scale in matrices.
    glEnable( GL_NORMALIZE );

    // Lighting on.
    glEnable ( GL_LIGHTING );
    m_Lighting = true;

    // Material.
    glColorMaterial( GL_FRONT, GL_AMBIENT_AND_DIFFUSE );
    glEnable       ( GL_COLOR_MATERIAL );
}

//==============================================================================
// EndFrame
//==============================================================================

void graphics_gl::EndFrame( void )
{
    DEJA_CONTEXT( "graphics_gl::EndFrame" );

    // Teardown.
    glFlush         ( );
    SwapBuffers     ( m_hDC );
    wglMakeCurrent  ( NULL, NULL );
    ValidateRect    ( m_hWnd, NULL );
    ReleaseDC       ( m_hWnd, m_hDC );
}

//==============================================================================
// GetViewport
//==============================================================================

void graphics_gl::GetViewport( int& l, int& t, int& w, int& h )
{
    l = 0;
    t = 0;
    w = m_w;
    h = m_h;
}

//==============================================================================
// SetView
//==============================================================================

void graphics_gl::SetView( const view& View )
{
    m_View = View;
    m_W2V  = View.GetW2V();
    m_W2V.RotateY( R_180 );     // Convert Deja space to OpenGL space.

    matrix4 L2V = m_W2V * m_L2W;

    glMatrixMode( GL_MODELVIEW );
    glLoadMatrixf( (GLfloat*)&L2V );

    // Prevent a divide by zero if the window is too small.
    float XFOV = View.GetXFOV();
    if( m_h < 1 )
        m_h = 1;
    float Aspect = (float)m_w / (float)m_h;

    if( View.GetMode() == view::PERSPECTIVE )
    {
        // Reset the perspective transformation.
        float Near, Far;
        View.GetZLimits( Near, Far );
        glMatrixMode  ( GL_PROJECTION );
        glLoadIdentity( );
    	GLfloat Range = Near * tanf( ( XFOV * 0.5f ) );
	    glFrustum( -Range, Range, -Range/Aspect, Range/Aspect, Near, Far );

        // Back to the modelview matrix.
        glMatrixMode( GL_MODELVIEW );
        glLoadIdentity( );
    }
    else
    {
        float Near, Far;
        float XWOV = View.GetXWOV();
        View.GetZLimits( Near, Far );

        glMatrixMode  ( GL_PROJECTION );
        glLoadIdentity( );
        glOrtho( -(XWOV*0.5f) * Aspect, (XWOV*0.5f) * Aspect, 
                 -(XWOV*0.5f), (XWOV*0.5f), 
                 Near, Far );

        // Back to the modelview matrix.
        glMatrixMode( GL_MODELVIEW );
        glLoadIdentity( );
    }
}

//==============================================================================
// SetL2W
//==============================================================================

void graphics_gl::SetL2W( const matrix4& L2W )
{
    m_L2W = L2W;

    matrix4 L2V = m_W2V * m_L2W;

    glMatrixMode( GL_MODELVIEW );
    glLoadMatrixf( (GLfloat*)&L2V );
}

//==============================================================================
// EnableZWrite
//==============================================================================

void graphics_gl::EnableZWrite( void )
{
    if( m_ZWrite )
        return;

    glDepthMask( GL_TRUE );

    m_ZWrite = true;
}

//==============================================================================
// DisableZWrite
//==============================================================================

void graphics_gl::DisableZWrite( void )
{
    if( !m_ZWrite )
        return;

    glDepthMask( GL_FALSE );

    m_ZWrite = false;
}

//==============================================================================
// EnableAlpha
//==============================================================================

void graphics_gl::EnableAlpha( void )
{
    if( m_Alpha )
        return;

    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glEnable   ( GL_BLEND );

    m_Alpha = true;
}

//==============================================================================
// DisableAlpha
//==============================================================================

void graphics_gl::DisableAlpha( void )
{
    if( !m_Alpha )
        return;

    glDisable( GL_BLEND );

    m_Alpha = false;
}

//==============================================================================
// EnableLighting
//==============================================================================

void graphics_gl::EnableLighting( void )
{
    if( m_Lighting )
        return;

    glEnable( GL_LIGHTING );

    m_Lighting = true;
}

//==============================================================================
// DisableLighting
//==============================================================================

void graphics_gl::DisableLighting( void )
{
    if( !m_Lighting )
        return;

    glDisable( GL_LIGHTING );

    m_Lighting = false;
}

//==============================================================================
// GetAmbientLight
//==============================================================================

color graphics_gl::GetAmbientLight( void )
{
    return m_Ambient;
}

//==============================================================================
// SetAmbientLight
//==============================================================================

void graphics_gl::SetAmbientLight( const color& Color )
{
    m_Ambient = Color;

    // Set the ambient color.
    GLfloat Ambient[] = { Color.R/255.0f, Color.G/255.0f, Color.B/255.0f, Color.A/255.0f };
    glLightModelfv( GL_LIGHT_MODEL_AMBIENT, Ambient );
}

//==============================================================================
// Clear
//==============================================================================

void graphics_gl::Clear( const color& Color )
{
    GLclampf R = Color.R / 255.0f;
    GLclampf G = Color.G / 255.0f;
    GLclampf B = Color.B / 255.0f;
    GLclampf A = Color.A / 255.0f;

    glClearColor( R, G, B, A );
    glClearDepth( 1.0f );
    glClear     ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

//==============================================================================
// SetColor
//==============================================================================

void graphics_gl::SetColor( const color& Color )
{
    glColor4ubv( (GLubyte*)&Color );
}

//==============================================================================
// DrawGrid
//==============================================================================
//  
//  For Steps = 2
//                         
//      *---*---*---*---*  
//      |   |   |   |   |     :   = Axis
//      *---+---+---+---*    ( )  = Origin
//      |   |   :   |   |  
//      *---+--( )--+---*  
//      |   |   |   |   |  
//      *---+---+---+---*  
//      |   |   |   |   |  
//      *---*---*---*---*  
//         
//==============================================================================

void graphics_gl::DrawGrid( const vector3& Origin,
                            const vector3& Normal,
                            const vector3& Axis1,
                            const color&   Color,
                                  int      Steps )
{
    DEJA_CONTEXT( "graphics_gl::DrawGrid" );

    // Make sure we have enough storage.

    int nLines = ((Steps * 2) + 1) * 2;
    int nVerts = nLines * 2;

    if( s_nVerts < nVerts )
    {
        free( s_pVert  );
        free( s_pColor );

        s_pVert  = (vector3*)malloc( nVerts * sizeof(vector3) );
        s_pColor = (color  *)malloc( nVerts * sizeof(color  ) );
        s_nVerts = nVerts;
    }

    // Create the second axis vector.

    vector3 Axis2 = Cross( Axis1, Normal );
    Axis2.Normalize();
    Axis2 = Axis2 * Axis1.GetLength();

    // Additional preparations.

    vector3* pVert  = s_pVert;
    color*   pColor = s_pColor;

    vector3  V1     = Axis1 * (float)Steps;
    vector3  V2     = Axis2 * (float)Steps;

    // Create the lines.

    for( int i = -Steps; i <= +Steps; i++ )
    {
        float  S  = (float)i;

        *pVert++  = Origin + V1 + (Axis2 * S);
        *pVert++  = Origin - V1 + (Axis2 * S);

        *pVert++  = Origin + V2 + (Axis1 * S);
        *pVert++  = Origin - V2 + (Axis1 * S);

        *pColor++ = Color;
        *pColor++ = Color;

        *pColor++ = Color;
        *pColor++ = Color;
    }

    // A little clean up.

    if( !m_Alpha )
        glEnable        ( GL_BLEND );
    if( m_Lighting )
        glDisable       ( GL_LIGHTING );

    glVertexPointer     ( 3, GL_FLOAT        , sizeof(vector3), s_pVert  );
    glColorPointer      ( 4, GL_UNSIGNED_BYTE, sizeof(color  ), s_pColor );
    glEnableClientState ( GL_VERTEX_ARRAY );
    glEnableClientState ( GL_COLOR_ARRAY  );

    glDrawArrays        ( GL_LINES, 0, nVerts );

    glDisableClientState( GL_VERTEX_ARRAY );
    glDisableClientState( GL_COLOR_ARRAY  );

    if( m_Lighting )
        glEnable        ( GL_LIGHTING );
    if( !m_Alpha )
        glDisable       ( GL_BLEND );
}                                       

//==============================================================================
// DrawTriGrid 
//==============================================================================
//
//  For Steps = 2
//                         
//        V2                 
//          *---*---*      
//         / \ / \ / \        ==  = Axis
//        *---+---+---*      ( )  = Origin
//       / \ / \ / \ / \   
//      *---+--( )==+---* V1 
//       \ / \ / \ / \ /   
//        *---+---+---*    
//         \ / \ / \ /     
//          *---*---*      
//        V3    
//            
//==============================================================================

void graphics_gl::DrawTriGrid( const vector3& Origin,
                               const vector3& Normal,
                               const vector3& Axis1,
                               const color&   Color,
                                     int      Steps )
{
    DEJA_CONTEXT( "graphics_gl::DrawTriGrid" );

    // Make sure we have enough storage.

    int nLines = ((Steps * 2) + 1) * 3;
    int nVerts = nLines * 2;

    if( s_nVerts < nVerts )
    {
        free( s_pVert  );
        free( s_pColor );

        s_pVert  = (vector3*)malloc( nVerts * sizeof(vector3) );
        s_pColor = (color  *)malloc( nVerts * sizeof(color  ) );
        s_nVerts = nVerts;
    }

    // Create the other two axis vectors.

    float   H     = Axis1.GetLength();
    vector3 AxisP = Cross( Axis1, Normal );
    AxisP.Normalize();
    H *= 0.86602540378443864676372317075294f;
    AxisP = AxisP * H;

    vector3 Axis2 = (Axis1 * -0.5f) + AxisP;
    vector3 Axis3 = (Axis1 * -0.5f) - AxisP;


    // Additional preparations.
    
    vector3* pVert  = s_pVert;
    color*   pColor = s_pColor;

    vector3  V1     = Axis1 * (float)Steps;
    vector3  V2     = Axis2 * (float)Steps;
    vector3  V3     = Axis3 * (float)Steps;

    // Create the lines.

    for( int i = 0; i < Steps; i++ )
    {
        float  S  = (float)i;

        *pVert++  = Origin + V1 + (Axis3 * S);
        *pVert++  = Origin - V3 - (Axis1 * S);

        *pVert++  = Origin + V2 + (Axis1 * S);
        *pVert++  = Origin - V1 - (Axis2 * S);

        *pVert++  = Origin + V3 + (Axis2 * S);
        *pVert++  = Origin - V2 - (Axis3 * S);

        *pVert++  = Origin + V3 + (Axis1 * S);
        *pVert++  = Origin - V1 - (Axis3 * S);

        *pVert++  = Origin + V1 + (Axis2 * S);
        *pVert++  = Origin - V2 - (Axis1 * S);

        *pVert++  = Origin + V2 + (Axis3 * S);
        *pVert++  = Origin - V3 - (Axis2 * S);

        *pColor++ = Color;
        *pColor++ = Color;
        *pColor++ = Color;
        *pColor++ = Color;
        *pColor++ = Color;
        *pColor++ = Color;
        *pColor++ = Color;
        *pColor++ = Color;
        *pColor++ = Color;
        *pColor++ = Color;
        *pColor++ = Color;
        *pColor++ = Color;
    }

    // Last 3 lines.

    {
        *pVert++  = Origin + V1;
        *pVert++  = Origin - V1;

        *pVert++  = Origin + V2;
        *pVert++  = Origin - V2;

        *pVert++  = Origin + V3;
        *pVert++  = Origin - V3;

        *pColor++ = Color;
        *pColor++ = Color;
        *pColor++ = Color;
        *pColor++ = Color;
        *pColor++ = Color;
        *pColor++ = Color;
    }

    // A little clean up.

    if( !m_Alpha )
        glEnable        ( GL_BLEND );
    if( m_Lighting )
        glDisable       ( GL_LIGHTING );

    glVertexPointer     ( 3, GL_FLOAT        , sizeof(vector3), s_pVert  );
    glColorPointer      ( 4, GL_UNSIGNED_BYTE, sizeof(color  ), s_pColor );
    glEnableClientState ( GL_VERTEX_ARRAY );
    glEnableClientState ( GL_COLOR_ARRAY  );

    glDrawArrays        ( GL_LINES, 0, nVerts );

    glDisableClientState( GL_VERTEX_ARRAY );
    glDisableClientState( GL_COLOR_ARRAY  );

    if( m_Lighting )
        glEnable        ( GL_LIGHTING );
    if( !m_Alpha )
        glDisable       ( GL_BLEND );
}                                       

//==============================================================================
// DrawSphere
//==============================================================================

void graphics_gl::DrawSphere( const vector3& Origin,
                              float          Radius,
                              int            Precision,
                              const color&   Color )
{
    DEJA_CONTEXT( "graphics_gl::DrawSphere" );

    const float PI     = 3.14159265358979f;
    const float TWOPI  = 6.28318530717958f;
    const float PIDIV2 = 1.57079632679489f;

    float cx = Origin.x;
    float cy = Origin.y;
    float cz = Origin.z;
    float r  = Radius;
    int   p  = Precision;

    float theta1 = 0.0;
    float theta2 = 0.0;
    float theta3 = 0.0;

    float ex = 0.0f;
    float ey = 0.0f;
    float ez = 0.0f;

    float px = 0.0f;
    float py = 0.0f;
    float pz = 0.0f;

    // Disallow a negative number for radius.
    if( r < 0 )
        r = -r;

    // Disallow a negative number for precision.
    if( p < 0 )
        p = -p;

    // Set color.
    glColor4ubv( (const GLubyte*)&Color );

    // Iterate.
    for( int i = 0; i < p/2; ++i )
    {
        theta1 = i * TWOPI / p - PIDIV2;
        theta2 = (i + 1) * TWOPI / p - PIDIV2;

        glBegin( GL_TRIANGLE_STRIP );
        {
            for( int j = 0; j <= p; ++j )
            {
                theta3 = j * TWOPI / p;

                ex = cosf(theta1) * cosf(theta3);
                ey = sinf(theta1);
                ez = cosf(theta1) * sinf(theta3);
                px = cx + r * ex;
                py = cy + r * ey;
                pz = cz + r * ez;

                glNormal3f( ex, ey, ez );
                glVertex3f( px, py, pz );

                ex = cosf(theta2) * cosf(theta3);
                ey = sinf(theta2);
                ez = cosf(theta2) * sinf(theta3);
                px = cx + r * ex;
                py = cy + r * ey;
                pz = cz + r * ez;

                glNormal3f( ex, ey, ez );
                glVertex3f( px, py, pz );

            }
        }
        glEnd();
    }
}

//==============================================================================
// DrawLine
//==============================================================================

void graphics_gl::DrawLine( const vector3&   v1, 
                            const vector3&   v2, 
                            const color&     Color )
{
    if( m_Lighting )
        glDisable( GL_LIGHTING );

    glBegin( GL_LINES );
        glColor4ubv( (const GLubyte*)&Color );
        glVertex3fv( (const GLfloat*)&v1 );
        glVertex3fv( (const GLfloat*)&v2 );
    glEnd();

    if( m_Lighting )
        glEnable( GL_LIGHTING );
}

//==============================================================================
// DrawTriangles
//==============================================================================

void graphics_gl::DrawLines( int nVerts, const vertex_pc* pVerts )
{
    if( m_Lighting )
        glDisable( GL_LIGHTING );

    glEnableClientState ( GL_VERTEX_ARRAY );
    glEnableClientState ( GL_COLOR_ARRAY  );

    glVertexPointer     ( 3, GL_FLOAT        , sizeof(*pVerts), &pVerts[0].Position );
    glColorPointer      ( 4, GL_UNSIGNED_BYTE, sizeof(*pVerts), &pVerts[0].Color    );

    glDrawArrays        ( GL_LINES, 0, nVerts );

    glDisableClientState( GL_VERTEX_ARRAY );
    glDisableClientState( GL_COLOR_ARRAY  );

    if( m_Lighting )
        glEnable( GL_LIGHTING );
}

//==============================================================================
// DrawTriangles
//==============================================================================

void graphics_gl::DrawTriangles( int nVerts, const vertex_pn* pVerts, const color& Color )
{
    glEnableClientState ( GL_VERTEX_ARRAY );
    glEnableClientState ( GL_NORMAL_ARRAY );

    glColor4ubv         ( (const GLubyte*)&Color );
    glVertexPointer     ( 3, GL_FLOAT, sizeof(*pVerts), &pVerts[0].Position );
    glNormalPointer     (    GL_FLOAT, sizeof(*pVerts), &pVerts[0].Normal   );

    glDrawArrays        ( GL_TRIANGLES, 0, nVerts );

    glDisableClientState( GL_VERTEX_ARRAY );
    glDisableClientState( GL_NORMAL_ARRAY );
}

//==============================================================================
// DrawTriangles
//==============================================================================

void graphics_gl::DrawTriangles( int nVerts, const vertex_pc* pVerts )
{
    glEnableClientState ( GL_VERTEX_ARRAY );
    glEnableClientState ( GL_COLOR_ARRAY );

    glVertexPointer     ( 3, GL_FLOAT        , sizeof(*pVerts), &pVerts[0].Position );
    glColorPointer      ( 4, GL_UNSIGNED_BYTE, sizeof(*pVerts), &pVerts[0].Color    );

    glDrawArrays        ( GL_TRIANGLES, 0, nVerts );

    glDisableClientState( GL_VERTEX_ARRAY );
    glDisableClientState( GL_COLOR_ARRAY );
}

//==============================================================================
// DrawText
//==============================================================================

void graphics_gl::DrawText( const char* pString, int x, int y, const color& Color )
{
    // Generate the bitmap font display lists.
    if( s_FontDisplayListBase == -1 )
    {
        s_FontDisplayListBase = glGenLists( 256-32 );
        HDC hDC = GetDC( m_hWnd );

        // Create a new font.
        LOGFONTW LogFont;
        memset( &LogFont, 0, sizeof(LogFont) );
        int LogPixelsY = GetDeviceCaps( hDC, LOGPIXELSY );
        wcscpy_s( LogFont.lfFaceName, 32, L"Tahoma" );
        LogFont.lfHeight         = -MulDiv( 8, LogPixelsY, 72 );
        LogFont.lfCharSet        = DEFAULT_CHARSET ;
        LogFont.lfItalic         = 0;
        LogFont.lfWeight         = 400;
        LogFont.lfUnderline      = 0;
        LogFont.lfStrikeOut      = 0;
        LogFont.lfPitchAndFamily = VARIABLE_PITCH;
        HFONT hFont = CreateFontIndirectW( &LogFont );
        SelectObject( hDC, hFont );

        wglUseFontBitmaps( hDC, 32, 256-32, s_FontDisplayListBase );

        MAT2 Mat2 ={{0,1},{0,0},{0,0},{0,1}};
        GLYPHMETRICS gm;
        for( int i=32; i<256; i++ )
        {
            GetGlyphOutline( hDC, i, GGO_METRICS, &gm, NULL, NULL, &Mat2 );
            s_GlyphWidths[i] = gm.gmCellIncX; //gm.gmBlackBoxX + 1;
        }

        ReleaseDC( m_hWnd, hDC );
    }

    // Setup the matrix for 2D rendering.
    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    glOrtho( 0.0f, m_w, 0.0f, m_h, -1, 1 );
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();

    // Setup the display list base.
    glListBase( s_FontDisplayListBase - 32 );

    // Disable lighting.
    glDisable( GL_LIGHTING );

    // Set the color of the text.
    glColor4ubv( (GLubyte*)&Color );

    // Set the start position, use glBitmap to avoid the culling problems 
    // with glRasterPos.
    glRasterPos2f( 0.0f, 0.0f );
    glBitmap( 0, 0, 0.0f, 0.0f, (GLfloat)x, (GLfloat)(m_h-y), NULL );

    // Draw the characters.
    int StringLen = (int)strlen( pString );
    glCallLists( StringLen, GL_UNSIGNED_BYTE, pString );

    // Restore matrices.
    glPopMatrix();
    glMatrixMode( GL_PROJECTION );
    glPopMatrix();
    glMatrixMode( GL_MODELVIEW );

    if( m_Lighting )
        glEnable( GL_LIGHTING );
}

//==============================================================================
