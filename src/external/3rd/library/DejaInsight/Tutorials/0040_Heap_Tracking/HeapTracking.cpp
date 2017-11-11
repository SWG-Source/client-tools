//==============================================================================
//
//  Heap Tracking
//
//==============================================================================
//  Copyright (C) DejaTools, LLC.  All rights reserved.
//==============================================================================

#include "..\..\DejaLib.h"

#ifdef WIN32
#pragma comment( lib, "../../DejaLib.Win32.lib" )
#endif
#ifdef XBOX
#pragma comment( lib, "../../DejaLib.X360.lib" )
#endif

//------------------------------------------------------------------------------
//
//  If you have a global memory manager in your project, then adding Insight
//  heap instrumentation should be fairly straight forward.
//
//  (If you do NOT have a memory manager in your project, adding Insight heap
//  support can be challenging.  We suggest that you first centralize all heap
//  activity into your own memory manager, even if said memory manager is merely
//  a wrapper around the standard library memory manager.)
//
//  This tutorial includes an extremely thin memory manager wrapper around the 
//  standard library heap functions.  It is called "MyMemoryMgr".
//
//------------------------------------------------------------------------------

#include "MyMemoryMgr.h"

//------------------------------------------------------------------------------
//
//  The following functions are available for instrumenting heap activity:
//
//    DEJA_LOG_MALLOC         - Log a memory allocation   from malloc.
//    DEJA_LOG_REALLOC        - Log a memory reallocation from realloc.
//    DEJA_LOG_FREE           - Log a memory free         from free.
//
//    DEJA_LOG_NEW            - Log a memory allocation   from new.
//    DEJA_LOG_DELETE         - Log a memory free         from delete.
//
//    DEJA_LOG_NEW_ARRAY      - Log a memory allocation   from new [].
//    DEJA_LOG_DELETE_ARRAY   - Log a memory free         from delete [].
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
//  Each of the functions is available in two forms where the alternate form
//  allows you to provide the source file and line number.
//
//    void DEJA_LOG_MALLOC      ( void* pMemory, size_t Size );
//    void DEJA_LOG_MALLOC      ( void* pMemory, size_t Size, 
//                                const char* pFileName, int Line );
//  
//    void DEJA_LOG_REALLOC     ( void* pNew, void* pOld, size_t NewSize );
//    void DEJA_LOG_REALLOC     ( void* pNew, void* pOld, size_t NewSize, 
//                                const char* pFileName, int Line );
//  
//    void DEJA_LOG_FREE        ( void* pMemory );
//    void DEJA_LOG_FREE        ( void* pMemory, 
//                                const char* pFileName, int Line );
//  
//    void DEJA_LOG_NEW         ( void* pMemory, size_t Size );
//    void DEJA_LOG_NEW         ( void* pMemory, size_t Size, 
//                                const char* pFileName, int Line );
//  
//    void DEJA_LOG_DELETE      ( void* pMemory );
//    void DEJA_LOG_DELETE      ( void* pMemory, 
//                                const char* pFileName, int Line );
//  
//    void DEJA_LOG_NEW_ARRAY   ( void* pMemory, size_t Size );
//    void DEJA_LOG_NEW_ARRAY   ( void* pMemory, size_t Size, 
//                                const char* pFileName, int Line );
//  
//    void DEJA_LOG_DELETE_ARRAY( void* pMemory );
//    void DEJA_LOG_DELETE_ARRAY( void* pMemory, 
//                                const char* pFileName, int Line );
//
//------------------------------------------------------------------------------
//
//  All of the heap instrumentation appears in the file MyMemoryMgr.cpp.
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  We need a little bit of infrastructure for our random heap operations.
//
//------------------------------------------------------------------------------

