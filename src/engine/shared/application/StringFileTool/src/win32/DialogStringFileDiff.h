// ======================================================================
//
// DialogStringFileDiff.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_DialogStringFileDiff_H
#define INCLUDED_DialogStringFileDiff_H

// ======================================================================

#include "Resource.h"

class StringTable;

// ======================================================================

class DialogStringFileDiff : public CDialog
{
public:

	DialogStringFileDiff(StringTable const & stringTable1, StringTable const & stringTable2, CWnd * pParent = NULL);

	//{{AFX_DATA(DialogStringFileDiff)
	enum { IDD = IDD_DIALOG_STRINGFILEDIFF };
	CButton	m_okButton;
	CComboBox	m_comboBox;
	CListCtrl	m_listCtrl;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(DialogStringFileDiff)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(DialogStringFileDiff)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSelchangeComboShow();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDblclkListctrl(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	
	HICON m_hIcon;
	CImageList m_imageList;
	StringTable const & m_stringTable1;
	StringTable const & m_stringTable2;

private:
	
	class Node;

	CListCtrl & GetListCtrl();
	void populate();
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif
