//
// AnimationTreeDialog.h
//
// copyright 2006, Sony Online Entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_AnimationTreeDialog_H
#define INCLUDED_AnimationTreeDialog_H

//-------------------------------------------------------------------

#include "resource.h"
#include "CMultiTree.h"
#include "ViewerDoc.h"

//-------------------------------------------------------------------

class AnimationTreeDialog : public CDialog
{
private:

	bool		m_initialized;
	bool		m_imageListSet;
	CImageList	m_imageList;
	CViewerDoc* m_activeDocument;

private:

	void openItem (HTREEITEM root, bool writeCustomizationData, bool closeDocument, bool debugDump);
	void openSingleItem (HTREEITEM root, bool writeCustomizationData, bool closeDocument, bool debugDump);

public:

	AnimationTreeDialog(CWnd* pParent = NULL);   
	void clear (void);
	void initTreeCtrl(CViewerDoc::AnimationFileList* animFileListPrimarySkel, CViewerDoc::AnimationFileList* animFileListSecondarySkel);

	CString getSelectedFilename() const;

	//{{AFX_DATA(AnimationTreeDialog)
	enum { IDD = IDD_ANIMATION_TREE };
	CMultiTree	m_treeCtrl;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(AnimationTreeDialog)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(AnimationTreeDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDblclkDirectoryTree(NMHDR* pNMHDR, LRESULT* pResult);
	virtual void OnOK();
	afx_msg void OnKeydownDirectoryTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRclickDirectoryTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemexpandingDirectoryTree(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

//-------------------------------------------------------------------

#endif 
