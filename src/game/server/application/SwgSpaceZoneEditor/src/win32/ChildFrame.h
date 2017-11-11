// ======================================================================
//
// ChildFrame.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_ChildFrm_H
#define INCLUDED_ChildFrm_H

// ======================================================================

class SpaceZoneMapView;
class SpaceZoneTreeView;
class SpaceZonePropertyView;

#include "LockedSplitterWnd.h"

// ======================================================================

class ChildFrame : public CMDIChildWnd
{
public:

	ChildFrame();
	virtual ~ChildFrame();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	SpaceZoneMapView * getMapView();
	SpaceZoneTreeView * getTreeView();
	SpaceZonePropertyView * getPropertyView();

	void setToolTip(CString const & toolTip);

public:

	//{{AFX_VIRTUAL(ChildFrame)
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void ActivateFrame(int nCmdShow = -1);
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(ChildFrame)
	afx_msg void OnButtonViewmodeXy();
	afx_msg void OnUpdateButtonViewmodeXy(CCmdUI* pCmdUI);
	afx_msg void OnButtonViewmodeXz();
	afx_msg void OnUpdateButtonViewmodeXz(CCmdUI* pCmdUI);
	afx_msg void OnButtonViewmodeZy();
	afx_msg void OnUpdateButtonViewmodeZy(CCmdUI* pCmdUI);
	afx_msg void OnButtonHideNavpoints();
	afx_msg void OnUpdateButtonHideNavpoints(CCmdUI* pCmdUI);
	afx_msg void OnButtonHideSpawners();
	afx_msg void OnUpdateButtonHideSpawners(CCmdUI* pCmdUI);
	afx_msg void OnButtonHideMisc();
	afx_msg void OnUpdateButtonHideMisc(CCmdUI* pCmdUI);
	afx_msg void OnButtonInformation();
	afx_msg void OnUpdateButtonInformation(CCmdUI* pCmdUI);
	afx_msg void OnButtonHidePaths();
	afx_msg void OnUpdateButtonHidePaths(CCmdUI* pCmdUI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnButtonValidate();
	afx_msg void OnButtonHideGrid();
	afx_msg void OnButtonP4();
	afx_msg void OnUpdateButtonHideGrid(CCmdUI* pCmdUI);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:

	DECLARE_DYNCREATE(ChildFrame)

private:

	void resize();

private:

	CStatusBar m_wndStatusBar;
	CSplitterWnd m_mainSplitterWnd;
	CSplitterWnd m_splitterWnd1;
	LockedSplitterWnd m_splitterWnd2;
	bool m_splitterCreated;
	SpaceZoneMapView * m_mapView;
	SpaceZoneTreeView * m_treeView;
	SpaceZonePropertyView * m_propertyView;
};

// ======================================================================

inline SpaceZoneMapView * ChildFrame::getMapView()
{
	return m_mapView;
}

// ----------------------------------------------------------------------

inline SpaceZoneTreeView * ChildFrame::getTreeView()
{
	return m_treeView;
}

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif
