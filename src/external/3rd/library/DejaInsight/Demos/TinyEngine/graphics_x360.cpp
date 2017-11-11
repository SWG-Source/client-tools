//==============================================================================
// graphics_x360.cpp
//==============================================================================
//  Copyright (C) DejaTools, LLC.  All rights reserved.
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "stdafx.h"
#include "graphics_x360.h"

#include "../../DejaLib.h"

//==============================================================================
//  STORAGE
//==============================================================================

vertex_pc*    s_pColorVert = NULL;
int           s_nVerts     = 0;

//==============================================================================
//  SHADERS
//==============================================================================

const char* g_VertexShaderProgram_pc = 
" float4x4 matWVP : register(c0);                       "
"                                                       "
" struct VS_IN                                          "
"                                                       "
" {                                                     "
"     float4 ObjPos   : POSITION;                       "  // Object space position
"     float4 Color    : COLOR;                          "  // Vertex color
" };                                                    "
"                                                       "
" struct VS_OUT                                         "
" {                                                     "
"     float4 ProjPos  : POSITION;                       "  // Projected space position
"     float4 Color    : COLOR;                          "
" };                                                    "
"                                                       "
" VS_OUT main( VS_IN In )                               "
" {                                                     "
"     VS_OUT Out;                                       "
"     Out.ProjPos = mul( matWVP, In.ObjPos );           "  // Transform vertex into
"     Out.Color.rgba = In.Color.bgra;                   "  // Projected space and
"     return Out;                                       "  // Transfer color
" }                                                     ";

const char* g_PixelShaderProgram_pc = 
" struct PS_IN                                          "
" {                                                     "
"     float4 Color : COLOR;                             "  // Interpolated color from                      
" };                                                    "  // the vertex shader
"                                                       "  
" float4 main( PS_IN In ) : COLOR                       "  
" {                                                     "  
"     return In.Color;                                  "  // Output color
" }                                                     "; 

const char* g_VertexShaderProgram_pn = 
" float4x4 matWVP  : register(c0);                      "  
" float4x4 matW2L  : register(c4);                      "  
" float4   inColor : register(c8);                      "
"                                                       "
" struct VS_IN                                          "
" {                                                     "
"     float4 ObjPos   : POSITION;                       "  // Object space position 
"     float4 ObjNormal: NORMAL;                         "  // Object space normal
" };                                                    "
"                                                       "
" struct VS_OUT                                         "
" {                                                     "
"     float4 ProjPos  : POSITION;                       "  // Projected space position 
"     float4 Color    : COLOR;                          "
" };                                                    "
"                                                       "
" VS_OUT main( VS_IN In )                               "
" {                                                     "
"     VS_OUT Out;                                       "
"     Out.ProjPos = mul( matWVP, In.ObjPos );           "  // Transform vertex into

"     float4 Ambient = float4(0.5,0.5,0.5,1);           "

"     float4 Diffuse = float4(0.5,0.5,0.5,1);           "
"     float4 LightPos = float4(100,100,100,1);          "
"     LightPos = mul(matW2L,LightPos);                  "
"     float4 LightDir = normalize(LightPos-In.ObjPos);  "
"     float NdotL = max(dot(In.ObjNormal,LightDir),0);  "
"     Ambient += Diffuse * NdotL;                       "

"     Out.Color.rgb = inColor.rgb * Ambient.rgb;        "
"     Out.Color.a   = inColor.a;                        "
"     return Out;                                       "
" }                                                     ";

const char* g_PixelShaderProgram_pn = 
" struct PS_IN                                          "
" {                                                     "
"     float4 Color : COLOR;                             "  // Interpolated color from
" };                                                    "  // the vertex shader
"                                                       "  
" float4 main( PS_IN In ) : COLOR                       "  
" {                                                     "  
"     return In.Color;                                  "  // Output color
" }                                                     "; 

//==============================================================================
// graphics_x360
//==============================================================================

graphics_x360::graphics_x360()
:   m_pD3D      ( NULL ),
    m_pd3dDevice( NULL )
{
}

//==============================================================================
// ~graphics_x360
//==============================================================================

graphics_x360::~graphics_x360( void )
{
}

//==============================================================================
// Init
//==============================================================================

