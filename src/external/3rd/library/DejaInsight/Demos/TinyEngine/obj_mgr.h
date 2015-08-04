//==============================================================================
//
//  obj_mgr.h
//
//==============================================================================
//  Copyright (C) DejaTools, LLC.  All rights reserved.
//==============================================================================

#ifndef OBJ_MGR_H
#define OBJ_MGR_H

//==============================================================================
//  DEFINES
//==============================================================================

#define MAX_TYPES         32
#define MAX_OBJECTS     4096

//==============================================================================
//  INCLUDES
//==============================================================================

#include "../TinyEngine/vector3.h"

//==============================================================================
//  TYPES
//==============================================================================

class  obj_mgr;
struct type_data;

//------------------------------------------------------------------------------
//
//  Object life cycle:
//
//      - constructor ...... Standard C++ construction.
//      - OnDejaObjInit .... Calls DEJA_OBJECT_INIT for the object.
//      {
//          - OnLogic ...... Runs logic on the object.
//          - OnDejaPost ... Calls DEJA_POST on the object.
//          - OnRender ..... If appropriate, renders the object.
//      }
//      - OnRemove ......... Called prior to object destruction.
//      - destructor ....... Standard C++ destruction. 
//
//------------------------------------------------------------------------------

class object
{
private:
        int         m_ID;
        vector3     m_Position;
        type_data*  m_pTypeData;
        object*     m_pNext;        // Next     instance within this type.
        object*     m_pPrev;        // Previous instance within this type.

public:
                    object          ( void );
virtual            ~object          ( void );

virtual void        OnLogic         ( float ) { };
virtual void        OnRender        ( void )  { };
virtual void        OnRemove        ( void )  { };

virtual void        OnDejaObjInit   ( void )  { };
virtual void        OnDejaPost      ( void );

virtual bool        OnDamage        ( const vector3&, int ) { return false; };
virtual void        OnKillCredit    ( void )  { };

        int         GetID           ( void );
const   vector3&    GetPosition     ( void );
        void        SetPosition     ( const vector3& V );
virtual vector3     GetPredictedPos ( float Time );

friend              obj_mgr;
friend  void        DejaDescriptor  ( const object& );
};

//==============================================================================

typedef object* factory_fn( void );

//------------------------------------------------------------------------------

struct type_data
{
public:
        int         iType;
const   char*       pName;
        factory_fn* pFactoryFn;
        int         nInstances;
        object*     pFirst;         // First instance of given type.
        object*     pCursor;        // Cursor for type iteration.

                    type_data       ( const char* apTypeName, 
                                      factory_fn* apFactoryFn );
                   ~type_data       ( void );
private:
                    type_data       ( void );
};

//==============================================================================

class obj_mgr
{
public:

                    obj_mgr         ( void );
                   ~obj_mgr         ( void );

        int         RegisterType    ( type_data&        TypeData );
        void        ResetTypeLoop   ( int               iType );
        object*     GetNextInType   ( int               iType );

        object*     GetObjByID      ( int               ID );
        int         TypeNameToIndex ( const char*       pTypeName );

        object*     CreateObject    ( int               iType );
        object*     CreateObject    ( const char*       pTypeName );
        void        AddObject       ( object*           pObject );
        void        RemoveObject    ( int               ID );
        void        RemoveObject    ( object*           pObject );
        void        DestroyObject   ( object*           pObject );
        void        RemoveDestroy   ( int               ID );
        void        RemoveDestroy   ( object*           pObject );
        void        RemoveDestroyAll( void );

        int         GetClosest      ( object*           pObject,
                                      const char*       pTypeName );

        void        ApplyDamage     ( const vector3&    Position,
                                      int               OwnerID,
                                      int               Damage,
                                      float             InnerRadius,
                                      float             OuterRadius );

        void        Logic           ( float             DeltaTime );
        void        Render          ( void );

protected:

        bool        m_Initialized;
        int         m_NextSlot;

static  type_data*  m_TypeData[ MAX_TYPES   ];
static  object*     m_pObject [ MAX_OBJECTS ];
static  int         m_nTypes;
static  int         m_nObjects;
};

//==============================================================================
//  STORAGE
//==============================================================================

extern obj_mgr  ObjMgr;

//==============================================================================
#endif // ifndef OBJ_MGR_H
//==============================================================================
