//======================================================================
//
// SwgCuiProfessionTemplateSelect.cpp
// copyrite (c) 2005 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiProfessionTemplateSelect.h"

#include "clientGame/ClientExpertiseManager.h"
#include "clientGame/Game.h"
#include "clientGame/PlayerCreatureController.h"
#include "clientGame/PlayerObject.h"
#include "clientGame/RoadmapManager.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiSkillManager.h"
#include "clientUserInterface/CuiWorkspace.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/MessageQueueSelectCurrentWorkingSkill.h"
#include "sharedNetworkMessages/MessageQueueSelectProfessionTemplate.h"
#include "sharedObject/Controller.h"
#include "swgClientUserInterface/SwgCuiExpertise.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"

#include "UIButton.h"
#include "UIData.h"
#include "UIEffector.h"
#include "UIImage.h"
#include "UIManager.h"
#include "UIText.h"
#include "UIUtils.h"
#include "UIVolumePage.h"

//======================================================================

namespace SwgCuiProfessionTemplateSelectNamespace
{
	typedef std::vector<UISmartPointer<UIPage> > ProfessionButtonPages;

	// 
	std::string const s_swgCuiProfessionTemplateSelectPage("/skill.professionTemplate");

	std::string const s_swgCuiProfessionTemplateSelectMediator("SwgCuiProfessionTemplateSelect");

	std::string const s_noProfession("");

	std::string const s_sampleProfessionPage("sampleProfessionButton");
	std::string const s_professionDescriptionPage("professionDescription");
	std::string const s_professionSpecialtiesPage("professionSpecialties");
	std::string const s_professionSelect("professionSelect");
	std::string const s_volumePage("volumePage");
	std::string const s_professionCancel("buttonCancel");
	std::string const s_preview("buttonPreview");
	std::string const s_mmc("buttonClose");
	std::string const s_popup("popup");

	// the profession string file.
	std::string const s_professionTemplateStringFile("@ui_prof:");
	std::string const s_stfDescription("_desc");
	std::string const s_stfSpecialties("_spec");
	std::string const s_stfButton("_bdesc");
	std::string const s_stfButtonTooltip("_btip");
	std::string const s_stfButtonCancelDisabledTooltip(s_professionTemplateStringFile + "select_cancel_disabled");
	std::string const s_stfButtonCancelEnabledTooltip(s_professionTemplateStringFile + "select_cancel");

	// button tags.
	UILowerString const s_defaultButtonPageSelection("defaultSelection");
	UILowerString const s_buttonIdProperty("ButtonIdProperty");
	std::string const s_buttonImage("image");
	std::string const s_buttonImageSelect("image_select");
	std::string const s_buttonImageSource("ui_profession_");
	std::string const s_buttonImageSourcePostfix("_a1");
	std::string const s_buttonSelected("_select");
	std::string const s_buttonText("text");
	std::string const s_buttonButton("button");
	std::string const s_buttonTitle("title");
	std::string const s_buttonTitle2("title2");

	// poup menu stuff
	UILowerString const s_templateProperty("templateName");
	std::string const s_traderButtonId("trader");

	struct EnableEffects
	{
		EnableEffects()
		{
			CuiSkillManager::enableEffects(true);
		}

		~EnableEffects()
		{
			CuiSkillManager::enableEffects(false);
		}
	};

	// hack - the source rect is the wrong size & will not get processed until after our demo.  
	// when the new art is made, we will use it.

	void allowMovement(bool const allow);

	void cancelProfessionTemplateSelect();
}

//----------------------------------------------------------------------

