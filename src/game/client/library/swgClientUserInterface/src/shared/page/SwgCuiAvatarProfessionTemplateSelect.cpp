//======================================================================
//
// SwgCuiAvatarProfessionTemplateSelect.cpp
// copyrite (c) 2005 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiAvatarProfessionTemplateSelect.h"

#include "clientGame/ConfigClientGame.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientGame/PlayerObject.h"
#include "clientGame/RoadmapManager.h"
#include "clientUserInterface/CuiLoginManager.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiSkillManager.h"
#include "clientUserInterface/CuiStringIdsServer.h"
#include "clientUserInterface/CuiTransition.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/MessageQueueSelectCurrentWorkingSkill.h"
#include "sharedNetworkMessages/MessageQueueSelectProfessionTemplate.h"
#include "sharedObject/Controller.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "swgClientUserInterface/SwgCuiAvatarCreationHelper.h"
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

namespace SwgCuiAvatarProfessionTemplateSelectNamespace
{
	typedef std::vector<UISmartPointer<UIPage> > ProfessionButtonPages;

	std::string const s_SwgCuiAvatarProfessionTemplateSelectPage("/AvProfessionTemplate");

	std::string const s_SwgCuiAvatarProfessionTemplateSelectMediator("SwgCuiAvatarProfessionTemplateSelect");

	std::string const s_noProfession("");

	std::string const s_sampleProfessionPage("sampleProfessionButton");
	std::string const s_professionDescriptionPage("professionDescription");
	std::string const s_professionSpecialtiesPage("professionSpecialties");
	std::string const s_professionSelect("professionSelect");
	std::string const s_volumePage("volumePage");
	std::string const s_professionCancel("buttonCancel");
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
}

using namespace SwgCuiAvatarProfessionTemplateSelectNamespace;

//----------------------------------------------------------------------

class SwgCuiAvatarProfessionTemplateSelect::Implementation
{
public:
	MessageDispatch::Callback * m_callback;
	
	ProfessionButtonPages m_professionPages;
	
	UIPage * m_sampleProfessionPage;
	UIButton * m_selectedButton;
	UIText * m_professionDescriptionText;
	UIText * m_professionSpecialtiesText;
	UIVolumePage * m_volumePage;
	UIPage *m_popupMenu;
	
	std::string m_defaultProfessionSelection;
	std::string m_selectedProfessionTemplate;
	
	Implementation() :
	m_callback(new MessageDispatch::Callback),
	m_professionPages(),
	m_sampleProfessionPage(),
	m_selectedButton(),
	m_professionDescriptionText(),
	m_professionSpecialtiesText(),
	m_volumePage(),
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
		m_professionDescriptionText = NULL;
		m_professionSpecialtiesText = NULL;
		m_volumePage = NULL;
	}
};

//----------------------------------------------------------------------

SwgCuiAvatarProfessionTemplateSelect::SwgCuiAvatarProfessionTemplateSelect(UIPage & page)
: CuiMediator(s_SwgCuiAvatarProfessionTemplateSelectMediator.c_str(), page)
, UIEventCallback()
, m_pimpl(new Implementation)
, m_buttonBack(0)
, m_buttonNext(0)
, m_buttonPreview(0)
, m_callback(new MessageDispatch::Callback)
, m_pendingCreationFinished(false)
, m_aborted(false)
, m_professionWasSelected(false)
{
	getCodeDataObject(TUIButton,  m_buttonBack, "buttonBack2");
	getCodeDataObject(TUIButton,  m_buttonNext, "buttonNext2");
	getCodeDataObject(TUIButton,  m_buttonPreview, "buttonPreview");

	registerMediatorObject(*m_buttonBack, true);
	registerMediatorObject(*m_buttonNext, true);
	registerMediatorObject(*m_buttonPreview, true);

	m_buttonPreview->SetVisible(false);
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

	getCodeDataObject(TUIText, m_pimpl->m_professionDescriptionText, s_professionDescriptionPage.c_str());
	getCodeDataObject(TUIText, m_pimpl->m_professionSpecialtiesText, s_professionSpecialtiesPage.c_str());

	//m_pimpl->m_defaultProfessionSelection = RoadmapManager::getDefaultStartingProfession();
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

	setState(MS_closeDeactivates);

	setSettingsAutoSizeLocation(true, true);

	page.SetLocation(0,0);
}

