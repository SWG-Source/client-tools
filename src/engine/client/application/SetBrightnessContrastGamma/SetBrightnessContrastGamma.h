// SetBrightnessContrastGamma.h : main header file for the SETBRIGHTNESSCONTRASTGAMMA application
//

#if !defined(AFX_SETBRIGHTNESSCONTRASTGAMMA_H__FA730086_0B88_4F33_87EA_B2130B687532__INCLUDED_)
#define AFX_SETBRIGHTNESSCONTRASTGAMMA_H__FA730086_0B88_4F33_87EA_B2130B687532__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CSetBrightnessContrastGammaApp:
// See SetBrightnessContrastGamma.cpp for the implementation of this class
//

class CSetBrightnessContrastGammaApp : public CWinApp
{
public:
	CSetBrightnessContrastGammaApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSetBrightnessContrastGammaApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CSetBrightnessContrastGammaApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETBRIGHTNESSCONTRASTGAMMA_H__FA730086_0B88_4F33_87EA_B2130B687532__INCLUDED_)
