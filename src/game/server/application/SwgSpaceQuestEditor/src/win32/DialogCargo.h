// ======================================================================
//
// DialogCargo.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_DialogCargo_H
#define INCLUDED_DialogCargo_H

// ======================================================================

#include "Resource.h"

// ======================================================================

class DialogCargo : public CDialog
{
public:

	DialogCargo(StringSet const & cargoSet, CString const & cargo);

	CString const & getResult() const;

	//{{AFX_DATA(DialogCargo)
	enum { IDD = IDD_DIALOG_CARGO };
	CListBox	m_cargoListBox;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(DialogCargo)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual void OnOK();
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(DialogCargo)
	virtual BOOL OnInitDialog();
	afx_msg void OnDblclkListCargo();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	StringSet m_cargoSet;
	CString m_result;
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
