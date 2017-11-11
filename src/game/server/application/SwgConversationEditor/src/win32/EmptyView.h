// ======================================================================
//
// EmptyView.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_EmptyView_H
#define INCLUDED_EmptyView_H

// ======================================================================

#include "Resource.h"

// ======================================================================

class EmptyView : public CFormView
{
protected:

	EmptyView();           
	DECLARE_DYNCREATE(EmptyView)

public:

	//{{AFX_DATA(EmptyView)
	enum { IDD = IDD_EMPTYVIEW };
	//}}AFX_DATA

	//{{AFX_VIRTUAL(EmptyView)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:

	virtual ~EmptyView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	//{{AFX_MSG(EmptyView)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
