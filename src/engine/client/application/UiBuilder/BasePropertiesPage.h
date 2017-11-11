#if !defined(AFX_BASEPROPERTIESPAGE_H__4DF0ABD1_EB13_4165_92ED_C4C02768FD1F__INCLUDED_)
#define AFX_BASEPROPERTIESPAGE_H__4DF0ABD1_EB13_4165_92ED_C4C02768FD1F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BasePropertiesPage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// BasePropertiesPage dialog

class BasePropertiesPage : public CPropertyPage
{
	DECLARE_DYNCREATE(BasePropertiesPage)

// Construction
public:
	BasePropertiesPage();
	~BasePropertiesPage();

// Dialog Data
	//{{AFX_DATA(BasePropertiesPage)
	enum { IDD = IDD_BASEPROPERTIES_PAGE };
	CString	m_name;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(BasePropertiesPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(BasePropertiesPage)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BASEPROPERTIESPAGE_H__4DF0ABD1_EB13_4165_92ED_C4C02768FD1F__INCLUDED_)
