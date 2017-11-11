//==============================================================================
//
//  Boid.h
//
//==============================================================================
//  Copyright (C) DejaTools, LLC.  All rights reserved.
//==============================================================================

#ifndef BOID_H
#define BOID_H

//==============================================================================
//  DEFINES
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "../TinyEngine/obj_mgr.h"
#include "../TinyEngine/mathdefs.h"
#include "../TinyEngine/color.h"

//==============================================================================
//  TYPES
//==============================================================================

class boid : public object
{
public:
                    boid            ( void );
virtual            ~boid            ( void );

virtual void        OnRemove        ( void );
virtual void        OnLogic         ( float Time );
virtual void        OnRender        ( void );
virtual bool        OnDamage        ( const vector3& Position, int Damage );

virtual void        OnDejaObjInit   ( void );
virtual void        OnDejaPost      ( void );

        void        Spawn           ( const vector3& Position, radian Yaw );
        void        SetVelocity     ( float Velocity );
        void        SetTeam         ( int   TeamID, color Color );
        void        SetMagnet       ( int   MagnetID );

        int         GetTeamID       ( void );
        int         GetMagnetID     ( void );
        
virtual vector3     GetPredictedPos ( float Time );

        void        Explode         ( const vector3& Velocity );

protected:
        vector3     m_Offset;
        float       m_Velocity;
        radian      m_Yaw;
        radian      m_DeltaYaw;
        color       m_Color;
        int         m_TeamID;
        int         m_MagnetID;
        int         m_Health;

friend  void        DejaDescriptor  ( const boid& );
};

//==============================================================================
//  FUNCTIONS
//==============================================================================

void SystemPrep_Boid( void );
void DejaDescriptor( const boid& );

//==============================================================================
//  STORAGE
//==============================================================================

extern type_data BoidTypeData;

//==============================================================================
#endif // ifndef BOID_H
//==============================================================================

