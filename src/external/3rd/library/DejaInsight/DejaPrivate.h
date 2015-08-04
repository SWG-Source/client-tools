//==============================================================================
//
//  DejaPrivate.h
//
//==============================================================================
//  Copyright (C) DejaTools, LLC.  All rights reserved.
//==============================================================================

#ifndef DEJA_PRIVATE_H
#define DEJA_PRIVATE_H

//==============================================================================
//  INCLUDES
//==============================================================================

#include <stdarg.h>

//==============================================================================
//  DYNAMIC LINK LIBRARY MACROS
//==============================================================================

#ifdef DEJA_DLL
    #ifdef DEJA_DLL_EXPORT
        #define DEJA_V_DECL __declspec(dllexport)
        #define DEJA_F_DECL __declspec(dllexport)
    #else
        #define DEJA_V_DECL __declspec(dllimport)
        #define DEJA_F_DECL 
    #endif
#else
        #define DEJA_V_DECL
        #define DEJA_F_DECL
#endif

//==============================================================================
//  WIDE CHARACTER SUPPORT IF 'wchar_t' IS NOT A BUILT IN TYPE
//      (For example: DevStudio 2003 with default compiler settings.)
//==============================================================================

#define  deja_wc  unsigned short

//==============================================================================
//  PULLED FROM DejaLib.h TEMPORARILY
//==============================================================================

struct deja_vector
{
    float X;
    float Y;
    float Z;
};

//------------------------------------------------------------------------------

void DEJA_3D_CHANNEL    ( const char* pChannel );
void DEJA_3D_COLOR      ( unsigned char R, 
                          unsigned char G, 
                          unsigned char B, 
                          unsigned char A = 255 );

void DEJA_3D_POINT      ( float X, float Y, float Z );
void DEJA_3D_POINT      ( const deja_vector& V );
void DEJA_3D_POINT_LIST ( const deja_vector* pVerts, int nVerts );

void DEJA_3D_LINE       ( const deja_vector& V0, 
                          const deja_vector& V1 );
void DEJA_3D_LINE_LIST  ( const deja_vector* pVerts, int nVerts );
void DEJA_3D_LINE_STRIP ( const deja_vector* pVerts, int nVerts );

void DEJA_3D_TRI        ( const deja_vector& V0, 
                          const deja_vector& V1, 
                          const deja_vector& V2 );
void DEJA_3D_TRI_LIST   ( const deja_vector* pVerts, int nVerts );
void DEJA_3D_TRI_STRIP  ( const deja_vector* pVerts, int nVerts );

void DEJA_3D_MARKER     ( float X, float Y, float Z );
void DEJA_3D_MARKER     ( const deja_vector& V );

void DEJA_3D_SPHERE     ( float X, float Y, float Z, float Radius );
void DEJA_3D_SPHERE     ( const deja_vector& Center, float Radius );

void DEJA_3D_BOX_LOCAL  ( const deja_vector& Corner0,
                          const deja_vector& Corner1 );
void DEJA_3D_BOX        ( const deja_vector& Anchor );

//==============================================================================
//  PLATFORM AND COMPILER AWARENESS
//==============================================================================

// Check if the target has already been defined by the x_files.
#ifndef TARGET_DEFINED    

    #if (defined( DEJA_TARGET_WIN32 ))
        #define TARGET_WIN32

    #elif (defined( DEJA_TARGET_XBOX ))
        #define TARGET_XBOX360

    #elif (defined( DEJA_TARGET_PS3 ))
        #define TARGET_PS3

    #elif (defined( DEJA_TARGET_OSX ))
        #define TARGET_PS3

    #elif (defined( XBOX ) || defined( _XBOX ))
        #define TARGET_XBOX360

    #elif (defined( PS3 ) || defined( _PS3 ) || defined( __CELLOS_LV2__ ))
        #define TARGET_PS3

    #elif (defined( WIN32 ) || defined( _WIN32 ))
        #define TARGET_WIN32

    #elif (defined( __APPLE__ ))
        #define TARGET_OSX

    #else
        #error Could not determine the compiler/platform.
    #endif

    #define TARGET_DEFINED

#endif

//------------------------------------------------------------------------------
//  Does the compiler support "variadic macros"?  That is, macros with variable 
//  arguments via an ellipsis such as:
//      #define MACRO( Arg1, Arg2, ... )
//------------------------------------------------------------------------------

#if defined( _MSC_VER )
    // Microsoft compilers.
    #if _MSC_VER >= 1400
    #define VARIADIC_MACROS
    #endif
#else
    // All other compilers.
    #define VARIADIC_MACROS
#endif

//------------------------------------------------------------------------------
//  Under Microsoft compilers, we can use a pragma to get the library linked
//  into the target application.  The customer will still have to provide path
//  information to find the lib.
//------------------------------------------------------------------------------

//** Unfortunately, we can't figure out how to squelch runtime library mismatch 
//** link warnings, so we're going to pass on the pragma technique for now.
//#ifdef TARGET_WIN32
//#pragma comment( lib, "DejaLib.Win32.lib" )
//#endif

//==============================================================================
//  FUNCTIONS AND STRUCTURES
//==============================================================================

//------------------------------------------------------------------------------
//  General
//------------------------------------------------------------------------------

struct deja_log_flag;

//------------------------------------------------------------------------------

DEJA_F_DECL void        DejaLock            ( const char*     pFileName, 
                                                    int       Line,
                                              deja_log_flag*  pLogFlag,                            
                                                    int       LogCode );
DEJA_F_DECL void        DejaLock            ( const char*     pFileName, 
                                                    int       Line );
DEJA_F_DECL void        DejaLock            (       void );
DEJA_F_DECL void        DejaUnlock          (       void );
DEJA_F_DECL void        DejaFlush           (       bool      Block = true );
DEJA_F_DECL void        DejaAutoFlush       (       void );
DEJA_F_DECL void        DejaSetAutoFlush    (       bool      AutoFlush,
                                              const char*     pFileName, 
                                                    int       Line );
DEJA_F_DECL bool        DejaLibConnected    (       void );
DEJA_F_DECL void        DejaAppLabel        ( const char*     pNLabelFormat, ... );
DEJA_F_DECL void        DejaAppLabel        ( const wchar_t*  pWLabelFormat, ... );
DEJA_F_DECL void        DejaAppLabel        ( const deja_wc*  pWLabelFormat, ... );
DEJA_F_DECL void        DejaThreadLabel     ( const char*     pNLabelFormat, ... );
DEJA_F_DECL void        DejaThreadLabel     ( const wchar_t*  pWLabelFormat, ... );
DEJA_F_DECL void        DejaThreadLabel     ( const deja_wc*  pWLabelFormat, ... );
DEJA_F_DECL void        DejaSetParameter    ( deja_parameter  Parameter, int Value );
DEJA_F_DECL void        DejaSetValue        ( const char*     pVariable, int Value,
                                              const char*     pFileName, int Line );
DEJA_F_DECL void        DejaSetValue        ( const wchar_t*  pVariable, int Value,
                                              const char*     pFileName, int Line );
DEJA_F_DECL void        DejaSetValue        ( const deja_wc*  pVariable, int Value,
                                              const char*     pFileName, int Line );
DEJA_F_DECL void        DejaScopeObject     ( const void*     pAddr );
DEJA_F_DECL void        DejaNextFrame       (       void );
DEJA_F_DECL void        DejaThreadInit      (       void );
DEJA_F_DECL void        DejaThreadKill      (       int       ExitCode );
DEJA_F_DECL void        DejaTerminate       (       void );
DEJA_F_DECL void        DejaEnumAlias       ( const char*     pType, 
                                                    int       Value,
                                              const char*     pNAlias,
                                                    bool      bAlias );
