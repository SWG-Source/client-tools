//======================================================================
//
// CuiKeypad.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiKeypad.h"

#include "UIButton.h"
#include "UIPage.h"
#include "UITextBox.h"
#include "UnicodeUtils.h"

//======================================================================

CuiKeypad::CuiKeypad (UIPage & page) :
CuiMediator            ("CuiKeypad", page),
UIEventCallback        (),
m_button1              (0),
m_button2              (0),
m_button3              (0),
m_button4              (0),
m_button5              (0),
m_button6              (0),
m_button7              (0),
m_button8              (0),
m_button9              (0),
m_button0              (0),
m_buttonKeycard        (0),
m_buttonSlice          (0),
m_buttonEnter          (0),
m_buttonSet            (false)
{
	IGNORE_RETURN(getCodeDataObject (TUIButton,     m_button1,       "numberOne"));
	IGNORE_RETURN(getCodeDataObject (TUIButton,     m_button2,       "numberTwo"));
	IGNORE_RETURN(getCodeDataObject (TUIButton,     m_button3,       "numberThree"));
	IGNORE_RETURN(getCodeDataObject (TUIButton,     m_button4,       "numberFour"));
	IGNORE_RETURN(getCodeDataObject (TUIButton,     m_button5,       "numberFive"));
	IGNORE_RETURN(getCodeDataObject (TUIButton,     m_button6,       "numberSix"));
	IGNORE_RETURN(getCodeDataObject (TUIButton,     m_button7,       "numberSeven"));
	IGNORE_RETURN(getCodeDataObject (TUIButton,     m_button8,       "numberEight"));
	IGNORE_RETURN(getCodeDataObject (TUIButton,     m_button9,       "numberNine"));
	IGNORE_RETURN(getCodeDataObject (TUIButton,     m_button0,       "numberZero"));
	IGNORE_RETURN(getCodeDataObject (TUIButton,     m_buttonKeycard, "buttonKeycard"));
	IGNORE_RETURN(getCodeDataObject (TUIButton,     m_buttonSlice,   "buttonSlice"));
	IGNORE_RETURN(getCodeDataObject (TUIButton,     m_buttonEnter,   "buttonEnter"));
	IGNORE_RETURN(getCodeDataObject (TUITextbox,    m_resultText,    "textboxNumbers"));

	m_resultText->SetLocalText(UIString());

	registerMediatorObject (*m_button1,       true);
	registerMediatorObject (*m_button2,       true);
	registerMediatorObject (*m_button3,       true);
	registerMediatorObject (*m_button4,       true);
	registerMediatorObject (*m_button5,       true);
	registerMediatorObject (*m_button6,       true);
	registerMediatorObject (*m_button7,       true);
	registerMediatorObject (*m_button8,       true);
	registerMediatorObject (*m_button9,       true);
	registerMediatorObject (*m_button0,       true);
	registerMediatorObject (*m_buttonKeycard, true);
	registerMediatorObject (*m_buttonSlice,   true);
	registerMediatorObject (*m_buttonEnter,   true);
	registerMediatorObject (*m_resultText,   true);
}

//----------------------------------------------------------------------

CuiKeypad::~CuiKeypad()
{
	m_button1 = 0;
	m_button2 = 0;
	m_button3 = 0;
	m_button4 = 0;
	m_button5 = 0;
	m_button6 = 0;
	m_button7 = 0;
	m_button8 = 0;
	m_button9 = 0;
	m_button0 = 0;
	m_buttonEnter   = 0;
	m_buttonSlice   = 0;
	m_buttonKeycard = 0;
}

//----------------------------------------------------------------------

void CuiKeypad::OnButtonPressed   (UIWidget *context)
{
	UIString currentText;
	m_resultText->GetLocalText(currentText);
	bool updateText = false;

	if (context == m_button1)
	{
		currentText += Unicode::narrowToWide("1");
		updateText = true;
	}
	else if (context == m_button2)
	{
		currentText += Unicode::narrowToWide("2");
		updateText = true;
	}
	else if (context == m_button3)
	{
		currentText += Unicode::narrowToWide("3");
		updateText = true;
	}
	else if (context == m_button4)
	{
		currentText += Unicode::narrowToWide("4");
		updateText = true;
	}
	else if (context == m_button5)
	{
		currentText += Unicode::narrowToWide("5");
		updateText = true;
	}
	else if (context == m_button6)
	{
		currentText += Unicode::narrowToWide("6");
		updateText = true;
	}
	else if (context == m_button7)
	{
		currentText += Unicode::narrowToWide("7");
		updateText = true;
	}
	else if (context == m_button8)
	{
		currentText += Unicode::narrowToWide("8");
		updateText = true;
	}
	else if (context == m_button9)
	{
		currentText += Unicode::narrowToWide("9");
		updateText = true;
	}
	else if (context == m_button0)
	{
		currentText += Unicode::narrowToWide("0");
		updateText = true;
	}
	else if (context == m_buttonKeycard)
	{
		if(!m_buttonSet)
		{
			m_buttonEnter->SetProperty(UILowerString ("ButtonPressed"), UIString(Unicode::narrowToWide("keycard")));
			m_buttonSet = true;
		}
		m_buttonEnter->Press();
	}
	else if (context == m_buttonSlice)
	{
		if(!m_buttonSet)
		{
			m_buttonEnter->SetProperty(UILowerString ("ButtonPressed"), UIString(Unicode::narrowToWide("slice")));
			m_buttonSet = true;
		}
		m_buttonEnter->Press();
	}
	else if (context == m_buttonEnter)
	{
		if(!m_buttonSet)
		{
			m_buttonEnter->SetProperty(UILowerString ("ButtonPressed"), UIString(Unicode::narrowToWide("enter")));
			m_buttonSet = true;
		}
	}

	if(updateText)
		m_resultText->SetLocalText(currentText);
}

//======================================================================
