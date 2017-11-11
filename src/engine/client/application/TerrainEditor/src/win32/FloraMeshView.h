//
// FloraMeshView.h
// asommers 10-9-2000
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#ifndef FLORAMESHVIEW_H
#define FLORAMESHVIEW_H

//-------------------------------------------------------------------

class ObjectListCamera;
class Object;
class ObjectList;

//-------------------------------------------------------------------

class FloraMeshView : public CView
{
private:

	ObjectListCamera* camera;
	ObjectList*       objectList;
	Object*           object;
	
	real              yaw;
	real              pitch;
	real              zoom;

	bool              hasName;

protected:

	FloraMeshView (void);         
	FloraMeshView (const FloraMeshView&);         
	FloraMeshView& operator= (const FloraMeshView&);         
	virtual ~FloraMeshView (void);           

	DECLARE_DYNCREATE(FloraMeshView)

public:

	void loadObject (const char* newName);

	//{{AFX_VIRTUAL(FloraMeshView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      
	//}}AFX_VIRTUAL

protected:

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	//{{AFX_MSG(FloraMeshView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnDestroy();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

//-------------------------------------------------------------------

#endif 

