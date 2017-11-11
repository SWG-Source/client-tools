// ======================================================================
//
// MainFrm.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#if !defined(AFX_MAINFRM_H__645B0C6A_C7F1_11D2_A5F3_00104BF0FB5C__INCLUDED_)
#define AFX_MAINFRM_H__645B0C6A_C7F1_11D2_A5F3_00104BF0FB5C__INCLUDED_

// ======================================================================

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "AnimationDialog.h"
#include "AnimationTreeDialog.h"
#include "CoolDialogBar.h"
#include "DirectoryDialog.h"
#include "HardpointDialog.h"
#include "OutputDialog.h"

class CViewerDoc;

// ======================================================================

class CMainFrame : public CMDIFrameWnd
{
public:

	CMainFrame();
	virtual ~CMainFrame();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	void updateStatusBar(const char* message);

	void clearMessage (void);
	void addMessage (const char* message);

	void updateAnimationView (const CView* newView, bool force=false);
	void updateAnimationView (const CView* newView, const SkeletalAnimationKey::Map& skeletalAnimationKeyMap, bool force=false);
	void updateHardpointTree (CView const * const view);

	void showOutputView (void);

	AnimationDialog *getAnimationDialog ();
	HardpointDialog *getHardpointDialog ();
	AnimationTreeDialog *getAnimationTreeDialog ();

	CViewerDoc * getViewerDoc();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnButtonPolycount();
	afx_msg void OnViewDirectoryview();
	afx_msg void OnUpdateViewDirectoryview(CCmdUI* pCmdUI);
	afx_msg void OnViewOutputwindow();
	afx_msg void OnUpdateViewOutputwindow(CCmdUI* pCmdUI);
	afx_msg void OnButtonCheckDupes();
	afx_msg void OnViewAnimationeditor();
	afx_msg void OnUpdateViewAnimationeditor(CCmdUI* pCmdUI);
	afx_msg void OnButtonSetdestdatabin();
	afx_msg void OnButtonMoveasset();
	afx_msg void OnButtonShowmipmaplevels();
	afx_msg void OnUpdateButtonShowmipmaplevels(CCmdUI* pCmdUI);
	afx_msg void OnButtonFixshtDds();
	afx_msg void OnUpdateButtonFixshtDds(CCmdUI* pCmdUI);
	afx_msg void OnButtonFixshtSt2();
	afx_msg void OnUpdateButtonFixshtSt2(CCmdUI* pCmdUI);
	afx_msg void OnButtonFind();
	afx_msg void OnUpdateButtonFind(CCmdUI* pCmdUI);
	afx_msg void OnButtonOpenall();
	afx_msg void OnUpdateButtonOpenall(CCmdUI* pCmdUI);
	afx_msg void OnViewGotDot3();
	afx_msg void OnUpdateViewGotDot3(CCmdUI* pCmdUI);
	afx_msg void OnViewGotZhak();
	afx_msg void OnUpdateViewGotZhak(CCmdUI* pCmdUI);
	afx_msg void OnButtonWriteCustomizationData();
	afx_msg void OnUpdateButtonWriteCustomizationData(CCmdUI* pCmdUI);
	afx_msg void OnButtonOpencloseall();
	afx_msg void OnUpdateButtonOpencloseall(CCmdUI* pCmdUI);
	afx_msg void OnButtonDebugdumpall();
	afx_msg void OnUpdateButtonDebugdumpall(CCmdUI* pCmdUI);
	afx_msg void OnButtonBuildasyncloaderdata();
	afx_msg void OnUpdateButtonBuildasyncloaderdata(CCmdUI* pCmdUI);
	afx_msg void OnButtonFindLmgs();
	afx_msg void OnUpdateButtonFindLmgs(CCmdUI* pCmdUI);
	afx_msg void OnButtonFixExportedSatFiles();
	afx_msg void OnUpdateButtonFixExportedSatFiles(CCmdUI* pCmdUI);
	afx_msg void OnViewClearoutput();
	afx_msg void OnUpdateViewHardpointTree(CCmdUI* pCmdUI);
	afx_msg void OnViewHardpointTree();
	afx_msg void OnUpdateViewAnimationTree(CCmdUI* pCmdUI);
	afx_msg void OnViewAnimationTree();
	afx_msg void OnHardpointAttach();
	afx_msg void OnUpdateHardpointAttach(CCmdUI* pCmdUI);
	afx_msg void OnHardpointAttachSelected();
	afx_msg void OnUpdateHardpointAttachSelected(CCmdUI* pCmdUI);
	afx_msg void OnRemoveAttachedObject();
	//}}AFX_MSG

protected:

	CStatusBar      m_wndStatusBar;
	CToolBar        m_wndToolBar;
	CToolBar        m_wndToolBarObject;
	CToolBar        m_wndToolBarFix;
	CReBar          m_wndReBar;

	DirectoryDialog directoryDialog;
	CCoolDialogBar  directoryDialogBar;

	OutputDialog    outputDialog;
	CCoolDialogBar  outputDialogBar;

	AnimationDialog animationDialog;
	CCoolDialogBar  animationDialogBar;
	const CView*    currentView;

	HardpointDialog hardpointDialog;
	CCoolDialogBar  hardpointDialogBar;

	AnimationTreeDialog m_animationTreeDialog;
	CCoolDialogBar m_animationTreeDialogBar;

	DECLARE_DYNAMIC(CMainFrame)
	DECLARE_MESSAGE_MAP()
};

// ======================================================================

inline AnimationDialog* CMainFrame::getAnimationDialog ()
{
	return &animationDialog;
}

// ======================================================================

inline HardpointDialog* CMainFrame::getHardpointDialog ()
{
	return &hardpointDialog;
}

// ======================================================================

inline AnimationTreeDialog* CMainFrame::getAnimationTreeDialog ()
{
	return &m_animationTreeDialog;
}

// ======================================================================

inline CMainFrame* GetMainFrame (void)
{
	return static_cast<CMainFrame*> (AfxGetMainWnd ());
}

// ======================================================================

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.
//}}AFX_INSERT_LOCATION

// ======================================================================

#endif
