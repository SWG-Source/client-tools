//
// BitmapPreviewView.h
//
// copyright 2004, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_BitmapPreviewView_H
#define INCLUDED_BitmapPreviewView_H

//-------------------------------------------------------------------

class CDib;
class Image;

//-------------------------------------------------------------------

class BitmapPreviewView : public CView
{
private:

	CDib* m_dib;
	int   m_dibSize;

protected:

	BitmapPreviewView();
	DECLARE_DYNCREATE(BitmapPreviewView)

public:

	void updateBitmap (const Image& image, float low, float high, float gain);

	//{{AFX_VIRTUAL(BitmapPreviewView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

protected:

	virtual ~BitmapPreviewView();
	void CreateDib(const Image* image);
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	//{{AFX_MSG(BitmapPreviewView)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

//-------------------------------------------------------------------

#endif 
