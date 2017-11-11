//
// FractalPreviewFrame.h
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_FractalPreviewFrame_H
#define INCLUDED_FractalPreviewFrame_H

//-------------------------------------------------------------------

class MultiFractal;

//-------------------------------------------------------------------

class FractalPreviewFrame : public CMDIChildWnd
{
private:

	CString m_windowName;

private:

	DECLARE_DYNCREATE(FractalPreviewFrame)

protected:

	FractalPreviewFrame();           
	virtual ~FractalPreviewFrame();
	
	//{{AFX_MSG(FractalPreviewFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	void updateBitmap (const MultiFractal& multiFractal, float low=0.f, float high=1.f);

	//{{AFX_VIRTUAL(FractalPreviewFrame)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL
};

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

//-------------------------------------------------------------------

#endif 
