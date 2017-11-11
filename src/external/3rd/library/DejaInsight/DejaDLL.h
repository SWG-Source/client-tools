//==============================================================================
//
//  DejaDLL.h
//
//==============================================================================
//  Copyright (C) DejaTools, LLC.  All rights reserved.
//==============================================================================

#ifndef DEJA_DLL_H
#define DEJA_DLL_H

#include <stdio.h>

//==============================================================================
//  CONTROL MACROS
//==============================================================================
//
//  To disable ALL DejaLib functionality, simply define DEJA_DISABLED *before*
//  including DejaDLL.h.
//
//==============================================================================

//==============================================================================
//  LOGGING FUNCTIONS
//==============================================================================
//
//  DEJA_TRACE          - Issue a normal  message to the Insight Trace Log.
//  DEJA_WARNING        - Issue a warning message to the Insight Trace Log.
//  DEJA_ERROR          - Issue an error  message to the Insight Trace Log.
//  DEJA_BOOKMARK       - Same as DEJA_TRACE, and bookmarks the message.
//
//==============================================================================

void DEJA_TRACE         ( const char*    pChannel, const char*    pFormat, ... );
void DEJA_WARNING       ( const char*    pChannel, const char*    pFormat, ... );
void DEJA_ERROR         ( const char*    pChannel, const char*    pFormat, ... );
void DEJA_BOOKMARK      ( const char*    pChannel, const char*    pFormat, ... );

//------------------------------------------------------------------------------
//  Wide character versions.
//------------------------------------------------------------------------------

void DEJA_TRACE         ( const wchar_t* pChannel, const char*    pFormat, ... );
void DEJA_TRACE         ( const char*    pChannel, const wchar_t* pFormat, ... );
void DEJA_TRACE         ( const wchar_t* pChannel, const wchar_t* pFormat, ... );

void DEJA_WARNING       ( const wchar_t* pChannel, const char*    pFormat, ... );
void DEJA_WARNING       ( const char*    pChannel, const wchar_t* pFormat, ... );
void DEJA_WARNING       ( const wchar_t* pChannel, const wchar_t* pFormat, ... );

void DEJA_ERROR         ( const wchar_t* pChannel, const char*    pFormat, ... );
void DEJA_ERROR         ( const char*    pChannel, const wchar_t* pFormat, ... );
void DEJA_ERROR         ( const wchar_t* pChannel, const wchar_t* pFormat, ... );

void DEJA_BOOKMARK      ( const wchar_t* pChannel, const char*    pFormat, ... );
void DEJA_BOOKMARK      ( const char*    pChannel, const wchar_t* pFormat, ... );
void DEJA_BOOKMARK      ( const wchar_t* pChannel, const wchar_t* pFormat, ... );

//==============================================================================
//  GENERAL FUNCTIONALITY
//==============================================================================
//
//  DEJA_FLUSH(false)   - Begin process of sending out any currently buffered
//                        information.  Do not wait for data to be sent.
//  DEJA_FLUSH(true)    - Send out all buffered data.  Do not return until all
//                        data has been sent.
//  DEJA_TERMINATE      - Shut down the DejaLib.  If dynamic memory was 
//                        allocated, it will be released.  All buffers are
//                        flushed.
//
//  DEJA_APP_LABEL      - Give a meaningful name to the application.  
//                        Examples: "Server" or "Client:7".
//                        
//  DEJA_THREAD_INIT    - Explicitly announce the start of a new thread to 
//                        Insight.  Should be executed within the thread.
//  DEJA_THREAD_KILL    - Explicitly announce the end of a thread to Insight.
//                        Should be executed within the thread.
//  DEJA_THREAD_LABEL   - Provide a meaningful name to the current thread.
//                        Examples: "main" or "Physics" or "Worker".
//
//------------------------------------------------------------------------------
//
//  DEJA_SET_AUTO_FLUSH - Turn auto flush on or off.
//
//------------------------------------------------------------------------------
//
//  DEJA_LIB_CONNECTED  - Returns false if the DejaLib failed to connect or is
//                        no longer connected to an Insight session.
//
//  DEJA_LAST_ERROR     - Returns a string describing the last error the DejaLib
//                        encountered.
//
//------------------------------------------------------------------------------
//                        
//  DEJA_SET_PARAMETER  - Change a DejaLib parameter using one of the listed 
//                        deja_parameter identifiers.  See documentation for
//                        more information.
//
//  DEJA_SET_VALUE      - Set a value to a variable which affects Insight.  See
//                        documentation for details.
//
//------------------------------------------------------------------------------
//                        
//  DEJA_ENUM_VALUE     - Provide an enumeration value and name  to Insight.
//  DEJA_ENUM_ALIAS     - Provide an enumeration value and alias to Insight.
//
//------------------------------------------------------------------------------
//                        
//  If the DejaLib needs to use dynamic memory...
//                        
//  DEJA_SET_MALLOC     - Set the malloc  function DejaLib will use.
//  DEJA_SET_REALLOC    - Set the realloc function DejaLib will use.
//  DEJA_SET_FREE       - Set the free    function DejaLib will use.
//
//==============================================================================

      void  DEJA_FLUSH          ( bool Block = true );
      void  DEJA_NEXT_FRAME     ( void );
      void  DEJA_TERMINATE      ( void );
  
      void  DEJA_APP_LABEL      ( const char* pLabelFormat, ... );
  
      void  DEJA_THREAD_INIT    ( void );
      void  DEJA_THREAD_KILL    ( int ExitCode );
      void  DEJA_THREAD_LABEL   ( const char* pLabelFormat, ... );
  
      void  DEJA_SET_AUTO_FLUSH ( bool AutoFlush );
  
      bool  DEJA_LIB_CONNECTED  ( void );