void graphics_x360::Init( void )
{
    // Create the D3D object.
    m_pD3D = Direct3DCreate9( D3D_SDK_VERSION );

    // Setup to create device.
    ZeroMemory( &m_VideoMode, sizeof( m_VideoMode ) );
    XGetVideoMode( &m_VideoMode );
    BOOL bEnable720p = (m_VideoMode.dwDisplayHeight >= 720) ? TRUE : FALSE;

    // Setup to create device.
    ZeroMemory( &m_d3dpp, sizeof( m_d3dpp ) );
    m_d3dpp.BackBufferWidth        = bEnable720p ? 1280 : 640;
    m_d3dpp.BackBufferHeight       = bEnable720p ? 720  : 480;
    m_d3dpp.BackBufferFormat       = D3DFMT_A8R8G8B8;
    m_d3dpp.MultiSampleType        = bEnable720p ? D3DMULTISAMPLE_NONE : D3DMULTISAMPLE_4_SAMPLES;
    m_d3dpp.MultiSampleQuality     = 0;
    m_d3dpp.BackBufferCount        = 1;
    m_d3dpp.EnableAutoDepthStencil = TRUE;
    m_d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
    m_d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
    m_d3dpp.PresentationInterval   = D3DPRESENT_INTERVAL_ONE;

    // Create the D3D Device.
    m_pD3D->CreateDevice( 0, D3DDEVTYPE_HAL, NULL, D3DCREATE_HARDWARE_VERTEXPROCESSING, &m_d3dpp, &m_pd3dDevice );

    // Buffers to hold compiled shaders and possible error messages.
    ID3DXBuffer* pShaderCode = NULL;
    ID3DXBuffer* pErrorMsg = NULL;

    // Compile vertex shader.
    HRESULT hr = D3DXCompileShader( g_VertexShaderProgram_pc, (UINT)strlen( g_VertexShaderProgram_pc ),
                                    NULL, NULL, "main", "vs_2_0", 0,
                                    &pShaderCode, &pErrorMsg, NULL );
    if( FAILED(hr) )
    {
        OutputDebugStringA( pErrorMsg ? (CHAR*)pErrorMsg->GetBufferPointer() : "" );
        exit(1);
    }
    
    // Create vertex shader.
    m_pd3dDevice->CreateVertexShader( (DWORD*)pShaderCode->GetBufferPointer(), 
                                      &m_VertexShader_pc );

    // Shader code is no longer required.
    pShaderCode->Release();
    pShaderCode = NULL;

    // Compile pixel shader.
    hr = D3DXCompileShader( g_PixelShaderProgram_pc, (UINT)strlen( g_PixelShaderProgram_pc ),
                            NULL, NULL, "main", "ps_2_0", 0,
                            &pShaderCode, &pErrorMsg, NULL );
    if( FAILED(hr) )
    {
        OutputDebugStringA( pErrorMsg ? (CHAR*)pErrorMsg->GetBufferPointer() : "" );
        exit(1);
    }

    // Create pixel shader.
    m_pd3dDevice->CreatePixelShader( (DWORD*)pShaderCode->GetBufferPointer(),
                                     &m_PixelShader_pc );
    
    // Shader code no longer required.
    pShaderCode->Release();
    pShaderCode = NULL;

    // Define the vertex elements.
    D3DVERTEXELEMENT9 VertexElements_pc[3] =
    {
        { 0,  0, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
        { 0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,    0 },
        D3DDECL_END()
    };

    // Create a vertex declaration from the element descriptions.
    m_pd3dDevice->CreateVertexDeclaration( VertexElements_pc, &m_VertexDecl_pc );

    // Compile vertex shader.
    hr = D3DXCompileShader( g_VertexShaderProgram_pn, (UINT)strlen( g_VertexShaderProgram_pn ),
                            NULL, NULL, "main", "vs_2_0", 0,
                            &pShaderCode, &pErrorMsg, NULL );
    if( FAILED(hr) )
    {
        OutputDebugStringA( pErrorMsg ? (CHAR*)pErrorMsg->GetBufferPointer() : "" );
        exit(1);
    }
    
    // Create vertex shader.
    m_pd3dDevice->CreateVertexShader( (DWORD*)pShaderCode->GetBufferPointer(), 
                                      &m_VertexShader_pn );

    // Shader code is no longer required.
    pShaderCode->Release();
    pShaderCode = NULL;

    // Compile pixel shader.
    hr = D3DXCompileShader( g_PixelShaderProgram_pn, (UINT)strlen( g_PixelShaderProgram_pn ),
                            NULL, NULL, "main", "ps_2_0", 0,
                            &pShaderCode, &pErrorMsg, NULL );
    if( FAILED(hr) )
    {
        OutputDebugStringA( pErrorMsg ? (CHAR*)pErrorMsg->GetBufferPointer() : "" );
        exit(1);
    }

    // Create pixel shader.
    m_pd3dDevice->CreatePixelShader( (DWORD*)pShaderCode->GetBufferPointer(),
                                     &m_PixelShader_pn );
    
    // Shader code no longer required.
    pShaderCode->Release();
    pShaderCode = NULL;

    // Define the vertex elements.
    D3DVERTEXELEMENT9 VertexElements_pn[3] =
    {
        { 0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
        { 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0 },
        D3DDECL_END()
    };

    // Create a vertex declaration from the element descriptions.
    m_pd3dDevice->CreateVertexDeclaration( VertexElements_pn, &m_VertexDecl_pn );

    // Set culling.
    m_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );

    // Setup Alpha blend equation
    m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND , D3DBLEND_SRCALPHA );
    m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

    // Setup depth bias to keep the lines in front of the triangles.
    float Bias      = 0.00f;
    float BiasScale = 1.00f;
    m_pd3dDevice->SetRenderState( D3DRS_DEPTHBIAS          , *(DWORD*)&Bias      );
    m_pd3dDevice->SetRenderState( D3DRS_SLOPESCALEDEPTHBIAS, *(DWORD*)&BiasScale );

    // Initialize the common shapes.
    InitShapes();
}

