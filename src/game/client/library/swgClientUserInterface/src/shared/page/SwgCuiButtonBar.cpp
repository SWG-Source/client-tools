//======================================================================
//
// SwgCuiButtonBar.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiButtonBar.h"

#include "UIButton.h"
#include "UIComposite.h"
#include "UICursor.h"
#include "UIData.h"
#include "UIEffector.h"
#include "UIManager.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UIPopupMenu.h"
#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientExpertiseManager.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/PlayerObject.h"
#include "clientGame/RoadmapManager.h"
#include "clientGraphics/ConfigClientGraphics.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiPersistentMessageManager.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiSoundManager.h"
#include "clientUserInterface/CuiSounds.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "clientUserInterface/CuiTextManager.h"
#include "clientUserInterface/CuiWorkspace.h"
#include "sharedGame/HyperspaceManager.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/VolumeContainer.h"
#include "sharedUtility/Callback.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"

//======================================================================

namespace SwgCuiButtonBarNamespace
{
	const std::string pop_horizontal = "horizontal";
	const std::string pop_vertical   = "vertical";

	const StringId inventory_full_warning = StringId("ui_inv", "inventory_full_warning");
	const StringId inventory_full = StringId("ui_inv", "inventory_full");
	const StringId squelched = StringId("ui_chatroom", "squelched");
	const StringId lotsOverLimitSpam = StringId("ui", "lots_over_limit_spam");

	// these numbers were reduced based on the removal of the help/cs and tcg menu buttons otherwise the hover offset is fucked
	// also for some reason these numbers need to be 2 less than the actual number of menu items, thanks SOE
	const int NORMAL_NUMBER_BUTTONS_SPACE = 16;
	const int NORMAL_NUMBER_BUTTONS_GROUND = 14;

	void onConfirmGoHomeClosed(const CuiMessageBox & box);

	class MyOpacityCallback : public CallbackReceiver
	{
	public:
		explicit MyOpacityCallback (SwgCuiButtonBar & buttonBar);

		virtual void performCallback()
		{
			m_buttonBar->onOpacityCallback();
		}

		~MyOpacityCallback()
		{
			m_buttonBar = 0;
		}

	private:
		// Not implemented.
		MyOpacityCallback();
		MyOpacityCallback & operator=(MyOpacityCallback const &);

	private:
		SwgCuiButtonBar * m_buttonBar;
	};

	MyOpacityCallback::MyOpacityCallback (SwgCuiButtonBar & buttonBar) :
	CallbackReceiver (),
		m_buttonBar (&buttonBar)
	{
	}
}

using namespace SwgCuiButtonBarNamespace;

//----------------------------------------------------------------------

