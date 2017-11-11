//===================================================================
//
// WorldSnapshotViewer.h
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_WorldSnapshotViewer_H
#define INCLUDED_WorldSnapshotViewer_H

//===================================================================

class WorldSnapshotViewerApp : public CWinApp
{
public:

	WorldSnapshotViewerApp (void);
	virtual ~WorldSnapshotViewerApp (void);

	//{{AFX_VIRTUAL(WorldSnapshotViewerApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(WorldSnapshotViewerApp)
	afx_msg void OnAppAbout();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//===================================================================

//{{AFX_INSERT_LOCATION}}

//===================================================================

#endif