//==============================================================================
// Kill
//==============================================================================

void graphics_x360::Kill( void )
{
}

//==============================================================================
// WindowResized
//==============================================================================

void graphics_x360::WindowResized( void )
{
}

//==============================================================================
// BeginFrame
//==============================================================================

void graphics_x360::BeginFrame( void )
{
    DEJA_CONTEXT( "graphics_x360::BeginFrame" );

    // Setup view.
    SetView( m_View );

    // Setup the shaders and vertex declaration.
    m_pd3dDevice->SetVertexShader     ( m_VertexShader_pc );
    m_pd3dDevice->SetPixelShader      ( m_PixelShader_pc  );
    m_pd3dDevice->SetVertexDeclaration( m_VertexDecl_pc   );
}

//==============================================================================
// EndFrame
//==============================================================================

void graphics_x360::EndFrame( void )
{
    DEJA_CONTEXT( "graphics_x360::EndFrame" );

    // Present the back buffer.
    m_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}

//==============================================================================
// GetViewport
//==============================================================================

void graphics_x360::GetViewport( int& l, int& t, int& w, int& h )
{
    l = 0;
    t = 0;
    w = m_d3dpp.BackBufferWidth;
    h = m_d3dpp.BackBufferHeight;
}

//==============================================================================
// SetView
//==============================================================================

void graphics_x360::SetView( const view& View )
{
    m_View = View;
    matrix4 W2V = View.GetW2V();
    W2V.RotateY( R_180 );     // Convert Deja space to X360 space.

    matrix4 L2V = W2V * m_L2W;

    // Prevent a divide by zero if the window is too small.
    float XFOV = View.GetXFOV();

    if( View.GetMode() == view::PERSPECTIVE )
    {
        // Reset the perspective transformation.
        float Near, Far;
        View.GetZLimits( Near, Far );

        FLOAT fAspectRatio = (FLOAT)m_d3dpp.BackBufferWidth / (FLOAT)m_d3dpp.BackBufferHeight;
        XMMATRIX matProj = XMMatrixPerspectiveFovRH( XFOV/fAspectRatio, fAspectRatio, Near, Far );
        m_WVP = *(XMMATRIX*)&W2V * matProj;
        XMMATRIX LWVP = *(XMMATRIX*)&m_L2W * m_WVP;
        m_pd3dDevice->SetVertexShaderConstantF( 0, (FLOAT*)&LWVP, 4 );
    }
    else
    {
        float Near, Far;
        View.GetZLimits( Near, Far );
        float XWOV = View.GetXWOV();

        FLOAT fAspectRatio = (FLOAT)m_d3dpp.BackBufferWidth / (FLOAT)m_d3dpp.BackBufferHeight;
        XMMATRIX matProj = XMMatrixOrthographicOffCenterRH( -(XWOV*0.5f) * fAspectRatio, (XWOV*0.5f) * fAspectRatio, 
                                                            -(XWOV*0.5f), (XWOV*0.5f), 
                                                            Near, Far );
        m_WVP = *(XMMATRIX*)&W2V * matProj;
        XMMATRIX LWVP = *(XMMATRIX*)&m_L2W * m_WVP;
        m_pd3dDevice->SetVertexShaderConstantF( 0, (FLOAT*)&LWVP, 4 );
    }
}

