// ============================================================================
//
// SwgCuiCommunity_Character.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiCommunity_Character.h"

#include "clientGame/ClientCommandQueue.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/MatchMakingManager.h"
#include "clientGame/PlayerObject.h"
#include "clientGame/Species.h"
#include "clientUserInterface/CuiActionManager.h"
#include "clientUserInterface/CuiActions.h"
#include "clientUserInterface/CuiSkillManager.h"
#include "clientUserInterface/CuiStringIdsCommunity.h"
#include "sharedFoundation/CalendarTime.h"
#include "sharedGame/CitizenRankDataTable.h"
#include "sharedGame/GameLanguageManager.h"
#include "sharedGame/GuildRankDataTable.h"
#include "sharedMessageDispatch/Emitter.h"
#include "sharedMessageDispatch/Message.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/Object.h"
#include "swgClientUserInterface/SwgCuiLFG.h"

#include "UIButton.h"
#include "UICheckbox.h"
#include "UIComboBox.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIList.h"
#include "UIText.h"

// ============================================================================
//
// SwgCuiCommunity::Character
//
// ============================================================================

namespace SwgCuiCommunityCharacterNamespace
{
	int const s_interestUpdateInterval = 10;
	int const s_interestUpdateDelay = 3;
	time_t s_timeToUpdateServerInterest = 0;
	bool s_interestsChanged = false;	
}

using namespace SwgCuiCommunityCharacterNamespace;

//-----------------------------------------------------------------------------

SwgCuiCommunity::Character::Character(UIPage &page, SwgCuiCommunity const &communityMediator)
 : CuiMediator("SwgCuiCommunity_Character", page)
 , UIEventCallback()
 , m_communityMediator(communityMediator)
 , m_nameText(NULL)
 , m_birthDateText(NULL)
 , m_speciesText(NULL)
 , m_bioText(NULL)
 , m_lookingForGroupCheckBox(NULL)
 , m_searchableByCtsSourceGalaxyCheckBox(NULL)
 , m_displayLocationInSearchResultsCheckBox(NULL)
 , m_characterSearchableCheckBox(NULL)
 , m_helperCheckBox(NULL)
 , m_rolePlayCheckBox(NULL)
 , m_lookingForWorkCheckBox(NULL)
 , m_awayFromKeyBoardCheckBox(NULL)
 , m_displayingFactionRankCheckBox(NULL)
 , m_titleComboBox(NULL)
 , m_languageComboBox(NULL)
 , m_callBack(new MessageDispatch::Callback)
 , m_biography()
 , m_currentTitle()
 , m_Lfg(NULL)
{
	m_callBack->connect(*this, &SwgCuiCommunity::Character::onBiographyRetrieved, static_cast<PlayerCreatureController::Messages::BiographyRetrieved *>(0));
	m_callBack->connect(*this, &SwgCuiCommunity::Character::onWhoStatusChanged, static_cast<PlayerObject::Messages::WhoStatusChanged *>(0));
	m_callBack->connect(*this, &SwgCuiCommunity::Character::onSpokenLanguageChanged, static_cast<PlayerObject::Messages::SpokenLangaugeChanged *>(0));

	getCodeDataObject(TUIText, m_nameText, "textName");
	registerMediatorObject(*m_nameText, true);

	getCodeDataObject(TUIText, m_birthDateText, "textBirthDate");
	registerMediatorObject(*m_birthDateText, true);
	m_birthDateText->Clear();

	getCodeDataObject(TUIText, m_speciesText, "textSpecies");
	registerMediatorObject(*m_speciesText, true);

	getCodeDataObject(TUIText, m_bioText, "textBio");
	registerMediatorObject(*m_bioText, true);
	m_bioText->SetEditable(false);
	m_bioText->SetMaxLines(-1);
	m_bioText->SetMaximumCharacters(1024);

	getCodeDataObject(TUICheckbox, m_searchableByCtsSourceGalaxyCheckBox, "checkSearchByCtsSourceGalaxy");
	registerMediatorObject(*m_searchableByCtsSourceGalaxyCheckBox, true);

	getCodeDataObject(TUICheckbox, m_displayLocationInSearchResultsCheckBox, "checkhideloc");
	registerMediatorObject(*m_displayLocationInSearchResultsCheckBox, true);

	getCodeDataObject(TUICheckbox, m_characterSearchableCheckBox, "checkMakeSearchable");
	registerMediatorObject(*m_characterSearchableCheckBox, true);

	getCodeDataObject(TUICheckbox, m_awayFromKeyBoardCheckBox, "checkAwayFromKeyboard");
	registerMediatorObject(*m_awayFromKeyBoardCheckBox, true);

	getCodeDataObject(TUICheckbox, m_displayingFactionRankCheckBox, "checkDisplayingFactionRank");
	registerMediatorObject(*m_displayingFactionRankCheckBox, true);

	getCodeDataObject(TUIComboBox, m_titleComboBox, "comboTitle");
	registerMediatorObject(*m_titleComboBox, true);

	getCodeDataObject(TUIComboBox, m_languageComboBox, "comboLanguage");
	registerMediatorObject(*m_languageComboBox, true);

	UIPage *lfgPage = NULL;
	getCodeDataObject(TUIPage, lfgPage, "pagelfg");
	m_Lfg = new SwgCuiLfg(*lfgPage, true);
	
	if (m_Lfg)
	{
		m_Lfg->fetch();

		lfgPage->Link();
	   	lfgPage->SetVisible(true);
		lfgPage->SetEnabled(true);

		UICheckbox * tempCheckbox = static_cast<UICheckbox *> (m_Lfg->getWidgetByName("lfg", "checkChosen"));
		
		if (tempCheckbox)
		{
			m_lookingForGroupCheckBox = tempCheckbox;
		}

		tempCheckbox = static_cast<UICheckbox *> (m_Lfg->getWidgetByName("helper", "checkChosen"));

		if (tempCheckbox)
		{
			m_helperCheckBox = tempCheckbox;
		}

		tempCheckbox = static_cast<UICheckbox *> (m_Lfg->getWidgetByName("rp", "checkChosen"));

		if (tempCheckbox)
		{
			m_rolePlayCheckBox = tempCheckbox;
		}

		tempCheckbox = static_cast<UICheckbox *> (m_Lfg->getWidgetByName("lfw", "checkChosen"));

		if (tempCheckbox)
		{
			m_lookingForWorkCheckBox = tempCheckbox;
		}	
	}
}

