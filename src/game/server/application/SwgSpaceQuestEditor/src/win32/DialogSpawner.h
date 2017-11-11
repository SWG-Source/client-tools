// ======================================================================
//
// DialogSpawner.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_DialogSpawner_H
#define INCLUDED_DialogSpawner_H

// ======================================================================

#include "Resource.h"

// ======================================================================

class DialogSpawner : public CDialog
{
public:

	DialogSpawner(CString const & spawner);

	CString const & getSpawner() const;

	//{{AFX_DATA(DialogSpawner)
	enum { IDD = IDD_DIALOG_NAVPOINT };
	CTreeCtrl	m_treeCtrl;
	CString	m_custom;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(DialogSpawner)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(DialogSpawner)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnDblclkTreeNavpoint(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	CString m_spawner;
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