//==============================================================================
// SetL2W
//==============================================================================

void graphics_x360::SetL2W( const matrix4& L2W )
{
    m_L2W = L2W;
    XMMATRIX LWVP = *(XMMATRIX*)&m_L2W * m_WVP;
    m_pd3dDevice->SetVertexShaderConstantF( 0, (FLOAT*)&LWVP , 4 );

    XMVECTOR Det;
    Det.x = Det.y = Det.z = Det.w = 1.0f;
    XMMATRIX t = XMMatrixInverse( &Det, *(XMMATRIX*)&m_L2W );
    m_pd3dDevice->SetVertexShaderConstantF( 4, (FLOAT*)&t, 4 );
}

//==============================================================================
// EnableZWrite
//==============================================================================

void graphics_x360::EnableZWrite( void )
{
    if( m_ZWrite )
        return;

    m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
    m_ZWrite = true;
}

//==============================================================================
// DisableZWrite
//==============================================================================

void graphics_x360::DisableZWrite( void )
{
    if( !m_ZWrite )
        return;

    m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
    m_ZWrite = false;
}

//==============================================================================
// EnableAlpha
//==============================================================================

void graphics_x360::EnableAlpha( void )
{
    if( m_Alpha )
        return;

    m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    m_Alpha = true;
}

//==============================================================================
// DisableAlpha
//==============================================================================

void graphics_x360::DisableAlpha( void )
{
    if( !m_Alpha )
        return;

    m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
    m_Alpha = false;
}

//==============================================================================
// EnableLighting
//==============================================================================

void graphics_x360::EnableLighting( void )
{
    if( m_Lighting )
        return;

    m_Lighting = true;
}

//==============================================================================
// DisableLighting
//==============================================================================

void graphics_x360::DisableLighting( void )
{
    if( !m_Lighting )
        return;

    m_Lighting = false;
}

//==============================================================================
// GetAmbientLight
//==============================================================================

color graphics_x360::GetAmbientLight( void )
{
    return m_Ambient;
}

//==============================================================================
// SetAmbientLight
//==============================================================================

void graphics_x360::SetAmbientLight( const color& Color )
{
    m_Ambient = Color;
}

//==============================================================================
// Clear
//==============================================================================

void graphics_x360::Clear( const color& Color )
{
    // Clear frame buffer
    color t = Color;
    DWORD c = (t.A<<24) | (t.R<<16) | (t.G<<8) | t.B;
    m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL, 
                         c, 1.0f, 0L );
}

//==============================================================================
// SetColor
//==============================================================================

