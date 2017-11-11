//
// IffTreeView.h
// asommers 6-12-99
//
// copyright 1999, bootprint entertainment
//

//-------------------------------------------------------------------

#ifndef IFFTREEVIEW_H
#define IFFTREEVIEW_H

#include "IffDoc.h"

class IffTreeView : public CTreeView
{
private:

	CImageList imageList;

protected:

	IffTreeView (void);
	DECLARE_DYNCREATE(IffTreeView)

public:

	IffDoc*       GetDocument (void);
	const IffDoc* GetDocument (void) const;

public:

	//{{AFX_VIRTUAL(IffTreeView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnInitialUpdate(); // called first time after construct
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

public:

	virtual ~IffTreeView (void);

	void Dump(const char * fileName);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	//{{AFX_MSG(IffTreeView)
	afx_msg void OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//-------------------------------------------------------------------

#ifndef _DEBUG 
inline IffDoc* IffTreeView::GetDocument (void)
{ 
	return static_cast<IffDoc*> (m_pDocument);
}

inline const IffDoc* IffTreeView::GetDocument (void) const
{ 
	return static_cast<const IffDoc*> (m_pDocument);
}

#endif

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

#endif
