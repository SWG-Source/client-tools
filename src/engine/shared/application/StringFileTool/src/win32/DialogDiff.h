// ======================================================================
//
// DialogDiff.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_DialogDiff_H
#define INCLUDED_DialogDiff_H

#include "Resource.h"

// ======================================================================

class DialogDiff : public CDialog
{
public:

	DialogDiff(CString const & stringId, CString const & stringTable1FileName, CString const & string1, CString const & stringTable2FileName, CString const & string2, CWnd* pParent = NULL);

	//{{AFX_DATA(DialogDiff)
	enum { IDD = IDD_DIALOG_DIFF };
	CButton	m_stringId;
	CString	m_string1;
	CString	m_string2;
	CString	m_stringTable1;
	CString	m_stringTable2;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(DialogDiff)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(DialogDiff)
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
