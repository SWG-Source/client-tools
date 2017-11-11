// ======================================================================
//
// DialogSelectNpc.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_DialogSelectNpc_H
#define INCLUDED_DialogSelectNpc_H

// ======================================================================

#include "Resource.h"

// ======================================================================

class DialogSelectNpc : public CDialog
{
public:

	enum NpcType
	{
		NT_giver,
		NT_target
	};

public:

	DialogSelectNpc(NpcType npcType, CWnd* pParent = NULL);

	//{{AFX_DATA(DialogSelectNpc)
	enum { IDD = IDD_DIALOG_SELECTNPC };
	CTreeCtrl	m_treeCtrl;
	//}}AFX_DATA

	CString m_selection;

	//{{AFX_VIRTUAL(DialogSelectNpc)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(DialogSelectNpc)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	NpcType const m_npcType;
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 