DEJA_F_DECL void        DejaEnumAlias       ( const char*     pType, 
                                                    int       Value,
                                              const wchar_t*  pWAlias,
                                                    bool      bAlias );
DEJA_F_DECL void        DejaEnumAlias       ( const char*     pType, 
                                                    int       Value,
                                              const deja_wc*  pWAlias,
                                                    bool      bAlias );

DEJA_F_DECL void        DejaSetMalloc       ( deja_std_malloc_fn*  );
DEJA_F_DECL void        DejaSetMalloc       ( deja_dbg_malloc_fn*  );
DEJA_F_DECL void        DejaSetRealloc      ( deja_std_realloc_fn* );
DEJA_F_DECL void        DejaSetRealloc      ( deja_dbg_realloc_fn* );
DEJA_F_DECL void        DejaSetFree         ( deja_std_free_fn*    );
DEJA_F_DECL void        DejaSetFree         ( deja_dbg_free_fn*    );

DEJA_F_DECL void        DejaLogMalloc       ( void* pMemory, size_t Size );
DEJA_F_DECL void        DejaLogMalloc       ( void* pMemory, size_t Size, 
                                              const char* pFileName, int Line );
DEJA_F_DECL void        DejaLogMalloc       ( void* pMemory, size_t Size, 
                                              const wchar_t* pFileName, int Line );
DEJA_F_DECL void        DejaLogMalloc       ( void* pMemory, size_t Size, 
                                              const deja_wc* pFileName, int Line );

DEJA_F_DECL void        DejaLogRealloc      ( void* pNewMem, void* pOldMem, size_t NewSize );
DEJA_F_DECL void        DejaLogRealloc      ( void* pNewMem, void* pOldMem, size_t NewSize, 
                                              const char* pFileName, int Line );
DEJA_F_DECL void        DejaLogRealloc      ( void* pNewMem, void* pOldMem, size_t NewSize, 
                                              const wchar_t* pFileName, int Line );
DEJA_F_DECL void        DejaLogRealloc      ( void* pNewMem, void* pOldMem, size_t NewSize, 
                                              const deja_wc* pFileName, int Line );

DEJA_F_DECL void        DejaLogFree         ( void* pMemory );
DEJA_F_DECL void        DejaLogFree         ( void* pMemory, 
                                              const char* pFileName, int Line );
DEJA_F_DECL void        DejaLogFree         ( void* pMemory, 
                                              const wchar_t* pFileName, int Line );
DEJA_F_DECL void        DejaLogFree         ( void* pMemory, 
                                              const deja_wc* pFileName, int Line );

DEJA_F_DECL void        DejaLogNew          ( void* pMemory, size_t Size );
DEJA_F_DECL void        DejaLogNew          ( void* pMemory, size_t Size, 
                                              const char* pFileName, int Line );
DEJA_F_DECL void        DejaLogNew          ( void* pMemory, size_t Size, 
                                              const wchar_t* pFileName, int Line );
DEJA_F_DECL void        DejaLogNew          ( void* pMemory, size_t Size, 
                                              const deja_wc* pFileName, int Line );

DEJA_F_DECL void        DejaLogDelete       ( void* pMemory );
DEJA_F_DECL void        DejaLogDelete       ( void* pMemory, 
                                              const char* pFileName, int Line );
DEJA_F_DECL void        DejaLogDelete       ( void* pMemory, 
                                              const wchar_t* pFileName, int Line );
DEJA_F_DECL void        DejaLogDelete       ( void* pMemory, 
                                              const deja_wc* pFileName, int Line );

DEJA_F_DECL void        DejaLogNewArray     ( void* pMemory, size_t Size );
DEJA_F_DECL void        DejaLogNewArray     ( void* pMemory, size_t Size, 
                                              const char* pFileName, int Line );
DEJA_F_DECL void        DejaLogNewArray     ( void* pMemory, size_t Size, 
                                              const wchar_t* pFileName, int Line );
DEJA_F_DECL void        DejaLogNewArray     ( void* pMemory, size_t Size, 
                                              const deja_wc* pFileName, int Line );

DEJA_F_DECL void        DejaLogDeleteArray  ( void* pMemory );
DEJA_F_DECL void        DejaLogDeleteArray  ( void* pMemory, 
                                              const char* pFileName, int Line );
DEJA_F_DECL void        DejaLogDeleteArray  ( void* pMemory, 
                                              const wchar_t* pFileName, int Line );
DEJA_F_DECL void        DejaLogDeleteArray  ( void* pMemory, 
                                              const deja_wc* pFileName, int Line );

DEJA_F_DECL void        DejaCommand         ( const char* pCommand );

DEJA_F_DECL char*       DejaGetLastError    ( void );

DEJA_V_DECL extern bool DejaSystemActive;

struct deja_object_scope
{
    deja_object_scope( const void* pAddr, const char* pFileName, int Line )
    {
        if( DejaSystemActive )
        {
            DejaLock( pFileName, Line );
            DejaScopeObject( pAddr );
            DejaUnlock();
        }
    }
   ~deja_object_scope( void )
    {
        if( DejaSystemActive )
        {
            DejaLock();
            DejaScopeObject( 0 );
            DejaUnlock();
        }
    }   
};

//------------------------------------------------------------------------------
//  Logging
//------------------------------------------------------------------------------

struct deja_log_flag
{
    deja_log_flag( void )
    {
        On             = true;
        Registered     = false;
        LiteralChannel = true;
        LiteralFormat  = true;
    }
    deja_log_flag( const char* pChannel, const char* pFormat )
    {
        On             = true;
        Registered     = false;
        LiteralChannel = (pChannel[0] == '"') || (pChannel[1] == '"');
        LiteralFormat  = (pFormat [0] == '"') || (pFormat [1] == '"');
    }
    deja_log_flag( const wchar_t* pChannel, const char* pFormat )
    {
        On             = true;
        Registered     = false;
        LiteralChannel = (pChannel[0] == '"') || (pChannel[1] == '"');
        LiteralFormat  = (pFormat [0] == '"') || (pFormat [1] == '"');
    }
    deja_log_flag( const char* pChannel, const wchar_t* pFormat )
    {
        On             = true;
        Registered     = false;
        LiteralChannel = (pChannel[0] == '"') || (pChannel[1] == '"');
        LiteralFormat  = (pFormat [0] == '"') || (pFormat [1] == '"');
    }
    deja_log_flag( const wchar_t* pChannel, const wchar_t* pFormat )
    {
        On             = true;
        Registered     = false;
        LiteralChannel = (pChannel[0] == '"') || (pChannel[1] == '"');
        LiteralFormat  = (pFormat [0] == '"') || (pFormat [1] == '"');
    }
    bool On             : 1;
    bool Registered     : 1;
    bool LiteralChannel : 1;
    bool LiteralFormat  : 1;
};

DEJA_F_DECL void        DejaLog             ( const char*    pChannel, const char*    pFormat, ... );
DEJA_F_DECL void        DejaLog             ( const wchar_t* pChannel, const char*    pFormat, ... );
DEJA_F_DECL void        DejaLog             ( const char*    pChannel, const wchar_t* pFormat, ... );
DEJA_F_DECL void        DejaLog             ( const wchar_t* pChannel, const wchar_t* pFormat, ... );
DEJA_F_DECL void        DejaLog             ( const deja_wc* pChannel, const char*    pFormat, ... );
DEJA_F_DECL void        DejaLog             ( const char*    pChannel, const deja_wc* pFormat, ... );
DEJA_F_DECL void        DejaLog             ( const deja_wc* pChannel, const deja_wc* pFormat, ... );
DEJA_F_DECL void        DejaLog_VS7         ( const char*    pChannel, const char*    pFormat, ... );
DEJA_F_DECL void        DejaLog_VS7         ( const wchar_t* pChannel, const char*    pFormat, ... );
DEJA_F_DECL void        DejaLog_VS7         ( const char*    pChannel, const wchar_t* pFormat, ... );
DEJA_F_DECL void        DejaLog_VS7         ( const wchar_t* pChannel, const wchar_t* pFormat, ... );
DEJA_F_DECL void        DejaLog_VS7         ( const deja_wc* pChannel, const char*    pFormat, ... );
DEJA_F_DECL void        DejaLog_VS7         ( const char*    pChannel, const deja_wc* pFormat, ... );
DEJA_F_DECL void        DejaLog_VS7         ( const deja_wc* pChannel, const deja_wc* pFormat, ... );