void graphics_x360::SetColor( const color& )
{
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

void graphics_x360::DrawGrid( const vector3& Origin,
                              const vector3& Normal,
                              const vector3& Axis1,
                              const color&   Color,
                                    int      Steps )
{
    DEJA_CONTEXT( "graphics_x360::DrawGrid" );

    // Make sure we have enough storage.

    int nLines = ((Steps * 2) + 1) * 2;
    int nVerts = nLines * 2;

    if( s_nVerts < nVerts )
    {
        free( s_pColorVert );

        s_pColorVert = (vertex_pc*)malloc( nVerts * sizeof(vertex_pc) );
        s_nVerts = nVerts;
    }

    // Create the second axis vector.

    vector3 Axis2 = Cross( Axis1, Normal );
    Axis2.Normalize();
    Axis2 = Axis2 * Axis1.GetLength();

    // Additional preparations.

    vertex_pc* pVert = s_pColorVert;

    vector3  V1     = Axis1 * (float)Steps;
    vector3  V2     = Axis2 * (float)Steps;

    // Create the lines.
    DWORD c = Color.A<<24 | Color.B<<16 | Color.G<<8 | Color.R;
    for( int i = -Steps; i <= +Steps; i++ )
    {
        float  S  = (float)i;

        pVert->Position = Origin + V1 + (Axis2 * S);
        pVert->Color = c;
        pVert++;

        pVert->Position = Origin - V1 + (Axis2 * S);
        pVert->Color = c;
        pVert++;

        pVert->Position = Origin + V2 + (Axis1 * S);
        pVert->Color = c;
        pVert++;

        pVert->Position = Origin - V2 + (Axis1 * S);
        pVert->Color = c;
        pVert++;
    }

    if( !m_Alpha )
        m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );

    // Setup the shaders and vertex declaration.
    m_pd3dDevice->SetVertexShader     ( m_VertexShader_pc );
    m_pd3dDevice->SetPixelShader      ( m_PixelShader_pc  );
    m_pd3dDevice->SetVertexDeclaration( m_VertexDecl_pc   );

    // Render
    m_pd3dDevice->DrawPrimitiveUP( D3DPT_LINELIST, nVerts/2, s_pColorVert, sizeof(vertex_pc) );

    if( !m_Alpha )
        m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
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

void graphics_x360::DrawTriGrid( const vector3& Origin,
                                 const vector3& Normal,
                                 const vector3& Axis1,
                                 const color&   Color,
                                       int      Steps )
{
    DEJA_CONTEXT( "graphics_x360::DrawTriGrid" );

    // Make sure we have enough storage.

    int nLines = ((Steps * 2) + 1) * 3;
    int nVerts = nLines * 2;

    if( s_nVerts < nVerts )
    {
        free( s_pColorVert  );

        s_pColorVert  = (vertex_pc*)malloc( nVerts * sizeof(vertex_pc) );
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
    
    vertex_pc* pVert = s_pColorVert;

    vector3  V1     = Axis1 * (float)Steps;
    vector3  V2     = Axis2 * (float)Steps;
    vector3  V3     = Axis3 * (float)Steps;

    // Create the lines.
    DWORD c = Color.A<<24 | Color.B<<16 | Color.G<<8 | Color.R;
    for( int i = 0; i < Steps; i++ )
    {
        float  S  = (float)i;

        pVert->Position = Origin + V1 + (Axis3 * S);
        pVert->Color = c;
        pVert++;

        pVert->Position = Origin - V3 - (Axis1 * S);
        pVert->Color = c;
        pVert++;

        pVert->Position = Origin + V2 + (Axis1 * S);
        pVert->Color = c;
        pVert++;

        pVert->Position = Origin - V1 - (Axis2 * S);
        pVert->Color = c;
        pVert++;

        pVert->Position = Origin + V3 + (Axis2 * S);
        pVert->Color = c;
        pVert++;

        pVert->Position = Origin - V2 - (Axis3 * S);
        pVert->Color = c;
        pVert++;

        pVert->Position = Origin + V3 + (Axis1 * S);
        pVert->Color = c;
        pVert++;

        pVert->Position = Origin - V1 - (Axis3 * S);
        pVert->Color = c;
        pVert++;

        pVert->Position = Origin + V1 + (Axis2 * S);
        pVert->Color = c;
        pVert++;

        pVert->Position = Origin - V2 - (Axis1 * S);
        pVert->Color = c;
        pVert++;

        pVert->Position = Origin + V2 + (Axis3 * S);
        pVert->Color = c;
        pVert++;

        pVert->Position = Origin - V3 - (Axis2 * S);
        pVert->Color = c;
        pVert++;
    }

    // Last 3 lines.

    {
        pVert->Position = Origin + V1;
        pVert->Color = c;
        pVert++;

        pVert->Position = Origin - V1;
        pVert->Color = c;
        pVert++;

        pVert->Position = Origin + V2;
        pVert->Color = c;
        pVert++;

        pVert->Position = Origin - V2;
        pVert->Color = c;
        pVert++;

        pVert->Position = Origin + V3;
        pVert->Color = c;
        pVert++;

        pVert->Position = Origin - V3;
        pVert->Color = c;
        pVert++;
    }

    if( !m_Alpha )
        m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );

    // Setup the shaders and vertex declaration.
    m_pd3dDevice->SetVertexShader     ( m_VertexShader_pc );
    m_pd3dDevice->SetPixelShader      ( m_PixelShader_pc  );
    m_pd3dDevice->SetVertexDeclaration( m_VertexDecl_pc   );

    // Render.
    m_pd3dDevice->DrawPrimitiveUP( D3DPT_LINELIST, nVerts/2, s_pColorVert, sizeof(vertex_pc) );

    if( !m_Alpha )
        m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
}                                       

//==============================================================================
// DrawLine
//==============================================================================

void graphics_x360::DrawLine( const vector3&   v1, 
                              const vector3&   v2, 
                              const color&     Color )
{
    DWORD c = Color.A<<24 | Color.B<<16 | Color.G<<8 | Color.R;

    vertex_pc v[2];
    v[0].Position = v1;
    v[0].Color    = c;
    v[1].Position = v2;
    v[1].Color    = c;

    // Setup the shaders and vertex declaration.
    m_pd3dDevice->SetVertexShader     ( m_VertexShader_pc );
    m_pd3dDevice->SetPixelShader      ( m_PixelShader_pc  );
    m_pd3dDevice->SetVertexDeclaration( m_VertexDecl_pc   );

    // Render.
    m_pd3dDevice->DrawPrimitiveUP( D3DPT_LINELIST, 1, v, sizeof(vertex_pc) );
}

//==============================================================================
// DrawTriangles
//==============================================================================

void graphics_x360::DrawTriangles( int nVerts, const vertex_pn* pVerts, const color& Color )
{
    // Setup the shaders and vertex declaration.
    float c[4] = { Color.R / 255.0f,
                   Color.G / 255.0f,
                   Color.B / 255.0f,
                   Color.A / 255.0f };
    m_pd3dDevice->SetVertexShaderConstantF( 8, (float*)&c, 1 );
    m_pd3dDevice->SetVertexShader     ( m_VertexShader_pn );
    m_pd3dDevice->SetPixelShader      ( m_PixelShader_pn  );
    m_pd3dDevice->SetVertexDeclaration( m_VertexDecl_pn   );

    // Render.
    m_pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLELIST, nVerts/3, pVerts, sizeof(vertex_pn) );
}

