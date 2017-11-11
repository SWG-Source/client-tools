// ======================================================================
//
// FirstSwgClientSetup.h
// asommers
//
// copyright 2002, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_FirstSwgClientSetup_H
#define INCLUDED_FirstSwgClientSetup_H

// ======================================================================

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#ifndef ENABLE_RATING_SYSTEM
#define ENABLE_RATING_SYSTEM 1 // Enable or disable rating system.
#endif

// ======================================================================

#include "../../../../../../external/3rd/library/atlmfc/include/afx.h"
#include "../../../../../../external/3rd/library/atlmfc/include/afxwin.h"         // MFC core and standard components
#include "../../../../../../external/3rd/library/atlmfc/include/afxext.h"        // MFC extensions
#include "../../../../../../external/3rd/library/atlmfc/include/afxdisp.h"        // MFC Automation classes
#include "../../../../../../external/3rd/library/atlmfc/include/afxdtctl.h"	// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include "../../../../../../external/3rd/library/atlmfc/include/afxcmn.h" // MFC support for Windows Common Controls
#endif
#include "../../../../../../external/3rd/library/atlmfc/include/atlbase.h"


#include <string>
#include <vector>
#include <string>
#include <map>
#include <vector>

#include "resource.h"

// ======================================================================

#pragma warning (disable: 4786)

// ======================================================================

template<class T>
inline const T& clamp (const T& a, const T& b, const T& c)
{
	return (b < a) ? a : ((b > c) ? c : b);
}

// ======================================================================
	
typedef DWORD uint32;

// ======================================================================
	
inline std::wstring const narrowToWide(char const * const str)
{
	wchar_t wbuf[2048];
	MultiByteToWideChar(CP_UTF8, 0, str, -1, wbuf, (sizeof(wbuf) / sizeof(wchar_t)) - 1);
	return std::wstring(wbuf);
}

// ----------------------------------------------------------------------
	
inline std::string const wideToNarrow(std::wstring const & str)
{
	// do not use best fit chars
#ifndef WC_NO_BEST_FIT_CHARS
	DWORD const WC_NO_BEST_FIT_CHARS = 0x00000400;  
#endif

	char buf[2048];
	WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, str.c_str(), -1, buf, (sizeof(buf) / sizeof(char)) - 1, NULL, NULL);
	return std::string(buf);
}

// ----------------------------------------------------------------------
	
inline std::string const wideToNarrow(CString const & string)
{
	return wideToNarrow(std::wstring(static_cast<LPCTSTR>(string)));
}

// ----------------------------------------------------------------------

namespace SwgClientSetupNamespace
{	
	void sendMail(std::string const & to, std::string const & from, std::string const & subject, std::string const & body, std::vector<std::string> const & attachments);
}

// ----------------------------------------------------------------------

inline void trimString(CString & str)
{
	str.Replace(_T("\n"), _T(" "));
	str.Replace(_T("\r"), _T(" "));
	str.Replace(_T("  "), _T(" "));
	str.TrimLeft();
	str.TrimRight();
}

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
