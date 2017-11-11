#if !defined(AFX_SELECTREGIONCANVAS_H__996F80F5_4998_46E2_9236_8EEF915603BB__INCLUDED_)
#define AFX_SELECTREGIONCANVAS_H__996F80F5_4998_46E2_9236_8EEF915603BB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SelectRegionCanvas.h : header file
//

#include "SelectionRect.h"

#include "UITypes.h"
#include "UISmartPointer.h"
#include "UIString.h"

class UIDirect3DPrimaryCanvas;
class UIImage;

/////////////////////////////////////////////////////////////////////////////
// SelectRegionCanvas window

class SelectRegionCanvas : public CWnd
{
// Construction
public:

	// Notification codes
	enum {
		 NC_rectChanged=1
		,NC_zoomUp
		,NC_zoomDown
	};

	static bool registerClass();

	SelectRegionCanvas();

	void construct(UIImage &image, const UISize &imageSize, const UINarrowString &value);

// Attributes
public:

	const UIRect &region() const { return mRegion; }
	      UIRect &region()       { return mRegion; }

	const CString &regionText() const { return mRegionText; }

	const UISize  &imageSize() const { return mImageSize; }

	float         zoomLevel() const { return mZoomLevel; }
	void          zoomLevel(float z) { mZoomLevel=z; }

	bool          drawHighlight()        const { return mDrawHighlight; }
	void          drawHighlight(bool x)        { mDrawHighlight=x; }

	SelectionRect &selectionRect() { return mSelectionRect; }

	void updateValue(const CString &i_newValue);
	void updateScrollbars(const UIPoint * center=0);

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SelectRegionCanvas)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~SelectRegionCanvas();

	// Generated message map functions
protected:

	void _notifyRectChanged();
	void _notifyZoomUp();
	void _notifyZoomDown();
	void _updateScrollbars(const CRect &windowRect, const UIPoint * center);
	void _updateCanvasScrollPosition(UINT uMsg, UINT sbCode);
	CPoint _inverseTranslatePosition(const CPoint &p);

	afx_msg LRESULT OnApp(WPARAM, LPARAM);

	//{{AFX_MSG(SelectRegionCanvas)
	afx_msg UINT OnGetDlgCode();
	afx_msg UINT OnNcHitTest(CPoint point);
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnDestroy();
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	// ==========================================================================

	UISmartPointer<UIImage>           mImage;
	UIDirect3DPrimaryCanvas          *mDisplay;
	UISize                            mImageSize;
	float                             mZoomLevel;
	UIPoint                           mScrollOffset;
	SelectionRect                     mSelectionRect;
	UIRect                            mRegion;
	CString                           mRegionText;
	int                               m_mouseWheelCounter;
	bool                              mDrawHighlight;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECTREGIONCANVAS_H__996F80F5_4998_46E2_9236_8EEF915603BB__INCLUDED_)
