//==============================================================================
// view.cpp
//==============================================================================
//  Copyright (C) DejaTools, LLC.  All rights reserved.
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "stdafx.h"
#include "view.h"

//==============================================================================
//  DEFINES   
//==============================================================================

//==============================================================================
//  FUNCTIONS
//==============================================================================

view::view( void )
{
    m_Position( 0.0f, 0.0f, 0.0f );
    m_Orient.Identity();
    m_VPLeft   =   50;
    m_VPTop    =   50;
    m_VPWidth  =  100;
    m_VPHeight =  100;
    m_ZNear    =    0.1f;
    m_ZFar     = 1000.0f;
    m_XFOV     = (radian)(60 * M_PI / 180);
    m_XWOV     =  100.0f;
    m_Mode     = PERSPECTIVE;
}

//==============================================================================

view::view( const view& View )
{
    m_Position = View.m_Position;
    m_Orient   = View.m_Orient;
    m_VPLeft   = View.m_VPLeft;
    m_VPTop    = View.m_VPTop;
    m_VPWidth  = View.m_VPWidth;
    m_VPHeight = View.m_VPHeight;
    m_ZNear    = View.m_ZNear;
    m_ZFar     = View.m_ZFar;
    m_XFOV     = View.m_XFOV;         
    m_XWOV     = View.m_XWOV;
    m_Mode     = View.m_Mode;
}

//==============================================================================

view::~view( void )
{
}

//==============================================================================

void view::SetViewport( int Left, int Top, int Width, int Height )
{
    m_VPLeft   = Left;
    m_VPTop    = Top;
    m_VPWidth  = Width;
    m_VPHeight = Height;
    m_XWOV     = (float)Width;
}

//==============================================================================

void view::SetXFOV( radian XFOV )
{
    m_XFOV = XFOV;
}

//==============================================================================

void view::SetXWOV( float XWOV )
{
    m_XWOV = XWOV;
}

//==============================================================================

void view::SetZLimits( float ZNear, float ZFar )
{
    m_ZNear = ZNear;
    m_ZFar  = ZFar;
}

//==============================================================================

void view::SetPosition( const vector3& Position )
{
    m_Position = Position;
}

//==============================================================================

void view::SetOrientation( const matrix4& Orientation )
{
    m_Orient = Orientation;

    m_Orient(3,0) = 0.0f;
    m_Orient(3,1) = 0.0f;
    m_Orient(3,2) = 0.0f;
    m_Orient(3,3) = 1.0f;
    m_Orient(2,3) = 0.0f;
    m_Orient(1,3) = 0.0f;
    m_Orient(0,3) = 0.0f;
}

//==============================================================================

void view::Translate( const vector3& Translation, system System )
{   
    switch( System )
    {
    case WORLD: m_Position += Translation;                  break;
    case VIEW:  m_Position += (m_Orient * Translation);     break;
    }
}

//==============================================================================

void view::RotateX( radian Rx, system System )
{
    switch( System )
    {
    case WORLD: m_Orient.RotateX( Rx );         break;                        
    case VIEW:  m_Orient.PreRotateX( Rx );      break;
    }
}

//==============================================================================

void view::RotateY( radian Ry, system System )
{
    switch( System )
    {
    case WORLD: m_Orient.RotateY( Ry );         break;                        
    case VIEW:  m_Orient.PreRotateY( Ry );      break;
    }
}

//==============================================================================

void view::RotateZ( radian Rz, system System )
{
    switch( System )
    {
    case WORLD: m_Orient.RotateZ( Rz );         break;                        
    case VIEW:  m_Orient.PreRotateZ( Rz );      break;                        
    }
}

//==============================================================================

void view::SetMode( mode Mode )
{
    m_Mode = Mode;
}

//==============================================================================

vector3 view::GetViewX( void ) const
{
    return( vector3( m_Orient( 0, 0 ),
                     m_Orient( 0, 1 ),
                     m_Orient( 0, 2 ) ) );
}

//==============================================================================

vector3 view::GetViewY( void ) const
{
    return( vector3( m_Orient( 1, 0 ),
                     m_Orient( 1, 1 ),
                     m_Orient( 1, 2 ) ) );
}

//==============================================================================

vector3 view::GetViewZ( void ) const
{
    return( vector3( m_Orient( 2, 0 ),
                     m_Orient( 2, 1 ),
                     m_Orient( 2, 2 ) ) );
}

//==============================================================================

void view::GetZLimits( float& ZNear, float& ZFar ) const
{
    ZNear = m_ZNear;
    ZFar  = m_ZFar;
}

//==============================================================================

void view::GetViewport( int& Left, int& Top, int& Width, int& Height ) const
{
    Left   = m_VPLeft;
    Top    = m_VPTop;
    Width  = m_VPWidth;
    Height = m_VPHeight;
}

//==============================================================================

radian view::GetXFOV( void ) const
{
    return( m_XFOV );
}

//==============================================================================