//------------------------------------------------------------------------------
//  Static 3D
//------------------------------------------------------------------------------
/*
DEJA_F_DECL void        Deja3DChannel       ( const char*    pChannel );
DEJA_F_DECL void        Deja3DChannel       ( const wchar_t* pChannel );
DEJA_F_DECL void        Deja3DColor         ( unsigned char R, 
                                              unsigned char G, 
                                              unsigned char B, 
                                              unsigned char A = 255 );

DEJA_F_DECL void        Deja3DPoint         ( float X, float Y, float Z );
DEJA_F_DECL void        Deja3DPoint         ( const deja_vector* pVerts );
DEJA_F_DECL void        Deja3DPointList     ( const deja_vector* pVerts, int nVerts );

DEJA_F_DECL void        Deja3DLine          ( const deja_vector& V0, 
                                              const deja_vector& V1 );
DEJA_F_DECL void        Deja3DLineList      ( const deja_vector* pVerts, int nVerts );
DEJA_F_DECL void        Deja3DLineStrip     ( const deja_vector* pVerts, int nVerts );

DEJA_F_DECL void        Deja3DTri           ( const deja_vector& V0, 
                                              const deja_vector& V1, 
                                              const deja_vector& V2 );
DEJA_F_DECL void        Deja3DTriList       ( const deja_vector* pVerts, int nVerts );
DEJA_F_DECL void        Deja3DTriStrip      ( const deja_vector* pVerts, int nVerts );

DEJA_F_DECL void        Deja3DMarker        ( float X, float Y, float Z );
DEJA_F_DECL void        Deja3DMarker        ( const deja_vector& V );

DEJA_F_DECL void        Deja3DSphere        ( float X, float Y, float Z, float Radius );
DEJA_F_DECL void        Deja3DSphere        ( const deja_vector& Center, float Radius );

DEJA_F_DECL void        Deja3DBoxLocal      ( const deja_vector& Corner0,
                                              const deja_vector& Corner1 );
DEJA_F_DECL void        Deja3DBox           ( const deja_vector& Anchor );
*/
//------------------------------------------------------------------------------
//  Context
//------------------------------------------------------------------------------

class deja_context
{
    private:
                        deja_context        (       void ) { };

    public:
        DEJA_F_DECL     deja_context        ( const void*    Address );
        DEJA_F_DECL    ~deja_context        (       void );        
};

class deja_context_reg
{
    private:
                        deja_context_reg    (       void ) { };

    public:
        DEJA_F_DECL     deja_context_reg    ( const char*    pContext,
                                              const char*    pFileName,
                                                    int      Line );
        DEJA_F_DECL     deja_context_reg    ( const wchar_t* pContext,
                                              const char*    pFileName,
                                                    int      Line );
};

//------------------------------------------------------------------------------
//  Data posting
//------------------------------------------------------------------------------

#define DEJA_VLA_QUEUE_SIZE 100

typedef void (deja_d_fn)( const void* pV );

struct deja_type;

//------------------------------------------------------------------------------

DEJA_F_DECL int         DejaReserveTypeSeq  (       void );
DEJA_F_DECL void        DejaObjectInfo      ( const void*    pAddr, 
                                              const char*    pName );
DEJA_F_DECL void        DejaObjectInit      (       void );
DEJA_F_DECL void        DejaObjectKill      (       void );
DEJA_F_DECL void        DejaObjectLabel     ( const void*    pAddr, 
                                              const char*    pNLabelFormat, ... );
DEJA_F_DECL void        DejaObjectLabel     ( const void*    pAddr, 
                                              const wchar_t* pWLabelFormat, ... );
DEJA_F_DECL void        DejaTerminateType   (       void );
DEJA_F_DECL bool        DejaPostLogic       ( deja_type&     DT,
                                              const void*    pV );
DEJA_F_DECL void        DejaOpenPost        ( deja_type&     DT );
DEJA_F_DECL void        DejaOpenSubPost     ( deja_type&     DT );
DEJA_F_DECL void        DejaClosePost       (       void );
DEJA_F_DECL void        DejaCompoundField   (       void );
DEJA_F_DECL void        DejaVLAField        ( deja_d_fn*     pVLAFn, 
                                              const void*    pData );
DEJA_F_DECL void        DejaPostFlush       (       void );
DEJA_F_DECL int         DejaStrLen          ( const char*    pString );
DEJA_F_DECL int         DejaStrLen          ( const wchar_t* pString );
DEJA_F_DECL int         DejaStrLen          ( const deja_wc* pString );

//------------------------------------------------------------------------------

DEJA_V_DECL extern         int     DejaArraySize;
DEJA_V_DECL extern         int     DejaPostPhase;
DEJA_V_DECL extern         int     DejaTypeSeq;
DEJA_V_DECL extern         int     DejaOwnerType;
DEJA_V_DECL extern         bool    DejaAtomicFlag;
DEJA_V_DECL extern const   void*   DejaFieldAddr;
DEJA_V_DECL extern const   char*   DejaFieldName;
DEJA_V_DECL extern const   char*   DejaEnumTypeName;
DEJA_V_DECL extern         bool    DejaIfPtrFlag;
DEJA_V_DECL extern         int     DejaVLAQCursor;
DEJA_V_DECL extern         int     DejaVLAQCount;
DEJA_V_DECL extern deja_d_fn*      DejaVLAFnPtr[ DEJA_VLA_QUEUE_SIZE ];
DEJA_V_DECL extern const   void*   DejaVLAData [ DEJA_VLA_QUEUE_SIZE ];
DEJA_V_DECL extern         int     DejaVLACount[ DEJA_VLA_QUEUE_SIZE ];

//------------------------------------------------------------------------------

struct deja_type
{
          deja_d_fn* pDescriptor;
    const char*      pTypeName;
          int        TypeSeq;
          int        TypeSize;
          bool       bDefined;
          void*      pNext;
    deja_type( deja_d_fn*  apDescriptor, int Size, const char* apTypeName )
    {
        pDescriptor = apDescriptor;
        pTypeName   = apTypeName;
        TypeSeq     = 0;            // FIELD_UNKNOWN = 0
        TypeSize    = Size;
        bDefined    = false;
        pNext       = NULL;
    }
};

//------------------------------------------------------------------------------

struct deja_owner
{
    int PreviousOwnerType;
    deja_type& DejaType;
    deja_owner( deja_type& DT )
    :   DejaType(DT)
    {
        PreviousOwnerType = DejaOwnerType;
        DejaOwnerType     = DT.TypeSeq;
    }
    ~deja_owner( void )
    {
        if( DejaPostPhase == 1 )
        {
            DejaTerminateType();
            DejaType.bDefined = true;
        }
        DejaOwnerType = PreviousOwnerType;
    }
private:
    deja_owner& operator = ( const deja_owner& ) { return(*this); };
};