//----------------------------------------------------------------------

SwgCuiAvatarProfessionTemplateSelect::~SwgCuiAvatarProfessionTemplateSelect()
{
	delete m_callback;
	m_callback=0;

	m_buttonBack = 0;
	m_buttonNext = 0;
	m_buttonPreview = 0;

	delete m_pimpl;
	m_pimpl = NULL;
}

//----------------------------------------------------------------------

SwgCuiAvatarProfessionTemplateSelect * SwgCuiAvatarProfessionTemplateSelect::createInto(UIPage & parent)
{
	UIPage * const dupe = NON_NULL(UIPage::DuplicateInto(parent, s_SwgCuiAvatarProfessionTemplateSelectPage.c_str()));
	return new SwgCuiAvatarProfessionTemplateSelect(*dupe);
}

//----------------------------------------------------------------------

void SwgCuiAvatarProfessionTemplateSelect::performActivate()
{
	CuiMediator::performActivate();

	m_callback->connect(*this, &SwgCuiAvatarProfessionTemplateSelect::onCreationAborted, static_cast<SwgCuiAvatarCreationHelper::Messages::Aborted*>(0));
	m_callback->connect(*this, &SwgCuiAvatarProfessionTemplateSelect::onCreationFinished, static_cast<SwgCuiAvatarCreationHelper::Messages::CreationFinished *>(0));

	CuiManager::requestPointer(true);
	buildProfessionButtons();

	setIsUpdating(true);

	CuiTransition::signalTransitionReady(CuiMediatorTypes::AvatarProfessionTemplateSelect);

	m_buttonNext->SetSelected(true);

	m_professionWasSelected = false;
}

//----------------------------------------------------------------------

void SwgCuiAvatarProfessionTemplateSelect::performDeactivate()
{
	m_callback->disconnect(*this, &SwgCuiAvatarProfessionTemplateSelect::onCreationAborted, static_cast<SwgCuiAvatarCreationHelper::Messages::Aborted*>(0));
	m_callback->disconnect(*this, &SwgCuiAvatarProfessionTemplateSelect::onCreationFinished, static_cast<SwgCuiAvatarCreationHelper::Messages::CreationFinished *>(0));

	CuiManager::requestPointer(false);
	destroyProfessionButtons();

	m_pimpl->m_selectedButton = 0;
	m_pimpl->m_professionDescriptionText->Clear();
	m_pimpl->m_professionSpecialtiesText->Clear();
	m_pimpl->m_popupMenu->SetVisible(false);
	m_buttonPreview->SetVisible(false);

	setIsUpdating(false);
}

//----------------------------------------------------------------------

void SwgCuiAvatarProfessionTemplateSelect::OnButtonPressed(UIWidget * const context)
{

	if(Game::isPlayingCutScene())
	{
		if(context == m_buttonBack)
		{
			Game::skipCutScene();
		}
		return;
	}

	if (context == m_buttonBack)
	{
		returnToPreviousScreen(false);
	}
	else if (context == m_buttonNext)
	{
		if (!m_pimpl->m_popupMenu->IsVisible())
		{
			if (m_professionWasSelected)
			{
				finishAndCreateCharacter();
			}
			else
			{
				CuiMessageBox::createInfoBox(CuiStringIdsServer::server_please_select_a_profession.localize());
			}
		}
	}
	else if (context == m_buttonPreview)
	{
		if (m_professionWasSelected)
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
		UIButton * const button = UI_ASOBJECT(UIButton, context);
		if (button != 0)
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
				sprintf(tmp, "trader_specialty_%s", Unicode::wideToNarrow(value).c_str());
				StringId s("ui_roadmap", tmp);
				currentSpecialtyText.append(s.localize());
				m_pimpl->m_professionSpecialtiesText->SetLocalText(currentSpecialtyText);
				setProfessionSubTemplate(value);
			}
			else if (button->GetProperty(s_buttonIdProperty, buttonId))
			{
				m_pimpl->m_selectedButton = button;
				
				// Set description page text.
				m_pimpl->m_professionDescriptionText->SetText(Unicode::narrowToWide(s_professionTemplateStringFile) + buttonId + Unicode::narrowToWide(s_stfDescription));
				
				// Set specialties page text.
				m_pimpl->m_professionSpecialtiesText->SetText(Unicode::narrowToWide(s_professionTemplateStringFile) + buttonId + Unicode::narrowToWide(s_stfSpecialties));

				UIString buttonId;
				if (m_pimpl->m_selectedButton->GetProperty(s_buttonIdProperty, buttonId))
				{
					setProfessionTemplate(buttonId);
				}

				// Open popup menu if this is trader
				if(Unicode::wideToNarrow(buttonId) == s_traderButtonId)
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

				m_buttonPreview->SetVisible(true);
			}
		}
	}

	setButtonSelected();

	if (m_buttonNext != 0)
	{
		m_buttonNext->SetSelected(true);
	}
}

