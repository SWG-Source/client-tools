//======================================================================
//
// CuiFontSizer.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiFontSizer.h"

#include "clientUserInterface/CuiChatManager.h"
#include "clientUserInterface/CuiWorkspace.h"

#include "UIButton.h"
#include "UIData.h"
#include "UIPage.h"
#include "UISliderbar.h"
#include "UIText.h"
#include "UITextStyle.h"
#include "UITextStyleManager.h"

#include <algorithm>
#include <vector>

//======================================================================

const UILowerString CuiFontSizer::Properties::FontSize  = UILowerString ("FontSize");
const UILowerString CuiFontSizer::Properties::FontIndex = UILowerString ("FontIndex");

//----------------------------------------------------------------------


CuiFontSizer::CuiFontSizer      (UIPage & page) :
CuiMediator          ("CuiFontSizer", page),
UIEventCallback      (),
m_buttonOk           (0),
m_buttonCancel       (0),
m_slider             (0),
m_textSample         (0),
m_targetText         (0),
m_sizeVector         (new SizeVector),
m_fontPrefix         (),
m_index              (0),
m_size               (0)
{
	getCodeDataObject (TUIButton,    m_buttonOk,        "buttonOk");
	getCodeDataObject (TUIButton,    m_buttonCancel,    "buttonCancel");
	getCodeDataObject (TUISliderbar, m_slider,          "slider");
	getCodeDataObject (TUIText,      m_textSample,      "textSample");

	setState (MS_closeable);
}

//----------------------------------------------------------------------

CuiFontSizer::~CuiFontSizer ()
{
	m_buttonOk           = 0;
	m_buttonCancel       = 0;
	m_slider             = 0;
	m_textSample         = 0;

	releaseTarget ();

	delete m_sizeVector;
	m_sizeVector         = 0;
}

//----------------------------------------------------------------------

void CuiFontSizer::performActivate   ()
{
	m_buttonOk->AddCallback      (this);
	m_buttonCancel->AddCallback  (this);
	m_slider->AddCallback        (this);

	updateSample                 ();
}

//----------------------------------------------------------------------

void CuiFontSizer::performDeactivate ()
{
	m_buttonOk->RemoveCallback      (this);
	m_buttonCancel->RemoveCallback  (this);
	m_slider->RemoveCallback        (this);
}

//----------------------------------------------------------------------

void CuiFontSizer::OnButtonPressed              (UIWidget * context)
{
	if (context == m_buttonOk)
	{
		ok ();
	}
	else if (context == m_buttonCancel)
	{
		
	}

	closeThroughWorkspace ();
}

//----------------------------------------------------------------------

void CuiFontSizer::OnSliderbarChanged           (UIWidget * context)
{
	if (context == m_slider)
	{
		updateSample ();
	}
}

//----------------------------------------------------------------------

void CuiFontSizer::releaseTarget ()
{
	if (m_targetText)
	{
		m_targetText->Detach (0);
		m_targetText = 0;
	}

	if (m_sizeVector)
		m_sizeVector->clear ();

	m_fontPrefix.clear ();
}

//----------------------------------------------------------------------

void CuiFontSizer::setTargetText                (UIText & text, const SizeVector & sv)
{
	text.Attach (0);

	releaseTarget ();

	m_targetText = &text;

	*m_sizeVector = sv;

	m_slider->SetLowerLimit (0);

	if (!m_sizeVector->empty ())
	{
		m_slider->SetUpperLimit (m_sizeVector->size () - 1);
		m_slider->SetEnabled (true);
	}
	else
		m_slider->SetEnabled (false);
	
	int size = 0;
	if (constructFontPrefix (text, *m_sizeVector, m_fontPrefix, size))
	{
		m_slider->SetValue (size, false);
		updateSample ();
	}
}

//----------------------------------------------------------------------

bool CuiFontSizer::constructFontPrefix (const UIText & text, const SizeVector & sv, std::string & prefix, int & size)
{
	const UITextStyle * const textStyle = text.GetTextStyle ();

	if (textStyle)
	{
		prefix = textStyle->GetFullPath ();

		//-- trim off the size part

		const size_t last_underscore = prefix.rfind ('_');

		if (last_underscore != std::string::npos)
		{
			const std::string numberpart = prefix.substr (last_underscore + 1);
			prefix = prefix.substr (0, last_underscore);

			const int tmpSize = atoi (numberpart.c_str ());

			const SizeVector::const_iterator it = std::lower_bound (sv.begin (), sv.end (), tmpSize);

			if (it != sv.end ())
			{
				size = std::distance (sv.begin (), it);
				return true;
			}
			else
				WARNING (true, ("Bad vector"));
		}
	}

	return false;
}

//----------------------------------------------------------------------

CuiFontSizer * CuiFontSizer::createInto (UIPage & parent)
{
	UIPage * const dupe = NON_NULL (UIPage::DuplicateInto (parent, "/pda.FontSizer"));	
	CuiFontSizer * const mediator = new CuiFontSizer (*dupe);
	return mediator;
}

//----------------------------------------------------------------------

void CuiFontSizer::updateSample ()
{
	m_index = m_slider->GetValue ();
	UITextStyle * const textStyle = getTextStyle (*m_textSample, *m_sizeVector, m_fontPrefix, m_index, m_size);
	m_textSample->SetStyle (textStyle);
}

//----------------------------------------------------------------------

void CuiFontSizer::ok ()
{
	UITextStyle * const textStyle = m_textSample->GetTextStyle ();
		
	if (m_targetText)
	{
		m_targetText->SetStyle (textStyle);
		m_targetText->SetPropertyInteger (Properties::FontSize,  m_size);
		m_targetText->SetPropertyInteger (Properties::FontIndex, m_index);
		
		CuiChatManager::setChatWindowFontSizeDefaultIndex(m_index);
	}
}

//----------------------------------------------------------------------

UITextStyle *  CuiFontSizer::getTextStyle (UIText & text, const SizeVector & sv, const std::string & fontPrefix, int index, int & size)
{
	std::string actualFontPrefix = fontPrefix;

	if (actualFontPrefix.empty ())
	{
		int dummySize = 0;
		if (!constructFontPrefix (text, sv, actualFontPrefix, dummySize))
		{
			WARNING (true, ("could not construct font prefix"));
			return 0;
		}
	}

	SizeVector::const_iterator it = sv.begin ();
	
	std::advance (it, index);
	
	if (it != sv.end ())
	{
		size = *it;
		
		char buf [64];
		_itoa (size, buf, 10);
		const std::string fontname = actualFontPrefix + '_' + buf;
		
		UITextStyle * const textStyle = UITextStyleManager::GetInstance()->GetFontForLogicalFont(fontname);
		return textStyle;
	}
	else
		WARNING (true, ("bad vector"));

	return 0;
}

//======================================================================
