//
// BlendGroupFrame.h
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_BlendGroupFrame_H
#define INCLUDED_BlendGroupFrame_H

//-------------------------------------------------------------------

class BlendTreeView;
class BlendView;

//-------------------------------------------------------------------

class BlendGroupFrame : public CMDIChildWnd
{
private:

	CString        m_windowName;
	CSplitterWnd   splitter;

	BlendTreeView* treeView;
	BlendView*     blendView;

protected:

	BlendGroupFrame();           
	DECLARE_DYNCREATE(BlendGroupFrame)
	virtual ~BlendGroupFrame();

	//{{AFX_MSG(BlendGroupFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	//{{AFX_VIRTUAL(BlendGroupFrame)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	//}}AFX_VIRTUAL
};

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

//-------------------------------------------------------------------

#endif 
