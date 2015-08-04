// ======================================================================
//
// DialogStringFileMerge.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_DialogStringFileMerge_H
#define INCLUDED_DialogStringFileMerge_H

// ======================================================================

#include "Resource.h"

#include <map>

class StringTable;

// ======================================================================

class DialogStringFileMerge : public CDialog
{
public:

	DialogStringFileMerge(StringTable const & originalStringTable, StringTable const & theirsStringTable, StringTable const & yoursStringTable, StringTable & mergedStringTable, CWnd * pParent = NULL);

	//{{AFX_DATA(DialogStringFileMerge)
	enum { IDD = IDD_DIALOG_STRINGFILEMERGE };
	CButton	m_cancelButton;
	CButton	m_okButton;
	CListCtrl	m_listCtrl;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(DialogStringFileMerge)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(DialogStringFileMerge)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDblclkListctrl(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	
	HICON m_hIcon;
	CImageList m_imageList;

	StringTable const & m_originalStringTable;
	StringTable const & m_theirsStringTable;
	StringTable const & m_yoursStringTable;
	StringTable & m_mergedStringTable;

private:
	
	class Node;

	CListCtrl & GetListCtrl();
	void populate();
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif
