// ======================================================================
//
// DialogEnterName.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#if !defined(AFX_DIALOGENTERNAME_H__9DD5FB28_29DF_480B_9528_AE1A00408419__INCLUDED_)
#define AFX_DIALOGENTERNAME_H__9DD5FB28_29DF_480B_9528_AE1A00408419__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif

// ======================================================================

#define IDD_ENTER_NAME  0x00000  // bogus value

// ======================================================================

class DialogEnterName : public CDialog
{
public:

	explicit DialogEnterName(const std::string &name, CWnd* pParent = NULL);

	//lint -save -e1925 // public symbols
	//{{AFX_DATA(DialogEnterName)
	enum { IDD = IDD_ENTER_NAME };
	CString	m_name;
	//}}AFX_DATA
	//lint -save -e1925

	//{{AFX_DATA(DialogEnterName)
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DialogEnterName)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:

	// Generated message map functions
	//{{AFX_MSG(DialogEnterName)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	// disabled
	DialogEnterName();
	DialogEnterName(const DialogEnterName&);
	DialogEnterName &operator =(const DialogEnterName&);
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
//}}AFX_INSERT_LOCATION

// ======================================================================

#undef IDD_ENTER_NAME

#endif
