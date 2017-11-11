//
// BitmapPreviewFrame.h
//
// copyright 2004, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_BitmapPreviewFrame_H
#define INCLUDED_BitmapPreviewFrame_H

//-------------------------------------------------------------------


//-------------------------------------------------------------------

class Image;

class BitmapPreviewFrame : public CMDIChildWnd
{
private:

	CString m_windowName;

private:

	DECLARE_DYNCREATE(BitmapPreviewFrame)

protected:

	BitmapPreviewFrame();           
	virtual ~BitmapPreviewFrame();
	
	//{{AFX_MSG(BitmapPreviewFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	void updateBitmap (const Image& image, float low=0.0f, float high=1.0f, float gain=0.0f);

	//{{AFX_VIRTUAL(BitmapPreviewFrame)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL
};

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

//-------------------------------------------------------------------

#endif 