SwgCuiButtonBar::SwgCuiButtonBar              (UIPage & page) :
SwgCuiLockableMediator             ("SwgCuiButtonBar", page),
m_communityButton       (0),
m_mailButton            (0),
m_inventoryButton       (0),
m_journalButton(NULL),
m_roadmapButton(NULL),
m_effectorNewMail       (0),
m_effectorInventoryFull (0),
m_journalButtonEffector(NULL),
m_effectingNewMail      (false),
m_effectingInventoryFull(false),
m_journalMissionUpdate(false),
m_journalMissionCount(0),
m_menuButton(0),
m_buttonsComposite(0),
m_mapButton(0),
m_datapadButton(0),
m_characterButton(0),
m_expertiseButton(0),
m_optionsButton(0),
m_commandsButton(0),
//m_serviceButton(0),
m_submenuButton(0),
m_shipDetailsButton(0),
m_homePortButton(0),
m_myCollectionsButton(0),
//m_tcgButton(0),
m_appearanceButton(0),
m_questBuilderButton(0),
m_gcwInfoButton(0),
m_effectingExpertise(false),
m_effectorExpertise(0),
m_effectingMenu(false),
m_effectorMenu(0),
m_menuButtonPage(0),
m_mouseoverPage(0),
m_numberButtons(0),
m_hoverState(false),
m_menuButtonRestLoc(),
m_menuMovedIgnoredPress(false),
m_scheduleButtonPress(false),
m_opacityCallback      (0)
{
	m_opacityCallback = new MyOpacityCallback (*this);

	getCodeDataObject (TUIButton,     m_communityButton,         "buttonCommunity");
	getCodeDataObject (TUIButton,     m_mailButton,              "buttonMail");
	getCodeDataObject (TUIButton,     m_inventoryButton,         "buttoninventory");
	getCodeDataObject (TUIButton,     m_journalButton,           "buttonJournal");
	getCodeDataObject (TUIButton,     m_roadmapButton,           "buttonRoadmap");
	getCodeDataObject (TUIButton,     m_mapButton,               "buttonMap");
	getCodeDataObject (TUIButton,     m_datapadButton,           "buttonDatapad");
	getCodeDataObject (TUIButton,     m_characterButton,         "buttonCharacter");
	getCodeDataObject (TUIButton,     m_expertiseButton,         "buttonExpertise");
	getCodeDataObject (TUIButton,     m_optionsButton,           "buttonOptions");
	getCodeDataObject (TUIButton,     m_commandsButton,          "buttonCommands");
	//getCodeDataObject (TUIButton,     m_serviceButton,           "buttonService");
	getCodeDataObject (TUIButton,     m_submenuButton,           "buttonMenu");
	getCodeDataObject (TUIButton,     m_shipDetailsButton,       "buttonShipDetails",    true);
	getCodeDataObject (TUIButton,     m_homePortButton,          "buttonHomePort",       true);
	getCodeDataObject (TUIButton,     m_myCollectionsButton,     "buttonMyCollections");
	//getCodeDataObject (TUIButton,     m_tcgButton,               "buttonTcg");
	getCodeDataObject (TUIButton,     m_appearanceButton,        "buttonAppearance");
	getCodeDataObject (TUIButton,     m_questBuilderButton,		 "buttonQuestBuilder");
	getCodeDataObject (TUIButton,     m_gcwInfoButton,           "buttonGCW");


	getCodeDataObject (TUIEffector,   m_effectorNewMail,         "effectorNewMail");
	getCodeDataObject (TUIEffector,   m_effectorInventoryFull,   "effectorInventoryFull");
	getCodeDataObject (TUIEffector,   m_effectorMenu,            "effectorMenu");
	getCodeDataObject (TUIEffector,   m_effectorExpertise,       "effectorExpertise");
	getCodeDataObject (TUIEffector,   m_journalButtonEffector,   "effectorJournalMissionUpdate");

	getCodeDataObject (TUIButton,     m_menuButton,              "bigMenuButton");
	getCodeDataObject (TUIComposite,  m_buttonsComposite,        "buttonsComposite");
	getCodeDataObject (TUIPage,       m_menuButtonPage,          "bigMenuPage");
	getCodeDataObject (TUIPage,       m_mouseoverPage,           "mouseover");

	registerMediatorObject (*m_communityButton,      true);
	registerMediatorObject (*m_mailButton,           true);
	registerMediatorObject (*m_inventoryButton,      true);
	registerMediatorObject (*m_journalButton,        true);
	registerMediatorObject (*m_mapButton,            true);
	registerMediatorObject (*m_menuButton,           true);
	registerMediatorObject (*m_datapadButton,        true);
	registerMediatorObject (*m_characterButton,      true);
	registerMediatorObject (*m_expertiseButton,      true);
	registerMediatorObject (*m_optionsButton,        true);
	registerMediatorObject (*m_commandsButton,       true);
	//registerMediatorObject (*m_serviceButton,        true);
	registerMediatorObject (*m_submenuButton,        true);
	registerMediatorObject (*m_myCollectionsButton,  true);
	//registerMediatorObject (*m_tcgButton,            true);
	registerMediatorObject (*m_appearanceButton,     true);
	registerMediatorObject (*m_questBuilderButton,   true);
	registerMediatorObject (*m_gcwInfoButton,        true);

	if (m_shipDetailsButton)
		registerMediatorObject (*m_shipDetailsButton, true);
	if (m_homePortButton)
		registerMediatorObject (*m_homePortButton, true);
	if (m_roadmapButton)
		registerMediatorObject (*m_roadmapButton, true);

	getPage ().SetContextCapable (true, true);

	registerMediatorObject (getPage (), true);

	for (int buttonStates = 0; buttonStates < MaxButtonStates; ++buttonStates)
	{
		m_buttonImages[buttonStates] = NULL;

		std::string imageNameBase("image");
		char const asciiNum = static_cast<char>('0' + buttonStates);
		imageNameBase += asciiNum;
		getCodeDataObject(TUIWidget, m_buttonImages[buttonStates], imageNameBase.c_str(), true);
	}

	m_menuButtonRestLoc = m_menuButtonPage->GetLocation();
	setSettingsAutoSizeLocation(true, true);

	m_menuButtonPage->SetOpacity(CuiPreferences::getCommandButtonOpacity());

	getPage().SetNotModifyingUseDefaultCursor(true);
	m_menuButtonPage->SetNotModifyingUseDefaultCursor(true);
	m_menuButton->SetNotModifyingUseDefaultCursor(true);

	registerMediatorObject(*m_menuButtonPage, true);
	setPageToLock(m_menuButtonPage);
}

