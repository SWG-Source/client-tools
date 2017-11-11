//
// BitmapView.h
//
// copyright 2004, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_BitmapView_H
#define INCLUDED_BitmapView_H

//-------------------------------------------------------------------

#include "sharedTerrain/BitmapGroup.h"

//-------------------------------------------------------------------

class BitmapView : public CTreeView
{
private:

	BitmapGroup*        bitmapGroup;

	bool                 deletingUnused;

private:

	bool      familyExists (const char* familyName) const;
	HTREEITEM findFamily (HTREEITEM hItem, int familyId) const;

	const CString createUniqueFamilyName (const CString* base=0) const;

private:

	HTREEITEM getSelectedFamily (void) const;

protected:

	BitmapView();           
	DECLARE_DYNCREATE(BitmapView)

public:

	void addFamily (const CString& familyName);
	void reset (void);

	//{{AFX_VIRTUAL(BitmapView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

protected:

	virtual ~BitmapView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	//{{AFX_MSG(BitmapView)
	afx_msg void OnNewfamily();
	afx_msg void OnUpdateNewfamily(CCmdUI* pCmdUI);
	afx_msg void OnDeletefamily();
	afx_msg void OnUpdateDeletefamily(CCmdUI* pCmdUI);
	afx_msg void OnDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnFindBitmap();
	afx_msg void OnBeginlabeledit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeydown(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnGroupDelete();
	afx_msg void OnGroupRename();
	afx_msg void OnButtonDeleteunused();
	afx_msg void OnGroupFindrulesusingthisfamily();
	afx_msg void OnButtonCopyCurrent();
	afx_msg void OnUpdateButtonCopyCurrent(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

//-------------------------------------------------------------------

#endif 