//-----------------------------------------------------------------------------

SwgCuiCommunity::Character::~Character()
{
	if (m_Lfg)
		m_Lfg->release();
	
	m_Lfg = NULL;

	delete m_callBack;
	m_callBack = NULL;

	m_nameText = NULL;
	m_speciesText = NULL;
	m_bioText = NULL;
	m_lookingForGroupCheckBox = NULL;
	m_searchableByCtsSourceGalaxyCheckBox = NULL;
	m_displayLocationInSearchResultsCheckBox = NULL;
	m_characterSearchableCheckBox = NULL;
	m_helperCheckBox = NULL;
	m_rolePlayCheckBox = NULL;
	m_lookingForWorkCheckBox = NULL;
	m_awayFromKeyBoardCheckBox = NULL;
	m_displayingFactionRankCheckBox = NULL;
	m_titleComboBox = NULL;
	m_languageComboBox = NULL;
}

//-----------------------------------------------------------------------------

void SwgCuiCommunity::Character::performActivate()
{
	setIsUpdating(true);

	Object *object = Game::getPlayer();
	ClientObject *clientObject = dynamic_cast<ClientObject *>(object);

	if (clientObject != NULL)
	{
		m_nameText->SetLocalText(clientObject->getLocalizedName());
	}

	CreatureObject *creatureObject = dynamic_cast<CreatureObject *>(object);

	if (creatureObject != NULL)
	{
		m_speciesText->SetPreLocalized(true);
		const Unicode::String & speciesString = Species::getLocalizedName(static_cast<SharedCreatureObjectTemplate::Species>(creatureObject->getSpecies()));
		m_speciesText->SetLocalText(speciesString);
	}

	// Make sure we have requested the biography

	PlayerObject * const playerObject = Game::getPlayerObject();

	if (playerObject != NULL)
	{
		//the actual request to the server only occurs once
		playerObject->requestBiography();

		if(playerObject->haveBiography())
		{
			m_bioText->SetLocalText(playerObject->getBiography());
			m_bioText->SetEditable(true);
			m_biography = playerObject->getBiography();
		}

		m_birthDateText->SetLocalText(Unicode::narrowToWide(CalendarTime::getCharacerBirthDateString(playerObject->getBornDate())));
	}

	// Get all the possible titles
	CreatureObject *localCreatureObject = m_communityMediator.getLocalCreatureObject();

	m_titleComboBox->Clear();

	if (localCreatureObject != NULL)
	{
		UnicodeStringToStringIdMap listItems;
		CreatureObject::SkillList const &skills = localCreatureObject->getSkills();
		CreatureObject::SkillList::const_iterator iterSkills = skills.begin();

#ifdef _DEBUG
		unsigned int const skillCount = skills.size();
		UNREF(skillCount);
#endif // _DEBUG

		for (; iterSkills != skills.end(); ++iterSkills)
		{
			SkillObject const *skillObject = (*iterSkills);

			if ((skillObject != NULL) &&
			    skillObject->isTitle())
			{
				Unicode::String localizedTitle;
				IGNORE_RETURN(CuiSkillManager::localizeSkillTitle(skillObject->getSkillName(), localizedTitle));

				IGNORE_RETURN(listItems.insert(std::make_pair(localizedTitle, skillObject->getSkillName())));
			}
		}

		UIDataSource *dataSource = m_titleComboBox->GetDataSource();

		if (dataSource != NULL)
		{
			// None is a valid skill title if you don't want to be searchable by skill title

			m_communityMediator.addItem(*dataSource, CuiStringIdsCommunity::no_title.localize(), "");

			// Citizenship is valid skill title to display what city (if any) you are a citizen of

			m_communityMediator.addItem(*dataSource, CuiStringIdsCommunity::citizenship_title.localize(), "citizenship");

			// City GCW Region Defender title and Guild GCW Region Defender title

			if (playerObject)
			{
				if (playerObject->cityGcwDefenderRegionTitleAvailable())
					m_communityMediator.addItem(*dataSource, CuiStringIdsCommunity::city_gcw_region_defender_title.localize(), "city_gcw_region_defender");

				if (playerObject->guildGcwDefenderRegionTitleAvailable())
					m_communityMediator.addItem(*dataSource, CuiStringIdsCommunity::guild_gcw_region_defender_title.localize(), "guild_gcw_region_defender");
			}

			// Add the sorted list of items

			UnicodeStringToStringIdMap::iterator iterListItems = listItems.begin();

			for (; iterListItems != listItems.end(); ++iterListItems)
			{
				m_communityMediator.addItem(*dataSource, iterListItems->first, iterListItems->second);
			}
		}

		setComboTitleToCurrentTitle();
	}

	// Set the check box states

	m_lookingForGroupCheckBox->SetChecked(MatchMakingManager::isLookingForGroup());
	m_searchableByCtsSourceGalaxyCheckBox->SetChecked(MatchMakingManager::isSearchableByCtsSourceGalaxy());
	m_displayLocationInSearchResultsCheckBox->SetChecked(MatchMakingManager::isDisplayLocationInSearchResults());
	m_characterSearchableCheckBox->SetChecked(!MatchMakingManager::isAnonymous());
	m_helperCheckBox->SetChecked(MatchMakingManager::isHelper());
	m_rolePlayCheckBox->SetChecked(MatchMakingManager::isRolePlay());
	m_lookingForWorkCheckBox->SetChecked(MatchMakingManager::isLookingForWork());
	m_awayFromKeyBoardCheckBox->SetChecked(MatchMakingManager::isAwayFromKeyBoard());
	m_displayingFactionRankCheckBox->SetChecked(MatchMakingManager::isDisplayingFactionRank());

	m_Lfg->updateBranchStateByName("general");

	if (m_characterSearchableCheckBox->IsChecked())
	{
		m_Lfg->enableWindow();
		m_Lfg->sendInterestsToServer();
		s_timeToUpdateServerInterest = ::time(NULL) + s_interestUpdateInterval;
	}
	else
	{
		m_Lfg->disableWindow();
		s_timeToUpdateServerInterest = 0;
	}

	// Set the languages

	populateLanguages();

	// Title

	if (playerObject != NULL)
	{
		//add collection and guild rank titles to title window
		UIDataSource *dataSource = m_titleComboBox->GetDataSource();
		if (dataSource != NULL)
		{
			bool addedAnyTitles = false;

			{
				std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & titleableSlots = CollectionsDataTable::getAllTitleableSlots();
				for (std::vector<CollectionsDataTable::CollectionInfoSlot const *>::const_iterator iterSlot = titleableSlots.begin(); iterSlot != titleableSlots.end(); ++iterSlot)
				{
					if (playerObject->hasCompletedCollectionSlot(**iterSlot))
					{
						for (std::vector<std::string>::const_iterator iterTitle = (*iterSlot)->titles.begin(); iterTitle != (*iterSlot)->titles.end(); ++iterTitle)
						{
							m_communityMediator.addItem(*dataSource, CollectionsDataTable::localizeCollectionTitle(*iterTitle), *iterTitle);
							addedAnyTitles = true;
						}
					}
				}
			}

			{
				std::vector<CollectionsDataTable::CollectionInfoCollection const *> const & titleableCollections = CollectionsDataTable::getAllTitleableCollections();
				for (std::vector<CollectionsDataTable::CollectionInfoCollection const *>::const_iterator iterCollection = titleableCollections.begin(); iterCollection != titleableCollections.end(); ++iterCollection)
				{
					if (playerObject->hasCompletedCollection((*iterCollection)->name))
					{
						for (std::vector<std::string>::const_iterator iterTitle = (*iterCollection)->titles.begin(); iterTitle != (*iterCollection)->titles.end(); ++iterTitle)
						{
							m_communityMediator.addItem(*dataSource, CollectionsDataTable::localizeCollectionTitle(*iterTitle), *iterTitle);
							addedAnyTitles = true;
						}
					}
				}
			}

			{
				std::vector<CollectionsDataTable::CollectionInfoPage const *> const & titleablePages = CollectionsDataTable::getAllTitleablePages();
				for (std::vector<CollectionsDataTable::CollectionInfoPage const *>::const_iterator iterPage = titleablePages.begin(); iterPage != titleablePages.end(); ++iterPage)
				{
					if (playerObject->hasCompletedCollectionPage((*iterPage)->name))
					{
						for (std::vector<std::string>::const_iterator iterTitle = (*iterPage)->titles.begin(); iterTitle != (*iterPage)->titles.end(); ++iterTitle)
						{
							m_communityMediator.addItem(*dataSource, CollectionsDataTable::localizeCollectionTitle(*iterTitle), *iterTitle);
							addedAnyTitles = true;
						}
					}
				}
			}

			{
				BitArray const & playerGuildRank = playerObject->getGuildRank();
				if (!playerGuildRank.empty())
				{
					std::vector<GuildRankDataTable::GuildRank const *> const & titleableRanks = GuildRankDataTable::getAllTitleableRanks();
					for (std::vector<GuildRankDataTable::GuildRank const *>::const_iterator iterRank = titleableRanks.begin(); iterRank != titleableRanks.end(); ++iterRank)
					{
						if (playerGuildRank.testBit((*iterRank)->slotId))
						{
							for (std::vector<std::string>::const_iterator iterTitle = (*iterRank)->titles.begin(); iterTitle != (*iterRank)->titles.end(); ++iterTitle)
							{
								m_communityMediator.addItem(*dataSource, StringId("guild_rank_title", *iterTitle).localize(), *iterTitle);
								addedAnyTitles = true;
							}
						}
					}
				}
			}

			{
				BitArray const & playerCitizenRank = playerObject->getCitizenRank();
				if (!playerCitizenRank.empty())
				{
					std::vector<CitizenRankDataTable::CitizenRank const *> const & titleableRanks = CitizenRankDataTable::getAllTitleableRanks();
					for (std::vector<CitizenRankDataTable::CitizenRank const *>::const_iterator iterRank = titleableRanks.begin(); iterRank != titleableRanks.end(); ++iterRank)
					{
						if (playerCitizenRank.testBit((*iterRank)->slotId))
						{
							for (std::vector<std::string>::const_iterator iterTitle = (*iterRank)->titles.begin(); iterTitle != (*iterRank)->titles.end(); ++iterTitle)
							{
								m_communityMediator.addItem(*dataSource, StringId("citizen_rank_title", *iterTitle).localize(), *iterTitle);
								addedAnyTitles = true;
							}
						}
					}
				}
			}

			if (addedAnyTitles)
				setComboTitleToCurrentTitle();
		}
	}
}

