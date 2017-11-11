// LaunchMeFirstDlg.h : header file
//

#if !defined(AFX_LAUNCHMEFIRSTDLG_H__8CFCC867_7D30_48B0_BD78_DAE7A4AF9DD9__INCLUDED_)
#define AFX_LAUNCHMEFIRSTDLG_H__8CFCC867_7D30_48B0_BD78_DAE7A4AF9DD9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CLaunchMeFirstDlg dialog

class CLaunchMeFirstDlg : public CDialog
{
// Construction
public:
	CLaunchMeFirstDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CLaunchMeFirstDlg)
	enum { IDD = IDD_LAUNCHMEFIRST_DIALOG };
	CButton	m_gameTowRetail;
	CButton	m_gameTowPreorder;
	CButton	m_gameTowBeta;
	CButton	m_gameJapaneseRecapture;
	CButton	m_subCombatBalance;
	CButton	m_subFreeTrial;
	CButton	m_subBase;
	CButton	m_gameSpacePreorder;
	CButton	m_gameSpaceRetail;
	CButton	m_gameSpaceBeta;
	CButton	m_gameCollectors;
	CButton	m_gameBase;
	CButton m_gameSwgRetail;
	CButton m_gameJapaneseRetail;
	CButton m_gameJapaneseCollectors;
	CButton m_gameCombatUpgrade;
	CButton m_gameEp3Retail;
	CButton m_gameEp3Preorder;
	int		m_radio;
	CString	m_custom;
	CString	m_commandLine;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLaunchMeFirstDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CLaunchMeFirstDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	int m_bitsToClear;

protected:
	bool run(const char *programName);
	bool writeStationConfig();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LAUNCHMEFIRSTDLG_H__8CFCC867_7D30_48B0_BD78_DAE7A4AF9DD9__INCLUDED_)
