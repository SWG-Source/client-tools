//
// This file was changed during internationalization on 12/12/2000 by Jens.


#ifndef _INC_ERROR_MACROS_H_
#define _INC_ERROR_MACROS_H_



/* ***************************************************************************
	ERROR HELPER MACROS
****************************************************************************/

#include <crtdbg.h>		// _ASSERTE


#define NXN_ASSERT(booleanExpresssion)				{ _ASSERTE(booleanExpresssion); }
#define NXN_ASSERT_MSG(booleanExpresssion, message) { _ASSERTE(booleanExpresssion); }
#define NXN_VERIFY(booleanExpresssion)				{ VERIFY(booleanExpresssion); }
#ifdef _DEBUG 
	#define NXN_IF_DEBUG(a) a
#else
	#define NXN_IF_DEBUG(a) {}
#endif




//!!RF: work in progress
//#ifdef NXN_LOG_ERROR2S_ON
//#include <nynutil.h>
#define NXN_LOG_ERROR2(a,hResult)  {}
/*
\
	{ \
		CString sOperation (_T(#a)); \
		CString sError(NyNGetErrorMessage (hResult));  \
		CString sMessage;  \
		sMessage.Format ( _T("%s failed\nError: %s"), (const char*)sOperation, (const char*)sError ); \
		NxNLogError ( (const char*)_T(#a)); \
	}
*/

//#else //!NXN_LOG_ERROR2S_ON

//	#define NXN_LOG_ERROR2(a)  {}

//#endif

//#define NXN_CREATE_STRUCT(st,v) st v; memset(&v,0,sizeof(st)); 





// throws HRESULT if some expression is true, allows catch handler with undo for multiple actions in one block 
#define THROW_IF_NOT_S_OK( x )		{ HRESULT _Result_ = (x); if( _Result_!=S_OK )	{ NXN_LOG_ERROR2(x,_Result_) throw( _Result_ ); } }
#define THROW_IF_FAILED( x )		{ HRESULT _Result_ = (x); if( FAILED(_Result_) ){ NXN_LOG_ERROR2(x,_Result_) throw( _Result_ ); } }
#define THROW_IF_NULL( x )			{ if( (x)==0 ) { NXN_ASSERT(!"NULL parameter"); throw( E_POINTER ); } }
#define THROW_IF( x, Result )		{ if (x) { NXN_LOG_ERROR2(x,Result) throw( Result ); } }
#define THROW_IF_ERROR(a)			THROW_IF_NOT_S_OK(a)

// returns a HRESULT if some expression is true
#define RETURN_IF_NOT_S_OK( x )		{ HRESULT _Result_ = (x); if( _Result_!=S_OK )	{ NXN_LOG_ERROR2(x,_Result_) return( _Result_ ); } }
#define RETURN_IF_S_OK( x )			{ HRESULT _Result_ = (x); if( _Result_==S_OK )	{ NXN_LOG_ERROR2(x,_Result_) return( _Result_ ); } }
#define RETURN_IF_FAILED( x )		{ HRESULT _Result_ = (x); if( FAILED(_Result_) ){ NXN_LOG_ERROR2(x,_Result_) return( _Result_ ); } }
#define RETURN_IF_SUCCEEDED( x )	{ HRESULT _Result_ = (x); if( SUCCEEDED(_Result_) ) { NXN_LOG_ERROR2(x,_Result_) return( _Result_ ); } }
#define RETURN_IF_NULL( x )			{ if( (x)==0 ) { NXN_ASSERT(!"NULL parameter"); return( E_POINTER ); } }
#define RETURN_IF( x, Result )		{ if (x) { NXN_LOG_ERROR2(x,Result) return( Result ); } }
#define RETURN_IF_ERROR(a)			RETURN_IF_NOT_S_OK(a)

// same as above, but for void methods
#define RETURN_VOID_IF_NOT_S_OK( x )	{ if( (x)!=S_OK ) { NXN_LOG_ERROR2(x,_Result_) return; } }
#define RETURN_VOID_IF_S_OK( x )		{ if( (x)==S_OK ) { NXN_LOG_ERROR2(x,_Result_) return; } }
#define RETURN_VOID_IF_FAILED( x )		{ if( FAILED(x) ) { NXN_LOG_ERROR2(x,_Result_) return; } }
#define RETURN_VOID_IF_SUCCEEDED( x )	{ if( SUCCEEDED(x) ) { NXN_LOG_ERROR2(x,_Result_) return; } } 
#define RETURN_VOID_IF_NULL( x )		{ if( (x)==0 ) { NXN_ASSERT(!"NULL parameter"); return; } }
#define RETURN_VOID_IF( x )				{ if (x) { return; } }
#define RETURN_VOID_IF_ERROR( x )		RETURN_VOID_IF_NOT_S_OK(x)

//_021
#ifdef _DEBUG
	#define ASSERT_OR_RETURN(Term,ReturnCode) NXN_ASSERT (Term)
	#define ASSERT_OR_RETURN_VOID(Term) NXN_ASSERT (Term)
#else
	#define ASSERT_OR_RETURN(Term,ReturnCode) RETURN_IF (!(Term), ReturnCode)
	#define ASSERT_OR_RETURN_VOID(Term) RETURN_VOID_IF (!(Term))
#endif


