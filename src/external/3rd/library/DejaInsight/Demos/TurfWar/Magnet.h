//==============================================================================
//
//  Magnet.h
//
//==============================================================================
//  Copyright (C) DejaTools, LLC.  All rights reserved.
//==============================================================================

#ifndef MAGNET_H
#define MAGNET_H

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

class magnet : public object
{
public:
                    magnet          ( void );
                   ~magnet          ( void );

virtual void        OnLogic         ( float Time );
virtual void        OnRender        ( void );
virtual void        OnRemove        ( void );

virtual void        OnDejaObjInit   ( void );
virtual void        OnDejaPost      ( void );

        void        Spawn           ( const vector3& Position,
                                      const vector3& Velocity );
        void        SetRange        ( const vector3& Min,
                                      const vector3& Max );

        int         GetBoidCount    ( void );
        void        IncBoidCount    ( void );
        void        DecBoidCount    ( void );
        
virtual vector3     GetPredictedPos ( float Time );

protected:
        vector3     m_Velocity;
        vector3     m_RangeMin;
        vector3     m_RangeMax;
        int         m_nBoids;

friend  void        DejaDescriptor  ( const magnet& );
};

//==============================================================================
//  FUNCTIONS
//==============================================================================

void SystemPrep_Magnet( void );
void DejaDescriptor( const magnet& );

//==============================================================================
//  STORAGE
//==============================================================================

extern type_data MagnetTypeData;

//==============================================================================
#endif // ifndef MAGNET_H
//==============================================================================
