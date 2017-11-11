//======================================================================
//
// CuiKeypad.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiKeypad_H
#define INCLUDED_CuiKeypad_H

//======================================================================

#include "UIEventCallback.h"
#include "clientUserInterface/CuiMediator.h"

class UIButton;
class UIPage;
class UITextbox;

// ======================================================================

/**
* CuiKeypad
*/

class CuiKeypad : 
public CuiMediator, 
public UIEventCallback
{
public:
	explicit                      CuiKeypad (UIPage & page);
	virtual void                  OnButtonPressed(UIWidget * context);
                                ~CuiKeypad();

private:
	//disabled
	                              CuiKeypad ();
	                              CuiKeypad (const CuiKeypad & rhs);
	CuiKeypad &                 operator=   (const CuiKeypad & rhs);

private:
	UIButton *                    m_button1;
	UIButton *                    m_button2;
	UIButton *                    m_button3;
	UIButton *                    m_button4;
	UIButton *                    m_button5;
	UIButton *                    m_button6;
	UIButton *                    m_button7;
	UIButton *                    m_button8;
	UIButton *                    m_button9;
	UIButton *                    m_button0;
	UIButton *                    m_buttonKeycard;
	UIButton *                    m_buttonSlice;
	UIButton *                    m_buttonEnter;
	UITextbox *                   m_resultText;
	bool                          m_buttonSet;
};

//======================================================================

#endif