#ifdef _MFC_VER
//!!! RF 09Apr99 macros for MessageBox error messages
#define PREPARE_ERROR_MESSAGE(a) CString _NxNErrorMessage (_T(a));
#define SET_ERROR_MESSAGE(a) _NxNErrorMessage = a;

#ifdef _DEBUG
	#define RETURN_WITH_MSG_IF(a,b) \
	{ \
		if ( a ) \
		{ \
			HRESULT hResult2 = b; \
			if ( hResult2 == S_OK ) return S_OK; \
			CString sOperation (_T(#a)); \
			CString sError (NyNGetErrorMessage (hResult2)); \
			CString sMessage; \
			sMessage.Format ( _T("%s failed\nError: %s"), (const char*)sOperation, (const char*)sError ); \
			NXN_TRACE ( _T("s"), (const char*)sMessage ); \
            CLanguageString sMsg( &g_NxNIntegratorSDKLanguageModule, IDS_ERR_XXX_FAILED_ERROR_XXX, (LPCWSTR)sOperation, (LPCWSTR)sError ); \
            sMsg.MessageBox( LANG_GET_MAIN_AFX_WND, MB_OK | MB_ICONEXCLAMATION ); \
			return b; \
		}\
	}
#else
	#define RETURN_WITH_MSG_IF(a,b)\
	{ \
		if ( a ) \
		{\
			HRESULT hResult2 = b; \
			if ( hResult2 == S_OK ) return S_OK; \
			CString sOperation (_NxNErrorMessage); \
			CString sError (NyNGetErrorMessage (hResult2)); \
			CString sMessage; \
			sMessage.Format ( _T("%s\nError: %s"), (const char*)_NxNErrorMessage, (const char*)sError ); \
			NXN_TRACE ( _T("s"), (const char*)sMessage ); \
            CLanguageString sMsg( &g_NxNIntegratorSDKLanguageModule, IDS_ERR_XXX_FAILED_ERROR_XXX, (LPCWSTR)sOperation, (LPCWSTR)sError ); \
            sMsg.MessageBox( LANG_GET_MAIN_AFX_WND, MB_OK | MB_ICONEXCLAMATION ); \
			return b; \
		}\
	}
#endif

#ifdef _DEBUG
	#define RETURN_WITH_MSG_IF_ERROR(a) \
	{ \
		HRESULT _hResult2 = a; \
		if ( _hResult2!=S_OK ) \
		{ \
			CString sOperation (_T(#a)); \
			CString sError (NyNGetErrorMessage (_hResult2)); \
			CString sMessage; \
			sMessage.Format ( _T("%s failed\nError: %s"), (const char*)sOperation, (const char*)sError ); \
			NXN_TRACE ( _T("s"), (const char*)sMessage ); \
            CLanguageString sMsg( IDS_ERR_XXX_FAILED_ERROR_XXX, (LPCWSTR)sOperation, (LPCWSTR)sError ); \
            sMsg.MessageBox( LANG_GET_MAIN_AFX_WND, MB_OK | MB_ICONEXCLAMATION ); \
			return _hResult2; \
		}\
	}
#else
	#define RETURN_WITH_MSG_IF_ERROR(a)\
	{ \
		HRESULT _hResult2 = a; \
		if ( _hResult2!=S_OK ) \
		{ \
			CString sOperation (_NxNErrorMessage); \
			CString sError (NyNGetErrorMessage (_hResult2)); \
			CString sMessage; \
			sMessage.Format ( _T("%s\nError: %s"), (const char*)_NxNErrorMessage, (const char*)sError ); \
			NXN_TRACE ( _T("s"), (const char*)sMessage ); \
            CLanguageString sMsg( IDS_ERR_XXX_FAILED_ERROR_XXX, (LPCWSTR)sOperation, (LPCWSTR)sError ); \
            sMsg.MessageBox( LANG_GET_MAIN_AFX_WND, MB_OK | MB_ICONEXCLAMATION ); \
			return _hResult2; \
		}\
	}
#endif
#endif // #ifdef _MFC_VER




// Tracing

#ifndef NXN_TRACE
#ifdef DEBUG 

// static functions and variables for selective trace
#ifdef __ME__
// trace is by default turned off and turned on for special users
static bool _selective_trace_it = false;
static bool _selective_trace_it_global = false;


#else // __ME__
// this user has no __ME__ defined therefore trace everything
static bool _selective_trace_it = true;
static bool _selective_trace_it_global = true;

#endif // __ME__

static bool _selective_trace_test()
{
	return ( _selective_trace_it);
}

static bool _selective_trace_test_toggle()
{
	// change the value of _selective_trace_it as a side effect
	if ( _selective_trace_it)
	{
		// reset _trace_it
		_selective_trace_it = _selective_trace_it_global;
		return true;
	}
	return false;
}

#	define NXN_TRACE if (_selective_trace_test()) TRACE("%s(%d):",__FILE__,__LINE__); if (_selective_trace_test_toggle()) TRACE
#else // DEBUG
#	define NXN_TRACE  TRACE
#endif // DEBUG

// define traces commands which turn tracing on for a special user 
// __ME__ is defined in local.h as CString ("Wolfgang")
#	define WOG_TRACE _selective_trace_it = (__ME__ == _T("Wolfgang") ); NXN_TRACE


#endif /* NXN_TRACE */




#endif // #ifndef _INC_ERROR_MACROS_H_
