//
// IffDataView.h
// asommers 6-12-99
//
// copyright 1999, bootprint entertainment
//

//-------------------------------------------------------------------

#ifndef IFFDATAVIEW_H
#define IFFDATAVIEW_H

class IffDataView : public CEditView
{
public:

	enum DisplayMode
	{
		DM_text  = 0,
		DM_byte  = 1,
		DM_word  = 2,
		DM_dword = 3
	};

protected:

	IffDataView();
	DECLARE_DYNCREATE(IffDataView)

private:

	CFont*      font;
	DisplayMode displayMode;

public:

	IffDoc*     GetDocument();
	void        setDisplayMode (DisplayMode newDisplayMode);
	DisplayMode getDisplayMode (void) const;

	//{{AFX_VIRTUAL(IffDataView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual void OnInitialUpdate(); // called first time after construct
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

public:

	virtual ~IffDataView();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	void updateOutput (void);

protected:

	//{{AFX_MSG(IffDataView)
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//-------------------------------------------------------------------

#ifndef _DEBUG
inline IffDoc* IffDataView::GetDocument()
   { return (IffDoc*)m_pDocument; }
#endif

inline void IffDataView::setDisplayMode (IffDataView::DisplayMode newDisplayMode)
{
	displayMode = newDisplayMode;
}

inline IffDataView::DisplayMode IffDataView::getDisplayMode (void) const
{
	return displayMode;
}

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

#endif
