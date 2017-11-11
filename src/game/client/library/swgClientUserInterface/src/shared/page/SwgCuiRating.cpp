//======================================================================
//
// SwgCuiRating.cpp
// copyright(c) 2009 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiRating.h"

#include "clientGame/CreatureController.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"

#include "sharedFoundation/GameControllerMessage.h"

#include "sharedNetworkMessages/MessageQueueGenericValueType.h"

#include "sharedObject/Controller.h"


#include "StringId.h"
#include "UIBaseObject.h"
#include "UIButton.h"
#include "UICheckbox.h"
#include "UIComposite.h"
#include "UILowerString.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UIText.h"
#include "UIWidget.h"

#include <vector>
// ---------------------------------------------------------------------

namespace SwgCuiRatingNamespace
{
	std::vector<UICheckbox* > ms_ratingBoxes;
}

using namespace SwgCuiRatingNamespace;

SwgCuiRating::SwgCuiRating (UIPage & page):
CuiMediator("Rating", page),
m_title(NULL),
m_description(NULL),
m_okButton(NULL),
m_cancelButton(NULL)
{
	getCodeDataObject(TUIText, m_title, "title");
	getCodeDataObject(TUIText, m_description, "Text");

	getCodeDataObject(TUIButton, m_okButton, "buttonOkay");
	getCodeDataObject(TUIButton, m_cancelButton, "buttonCancel");
	getCodeDataObject(TUIButton, m_close, "buttonclose");

	m_title->SetPreLocalized(true);
	m_description->SetPreLocalized(true);

	registerMediatorObject(*m_okButton, true);
	registerMediatorObject(*m_cancelButton, true);
	registerMediatorObject(*m_close, true);

	UIComposite * ratingComp;
	getCodeDataObject(TUIComposite, ratingComp, "ratingComp");
	
	UIBaseObject::UIObjectList childrenList;
	ratingComp->GetChildren(childrenList);

	UIBaseObject::UIObjectList::iterator iter = childrenList.begin();
	for(; iter != childrenList.end(); ++iter)
	{
		if((*iter)->IsA(TUICheckbox))
		{
			ms_ratingBoxes.push_back(static_cast<UICheckbox *>((*iter)));
			registerMediatorObject(*(*iter), true);
		}
	}

	for(unsigned int i = 0; i < ms_ratingBoxes.size(); ++i)
	{
		ms_ratingBoxes[i]->SetChecked(false, false);
		ms_ratingBoxes[i]->SetRadio(false);
	}


}

SwgCuiRating::~SwgCuiRating()
{
	ms_ratingBoxes.clear();
}

void SwgCuiRating::performActivate()
{
	for(unsigned int i = 0; i < ms_ratingBoxes.size(); ++i)
		ms_ratingBoxes[i]->SetChecked(false, false);	
}

void SwgCuiRating::performDeactivate()
{

}

void SwgCuiRating::OnButtonPressed(UIWidget * context)
{
	if(context == m_okButton)
	{
		unsigned int i = 0;
		for(; i < ms_ratingBoxes.size(); ++i)
			if(!ms_ratingBoxes[i]->IsChecked())
				break;
			
		sendRating(static_cast<int>(i));
		closeNextFrame();
	}
	else if (context == m_cancelButton || context == m_close)
	{
		sendRating(-1);
		closeNextFrame();
	}
}

void SwgCuiRating::OnCheckboxSet(UIWidget *context)
{
	for(unsigned int i = 0; i < ms_ratingBoxes.size(); ++i)
		if(ms_ratingBoxes[i] == context)
			setupRatingBoxes(i);
}

void SwgCuiRating::OnCheckboxUnset(UIWidget *context)
{
	for(unsigned int i = 0; i < ms_ratingBoxes.size(); ++i)
		if(ms_ratingBoxes[i] == context)
			setupRatingBoxes(i);
}

void SwgCuiRating::setWindowTitle(std::string title)
{
	m_title->SetLocalText(Unicode::narrowToWide(title));
}

void SwgCuiRating::setWindowDescription(std::string desc)
{
	m_description->SetLocalText(Unicode::narrowToWide(desc));
}

void SwgCuiRating::sendRating(int value)
{
	Game::getPlayer()->getController()->appendMessage (
		CM_ratingFinished, 
		0.0f, 
		new MessageQueueGenericValueType<int>(value), 
		GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_SERVER);
}

void SwgCuiRating::setupRatingBoxes(int checkedIndex)
{
	ms_ratingBoxes[checkedIndex]->SetChecked(true, false);

	// Check up to our index.
	for(unsigned int j = 0; j < static_cast<unsigned int>(checkedIndex); ++j)
		ms_ratingBoxes[j]->SetChecked(true, false);
	
	// Uncheck every other box.
	for(unsigned int h = checkedIndex + 1; h < ms_ratingBoxes.size(); ++h)
		ms_ratingBoxes[h]->SetChecked(false, false);
}