//----------------------------------------------------------------------

SwgCuiButtonBar::~SwgCuiButtonBar                ()
{
	m_communityButton       = 0;
	m_mailButton            = 0;
	m_inventoryButton       = 0;
	m_journalButton         = 0;
	m_effectorNewMail       = 0;
	m_effectorInventoryFull = 0;
	m_effectorExpertise     = 0;
	m_journalButtonEffector = 0;
	m_homePortButton        = 0;

	for (int buttonStates = 0; buttonStates < MaxButtonStates; ++buttonStates)
	{
		m_buttonImages[buttonStates] = NULL;
	}
	delete m_opacityCallback;
	m_opacityCallback = 0;
}

//----------------------------------------------------------------------

void  SwgCuiButtonBar::performActivate()
{
	m_menuButtonPage->SetOpacity(CuiPreferences::getCommandButtonOpacity());
	CuiPreferences::getCommandButtonOpacityCallback ().attachReceiver (*m_opacityCallback);
	setIsUpdating (true);
}

//----------------------------------------------------------------------

void  SwgCuiButtonBar::performDeactivate()
{
	CuiPreferences::getCommandButtonOpacityCallback ().detachReceiver (*m_opacityCallback);
	setIsUpdating (false);

	m_mailButton->CancelEffector (*m_effectorNewMail);
	m_effectingNewMail = false;

	m_inventoryButton->CancelEffector (*m_effectorInventoryFull);
	m_effectingInventoryFull = false;

	m_menuButton->CancelEffector(*m_effectorMenu);
	m_effectingMenu = false;

	m_expertiseButton->CancelEffector(*m_effectorExpertise);
	m_effectingExpertise = false;

	m_journalMissionCount = 0;
}

//----------------------------------------------------------------------

void SwgCuiButtonBar::turnOffInventoryEffector()
{
	m_inventoryButton->CancelEffector (*m_effectorInventoryFull);
	m_inventoryButton->SetColor (UIColor::white);
	m_inventoryButton->SetBackgroundColor (UIColor::white);
	m_inventoryButton->SetBackgroundTint (UIColor::white);
	m_effectingInventoryFull = false;
}

//----------------------------------------------------------------------

