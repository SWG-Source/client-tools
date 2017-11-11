//
// DialogCatalog.h
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_DialogCatalog_H
#define INCLUDED_DialogCatalog_H

//-------------------------------------------------------------------

#include "Resource.h"

//-------------------------------------------------------------------

class DialogCatalog : public CDialog
{
private:

	BOOL        initialized;

	bool        imageListSet;
	CImageList  imageList;

	HTREEITEM   m_shaders;
	HTREEITEM   m_flora;
	HTREEITEM   m_radial;
	HTREEITEM   m_blends;

private:

	void readIniFile (const char* filename);
	
	void add (HTREEITEM parent, const CString& family, const CString& child);

public:

	explicit DialogCatalog (CWnd* pParent = NULL);   
	virtual ~DialogCatalog (void);   

	void reset (void);

protected:

	//{{AFX_DATA(DialogCatalog)
	enum { IDD = IDD_DIALOG_CATALOG };
	CTreeCtrl	m_treeCtrl;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(DialogCatalog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(DialogCatalog)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnInsertintoshadergroup();
	afx_msg void OnRclickCatalogTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnInsertintofloragroup();
	afx_msg void OnInsertintoradialgroup();
	afx_msg void OnUpdateInsertintofloragroup(CCmdUI* pCmdUI);
	afx_msg void OnUpdateInsertintoradialgroup(CCmdUI* pCmdUI);
	afx_msg void OnUpdateInsertintoshadergroup(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

//-------------------------------------------------------------------

#endif 
