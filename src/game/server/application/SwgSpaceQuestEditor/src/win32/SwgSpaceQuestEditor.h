// ======================================================================
//
// SwgSpaceQuestEditor.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgSpaceQuestEditor_H
#define INCLUDED_SwgSpaceQuestEditor_H

// ======================================================================

class SwgSpaceQuestEditorApp : public CWinApp
{
public:

	SwgSpaceQuestEditorApp();

	CDocument * GetActiveDocument();

	//{{AFX_VIRTUAL(SwgSpaceQuestEditorApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(SwgSpaceQuestEditorApp)
	afx_msg void OnAppAbout();
	afx_msg void OnFileCloseall();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