float view::GetXWOV( void ) const
{
    return( m_XWOV );
}

//==============================================================================

view::mode view::GetMode( void ) const
{
    return( m_Mode );
}

//==============================================================================

matrix4 view::GetW2V( void ) const
{
    matrix4 W2V = m_Orient;
    W2V.Transpose();
    W2V.PreTranslate( -m_Position );
    return( W2V );
}

//==============================================================================

matrix4 view::GetV2W( void ) const
{
    matrix4 V2W = m_Orient;
    V2W.SetTranslation( m_Position );
    return( V2W );
}

//==============================================================================

void view::LookTo( const vector3& To, 
                         system   System )
{
    vector3 Point;

    switch( System )
    {
    case WORLD: Point = To;                 break;
    case VIEW:  Point = ApplyV2W( To );     break;
    }

    vector3 Ray = Point - m_Position;
    radian  Pitch;
    radian  Yaw;

    Ray.GetPitchYaw( Pitch, Yaw );

    m_Orient.Identity();
    m_Orient.RotateX( Pitch );
    m_Orient.RotateY( Yaw   );
}

//==============================================================================

void view::LookFromTo( const vector3& From, 
                       const vector3& To, 
                             system   System )
{
    vector3 Point;

    switch( System )
    {
    case WORLD: Point = From;               break;
    case VIEW:  Point = ApplyV2W( From );   break;
    }

    SetPosition( Point );
    LookTo( To, System );
}

//==============================================================================

void view::PixelToRay( vector3& RayStart, 
                       vector3& RayDir, 
                       int      PixelX, 
                       int      PixelY, 
                       system   System ) const
{   
    if( m_Mode == PERSPECTIVE )
    {
        vector3 v;

        v.x = (m_VPWidth  * 0.5f) - (PixelX - m_VPLeft);
        v.y = (m_VPHeight * 0.5f) - (PixelY - m_VPTop);
        v.z = (m_VPWidth  * 0.5f) / tanf( m_XFOV * 0.5f );

        // Vector v points from the eye to the pixel in view space.

        switch( System )
        {
        case WORLD:
            v = m_Orient * v;
            v.Normalize();
            RayDir   = v;
            RayStart = m_Position;
            break;
        case VIEW:
            v.Normalize();
            RayDir = v;
            RayStart( 0, 0, 0 );
            break;
        }
    }
    else
    {   
        vector3 v;

        float Ratio = (float)m_VPHeight / (float)m_VPWidth;

        v.x = 1.0f - ((PixelX - m_VPLeft) / (m_VPWidth  * 0.5f));
        v.y = 1.0f - ((PixelY - m_VPTop ) / (m_VPHeight * 0.5f));
        v.z = 0.0f;

        v.x *= (m_XWOV * 0.5f);
        v.y *= (m_XWOV * 0.5f * Ratio);

        // Vector v is on the eye plane in view space.

        switch( System )
        {
        case WORLD:
            RayStart = ApplyV2W( v );
            RayDir   = m_Orient * vector3(0,0,1); // TODO - Optimize this.
            break;
        case VIEW:
            RayStart = v;
            RayDir( 0, 0, 1 );
            break;
        }
    }
}

//==============================================================================

void view::PointToPixel( const vector3& Point,
                               vector3& Pixel,
                               system   System ) const
{
    vector3 v = Point;

    if( System == WORLD )
    {
        v = ApplyW2V( v );
    }

    float z = v.z;
    if( z < 0.001f )
    {
        if( z > -0.001f )   z = 0.001f;
        if( z <  0.000f )   z = -z;
    }

    if( m_Mode == PERSPECTIVE )
    {
        float f  = (m_VPWidth * 0.5f) / tanf( m_XFOV * 0.5f );

        Pixel.x = (m_VPLeft + (m_VPWidth  * 0.5f)) - (f * (v.x / z));
        Pixel.y = (m_VPTop  + (m_VPHeight * 0.5f)) - (f * (v.y / z));
        Pixel.z = v.z;
    }
    else
    {
        float Ratio = (float)m_VPWidth / (float)m_VPHeight; 

        v.y *= Ratio;

        Pixel.x = (((v.x / m_XWOV) + 0.5f) * m_VPWidth ) + m_VPLeft;
        Pixel.y = (((v.y / m_XWOV) + 0.5f) * m_VPHeight) + m_VPTop;
        Pixel.z = v.z;
    }
}

//==============================================================================

vector3 view::ApplyV2W( const vector3& v ) const
{
    matrix4 V2W = GetV2W();
    return( V2W * v );
}

//==============================================================================

vector3 view::ApplyW2V( const vector3& v ) const
{
    matrix4 W2V = GetW2V();
    return( W2V * v );
}

//==============================================================================

vector3 view::GetPosition( void ) const
{
    return( m_Position );
}

//==============================================================================

matrix4 view::GetOrientation( void ) const
{
    return( m_Orient );
}

//==============================================================================
