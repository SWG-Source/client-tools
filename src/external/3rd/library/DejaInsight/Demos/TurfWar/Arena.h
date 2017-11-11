//==============================================================================
//
//  Arena.h
//
//==============================================================================
//  Copyright (C) DejaTools, LLC.  All rights reserved.
//==============================================================================

#ifndef ARENA_H
#define ARENA_H

//==============================================================================
//  DEFINES
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "../TinyEngine/obj_mgr.h"
#include "../TinyEngine/mathdefs.h"
#include "../TinyEngine/graphics.h"

//==============================================================================
//  cell
//==============================================================================

struct cell
{
    enum state
    {
        STATE_DELAY,
        STATE_MOVING
    };

    // Arena coordinates.
    int         m_i;
    int         m_j;

    // Rendering data.
    vector3     m_Base[7];
    vertex_pn   m_Triangles[18*3];
    vertex_pc   m_Lines[12*2];

    // Movement data.
    int         m_MoveState;
    float       m_MoveTimer;
    float       m_Radius;
    float       m_Height;
    float       m_TargetHeight;
    float       m_DeltaHeight;

    // Color data.
    color       m_SrcColor;
    color       m_DstColor;
    color       m_Color;
    float       m_ColorTimer;
    float       m_ColorAnimTime;
};

//==============================================================================
//  arena
//==============================================================================

class arena : public object
{
public:
                    arena           ( void );
virtual            ~arena           ( void );

        void        Init            ( int ArenaSize );

virtual void        OnLogic         ( float Time );
virtual void        OnRender        ( void );

virtual void        OnDejaObjInit   ( void );
virtual void        OnDejaPost      ( void );

        cell*       GetCell         ( int i, int j );
        cell*       GetCell         ( const vector3&    Point );

        bool        Intersects      ( const vector3&    Point,
                                      float             Radius );

protected:
friend  void        DejaDescriptor  ( const arena& );

protected:
        int         m_Size;
        int         m_GridSize;
        cell**      m_pGrid;
};

//==============================================================================
//  FUNCTIONS
//==============================================================================

void SystemPrep_Arena( void );
void DejaDescriptor( const arena& );

//==============================================================================
//  STORAGE
//==============================================================================

extern type_data ArenaTypeData;

//==============================================================================
#endif // ifndef ARENA_H
//==============================================================================