//-----------------------------------------------------------------------------

void SwgCuiCommunity::Character::setComboTitleToCurrentTitle()
{
	PlayerObject *playerObject = Game::getPlayerObject();
	bool titleFound = false;

	if (playerObject != NULL)
	{
		// Select the player's current skill title

		for (int index = 0; index < m_titleComboBox->GetItemCount(); ++index)
		{
			std::string name;

			if (m_titleComboBox->GetIndexName(index, name))
			{
				if (name == playerObject->getTitle())
				{
					m_titleComboBox->SetSelectedIndex(index);
					titleFound = true;
					break;
				}
			}
		}
	}

	if (!titleFound)
	{
		m_titleComboBox->SetSelectedIndex(0);
	}
}

//-----------------------------------------------------------------------------

void SwgCuiCommunity::Character::performDeactivate()
{
	setIsUpdating(false);

	m_Lfg->sendInterestsToServer();
	m_Lfg->saveOrLoadBackup(true);
	
	// Synchronize the biography to the server if it has changed

	Unicode::String const &biographyWidgetText = m_bioText->GetLocalText();

	if (m_biography != biographyWidgetText)
	{
		PlayerObject *playerObject = Game::getPlayerObject();

		if (playerObject != NULL)
		{
			playerObject->setBiography(biographyWidgetText, true);
		}
	}
}

