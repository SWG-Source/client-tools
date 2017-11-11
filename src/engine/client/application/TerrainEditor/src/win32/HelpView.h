//
// HelpView.h
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_HelpView_H
#define INCLUDED_HelpView_H

//-------------------------------------------------------------------

#include <afxhtml.h>

//-------------------------------------------------------------------

class HelpView : public CHtmlView
{
protected:

	HelpView();           
	DECLARE_DYNCREATE(HelpView)

public:

	//{{AFX_DATA(HelpView)
	//}}AFX_DATA

public:

	//{{AFX_VIRTUAL(HelpView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:

	virtual ~HelpView();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(HelpView)
	afx_msg void OnButtonBack();
	afx_msg void OnButtonForward();
	afx_msg void OnButtonHome();
	afx_msg void OnButtonStop();
	afx_msg void OnButtonRefresh();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

//-------------------------------------------------------------------

#endif 
