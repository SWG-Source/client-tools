//===================================================================
//
// MainFrm.h
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_MainFrm_H
#define INCLUDED_MainFrm_H

//===================================================================

#include "sharedMath/Vector2d.h"

//===================================================================

class CMainFrame : public CMDIFrameWnd
{
	DECLARE_DYNAMIC(CMainFrame)

public:

	CMainFrame();

	void setMapPosition (const Vector2d& mapPosition);

public:

	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

public:

	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  

	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;
	CReBar      m_wndReBar;
	CDialogBar  m_wndDlgBar;
	Vector2d    m_mapPosition;

protected:

	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnUpdate2dMapPosition(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//===================================================================

inline void CMainFrame::setMapPosition (const Vector2d& mapPosition)
{
	m_mapPosition = mapPosition;
}

//===================================================================

//{{AFX_INSERT_LOCATION}}

//===================================================================

#endif 
