#if !defined(AFX_ADDPROPERTYDIALOGBOX_H__3C827FB1_465C_4588_9E8D_7CD24E0302A3__INCLUDED_)
#define AFX_ADDPROPERTYDIALOGBOX_H__3C827FB1_465C_4588_9E8D_7CD24E0302A3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AddPropertyDialogBox.h : header file
//
#include "UIString.h"
#include <list>

/////////////////////////////////////////////////////////////////////////////
// AddPropertyDialogBox dialog

class AddPropertyDialogBox : public CDialog
{
// Construction
public:
	AddPropertyDialogBox(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(AddPropertyDialogBox)
	enum { IDD = IDD_ADDPROPERTY };
	CString	m_propertyName;
	CString	m_propertyValue;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(AddPropertyDialogBox)
	public:
	virtual int DoModal();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	typedef std::list<UINarrowString> NarrowStringList;
	typedef std::list<UIString>       StringList;

	static NarrowStringList s_oldPropertyNames;
	static StringList       s_oldPropertyValues;

	// Generated message map functions
	//{{AFX_MSG(AddPropertyDialogBox)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADDPROPERTYDIALOGBOX_H__3C827FB1_465C_4588_9E8D_7CD24E0302A3__INCLUDED_)
