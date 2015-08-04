//==============================================================================
//
//  Missile.h
//
//==============================================================================
//  Copyright (C) DejaTools, LLC.  All rights reserved.
//==============================================================================

#ifndef MISSILE_H
#define MISSILE_H

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

class missile : public projectile
{
public:
    enum state
    {
        MISSILE_STATE_LAUNCH = PROJECTILE_STATE_MAX,
        MISSILE_STATE_TRACK,
        MISSILE_STATE_MISSED,
        MISSILE_STATE_OUT_OF_FUEL,
        MISSILE_STATE_EXPLODE
    };

public:
                    missile             ( void );
                   ~missile             ( void );

virtual void        OnLogic             ( float Time );
virtual void        OnRender            ( void );

virtual void        OnDejaObjInit       ( void );
virtual void        OnDejaPost          ( void );

        void        LogicLaunch         ( float Time );
        void        LogicTrack          ( float Time );
        void        LogicMissed         ( float Time );
        void        LogicOutOfFuel      ( float Time );

        void        Spawn               ( const vector3& Position,
                                          int            OwnerID,
                                          const vector3& Heading,
                                          int TargetID );

        void        UpdateTargetPos     ( void );

protected:
        float       m_Timer;
        float       m_Fuel;
        float       m_Speed;
        vector3     m_Heading;
        int         m_TargetID;
        vector3     m_TargetPos;
        vector3     m_TargetPredPos;
        bool        m_Locked;

friend  void        DejaDescriptor      ( const missile& );
};

//==============================================================================
//  FUNCTIONS
//==============================================================================

void SystemPrep_Missile( void );
void DejaDescriptor( const missile& );

//==============================================================================
//  STORAGE
//==============================================================================

extern type_data MissileTypeData;

//==============================================================================
#endif // ifndef MISSILE_H
//==============================================================================