//-----------------------------------------------------------------------------

void SwgCuiCommunity::Character::OnCheckboxSet(UIWidget *context)
{
	s_interestsChanged = true;
	s_timeToUpdateServerInterest = ::time(NULL) + s_interestUpdateDelay;

	PlayerObject *playerObject = Game::getPlayerObject();

	if (playerObject != NULL)
	{
		if (context == m_lookingForGroupCheckBox)
		{
			playerObject->toggleLookingForGroup();
		}
		else if (context == m_searchableByCtsSourceGalaxyCheckBox)
		{
			playerObject->toggleSearchableByCtsSourceGalaxy();
		}
		else if (context == m_displayLocationInSearchResultsCheckBox)
		{
			playerObject->toggleDisplayLocationInSearchResults();
		}
		else if (context == m_characterSearchableCheckBox)
		{
			playerObject->toggleAnonymous();
			m_Lfg->enableWindow();
		}
		else if (context == m_helperCheckBox)
		{
			playerObject->toggleHelper();
		}
		else if (context == m_rolePlayCheckBox)
		{
			playerObject->toggleRolePlay();
		}
		else if (context == m_awayFromKeyBoardCheckBox)
		{
			playerObject->toggleAwayFromKeyBoard();
		}
		else if (context == m_displayingFactionRankCheckBox)
		{
			playerObject->toggleDisplayingFactionRank();
		}
		else if (context == m_lookingForWorkCheckBox)
		{
			playerObject->toggleLookingForWork();
		}
	}
}

