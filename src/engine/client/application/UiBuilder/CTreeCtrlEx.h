#if !defined(AFX_CTREECTRLEX_H__FD20A7E3_5ECE_11D5_BDE8_00500409FE16__INCLUDED_)
#define AFX_CTREECTRLEX_H__FD20A7E3_5ECE_11D5_BDE8_00500409FE16__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CTreeCtrlEx.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTreeCtrlEx window

class CTreeCtrlEx : public CTreeCtrl
{
// Construction
public:
	CTreeCtrlEx();

   int  countSelections();
   void clearSelections(bool multi_only=false);
   bool isSelected(HTREEITEM item) { return (GetItemState(item,TVIS_SELECTED)&TVIS_SELECTED)!=0; }
   void selectItem(HTREEITEM item, bool select);

   bool isExpanded(HTREEITEM item) { return (GetItemState(item,TVIS_EXPANDED)&TVIS_EXPANDED)!=0; }

   HTREEITEM getFirstSelectedItem();
   HTREEITEM getNextSelectedItem(HTREEITEM x);
   HTREEITEM getLastSelectedItem();
   HTREEITEM getPrevSelectedItem(HTREEITEM x);

	void FullyExpand(HTREEITEM x);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTreeCtrlEx)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTreeCtrlEx();

	// Generated message map functions
protected:


   void _selectMultiple(HTREEITEM item, UINT flags);
   bool _selectItemRange(HTREEITEM start, HTREEITEM end);
   void _notifySelect(HTREEITEM item, HTREEITEM old_item);

	//{{AFX_MSG(CTreeCtrlEx)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetfocus(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg BOOL OnKillfocus(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg BOOL OnItemexpanding(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnSelchanging(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg BOOL OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

   HTREEITEM clicked_item, initial_sel;
   CPoint click_pt;
   bool click_handled, supress_sel_notifications;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CTREECTRLEX_H__FD20A7E3_5ECE_11D5_BDE8_00500409FE16__INCLUDED_)
