//======================================================================
//
// SwgCuiNews.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiNews.h"

#include "UIPage.h"
#include "UIData.h"
#include "UIButton.h"
#include "UIText.h"
#include "UIImage.h"

//======================================================================


SwgCuiNews::SwgCuiNews (UIPage & page) :
CuiMediator       ("SwgCuiNews", page),
UIEventCallback   (),
m_buttonBack      (0),
m_buttonNext      (0),
m_buttonIntro     (0),
m_text            (0),
m_image           (0)

{
	getCodeDataObject (TUIButton,    m_buttonBack,    "buttonBack");
	getCodeDataObject (TUIButton,    m_buttonNext,    "buttonNext");
	getCodeDataObject (TUIButton,    m_buttonIntro,   "buttonIntro");

	getCodeDataObject (TUIText,      m_text,          "text");
	getCodeDataObject (TUIImage,     m_image,         "image");
}

//----------------------------------------------------------------------

SwgCuiNews::~SwgCuiNews ()
{
	m_buttonBack    = 0;
	m_buttonNext    = 0;
	m_buttonIntro   = 0;

	m_text          = 0;
	m_image         = 0;

}

//----------------------------------------------------------------------

void SwgCuiNews::performActivate   ()
{
	m_buttonBack->AddCallback (this);
	m_buttonNext->AddCallback (this);
	m_buttonIntro->AddCallback (this);
}

//----------------------------------------------------------------------

void SwgCuiNews::performDeactivate ()
{
	m_buttonBack->RemoveCallback (this);
	m_buttonNext->RemoveCallback (this);
	m_buttonIntro->RemoveCallback (this);
}

//----------------------------------------------------------------------

void SwgCuiNews::OnButtonPressed   (UIWidget *context)
{
	if (context == m_buttonBack)
	{
	}
	else if (context == m_buttonNext)
	{
	}
	else if (context == m_buttonIntro)
	{
	}
}
//======================================================================
