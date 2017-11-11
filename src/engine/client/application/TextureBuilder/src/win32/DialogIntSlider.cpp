// ======================================================================
//
// DialogIntSlider.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "FirstTextureBuilder.h"
#include "DialogIntSlider.h"

#include "IntSliderElement.h"

// ======================================================================

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// ======================================================================

DialogIntSlider::DialogIntSlider(const IntSliderElement &intSliderElement, CWnd* pParent)
:	CDialog(DialogIntSlider::IDD, pParent),
	m_valueControl(),
	m_intSliderControl(),
	m_intSliderValue(0),
	m_convertedValueString(),
	m_valueLabel(),
	m_intSliderElement(intSliderElement),
	m_rangeFirst(0),
	m_rangeLast(100),
	m_valuesPerTick(5)
{
	//{{AFX_DATA_INIT(DialogIntSlider)
	m_intSliderValue = 0;
	m_convertedValueString = _T("");
	m_valueLabel = _T("");
	//}}AFX_DATA_INIT
}

// ----------------------------------------------------------------------

void DialogIntSlider::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogIntSlider)
	DDX_Control(pDX, IDC_VALUE, m_valueControl);
	DDX_Control(pDX, IDC_SLIDER, m_intSliderControl);
	DDX_Slider(pDX, IDC_SLIDER, m_intSliderValue);
	DDX_Text(pDX, IDC_VALUE, m_convertedValueString);
	DDX_Text(pDX, IDC_VALUE_LABEL, m_valueLabel);
	//}}AFX_DATA_MAP

	if (!pDX->m_bSaveAndValidate)
	{
		// entering dialog
		m_intSliderControl.SetTicFreq(m_valuesPerTick);
		m_intSliderControl.SetRange(m_rangeFirst, m_rangeLast, true);
	}
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(DialogIntSlider, CDialog)
	//{{AFX_MSG_MAP(DialogIntSlider)
	ON_WM_HSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

void DialogIntSlider::setSliderRange(int rangeFirst, int rangeLast, int valuesPerTick)
{
	m_rangeFirst    = rangeFirst;
	m_rangeLast     = rangeLast;
	m_valuesPerTick = valuesPerTick;
}

// ----------------------------------------------------------------------

void DialogIntSlider::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CSliderCtrl *const sliderControl = dynamic_cast<CSliderCtrl*>(pScrollBar);
	if (sliderControl)
	{
		// slider is modified

		//-- update converted value
		char      displayValue[32];
		const int convertedValue = m_intSliderElement.convertSliderToDisplayValue(sliderControl->GetPos());
		IGNORE_RETURN(_itoa(convertedValue, displayValue, 10));
		
		//-- set text into static
		m_valueControl.SetWindowText(displayValue);
	}

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

// ======================================================================
