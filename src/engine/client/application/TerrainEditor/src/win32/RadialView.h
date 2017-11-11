//
// RadialView.h
// asommers 10-13-2000
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#ifndef INCLUDED_RadialView_H
#define INCLUDED_RadialView_H

//-------------------------------------------------------------------

class Shader;
class StaticVertexBuffer;

//-------------------------------------------------------------------

class RadialView : public CView
{
private:

	Shader*             shader;
	StaticVertexBuffer* vertexBuffer;
	bool                hasName;

private:

	void destroy (void);

public:

	void loadShader (const char* newName);

protected:

	RadialView (void);
	virtual ~RadialView();

	DECLARE_DYNCREATE(RadialView)

public:

	//{{AFX_VIRTUAL(RadialView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnInitialUpdate();     // first time after construct
	//}}AFX_VIRTUAL

protected:

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	//{{AFX_MSG(RadialView)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

#endif