//----------------------------------------------------------------------

void SwgCuiAvatarProfessionTemplateSelect::update(float const deltaTimeSecs)
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

void SwgCuiAvatarProfessionTemplateSelect::buildProfessionButtons()
{
	std::vector<std::string> startingTemplates;
	RoadmapManager::getStartingProfessions(startingTemplates, true);

	UIButton * firstButton = NULL;

	volumePageFixup();

	for (std::vector<std::string>::const_iterator itStartingTemplate = startingTemplates.begin(); itStartingTemplate != startingTemplates.end(); ++itStartingTemplate) 
	{
		std::string const & templateBaseName = *itStartingTemplate;
		if (!templateBaseName.empty()) 
		{
			UISmartPointer<UIPage> newProfessionButtonPage(UI_ASOBJECT(UIPage, m_pimpl->m_sampleProfessionPage->DuplicateObject()));
			m_pimpl->m_professionPages.push_back(newProfessionButtonPage);

			// add the page to the volume.
			m_pimpl->m_volumePage->AddChild(newProfessionButtonPage);

			// set the location.
			newProfessionButtonPage->SetLocation(UIPoint::zero);
			newProfessionButtonPage->SetVisible(true);
			newProfessionButtonPage->SetActivated(true);
			newProfessionButtonPage->SetEnabled(true);
			
			// link it up.
			newProfessionButtonPage->Link();
			newProfessionButtonPage->SetPackDirty(true);
			newProfessionButtonPage->ForcePackChildren();

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

			// link it up.
			newProfessionButtonPage->SetVisible(true);
			newProfessionButtonPage->SetActivated(true);
			newProfessionButtonPage->SetEnabled(true);

			newProfessionButtonPage->Link();
			newProfessionButtonPage->SetPackDirty(true);
			newProfessionButtonPage->ForcePackChildren();

			// keep track of the first button in case we need to select it.
			if (!firstButton) 
			{
				firstButton = button;
			}
			
			// select the default button.
			if (m_pimpl->m_defaultProfessionSelection == templateBaseName) 
			{
				button->SetSelected(true);
				OnButtonPressed(button);
			}
		}
	}


	volumePageFixup();

	// link it up.
	m_pimpl->m_volumePage->Link();
	m_pimpl->m_volumePage->SetVisible(true);
	m_pimpl->m_volumePage->SetActivated(true);
	m_pimpl->m_volumePage->SetEnabled(true);

	setButtonSelected();
}

//----------------------------------------------------------------------

void SwgCuiAvatarProfessionTemplateSelect::destroyProfessionButtons()
{
	m_pimpl->m_professionPages.clear();
	m_pimpl->m_volumePage->Clear();
	unregisterMediatorObjects();
}

//----------------------------------------------------------------------

void SwgCuiAvatarProfessionTemplateSelect::setProfessionTemplate(UIString const & templateName)
{
	if (!templateName.empty()) 
	{
		std::string professionTemplate("smuggler_2a");
		std::string startingSkill("combat_brawler_2handmelee_01");
		RoadmapManager::getStartingProfessionTemplateAndSkill(Unicode::wideToNarrow(templateName), professionTemplate, startingSkill);

		CuiSkillManager::setSkillTemplate(professionTemplate);
		CuiSkillManager::setWorkingSkill(startingSkill);
		m_professionWasSelected = true;
	}
}

//----------------------------------------------------------------------