void SwgCuiButtonBar::update (float deltaTimeSecs)
{
	CuiMediator::update (deltaTimeSecs);

	updateExpertiseEffector();

	updateJournalEffector();

	updateMenuPosition();
	updateMenuHighlight();
	updateSkinnedImageState();

	if(m_scheduleButtonPress)
	{
		m_scheduleButtonPress = false;
		if(m_menuMovedIgnoredPress)
			m_menuMovedIgnoredPress = false;
		else
			m_menuButton->Press();
	}

	if (m_buttonsComposite->IsVisible())
	{
		UIPage * const parent = dynamic_cast<UIPage *>(getPage ().GetParent ());

		if (parent)
		{
			parent->MoveChild(&getPage(),UIBaseObject::Top);
		}
	}

	const bool hasNewMail = CuiPersistentMessageManager::hasNewMail ();

	bool inventoryFull = false;
	const CreatureObject * const player = Game::getPlayerCreature ();
	if (player)
	{
		Unicode::String squelchedAndOrLotsOverLimitSpamStr;

		{
			if (Game::isPlayerSquelched())
				squelchedAndOrLotsOverLimitSpamStr = squelched.localize();

			PlayerObject const * const playerObject = player->getPlayerObject();
			if (playerObject && (playerObject->getAccountNumLotsOverLimitSpam() > 0))
			{
				if (!squelchedAndOrLotsOverLimitSpamStr.empty())
					squelchedAndOrLotsOverLimitSpamStr += Unicode::narrowToWide("\r\n");

				Unicode::String tempStr;
				CuiStringVariablesManager::process(lotsOverLimitSpam, Unicode::emptyString, Unicode::emptyString, Unicode::emptyString, playerObject->getAccountNumLotsOverLimitSpam(), 0.0f, tempStr);
				squelchedAndOrLotsOverLimitSpamStr += tempStr;
			}
		}

		const ClientObject * const inventory = player->getInventoryObject();
		if (inventory)
		{
			const VolumeContainer* const volume = ContainerInterface::getVolumeContainer(*inventory);
			if (volume)
			{
				const int total = volume->getTotalVolume();
				const int v = volume->getCurrentVolume();
				if (v > total)
				{
					inventoryFull = true;
				}

				if (v >= (total + ConfigClientGame::getDisableMovementInventoryOverload()))
				{
					if (squelchedAndOrLotsOverLimitSpamStr.empty())
					{
						// in space, don't display the "CANNOT MOVE" message, since they can move
						// (i.e. steer the ship) just fine; just display the OVERLOADED message
						if (!Game::isHudSceneTypeSpace())
							CuiTextManager::showSystemStatusString(inventory_full.localize());
						else
							CuiTextManager::showSystemStatusString(inventory_full_warning.localize());
					}
					else
					{
						squelchedAndOrLotsOverLimitSpamStr += Unicode::narrowToWide("\r\n");

						// in space, don't display the "CANNOT MOVE" message, since they can move
						// (i.e. steer the ship) just fine; just display the OVERLOADED message
						if (!Game::isHudSceneTypeSpace())
							squelchedAndOrLotsOverLimitSpamStr += inventory_full.localize();
						else
							squelchedAndOrLotsOverLimitSpamStr += inventory_full_warning.localize();

						CuiTextManager::showSystemStatusString(squelchedAndOrLotsOverLimitSpamStr);
					}
				}
				else if (inventoryFull)
				{
					if (squelchedAndOrLotsOverLimitSpamStr.empty())
					{
						CuiTextManager::showSystemStatusString(inventory_full_warning.localize());
					}
					else
					{
						squelchedAndOrLotsOverLimitSpamStr += Unicode::narrowToWide("\r\n");
						squelchedAndOrLotsOverLimitSpamStr += inventory_full_warning.localize();
						CuiTextManager::showSystemStatusString(squelchedAndOrLotsOverLimitSpamStr);
					}
				}
				else if (!squelchedAndOrLotsOverLimitSpamStr.empty())
				{
					CuiTextManager::showSystemStatusString(squelchedAndOrLotsOverLimitSpamStr);
				}
			}
			else
			{
				if (!squelchedAndOrLotsOverLimitSpamStr.empty())
					CuiTextManager::showSystemStatusString(squelchedAndOrLotsOverLimitSpamStr);

				turnOffInventoryEffector();
				return;
			}
		}
		else
		{
			if (!squelchedAndOrLotsOverLimitSpamStr.empty())
				CuiTextManager::showSystemStatusString(squelchedAndOrLotsOverLimitSpamStr);

			turnOffInventoryEffector();
			return;
		}
	}
	else
	{
		turnOffInventoryEffector();
		return;
	}

	if (m_effectingNewMail)
	{
		if (!hasNewMail)
		{
			m_mailButton->CancelEffector (*m_effectorNewMail);
			m_mailButton->SetColor (UIColor::white);
			m_effectingNewMail = false;
		}
	}
	else if (hasNewMail)
	{
		m_effectingNewMail = true;
		m_mailButton->ExecuteEffector (*m_effectorNewMail);
	}
	if (m_effectingInventoryFull)
	{
		if (!inventoryFull)
		{
			turnOffInventoryEffector();
		}
	}
	else if (inventoryFull)
	{
		m_effectingInventoryFull = true;
		m_inventoryButton->ExecuteEffector (*m_effectorInventoryFull);
	}

	updateMenuEffector();
}