const char* DEJA_LAST_ERROR     ( void );

//------------------------------------------------------------------------------
//  Configuration parameters.
//------------------------------------------------------------------------------

enum deja_parameter
{
    DEJA_CACHE_SIZE_BYTES_FILE,
    DEJA_CACHE_SIZE_BYTES_LABEL,
    DEJA_CACHE_SIZE_BYTES_CHANNEL,
    DEJA_CACHE_SIZE_BYTES_FORMAT,
    DEJA_CACHE_SIZE_BYTES_ARGUMENT,

    DEJA_CACHE_SIZE_ITEMS_FILE,
    DEJA_CACHE_SIZE_ITEMS_LABEL,
    DEJA_CACHE_SIZE_ITEMS_CHANNEL,
    DEJA_CACHE_SIZE_ITEMS_FORMAT,
    DEJA_CACHE_SIZE_ITEMS_ARGUMENT,

    DEJA_BUFFER_SIZE_BYTES,
    DEJA_BUFFER_COUNT
};

void DEJA_SET_PARAMETER ( deja_parameter Parameter, int Value );

//------------------------------------------------------------------------------
//  Insight variables.
//------------------------------------------------------------------------------

void DEJA_SET_VALUE     ( const char* pVariableName, int Value );

//------------------------------------------------------------------------------
//  Enumeration support.
//------------------------------------------------------------------------------
/*
For enumerated type E:

    void DEJA_ENUM_VALUE( E, int EValue );
    void DEJA_ENUM_ALIAS( E, int EValue, const char* pEAlias );
*/
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  EXAMPLE
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
//  enum compass { COMPASS_NORTH, COMPASS_SOUTH, COMPASS_EAST, COMPASS_WEST };
//  
//  DEJA_ENUM_VALUE( compass, COMPASS_NORTH );
//  DEJA_ENUM_ALIAS( compass, COMPASS_NORTH, "North" );
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//------------------------------------------------------------------------------
//  Dynamic memory support.
//------------------------------------------------------------------------------

typedef void* deja_std_malloc_fn (        size_t                              );
typedef void* deja_dbg_malloc_fn (        size_t, const char* pFile, int Line );
typedef void* deja_std_realloc_fn( void*, size_t                              );
typedef void* deja_dbg_realloc_fn( void*, size_t, const char* pFile, int Line );
typedef void  deja_std_free_fn   ( void*                                      );
typedef void  deja_dbg_free_fn   ( void*,         const char* pFile, int Line );

void DEJA_SET_MALLOC    ( deja_std_malloc_fn*  pStdMallocFn  );
void DEJA_SET_MALLOC    ( deja_dbg_malloc_fn*  pDbgMallocFn  );
void DEJA_SET_REALLOC   ( deja_std_realloc_fn* pStdReallocFn );
void DEJA_SET_REALLOC   ( deja_dbg_realloc_fn* pDbgReallocFn );
void DEJA_SET_FREE      ( deja_std_free_fn*    pStdFreeFn    );
void DEJA_SET_FREE      ( deja_dbg_free_fn*    pDbgFreeFn    );

//==============================================================================
//  HEAP OPERATIONS
//==============================================================================
//
//  DEJA_LOG_MALLOC         - Log a memory allocation   from malloc.
//  DEJA_LOG_REALLOC        - Log a memory reallocation from realloc.
//  DEJA_LOG_FREE           - Log a memory free         from free.
//
//  DEJA_LOG_NEW            - Log a memory allocation   from new.
//  DEJA_LOG_DELETE         - Log a memory free         from delete.
//
//  DEJA_LOG_NEW_ARRAY      - Log a memory allocation   from new [].
//  DEJA_LOG_DELETE_ARRAY   - Log a memory free         from delete [].
//
//==============================================================================