void SwgCuiProfessionTemplateSelectNamespace::allowMovement(bool const allow)
{
	Object * const player = Game::getPlayer();
	if (player != 0)
	{
		PlayerCreatureController * const controller = dynamic_cast<PlayerCreatureController *>(player->getController());

		if (controller != 0)
		{
			controller->allowMovement(allow);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiProfessionTemplateSelectNamespace::cancelProfessionTemplateSelect()
{
	// tell the server to clear any data it may have saved
	Object * const player = Game::getPlayer();
	Controller * const controller = player ? player->getController() : NULL;
	if (controller)				
	{
		MessageQueueSelectProfessionTemplate * const msgSetProfessionTemplate = new MessageQueueSelectProfessionTemplate("");
		controller->appendMessage(	CM_setProfessionTemplate, 
			0.0f, 
			msgSetProfessionTemplate, 
			GameControllerMessageFlags::SEND |
			GameControllerMessageFlags::RELIABLE |
			GameControllerMessageFlags::DEST_AUTH_SERVER |
			GameControllerMessageFlags::DEST_AUTH_CLIENT);
	}
}

//----------------------------------------------------------------------

using namespace SwgCuiProfessionTemplateSelectNamespace;

//----------------------------------------------------------------------

class SwgCuiProfessionTemplateSelect::Implementation
{
public:
	MessageDispatch::Callback * m_callback;
	
	ProfessionButtonPages m_professionPages;
	
	UIPage * m_sampleProfessionPage;
	UIButton * m_selectedButton;
	UIButton * m_selectProfessionButton;
	UIButton * m_cancelButton;
	UIButton * m_previewButton;
	UIButton * m_mmc;
	UIText * m_professionDescriptionText;
	UIText * m_professionSpecialtiesText;
	UIVolumePage * m_volumePage;
	UIPage *m_popupMenu;
	
	Unicode::String m_defaultProfessionSelection;
	Unicode::String m_selectedProfessionTemplate;
	
	Implementation() :
	m_callback(new MessageDispatch::Callback),
	m_professionPages(),
	m_sampleProfessionPage(0),
	m_selectedButton(0),
	m_selectProfessionButton(0),
	m_cancelButton(0),
	m_previewButton(0),
	m_mmc(0),
	m_professionDescriptionText(0),
	m_professionSpecialtiesText(0),
	m_volumePage(0),
	m_defaultProfessionSelection(),
	m_selectedProfessionTemplate(),
	m_popupMenu(0)
	{
	}
	
	~Implementation()
	{
		delete m_callback;
		m_callback = NULL;

		m_sampleProfessionPage = NULL;
		m_selectedButton = NULL;
		m_selectProfessionButton = NULL;
		m_cancelButton = NULL;
		m_previewButton = NULL;
		m_mmc = NULL;
		m_professionDescriptionText = NULL;
		m_professionSpecialtiesText = NULL;
		m_volumePage = NULL;
	}
};

//----------------------------------------------------------------------

SwgCuiProfessionTemplateSelect::SwgCuiProfessionTemplateSelect(UIPage & page) :
CuiMediator(s_swgCuiProfessionTemplateSelectMediator.c_str(), page),
UIEventCallback(),
m_pimpl(new Implementation)
{
	// get the sample pages.
	{
		getCodeDataObject(TUIPage, m_pimpl->m_sampleProfessionPage, s_sampleProfessionPage.c_str());
		m_pimpl->m_sampleProfessionPage->SetVisible(false);
		m_pimpl->m_sampleProfessionPage->SetActivated(false);
		m_pimpl->m_sampleProfessionPage->SetEnabled(false);
	}

	// get the parent volume page.
	{
		getCodeDataObject(TUIVolumePage, m_pimpl->m_volumePage, s_volumePage.c_str());
		m_pimpl->m_volumePage->Clear();
		m_pimpl->m_volumePage->SetVisible(true);
	}

	getCodeDataObject(TUIButton, m_pimpl->m_selectProfessionButton, s_professionSelect.c_str());
	registerMediatorObject(*m_pimpl->m_selectProfessionButton, true);

	getCodeDataObject(TUIButton, m_pimpl->m_cancelButton, s_professionCancel.c_str());
	registerMediatorObject(*m_pimpl->m_cancelButton, true);

	getCodeDataObject(TUIButton, m_pimpl->m_previewButton, s_preview.c_str());
	registerMediatorObject(*m_pimpl->m_previewButton, true);

	getCodeDataObject(TUIButton, m_pimpl->m_mmc, s_mmc.c_str());
	registerMediatorObject(*m_pimpl->m_mmc, true);

	
	getCodeDataObject(TUIText, m_pimpl->m_professionDescriptionText, s_professionDescriptionPage.c_str());
	getCodeDataObject(TUIText, m_pimpl->m_professionSpecialtiesText, s_professionSpecialtiesPage.c_str());

	m_pimpl->m_defaultProfessionSelection = Unicode::narrowToWide(RoadmapManager::getDefaultStartingProfession());
	
	getCodeDataObject(TUIPage, m_pimpl->m_popupMenu, s_popup.c_str());

	UIBaseObject::UIObjectList children;
	m_pimpl->m_popupMenu->GetChildren(children);
	UIBaseObject::UIObjectList::iterator i;
	for(i = children.begin(); i != children.end(); ++i)
	{
		UIBaseObject *topPtr = *i;
		UIBaseObject::UIObjectList subChildren;
		topPtr->GetChildren(subChildren);
		UIBaseObject::UIObjectList::iterator subI;
		for(subI = subChildren.begin(); subI != subChildren.end(); ++subI)
		{
			UIBaseObject *curPtr = *subI;
			if(curPtr->IsA(TUIButton))
			{
				UIButton *curButton = safe_cast<UIButton*>(curPtr);
				registerMediatorObject(*curButton, true);
			}
		}
	}
	m_pimpl->m_popupMenu->SetVisible(false);


	setState (MS_closeDeactivates);

	setSettingsAutoSizeLocation(true, true);

	page.SetLocation(0,0);

	allowCancel(false);

	setStickyVisible(true);
}

//----------------------------------------------------------------------

SwgCuiProfessionTemplateSelect::~SwgCuiProfessionTemplateSelect()
{
	delete m_pimpl;
	m_pimpl = NULL;
}

//----------------------------------------------------------------------

SwgCuiProfessionTemplateSelect * SwgCuiProfessionTemplateSelect::createInto(UIPage & parent)
{
	UIPage * const dupe = NON_NULL(UIPage::DuplicateInto(parent, s_swgCuiProfessionTemplateSelectPage.c_str()));
	return new SwgCuiProfessionTemplateSelect(*dupe);
}

//----------------------------------------------------------------------

void SwgCuiProfessionTemplateSelect::performActivate()
{
	CuiManager::requestPointer(true);
	m_pimpl->m_volumePage->AddCallback(this);
	m_pimpl->m_previewButton->SetVisible(false);
	buildProfessionButtons();

	if(RoadmapManager::playerIsOnRoadmap() && !RoadmapManager::playerIsNewCharacter() && !CuiSkillManager::getWorkingSkill().empty())
		allowCancel(true);
	else
		allowCancel(false);

	allowMovement(false);
}

//----------------------------------------------------------------------

void SwgCuiProfessionTemplateSelect::performDeactivate()
{
	CuiManager::requestPointer(false);
	m_pimpl->m_volumePage->RemoveCallback(this);
	destroyProfessionButtons();

	allowMovement(true);
}

//----------------------------------------------------------------------

void SwgCuiProfessionTemplateSelect::OnButtonPressed(UIWidget * const context)
{
	if (context) 
	{
		UIButton * const button = UI_ASOBJECT(UIButton, context);
		if (button) 
		{
			if (m_pimpl->m_selectProfessionButton == button) 
			{
				if(!m_pimpl->m_popupMenu->IsVisible())
				{				
					UIString buttonId;
					if (m_pimpl->m_selectedButton != 0)
					{
						setProfessionTemplate(m_pimpl->m_selectedProfessionTemplate);
						closeNextFrame();
					}
				}
			}
			else if (m_pimpl->m_cancelButton == button) 
			{
				cancelProfessionTemplateSelect();
				closeNextFrame();
			}
			else if (m_pimpl->m_previewButton == button) 
			{
				if (m_pimpl->m_selectedButton != NULL)
				{
					UIString buttonId;
					if (m_pimpl->m_selectedButton->GetProperty(s_buttonIdProperty, buttonId))
					{
						Game::playProfessionMovie( Unicode::wideToNarrow(buttonId).c_str() );		
					}
				}
			}
			else
			{
				UIString buttonId;
				if(button->HasProperty(s_templateProperty))
				{
					// This is a trader popup button
					m_pimpl->m_popupMenu->SetVisible(false);
					
					UIString value;
					button->GetProperty(s_templateProperty, value);

					UIString currentSpecialtyText;
					m_pimpl->m_professionSpecialtiesText->GetLocalText(currentSpecialtyText);

					char tmp[512];
					snprintf(tmp, sizeof(tmp)-1, "trader_specialty_%s", Unicode::wideToNarrow(value).c_str());
					StringId s("ui_roadmap", tmp);
					currentSpecialtyText.append(s.localize());
					m_pimpl->m_professionSpecialtiesText->SetLocalText(currentSpecialtyText);
					
					m_pimpl->m_selectedProfessionTemplate = value;
				}
				else if (button->GetProperty(s_buttonIdProperty, buttonId))
				{
					m_pimpl->m_selectedButton = button;
					
					// Set description page text.
					m_pimpl->m_professionDescriptionText->SetText(Unicode::narrowToWide(s_professionTemplateStringFile) + buttonId + Unicode::narrowToWide(s_stfDescription));
					
					// Set specialties page text.
					m_pimpl->m_professionSpecialtiesText->SetText(Unicode::narrowToWide(s_professionTemplateStringFile) + buttonId + Unicode::narrowToWide(s_stfSpecialties));

					// Show the preview button
					m_pimpl->m_previewButton->SetVisible(true);

					// Open popup menu if this is trader
					std::string value = Unicode::wideToNarrow(buttonId);
					if(value == s_traderButtonId)
					{
						m_pimpl->m_popupMenu->SetVisible(true);
						UIPoint mouseCoord = UIManager::gUIManager().GetLastMouseCoord();
						if(mouseCoord.y > (m_pimpl->m_volumePage->GetSize().y - 120))
							mouseCoord.y = m_pimpl->m_volumePage->GetSize().y - 120;
						m_pimpl->m_popupMenu->SetLocation(mouseCoord);
					}
					else
					{
						m_pimpl->m_popupMenu->SetVisible(false);
					}

					m_pimpl->m_selectedProfessionTemplate = buttonId;
				}
			}
		}
	}

	setButtonSelected();
}

//----------------------------------------------------------------------

void SwgCuiProfessionTemplateSelect::update(float const deltaTimeSecs)
{
	CuiMediator::update(deltaTimeSecs);

	m_pimpl->m_volumePage->Link();
	m_pimpl->m_volumePage->SetVisible(true);
	m_pimpl->m_volumePage->SetActivated(true);
	m_pimpl->m_volumePage->SetEnabled(true);
	m_pimpl->m_volumePage->SetPackDirty(true);
	m_pimpl->m_volumePage->ForcePackChildren();
}

//----------------------------------------------------------------------

void SwgCuiProfessionTemplateSelect::buildProfessionButtons()
{
	std::vector<std::string> startingTemplates;
	RoadmapManager::getStartingProfessions(startingTemplates, true);

	UIButton * lastResortButton = NULL;
	bool defaultSet = false;

	volumePageFixup();

	for (std::vector<std::string>::const_iterator itStartingTemplate = startingTemplates.begin(); itStartingTemplate != startingTemplates.end(); ++itStartingTemplate) 
	{
		std::string const & templateBaseName = *itStartingTemplate;
		if (!templateBaseName.empty()) 
		{
			UISmartPointer<UIPage> newProfessionButtonPage(UI_ASOBJECT(UIPage, m_pimpl->m_sampleProfessionPage->DuplicateObject()));
			m_pimpl->m_professionPages.push_back(newProfessionButtonPage);

			// set the location.
			newProfessionButtonPage->SetLocation(UIPoint::zero);
			newProfessionButtonPage->SetVisible(true);
			newProfessionButtonPage->SetActivated(true);
			newProfessionButtonPage->SetEnabled(true);
			
			// add the page to the volume.
			m_pimpl->m_volumePage->AddChild(newProfessionButtonPage);

			// link it up.
			newProfessionButtonPage->Link();
			
			// register mediator on the button.
			UIButton * button = NULL;
			IGNORE_RETURN(newProfessionButtonPage->GetCodeDataObject(TUIButton, button, s_buttonButton.c_str(), false));
			
			UIText * buttonTitle = NULL;
			IGNORE_RETURN(newProfessionButtonPage->GetCodeDataObject(TUIText, buttonTitle, s_buttonTitle.c_str(), false));
			StringId sid("ui_roadmap", templateBaseName.c_str());
			buttonTitle->SetText(Unicode::emptyString);
			buttonTitle->SetPreLocalized(true);
			buttonTitle->SetLocalText(sid.localize());	
			
			UIText * buttonTitle2 = NULL;
			IGNORE_RETURN(newProfessionButtonPage->GetCodeDataObject(TUIText, buttonTitle2, s_buttonTitle2.c_str(), false));
			buttonTitle2->SetText(Unicode::emptyString);
			buttonTitle2->SetPreLocalized(true);
			buttonTitle2->SetLocalText(sid.localize());

			// set the tooltip on the button.
			NON_NULL(button)->SetTooltipDelay(true);
			NON_NULL(button)->SetTooltip(Unicode::narrowToWide(s_professionTemplateStringFile + templateBaseName + s_stfButtonTooltip));
			
			// set an identifier on the button.
			NON_NULL(button)->SetProperty(s_buttonIdProperty, Unicode::narrowToWide(templateBaseName));
			
			// finally, show this thing.
			NON_NULL(button)->SetSelected(false);
			
			registerMediatorObject(*NON_NULL(button), true);
			
			// keep track of the first button in case we need to select it.
			if (!lastResortButton || m_pimpl->m_defaultProfessionSelection == Unicode::narrowToWide(templateBaseName))
			{
				lastResortButton = button;
			}

			
			// select the default button.
			if (!defaultSet)
			{
				std::string professionTemplate;
				std::string startingSkill;

				RoadmapManager::getStartingProfessionTemplateAndSkill(templateBaseName, professionTemplate, startingSkill);

				if (CuiSkillManager::getSkillTemplate() == professionTemplate)
				{
					button->SetSelected(true);
					OnButtonPressed(button);
					defaultSet = true;
				}
			}
		}
	}

	// ensure we have a button selected.
	if (!defaultSet && lastResortButton) 
	{
		lastResortButton->SetSelected(true);
		OnButtonPressed(lastResortButton);
	}

	volumePageFixup();

	// link it up.
	m_pimpl->m_volumePage->Link();

	setButtonSelected();
}

//----------------------------------------------------------------------

void SwgCuiProfessionTemplateSelect::destroyProfessionButtons()
{
	m_pimpl->m_professionPages.clear();
	m_pimpl->m_volumePage->Clear();
	unregisterMediatorObjects();
}

//----------------------------------------------------------------------

void SwgCuiProfessionTemplateSelect::setProfessionTemplate(UIString const & templateName)
{
	if (!templateName.empty()) 
	{
		Object * const player = Game::getPlayer();
		Controller * const controller = player ? player->getController() : NULL;
		if (controller)				
		{
			// force close the expertise window
			CuiWorkspace * const workspace = CuiWorkspace::getGameWorkspace();
			if (workspace)
			{
				SwgCuiExpertise * const expertiseWindow = safe_cast<SwgCuiExpertise *>(workspace->findMediatorByType(typeid(SwgCuiExpertise)));
				if (expertiseWindow)
					expertiseWindow->closeThroughWorkspace();
			}

			// wipe out any existing allocated but not applied expertise points
			ClientExpertiseManager::clearAllocatedExpertises();

			std::string professionTemplate = Unicode::wideToNarrow(templateName);
			std::string startingSkill;

			RoadmapManager::getStartingProfessionSkill(professionTemplate, startingSkill);

			if (startingSkill.empty())
			{
				RoadmapManager::getStartingProfessionTemplateAndSkill(Unicode::wideToNarrow(templateName), professionTemplate, startingSkill);
			}

			if (!startingSkill.empty())
			{
				EnableEffects effects;

				std::string oldProfessionTemplate = CuiSkillManager::getSkillTemplate();
				std::string oldWorkingSkill = CuiSkillManager::getWorkingSkill();
				CuiSkillManager::setSkillTemplate(professionTemplate);
				CuiSkillManager::setWorkingSkill(startingSkill);

				if (oldProfessionTemplate == CuiSkillManager::getSkillTemplate())
					cancelProfessionTemplateSelect();

				//This is here in case the set on the server doesn't work; we rely on the server to get back to us with confirmation
				CuiSkillManager::setSkillTemplate(oldProfessionTemplate, false);
				CuiSkillManager::setWorkingSkill(oldWorkingSkill, false);
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiProfessionTemplateSelect::setButtonSelected()
{
	for (ProfessionButtonPages::iterator it = m_pimpl->m_professionPages.begin(); it != m_pimpl->m_professionPages.end(); ++it)
	{
		UISmartPointer<UIPage> const page(*it);
		if (page) 
		{
			// Set the button state.
			UIButton * button = NULL;
			IGNORE_RETURN(page->GetCodeDataObject(TUIButton, button, s_buttonButton.c_str(), false));
			if (button) 
			{
				bool const isSelected = (button == m_pimpl->m_selectedButton);
				
				button->SetSelected(isSelected);

				UIString templateName;
				if (button->GetProperty(s_buttonIdProperty, templateName)) 
				{
					// Set the image state.
					UIImage * image = NULL;
					page->GetCodeDataObject(TUIImage, image, s_buttonImage.c_str(), false);

					if (image)
					{
						std::string imageSource(s_buttonImageSource + Unicode::wideToNarrow(templateName));
						
						if (isSelected) 
						{
							imageSource += s_buttonSelected;
						}
						imageSource += s_buttonImageSourcePostfix;
						
						IGNORE_RETURN(NON_NULL(image)->SetSourceResource(Unicode::narrowToWide(imageSource)));
					}
				}
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiProfessionTemplateSelect::allowCancel(bool allow)
{
	m_pimpl->m_cancelButton->SetVisible(allow);
	m_pimpl->m_mmc->SetVisible(allow);

	// rls - not used right now.
	// m_pimpl->m_cancelButton->SetTooltip(Unicode::narrowToWide(allow ? s_stfButtonCancelEnabledTooltip : s_stfButtonCancelDisabledTooltip));
}

//----------------------------------------------------------------------

void SwgCuiProfessionTemplateSelect::volumePageFixup()
{
	// Hack to fill volume page correctly.
	UISize const cellCount = m_pimpl->m_volumePage->GetCellCountFixed();
	m_pimpl->m_volumePage->SetCellCountFixed(UISize(cellCount.x+1, cellCount.y+1));
	m_pimpl->m_volumePage->SetCellCountFixed(cellCount);
}

//======================================================================
