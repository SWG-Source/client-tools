// ======================================================================
//
// DialogDirectory.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_DialogDirectory_H
#define INCLUDED_DialogDirectory_H

// ======================================================================

#include "Resource.h"

// ======================================================================

class DialogDirectory : public CDialog
{
public:

	DialogDirectory(CWnd* pParent = NULL);   // standard constructor

	//{{AFX_DATA(DialogDirectory)
	enum { IDD = IDD_DIALOG_DIRECTORY };
	CTreeCtrl	m_treeCtrl;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(DialogDirectory)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(DialogDirectory)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnDblclkTreeview(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeydownTreeview(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	void openAll();
	void scanAll();
	void saveAll();
	void editAll();
	void refresh();

private:

	CTreeCtrl & GetTreeCtrl();
	void reset(bool refresh);

	void openItem(HTREEITEM root, bool const scanDocument, bool const  saveDocument, bool const closeDocument, bool const editDocument);

private:

	bool m_initialized;

	bool m_imageListSet;
	CImageList m_imageList;
};

// ----------------------------------------------------------------------

inline CTreeCtrl & DialogDirectory::GetTreeCtrl()
{
	return m_treeCtrl;
}

// ----------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 