//----------------------------------------------------------------------

void SwgCuiButtonBar::updateMenuPosition()
{
	if(!m_menuButtonPage)
		return;
	UIPoint menuButtonLoc = m_menuButtonPage->GetLocation();
	if((menuButtonLoc.x != m_menuButtonRestLoc.x) || (menuButtonLoc.y != m_menuButtonRestLoc.y))
	{
		UIPoint menuButtonDelta = menuButtonLoc - m_menuButtonRestLoc;
		UIPoint topLoc = getPage().GetLocation();
		topLoc += menuButtonDelta;
		m_menuButtonPage->SetLocation(m_menuButtonRestLoc);
		getPage().SetLocation(topLoc);
		m_menuMovedIgnoredPress = true;
	}
}

//----------------------------------------------------------------------

void SwgCuiButtonBar::updateMenuEffector()
{
	//Menu effector should be on if anything else is (except expertise effector)
	if(m_effectingNewMail || m_effectingInventoryFull || m_journalMissionUpdate)
	{
		if(!m_effectingMenu)
		{
			m_effectingMenu = true;
			m_menuButton->ExecuteEffector(*m_effectorMenu);
		}
	}
	else
	{
		if(m_effectingMenu)
		{
			m_effectingMenu = false;
			m_menuButton->CancelEffector(*m_effectorMenu);
		}
	}
}

//----------------------------------------------------------------------

bool SwgCuiButtonBar::isCompositeVisible()
{
	return m_buttonsComposite->IsVisible();
}

//----------------------------------------------------------------------

void SwgCuiButtonBar::updateMenuHighlight()
{
	if(!m_buttonsComposite->IsVisible())
		return;

	UIPoint mouseCoord = UIManager::gUIManager().GetLastMouseCoord();

	UIPoint loc    = m_buttonsComposite->GetWorldLocation();
	UIPoint locRel = m_buttonsComposite->GetLocation();

	float cellSize = static_cast<float>(m_buttonsComposite->GetSize().y) / m_numberButtons;

	int slot = static_cast<int>((mouseCoord.y - loc.y) / cellSize);
	if(slot < 0) slot = 0;
	if(slot > (m_numberButtons - 1)) slot = (m_numberButtons - 1);

	int offset = static_cast<int>(ceil(slot * cellSize));

	m_mouseoverPage->SetLocation(locRel.x + 3, locRel.y + offset);
}

//----------------------------------------------------------------------

bool SwgCuiButtonBar::OnMessage (UIWidget * context, const UIMessage & msg)
{
	NOT_NULL (context);

	if (context == &getPage ())
	{
		if(msg.Type == UIMessage::LeftMouseUp || msg.Type == UIMessage::LeftMouseDown)
		{
			UIWidget *widget = getPage().GetWidgetFromPoint(msg.MouseCoords, false);
			if(!widget)
				return true;
			if(!widget->IsA(TUIText))
			{
				return true;
			}
			if(!m_buttonsComposite->IsVisible())
				return true;
			if (msg.Type == UIMessage::LeftMouseDown)
				return false;
			//Get parent, get the second child of that, press it
			UIBaseObject *parent = widget->GetParentWidget();
			if(!parent)
				return true;
			UIBaseObject::UIObjectList childList;
			UIBaseObject::UIObjectList::iterator buttonListI;
			parent->GetChildren(childList);
			buttonListI = childList.begin();
			if(buttonListI == childList.end())
				return true;
			++buttonListI;
			if(buttonListI == childList.end())
				return true;
			UIBaseObject *button = (*buttonListI);
			if(!button->IsA(TUIButton))
				return true;
			(safe_cast<UIButton *>(button))->Press();
			return true;
		}
	}
	else if (context == m_menuButtonPage)
	{
		if (msg.Type == UIMessage::RightMouseUp)
		{
			SwgCuiLockableMediator::generateLockablePopup(context, msg);
			return false;
		}
	}
	return true;
}