#include <stdlib.h>     // For function rand().

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#define ITEMS         100
#define ITERATIONS    500

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class foo
{
    char m_Field[100];
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

foo*    pFoo[ ITEMS ] = { 0 };
char*   pMem[ ITEMS ] = { 0 };

//------------------------------------------------------------------------------
//
//  Random malloc / realloc / free operations.
//
//------------------------------------------------------------------------------

void MallocReallocFree( void )
{
    DEJA_CONTEXT( "MallocReallocFree" );

    int i;
    int Index;

    DEJA_BOOKMARK( "Heap", "Heap should be empty." );
    DEJA_TRACE   ( "Heap", "Begin random malloc / realloc / free." );

    for( i = 0; i < ITERATIONS; i++ )
    {
        Index = rand() % (ITEMS-1);

        if( pMem[Index] )
        {
            if( rand() & 1 )
            {
                pMem[Index] = (char*)MyRealloc( pMem[Index], (i+1)*100 );
            }
            else
            {
                MyFree( pMem[Index] );
                pMem[Index] = NULL;
            }            
        }
        else
        {
            pMem[Index] = (char*)MyMalloc( (i+1)*100 );
        }
    }

    DEJA_BOOKMARK( "Heap", "Heap should NOT be empty." );
    DEJA_TRACE   ( "Heap", "Clean up any remaining allocations." );

    for( Index = 0; Index < ITEMS; Index++ )
    {
        if( pMem[Index] )
        {
            MyFree( pMem[Index] );
            pMem[Index] = NULL;
        }
    }
}

//------------------------------------------------------------------------------
//
//  Random new / delete operations.
//
//------------------------------------------------------------------------------

void NewDelete( void )
{
    DEJA_CONTEXT( "NewDelete" );

    int i;
    int Index;

    DEJA_BOOKMARK( "Heap", "Heap should be empty." );
    DEJA_TRACE   ( "Heap", "Begin random new / delete." );

    for( i = 0; i < ITERATIONS; i++ )
    {
        Index = rand() % (ITEMS-1);

        if( pFoo[Index] )
        {
            delete pFoo[Index];
            pFoo[Index] = NULL;
        }
        else
        {
            pFoo[Index] = new foo;
        }
    }

    DEJA_BOOKMARK( "Heap", "Heap should NOT be empty." );
    DEJA_TRACE   ( "Heap", "Clean up any remaining allocations." );

    for( Index = 0; Index < ITEMS; Index++ )
    {
        if( pFoo[Index] )
        {
            delete pFoo[Index];
            pFoo[Index] = NULL;
        }
    } 
}

//------------------------------------------------------------------------------
//
//  Random new[] / delete[] operations.
//
//------------------------------------------------------------------------------

void NewDeleteArray( void )
{
    DEJA_CONTEXT( "NewDeleteArray" );

    int i;
    int Index;

    DEJA_BOOKMARK( "Heap", "Heap should be empty." );
    DEJA_TRACE   ( "Heap", "Begin random new[ ] / delete[ ]." );

    for( i = 0; i < ITERATIONS; i++ )
    {
        Index = rand() % (ITEMS-1);

        if( pFoo[Index] )
        {
            delete [] pFoo[Index];
            pFoo[Index] = NULL;
        }
        else
        {
            pFoo[Index] = new foo[i+1];
        }
    }

    DEJA_BOOKMARK( "Heap", "Heap should NOT be empty." );
    DEJA_TRACE   ( "Heap", "Clean up any remaining allocations." );

    for( Index = 0; Index < ITEMS; Index++ )
    {
        if( pFoo[Index] )
        {
            delete [] pFoo[Index];
            pFoo[Index] = NULL;
        }
    }
}

//------------------------------------------------------------------------------
//
//  This function intentionally causes heap problems to show how Insight reacts.
//  (By default, the call to this function is commented out.  See the comments
//  in function main().)
//
//  Feel free to alter this function to create various heap mishaps.  Note that
//  the OS may trap some errors.
//
//------------------------------------------------------------------------------

void HeapErrors( void )
{
    DEJA_CONTEXT( "HeapErrors" );

    DEJA_BOOKMARK( "Heap", "Heap should be empty." );
    DEJA_WARNING ( "Heap", "Begin intentional heap errors." );

    foo* pTest = NULL;

    // Double free.
//  OOPS - Windows traps this particular error.  Commented out for now.
//  pTest = (foo*)MyMalloc( sizeof(foo) );
//  MyFree( pTest );
//  MyFree( pTest );

    // Double delete.
//  OOPS - Windows traps this particular error.  Commented out for now.
//  pTest = new foo;
//  delete pTest;
//  delete pTest;

    // Bad pointer to free.
    pTest = (foo*)0x12345678;
    MyFree( pTest );

    // Bad delete.
    pTest = (foo*)0x12345678;
    delete pTest;

    // Crossed API operation: malloc and delete.
    pTest = (foo*)MyMalloc( sizeof(foo) );
    delete pTest;

    // Crossed API operation: new and free.
    pTest = new foo;
    MyFree( pTest );

    // Crossed API operation: new[] and delete.
    pTest = new foo[2];
    delete pTest;
}

//------------------------------------------------------------------------------
//
//  Demonstrate how to register custom heap functions with the DejaLib and show
//  that the DejaLib will use those functions thus allowing DejaLib's own heap
//  activity to be logged.
//
//------------------------------------------------------------------------------

void DejaLibHeapActivity( void )
{
    DEJA_CONTEXT( "DejaLibHeapActivity" );

    DEJA_BOOKMARK( "Heap", "Heap should be empty." );
    DEJA_WARNING ( "Heap", "Begin forced DejaLib heap operations." );

    // Register the custom heap functions with the DejaLib.  Note that the 
    // public "functions" are actually macros so that the source code file and
    // line can be silently added.  To register the heap functions, we have to
    // use the actual function names.
    DEJA_SET_MALLOC ( StdMalloc  );
    DEJA_SET_REALLOC( StdRealloc );
    DEJA_SET_FREE   ( StdFree    );

    // Although it is not required, you can also register the debug variations.
    // The DejaLib will use the "best" function available.
    DEJA_SET_MALLOC ( DbgMalloc  );
    DEJA_SET_REALLOC( DbgRealloc );
    DEJA_SET_FREE   ( DbgFree    );

    // Force the DejaLib to perform heap operations.
    DEJA_SET_PARAMETER( DEJA_BUFFER_COUNT, 5 );

    // Memory allocated by the DejaLib may not be released until the DejaLib is 
    // explicitly terminated via DEJA_TERMINATE.  In this tutorial, a call to
    // DEJA_TERMINATE appears as the last line in main().
}

//------------------------------------------------------------------------------
//
//  main
//
//------------------------------------------------------------------------------

void main( void )
{
    DEJA_CONTEXT( "main" );

    DEJA_WARNING( "Demo note", "Press F2 to cycle through bookmarks..." );
    DEJA_WARNING( "Demo note", "...in both the Trace and Heap Log views." );

    MallocReallocFree();
    NewDelete();
    NewDeleteArray();

    DejaLibHeapActivity();

    // By default, the following function call is commented out.  After all,
    // a tutorial which generates errors in the output is unflattering.  You
    // are encouraged to uncomment the function call and experiment with the 
    // function code to see what problems Insight will catch.

//  HeapErrors();

    // Finally, terminate the DejaLib.  This will force DejaLib to release any 
    // allocations.

    DEJA_BOOKMARK( "Heap", "About to call DEJA_TERMINATE..." );
    DEJA_TERMINATE();
}

//------------------------------------------------------------------------------
