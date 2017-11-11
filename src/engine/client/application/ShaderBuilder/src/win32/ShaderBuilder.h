// ShaderBuilder.h : main header file for the SHADERBUILDER application
//

#if !defined(AFX_SHADERBUILDER_H__F462369C_ACAD_4B32_8A7C_2C06FB083C96__INCLUDED_)
#define AFX_SHADERBUILDER_H__F462369C_ACAD_4B32_8A7C_2C06FB083C96__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include <string>

/////////////////////////////////////////////////////////////////////////////

void Pad4(CEdit &edit);

/////////////////////////////////////////////////////////////////////////////
// CShaderBuilderApp:
// See ShaderBuilder.cpp for the implementation of this class
//

class CShaderBuilderApp : public CWinApp
{
public:
	CShaderBuilderApp();
	virtual ~CShaderBuilderApp();

	std::string stripTreefileSearchPathFromFile(const std::string &pathedFilename);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CShaderBuilderApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CShaderBuilderApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	private:
		char* m_workingDirectory;
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SHADERBUILDER_H__F462369C_ACAD_4B32_8A7C_2C06FB083C96__INCLUDED_)
