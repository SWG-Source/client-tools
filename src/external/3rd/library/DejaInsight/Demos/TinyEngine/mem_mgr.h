//==============================================================================
//
//  mem_mgr.h
//
//  Simple heap interface wrapper which includes appropriate DejaLib 
//  instrumentation.
//
//==============================================================================
//  Copyright (C) DejaTools, LLC.  All rights reserved.
//==============================================================================

#ifndef MEM_MGR_H
#define MEM_MGR_H

//------------------------------------------------------------------------------
//
//  Configurable options:
//
//    - ENABLE_MEM_MGR - If defined, enables the mem_mgr within the TinyEngine.  
//          If not defined, then all heap funtions compile directly into their
//          standard library heap function analogs.
//
//    - ENABLE_SOURCE_CODE_INFO - If defined, allows the TinyEngine's mem_mgr to
//          note source code file name and line number.  The file/line 
//          information is provided to the DejaLib instrumentation.
//
//------------------------------------------------------------------------------

// Comment/uncomment these two macros as needed.
// Alternately define them based on the project configuration.

#define ENABLE_MEM_MGR
#define ENABLE_SOURCE_CODE_INFO

//------------------------------------------------------------------------------
//
//  The interface is as follows:
//
//      malloc              - Defined into call to tiny_malloc.
//      realloc             - Defined into call to tiny_realloc.
//      free                - Defined into call to tiny_free.
//
//      operator new        - Replaces default global operator new.
//      operator new []     - Replaces default global operator new [].
//      operator delete     - Replaces default global operator delete.
//      operator delete []  - Replaces default global operator delete [].
//
//------------------------------------------------------------------------------

//==============================================================================
//  DEFINES
//==============================================================================

#ifdef ENABLE_MEM_MGR

    #ifdef ENABLE_SOURCE_CODE_INFO
        #define malloc(     s )     tiny_malloc (    s, __FILE__, __LINE__ )
        #define realloc( p, s )     tiny_realloc( p, s, __FILE__, __LINE__ )
        #define free(    p    )     tiny_free   ( p,    __FILE__, __LINE__ )
    #else
        #define malloc(     s )     tiny_malloc (    s )
        #define realloc( p, s )     tiny_realloc( p, s )
        #define free(    p    )     tiny_free   ( p    )
    #endif

#endif

//==============================================================================
//  FUNCTIONS
//==============================================================================

#ifdef ENABLE_MEM_MGR

void* tiny_malloc (           size_t Size                                  );
void* tiny_malloc (           size_t Size, const char*    pFName, int Line );
void* tiny_malloc (           size_t Size, const wchar_t* pFName, int Line );
void* tiny_realloc( void*  p, size_t Size                                  );
void* tiny_realloc( void*  p, size_t Size, const char*    pFName, int Line );
void* tiny_realloc( void*  p, size_t Size, const wchar_t* pFName, int Line );
void  tiny_free   ( void*  p                                               );
void  tiny_free   ( void*  p,              const char*    pFName, int Line );
void  tiny_free   ( void*  p,              const wchar_t* pFName, int Line );

void* operator new       ( size_t Size );
void* operator new    [] ( size_t Size );

void  operator delete    ( void* p );
void  operator delete [] ( void* p );

void* operator new       ( size_t Size, const char*    pFileName, int Line );
void* operator new       ( size_t Size, const wchar_t* pFileName, int Line );
void* operator new    [] ( size_t Size, const char*    pFileName, int Line );
void* operator new    [] ( size_t Size, const wchar_t* pFileName, int Line );

void  operator delete    ( void* p,     const char*    pFileName, int Line );
void  operator delete    ( void* p,     const wchar_t* pFileName, int Line );
void  operator delete [] ( void* p,     const char*    pFileName, int Line );
void  operator delete [] ( void* p,     const wchar_t* pFileName, int Line );

#ifdef ENABLE_SOURCE_CODE_INFO
#define new new( __FILE__, __LINE__ )
#endif

#endif

//==============================================================================
#endif // ifndef MEM_MGR_H
//==============================================================================
