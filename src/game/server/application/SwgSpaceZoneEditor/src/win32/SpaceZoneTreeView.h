// ======================================================================
//
// SpaceZoneTreeView.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_SpaceZoneTreeView_H
#define INCLUDED_SpaceZoneTreeView_H

// ======================================================================

#include "sharedMath/Vector.h"
#include "SwgSpaceZoneEditorDoc.h"
// ======================================================================

class SpaceZoneTreeView : public CTreeView
{
public:

	//{{AFX_VIRTUAL(SpaceZoneTreeView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnInitialUpdate(); // called first time after construct
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

public:

	virtual ~SpaceZoneTreeView ();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	void showInformation();
	bool hasInformation() const;

	void selectItem(HTREEITEM item);
	void selectObject(SwgSpaceZoneEditorDoc::Object const * object);
	void centerSelectedItem();

	void validate();

	void perforceEdit();

protected:

	SpaceZoneTreeView ();
	DECLARE_DYNCREATE(SpaceZoneTreeView)

protected:
	//{{AFX_MSG(SpaceZoneTreeView)
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnProperties();
	afx_msg void OnDelete();
	afx_msg void OnRename();
	afx_msg void OnDuplicate();
	afx_msg void OnBeginlabeledit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeydown(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSpawnerAddpatrolpoint();
	afx_msg void OnSpawnerSelectallpatrolpoints();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	void addNode(HTREEITEM parent, HTREEITEM after, SwgSpaceZoneEditorDoc::Object const * object);
	HTREEITEM find(HTREEITEM hItem, SwgSpaceZoneEditorDoc::Object const * object) const;

private:

	HTREEITEM m_navPointRoot;
	HTREEITEM m_spawnerRoot;
	HTREEITEM m_miscRoot;
public:
	afx_msg void OnSpawnerDuplicate();
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif
