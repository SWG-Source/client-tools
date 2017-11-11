// ======================================================================
//
// DialogMerge.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_DialogMerge_H
#define INCLUDED_DialogMerge_H

#include "Resource.h"

// ======================================================================

class DialogMerge : public CDialog
{
public:

	DialogMerge(CString const & stringId, CString const & original, CString const & theirs, CString const & yours, CWnd* pParent = NULL);

	//{{AFX_DATA(DialogMerge)
	enum { IDD = IDD_DIALOG_MERGE };
	CButton	m_stringId;
	CString	m_original;
	CString	m_theirs;
	CString	m_yours;
	CString	m_merged;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(DialogMerge)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual void OnOK();
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(DialogMerge)
	afx_msg void OnButtonOriginal();
	afx_msg void OnButtonTheirs();
	afx_msg void OnButtonYours();
	afx_msg void OnButtonMerged();
	afx_msg void OnChangeEditMerged();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	CString m_stringIdText;
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
