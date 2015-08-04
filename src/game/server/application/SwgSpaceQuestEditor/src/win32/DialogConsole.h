// ======================================================================
//
// DialogConsole.h
// asommers
//
// copyright 2004, sony online entertainment
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
	virtual ~DialogConsole();

	//{{AFX_DATA(DialogConsole)
	enum { IDD = IDD_DIALOG_CONSOLE };
	CEdit	m_configurationEditCtrl;
	CEdit	m_outputEditCtrl;
	CEdit	m_warningEditCtrl;
	CEdit	m_shellEditCtrl;
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

	void addConfiguration(CString const & text);
	void addOutput(CString const & text);
	void addWarning(CString const & text);
	void execute(CString const & text);

private:

	//-- adapted from CShellView
	void AddTexts(LPCTSTR string);
	void WriteToPipe(LPCTSTR line);
	BOOL CreateShellRedirect();
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
		T_configuration,
		T_output,
		T_warning,
		T_shell,

		T_COUNT
	};

private:

	CTabCtrl & GetSelectorTabCtrl();
	CEdit & GetConfigurationEditCtrl();
	CEdit & GetOutputEditCtrl();
	CEdit & GetWarningEditCtrl();
	CEdit & GetShellEditCtrl();
	void reset();

private:

	bool m_initialized;
	LOGFONT m_lf;

	CFont m_fontList[T_COUNT];
	CEdit * m_editCtrlList[T_COUNT];
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