void DEJA_LOG_MALLOC        ( void* pMemory, size_t Size );
void DEJA_LOG_MALLOC        ( void* pMemory, size_t Size, const char*    pFileName, int Line );
void DEJA_LOG_MALLOC        ( void* pMemory, size_t Size, const wchar_t* pFileName, int Line );

void DEJA_LOG_REALLOC       ( void* pNewMem, void* pOldMem, size_t NewSize );
void DEJA_LOG_REALLOC       ( void* pNewMem, void* pOldMem, size_t NewSize, const char*    pFileName, int Line );
void DEJA_LOG_REALLOC       ( void* pNewMem, void* pOldMem, size_t NewSize, const wchar_t* pFileName, int Line );

void DEJA_LOG_FREE          ( void* pMemory );
void DEJA_LOG_FREE          ( void* pMemory, const char*    pFileName, int Line );
void DEJA_LOG_FREE          ( void* pMemory, const wchar_t* pFileName, int Line );

void DEJA_LOG_NEW           ( void* pMemory, size_t Size );
void DEJA_LOG_NEW           ( void* pMemory, size_t Size, const char*    pFileName, int Line );
void DEJA_LOG_NEW           ( void* pMemory, size_t Size, const wchar_t* pFileName, int Line );

void DEJA_LOG_DELETE        ( void* pMemory );
void DEJA_LOG_DELETE        ( void* pMemory, const char*    pFileName, int Line );
void DEJA_LOG_DELETE        ( void* pMemory, const wchar_t* pFileName, int Line );

void DEJA_LOG_NEW_ARRAY     ( void* pMemory, size_t Size );
void DEJA_LOG_NEW_ARRAY     ( void* pMemory, size_t Size, const char*    pFileName, int Line );
void DEJA_LOG_NEW_ARRAY     ( void* pMemory, size_t Size, const wchar_t* pFileName, int Line );

void DEJA_LOG_DELETE_ARRAY  ( void* pMemory );
void DEJA_LOG_DELETE_ARRAY  ( void* pMemory, const char*    pFileName, int Line );
void DEJA_LOG_DELETE_ARRAY  ( void* pMemory, const wchar_t* pFileName, int Line );

//==============================================================================
//  CONTEXT
//==============================================================================
//
//  DEJA_CONTEXT        - Enter and name a context.  Only one allowed per scope.
//                        Exit is automatic when execution leaves the scope.
//                        The parameter must be a literal string.
//
//==============================================================================

void DEJA_CONTEXT       ( const char* pContextLabel );                         

