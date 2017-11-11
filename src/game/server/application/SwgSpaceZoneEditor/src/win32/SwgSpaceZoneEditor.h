// ======================================================================
//
// SwgSpaceZoneEditor.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgSpaceZoneEditor_H
#define INCLUDED_SwgSpaceZoneEditor_H

// ======================================================================

class SwgSpaceZoneEditorApp : public CWinApp
{
public:

	SwgSpaceZoneEditorApp (void);
	virtual ~SwgSpaceZoneEditorApp (void);

	//{{AFX_VIRTUAL(SwgSpaceZoneEditorApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(SwgSpaceZoneEditorApp)
	afx_msg void OnAppAbout();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif
