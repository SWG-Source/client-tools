//
//
//
//
//
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "SmartEditCtrl.h"

#include "PropertyView.h"
#include "SmartSliderCtrl.h"

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(SmartEditCtrl, CEdit)
	//{{AFX_MSG_MAP(SmartEditCtrl)
	ON_WM_CHAR()
	ON_CONTROL_REFLECT(EN_UPDATE, OnUpdate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

SmartEditCtrl::SmartEditCtrl (PropertyView* propertyView) :
	m_propertyView (propertyView)
{
	m_wParseType = SES_ALL;
	m_pSlider = NULL;
	m_iValue = INT_MIN;
	m_pFormat = NULL;
	m_bUseFp = FALSE;
}

//-------------------------------------------------------------------

SmartEditCtrl::~SmartEditCtrl()
{
	if (m_pFormat)
		delete [] m_pFormat;
}

//-------------------------------------------------------------------

void SmartEditCtrl::SetParseType( WORD type )
{
	m_wParseType = type;
}

//-------------------------------------------------------------------

void SmartEditCtrl::OnChar( UINT nChar, UINT nRepCnt, UINT nFlags )
{
	WORD type = SES_OTHERCHARS;		// default
	int	alpha = IsCharAlpha( (TCHAR)nChar );
	int	number = IsCharAlphaNumeric( (TCHAR)nChar ) && ! alpha;

	if( nChar < 0x20 )
		type = SES_ALL; 			// always allow control chars ???
	else if( number )
		type = SES_NUMBERS | SES_SIGNED | SES_FLOATINGPT | SES_HEXADECIMAL;
	else if( alpha )
		{
		type = SES_LETTERS;
		int uprval = toupper( (TCHAR)nChar );
		if( uprval == 'E' )
			type |= ( SES_FLOATINGPT | SES_HEXADECIMAL );
		else if( uprval == 'X' )
			type |= SES_HEXADECIMAL;
		else if( ( uprval >= 'A' ) && ( uprval <= 'F' ) )
			type |= SES_HEXADECIMAL;
		}
	else if( (TCHAR)nChar == '-' )		// dash
		type = SES_SIGNED | SES_FLOATINGPT;
	else if( (TCHAR)nChar == '.' )		// dot
		type = SES_FLOATINGPT;
	else if( (TCHAR)nChar == '_' )		// underscore
		type = SES_UNDERSCORE | SES_OTHERCHARS;

	if( m_wParseType & type )
		{
		CEdit::OnChar(nChar, nRepCnt, nFlags);	// permitted

		if (m_propertyView)
			m_propertyView->ApplyChanges ();
		}
	else
		{
		// illegal character - inform parent
		OnBadInput();
		}
}

//-------------------------------------------------------------------

void SmartEditCtrl::OnBadInput()
{
	MessageBeep((UINT)-1);
}

//-------------------------------------------------------------------

void SmartEditCtrl::OnUpdate() 
{
	if( ! m_pSlider )
		return;

	CString buf;
	GetWindowText (buf);

	int	error = TRUE;
	if( m_bUseFp )
	{
		m_fValue = static_cast<float> (atof( buf ));
		if( m_fValue < m_fMin )
		{
			m_fValue = m_fMin;
			OnBadInput();
		}
		else if( m_fValue > m_fMax )
		{
			m_fValue = m_fMax;
			OnBadInput();
		}
		else
			error = FALSE;

		// scale to integer

		float dpos = ( m_fValue - m_fMin ) * (float)m_iRange / m_fRange;
		m_iValue = (int)dpos;
	}
	else
	{
		m_iValue = atoi( buf );
		if( m_iValue < m_iMin )
		{
			m_iValue = m_iMin;
			OnBadInput();
		}
		else if( m_iValue > m_iMax )
		{
			m_iValue = m_iMax;
			OnBadInput();
		}
		else
			error = FALSE;
	}

	if( m_pSlider )
		m_pSlider->SetSlidePos( m_iValue );

	if( error )
	{
		int posn = m_iValue;
		m_iValue = INT_MIN;
		UpdateEdit( posn );
	}
}

//-------------------------------------------------------------------

void SmartEditCtrl::UpdateEdit (int position)
{
	CString buffer;

	if( position == m_iValue )
		return;

	m_iValue = position;
	if( m_bUseFp )
	{
		// scale to float value
		m_fValue = m_fMin + (float)m_iValue * m_fRange / 1000.0f;
		buffer.Format (m_pFormat, m_fValue);
	}
	else
		buffer.Format ("%d", m_iValue);

	SetWindowText (buffer);

	if (m_propertyView)
		m_propertyView->ApplyChanges ();
}

//-------------------------------------------------------------------

void SmartEditCtrl::LinkSmartSliderCtrl (SmartSliderCtrl* slider)
{
	m_pSlider = slider;
	ASSERT (m_pSlider);

	m_pSlider->LinkSmartEditCtrl (this);
}

//-------------------------------------------------------------------

void SmartEditCtrl::InitSlider()
{
	if (!m_pSlider)
		return;

	m_pSlider->LinkSmartEditCtrl (this);
	m_iRange = m_iMax - m_iMin;
	m_pSlider->SetSlideRange( m_iMin, m_iMax );
	int freq = m_iRange / m_iBands;
	m_pSlider->SetTicFreq( freq );
}

//-------------------------------------------------------------------

void SmartEditCtrl::SetFormatString (const char* formatString)
{
	if (m_pFormat)
		delete [] m_pFormat;

	// this method is used so we can track allocations
	m_pFormat = DuplicateString (formatString);
}

//-------------------------------------------------------------------

void SmartEditCtrl::SetParams (int imin, int imax, int ticks)
{
	m_bUseFp = FALSE;
	m_iBands = ticks;
	m_iMin   = imin;
	m_iMax   = imax;

	InitSlider();

	SetFormatString ("%i");
	SetParseType ((imin < 0) ? static_cast<WORD> ((SES_NUMBERS | SES_SIGNED)) : static_cast<WORD> (SES_NUMBERS));
}

//-------------------------------------------------------------------

void SmartEditCtrl::SetParams (float fmin, float fmax, int ticks, const char* formatString)
{
	m_bUseFp = TRUE;
	m_iBands = ticks;
	m_fRange = fmax - fmin;
	m_fMin   = fmin;
	m_fMax   = fmax;
	m_iMin   = 0;
	m_iMax   = 1000;

	InitSlider();

	SetFormatString (formatString);
	SetParseType (SES_FLOATINGPT);
}

//-------------------------------------------------------------------

bool SmartEditCtrl::SetValue (int value)
{
	if (m_bUseFp)
		return false;

	if (value < m_iMin || value > m_iMax)
		return false;

	CString buffer;
	buffer.Format (m_pFormat, value);
	SetWindowText (buffer);

	return true;
}

//-------------------------------------------------------------------

bool SmartEditCtrl::SetValue (float value)
{
	if (!m_bUseFp)
		return false;

	if (value < m_fMin || value > m_fMax)
		return false;

	CString buffer;
	buffer.Format (m_pFormat, value);
	SetWindowText (buffer);

	return true;
}

//-------------------------------------------------------------------

int SmartEditCtrl::GetValueInt () const
{
	CString buffer;
	GetWindowText (buffer);

	return atoi (buffer);
}

//-------------------------------------------------------------------

float SmartEditCtrl::GetValueFloat () const
{
	CString buffer;
	GetWindowText (buffer);

	return static_cast<float> (atof (buffer));
}

//-------------------------------------------------------------------


