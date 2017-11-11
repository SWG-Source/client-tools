//==============================================================================
//
//  Turret.h
//
//==============================================================================
//  Copyright (C) DejaTools, LLC.  All rights reserved.
//==============================================================================

#ifndef TURRET_H
#define TURRET_H

//==============================================================================
//  DEFINES
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "../TinyEngine/obj_mgr.h"
#include "../TinyEngine/mathdefs.h"

//==============================================================================
//  TYPES
//==============================================================================

class turret : public object
{
public:
    enum type
    {
        TYPE_MISSILE,
        TYPE_MORTAR
    };

public:
                    turret          ( void );
                   ~turret          ( void );

virtual void        OnDejaObjInit           ( void );
virtual void        OnLogic         ( float Time );
        void        OnLogicLaser    ( float Time );
        void        OnLogicMissile  ( float Time );
        void        OnLogicMortar   ( float Time );
virtual void        OnKillCredit    ( void );
virtual void        OnRender        ( void );
virtual void        OnDejaPost      ( void );

        void        Spawn           ( const vector3&    Position,
                                      int               Type,
                                      float             MinRange,
                                      float             MaxRange );

protected:
        int         m_Type;
        float       m_MinRange;
        float       m_MaxRange;
        float       m_Yaw;
        float       m_Timer;
        int         m_ShotsFired;
        int         m_Kills;

friend  void        DejaDescriptor  ( const turret& );
};

//==============================================================================
//  FUNCTIONS
//==============================================================================

void SystemPrep_Turret( void );
void DejaDescriptor( const turret& );

//==============================================================================
//  STORAGE
//==============================================================================

extern type_data TurretTypeData;

//==============================================================================
#endif // ifndef TURRET_H
//==============================================================================
