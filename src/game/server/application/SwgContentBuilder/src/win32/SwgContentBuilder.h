// ======================================================================
//
// SwgContentBuilder.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgContentBuilder_H
#define INCLUDED_SwgContentBuilder_H

// ======================================================================

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

// ======================================================================

class SwgContentBuilderApp : public CWinApp
{
public:

	SwgContentBuilderApp();

	//{{AFX_VIRTUAL(SwgContentBuilderApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(SwgContentBuilderApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 

