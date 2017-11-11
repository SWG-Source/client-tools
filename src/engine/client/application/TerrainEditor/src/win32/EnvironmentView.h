//
// EnvironmentView.h
// asommers
//
// copyright 2000, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_EnvironmentView_H
#define INCLUDED_EnvironmentView_H

//-------------------------------------------------------------------

#include "sharedTerrain/EnvironmentGroup.h"

//-------------------------------------------------------------------

class EnvironmentView : public CTreeView
{
private:

	EnvironmentGroup*    environmentGroup;

	bool                 deletingUnused;

private:

	bool      familyExists (const char* familyName) const;
	HTREEITEM findFamily (HTREEITEM hItem, int familyId) const;

	const CString CreateUniqueFamilyName (const CString* base=0) const;

private:

	HTREEITEM getSelectedFamily (void) const;

protected:

	EnvironmentView();           
	DECLARE_DYNCREATE(EnvironmentView)

public:

	void addFamily (const CString& familyName);
	void reset (void) const;

	//{{AFX_VIRTUAL(EnvironmentView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

protected:

	virtual ~EnvironmentView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	//{{AFX_MSG(EnvironmentView)
	afx_msg void OnNewenvironmentfamily();
	afx_msg void OnUpdateNewenvironmentfamily(CCmdUI* pCmdUI);
	afx_msg void OnDeleteenvironment();
	afx_msg void OnUpdateDeleteenvironment(CCmdUI* pCmdUI);
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
