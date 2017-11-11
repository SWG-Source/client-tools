//======================================================================
//
// CuiTransfer.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiTransfer.h"

#include "UIButton.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIImage.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UISliderbar.h"
#include "UIText.h"
#include "UITextBox.h"
#include "UIUtils.h"
#include "UnicodeUtils.h"

//======================================================================

namespace CuiTransferNamespace
{
	namespace DataProperties
	{
		const UILowerString ConversionRatioFrom("ConversionRatioFrom");
		const UILowerString ConversionRatioTo("ConversionRatioTo");
	}

	const int cs_defaultConversionRatioFrom = 1;
	const int cs_defaultConversionRatioTo   = 1;
}

using namespace CuiTransferNamespace;

//======================================================================

CuiTransfer::CuiTransfer (UIPage & page) :
CuiMediator("CuiTransfer", page),
UIEventCallback(),
m_startingFromText(0),
m_startingToText(0),
m_currentFromText(0),
m_currentToText(0),
m_slider(0),
m_buttonCancel(0),
m_buttonRevert(0),
m_buttonOk(0),
m_transaction(0),
m_currentFrom(0),
m_currentTo(0),
m_startingFrom(0),
m_startingTo(0),
m_conversionRatioFrom(cs_defaultConversionRatioFrom),
m_conversionRatioTo(cs_defaultConversionRatioTo),
m_startingTotal(0),
m_modulusFrom(0),
m_modulusTo(0)
{
	getCodeDataObject (TUIButton,     m_buttonCancel,      "btnCancel");
	getCodeDataObject (TUIButton,     m_buttonRevert,      "btnRevert");
	getCodeDataObject (TUIButton,     m_buttonOk,          "btnOk");
	getCodeDataObject (TUISliderbar,  m_slider,            "transaction.slider");
	getCodeDataObject (TUIText,       m_startingFromText,  "transaction.lblStartingFrom");
	getCodeDataObject (TUIText,       m_startingToText,    "transaction.lblStartingTo");
	getCodeDataObject (TUITextbox,    m_currentFromText,   "transaction.txtInputFrom");
	getCodeDataObject (TUITextbox,    m_currentToText,     "transaction.txtInputTo");
	getCodeDataObject (TUIPage,       m_transaction,       "transaction");

	
	registerMediatorObject(*m_buttonCancel, true);
	registerMediatorObject(*m_buttonRevert, true);
	registerMediatorObject(*m_buttonOk, true);
	registerMediatorObject(*m_slider, true);
	registerMediatorObject(*m_currentFromText, true);
	registerMediatorObject(*m_currentToText, true);
}

//----------------------------------------------------------------------

void CuiTransfer::OnSliderbarChanged(UIWidget *context)
{
	if (context == m_slider)
	{
		matchValuesToSlider();
	}
}

//----------------------------------------------------------------------

void CuiTransfer::OnButtonPressed   (UIWidget *context)
{
	if (context == m_buttonCancel)
	{
		deactivate ();
	}
	else if (context == m_buttonRevert)
	{
		revert();
	}
}

//----------------------------------------------------------------------

/** Determine which text value changed, update both text widgets and then set the slider to match them. 
*/
void CuiTransfer::OnTextboxChanged(UIWidget * context)
{
	const int from  = m_currentFrom;
	const int to    = m_currentTo;
	const int total = calculateTotal(from, to);

	if (context == m_currentFromText)
	{
		//check the values before any changes
		if(!checkTotal(from, to))
			return;

		//get the int value
		int newFrom;
		m_currentFromText->GetPropertyInteger(UIText::PropertyName::LocalText, newFrom);

		//if it's not evenly divisble by the conversion rate, floor it down to the next one
		if(newFrom % m_conversionRatioFrom != 0)
			newFrom -= newFrom % m_conversionRatioFrom;
		newFrom = clamp(0, newFrom, getMaxFrom());

		//determine and clamp to value
		int newTo = (total - (newFrom * m_conversionRatioTo)) / m_conversionRatioFrom;
		newTo = clamp(0, newTo, getMaxTo());

		//check the new values
		if(!checkTotal(newFrom, newTo))
			return;

		//store and display the new values
		setCurrentFromAndTo(newFrom, newTo);
		matchSliderToValues();
	}
	else if(context == m_currentToText)
	{
		//check the values before any changes
		if(!checkTotal(from, to))
			return;

		//get the text string
		//get the int value
		int newTo;
		m_currentToText->GetPropertyInteger(UIText::PropertyName::LocalText, newTo);

		//if it's not evenly divisble by the conversion rate, floor it down to the next one
		if(newTo % m_conversionRatioTo != 0)
			newTo -= newTo % m_conversionRatioTo;
		newTo = clamp(0, newTo, getMaxTo());
		
		//determine and clamp from value
		int newFrom = (total - (newTo * m_conversionRatioFrom)) / m_conversionRatioTo;
		newFrom = clamp(0, newFrom, getMaxFrom());

		//check the new values
		if(!checkTotal(newFrom, newTo))
			return;

		//store and display the new values
		setCurrentFromAndTo(newFrom, newTo);
		matchSliderToValues();
	}
}