//---------------------
#ifndef VARIADIC_MACROS
//------------------------------------------------------------------------------
//  Without variadic macros, we must use a template function to both accept an
//  arbitrary object and accept variable parameters.  This approach is 
//  functionally equivalent to the variadic macro form.  The only downside is
//  the need to employ templates which generate more code.
//------------------------------------------------------------------------------

template< class T >
void DejaObjectLabelT( const T&       Object,   
                       const char*    pNLabelFormat, ... );

template< class T >
void DejaObjectLabelT( const T&       Object,   
                       const wchar_t* pWLabelFormat, ... );

void DejaObjectLabel ( const void*    pAddr, 
                       const char*    pNLabelFormat, 
                       const wchar_t* pWLabelFormat,
                             va_list  args );

//-----------------------
#endif // VARIADIC_MACROS
//------------------------------------------------------------------------------
// This form catches the following:
//  - Arrays in DEJA_FIELD or DEJA_ENUM.

template< class T, int N >
void DejaField( const T(&V)[N] )
{
    DejaArraySize = N;
    DejaField( V[0] );
}

//------------------------------------------------------------------------------
// This form catches the following:
//  - Non-predefined types, including enums.
//  - Pointers to non-atomic types where T = "type*".

template< class T >
void DejaField( const T& V )
{
    DejaDescriptor( V );

    if( !DejaAtomicFlag )
    {
        if( DejaArraySize > 1 )
        {
            const T* pV = &V;
            for( int i = 1; i < DejaArraySize; i++ )
            {
                DejaFieldAddr = &pV[i];
                DejaDescriptor( pV[i] );
            }
        }

        DejaCompoundField();
    }

    // Safe to clear the Atomic flag on this execution path.
    DejaAtomicFlag = false;  
}

//==============================================================================

template< class T >
void DejaVLAField( const T* pV )
{
    void DejaDescriptor( const T& );
    void (*pFn)( const T& ) = DejaDescriptor;
    DejaVLAField( (deja_d_fn*)pFn, (void*)pV );

    // Safe to clear the Atomic flag on this execution path.
    DejaAtomicFlag = false;
}

//------------------------------------------------------------------------------
//  Announce the explicit expansion for DejaVLAField<void>.
//------------------------------------------------------------------------------

DEJA_F_DECL void DejaVLAField( const void* pV );

//==============================================================================
//  We need to announce an explicit expansion of template functions DejaField
//  and DejaDescriptor for all atomic data types (char, int, float, etc).  
//------------------------------------------------------------------------------

DEJA_F_DECL void DejaField( const          void**     ); 
DEJA_F_DECL void DejaField( const          bool&      ); 
DEJA_F_DECL void DejaField( const          float&     ); 
DEJA_F_DECL void DejaField( const          double&    ); 
DEJA_F_DECL void DejaField( const          char&      ); 
DEJA_F_DECL void DejaField( const   signed char&      ); 
DEJA_F_DECL void DejaField( const unsigned char&      ); 
DEJA_F_DECL void DejaField( const          wchar_t&   );
DEJA_F_DECL void DejaField( const   signed short&     ); 
DEJA_F_DECL void DejaField( const unsigned short&     ); 
DEJA_F_DECL void DejaField( const   signed int&       );
DEJA_F_DECL void DejaField( const unsigned int&       ); 
DEJA_F_DECL void DejaField( const   signed long&      );
DEJA_F_DECL void DejaField( const unsigned long&      ); 
DEJA_F_DECL void DejaField( const   signed long long& );
DEJA_F_DECL void DejaField( const unsigned long long& ); 

//------------------------------------------------------------------------------

DEJA_F_DECL void DejaDescriptor(          void* const&     ); 
DEJA_F_DECL void DejaDescriptor( const          void*      ); 
DEJA_F_DECL void DejaDescriptor( const          bool&      ); 
DEJA_F_DECL void DejaDescriptor( const          float&     ); 
DEJA_F_DECL void DejaDescriptor( const          double&    ); 
DEJA_F_DECL void DejaDescriptor( const          char&      ); 
DEJA_F_DECL void DejaDescriptor( const   signed char&      ); 
DEJA_F_DECL void DejaDescriptor( const unsigned char&      ); 
DEJA_F_DECL void DejaDescriptor( const          wchar_t&   );
DEJA_F_DECL void DejaDescriptor( const   signed short&     ); 
DEJA_F_DECL void DejaDescriptor( const unsigned short&     ); 
DEJA_F_DECL void DejaDescriptor( const   signed int&       );
DEJA_F_DECL void DejaDescriptor( const unsigned int&       ); 
DEJA_F_DECL void DejaDescriptor( const   signed long&      );
DEJA_F_DECL void DejaDescriptor( const unsigned long&      ); 
DEJA_F_DECL void DejaDescriptor( const   signed long long& );
DEJA_F_DECL void DejaDescriptor( const unsigned long long& ); 

//==============================================================================

template< class T, int N >
void DejaDescriptor( const T(&V)[N] )
{
    DejaArraySize = N;
    DejaDescriptor( V[0] );
}

//==============================================================================
//  MACROS
//==============================================================================

//==============================================================================
#ifndef DEJA_DISABLED
//==============================================================================

//------------------------------------------------------------------------------
//  Utility
//------------------------------------------------------------------------------

//  The compiler in Visual Studio.NET 2003 does not properly compile constructs
//  using the token splice preprocessor directive (##) and the predefined line 
//  number macro (__LINE__).  This defect is present in SP1 as well.  The macro
//  __COUNTER__ can be used as a work-around as long as the generated symbol is
//  only needed once.  Also, the Edit and Continue features in Visual Studio
//  are "unhappy" when static variables are renamed, which can happen if the
//  __LINE__ macro is used to create unique names.  So, on Microsoft compilers
//  use the Microsoft specific macro __COUNTER__ and be certain not to need to
//  reference the generated unique name more than once.

#if defined( _MSC_VER )

#define DEJA_UNIQUE_NAME(a)  DEJA_SPLICE1( a, __COUNTER__ )
#define DEJA_SPLICE1(a,b)    DEJA_SPLICE2( a, b )
#define DEJA_SPLICE2(a,b)    a ## b

#else

#define DEJA_UNIQUE_NAME(a)  DEJA_SPLICE1( a, __LINE__ )
#define DEJA_SPLICE1(a,b)    DEJA_SPLICE2( a, b )
#define DEJA_SPLICE2(a,b)    a ## b

#endif

//------------------------------------------------------------------------------
//  General
//------------------------------------------------------------------------------

#define DEJA_FLUSH                  DejaFlush
#define DEJA_APP_LABEL              DejaAppLabel
#define DEJA_THREAD_LABEL           DejaThreadLabel
#define DEJA_SET_PARAMETER          DejaSetParameter
#define DEJA_NEXT_FRAME             DejaNextFrame
#define DEJA_SET_AUTO_FLUSH( F )    DejaSetAutoFlush( F, __FILE__, __LINE__ );
#define DEJA_LIB_CONNECTED          DejaLibConnected
#define DEJA_LAST_ERROR             DejaGetLastError
#define DEJA_COMMAND                DejaCommand
#define DEJA_SET_VALUE( N, V )      DejaSetValue( N, V, __FILE__, __LINE__ );

#define DEJA_SET_MALLOC             DejaSetMalloc
#define DEJA_SET_REALLOC            DejaSetRealloc
#define DEJA_SET_FREE               DejaSetFree

#define DEJA_LOG_MALLOC             DejaLogMalloc
#define DEJA_LOG_REALLOC            DejaLogRealloc
#define DEJA_LOG_FREE               DejaLogFree
#define DEJA_LOG_NEW                DejaLogNew 
#define DEJA_LOG_DELETE             DejaLogDelete
#define DEJA_LOG_NEW_ARRAY          DejaLogNewArray
#define DEJA_LOG_DELETE_ARRAY       DejaLogDeleteArray