//-----------------------------------------------------------------------------

void SwgCuiCommunity::Character::OnCheckboxUnset(UIWidget *context)
{
	s_interestsChanged = true;
	s_timeToUpdateServerInterest = ::time(NULL) + s_interestUpdateDelay;

	PlayerObject *playerObject = Game::getPlayerObject();

	if (playerObject != NULL)
	{
		if (context == m_lookingForGroupCheckBox)
		{
			playerObject->toggleLookingForGroup();
		}
		else if (context == m_searchableByCtsSourceGalaxyCheckBox)
		{
			playerObject->toggleSearchableByCtsSourceGalaxy();
		}
		else if (context == m_displayLocationInSearchResultsCheckBox)
		{
			playerObject->toggleDisplayLocationInSearchResults();
		}
		else if (context == m_characterSearchableCheckBox)
		{
			playerObject->toggleAnonymous();
			m_Lfg->disableWindow();
			s_timeToUpdateServerInterest = 0;
		}
		else if (context == m_helperCheckBox)
		{
			playerObject->toggleHelper();
		}
		else if (context == m_rolePlayCheckBox)
		{
			playerObject->toggleRolePlay();
		}
		else if (context == m_awayFromKeyBoardCheckBox)
		{
			playerObject->toggleAwayFromKeyBoard();
		}
		else if (context == m_displayingFactionRankCheckBox)
		{
			playerObject->toggleDisplayingFactionRank();
		}
		else if (context == m_lookingForWorkCheckBox)
		{
			playerObject->toggleLookingForWork();
		}
	}
}

//-----------------------------------------------------------------------------

void SwgCuiCommunity::Character::onBiographyRetrieved(PlayerCreatureController::Messages::BiographyRetrieved::BiographyOwner const &biographyOwner)
{
	// Only update the local biography if this biography is for the local player

	NetworkId const &networkId = biographyOwner.first;
	PlayerObject const * const playerObject = biographyOwner.second;
	Object const *localPlayer = Game::getPlayer();

	if (   (localPlayer != NULL)
	    && (playerObject != NULL)
	    && (localPlayer->getNetworkId() == networkId))
	{
		m_bioText->SetLocalText(playerObject->getBiography());
		m_bioText->SetEditable(true);
		m_biography = playerObject->getBiography();
	}
}

