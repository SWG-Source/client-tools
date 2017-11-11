// ======================================================================
//
// DialogNavPoint.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_DialogNavPoint_H
#define INCLUDED_DialogNavPoint_H

// ======================================================================

#include "Resource.h"

// ======================================================================

class DialogNavPoint : public CDialog
{
public:

	DialogNavPoint(CString const & navPoint);

	CString const & getNavPoint() const;

	//{{AFX_DATA(DialogNavPoint)
	enum { IDD = IDD_DIALOG_NAVPOINT };
	CTreeCtrl	m_treeCtrl;
	CString	m_custom;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(DialogNavPoint)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(DialogNavPoint)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnDblclkTreeNavpoint(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	CString m_navPoint;
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