#define DEJA_ENUM_VALUE( T, V )     DejaEnumAlias( #T, V, #V, false )
#define DEJA_ENUM_ALIAS( T, V, A )  DejaEnumAlias( #T, V,  A, true  )

#define DEJA_SCOPE_OBJECT( O )                                                 \
    deja_object_scope DEJA_UNIQUE_NAME(ObjScope)( &O, __FILE__, __LINE__ );

#define DEJA_THREAD_INIT( )                                                    \
    if( !DejaSystemActive ) { } else                                           \
    {                                                                          \
        DejaLock( __FILE__, __LINE__ );                                        \
        DejaThreadInit();                                                      \
        DejaUnlock();                                                          \
    }

#define DEJA_THREAD_KILL( Code )                                               \
    if( !DejaSystemActive ) { } else                                           \
    {                                                                          \
        DejaLock( __FILE__, __LINE__ );                                        \
        DejaThreadKill( Code );                                                \
        DejaUnlock();                                                          \
    }

#define DEJA_TERMINATE( )                                                      \
    if( !DejaSystemActive ) { } else                                           \
    {                                                                          \
        DejaLock( __FILE__, __LINE__ );                                        \
        DejaTerminate();                                                       \
        DejaUnlock();                                                          \
    }

//------------------------------------------------------------------------------
//  Logging
//------------------------------------------------------------------------------

//====================
#ifdef VARIADIC_MACROS
//--------------------

#define DEJA_CREATE_LOG_FLAG( c, f ) static deja_log_flag DejaLogFlag( #c, #f )

#define DEJA_TRACE( Channel, Format, ... )                                     \
    if( DejaSystemActive )                                                     \
    {                                                                          \
        DEJA_CREATE_LOG_FLAG( Channel, Format );                               \
        if( DejaLogFlag.On )                                                   \
        {                                                                      \
            DejaLock( __FILE__, __LINE__, &DejaLogFlag, 0 );                   \
            DejaLog( Channel, Format , ## __VA_ARGS__ );                       \
        }                                                                      \
        else                                                                   \
        {                                                                      \
        }                                                                      \
    }                                                                          \
    else                                                                       \
    {                                                                          \
    }

#define DEJA_WARNING( Channel, Format, ... )                                   \
    if( DejaSystemActive )                                                     \
    {                                                                          \
        DEJA_CREATE_LOG_FLAG( Channel, Format );                               \
        if( DejaLogFlag.On )                                                   \
        {                                                                      \
            DejaLock( __FILE__, __LINE__, &DejaLogFlag, 1 );                   \
            DejaLog( Channel, Format , ## __VA_ARGS__ );                       \
        }                                                                      \
        else                                                                   \
        {                                                                      \
        }                                                                      \
    }                                                                          \
    else                                                                       \
    {                                                                          \
    }

#define DEJA_ERROR( Channel, Format, ... )                                     \
    if( DejaSystemActive )                                                     \
    {                                                                          \
        DEJA_CREATE_LOG_FLAG( Channel, Format );                               \
        if( DejaLogFlag.On )                                                   \
        {                                                                      \
            DejaLock( __FILE__, __LINE__, &DejaLogFlag, 2 );                   \
            DejaLog( Channel, Format , ## __VA_ARGS__ );                       \
        }                                                                      \
        else                                                                   \
        {                                                                      \
        }                                                                      \
    }                                                                          \
    else                                                                       \
    {                                                                          \
    }

#define DEJA_BOOKMARK( Channel, Format, ... )                                  \
    if( DejaSystemActive )                                                     \
    {                                                                          \
        DEJA_CREATE_LOG_FLAG( Channel, Format );                               \
        if( DejaLogFlag.On )                                                   \
        {                                                                      \
            DejaLock( __FILE__, __LINE__, &DejaLogFlag, 3 );                   \
            DejaLog( Channel, Format , ## __VA_ARGS__ );                       \
        }                                                                      \
        else                                                                   \
        {                                                                      \
        }                                                                      \
    }                                                                          \
    else                                                                       \
    {                                                                          \
    }

//-----------------------
#endif // VARIADIC_MACROS
//=======================

//=====================
#ifndef VARIADIC_MACROS
//---------------------

#if defined( _MSC_VER ) && (_MSC_VER >= 1300) && (_MSC_VER < 1400)

////
//// if( Microsoft Visual Studio.NET 2003 )  (which doesn't have variadic)
////
//  The compiler in Visual Studio.NET 2003 does not properly compile constructs
//  using the token splice preprocessor directive (##) and the predefined line 
//  number macro (__LINE__).  This defect is present in SP1 as well.  The 
//  standard work-around is to use __COUNTER__ instead.  But, when using the
//  __COUNTER__ macro, you cannot refer to a symbol more than once.  This
//  version of the DEJA_TRACE macro set is less efficient, but it completely
//  avoids the issue by isolating the static variable to a localized scope.

#define DEJA_TRACE          {                                                  \
                                static deja_log_flag LogFlag;                  \
                                DejaLock( __FILE__, __LINE__, &LogFlag, 0 );   \
                            }                                                  \
                            DejaLog_VS7 

#define DEJA_WARNING        {                                                  \
                                static deja_log_flag LogFlag;                  \
                                DejaLock( __FILE__, __LINE__, &LogFlag, 1 );   \
                            }                                                  \
                            DejaLog_VS7 

#define DEJA_ERROR          {                                                  \
                                static deja_log_flag LogFlag;                  \
                                DejaLock( __FILE__, __LINE__, &LogFlag, 2 );   \
                            }                                                  \
                            DejaLog_VS7 

#define DEJA_BOOKMARK       {                                                  \
                                static deja_log_flag LogFlag;                  \
                                DejaLock( __FILE__, __LINE__, &LogFlag, 3 );   \
                            }                                                  \
                            DejaLog_VS7 

#else

////
//// NOT( Microsoft Visual Studio.NET 2003 ) and NOT( variadic macro enabled )
////

//------------------------------------------------------------------------------
//
//  This form of the logging functions is slightly sensitive to syntactic 
//  placement.  It is composed of multiple statements, but it cannot be wrapped
//  in scope braces.  Otherwise, the parameters would not bind to the DejaLog
//  call.  Consider:
//
//      if( Expression )
//          DEJA_TRACE( ... );
//      else
//          ASSERT( false );
//
//  This example would compile; but, the ASSERT would always be executed, 
//  regardless of the value of Expression.
//
//  Also, this form can present problems under Microsoft compilers when not
//  enclosed in braces in switch case clauses.  The following code will generate
//  error C2361 at compile:
//
//      switch( Value )
//      {
//      case 1:     This();         break;
//      case 2:     That();         break;
//      case 3:     TheOther();     break;
//      default:    ASSERT(false);  break;    
//      }
//
//  Similarly, the following code will generate error C4533 at compile:
//
//      void Function( void )
//      {
//          if( Expression )
//              goto jump;
//
//          ASSERT( Condition );
//
//      jump:
//          // Do something relevant here.
//      }
//
//  (Obviously, this example is contrived and could be implemented without using
//  a goto statement.)
//
//  Both examples have a workaround.  Simply wrap the ASSERT statements in scope
//  braces, like so:
//
//      { ASSERT(false); }
//
//      { ASSERT( Condition ); }
//
//------------------------------------------------------------------------------

#define DEJA_TRACE          { }                                                \
                            static deja_log_flag DEJA_UNIQUE_NAME(LogFlag);    \
                            if( !DEJA_UNIQUE_NAME(LogFlag).On ||               \
                                !DejaSystemActive )                            \
                                { }                                            \
                            else                                               \
                                DejaLock( __FILE__, __LINE__,                  \
                                          &DEJA_UNIQUE_NAME(LogFlag), 0 ),     \
                                DejaLog

