//==============================================================================
//
//  TurfTeam.h
//
//==============================================================================
//  Copyright (C) DejaTools, LLC.  All rights reserved.
//==============================================================================

#ifndef TURF_TEAM_H
#define TURF_TEAM_H

//==============================================================================
//  DEFINES
//==============================================================================

#define MAGNETS_PER_TEAM  2
#define BOIDS_PER_MAGNET 10

//==============================================================================
//  INCLUDES
//==============================================================================

#include "../TinyEngine/obj_mgr.h"
#include "../TinyEngine/color.h"

//==============================================================================
//  TYPES
//==============================================================================

class turf_team : public object
{
public:
                    turf_team       ( void );
                   ~turf_team       ( void );

virtual void        OnDejaObjInit           ( void );
virtual void        OnLogic         ( float Time );
virtual void        OnRender        ( void );
virtual void        OnRemove        ( void );
virtual void        OnDejaPost      ( void );

        void        SetColor        ( color& Color );
        color       GetColor        ( void );
        
protected:

        int         m_MagnetID[ MAGNETS_PER_TEAM ];
        color       m_Color;

friend  void        DejaDescriptor  ( const turf_team& );
};

//==============================================================================
//  FUNCTIONS
//==============================================================================

void SystemPrep_TurfTeam( void );
void DejaDescriptor( const turf_team& );

//==============================================================================
//  STORAGE
//==============================================================================

extern type_data TurfTeamTypeData;

//==============================================================================
#endif // ifndef TURF_TEAM_H
//==============================================================================
