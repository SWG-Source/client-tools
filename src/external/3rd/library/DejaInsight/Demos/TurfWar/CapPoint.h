//==============================================================================
//
//  CapPoint.h
//
//==============================================================================
//  Copyright (C) DejaTools, LLC.  All rights reserved.
//==============================================================================

#ifndef CAP_POINT_H
#define CAP_POINT_H

//==============================================================================
//  DEFINES
//==============================================================================

#define CAP_POINT_MAX_TEAMS  3

//==============================================================================
//  INCLUDES
//==============================================================================

#include "../TinyEngine/obj_mgr.h"
#include "../TinyEngine/mathdefs.h"
#include "../TinyEngine/color.h"

//==============================================================================
//  TYPES
//==============================================================================

class cap_point : public object
{
public:
    enum state
    {
        CAP_POINT_NEUTRAL,
        CAP_POINT_SECURING,
        CAP_POINT_NEUTRALIZING,
    };

                    cap_point           ( void );
virtual            ~cap_point           ( void );

virtual void        OnLogic             ( float Time );
virtual void        OnRender            ( void );
virtual void        OnRemove            ( void );

virtual void        OnDejaObjInit       ( void );
virtual void        OnDejaPost          ( void );

        int         GetOwnerID          ( void );
        float       GetInfluenceRange   ( void );

protected:

        state       m_State;

        int         m_iOwner;   // Index into m_TeamID[].
        int         m_iTarget;  // Index into m_TeamID[].
        float       m_Level;
        float       m_Range;
        float       m_InfluenceRange;

        radian      m_Yaw;

static  int         m_nTeams;
static  int         m_TeamID[ CAP_POINT_MAX_TEAMS ];

friend  void        DejaDescriptor  ( const cap_point& );
};

//==============================================================================
//  FUNCTIONS
//==============================================================================

void SystemPrep_CapPoint( void );
void DejaDescriptor( const cap_point& );

//==============================================================================
//  STORAGE
//==============================================================================

extern type_data CapPointTypeData;

//==============================================================================
#endif // ifndef CAP_POINT_H
//==============================================================================

