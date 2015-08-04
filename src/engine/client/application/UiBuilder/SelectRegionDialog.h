#if !defined(AFX_SELECTREGIONDIALOG_H__A8CB1A4F_D68A_45DF_895A_3C599662A730__INCLUDED_)
#define AFX_SELECTREGIONDIALOG_H__A8CB1A4F_D68A_45DF_895A_3C599662A730__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SelectRegionDialog.h : header file
//

#include "UISmartPointer.h"
#include "SelectRegionCanvas.h"
#include "UIString.h"

class UIImageStyle;

/////////////////////////////////////////////////////////////////////////////
// SelectRegionDialog dialog

class SelectRegionDialog : public CDialog
{
// Construction
public:
	SelectRegionDialog(CWnd* pParent = NULL);   // standard constructor

	bool editProperty(UIImageStyle *theStyle, const UINarrowString &Value);
	bool editProperty(UIImage *theImage, const UINarrowString &Value);

	const CString &regionText() const { return m_canvas.regionText(); }

// Dialog Data
	//{{AFX_DATA(SelectRegionDialog)
	enum { IDD = IDD_SELECTREGION };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SelectRegionDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:

	void _updateTextbox();
	void _updateSelectionRect();
	void _sizeToContent();
	void _enableControls();
	void _showZoomLevel();
	void _readRectFromTextbox();
	void _positionControl(int id, int x, int y, int cx, int cy);
	void _zoomUp();
	void _zoomDown();

	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnCanvasRectChanged();
	afx_msg void OnCanvasZoomUp();
	afx_msg void OnCanvasZoomDown();

	// Generated message map functions
	//{{AFX_MSG(SelectRegionDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnDrawhighlight();
	afx_msg void OnZoomin();
	afx_msg void OnZoomout();
	afx_msg void OnChangeValue();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CSize                             mOldSize;
	CSize                             mInitialSize;
	SelectRegionCanvas                m_canvas;
	int                               m_mouseWheelCounter;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECTREGIONDIALOG_H__A8CB1A4F_D68A_45DF_895A_3C599662A730__INCLUDED_)
