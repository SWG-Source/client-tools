// ======================================================================
//
// MainFrame.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_MainFrame_H
#define INCLUDED_MainFrame_H

// ======================================================================

class MainFrame : public CMDIFrameWnd
{
	DECLARE_DYNAMIC(MainFrame)

public:

	MainFrame();
	virtual ~MainFrame();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	void setMapPosition (CPoint const & mapPosition);
	bool getOptimizeOnLoad () const;

	//{{AFX_VIRTUAL(MainFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

protected:

	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;
	CReBar      m_wndReBar;
	CDialogBar  m_wndDlgBar;

protected:

	//{{AFX_MSG(MainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnInitMenu(CMenu* pMenu);
	afx_msg void OnOptionsOptimizefileonload();
	afx_msg void OnUpdateOptionsOptimizefileonload(CCmdUI* pCmdUI);
	//}}AFX_MSG
	afx_msg void OnExecuteDynamicMenu(UINT nID);
	afx_msg void OnUpdate2dMapPosition(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

private:

	typedef std::map<UINT, std::string> IdStringMap;
	IdStringMap m_planetMap;
	IdStringMap m_hostMap;

	CPoint m_mapPosition_w;
	bool m_optimizeOnLoad;
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 

