//
// BlendView.h
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_BlendView_H
#define INCLUDED_BlendView_H

//-------------------------------------------------------------------

class BlendView : public CView
{
protected:

	BlendView();           
	DECLARE_DYNCREATE(BlendView)
	virtual ~BlendView();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	//{{AFX_MSG(BlendView)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	//{{AFX_VIRTUAL(BlendView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL
};

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

//-------------------------------------------------------------------

#endif 
