#if !defined(AFX_NEWHEADERCTRL_H__99EB0481_4FA1_11D1_980A_004095E0DEFA__INCLUDED_)
#define AFX_NEWHEADERCTRL_H__99EB0481_4FA1_11D1_980A_004095E0DEFA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// NewHeaderCtrl.h : header file
//

#include <map>

/////////////////////////////////////////////////////////////////////////////
// CNewHeaderCtrl window

class CNewHeaderCtrl : public CHeaderCtrl
{
// Construction
public:
	CNewHeaderCtrl();

// Attributes
protected:
	CImageList *m_pImageList;
	std::map<int, int> m_mapImageIndex;

private:
	BOOL m_bAutofit;
	void Autofit(int nOverrideItemData = -1, int nOverrideWidth = 0);

// Operations
public:
	BOOL m_RTL;

	void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );
	CImageList* SetImageList( CImageList* pImageList );
	int GetItemImage( int nItem );
	void SetItemImage( int nItem, int nImage );
	void SetAutofit(bool bAutofit = true) { m_bAutofit = bAutofit; Autofit(); }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNewHeaderCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CNewHeaderCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CNewHeaderCtrl)
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEWHEADERCTRL_H__99EB0481_4FA1_11D1_980A_004095E0DEFA__INCLUDED_)
