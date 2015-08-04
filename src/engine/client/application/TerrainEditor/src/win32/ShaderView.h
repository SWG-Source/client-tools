//
// ShaderView.cpp
// asommers 10-13-2000
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#ifndef SHADERVIEW_H
#define SHADERVIEW_H

//-------------------------------------------------------------------

class Shader;
class StaticVertexBuffer;

//-------------------------------------------------------------------

class ShaderView : public CView
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

	ShaderView (void);
	virtual ~ShaderView();

	DECLARE_DYNCREATE(ShaderView)

public:

	//{{AFX_VIRTUAL(ShaderView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnInitialUpdate();     // first time after construct
	//}}AFX_VIRTUAL

protected:

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	//{{AFX_MSG(ShaderView)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

#endif
