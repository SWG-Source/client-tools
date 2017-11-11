// ======================================================================
//
// DialogMapAnimation.h
// Portions Copyright 1999, Bootprint Entertainment
// Portions Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#if !defined(AFX_MAPMESHANIMATION_H__DC055FEF_28E1_11D3_B3E6_009027076705__INCLUDED_)
#define AFX_MAPMESHANIMATION_H__DC055FEF_28E1_11D3_B3E6_009027076705__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DialogMapAnimation.h : header file
//

class CrcLowerString;

/////////////////////////////////////////////////////////////////////////////
// DialogMapAnimation dialog

class DialogMapAnimation : public CDialog
{

	void OnOK ();

// Construction
public:

	explicit DialogMapAnimation(CWnd* pParent = NULL);   // standard constructor

	const CrcLowerString &getTrackName() const;

	//lint -save -e1925 // public data member
	// Dialog Data
	//{{AFX_DATA(DialogMapAnimation)
	enum { IDD = IDD_MAP_MESH_ANIMATION };
	CEdit	animationFileNameEdit;
	CString	animationFileName;
	float	blendTime;
	CString	key;
	float	timeScale;
	int		playModeIndex;
	BOOL	isLooping;
	BOOL	isInterruptible;
	float	m_weight;
	CString	m_soundFilename;
	int		m_trackNameIndex;
	//}}AFX_DATA
	//lint -restore

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DialogMapAnimation)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(DialogMapAnimation)
	afx_msg void OnButtonBrowse();
	afx_msg void OnButtonSoundBrowse();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAPMESHANIMATION_H__DC055FEF_28E1_11D3_B3E6_009027076705__INCLUDED_)
