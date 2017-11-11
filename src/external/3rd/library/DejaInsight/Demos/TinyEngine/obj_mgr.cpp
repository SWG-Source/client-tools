//==============================================================================
//
//  obj_mgr.cpp
//
//==============================================================================
//  Copyright (C) DejaTools, LLC.  All rights reserved.
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "stdafx.h"
#include "obj_mgr.h"
#include "engine.h"
#include <string.h> // strcmp()

#include "../../DejaLib.h"

//==============================================================================
//  STORAGE
//==============================================================================

obj_mgr ObjMgr;

type_data*  obj_mgr::m_TypeData[ MAX_TYPES   ] = { 0 };
object*     obj_mgr::m_pObject [ MAX_OBJECTS ] = { 0 };
int         obj_mgr::m_nTypes                  =   0;
int         obj_mgr::m_nObjects                =   0;

//==============================================================================
//  FUNCTIONS - type_data
//==============================================================================

type_data::type_data( void )
{
    // 'void' construction is not allowed.
    ASSERT( false );
}

//==============================================================================

type_data::type_data( const char* apTypeName, factory_fn* apFactoryFn )
:   iType       (   -1 ),
    pName       ( NULL ),
    pFactoryFn  ( NULL ),
    nInstances  (    0 ),
    pFirst      ( NULL ),
    pCursor     ( NULL )
{
    pName      = apTypeName;
    pFactoryFn = apFactoryFn;
}

//==============================================================================

type_data::~type_data( void )
{
    // Do some sanity checks on the way out.
    if( pName == NULL )
    {
        DEJA_WARNING( "type_data::~type_data", 
                      "Type never named -- type_data at 0x%P", this );
    }
    else
    if( iType == -1 )
    {
        DEJA_WARNING( "type_data::~type_data", 
                      "Type '%s' was never registered", pName );
    }
}

//==============================================================================
//  FUNCTIONS - object
//==============================================================================

object::object( void )
:   m_ID        (   -1 ),
    m_pTypeData ( NULL ),
    m_pNext     ( NULL ),
    m_pPrev     ( NULL )
{
}

//==============================================================================

object::~object( void )
{
    ASSERT( m_ID == -1 );
}

//==============================================================================

const vector3& object::GetPosition( void )
{
    return( m_Position );
}

//==============================================================================

void object::SetPosition( const vector3& V )
{
    m_Position = V;
}

//==============================================================================

int object::GetID( void )
{
    return( m_ID );
}

//==============================================================================

vector3 object::GetPredictedPos( float Time )
{
    (void)Time;
    return GetPosition();
}

//==============================================================================

void object::OnDejaPost( void )
{
    DEJA_POST( *this );
}

//==============================================================================

void DejaDescriptor( const object& O )
{
    DEJA_TYPE ( O, object );
    DEJA_FIELD( O.m_ID );
    DEJA_FIELD( O.m_Position );
}

//==============================================================================
//  FUNCTIONS - obj_mgr
//==============================================================================

obj_mgr::obj_mgr( void )
{
    // 'Singleton' construction sanity check.
    ASSERT( !m_Initialized );
    m_Initialized = true;
    m_NextSlot = 0;
}

//==============================================================================

obj_mgr::~obj_mgr( void )
{
    // 'Singleton' destruction sanity check.
    ASSERT( m_Initialized );
    m_Initialized = false;

    // ObjMgr sanity checks.
    ASSERT( m_nObjects == 0 );
}

//==============================================================================

int obj_mgr::RegisterType( type_data& TypeData )
{
    ASSERT( m_Initialized );
    ASSERT( m_nTypes < MAX_TYPES );
    ASSERT( TypeData.pName );

    // Search the registered types to see if this type is already listed.
    for( int i = 0; i < m_nTypes; i++ )
    {
        if( m_TypeData[i] == &TypeData )
        {
            return( i );
        }
    }

    // Type was not previously registered.  Do so now.
    int Result = m_nTypes;
    m_TypeData[m_nTypes] = (type_data*)&TypeData;
    TypeData.iType = m_nTypes;
    m_nTypes += 1;
    return( Result );
}

//==============================================================================

void obj_mgr::ResetTypeLoop( int iType )
{
    ASSERT( m_Initialized );
    ASSERT( iType < m_nTypes );
    
    m_TypeData[iType]->pCursor = m_TypeData[iType]->pFirst;
}

//==============================================================================

