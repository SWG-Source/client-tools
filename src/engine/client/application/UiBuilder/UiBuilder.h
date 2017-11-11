// UiBuilder.h : main header file for the UIBUILDER application
//

#if !defined(AFX_UIBUILDER_H__29D8C0F0_7A44_48A6_AD48_AF2781688597__INCLUDED_)
#define AFX_UIBUILDER_H__29D8C0F0_7A44_48A6_AD48_AF2781688597__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CUiBuilderApp:
// See UiBuilder.cpp for the implementation of this class
//

class CUiBuilderApp : public CWinApp
{
public:
	CUiBuilderApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUiBuilderApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation

public:

	void saveDialogPosition(const char *i_section, const CRect &i_windowRect);
	bool getDialogPosition(CRect &o_windowRect, const char *i_section, bool clipToDesktop=true);

	bool getRecentFileName(CString &o_fileName, UINT index);

	afx_msg void OnRecentfile(UINT nID);
	//{{AFX_MSG(CUiBuilderApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UIBUILDER_H__29D8C0F0_7A44_48A6_AD48_AF2781688597__INCLUDED_)
