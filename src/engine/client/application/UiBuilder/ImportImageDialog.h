#if !defined(AFX_IMPORTIMAGEDIALOG_H__9C0D8000_71B6_461A_8222_DC4437BCC36E__INCLUDED_)
#define AFX_IMPORTIMAGEDIALOG_H__9C0D8000_71B6_461A_8222_DC4437BCC36E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ImportImageDialog.h : header file
//

#include <UITypes.h>
#include <list>

class ProgressDialog;
class UIImageFrame;

/////////////////////////////////////////////////////////////////////////////
// ImportImageDialog dialog

class ImportImageDialog : public CDialog
{
// Construction
public:
	ImportImageDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(ImportImageDialog)
	enum { IDD = IDD_IMPORTIMAGE };
	CSliderCtrl	m_tileMin;
	CSliderCtrl	m_tileMax;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ImportImageDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	static void _saveBitmapToFile( HDC hdc, HBITMAP hbmp, const char *Filename );
	UIImageFrame *_generateImageFrameFromData();
	void _sliceImagesToFiles(ProgressDialog *progress);
	void _enableControls();
	void _calculateRectangleSlices(const UIRect &r, long TileSize);
	void _updateFragmentData();
	void _loadData();
	void _saveData();

	typedef std::list<UIRect>		 FragmentRectangleList;

	char m_sourceFile[_MAX_PATH];
	char m_alphaFile[_MAX_PATH];
	char m_parentPath[_MAX_PATH];

	FragmentRectangleList m_fragmentRectangles;

	long m_compressionMethodIndex;
	int  m_tileMaxIndex;
	int  m_tileMinIndex;
	long m_fileWidth;
	long m_fileHeight;

	bool m_fileIsValid;
	bool m_alphaFileIsValid;
	bool m_fileIsBitmap;
	bool m_useWholeImage;
	bool m_generateFragments;
	bool m_compressFragments;

	static char *s_compressionMethods[];
	static long  s_tileSizes[];

	// Generated message map functions
	//{{AFX_MSG(ImportImageDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnChangeSubimagerect();
	afx_msg void OnChangeSourcefile();
	afx_msg void OnBrowsesource();
	afx_msg void OnUsesubimage();
	afx_msg void OnUsewholeimage();
	afx_msg void OnCompressfragments();
	afx_msg void OnGeneratefragments();
	afx_msg void OnSelchangeCompressionformat();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMPORTIMAGEDIALOG_H__9C0D8000_71B6_461A_8222_DC4437BCC36E__INCLUDED_)
