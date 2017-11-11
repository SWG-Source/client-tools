// LaunchMeFirst.h : main header file for the LAUNCHMEFIRST application
//

#if !defined(AFX_LAUNCHMEFIRST_H__4CA62D16_9835_47D1_AB2E_C9C30E56F5E8__INCLUDED_)
#define AFX_LAUNCHMEFIRST_H__4CA62D16_9835_47D1_AB2E_C9C30E56F5E8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CLaunchMeFirstApp:
// See LaunchMeFirst.cpp for the implementation of this class
//

class CLaunchMeFirstApp : public CWinApp
{
public:
	CLaunchMeFirstApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLaunchMeFirstApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CLaunchMeFirstApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LAUNCHMEFIRST_H__4CA62D16_9835_47D1_AB2E_C9C30E56F5E8__INCLUDED_)
