//
// ChildFrm.h
// asommers 6-12-99
//
// copyright 1999, bootprint entertainment
//

//-------------------------------------------------------------------

#ifndef CHILDFRM_H
#define CHILDFRM_H

class IffDataView;
class IffTreeView;

class CChildFrame : public CMDIChildWnd
{
private:

	IffDataView* dataView;
	IffTreeView* treeView;

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

	IffDataView* getDataView (void);
	IffTreeView* getTreeView (void);

protected:

	//{{AFX_MSG(CChildFrame)
	afx_msg void OnByte();
	afx_msg void OnUpdateByte(CCmdUI* pCmdUI);
	afx_msg void OnDword();
	afx_msg void OnUpdateDword(CCmdUI* pCmdUI);
	afx_msg void OnText();
	afx_msg void OnUpdateText(CCmdUI* pCmdUI);
	afx_msg void OnWord();
	afx_msg void OnUpdateWord(CCmdUI* pCmdUI);
	afx_msg void OnDump();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//-------------------------------------------------------------------

inline IffDataView* CChildFrame::getDataView (void)
{
	return dataView;
}

inline IffTreeView* CChildFrame::getTreeView (void)
{
	return treeView;
}

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

#endif
