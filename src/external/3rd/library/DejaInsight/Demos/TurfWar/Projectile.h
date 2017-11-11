//==============================================================================
//
//  Projectile.h
//
//==============================================================================
//  Copyright (C) DejaTools, LLC.  All rights reserved.
//==============================================================================

#ifndef PROJECTILE_H
#define PROJECTILE_H

//==============================================================================
//  DEFINES
//==============================================================================

#define GRAVITY       40.0f

//==============================================================================
//  INCLUDES
//==============================================================================

#include "../TinyEngine/obj_mgr.h"
#include "../TinyEngine/mathdefs.h"
#include "../TinyEngine/color.h"

//==============================================================================
//  TYPES
//==============================================================================

class projectile : public object
{
public:
    enum state
    {
        PROJECTILE_STATE_NULL,
        PROJECTILE_STATE_FINALIZE,
        PROJECTILE_STATE_MAX
    };

                    projectile      ( void );
                   ~projectile      ( void );

virtual void        OnLogic         ( float Time );
virtual void        OnDejaPost      ( void );

        void        AddToTrail      ( const vector3& Point );
        void        RenderTrail     ( void );

protected:
        int         m_State;
        int         m_OwnerID;

        vector3     m_Trail[40];
        int         m_TrailCount;
        color       m_TrailColor;

friend  void        DejaDescriptor  ( const projectile& );
};

//==============================================================================
//  FUNCTIONS
//==============================================================================

void SystemPrep_Projectile( void );
void DejaDescriptor( const projectile& );

//==============================================================================
//  STORAGE
//==============================================================================

extern type_data ProjectileTypeData;

//==============================================================================
#endif // ifndef PROJECTILE_H
//==============================================================================
