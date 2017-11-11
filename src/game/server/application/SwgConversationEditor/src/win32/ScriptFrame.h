// ======================================================================
//
// ScriptFrame.h
// asommers 2003-09-23
//
// copyright2003, sony online entertainment
// 
// ======================================================================

#ifndef INCLUDED_ScriptFrame_H
#define INCLUDED_ScriptFrame_H

// ======================================================================

class ScriptTreeView;
class ScriptView;

// ======================================================================

class ScriptFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(ScriptFrame)

protected:

	ScriptFrame();           

public:

	//{{AFX_VIRTUAL(ScriptFrame)
	public:
	virtual void ActivateFrame(int nCmdShow = -1);
	protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

protected:

	virtual ~ScriptFrame();

	//{{AFX_MSG(ScriptFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	void editCondition (int conditionFamilyId);
	void editAction (int actionFamilyId);
	void editTokenTO (int tokenTOFamilyId);
	void editTokenDF (int tokenDFFamilyId);
	void editTokenDI (int tokenDIFamilyId);
	void editTrigger ();
	void editNothing ();

private:

	CString const m_windowName;
	CToolBar m_wndToolBar;
	CSplitterWnd m_splitter;
	ScriptTreeView * m_treeView;
	ScriptView * m_scriptView;
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 

