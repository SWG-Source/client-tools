//==============================================================================
//
//  My Memory Manager
//
//==============================================================================
//  Copyright (C) DejaTools, LLC.  All rights reserved.
//==============================================================================

//------------------------------------------------------------------------------
#ifdef ENABLE_MY_MEMORY_MGR
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//----
// First, handle the standard C heap functions.
//----

// Define macros to divert the public interface to the internal functions 
// capturing file/line if needed.
#ifdef ENABLE_SOURCE_CODE_INFO
    #define MyMalloc( s )     DbgMalloc (    s, __FILE__, __LINE__ )
    #define MyRealloc( p, s ) DbgRealloc( p, s, __FILE__, __LINE__ )
    #define MyFree( p )       DbgFree   ( p,    __FILE__, __LINE__ )
#else
    #define MyMalloc( s )     StdMalloc (    s )
    #define MyRealloc( p, s ) StdRealloc( p, s )
    #define MyFree( p )       StdFree   ( p    )
#endif

// Announce the internal functions.
void* StdMalloc (              size_t Size                                  );
void* DbgMalloc (              size_t Size, const char*    pFName, int Line );
void* DbgMalloc (              size_t Size, const wchar_t* pFName, int Line );
void* StdRealloc( void*  pMem, size_t Size                                  );
void* DbgRealloc( void*  pMem, size_t Size, const char*    pFName, int Line );
void* DbgRealloc( void*  pMem, size_t Size, const wchar_t* pFName, int Line );
void  StdFree   ( void*  pMem                                               );
void  DbgFree   ( void*  pMem,              const char*    pFName, int Line );
void  DbgFree   ( void*  pMem,              const wchar_t* pFName, int Line );

//----
// Now, announce extended versions of the C++ heap functions.  The standard
// versions were already announced in MyMemoryMgr.h as part of the public
// interface.
//----

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

//----
//
// A couple of notes here for those of you who haven't yet grappled with 
// overriding new and delete.
//
// For the standard C library heap functions malloc, realloc, and free, we
// decided to provide alternate identifiers MyMalloc, MyRealloc, and MyFree.
// It is possible to use macros to "silently" convert malloc to MyMalloc.
// However, using MyMalloc reveals to people reading the code that the heap
// functionality is defined by the project.  (Unfortunately, you can't do the
// same with new and delete since they are operators.)  Also, using a macro
// to change malloc into something else could cause diffilculties with code
// provided by other parties.
//
// If you define versions of global operator new taking anything but a single
// size_t parameter, then you might need to define corresponding versions of 
// global operator delete.  There is no way to directly call these extended 
// versions of delete.  However, the compiler will automatically generate calls 
// to them if an exception occurs within a constructor triggered by an extended 
// version of operator new.  Some compilers are more finicky than others in this
// regard.  Best to just define them and be done with it.
//
// And do you like that macro definition of new?  Strange as it seems, when such
// a macro is in place, the preprocessor and compiler essentially convert the 
// code as shown here:
//
//      pWidget = new widget;                                 // Original code.
//      pWidget = new( __FILE__, __LINE__ ) widget;           // Preprocessed.
//      pWidget = new( sizeof(widget), __FILE__, __LINE__ );  // Compiled.
//
//----

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#else // ifdef ENABLE_MY_MEMORY_MGR
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//
// If we are compiling here, then MyMemoryMgr is disabled.  Define the macros
// to produce standard C library function calls.
//

#include <memory.h>

#define MyMalloc( s )     malloc (    s )
#define MyRealloc( p, s ) realloc( p, s )
#define MyFree( p )       free   ( p    )

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#endif // ifdef ENABLE_MY_MEMORY_MGR
//------------------------------------------------------------------------------