//==============================================================================
// DrawTriangles
//==============================================================================

void graphics_x360::DrawTriangles( int nVerts, const vertex_pc* pVerts )
{
    // Setup the shaders and vertex declaration.
    m_pd3dDevice->SetVertexShader     ( m_VertexShader_pc );
    m_pd3dDevice->SetPixelShader      ( m_PixelShader_pc  );
    m_pd3dDevice->SetVertexDeclaration( m_VertexDecl_pc   );

    // Render.
    m_pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLELIST, nVerts/3, pVerts, sizeof(vertex_pc) );
}

//==============================================================================
// DrawLines
//==============================================================================

void graphics_x360::DrawLines( int nVerts, const vertex_pc* pVerts )
{
    // Setup the shaders and vertex declaration.
    m_pd3dDevice->SetVertexShader     ( m_VertexShader_pc );
    m_pd3dDevice->SetPixelShader      ( m_PixelShader_pc  );
    m_pd3dDevice->SetVertexDeclaration( m_VertexDecl_pc   );

    // Render.
    m_pd3dDevice->DrawPrimitiveUP( D3DPT_LINELIST, nVerts/2, pVerts, sizeof(vertex_pc) );
}

//==============================================================================
// DrawText
//==============================================================================

void graphics_x360::DrawText( const char* pString, int x, int y, const color& Color )
{
/*
    // Generate the bitmap font display lists
    if( s_FontDisplayListBase == -1 )
    {
        s_FontDisplayListBase = glGenLists( 256-32 );
        HDC hDC = GetDC( m_hWnd );

        // Create a new font
        LOGFONTW LogFont;
        memset( &LogFont, 0, sizeof(LogFont) );
        int LogPixelsY = GetDeviceCaps( hDC, LOGPIXELSY );
        wcscpy_s( LogFont.lfFaceName, 32, L"Tahoma" );
        LogFont.lfHeight         = -MulDiv( 28, LogPixelsY, 72 );
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

    // Setup the matrix for 2d rendering
    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    glOrtho( 0.0f, m_w, 0.0f, m_h, -1, 1 );
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();

    // Setup the display list base
    glListBase( s_FontDisplayListBase - 32 );

    // Disable lighting
    glDisable( GL_LIGHTING );

    // Set the color of the text
    glColor4ubv( (GLubyte*)&Color );

    // Set the start position, use glBitmap to avoid the culling problems with glRasterPos.
    glRasterPos2f( 0.0f, 0.0f );
    glBitmap( 0, 0, 0.0f, 0.0f, (GLfloat)x, (GLfloat)(m_h-y), NULL );

    // Draw the characters
    int StringLen = (int)strlen( pString );
    glCallLists( StringLen, GL_UNSIGNED_BYTE, pString );

    // Restore matrices
    glPopMatrix();
    glMatrixMode( GL_PROJECTION );
    glPopMatrix();
    glMatrixMode( GL_MODELVIEW );

    if( m_Lighting )
        glEnable( GL_LIGHTING );
*/
}

//==============================================================================
