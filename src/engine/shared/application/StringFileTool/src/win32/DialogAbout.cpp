// ======================================================================
//
// DialogAbout.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "FirstStringFileTool.h"
#include "DialogAbout.h"

// ======================================================================

DialogAbout::DialogAbout() : CDialog(DialogAbout::IDD)
{
	//{{AFX_DATA_INIT(DialogAbout)
	//}}AFX_DATA_INIT
}

// ----------------------------------------------------------------------

void DialogAbout::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogAbout)
	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(DialogAbout, CDialog)
	//{{AFX_MSG_MAP(DialogAbout)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ======================================================================