object* obj_mgr::GetNextInType( int iType )
{
    ASSERT( m_Initialized );
    ASSERT( iType < m_nTypes );

    object* pObject = m_TypeData[iType]->pCursor;

    if( pObject )
    {
        m_TypeData[iType]->pCursor = pObject->m_pNext;
    }
    
    return( pObject );
}

//==============================================================================

object* obj_mgr::GetObjByID( int ID )
{
    return( m_pObject[ID] );
}

//==============================================================================

int obj_mgr::TypeNameToIndex( const char* pTypeName )
{
    ASSERT( pTypeName );

    for( int iType = 0; iType < MAX_TYPES; iType++ )
    {
        if( (m_TypeData[iType]) && 
            (strcmp( pTypeName, m_TypeData[iType]->pName ) == 0) )
        {
            return iType;
        }
    }

    // Not found
    return -1;
}

//==============================================================================

object* obj_mgr::CreateObject( int iType )
{
    ASSERT( m_nObjects < MAX_OBJECTS );
    ASSERT( m_TypeData[iType] );

    object* pObject = m_TypeData[iType]->pFactoryFn();
    pObject->m_pTypeData = m_TypeData[iType];
    m_TypeData[iType]->nInstances += 1;
    return( pObject );
}

//==============================================================================

object* obj_mgr::CreateObject( const char* pTypeName )
{
    int iType = TypeNameToIndex( pTypeName );
    if( iType != -1 )
        return( CreateObject( iType ) );

    ASSERT( false );
    return( NULL );
}

//==============================================================================

void obj_mgr::AddObject( object* pObject )
{
    ASSERT( pObject );
    ASSERT( pObject->m_pTypeData );
    ASSERT( m_TypeData[pObject->m_pTypeData->iType] == pObject->m_pTypeData );

    // Search for an available slot.
    int Slot = m_NextSlot;
    for( int i = 0; i < MAX_OBJECTS; i++ )
    {
        if( !m_pObject[Slot] )
        {
            // Grab a reference to the type data for ease of access.
            type_data& TypeData = *(pObject->m_pTypeData);

            // Slot the object in.
            m_pObject[Slot] = pObject;
            pObject->m_ID   = Slot;

            // Link the object into the type chain.
            object*& pFirst  = TypeData.pFirst;
            pObject->m_pNext = pFirst;
            pObject->m_pPrev = NULL;
            if( pFirst )
            {
                pFirst->m_pPrev = pObject;
            }
            pFirst = pObject;

            // Bump up the object count.
            m_nObjects += 1;

            // Register the object with the DejaLib.
            pObject->OnDejaObjInit();

            // For the DejaLib, give the object a better name than '*this'.
            DEJA_OBJECT_LABEL( *pObject, "%s[%d]", TypeData.pName, Slot );

            // Update the next slot to allocate.
            m_NextSlot = (Slot+1) % MAX_OBJECTS;

            // And we're done.
            return;
        }
        else
        {
            Slot = (Slot+1) % MAX_OBJECTS;
        }
    }

    DEJA_ERROR( "obj_mgr::AddObject", "ObjMgr is FULL." );
    ASSERT( false );
}

//==============================================================================

void obj_mgr::RemoveObject( object* pObject )
{
    ASSERT( pObject );
    ASSERT( m_pObject[pObject->m_ID] == pObject );

    // Inform the object.
    pObject->OnRemove();

    // Grab a reference to the type data for ease of access.
    type_data& TypeData = *(pObject->m_pTypeData);

    // Slot the object out.
    m_pObject[pObject->m_ID] = NULL;
    pObject->m_ID            = -1;

    // Unlink the object from the type chain.
    if( TypeData.pFirst == pObject ) TypeData.pFirst = pObject->m_pNext;
    if( pObject->m_pNext )  pObject->m_pNext->m_pPrev = pObject->m_pPrev;
    if( pObject->m_pPrev )  pObject->m_pPrev->m_pNext = pObject->m_pNext;

    // If the type cursor is on the object, move it to the next.
    if( TypeData.pCursor == pObject )
        TypeData.pCursor = pObject->m_pNext;

    // In the interest of safety, clear the object's type chain pointers.
    pObject->m_pNext = NULL;
    pObject->m_pPrev = NULL;

    // Bump down the object count.
    m_nObjects -=  1;
}

//==============================================================================

void obj_mgr::RemoveObject( int ID )
{
    ASSERT( m_pObject[ID] );
    RemoveObject( m_pObject[ID] );
}

