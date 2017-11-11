// UIFontBuilderDlg.h : header file
//

#if !defined(AFX_UIFONTBUILDERDLG_H__1A78D845_7672_4531_99B4_AEECC647D137__INCLUDED_)
#define AFX_UIFONTBUILDERDLG_H__1A78D845_7672_4531_99B4_AEECC647D137__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Unicode.h"
#include <set>

/////////////////////////////////////////////////////////////////////////////
// CUIFontBuilderDlg dialog

class CUIFontBuilderDlg : public CDialog
{
// Construction
public:
	CUIFontBuilderDlg(TCHAR * unicodeFilename, CWnd* pParent = NULL);	// standard constructor
	~CUIFontBuilderDlg ();
	typedef std::set<Unicode::unicode_char_t> IdSet_t;

// Dialog Data
	//{{AFX_DATA(CUIFontBuilderDlg)
	enum { IDD = IDD_UIFONTBUILDER_DIALOG };
	CEdit	m_editOutputDirImage;
	CComboBox	m_comboTileSize;
	CComboBox	m_comboFileFormat;
	CEdit	m_editPreview;
	CButton	m_buttonNodeUpFull;
	CButton	m_buttonNodeUp;
	CButton	m_buttonNodeDownFull;
	CButton	m_buttonNodeDown;
	CEdit	m_editStyleName;
	CEdit	m_editStyleFile;
	CEdit	m_editImageName;
	CStatic	m_textFontSample;
	CTreeCtrl	m_treeUnicodeSubsets;
	CString	m_textFontSampleText;
	CString	m_textFontSizeText;
	CString	m_editImageNameText;
	CString	m_editStyleNameText;
	BOOL	m_checkAntialiasValue;
	CString	m_editPadBottomValue;
	CString	m_editPadLeftValue;
	CString	m_editPadRightValue;
	CString	m_editPadTopValue;
	CString	m_editPreviewValue;
	BOOL	m_excludeControlValue;
	BOOL	m_checkOutlinesValue;
	CString	m_editStyleFileValue;
	CString	m_editOutputDirImageValue;
	int		m_editTextPixelsValue;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUIFontBuilderDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CUIFontBuilderDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSelectFontButton();
	afx_msg void OnQuit();
	afx_msg void OnButtonCollapseAll();
	afx_msg void OnButtonExpandAll();
	afx_msg void OnBeginlabeleditTreeUnicodeSubsets(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeleditTreeUnicodeSubsets(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButtonNewInclusion();
	afx_msg void OnButtonNewExclusion();
	afx_msg void OnButtonDeleteRange();
	afx_msg void OnButtonEditRange();
	afx_msg void OnButtonReloadBlocks();
	afx_msg void OnButtonPreviewRange();
	afx_msg void OnButtonNodeUp();
	afx_msg void OnButtonNodeDown();
	afx_msg void OnButtonNodeDownFull();
	afx_msg void OnButtonNodeUpFull();
	afx_msg void OnBuildFont();
	afx_msg void OnButtonChangeStyleFile();
	afx_msg void OnButtonChangeOutputdirStyle();
	afx_msg void OnButtonChangeOutputdirImage();
	afx_msg void OnChangeEditTextPixels();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	CFont *               m_fixedFont;
	LOGFONT               m_selectedLogFont;
	CFont *               m_selectedFont;
	CFont *               m_unicodeFont;

	size_t                m_selectedFontSizePointTenths;

	CImageList *          m_nodeImageList;
	CImageList *          m_buttonImageList;
	Unicode::NarrowString m_unicodeFilename;

	void                  setupFontControls ();

	int                   convertPointTenthsToPixels (int pointTenths);
	int                   convertPixelsToPointTenths (int pixels);

	void                  createNewRange (bool inclusive);
	void                  moveItem (HTREEITEM htitem, HTREEITEM insertAfter);
	HTREEITEM             getRangeSelection (bool showmsg);

	IdSet_t &             getIdSet (IdSet_t &);

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UIFONTBUILDERDLG_H__1A78D845_7672_4531_99B4_AEECC647D137__INCLUDED_)
