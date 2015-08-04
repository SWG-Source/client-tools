//==============================================================================
//
//  Explosion.h
//
//==============================================================================
//  Copyright (C) DejaTools, LLC.  All rights reserved.
//==============================================================================

#ifndef EXPLOSION_H
#define EXPLOSION_H

//==============================================================================
//  DEFINES
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "../TinyEngine/color.h"
#include "../TinyEngine/obj_mgr.h"
#include "../TinyEngine/mathdefs.h"

//==============================================================================
//  TYPES
//==============================================================================

class explosion : public object
{
public:
                    explosion       ( void );
                   ~explosion       ( void );

virtual void        OnLogic         ( float Time );
virtual void        OnRender        ( void );

virtual void        OnDejaObjInit   ( void );
virtual void        OnDejaPost      ( void );

        void        SpawnMissile    ( const vector3& Position,
                                      const vector3& Velocity,
                                      float          Size,
                                      const color&   Color );

        void        SpawnBoid       ( const vector3& Position,
                                      const vector3& Velocity,
                                      float          Size,
                                      const color&   Color );

        void        SpawnMortar     ( const vector3& Position,
                                      const vector3& Velocity,
                                      float          Size,
                                      const color&   Color );

protected:
        bool        m_Smooth;
        color       m_Color;
        float       m_Size;
        float       m_Growth;
        float       m_GrowthDecay;
        float       m_Timer;
        float       m_AlphaTime;
        vector3     m_Velocity;
        vector3     m_Spin;
        vector3     m_Rotation;

friend  void        DejaDescriptor  ( const explosion& );
};

//==============================================================================
//  FUNCTIONS
//==============================================================================

void SystemPrep_Explosion( void );
void DejaDescriptor( const explosion& );

//==============================================================================
//  STORAGE
//==============================================================================

extern type_data ExplosionTypeData;

//==============================================================================
#endif // ifndef EXPLOSION_H
//==============================================================================