void SwgCuiAvatarProfessionTemplateSelect::setProfessionSubTemplate(UIString const & templateName)
{
	if (!templateName.empty()) 
	{
		std::string startingSkill("combat_brawler_2handmelee_01");
		std::string narrowTemplateName = Unicode::wideToNarrow(templateName);
		RoadmapManager::getStartingProfessionSkill(narrowTemplateName, startingSkill);

		DEBUG_WARNING(true, ("setProfessionSubTemplate setting template to %s", narrowTemplateName.c_str()));
		CuiSkillManager::setSkillTemplate(narrowTemplateName);
		CuiSkillManager::setWorkingSkill(startingSkill);
	}
}

//----------------------------------------------------------------------

void SwgCuiAvatarProfessionTemplateSelect::setButtonSelected()
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

void SwgCuiAvatarProfessionTemplateSelect::volumePageFixup()
{
	// Hack to fill volume page correctly.
	UISize const cellCount = m_pimpl->m_volumePage->GetCellCountFixed();
	m_pimpl->m_volumePage->SetCellCountFixed(UISize(cellCount.x+1, cellCount.y+1));
	m_pimpl->m_volumePage->SetCellCountFixed(cellCount);
}

//----------------------------------------------------------------------

void SwgCuiAvatarProfessionTemplateSelect::onCreationAborted(bool)
{
	m_aborted = true;

	if (m_pendingCreationFinished)
	{
		WARNING (true,("Got abort message in the same frame as pending creation finished message ."));
	}
	else
	{
		GameNetwork::setAcceptSceneCommand(false);
		GameNetwork::disconnectConnectionServer();
		CuiMediatorFactory::activate(CuiMediatorTypes::AvatarSelection);
		SwgCuiAvatarCreationHelper::requestRandomName(false);
		SwgCuiAvatarCreationHelper::purgePool();
		deactivate();
	}
}

//----------------------------------------------------------------------

void SwgCuiAvatarProfessionTemplateSelect::onCreationFinished(bool b)
{
	if (b)
	{
		if (m_aborted)
		{
			WARNING (true,("Got creation finished message in the same frame as user abort message."));
		}
		else
		{
			m_pendingCreationFinished = false;

			CreatureObject * const player = SwgCuiAvatarCreationHelper::getCreature();

			Unicode::String playerName;

			if (player == 0)
			{
				if (!SwgCuiAvatarCreationHelper::wasLastCreationAutomatic(playerName))
				{
					WARNING(true, ("No player"));
					return;
				}
			}
			else
			{
				playerName = player->getObjectName();
			}

			ConfigClientGame::setLauncherAvatarName(std::string());
			ConfigClientGame::setLauncherClusterId(CuiLoginManager::getConnectedClusterId ());
			ConfigClientGame::setAvatarName(Unicode::wideToNarrow (playerName));
			ConfigClientGame::setCentralServerName(CuiLoginManager::getConnectedClusterName ());
			ConfigClientGame::setNextAutoConnectToGameServer(true);

			CuiMediatorFactory::activate(CuiMediatorTypes::AvatarSelection);

			SwgCuiAvatarCreationHelper::purgePool();
			deactivate();
		}
	}
	else
	{
		returnToPreviousScreen(true);
	}
}

//----------------------------------------------------------------------

void SwgCuiAvatarProfessionTemplateSelect::finishAndCreateCharacter()
{
	CreatureObject * const player = SwgCuiAvatarCreationHelper::getCreature();

	Unicode::String playerName;

	if (!player)		
	{
		if (!SwgCuiAvatarCreationHelper::wasLastCreationAutomatic(playerName))
		{
			WARNING (true, ("No player"));
			return;
		}
	}
	else
	{
		playerName = player->getObjectName();
	}

	SwgCuiAvatarCreationHelper::purgeExtraPoolMembers();
	SwgCuiAvatarCreationHelper::setCreatureCustomized(true);

	if (SwgCuiAvatarCreationHelper::finishCreation())
	{
		deactivate();
	}
}

//----------------------------------------------------------------------

void SwgCuiAvatarProfessionTemplateSelect::returnToPreviousScreen(bool const dueToError)
{
	std::string const whereTo((dueToError) ? CuiMediatorTypes::AvatarSimple : CuiMediatorTypes::AvatarCustomize);

	CuiTransition::startTransition(CuiMediatorTypes::AvatarProfessionTemplateSelect, whereTo);
}

//======================================================================
