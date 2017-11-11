//
// SmartSliderCtrl.h - header for slider subclassing module
//
///////////////////////////////////////////////////////////////////
//
//	Copyright (C) 1998 Rick York - Feel free to use this as
//	you wish provided that this notice remains intact and
//	a small credit is given (like in an about box :).
//
//	This class is used to link a slider with an edit box
//	for coordinated updates.
//
///////////////////////////////////////////////////////////////////
//
//@tabs=4

//-------------------------------------------------------------------

#ifndef INCLUDED_SmartSliderCtrl_H
#define INCLUDED_SmartSliderCtrl_H

//-------------------------------------------------------------------

class SmartEditCtrl;

//-------------------------------------------------------------------

class SmartSliderCtrl : public CSliderCtrl
{
public:
	// Construction
	SmartSliderCtrl();

	SmartEditCtrl *	m_pEdit;

	int		m_iMin;
	int		m_iMax;
	int		m_iPosition;
	BOOL	m_bVertical;

	void	SetSlidePos( const int pos );
	void	SetSlideRange( const int min, const int max );

	void	LinkSmartEditCtrl( SmartEditCtrl *pedit );

	// Implementation
protected:
	//{{AFX_MSG(SmartSliderCtrl)
	afx_msg void HScroll( UINT ncode, UINT pos );
	afx_msg void VScroll( UINT ncode, UINT pos );
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//-------------------------------------------------------------------

#endif