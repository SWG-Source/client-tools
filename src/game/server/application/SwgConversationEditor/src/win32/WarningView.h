// ======================================================================
//
// WarningView.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_WarningView_H
#define INCLUDED_WarningView_H

// ======================================================================

#include "SwgConversationEditorDoc.h"

// ======================================================================

class WarningView : public CListView
{
protected:

	WarningView();           

	DECLARE_DYNCREATE(WarningView)

public:

	//{{AFX_VIRTUAL(WarningView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

protected:

	virtual ~WarningView();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	//{{AFX_MSG(WarningView)
	afx_msg void OnDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	void clear ();
	void add (int type, CString const & message, ConversationItem const * conversationItem);

private:

	bool m_imageListSet;
	CImageList m_imageList;
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
