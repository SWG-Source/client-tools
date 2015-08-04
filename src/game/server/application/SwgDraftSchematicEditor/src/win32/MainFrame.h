// ======================================================================
//
// MainFrame.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_MainFrame_H
#define INCLUDED_MainFrame_H

// ======================================================================

#include "CoolDialogBar.h"
#include "DialogConsole.h"
#include "DialogDirectory.h"
#include "DraftSchematic.h"

// ======================================================================

class MainFrame : public CMDIFrameWnd
{
	DECLARE_DYNAMIC(MainFrame)

public:

	MainFrame();

	//{{AFX_VIRTUAL(MainFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

public:

	virtual ~MainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  

	CStatusBar m_wndStatusBar;
	CToolBar m_wndToolBar;

protected:

	//{{AFX_MSG(MainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnToolsOpenall();
	afx_msg void OnViewConsole();
	afx_msg void OnUpdateViewConsole(CCmdUI* pCmdUI);
	afx_msg void OnViewDirectory();
	afx_msg void OnUpdateViewDirectory(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	void consolePrint (CString const & text);
	void consoleExecute (CString const & text);
	void updateTpfTabs (CString const & oldServerText, CString const & newServerText, CString const & oldSharedText, CString const & newSharedText);
	void clearTpfTabs ();
	void refreshDirectory ();

private:

	DialogDirectory m_directoryDialog;
	CCoolDialogBar m_directoryDialogBar;

	DialogConsole m_consoleDialog;
	CCoolDialogBar m_consoleDialogBar;
};

// ======================================================================

inline void CONSOLE_PRINT (CString const & text)
{
	safe_cast<MainFrame *> (AfxGetApp ()->GetMainWnd ())->consolePrint (text);
}

// ----------------------------------------------------------------------

inline void CONSOLE_EXECUTE (CString const & text)
{
	safe_cast<MainFrame *> (AfxGetApp ()->GetMainWnd ())->consoleExecute (text);
}

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
