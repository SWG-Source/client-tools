#if !defined(AFX_DIALOGFINDOBJECT_H__94014AAB_8EA0_4389_ABDB_E4256A0DB2AC__INCLUDED_)
#define AFX_DIALOGFINDOBJECT_H__94014AAB_8EA0_4389_ABDB_E4256A0DB2AC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DialogFindObject.h : header file
//

#include "Resource.h"

/////////////////////////////////////////////////////////////////////////////
// DialogFindObject dialog

class DialogFindObject : public CDialog
{
// Construction
public:
	DialogFindObject(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(DialogFindObject)
	enum { IDD = IDD_DIALOG_FINDOBJECT };
	CButton	m_okButton;
	CString	m_objectId;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DialogFindObject)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(DialogFindObject)
	virtual void OnOK();
	afx_msg void OnChangeEditObjectid();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIALOGFINDOBJECT_H__94014AAB_8EA0_4389_ABDB_E4256A0DB2AC__INCLUDED_)
