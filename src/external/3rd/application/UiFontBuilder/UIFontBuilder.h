// UIFontBuilder.h : main header file for the UIFONTBUILDER application
//

#if !defined(AFX_UIFONTBUILDER_H__148230AB_62C2_4649_8A5C_7F11A7A367DF__INCLUDED_)
#define AFX_UIFONTBUILDER_H__148230AB_62C2_4649_8A5C_7F11A7A367DF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CUIFontBuilderApp:
// See UIFontBuilder.cpp for the implementation of this class
//

class CUIFontBuilderApp : public CWinApp
{
public:
	CUIFontBuilderApp();
	static const TCHAR * const     ms_dbFileKey;
	static const TCHAR * const     ms_lastFontFaceKey;
	static const TCHAR * const     ms_lastFontSizeKey;
	static const TCHAR * const     ms_lastStyleFileKey;
	static const TCHAR * const     ms_lastImageDirKey;

	static HKEY                    ms_hkey;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUIFontBuilderApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CUIFontBuilderApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UIFONTBUILDER_H__148230AB_62C2_4649_8A5C_7F11A7A367DF__INCLUDED_)
