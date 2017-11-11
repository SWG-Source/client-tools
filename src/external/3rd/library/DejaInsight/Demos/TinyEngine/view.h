//==============================================================================
//  view.h
//==============================================================================
//  Copyright (C) DejaTools, LLC.  All rights reserved.
//==============================================================================

#ifndef VIEW_H
#define VIEW_H

//==============================================================================
//  INCLUDES
//==============================================================================

#include "matrix4.h"

//==============================================================================
//  TYPES
//==============================================================================

class view
{

//------------------------------------------------------------------------------
//  Local Types
//------------------------------------------------------------------------------

public:
    // Coordinate systems.
    enum system
    { 
        WORLD, 
        VIEW 
    };

    // View mode.
    enum mode
    {
        PERSPECTIVE,
        PARALLEL,
    };

    // Visiblity results.
    enum visible
    {
        VISIBLE_NONE,     // Totally outside of volume.
        VISIBLE_FULL,     // Totally within volume.
        VISIBLE_PARTIAL   // Partially within volume; clipping may be needed.
    }; 

//------------------------------------------------------------------------------
//  Public Functions
//------------------------------------------------------------------------------

public:
                                
                view            ( void );
                view            ( const view& View );
               ~view            ( void );
     
void            SetViewport     ( int Left, int Top, int Width, int Height );
void            SetXFOV         ( radian XFOV );                                // Set angular field of view.
void            SetXWOV         ( float  XWOV );                                // Set parallel width of view.
void            SetZLimits      ( float ZNear, float ZFar );                    // Set near and far clip planes.
void            SetMode         ( mode Mode );

//------------------------------------------------------------------------------

void            SetPosition     ( const vector3& Position );                    // Set position of camera in world.
void            SetOrientation  ( const matrix4& Orientation );                 // Set orientation of camera in world.
void            Translate       ( const vector3& Translation, system System = WORLD );
void            RotateX         ( radian Rx, system System = WORLD );
void            RotateY         ( radian Ry, system System = WORLD );
void            RotateZ         ( radian Rz, system System = WORLD );

//------------------------------------------------------------------------------

vector3         GetPosition     ( void ) const;
matrix4         GetOrientation  ( void ) const;

void            GetViewport     ( int& Left, int& Top, int& Width, int& Height ) const;
radian          GetXFOV         ( void ) const;                                 // Set angular field of view.
float           GetXWOV         ( void ) const;                                 // Set parallel width of view.
void            GetZLimits      ( float& ZNear, float& ZFar ) const;            // Set near and far clip planes.
mode            GetMode         ( void ) const;

//------------------------------------------------------------------------------

vector3         GetViewX        ( void ) const;     // Camera "left".
vector3         GetViewY        ( void ) const;     // Camera "up".
vector3         GetViewZ        ( void ) const;     // Camera "line of sight".

matrix4         GetW2V          ( void ) const;     // Get the W2V matrix.
matrix4         GetV2W          ( void ) const;     // Get the V2W matrix.

//------------------------------------------------------------------------------

void            LookTo          ( const vector3& ToPoint,
                                        system   System = WORLD );
void            LookFromTo      ( const vector3& FromPoint, 
                                  const vector3& ToPoint, 
                                        system   System = WORLD );

//------------------------------------------------------------------------------

void            PixelToRay      ( vector3& RayStart,
                                  vector3& RayDir,
                                  int      PixelX, 
                                  int      PixelY, 
                                  system   System = WORLD ) const;

void            PointToPixel    ( const vector3& Point,
                                        vector3& Pixel,
                                        system   System = WORLD ) const;

//------------------------------------------------------------------------------

vector3         ApplyW2V        ( const vector3& v ) const;     // Convert world point to view  space.
vector3         ApplyV2W        ( const vector3& v ) const;     // Convert view  point to world space.

//------------------------------------------------------------------------------
//  Member Fields
//------------------------------------------------------------------------------

protected:

        vector3     m_Position;         // Position    of camera in World (V2W).
        matrix4     m_Orient;           // Orientation of camera in World (V2W).
                                        
        int         m_VPLeft;           // View port Left coordinate.
        int         m_VPTop;            // View port Top  coordinate.
        int         m_VPWidth;          // View port Width.
        int         m_VPHeight;         // View port Height.
                                        
        radian      m_XFOV;             // Field of view in x for perspective.
        float       m_XWOV;             // Width of view in x for parallel.
                                        
        float       m_ZNear;            // Near plane in (view) Z.
        float       m_ZFar;             // Far  plane in (view) Z.

        mode        m_Mode;             // PERSPECTIVE or PARALLEL.

};                

//==============================================================================
#endif // VIEW_H
//==============================================================================
