// ======================================================================
//
// FormEmptyView.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_FormEmptyView_H
#define INCLUDED_FormEmptyView_H

// ======================================================================

#include "Resource.h"

// ======================================================================

class FormEmptyView : public CFormView
{
protected:

	FormEmptyView();           
	DECLARE_DYNCREATE(FormEmptyView)

public:

	//{{AFX_DATA(FormEmptyView)
	enum { IDD = IDD_FORM_EMPTYVIEW };
	//}}AFX_DATA

public:

	//{{AFX_VIRTUAL(FormEmptyView)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:

	virtual ~FormEmptyView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	//{{AFX_MSG(FormEmptyView)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
