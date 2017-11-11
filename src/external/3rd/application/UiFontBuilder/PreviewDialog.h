#if !defined(AFX_PREVIEWDIALOG_H__2E02EFE7_912F_46E0_80A7_4A0DBECFEBEB__INCLUDED_)
#define AFX_PREVIEWDIALOG_H__2E02EFE7_912F_46E0_80A7_4A0DBECFEBEB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PreviewDialog.h : header file
//

#include "Unicode.h"
#include <vector>
#include <set>

#include "Resource.h"

/////////////////////////////////////////////////////////////////////////////
// CPreviewDialog dialog

class CPreviewDialog : public CDialog
{
// Construction
public:
	CPreviewDialog(CWnd* pParent, const std::set<Unicode::unicode_char_t> & idSet, CFont * font, int fontSizePixels);   // standard constructor
	typedef std::vector<Unicode::unicode_char_t> ExclusionVector_t;

	const ExclusionVector_t & getExclusionVector () const;


// Dialog Data
	//{{AFX_DATA(CPreviewDialog)
	enum { IDD = IDD_DIALOG_PREVIEW };
	CListCtrl	m_listPreview;
	CString	m_textInfoValue;
	CString	m_textInfoExcludedValue;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPreviewDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL


// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPreviewDialog)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnItemchangedListPreview(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	CString m_data;
	CFont * m_font;

	std::set<Unicode::unicode_char_t> m_idSet;
	ExclusionVector_t  m_exclusionVector;

	int m_fontSizePixels;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PREVIEWDIALOG_H__2E02EFE7_912F_46E0_80A7_4A0DBECFEBEB__INCLUDED_)
