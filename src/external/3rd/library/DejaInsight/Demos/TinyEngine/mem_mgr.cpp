//==============================================================================
//
//  mem_mgr.cpp
//
//==============================================================================
//  Copyright (C) DejaTools, LLC.  All rights reserved.
//==============================================================================

#include "stdafx.h"

#include "..\..\DejaLib.h"
#include <stdlib.h>

//==============================================================================

#ifdef new
#undef new
#endif

#ifdef delete
#undef 
#endif

#ifdef malloc
#undef malloc
#endif

#ifdef realloc
#undef realloc
#endif

#ifdef free
#undef free
#endif

//==============================================================================
//
//  First, the functions which actually perform the heap operations.  These just 
//  wrap the standard C library heap functions.
//
//  Both the standard C library heap function replacements (MyMalloc, MyRealloc,
//  and MyFree) and the C++ global operators new and delete are routed through 
//  here.
//
//==============================================================================

void* HeapMalloc( size_t Size )
{
    void* pResult = malloc( Size );
    return( pResult );
}

//------------------------------------------------------------------------------

void* HeapRealloc( void* pMemory, size_t Size )
{
    void* pResult = realloc( pMemory, Size );
    return( pResult );
}

//------------------------------------------------------------------------------

void HeapFree( void* pMemory )
{
    free( pMemory );
}

//==============================================================================
//
//  Next, the public functions which provide the standard C heap functionality.
//  These functions include appropriate DejaLib instrumentation.
//
//==============================================================================

void* tiny_malloc( size_t Size )
{
    void* pResult = HeapMalloc( Size );
    DEJA_LOG_MALLOC( pResult, Size );
    return( pResult );
}

//------------------------------------------------------------------------------

void* tiny_malloc( size_t Size, const char* pFileName, int Line )
{
    void* pResult = HeapMalloc( Size );
    DEJA_LOG_MALLOC( pResult, Size, pFileName, Line );
    return( pResult );
}

//------------------------------------------------------------------------------

void* tiny_malloc( size_t Size, const wchar_t* pFileName, int Line )
{
    void* pResult = HeapMalloc( Size );
    DEJA_LOG_MALLOC( pResult, Size, pFileName, Line );
    return( pResult );
}

//==============================================================================

void* tiny_realloc( void* pMemory, size_t Size )
{
    void* pResult = HeapRealloc( pMemory, Size );
    DEJA_LOG_REALLOC( pResult, pMemory, Size );
    return( pResult );
}

//------------------------------------------------------------------------------

void* tiny_realloc(       void*  pMemory, 
                        size_t Size, 
                  const char*  pFileName, 
                        int    Line )
{
    void* pResult = HeapRealloc( pMemory, Size );
    DEJA_LOG_REALLOC( pResult, pMemory, Size, pFileName, Line );
    return( pResult );
}

//------------------------------------------------------------------------------

void* tiny_realloc(       void*    pMemory, 
                        size_t   Size, 
                  const wchar_t* pFileName, 
                        int      Line )
{
    void* pResult = HeapRealloc( pMemory, Size );
    DEJA_LOG_REALLOC( pResult, pMemory, Size, pFileName, Line );
    return( pResult );
}

//==============================================================================

void tiny_free( void* pMemory )
{
    if( pMemory ) 
    { 
        // Recommendation:  Only log when the pointer is non-NULL.
        DEJA_LOG_FREE( pMemory );
    }

    HeapFree( pMemory );
}

//------------------------------------------------------------------------------

void tiny_free( void* pMemory, const char* pFileName, int Line )
{
    if( pMemory ) 
    { 
        // Recommendation:  Only log when the pointer is non-NULL.
        DEJA_LOG_FREE( pMemory, pFileName, Line );
    }

    HeapFree( pMemory );
}

//------------------------------------------------------------------------------

void tiny_free( void* pMemory, const wchar_t* pFileName, int Line )
{
    if( pMemory ) 
    { 
        // Recommendation:  Only log when the pointer is non-NULL.
        DEJA_LOG_FREE( pMemory, pFileName, Line );
    }

    HeapFree( pMemory );
}

//==============================================================================
//
//  Finally, the public versions of global operators new and delete.  These 
//  functions include appropriate DejaLib instrumentation.
//
//==============================================================================

void* operator new ( size_t Size )
{
    void* pResult = HeapMalloc( Size );
    DEJA_LOG_NEW( pResult, Size );
    return( pResult );
}

//------------------------------------------------------------------------------

void* operator new ( size_t Size, const char* pFileName, int Line )
{
    void* pResult = HeapMalloc( Size );
    DEJA_LOG_NEW( pResult, Size, pFileName, Line );
    return( pResult );
}

//------------------------------------------------------------------------------

void* operator new ( size_t Size, const wchar_t* pFileName, int Line )
{
    void* pResult = HeapMalloc( Size );
    DEJA_LOG_NEW( pResult, Size, pFileName, Line );
    return( pResult );
}

//==============================================================================

void* operator new [] ( size_t Size )
{
    void* pResult = HeapMalloc( Size );
    DEJA_LOG_NEW_ARRAY( pResult, Size );
    return( pResult );
}

//------------------------------------------------------------------------------

void* operator new [] ( size_t Size, const char* pFileName, int Line )
{
    void* pResult = HeapMalloc( Size );
    DEJA_LOG_NEW_ARRAY( pResult, Size, pFileName, Line );
    return( pResult );
}

//------------------------------------------------------------------------------

void* operator new [] ( size_t Size, const wchar_t* pFileName, int Line )
{
    void* pResult = HeapMalloc( Size );
    DEJA_LOG_NEW_ARRAY( pResult, Size, pFileName, Line );
    return( pResult );
}

//==============================================================================

void operator delete ( void* pMemory )
{
    if( pMemory )
    {
        // Recommendation:  Only log when the pointer is non-NULL.
        DEJA_LOG_DELETE( pMemory );
    }

    HeapFree( pMemory );
}

//------------------------------------------------------------------------------

void operator delete ( void* pMemory, const char* pFileName, int Line )
{
    if( pMemory )
    {
        // Recommendation:  Only log when the pointer is non-NULL.
        DEJA_LOG_DELETE( pMemory, pFileName, Line );
    }

    HeapFree( pMemory );
}

//------------------------------------------------------------------------------

void operator delete ( void* pMemory, const wchar_t* pFileName, int Line )
{
    if( pMemory )
    {
        // Recommendation:  Only log when the pointer is non-NULL.
        DEJA_LOG_DELETE( pMemory, pFileName, Line );
    }

    HeapFree( pMemory );
}

//==============================================================================

void operator delete [] ( void* pMemory )
{
    if( pMemory )
    {
        // Recommendation:  Only log when the pointer is non-NULL.
        DEJA_LOG_DELETE_ARRAY( pMemory );
    }

    HeapFree( pMemory );
}

//------------------------------------------------------------------------------

void operator delete [] ( void* pMemory, const char* pFileName, int Line )
{
    if( pMemory )
    {
        // Recommendation:  Only log when the pointer is non-NULL.
        DEJA_LOG_DELETE_ARRAY( pMemory, pFileName, Line );
    }

    HeapFree( pMemory );
}

//------------------------------------------------------------------------------

void operator delete [] ( void* pMemory, const wchar_t* pFileName, int Line )
{
    if( pMemory )
    {
        // Recommendation:  Only log when the pointer is non-NULL.
        DEJA_LOG_DELETE_ARRAY( pMemory, pFileName, Line );
    }

    HeapFree( pMemory );
}

//==============================================================================
