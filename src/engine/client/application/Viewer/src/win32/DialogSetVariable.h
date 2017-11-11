#if !defined(AFX_DIALOGSETVARIABLE_H__5E1F88DA_17A3_4796_85A2_0F54FD1F6F6E__INCLUDED_)
#define AFX_DIALOGSETVARIABLE_H__5E1F88DA_17A3_4796_85A2_0F54FD1F6F6E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DialogSetVariable.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// DialogSetVariable dialog

class DialogSetVariable : public CDialog
{
// Construction
public:
	DialogSetVariable(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(DialogSetVariable)
	enum { IDD = IDD_SET_VARIABLE };
	CString	m_valueString;
	CString	m_variableName;
	int		m_valueType;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DialogSetVariable)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(DialogSetVariable)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIALOGSETVARIABLE_H__5E1F88DA_17A3_4796_85A2_0F54FD1F6F6E__INCLUDED_)
