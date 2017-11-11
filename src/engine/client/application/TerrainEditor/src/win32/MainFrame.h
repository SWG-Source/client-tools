//
// MainFrame.h
// asommers 10-9-2000
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#ifndef MAINFRAME_H
#define MAINFRAME_H

//-------------------------------------------------------------------

#include "CoolDialogBar.h"
#include "DialogCatalog.h"
#include "sharedMath/Vector.h"

//-------------------------------------------------------------------

class MainFrame : public CMDIFrameWnd
{
private:

	DECLARE_DYNAMIC(MainFrame)

protected:

	CStatusBar     m_wndStatusBar;
	CToolBar       m_wndToolBar;
	CReBar         m_wndReBar;
//	CDialogBar     m_wndDlgBar;

	DialogCatalog  catalogDialog;
	CCoolDialogBar catalogDialogBar;

	Vector         mousePosition_w;
	real           zoomLevel;
	CString        shaderFamilyName;
	CString        floraStaticCollidableFamilyName;
	CString        floraStaticNonCollidableFamilyName;
	CString        floraDynamicNearFamilyName;
	CString        floraDynamicFarFamilyName;

public:

	MainFrame (void);
	virtual ~MainFrame();

public:

	void setMousePosition_w (const Vector& newMousePosition_w);
	void setZoomLevel (real newZoomLevel);
	real getZoomLevel (void) const;
	void setShaderFamilyName (const CString& newShaderFamilyName);
	void setFloraStaticCollidableFamilyName  (const CString& newFloraStaticCollidableFamilyName);
	void setFloraStaticNonCollidableFamilyName  (const CString& newFloraStaticNonCollidableFamilyName);
	void setFloraDynamicNearFamilyName (const CString& newFloraDynamicNearFamilyName);
	void setFloraDynamicFarFamilyName (const CString& newFloraDynamicFarFamilyName);

	//{{AFX_VIRTUAL(MainFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	afx_msg void OnInitMenu(CMenu* pMenu);

	//{{AFX_MSG(MainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnUpdate2dMapPosition(CCmdUI* pCmdUI);
	afx_msg void OnUpdateZoomLevel(CCmdUI* pCmdUI);
	afx_msg void OnUpdateShaderFamilyName(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFloraStaticCollidableFamilyName(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFloraStaticNonCollidableFamilyName(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFloraDynamicNearFamilyName(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFloraDynamicFarFamilyName(CCmdUI* pCmdUI);
	afx_msg void OnViewCatalog();
	afx_msg void OnUpdateViewCatalog(CCmdUI* pCmdUI);
	afx_msg void OnDebugTest();
	afx_msg void OnUpdateDebugTest(CCmdUI* pCmdUI);
	afx_msg void OnDebugTestpointeditor();
	afx_msg void OnUpdateDebugTestpointeditor(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//-------------------------------------------------------------------

inline void MainFrame::setMousePosition_w (const Vector& newMousePosition_w)
{
	mousePosition_w = newMousePosition_w;
}

//-------------------------------------------------------------------

inline void MainFrame::setZoomLevel (real newZoomLevel)
{
	zoomLevel = newZoomLevel;
}

//-------------------------------------------------------------------

inline real MainFrame::getZoomLevel (void) const
{
	return zoomLevel;
}

//-------------------------------------------------------------------

inline void MainFrame::setShaderFamilyName (const CString& newShaderFamilyName)
{
	shaderFamilyName = newShaderFamilyName;
}

//-------------------------------------------------------------------

inline void MainFrame::setFloraStaticCollidableFamilyName  (const CString& newFloraStaticCollidableFamilyName)
{
	floraStaticCollidableFamilyName = newFloraStaticCollidableFamilyName;
}

//-------------------------------------------------------------------

inline void MainFrame::setFloraStaticNonCollidableFamilyName  (const CString& newFloraStaticNonCollidableFamilyName)
{
	floraStaticNonCollidableFamilyName = newFloraStaticNonCollidableFamilyName;
}

//-------------------------------------------------------------------

inline void MainFrame::setFloraDynamicNearFamilyName (const CString& newFloraDynamicNearFamilyName)
{
	floraDynamicNearFamilyName = newFloraDynamicNearFamilyName;
}

//-------------------------------------------------------------------

inline void MainFrame::setFloraDynamicFarFamilyName (const CString& newFloraDynamicFarFamilyName)
{
	floraDynamicFarFamilyName = newFloraDynamicFarFamilyName;
}

//-------------------------------------------------------------------

inline MainFrame* GetMainFrame (void)
{
	return safe_cast<MainFrame*> (AfxGetMainWnd ());
}

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

#endif 
