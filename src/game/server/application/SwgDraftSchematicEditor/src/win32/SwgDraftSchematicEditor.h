// ======================================================================
//
// SwgDraftSchematicEditor.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgDraftSchematicEditor_H
#define INCLUDED_SwgDraftSchematicEditor_H

// ======================================================================

class SwgDraftSchematicEditorApp : public CWinApp
{
public:

	SwgDraftSchematicEditorApp();

	//{{AFX_VIRTUAL(SwgDraftSchematicEditorApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(SwgDraftSchematicEditorApp)
	afx_msg void OnAppAbout();
	afx_msg void OnFileCloseall();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