#define DEJA_WARNING        { }                                                \
                            static deja_log_flag DEJA_UNIQUE_NAME(LogFlag);    \
                            if( !DEJA_UNIQUE_NAME(LogFlag).On ||               \
                                !DejaSystemActive )                            \
                                { }                                            \
                            else                                               \
                                DejaLock( __FILE__, __LINE__,                  \
                                          &DEJA_UNIQUE_NAME(LogFlag), 1 ),     \
                                DejaLog

#define DEJA_ERROR          { }                                                \
                            static deja_log_flag DEJA_UNIQUE_NAME(LogFlag);    \
                            if( !DEJA_UNIQUE_NAME(LogFlag).On ||               \
                                !DejaSystemActive )                            \
                                { }                                            \
                            else                                               \
                                DejaLock( __FILE__, __LINE__,                  \
                                          &DEJA_UNIQUE_NAME(LogFlag), 2 ),     \
                                DejaLog

#define DEJA_BOOKMARK       { }                                                \
                            static deja_log_flag DEJA_UNIQUE_NAME(LogFlag);    \
                            if( !DEJA_UNIQUE_NAME(LogFlag).On ||               \
                                !DejaSystemActive )                            \
                                { }                                            \
                            else                                               \
                                DejaLock( __FILE__, __LINE__,                  \
                                          &DEJA_UNIQUE_NAME(LogFlag), 3 ),     \
                                DejaLog

#endif // Work-arounds specific to Visual Studio.NET 2003.

//------------------------
#endif // !VARIADIC_MACROS
//========================

//------------------------------------------------------------------------------
//  Static 3D
//------------------------------------------------------------------------------
                            
#define DEJA_3D_CHANNEL     if( !DejaSystemActive ) { } else DejaLock( __FILE__, __LINE__ ), Deja3DChannel
#define DEJA_3D_COLOR       if( !DejaSystemActive ) { } else DejaLock( __FILE__, __LINE__ ), Deja3DColor
#define DEJA_3D_POINT       if( !DejaSystemActive ) { } else DejaLock( __FILE__, __LINE__ ), Deja3DPoint
#define DEJA_3D_POINT_LIST  if( !DejaSystemActive ) { } else DejaLock( __FILE__, __LINE__ ), Deja3DPointList
#define DEJA_3D_LINE        if( !DejaSystemActive ) { } else DejaLock( __FILE__, __LINE__ ), Deja3DLine
#define DEJA_3D_LINE_LIST   if( !DejaSystemActive ) { } else DejaLock( __FILE__, __LINE__ ), Deja3DLineList
#define DEJA_3D_LINE_STRIP  if( !DejaSystemActive ) { } else DejaLock( __FILE__, __LINE__ ), Deja3DLineStrip
#define DEJA_3D_TRI         if( !DejaSystemActive ) { } else DejaLock( __FILE__, __LINE__ ), Deja3DTri
#define DEJA_3D_TRI_LIST    if( !DejaSystemActive ) { } else DejaLock( __FILE__, __LINE__ ), Deja3DTriList
#define DEJA_3D_TRI_STRIP   if( !DejaSystemActive ) { } else DejaLock( __FILE__, __LINE__ ), Deja3DTriStrip
#define DEJA_3D_MARKER      if( !DejaSystemActive ) { } else DejaLock( __FILE__, __LINE__ ), Deja3DMarker
#define DEJA_3D_SPHERE      if( !DejaSystemActive ) { } else DejaLock( __FILE__, __LINE__ ), Deja3DSphere
#define DEJA_3D_BOX_LOCAL   if( !DejaSystemActive ) { } else DejaLock( __FILE__, __LINE__ ), Deja3DBoxLocal
#define DEJA_3D_BOX         if( !DejaSystemActive ) { } else DejaLock( __FILE__, __LINE__ ), Deja3DBox

//------------------------------------------------------------------------------
//  Context
//------------------------------------------------------------------------------

