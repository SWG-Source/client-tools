//===================================================================
//
// ChildFrm.h
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_ChildFrm_H
#define INCLUDED_ChildFrm_H

//===================================================================

class WorldSnapshotDataView;
class WorldSnapshotTreeView;

//===================================================================

class CChildFrame : public CMDIChildWnd
{
private:

	WorldSnapshotDataView* dataView;
	WorldSnapshotTreeView* treeView;

protected:

	DECLARE_DYNCREATE(CChildFrame)

public:

	CChildFrame();

protected:

	CSplitterWnd m_wndSplitter;

public:

	//{{AFX_VIRTUAL(CChildFrame)
	public:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

public:

	virtual ~CChildFrame();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	WorldSnapshotDataView* getDataView ();
	WorldSnapshotTreeView* getTreeView ();

protected:

	//{{AFX_MSG(CChildFrame)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//===================================================================

inline WorldSnapshotDataView* CChildFrame::getDataView ()
{
	return dataView;
}

//-------------------------------------------------------------------

inline WorldSnapshotTreeView* CChildFrame::getTreeView ()
{
	return treeView;
}

//===================================================================

//{{AFX_INSERT_LOCATION}}

//===================================================================

#endif
