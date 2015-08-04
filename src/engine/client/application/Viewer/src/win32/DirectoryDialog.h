//
// DirectoryDialog.h
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_DirectoryDialog_H
#define INCLUDED_DirectoryDialog_H

//-------------------------------------------------------------------

#include "resource.h"
#include "CMultiTree.h"

class DialogProgress;

//-------------------------------------------------------------------

class DirectoryDialog : public CDialog
{
private:

	BOOL       initialized;

	bool        imageListSet;
	CImageList  imageList;

private:

	void openItem (HTREEITEM root, bool writeCustomizationData, bool closeDocument, bool debugDump);
	void openSingleItem (HTREEITEM root, bool writeCustomizationData, bool closeDocument, bool debugDump);
	void writeSatCustomizationData(HTREEITEM treeItem);
	bool openMeshItem(DialogProgress * dialog, int & current, int maximum, CString const &parentPath, HTREEITEM root, bool detectAlphaOnly) const;
	void checkForLmg (CString const &parentPath, HTREEITEM root);
	void fixExportedSatFiles (CString const &parentPath, HTREEITEM root);

public:

	DirectoryDialog(CWnd* pParent = NULL);   

	void OnButtonBuildAsynchronousLoaderData ();
	void OnButtonDebugDumpAll ();
	void OnButtonWriteCustomizationData ();
	void OnButtonOpenAll (bool closeDocument);
	void OnButtonFind(const CString& name);
	void OnButtonCheckDupes();
	void OnButtonFindLmgs();
	void OnButtonFixExportedSatFiles();
	void reset (void);
	CString getSelectedFilename() const;

	//{{AFX_DATA(DirectoryDialog)
	enum { IDD = IDD_DIRECTORY_TREE };
	CMultiTree m_treeCtrl;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(DirectoryDialog)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(DirectoryDialog)
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