#define DEJA_CONTEXT_REGISTRATION( C )                                         \
    static deja_context_reg DejaContextReg( #C, __FILE__, __LINE__ );

#define DEJA_CONTEXT( ContextLabel )                                           \
    DEJA_CONTEXT_REGISTRATION( ContextLabel )                                  \
    deja_context DejaContext( &DejaContextReg )

//------------------------------------------------------------------------------
//  Data posting
//------------------------------------------------------------------------------

#define DEJA_OBJECT_INIT( Object )                                             \
    if( !DejaSystemActive ) { } else                                           \
    {                                                                          \
        DejaLock( __FILE__, __LINE__ );                                        \
        DejaPostPhase = 2;                                                     \
        DejaObjectInfo( &Object, #Object );                                    \
        DejaDescriptor( Object );                                              \
        DejaObjectInit();                                                      \
        DejaUnlock();                                                          \
    }

//------------------------------------------------------------------------------

#define DEJA_OBJECT_KILL( Object )                                             \
    if( !DejaSystemActive ) { } else                                           \
    {                                                                          \
        DejaLock( __FILE__, __LINE__ );                                        \
        DejaObjectInfo( &Object, #Object );                                    \
        DejaObjectKill();                                                      \
        DejaUnlock();                                                          \
    }

//------------------------------------------------------------------------------
#ifdef VARIADIC_MACROS
//--------------------

#define DEJA_OBJECT_LABEL( Object, Label, ... )                                \
    if( !DejaSystemActive ) { } else                                           \
    {                                                                          \
        DejaLock( __FILE__, __LINE__ );                                        \
        DejaObjectLabel( &Object, Label , ## __VA_ARGS__ );                    \
        DejaUnlock();                                                          \
    }

//----------------------
#else // VARIADIC_MACROS
//----------------------

#define DEJA_OBJECT_LABEL                                                      \
    DejaLock( __FILE__, __LINE__ ), DejaObjectLabelT

template< class T >
void DejaObjectLabelT( const T& Object, const char* pNLabelFormat, ... )
{
    va_list args;
    va_start( args, pNLabelFormat );
    DejaObjectLabel( (void*)&Object, pNLabelFormat, NULL, args );
    DejaUnlock();
}

template< class T >
void DejaObjectLabelT( const T& Object, const wchar_t* pWLabelFormat, ... )
{
    va_list args;
    va_start( args, pWLabelFormat );
    DejaObjectLabel( (void*)&Object, NULL, pWLabelFormat, args );
    DejaUnlock();
}

//-----------------------
#endif // VARIADIC_MACROS
//------------------------------------------------------------------------------

#define DEJA_POST( Object )                                                    \
    if( !DejaSystemActive ) { } else                                           \
    {                                                                          \
        DejaLock( __FILE__, __LINE__ );                                        \
        DejaPostPhase  = 3;                                                    \
        DejaObjectInfo( &Object, #Object );                                    \
        DejaDescriptor( Object );                                              \
        DejaAtomicFlag = false;  /* Atomic posts can't clear this. */          \
        DejaArraySize  = 1;      /* Atomic posts can't clear this. */          \
        DejaPostFlush();                                                       \
        DejaAutoFlush();                                                       \
        DejaUnlock();                                                          \
    }

//------------------------------------------------------------------------------

#define DEJA_TYPE( Object, Type )                                              \
    if( !DejaSystemActive )                                                    \
    {                                                                          \
       return;                                                                 \
    }                                                                          \
    void (*pDescriptor)( const Type& ) = DejaDescriptor;                       \
    static deja_type DejaType( (deja_d_fn*)pDescriptor, sizeof(Type), #Type ); \
    if( !DejaPostLogic( DejaType, (void*)&Object ) )                           \
        return;                                                                \
    DejaType.pNext = (void*)( ((Type*)(&Object)) + 1 );                        \
    deja_owner Owner( DejaType );

//------------------------------------------------------------------------------

#define DEJA_BASE( Object, BaseType )                                          \
    {                                                                          \
        const void* pOldFieldAddr    = DejaFieldAddr;                          \
        const char* pOldFieldName    = DejaFieldName;                          \
        const char* pOldEnumTypeName = DejaEnumTypeName;                       \
        int         OldArraySize     = DejaArraySize;                          \
        int         OldTypeSeq       = DejaTypeSeq;                            \
        bool        OldAtomicFlag    = DejaAtomicFlag;                         \
                                                                               \
        DejaFieldAddr    = (void*)( (BaseType*)(&(Object)) );                  \
        DejaFieldName    = #BaseType;                                          \
        DejaArraySize    = 1;                                                  \
        DejaEnumTypeName = NULL;                                               \
                                                                               \
        DejaField( *(BaseType*)(&(Object)) );                                  \
                                                                               \
        DejaFieldAddr    = pOldFieldAddr;                                      \
        DejaFieldName    = pOldFieldName;                                      \
        DejaEnumTypeName = pOldEnumTypeName;                                   \
        DejaArraySize    = OldArraySize;                                       \
        DejaTypeSeq      = OldTypeSeq;                                         \
        DejaAtomicFlag   = OldAtomicFlag;                                      \
}

//------------------------------------------------------------------------------

#define DEJA_FIELD( Field )                                                    \
    {                                                                          \
        const void* pOldFieldAddr    = DejaFieldAddr;                          \
        const char* pOldFieldName    = DejaFieldName;                          \
        const char* pOldEnumTypeName = DejaEnumTypeName;                       \
        int         OldArraySize     = DejaArraySize;                          \
        int         OldTypeSeq       = DejaTypeSeq;                            \
        bool        OldAtomicFlag    = DejaAtomicFlag;                         \
                                                                               \
        DejaFieldAddr    = &(Field);                                           \
        DejaFieldName    = #Field;                                             \
        DejaArraySize    = 1;                                                  \
        DejaEnumTypeName = NULL;                                               \
                                                                               \
        DejaField( Field );                                                    \
                                                                               \
        DejaFieldAddr    = pOldFieldAddr;                                      \
        DejaFieldName    = pOldFieldName;                                      \
        DejaEnumTypeName = pOldEnumTypeName;                                   \
        DejaArraySize    = OldArraySize;                                       \
        DejaTypeSeq      = OldTypeSeq;                                         \
        DejaAtomicFlag   = OldAtomicFlag;                                      \
    }

//------------------------------------------------------------------------------

#define DEJA_ENUM( Field, EnumType )                                           \
    {                                                                          \
        const void* pOldFieldAddr    = DejaFieldAddr;                          \
        const char* pOldFieldName    = DejaFieldName;                          \
        const char* pOldEnumTypeName = DejaEnumTypeName;                       \
        int         OldArraySize     = DejaArraySize;                          \
        int         OldTypeSeq       = DejaTypeSeq;                            \
        bool        OldAtomicFlag    = DejaAtomicFlag;                         \
                                                                               \
        DejaFieldAddr    = &(Field);                                           \
        DejaFieldName    = #Field;                                             \
        DejaArraySize    = 1;                                                  \
        DejaEnumTypeName = #EnumType;                                          \
                                                                               \
        DejaField( Field );                                                    \
                                                                               \
        DejaFieldAddr    = pOldFieldAddr;                                      \
        DejaFieldName    = pOldFieldName;                                      \
        DejaEnumTypeName = pOldEnumTypeName;                                   \
        DejaArraySize    = OldArraySize;                                       \
        DejaTypeSeq      = OldTypeSeq;                                         \
        DejaAtomicFlag   = OldAtomicFlag;                                      \
    }

//------------------------------------------------------------------------------

#define DEJA_VLARRAY( Pointer, Count )                                         \
    {                                                                          \
        const void* pOldFieldAddr    = DejaFieldAddr;                          \
        const char* pOldFieldName    = DejaFieldName;                          \
        const char* pOldEnumTypeName = DejaEnumTypeName;                       \
        int         OldArraySize     = DejaArraySize;                          \
        int         OldTypeSeq       = DejaTypeSeq;                            \
        bool        OldAtomicFlag    = DejaAtomicFlag;                         \
        bool        OldIfPtrFlag     = DejaIfPtrFlag;                          \
                                                                               \
        DejaFieldAddr    = Pointer;                                            \
        DejaFieldName    = #Pointer;                                           \
        DejaArraySize    = Count;                                              \
        DejaEnumTypeName = NULL;                                               \
        DejaIfPtrFlag    = false;                                              \
                                                                               \
        DejaVLAField( Pointer );                                               \
                                                                               \
        DejaFieldAddr    = pOldFieldAddr;                                      \
        DejaFieldName    = pOldFieldName;                                      \
        DejaEnumTypeName = pOldEnumTypeName;                                   \
        DejaArraySize    = OldArraySize;                                       \
        DejaTypeSeq      = OldTypeSeq;                                         \
        DejaAtomicFlag   = OldAtomicFlag;                                      \
        DejaIfPtrFlag    = OldIfPtrFlag;                                       \
    }

//------------------------------------------------------------------------------

#define DEJA_IF_PTR( Pointer )                                                 \
    {                                                                          \
        const void* pOldFieldAddr    = DejaFieldAddr;                          \
        const char* pOldFieldName    = DejaFieldName;                          \
        const char* pOldEnumTypeName = DejaEnumTypeName;                       \
        int         OldArraySize     = DejaArraySize;                          \
        int         OldTypeSeq       = DejaTypeSeq;                            \
        bool        OldAtomicFlag    = DejaAtomicFlag;                         \
        bool        OldIfPtrFlag     = DejaIfPtrFlag;                          \
                                                                               \
        DejaFieldAddr    = Pointer;                                            \
        DejaFieldName    = #Pointer;                                           \
        DejaArraySize    = (Pointer ? 1 : 0);                                  \
        DejaEnumTypeName = NULL;                                               \
        DejaIfPtrFlag    = true;                                               \
                                                                               \
        DejaVLAField( Pointer );                                               \
                                                                               \
        DejaFieldAddr    = pOldFieldAddr;                                      \
        DejaFieldName    = pOldFieldName;                                      \
        DejaEnumTypeName = pOldEnumTypeName;                                   \
        DejaArraySize    = OldArraySize;                                       \
        DejaTypeSeq      = OldTypeSeq;                                         \
        DejaAtomicFlag   = OldAtomicFlag;                                      \
        DejaIfPtrFlag    = OldIfPtrFlag;                                       \
    }

//------------------------------------------------------------------------------

#define DEJA_CSTRING( pString ) DEJA_VLARRAY( pString, DejaStrLen( pString ) );

//------------------------------------------------------------------------------
// The DEJA_ARRAY command is deprecated.  Use DEJA_FIELD going forward.  The 
// following definition of DEJA_ARRAY provides backwards compatibility.  It will
// be removed in a near future version.

#define DEJA_ARRAY( StaticArray ) DEJA_FIELD( StaticArray )

//==============================================================================
#else // DEJA_DISABLED
//==============================================================================

    //==========================================================================
    #ifdef VARIADIC_MACROS
    //==========================================================================

    //--------------------------------------------------------------------------
    //  General
    //--------------------------------------------------------------------------

    #define DEJA_FLUSH( ... )
    #define DEJA_APP_LABEL( ... )
    #define DEJA_THREAD_LABEL( ... )
    #define DEJA_SET_PARAMETER( ... )
    #define DEJA_NEXT_FRAME( ... )
    #define DEJA_SET_AUTO_FLUSH( ... )
    #define DEJA_LIB_CONNECTED( )       false
    #define DEJA_LAST_ERROR( )          ""
    #define DEJA_COMMAND( ... )
    #define DEJA_SET_VALUE( ... )

    #define DEJA_SET_MALLOC( ... )
    #define DEJA_SET_REALLOC( ... )
    #define DEJA_SET_FREE( ... )

    #define DEJA_LOG_MALLOC( ... )
    #define DEJA_LOG_REALLOC( ... )
    #define DEJA_LOG_FREE( ... )
    #define DEJA_LOG_NEW( ... )
    #define DEJA_LOG_DELETE( ... )
    #define DEJA_LOG_NEW_ARRAY( ... )
    #define DEJA_LOG_DELETE_ARRAY( ... )

    #define DEJA_ENUM_VALUE( ... )
    #define DEJA_ENUM_ALIAS( ... )

    #define DEJA_SCOPE_OBJECT( ... )    

    #define DEJA_THREAD_INIT( ... )
    #define DEJA_THREAD_KILL( ... )
    #define DEJA_THREAD_KILL( ... )
    #define DEJA_TERMINATE( ... )

    //--------------------------------------------------------------------------
    //  Logging
    //--------------------------------------------------------------------------

    #define DEJA_TRACE(    Channel, Format, ... )
    #define DEJA_WARNING(  Channel, Format, ... )
    #define DEJA_ERROR(    Channel, Format, ... )
    #define DEJA_BOOKMARK( Channel, Format, ... )

    //--------------------------------------------------------------------------
    //  Static 3D
    //--------------------------------------------------------------------------
                                
    #define DEJA_3D_CHANNEL( ... )
    #define DEJA_3D_COLOR( ... )
    #define DEJA_3D_POINT( ... )
    #define DEJA_3D_POINT_LIST( ... )
    #define DEJA_3D_LINE( ... )
    #define DEJA_3D_LINE_LIST( ... )
    #define DEJA_3D_LINE_STRIP( ... )
    #define DEJA_3D_TRI( ... )
    #define DEJA_3D_TRI_LIST( ... )
    #define DEJA_3D_TRI_STRIP( ... )
    #define DEJA_3D_MARKER( ... )
    #define DEJA_3D_SPHERE( ... )
    #define DEJA_3D_BOX_LOCAL( ... )
    #define DEJA_3D_BOX( ... )

    //--------------------------------------------------------------------------
    //  Context
    //--------------------------------------------------------------------------

    #define DEJA_CONTEXT( ... )           

    //--------------------------------------------------------------------------
    //  Data posting
    //--------------------------------------------------------------------------

    #define DEJA_OBJECT_INIT( ... )
    #define DEJA_OBJECT_KILL( ... )
    #define DEJA_OBJECT_LABEL( ... )
    #define DEJA_POST( ... )
    #define DEJA_TYPE( I, T )           (void)I
    #define DEJA_BASE( ... )
    #define DEJA_FIELD( ... ) 
    #define DEJA_ENUM( ... )
    #define DEJA_ARRAY( ... )
    #define DEJA_VLARRAY( ... )
    #define DEJA_CSTRING( ... )
    #define DEJA_IF_PTR( ... )

    //==========================================================================
    #else // #ifdef VARIADIC_MACROS
    //==========================================================================

    #define DEJA_NULL_FN    __noop

    //--------------------------------------------------------------------------
    //  General
    //--------------------------------------------------------------------------

    #define DEJA_FLUSH                  DEJA_NULL_FN
    #define DEJA_APP_LABEL              DEJA_NULL_FN
    #define DEJA_THREAD_LABEL           DEJA_NULL_FN
    #define DEJA_SET_PARAMETER( a, b )
    #define DEJA_NEXT_FRAME( )
    #define DEJA_SET_AUTO_FLUSH         DEJA_NULL_FN
    #define DEJA_LIB_CONNECTED( )       false
    #define DEJA_LAST_ERROR( )          ""
    #define DEJA_COMMAND                DEJA_NULL_FN
    #define DEJA_SET_VALUE              DEJA_NULL_FN

    #define DEJA_SET_MALLOC             DEJA_NULL_FN
    #define DEJA_SET_REALLOC            DEJA_NULL_FN
    #define DEJA_SET_FREE               DEJA_NULL_FN

    #define DEJA_LOG_MALLOC             DEJA_NULL_FN
    #define DEJA_LOG_REALLOC            DEJA_NULL_FN
    #define DEJA_LOG_FREE               DEJA_NULL_FN
    #define DEJA_LOG_NEW                DEJA_NULL_FN
    #define DEJA_LOG_DELETE             DEJA_NULL_FN
    #define DEJA_LOG_NEW_ARRAY          DEJA_NULL_FN
    #define DEJA_LOG_DELETE_ARRAY       DEJA_NULL_FN

    #define DEJA_ENUM_VALUE( T, V )     
    #define DEJA_ENUM_ALIAS( T, V, A )  

    #define DEJA_SCOPE_OBJECT( O )    

    #define DEJA_THREAD_INIT( )
    #define DEJA_THREAD_KILL( Code )
    #define DEJA_TERMINATE( )

    //--------------------------------------------------------------------------
    //  Logging
    //--------------------------------------------------------------------------

    #define DEJA_TRACE                  DEJA_NULL_FN
    #define DEJA_WARNING                DEJA_NULL_FN
    #define DEJA_ERROR                  DEJA_NULL_FN
    #define DEJA_BOOKMARK               DEJA_NULL_FN

    //--------------------------------------------------------------------------
    //  Static 3D
    //--------------------------------------------------------------------------
                                
    #define DEJA_3D_CHANNEL             DEJA_NULL_FN
    #define DEJA_3D_COLOR               DEJA_NULL_FN
    #define DEJA_3D_POINT               DEJA_NULL_FN
    #define DEJA_3D_POINT_LIST          DEJA_NULL_FN
    #define DEJA_3D_LINE                DEJA_NULL_FN
    #define DEJA_3D_LINE_LIST           DEJA_NULL_FN
    #define DEJA_3D_LINE_STRIP          DEJA_NULL_FN
    #define DEJA_3D_TRI                 DEJA_NULL_FN
    #define DEJA_3D_TRI_LIST            DEJA_NULL_FN
    #define DEJA_3D_TRI_STRIP           DEJA_NULL_FN
    #define DEJA_3D_MARKER              DEJA_NULL_FN
    #define DEJA_3D_SPHERE              DEJA_NULL_FN
    #define DEJA_3D_BOX_LOCAL           DEJA_NULL_FN
    #define DEJA_3D_BOX                 DEJA_NULL_FN

    //--------------------------------------------------------------------------
    //  Context
    //--------------------------------------------------------------------------

    #define DEJA_CONTEXT( C )           

    //--------------------------------------------------------------------------
    //  Data posting
    //--------------------------------------------------------------------------

    #define DEJA_OBJECT_INIT( E )
    #define DEJA_OBJECT_KILL( E )
    #define DEJA_OBJECT_LABEL           DEJA_NULL_FN
    #define DEJA_POST( V )
    #define DEJA_TYPE( I, T )           (void)I
    #define DEJA_BASE( I, T )
    #define DEJA_FIELD( F )  
    #define DEJA_ENUM( T, F )
    #define DEJA_ARRAY( A )  
    #define DEJA_VLARRAY( A, C )
    #define DEJA_CSTRING( S )
    #define DEJA_IF_PTR( P )

    //==========================================================================
    #endif // #ifdef VARIADIC_MACROS
    //==========================================================================

//==============================================================================
#endif // #ifndef DEJA_DISABLED
//==============================================================================

//==============================================================================
#endif // #ifndef DEJA_PRIVATE_H
//==============================================================================