//----------------------------------------------------------------------

/** Match the text fields to the value of the slider
*/
void CuiTransfer::matchValuesToSlider()
{
	//check the values before any changes
	if(!checkTotal(m_currentFrom, m_currentTo))
		return;

	//determine the new values
 	const int numTicks = m_slider->GetUpperLimit();
	const long  value = m_slider->GetValue();
	int newFrom       = (numTicks - value) * m_conversionRatioFrom;
	int newTo         = value * m_conversionRatioTo;
	newFrom = clamp(0, newFrom, getMaxFrom());
	newTo   = clamp(0, newTo,   getMaxTo());

	//check the new values
	if(!checkTotal(newFrom, newTo))
		return;

	//store the new values
	setCurrentFromAndTo(newFrom, newTo);
}

//----------------------------------------------------------------------

/** Match the slider's position to the values in the text fields
*/
void CuiTransfer::matchSliderToValues()
{
	//check the values before any changes
	if(!checkTotal(m_currentFrom, m_currentTo))
		return;

	//determine the new tick for the slider, push the value in
	const int numTicks = m_slider->GetUpperLimit();
	int toTick = m_currentTo / m_conversionRatioTo;
	toTick = clamp(0, toTick, numTicks);

	//set the new slider value
	m_slider->SetValue(toTick, false);
}

//----------------------------------------------------------------------

/** Put the requested from and to values into the UI
*/
void CuiTransfer::setCurrentFromAndTo(int currentFrom, int currentTo)
{
	//check the values before any changes
	if(!checkTotal(currentFrom, currentTo))
		return;
	
	displayFrom(currentFrom);
	m_currentFrom = currentFrom;

	displayTo(currentTo);
	m_currentTo = currentTo;
}

//----------------------------------------------------------------------

/** Put back the original text and slider values
*/
void CuiTransfer::revert()
{
	setCurrentFromAndTo(m_startingFrom, m_startingTo);
	matchSliderToValues();
}

//----------------------------------------------------------------------

/** Return the common-denominator total
*/
int CuiTransfer::calculateTotal(int from, int to) const
{
	return (from * m_conversionRatioTo) + (to * m_conversionRatioFrom);
}

//----------------------------------------------------------------------

/** Sanity check the requested new total with the starting total
*/
bool CuiTransfer::checkTotal(int from, int to) const
{
	if(calculateTotal(from, to) == m_startingTotal)
	{
		return true;
	}
	else
	{
		DEBUG_FATAL(true, ("Totals don't match"));
		return false; //lint !e527 unreachable.  It is reachable in release build.
	}
}

//----------------------------------------------------------------------

/** Display the given number in the current from field.  Note that this number must have the extra modulus points added back in before display.
*/
void CuiTransfer::displayFrom(int from)
{	
	Unicode::String formattedString;

	UIUtils::FormatInteger(formattedString,from + m_modulusFrom );

	m_currentFromText->SetText(formattedString);
}

//----------------------------------------------------------------------

/** Display the given number in the current to field.  Note that this number must have the extra modulus points added back in before display.
*/
void CuiTransfer::displayTo(int to)
{
	Unicode::String formattedString;

	UIUtils::FormatInteger(formattedString, to + m_modulusTo);

	m_currentToText->SetText(formattedString);
}

//----------------------------------------------------------------------