//-----------------------------------------------------------------------------

void SwgCuiCommunity::Character::onWhoStatusChanged(MatchMakingId const &matchMakingId)
{
	// This function cathces any messages that the who id was changed from a source
	// other than this ui page

	m_lookingForGroupCheckBox->SetChecked(matchMakingId.isBitSet(MatchMakingId::B_lookingForGroup), false);
	m_searchableByCtsSourceGalaxyCheckBox->SetChecked(matchMakingId.isBitSet(MatchMakingId::B_searchableByCtsSourceGalaxy), false);
	m_displayLocationInSearchResultsCheckBox->SetChecked(matchMakingId.isBitSet(MatchMakingId::B_displayLocationInSearchResults), false);
	m_characterSearchableCheckBox->SetChecked(!matchMakingId.isBitSet(MatchMakingId::B_anonymous), false);
	m_helperCheckBox->SetChecked(matchMakingId.isBitSet(MatchMakingId::B_helper), false);
	m_rolePlayCheckBox->SetChecked(matchMakingId.isBitSet(MatchMakingId::B_rolePlay), false);
	m_lookingForWorkCheckBox->SetChecked(matchMakingId.isBitSet(MatchMakingId::B_lookingForWork), false);
	m_awayFromKeyBoardCheckBox->SetChecked(matchMakingId.isBitSet(MatchMakingId::B_awayFromKeyBoard), false);
	m_displayingFactionRankCheckBox->SetChecked(matchMakingId.isBitSet(MatchMakingId::B_displayingFactionRank), false);

	m_Lfg->updateBranchStateByName("general");

	if (m_characterSearchableCheckBox->IsChecked())
	{
		m_Lfg->enableWindow();
	}
	else
	{
		m_Lfg->disableWindow();
		s_timeToUpdateServerInterest = 0;
	}
}

//-----------------------------------------------------------------------------

void SwgCuiCommunity::Character::onSpokenLanguageChanged(PlayerObject::Messages::SpokenLangaugeChanged::Language const &)
{
	populateLanguages();
}

//-----------------------------------------------------------------------------

void SwgCuiCommunity::Character::OnGenericSelectionChanged(UIWidget *context)
{
	if (context == m_titleComboBox)
	{
		PlayerObject *playerObject = Game::getPlayerObject();

		if (playerObject != NULL)
		{
			std::string title;

			m_titleComboBox->GetSelectedIndexName(title);

			playerObject->setTitle(title);
		}
	}
	else if (context == m_languageComboBox)
	{
		PlayerObject *playerObject = Game::getPlayerObject();

		if (playerObject != NULL)
		{
			std::string language;

			m_languageComboBox->GetSelectedIndexName(language);

			playerObject->setSpokenLanguage(GameLanguageManager::getLanguageId(language));
		}
	}
}

//-----------------------------------------------------------------------------

void SwgCuiCommunity::Character::populateLanguages()
{
	PlayerObject *playerObject = Game::getPlayerObject();

	if (playerObject != NULL)
	{
		m_languageComboBox->Clear();

		GameLanguageManager::StringVector languages;
		GameLanguageManager::getLanguages(languages);

		GameLanguageManager::StringVector::const_iterator iterLanguages = languages.begin();

		int itemCount = 0;

		for (; iterLanguages != languages.end(); ++iterLanguages)
		{
			std::string const &language = (*iterLanguages);
			int const languageId = GameLanguageManager::getLanguageId(language);

			if (playerObject->speaksLanguage(languageId))
			{
				Unicode::String localizedLanguage;
				GameLanguageManager::getLocalizedLanguageName(languageId, localizedLanguage);

				m_languageComboBox->AddItem(localizedLanguage, language);
				++itemCount;

				if (playerObject->getSpokenLanguage() == languageId)
				{
					m_languageComboBox->SetSelectedIndex(itemCount);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------

void SwgCuiCommunity::Character::update(float deltaTimeSecs)
{
	CuiMediator::update(deltaTimeSecs);

	bool const timeForServerUpdate = (s_timeToUpdateServerInterest > 0) && (s_timeToUpdateServerInterest <= ::time(NULL));

	if (s_interestsChanged && timeForServerUpdate)
	{
		m_Lfg->sendInterestsToServer();

		s_interestsChanged = false;
		s_timeToUpdateServerInterest = ::time(NULL) + s_interestUpdateInterval;
	}
}

// ============================================================================
