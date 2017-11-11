// UnicodeTester.h : main header file for the TESTER application
//

#if !defined(AFX_TESTER_H__E47F2308_3902_4955_A912_ADC7C7E7DEB8__INCLUDED_)
#define AFX_TESTER_H__E47F2308_3902_4955_A912_ADC7C7E7DEB8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CUnicodeTesterApp:
// See UnicodeTester.cpp for the implementation of this class
//

class CUnicodeTesterApp : public CWinApp
{
public:
	CUnicodeTesterApp();
	~CUnicodeTesterApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUnicodeTesterApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CUnicodeTesterApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESTER_H__E47F2308_3902_4955_A912_ADC7C7E7DEB8__INCLUDED_)