void SwgCuiButtonBar::toggleMenu()
{
	if(m_buttonsComposite->IsVisible())
	{
		m_menuButtonPage->SetOpacity(CuiPreferences::getCommandButtonOpacity());
		m_mouseoverPage->SetVisible(false);
		m_buttonsComposite->SetVisible(false);

		CuiManager::requestPointer (false);

	}
	else
	{
		getPage().SetGetsInput(true);
		m_menuButtonPage->SetOpacity(1.0f);
		m_buttonsComposite->SetGetsInput(true);
		m_buttonsComposite->SetVisible(true);
		m_mouseoverPage->SetVisible(true);

		//Only show the roadmap and expertise options if the correct conditions are met.
		m_numberButtons = Game::isHudSceneTypeSpace() ? NORMAL_NUMBER_BUTTONS_SPACE : NORMAL_NUMBER_BUTTONS_GROUND;

		if(RoadmapManager::playerIsNewCharacter())
		{
			m_expertiseButton->GetParentWidget()->SetVisible(false);
			m_roadmapButton->GetParentWidget()->SetVisible(false);
		}
		else
		{
			if(m_roadmapButton)
			{
				if (RoadmapManager::playerIsOnRoadmap())
				{
					m_roadmapButton->GetParentWidget()->SetVisible(true);
					++m_numberButtons;
				}
				else
				{
					m_roadmapButton->GetParentWidget()->SetVisible(false);
				}
			}
			if(m_expertiseButton)
			{
				if (ClientExpertiseManager::hasExpertiseTrees() && ClientExpertiseManager::getExpertisePointsTotalForPlayer() > 0)
				{
					m_expertiseButton->GetParentWidget()->SetVisible(true);
					++m_numberButtons;
				}
				else
				{
					m_expertiseButton->GetParentWidget()->SetVisible(false);
				}
			}
		}

		m_buttonsComposite->Pack();
		m_buttonsComposite->Pack();

		//Position the menu appropriately; detect if menu is on left or top of screen
		UIPoint screenLoc = m_menuButton->GetWorldLocation();
		UIPoint newLoc(0, 0);

		if (screenLoc.x < (m_buttonsComposite->GetSize().x + 5))
			newLoc.x = 110;

		if (screenLoc.y < (m_buttonsComposite->GetSize().y + 5))
			newLoc.y = m_menuButtonPage->GetLocation().y + m_menuButtonPage->GetSize().y;
		else
			newLoc.y = m_menuButtonPage->GetLocation().y - m_buttonsComposite->GetSize().y;

		m_buttonsComposite->SetLocation(newLoc);

		CuiManager::requestPointer (true);
	}
}

//----------------------------------------------------------------------

void SwgCuiButtonBar::OnButtonPressed   (UIWidget * context)
{
	if (context == m_menuButton)
	{
		if(m_menuMovedIgnoredPress)
		{
			m_menuMovedIgnoredPress = false;
			return;
		}
		toggleMenu();
	}
	else
	{
		if (context == m_homePortButton)
		{
			if (HyperspaceManager::isValidSceneForHomeLocationHyperspace(Game::getSceneId()))
			{
				CuiMessageBox::createYesNoBox(CuiStringIds::confirm_go_home.localize(), &onConfirmGoHomeClosed);
			}
		}
		else
		{
			m_mouseoverPage->SetVisible(false);
			m_buttonsComposite->SetVisible(false);

			CuiManager::requestPointer (false);
		}

	}
}

//----------------------------------------------------------------------

void SwgCuiButtonBar::ensureMenuIsVisible()
{
	if (!m_buttonsComposite->IsVisible())
	{
		m_menuButtonPage->SetOpacity(1.0f);
		m_buttonsComposite->SetVisible(true);
		m_mouseoverPage->SetVisible(true);

		CuiManager::requestPointer (true);
	}
}

//----------------------------------------------------------------------

