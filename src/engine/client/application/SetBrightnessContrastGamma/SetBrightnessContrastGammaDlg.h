// SetBrightnessContrastGammaDlg.h : header file
//

#if !defined(AFX_SETBRIGHTNESSCONTRASTGAMMADLG_H__5F3103EB_9DF1_43F2_9DA0_8651675DB4C6__INCLUDED_)
#define AFX_SETBRIGHTNESSCONTRASTGAMMADLG_H__5F3103EB_9DF1_43F2_9DA0_8651675DB4C6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CSetBrightnessContrastGammaDlg dialog

class CSetBrightnessContrastGammaDlg : public CDialog
{
// Construction
public:
	CSetBrightnessContrastGammaDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CSetBrightnessContrastGammaDlg)
	enum { IDD = IDD_SETBRIGHTNESSCONTRASTGAMMA_DIALOG };
	CEdit	m_gamma;
	CEdit	m_contrast;
	CEdit	m_brightness;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSetBrightnessContrastGammaDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
	WORD  m_ramp[256 * 3];

	void buildRamp(float brightness, float contrast, float gamma);

	// Generated message map functions
	//{{AFX_MSG(CSetBrightnessContrastGammaDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnApply();
	afx_msg void OnQuit();
	afx_msg void OnMinimize();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETBRIGHTNESSCONTRASTGAMMADLG_H__5F3103EB_9DF1_43F2_9DA0_8651675DB4C6__INCLUDED_)
