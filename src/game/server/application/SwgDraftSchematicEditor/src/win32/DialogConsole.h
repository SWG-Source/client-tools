// ======================================================================
//
// DialogConsole.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_DialogConsole_H
#define INCLUDED_DialogConsole_H

// ======================================================================

#include "Resource.h"

// ======================================================================

class DialogConsole : public CDialog
{
public:

	DialogConsole(CWnd* pParent = NULL);   
	virtual ~DialogConsole ();

	//{{AFX_DATA(DialogConsole)
	enum { IDD = IDD_DIALOG_CONSOLE };
	CEdit	m_oldSharedEditCtrl;
	CEdit	m_newSharedEditCtrl;
	CEdit	m_newServerEditCtrl;
	CEdit	m_oldServerEditCtrl;
	CEdit	m_shellEditCtrl;
	CEdit	m_outputEditCtrl;
	CTabCtrl	m_selectorTabCtrl;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(DialogConsole)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(DialogConsole)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual void OnOK();
	afx_msg void OnDestroy();
	afx_msg void OnSelchangeTabSelector(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	void print (CString const & text);
	void execute (CString const & text);

	void setOldServerTpf (CString const & text);
	void setOldSharedTpf (CString const & text);
	void setNewServerTpf (CString const & text);
	void setNewSharedTpf (CString const & text);

private:

	//-- adapted from CShellView
	void AddTexts (LPCTSTR string);
	void WriteToPipe (LPCTSTR line);
	BOOL CreateShellRedirect ();
	BOOL CreateChildProcess();
	static UINT ReadPipeThreadProc( LPVOID pParam );
	void MoveToEnd();
	HANDLE hChildStdinRd, hChildStdinWr, hChildStdinWrDup, 
		hChildStdoutRd, hChildStdoutWr, hChildStdoutRdDup, 
		hSaveStdin, hSaveStdout;  
	CWinThread*	m_pReadThread;
	DWORD dwProcessId;
	//--

private:

	enum Tabs
	{
		T_output,
		T_shell,
		T_oldServer,
		T_oldShared,
		T_newServer,
		T_newShared,

		T_COUNT
	};

private:

	CTabCtrl & GetSelectorTabCtrl ();
	CEdit & GetOutputEditCtrl ();
	CEdit & GetShellEditCtrl ();
	CEdit & GetOldServerEditCtrl ();
	CEdit & GetNewServerEditCtrl ();
	CEdit & GetOldSharedEditCtrl ();
	CEdit & GetNewSharedEditCtrl ();
	void reset ();

private:

	bool m_initialized;
	LOGFONT m_lf;

	CFont m_fontList [T_COUNT];
	CEdit * m_editCtrlList [T_COUNT];
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
