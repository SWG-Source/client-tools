//
// BlendTreeView.h
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_BlendTreeView_H
#define INCLUDED_BlendTreeView_H

//-------------------------------------------------------------------

class BlendTreeView : public CTreeView
{
protected:

	BlendTreeView();           
	DECLARE_DYNCREATE(BlendTreeView)
	virtual ~BlendTreeView();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	//{{AFX_MSG(BlendTreeView)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	//{{AFX_VIRTUAL(BlendTreeView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL
};

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

//-------------------------------------------------------------------

#endif 