void CuiTransfer::handleMediatorPropertiesChanged ()
{
	//grab any server-set conversion rate, sanity check and set to 1:1 if necessary
	bool result = m_transaction->GetPropertyInteger (DataProperties::ConversionRatioFrom, m_conversionRatioFrom);
	if(!result || m_conversionRatioFrom <= 0)
		m_conversionRatioFrom = cs_defaultConversionRatioFrom;
	result = m_transaction->GetPropertyInteger (DataProperties::ConversionRatioTo, m_conversionRatioTo);
	if(!result || m_conversionRatioTo <= 0)
		m_conversionRatioTo = cs_defaultConversionRatioTo;

	//the widget has been preseeded with values from the SUI, store these interally
	m_startingFromText->GetPropertyInteger(UIText::PropertyName::LocalText, m_startingFrom);
	m_startingToText->GetPropertyInteger(UIText::PropertyName::LocalText, m_startingTo);
	m_currentFromText->GetPropertyInteger(UIText::PropertyName::LocalText, m_currentFrom);
	m_currentToText->GetPropertyInteger(UIText::PropertyName::LocalText, m_currentTo);

	//ensure no preseeded values are < 0
	m_currentFrom  = std::max(m_currentFrom, 0);
	m_currentTo    = std::max(m_currentTo, 0);
	m_startingFrom = std::max(m_startingFrom, 0);
	m_startingTo   = std::max(m_startingTo, 0);

	char buffer[256];
	//put current values back into text box (in case we had to clamp them)
	m_startingFromText->SetPropertyInteger(UIText::PropertyName::Text, m_startingFrom);
	
	_itoa(m_startingFrom, buffer, 10);
	m_startingFromText->SetText(UIUtils::FormatDelimitedInteger(Unicode::narrowToWide(buffer)));

	m_startingToText->SetPropertyInteger(UIText::PropertyName::Text, m_startingTo);

	_itoa(m_startingTo, buffer, 10);
	m_startingToText->SetText(UIUtils::FormatDelimitedInteger(Unicode::narrowToWide(buffer)));

	m_currentFromText->SetPropertyInteger(UITextbox::PropertyName::Text, m_currentFrom);

	_itoa(m_currentFrom, buffer, 10);
	m_currentFromText->SetText(Unicode::narrowToWide(buffer));

	m_currentToText->SetPropertyInteger(UITextbox::PropertyName::Text, m_currentTo);

	_itoa(m_currentTo, buffer, 10);
	m_currentToText->SetText(Unicode::narrowToWide(buffer));

	//given the conversion ratio, find out what modulus values we have (points we can't move around), and remove them from our math.
		//Tack them back on before display.
	m_modulusFrom = m_startingFrom % m_conversionRatioFrom;
	m_modulusTo = m_startingTo   % m_conversionRatioTo;
	m_startingFrom -= m_modulusFrom;
	m_startingTo -= m_modulusTo;
	m_currentFrom -= m_modulusFrom;
	m_currentTo -= m_modulusTo;

	//determine the number of ticks for the slider
	const int fromTicks = m_startingFrom / m_conversionRatioFrom;
	const int toTicks = m_startingTo / m_conversionRatioTo;
	const int totalTicks = fromTicks + toTicks;
	m_slider->SetUpperLimit(totalTicks);

	//if we don't have a 1:n or n:1 ratio, typing in the "current" boxes is unintuitive, so disable it
	if(m_conversionRatioFrom != 1 && m_conversionRatioTo != 1)
	{
		m_currentFromText->SetEditable(false);
		m_currentToText->SetEditable(false);
	}

	//determine the initial starting total (under the common denominator).  We will routinely recheck that the new total matches this number
	m_startingTotal = calculateTotal(m_startingFrom, m_startingTo);

	// allow 9 values in the entry field if the total is > 99,999,999
	if (m_startingTotal > 99999999)
	{
		m_currentFromText->SetMaxIntegerLength(9);
		m_currentToText->SetMaxIntegerLength(9);
	}

	//the text fields were initialized from script, make the slider match them
	matchSliderToValues();
}

//----------------------------------------------------------------------

void CuiTransfer::performActivate ()
{
	handleMediatorPropertiesChanged();
	setCurrentFromAndTo(m_currentFrom, m_currentTo);
}

//======================================================================