//==============================================================================
//  OBJECT TRACKING FUNCTIONS
//==============================================================================
//
//  DEJA_OBJECT_INIT    - Announce the creation of an object instance.
//                        Requires a corresponding DejaDescriptor function.
//  DEJA_OBJECT_KILL    - Announce the departure of an object instance.
//  DEJA_OBJECT_LABEL   - Associate a meaningful name with a known object.
//                        Default name is "<type> @ <address>".
//
//  DEJA_SCOPE_OBJECT   - Set object as 'active' for remainder of current scope.
//                        Automatically reverts to previously scoped object when
//                        when execution leaves current scope.  Object must be
//                        previously announced via DEJA_OBJECT_INIT.
//
//  DEJA_POST           - For a known object, post fields listed in the 
//                        associated DejaDescriptor function to Insight.  Object 
//                        must be previously announced via DEJA_OBJECT_INIT.
//
//------------------------------------------------------------------------------
//
//  DejaDescriptor      - User provided function.  Describes the given type 
//                        using the following functions:
//
//      DEJA_TYPE       - Identifies the type of the object.  Must be first in
//                        the DejaDescriptor function.
//      DEJA_BASE       - Optional: Identifies the base class of the object.
//                        Corresponding DejaDescriptor for the base class must
//                        be defined if DEJA_BASE is used.
//      DEJA_FIELD      - Post data for a non-enum field.  If the field type is 
//                        not a predefined type (such as int or float), then a 
//                        corresponding DejaDescriptor function must be defined.
//                        If the field is a static array, all elements of the 
//                        array will be posted.
//      DEJA_ENUM       - Post an enumerated value and display based on the 
//                        type.  (See DEJA_ENUM_VALUE and DEJA_ENUM_ALIAS.)
//      DEJA_VLARRAY    - Post the content of a pointer as a variable length 
//                        array.  The number of elements in the array must be 
//                        specified.  If the content type is not a predefined
//                        type, a corresponding DejaDescriptor function must be
//                        defined.
//      DEJA_IF_PTR     - If the given pointer to data is not NULL, then the 
//                        target data is posted.  This will only post a single
//                        instance of the target type.
//      DEJA_CSTRING    - Post a NULL terminated string given a char* or 
//                        wchar_t*.
//                        
//==============================================================================
/*
For arbitrary type T:

    void DEJA_OBJECT_INIT   ( const T&    Instance );
    void DEJA_OBJECT_KILL   ( const T&    Instance );
    void DEJA_OBJECT_LABEL  ( const T&    Instance, 
                              const char* pLabelFormat, ... );

    void DEJA_SCOPE_OBJECT  ( const T&    Instance );

    void DEJA_POST          ( const T&    Instance );
*/
//------------------------------------------------------------------------------
/*
For arbitrary type T, base class B, and enumerated type E:

    void DejaDescriptor     ( const T&       Instance_of_T     );

    void DEJA_TYPE          ( const T&       Instance_of_T,  T );
    void DEJA_BASE          ( const T&       Instance_of_T,  B );

    void DEJA_FIELD         ( const F&       Field_in_T        );
    void DEJA_ENUM          ( const E        Enum_in_T,      E );
    void DEJA_VLARRAY       ( const T*       Ptr_to_T,   int Count );
    void DEJA_IF_PTR        ( const T*       Ptr_to_T );
    void DEJA_CSTRING       ( const char*    pNString );
    void DEJA_CSTRING       ( const wchar_t* pWString );
*/
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  EXAMPLE
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
//  enum compass { COMPASS_NORTH, COMPASS_SOUTH, COMPASS_EAST, COMPASS_WEST };
//  
//  class my_class { ... };
//  class my_base  { ... };
//
//  class my_descent : public my_base
//  {
//      float     m_Float;          // Atomic   type member variable.
//      my_class  m_Instance;       // Compound type member variable.
//      int       m_Array[4];       // Atomic   type member fixed size array.
//      char      m_NName[16];      // Narrow string member fixed size array.
//      wchar_t   m_WName[16];      // Wide   string member fixed size array.
//      int       m_IntAsEnum;      // Integer       member holding enum value.
//      compass   m_Enum;           // Enum     type member variable.
//      float*    m_pFloat;         // Pointer to variable length array.
//      int       m_nFloat;         // Number of floats in m_pFloat.
//      my_class* m_pMyClass;       // Pointer to variable length array.
//      int       m_nMyClass;       // Number of instances in m_pMyClass.
//      char*     m_pString;        // Standard NULL terminated string.
//      ...
//      
//      // Must grant friendship to access non-public fields.
//      friend void DejaDescriptor( const my_descent& );
//  };
//
//  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
//  void DejaDescriptor( const my_class& C )
//  {
//      DEJA_TYPE   ( C, my_class );
//      ...
//  }
//  
//  void DejaDescriptor( const my_base& B )
//  {
//      DEJA_TYPE   ( B, my_base );
//      ...
//  }
//  
//  void DejaDescriptor( const my_descent& D )
//  {
//      DEJA_TYPE   ( D, my_descent );      // Must provide the type.
//      DEJA_BASE   ( D, my_base    );      // Optional: Provide base class.
//
//      DEJA_FIELD  ( D.m_Float );                  // Atomic type field.
//      DEJA_FIELD  ( D.m_Instance );               // Compound type field.
//      DEJA_FIELD  ( D.m_Array );                  // Fixed size array of int.
//      DEJA_FIELD  ( D.m_NName );                  // Fixed size array of char.
//      DEJA_FIELD  ( D.m_WName );                  // Fixed size array of wchar_t.
//      DEJA_ENUM   ( D.m_IntAsEnum, compass );     // m_IntAsEnum = COMPASS_NORTH;
//      DEJA_ENUM   ( D.m_Enum,      compass );     // m_Enum      = COMPASS_NORTH;
//  //  DEJA_FIELD  ( D.m_Enum );                 //// ERROR - Use DEJA_ENUM.
//      DEJA_VLARRAY( D.m_pFloat,   D.m_nFloat   ); // Run-time sized array.
//      DEJA_VLARRAY( D.m_pMyClass, D.m_nMyClass ); // Run-time sized array.
//      DEJA_IF_PTR ( D.m_pMyClass );               // If non-null, *(m_pMyClass).
//      DEJA_CSTRING( D.m_pString );                // NULL terminated string.
//  }
// 
//  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// 
//  // Example life cycle.
//  {
//      my_descent* pMyD = new my_descent;
//      DEJA_OBJECT_INIT ( *pMyD );
//      DEJA_OBJECT_LABEL( *pMyD, "MyDescent instance" );
//      DEJA_POST        ( *pMyD );
//      ...
//
//      pMyD->MyFunction();
//      ...
//
//      DEJA_OBJECT_KILL( *this );
//  }
//
//  void my_descent::MyFunction( void )
//  {
//      DEJA_SCOPE_OBJECT( *this );
//      DEJA_POST( *this );
//      ...
//  } 
// 
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//==============================================================================
//  PRIVATE
//==============================================================================

#define DEJA_DLL
#include "DejaPrivate.h"

//==============================================================================
#endif // DEJA_DLL_H
//==============================================================================
