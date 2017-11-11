// ======================================================================
//
// DialogIntSlider.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#if !defined(AFX_DIALOGINTSLIDER_H__1FBD81F4_A367_4E0F_9E65_64E2EF43267A__INCLUDED_)
#define AFX_DIALOGINTSLIDER_H__1FBD81F4_A367_4E0F_9E65_64E2EF43267A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif

// ======================================================================

#include "resource.h"

class IntSliderElement;

// ======================================================================

class DialogIntSlider : public CDialog
{
public:

	explicit DialogIntSlider(const IntSliderElement &intSliderElement, CWnd* pParent = NULL);

	void setSliderRange(int rangeFirst, int rangeLast, int valuesPerTick);

	//lint -save -e1925 // public data member
	//{{AFX_DATA(DialogIntSlider)
	enum { IDD = IDD_INT_SLIDER };
	CStatic	m_valueControl;
	CSliderCtrl	m_intSliderControl;
	int		m_intSliderValue;
	CString	m_convertedValueString;
	CString	m_valueLabel;
	//}}AFX_DATA
	//lint -restore

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DialogIntSlider)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:

	// Generated message map functions
	//{{AFX_MSG(DialogIntSlider)
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	const IntSliderElement &m_intSliderElement;

	int                     m_rangeFirst;
	int                     m_rangeLast;
	int                     m_valuesPerTick;

private:
	// disabled
	DialogIntSlider();
	DialogIntSlider(const DialogIntSlider&);
	DialogIntSlider &operator =(const DialogIntSlider&);
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
//}}AFX_INSERT_LOCATION

// ======================================================================

#endif
