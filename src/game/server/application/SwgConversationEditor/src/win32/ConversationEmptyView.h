// ======================================================================
//
// ConversationEmptyView.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_ConversationEmptyView_H
#define INCLUDED_ConversationEmptyView_H

// ======================================================================

#include "Resource.h"

// ======================================================================

class ConversationEmptyView : public CFormView
{
protected:

	ConversationEmptyView();           
	DECLARE_DYNCREATE(ConversationEmptyView)

public:

	//{{AFX_DATA(ConversationEmptyView)
	enum { IDD = IDD_EMPTYVIEW };
	//}}AFX_DATA

	//{{AFX_VIRTUAL(ConversationEmptyView)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:

	virtual ~ConversationEmptyView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	//{{AFX_MSG(ConversationEmptyView)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
