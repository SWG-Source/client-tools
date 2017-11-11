//
// FractalPreviewView.h
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_FractalPreviewView_H
#define INCLUDED_FractalPreviewView_H

//-------------------------------------------------------------------

class CDib;
class MultiFractal;

//-------------------------------------------------------------------

class FractalPreviewView : public CView
{
private:

	CDib* m_dib;
	int   m_dibSize;

protected:

	FractalPreviewView();
	DECLARE_DYNCREATE(FractalPreviewView)

public:

	void updateBitmap (const MultiFractal& multiFractal, float low, float high);

	//{{AFX_VIRTUAL(FractalPreviewView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

protected:

	virtual ~FractalPreviewView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	//{{AFX_MSG(FractalPreviewView)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

//-------------------------------------------------------------------

#endif 
