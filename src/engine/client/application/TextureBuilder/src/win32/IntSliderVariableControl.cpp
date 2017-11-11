// ======================================================================
//
// IntSliderVariableControl.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "FirstTextureBuilder.h"
#include "IntSliderVariableControl.h"

#include "IntVariableElement.h"

#include <string>

// ======================================================================

namespace
{
	const int ms_editHeight   = 25;
	const int ms_editWidth    = 45;
	const int ms_sliderHeight = 30;
}

// ======================================================================

IntSliderVariableControl::IntSliderVariableControl(CWnd *parentWindow, IntVariableElement &variableElement)
:	VariableControl(variableElement),
	m_sliderControl(new CSliderCtrl()),
	m_editControl(new CEdit()),
	m_variableNameControl(new CStatic()),
	m_ignoreNextEditChange(true)
{
	RECT  r;

	//-- create slider
	r.left   = 5;
	r.right  = 200;
	r.top    = 20;
	r.bottom = 40;

	const BOOL scSuccess = m_sliderControl->Create(TBS_HORZ | TBS_NOTICKS | TBS_BOTH | WS_CHILD, r, parentWindow, reinterpret_cast<UINT>(this));
	FATAL(!scSuccess, ("failed to create slider control"));

	//-- create edit control
	r.left   = 125;
	r.right  = 200;
	r.top    = 5;
	r.bottom = 20;

	const BOOL ecSuccess = m_editControl->Create(ES_LEFT | WS_CHILD, r, parentWindow, reinterpret_cast<UINT>(this));
	FATAL(!ecSuccess, ("failed to create edit control"));

	IntVariableElement &element = getIntVariableElement();
	setEditValue(element.getValue());

	//-- create variable name control
	// (CStatic wasn't working out)
	r.left   = 5;
	r.right  = 120;
	r.top    = 5;
	r.bottom = 20;


	const BOOL vncSuccess = m_variableNameControl->Create(element.getVariableName().c_str(), SS_LEFT | WS_CHILD, r, parentWindow);
	FATAL(!vncSuccess, ("failed to create variable name static control"));
}

// ----------------------------------------------------------------------

IntSliderVariableControl::~IntSliderVariableControl()
{
	delete m_variableNameControl;
	delete m_editControl;
	delete m_sliderControl;
}

// ----------------------------------------------------------------------

int IntSliderVariableControl::setupDimensions(int startX, int startY, int endX)
{
	int currentY = startY;

	//-- handle edit
	const BOOL eSuccess = m_editControl->SetWindowPos(&CWnd::wndTop, endX - ms_editWidth, currentY, ms_editWidth, ms_editHeight, SWP_SHOWWINDOW);
	DEBUG_FATAL(!eSuccess, ("failed to resize edit window"));

	//-- handle variable name control
	const BOOL vnSuccess = m_variableNameControl->SetWindowPos(&CWnd::wndTop, startX, currentY, endX - ms_editWidth, ms_editHeight, SWP_SHOWWINDOW);
	DEBUG_FATAL(!vnSuccess, ("failed to resize variable name window"));
	currentY += ms_editHeight;

	//-- handle slider
	const BOOL sSuccess = m_sliderControl->SetWindowPos(&CWnd::wndTop, startX, currentY, endX - startX, ms_sliderHeight, SWP_SHOWWINDOW);
	DEBUG_FATAL(!sSuccess, ("failed to resize slider window"));
	currentY += ms_sliderHeight;

	return currentY;
}

// ----------------------------------------------------------------------

void IntSliderVariableControl::setEditValue(int value)
{
	char  conversionBuffer[32];
	IGNORE_RETURN(_itoa(value, conversionBuffer, 10));
	m_editControl->SetWindowText(conversionBuffer);
}

// ----------------------------------------------------------------------

int IntSliderVariableControl::getEditValue() const
{
	CString valueString;

	m_editControl->GetWindowText(valueString);
	return atoi(valueString);
}

// ----------------------------------------------------------------------

bool IntSliderVariableControl::saveControlToVariable(const CWnd *actionSource)
{
	bool getFromEdit = true;

	const CSliderCtrl *const slider = dynamic_cast<const CSliderCtrl*>(actionSource);
	if (slider)
	{
		getFromEdit = false;
	}

	IntVariableElement &element = getIntVariableElement();

	if (getFromEdit)
	{
		//-- HACK HACK HACK HACK
		// we need to ignore the next edit change in certain circumstances.  This occurs because
		// we only have the ability to receive a notification when the edit changes for any reason
		// (including setting the value of the edit).  We do not have the ability to receive a
		// notification when the user presses enter in the edit.

		// edit box control changed
		if (m_ignoreNextEditChange)
		{
			// skip this input
			m_ignoreNextEditChange = false;
			return false;
		}
		else
		{
			// user modified edit control value

			//-- retrieve variable from edit
			const int newValue = getEditValue();

			//-- validate range
			if (element.isBounded())
			{
				if (!WithinRangeInclusiveInclusive(element.getLowerBoundInclusive(), newValue, element.getUpperBoundInclusive()))
				{
					//-- reject, not within valid range, reset to previous value
					m_ignoreNextEditChange = true;
					setEditValue(element.getValue());
					return false;
				}
			}

			//-- update slider
			m_sliderControl->SetPos(newValue);

			const bool elementChangedValue = (newValue != element.getValue());

			//-- update the variable element
			element.setValue(newValue);

			//-- report this to the document
			return elementChangedValue;
		}
	}
	else
	{
		// value changed by slider

		//-- retrieve variable from slider
		const int newValue = m_sliderControl->GetPos();

		//-- update edit
		m_ignoreNextEditChange = true;
		setEditValue(newValue);
		
		//-- update the variable element
		const bool elementChangedValue = (newValue != element.getValue());
		element.setValue(newValue);

		//-- report this to the document
		return elementChangedValue;
	}
}

// ----------------------------------------------------------------------

void IntSliderVariableControl::loadControlFromVariable()
{
	//-- retrieve value from variable element

	IntVariableElement &element = getIntVariableElement();
	const int           value   = element.getValue();

	//-- set slider value
	m_sliderControl->SetPos(value);
	m_sliderControl->SetRange(element.getLowerBoundInclusive(), element.getUpperBoundInclusive(), true);

	//-- set edit value
	m_ignoreNextEditChange = true;
	setEditValue(value);
}

// ----------------------------------------------------------------------

IntVariableElement &IntSliderVariableControl::getIntVariableElement()
{
	return dynamic_cast<IntVariableElement&>(getVariableElement());
}

// ======================================================================
