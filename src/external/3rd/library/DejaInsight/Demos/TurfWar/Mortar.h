//==============================================================================
//
//  Mortar.h
//
//==============================================================================
//  Copyright (C) DejaTools, LLC.  All rights reserved.
//==============================================================================

#ifndef MORTAR_H
#define MORTAR_H

//==============================================================================
//  DEFINES
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "Projectile.h"
#include "../TinyEngine/obj_mgr.h"
#include "../TinyEngine/mathdefs.h"

//==============================================================================
//  TYPES
//==============================================================================

class mortar : public projectile
{
public:
    enum state
    {
        MORTAR_STATE_BALLISTIC = PROJECTILE_STATE_MAX
    };

public:
                    mortar              ( void );
                   ~mortar              ( void );

virtual void        OnLogic             ( float Time );
virtual void        OnRender            ( void );

virtual void        OnDejaObjInit       ( void );
virtual void        OnDejaPost          ( void );

        void        Spawn               ( const vector3& Position,
                                          int            OwnerID,
                                          const vector3& Velocity );

        void        LogicBallistic      ( float Time );
        void        Explode             ( void );


protected:
        vector3     m_Velocity;

friend  void        DejaDescriptor      ( const mortar& );
};

//==============================================================================
//  FUNCTIONS
//==============================================================================

void SystemPrep_Mortar( void );
void DejaDescriptor( const mortar& );

//==============================================================================
//  STORAGE
//==============================================================================

extern type_data MortarTypeData;

//==============================================================================
#endif // ifndef MORTAR_H
//==============================================================================
