//
//
//
//
//
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "SmartSliderCtrl.h"
#include "SmartEditCtrl.h"

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(SmartSliderCtrl, CSliderCtrl)
	//{{AFX_MSG_MAP(SmartSliderCtrl)
	ON_WM_HSCROLL_REFLECT()
	ON_WM_VSCROLL_REFLECT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

SmartSliderCtrl::SmartSliderCtrl() : CSliderCtrl()
{
	m_pEdit = NULL;
	m_iPosition = INT_MAX;
	m_bVertical = FALSE;
}

//-------------------------------------------------------------------

void SmartSliderCtrl::LinkSmartEditCtrl( SmartEditCtrl *pedit )
{
	m_pEdit = dynamic_cast<SmartEditCtrl*> (pedit);
	ASSERT (m_pEdit);
	DWORD style = GetStyle();
	if( style & TBS_VERT )
		m_bVertical = TRUE;
}

//-------------------------------------------------------------------

void SmartSliderCtrl::SetSlidePos( const int pos )
{
	if( m_bVertical )
		m_iPosition = m_iMax - ( pos - m_iMin );
	else
		m_iPosition = pos;

	SetPos( m_iPosition );
}

//-------------------------------------------------------------------

void SmartSliderCtrl::SetSlideRange( const int imin, const int imax )
{
	m_iMin = imin;
	m_iMax = imax;
	SetRange( m_iMin, m_iMax );
}

//-------------------------------------------------------------------

void SmartSliderCtrl::HScroll( UINT ncode, UINT pos )
{
	CSliderCtrl::OnHScroll( ncode, pos, NULL );
	if( ! m_pEdit )
		return;

	switch( ncode )
		{
		case TB_PAGEUP :
		case TB_PAGEDOWN :
			m_iPosition = GetPos();
			m_pEdit->UpdateEdit( m_iPosition );
			break;
	
		case TB_THUMBTRACK :
			m_iPosition = pos;
			m_pEdit->UpdateEdit( pos );
			break;
		}
}

//-------------------------------------------------------------------

void SmartSliderCtrl::VScroll( UINT ncode, UINT pos )
{
	CSliderCtrl::OnVScroll( ncode, pos, NULL );
	if( ! m_pEdit )
		return;

	switch( ncode )
		{
		// we have to play some games with vertical sliders -
		// max position is on the bottom and min is on the top

		case TB_PAGEUP :
		case TB_PAGEDOWN :
			m_iPosition = m_iMax - ( GetPos() - m_iMin );
			m_pEdit->UpdateEdit( m_iPosition );
			break;

		case TB_THUMBTRACK :
			m_iPosition = m_iMax - ( pos - m_iMin );
			m_pEdit->UpdateEdit( m_iPosition );
		}
}

//-------------------------------------------------------------------