void SwgCuiButtonBar::OnPopupMenuSelection (UIWidget * context)
{
	NOT_NULL (context);

	const UIPopupMenu * const pop = safe_cast<UIPopupMenu *>(context);

	const std::string & sel = pop->GetSelectedName ();

	if (sel == pop_horizontal)
	{
		const UISize & size_max = getPage ().GetMaximumSize ();
		const UISize & size_min = getPage ().GetMinimumSize ();
		getPage ().SetSize (UISize (size_max.x, size_min.y));
		CuiSoundManager::play (CuiSounds::increment_big);
	}
	else if (sel == pop_vertical)
	{
		const UISize & size_max = getPage ().GetMaximumSize ();
		const UISize & size_min = getPage ().GetMinimumSize ();
		getPage ().SetSize (UISize (size_min.x, size_max.y));

		CuiSoundManager::play (CuiSounds::increment_big);
	}
	else
		SwgCuiLockableMediator::OnPopupMenuSelection(context);

	CuiWorkspace * const ws = getContainingWorkspace ();
	if (ws)
		ws->positionMediator (*this, getPage ().GetLocation (), false);

	saveSettings ();
}

//----------------------------------------------------------------------

void SwgCuiButtonBar::enableJournalEffector(bool const enabled)
{
	if (!boolEqual(enabled, m_journalMissionUpdate))
	{
		m_journalMissionUpdate = enabled;

		if (enabled)
		{
			m_journalButton->ExecuteEffector(*m_journalButtonEffector);
		}
		else
		{
			m_journalButton->CancelEffector(*m_journalButtonEffector);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiButtonBar::updateJournalEffector()
{
	PlayerObject const * const playerObject = Game::getPlayerObject();

	int const currentJournalCount = playerObject ? playerObject->questCount() : 0;
	if (currentJournalCount != m_journalMissionCount)
	{
		m_journalMissionCount = currentJournalCount;
		enableJournalEffector(currentJournalCount > 0);
	}
	else if (m_journalMissionUpdate)
	{
		CuiMediator const * const journalMediator = CuiMediatorFactory::getInWorkspace(CuiMediatorTypes::WS_QuestJournal);
		if (journalMediator && journalMediator->isEnabled())
		{
			enableJournalEffector(false);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiButtonBar::updateExpertiseEffector()
{
	if(!m_buttonsComposite->IsVisible())
		return;

	// @todo: connect to onSkillsChanged and onLevelChanged callbacks in order to set this on demand (as in SwgCuiExpertise)
	if (ClientExpertiseManager::getExpertisePointsRemainingForPlayer() > 0)
	{
		if(!m_effectingExpertise)
		{
			m_effectingExpertise = true;
			m_expertiseButton->ExecuteEffector(*m_effectorExpertise);
		}
	}
	else
	{
		if(m_effectingExpertise)
		{
			m_effectingExpertise = false;
			m_expertiseButton->CancelEffector(*m_effectorExpertise);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiButtonBar::OnHoverOut( UIWidget *Context )
{
	if (m_menuButton == Context)
	{
		m_menuButtonPage->SetOpacity(CuiPreferences::getCommandButtonOpacity());
		m_hoverState = false;
	}
}

//----------------------------------------------------------------------

void SwgCuiButtonBar::OnHoverIn( UIWidget *Context )
{
	if (m_menuButton == Context)
	{
		m_menuButtonPage->SetOpacity(1.0f);
		m_hoverState = true;
	}
}

//----------------------------------------------------------------------

void SwgCuiButtonBar::updateSkinnedImageState()
{
	int const stateIndex = static_cast<int>(m_hoverState) + static_cast<int>(m_buttonsComposite->IsVisible()) * 2;
	for (int buttonStates = 0; buttonStates < MaxButtonStates; ++buttonStates)
	{
		if (m_buttonImages[buttonStates])
		{
			if (Game::isHudSceneTypeSpace())
			{
				m_buttonImages[buttonStates]->SetVisible(false);
			}
			else
			{
				m_buttonImages[buttonStates]->SetVisible(buttonStates == stateIndex || buttonStates == 0);
			}
		}
	}
}

//-----------------------------------------------------------------

void SwgCuiButtonBarNamespace::onConfirmGoHomeClosed(const CuiMessageBox & box)
{
	if (box.completedAffirmative())
	{
		IGNORE_RETURN(ClientCommandQueue::enqueueCommand("hyperspace", NetworkId::cms_invalid, Unicode::narrowToWide(HyperspaceManager::getHomeLocationHyperspacePointName())));
	}
}

//-----------------------------------------------------------------

void SwgCuiButtonBar::onOpacityCallback()
{
	m_menuButtonPage->SetOpacity(CuiPreferences::getCommandButtonOpacity());
}

//======================================================================