//==============================================================================

void obj_mgr::DestroyObject( object* pObject )
{
    ASSERT( pObject );
    ASSERT( pObject->m_ID == -1 );
    ASSERT( pObject->m_pTypeData );

    pObject->m_pTypeData->nInstances -= 1;
    pObject->m_pTypeData = NULL;

    // Kill the object in Insight.
    DEJA_OBJECT_KILL( *pObject );

    // Delete the object.
    delete pObject;
}

//==============================================================================

void obj_mgr::RemoveDestroy( int ID )
{
    ASSERT( m_pObject[ID] );

    object* pObject = m_pObject[ID];

    RemoveObject ( pObject );
    DestroyObject( pObject );
}

//==============================================================================

void obj_mgr::RemoveDestroy( object* pObject )
{
    ASSERT( pObject );

    RemoveObject ( pObject );
    DestroyObject( pObject );
}

//==============================================================================

void obj_mgr::RemoveDestroyAll( void )
{
    for( int i=0; i<MAX_OBJECTS; i++ )
    {
        object* pObject = m_pObject[i];
        if( pObject )
        {
            RemoveDestroy( pObject );
            m_pObject[i] = NULL;
        }
    }
}

//==============================================================================

int obj_mgr::GetClosest( object* pObject, const char* pTypeName )
{
    DEJA_CONTEXT( "obj_mgr::GetClosest" );

    int iType = TypeNameToIndex( pTypeName );
    if( iType != -1 )
    {
        vector3 p = pObject->GetPosition();

        object* pTest = m_TypeData[iType]->pFirst;
        if( pTest )
        {
            float ClosestDist = (pTest->GetPosition() - p).GetLengthSqr();
            object* pClosest = pTest;
            pTest = pTest->m_pNext;
            while( pTest )
            {
                float Dist = (pTest->GetPosition() - p).GetLengthSqr();
                if( Dist < ClosestDist )
                {
                    ClosestDist = Dist;
                    pClosest = pTest;
                }
                pTest = pTest->m_pNext;
            }

            return pClosest->m_ID;
        }
    }

    return( -1 );
}

//==============================================================================

void obj_mgr::ApplyDamage( const vector3& Position, 
                                 int      OwnerID, 
                                 int      Damage, 
                                 float    InnerRadius, 
                                 float    OuterRadius )
{
    DEJA_CONTEXT( "obj_mgr::ApplyDamage" );
    DEJA_TRACE( "obj_mgr::ApplyDamage", 
                "Center:%g,%g,%g - Damage:%d", 
                Position.x, Position.y, Position.z, Damage );

    float InnerToOuter = OuterRadius - InnerRadius;

    for( int i=0; i<MAX_OBJECTS; i++ )
    {
        object* pObject = m_pObject[i];
        if( pObject )
        {
            vector3 Delta = pObject->GetPosition() - Position;
            float Distance = Delta.GetLength();
            if( Distance < OuterRadius )
            {
                int d = Damage;
                if( Distance > InnerRadius )
                    d = (int)(Damage * (OuterRadius - Distance)/InnerToOuter);

                DEJA_TRACE( "obj_mgr::ApplyDamage",
                            "Recipient:%$ - Damage:%d", 
                            pObject, d );

                bool Kill = pObject->OnDamage( Position, d );
                if( Kill )
                {
                    object* pObject = GetObjByID( OwnerID );
                    if( pObject )
                    {
                        pObject->OnKillCredit();
                    }
                }
            }
        }
    }
}

//==============================================================================

void obj_mgr::Logic( float DeltaTime )
{
    DEJA_CONTEXT( "obj_mgr::Logic" );
    for( int i = 0; i < MAX_OBJECTS; i++ )
    {
        if( m_pObject[i] )
        {
            DEJA_SCOPE_OBJECT( *m_pObject[i] );
            m_pObject[i]->OnLogic( DeltaTime );
        }

        // The object may have deleted itself, so check if it is still there.
        if( m_pObject[i] )
        {
            m_pObject[i]->OnDejaPost();
        }
    }
}

//==============================================================================

void obj_mgr::Render( void )
{
    DEJA_CONTEXT( "obj_mgr::Render" );
    for( int i = 0; i < MAX_OBJECTS; i++ )
    {
        if( m_pObject[i] )
        {
            DEJA_SCOPE_OBJECT( *m_pObject[i] );
            m_pObject[i]->OnRender();
        }
    }
}

//==============================================================================
