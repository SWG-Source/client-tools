// ============================================================================
//
// SwgCuiCommunity_Personal.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiCommunity_Personal.h"

#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/MatchMakingManager.h"
#include "clientGame/PlayerObject.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiStringIdsCommunity.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/Crc.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedUtility/DataTable.h"
#include "UIButton.h"
#include "UICheckbox.h"
#include "UIComboBox.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIList.h"
#include "UISliderbar.h"
#include "UIText.h"

#include <cstdio>
#include <set>
#include <vector>

// ============================================================================
//
// SwgCuiCommunityPersonalNameSpace
//
// ============================================================================

namespace SwgCuiCommunityPersonalNameSpace
{
	typedef std::string                                     CategoryName;
	typedef std::string                                     TypeName;
	typedef std::set<CategoryName>                          CategoryEntries;
	typedef std::vector<std::pair<TypeName, CategoryName> > TypeEntries;

	CategoryEntries s_categoryEntries;
	TypeEntries     s_typeEntries;
	float const     s_neverUpdate = 0.0f;
	float const     s_updateEveryTwoMinutes = 60.0f * 2.0f;
	float const     s_updateEveryFiveMinutes = 60.0f * 5.0f;
	float const     s_updateEveryTenMinutes = 60.0f * 10.0f;
}

using namespace SwgCuiCommunityPersonalNameSpace;

// ============================================================================
//
// SwgCuiCommunity::Personal
//
// ============================================================================

//-----------------------------------------------------------------

SwgCuiCommunity::Personal::Personal(UIPage &page, SwgCuiCommunity const &communityPage)
 : CuiMediator("SwgCuiCommunity_Personal", page)
 , UIEventCallback()
 , m_preferenceButtonRemove(NULL)
 , m_preferenceButtonAdd(NULL)
 , m_preferenceButtonReset(NULL)
 , m_preferenceComboCategory(NULL)
 , m_preferenceComboType(NULL)
 , m_preferenceComboTime(NULL)
 , m_preferenceList(NULL)
 , m_profileButtonRemove(NULL)
 , m_profileButtonAdd(NULL)
 , m_profileButtonReset(NULL)
 , m_profileComboCategory(NULL)
 , m_profileComboType(NULL)
 , m_profileCheckSearchable(NULL)
 , m_profileTextSensitive(NULL)
 , m_profileSliderbarSensitive(NULL)
 , m_profileList(NULL)
 , m_matchMakingPersonalId()
 , m_communityMediator(communityPage)
 , m_callBack(new MessageDispatch::Callback)
{
	// Preference widgets

	getCodeDataObject(TUIButton, m_preferenceButtonRemove, "prefButtonRemove");
	registerMediatorObject(*m_preferenceButtonRemove, true);

	getCodeDataObject(TUIButton, m_preferenceButtonAdd, "prefButtonAdd");
	registerMediatorObject(*m_preferenceButtonAdd, true);

	getCodeDataObject(TUIButton, m_preferenceButtonReset, "buttonPrefReset");
	registerMediatorObject(*m_preferenceButtonReset, true);

	getCodeDataObject(TUIComboBox, m_preferenceComboCategory, "prefComboCategory");
	registerMediatorObject(*m_preferenceComboCategory, true);

	getCodeDataObject(TUIComboBox, m_preferenceComboType, "prefComboType");
	registerMediatorObject(*m_preferenceComboType, true);

	getCodeDataObject(TUIComboBox, m_preferenceComboTime, "prefCombotime");
	registerMediatorObject(*m_preferenceComboTime, true);

	getCodeDataObject(TUIList, m_preferenceList, "prefListAll");
	registerMediatorObject(*m_preferenceList, true);

	// Profile widgets

	getCodeDataObject(TUIButton, m_profileButtonRemove, "profileButtonRemove");
	registerMediatorObject(*m_profileButtonRemove, true);

	getCodeDataObject(TUIButton, m_profileButtonAdd, "profileButtonAdd");
	registerMediatorObject(*m_profileButtonAdd, true);

	getCodeDataObject(TUIButton, m_profileButtonReset, "buttonProfileReset");
	registerMediatorObject(*m_profileButtonReset, true);

	getCodeDataObject(TUIComboBox, m_profileComboCategory, "profileComboCategory");
	registerMediatorObject(*m_profileComboCategory, true);

	getCodeDataObject(TUIComboBox, m_profileComboType, "profileComboType");
	registerMediatorObject(*m_profileComboType, true);

	getCodeDataObject(TUIComboBox, m_profileComboType, "profileComboType");
	registerMediatorObject(*m_profileComboType, true);

	getCodeDataObject(TUICheckbox, m_profileCheckSearchable, "profileCheck");
	registerMediatorObject(*m_profileCheckSearchable, true);

	getCodeDataObject(TUIText, m_profileTextSensitive, "sensitivePercent");
	registerMediatorObject(*m_profileTextSensitive, true);

	getCodeDataObject(TUISliderbar, m_profileSliderbarSensitive, "sensitiveSlider");
	registerMediatorObject(*m_profileSliderbarSensitive, true);
	m_profileSliderbarSensitive->SetLowerLimit(80);
	m_profileSliderbarSensitive->SetUpperLimit(100);

	getCodeDataObject(TUIList, m_profileList, "profileListAll");
	registerMediatorObject(*m_profileList, true);

	// Load up the categories and types

	char const matchMakingFile[] = "datatables/matchmaking/matchmaking.iff";
	Iff matchMakingIff;

	if (matchMakingIff.open(matchMakingFile, true))
	{
		DataTable dataTable;
		dataTable.load(matchMakingIff);

		int const rowCount = dataTable.getNumRows();

		// Get all the categories from the data table

		for (int i = 0; i < rowCount; ++i)
		{
			int const row = i;
			std::string const &category = dataTable.getStringValue("category", row);
			std::string const &type = dataTable.getStringValue("type", row);

			s_categoryEntries.insert(category);
			s_typeEntries.push_back(std::make_pair(type, category));
		}
	}
	else
	{
		DEBUG_WARNING(true, ("Unable to load the matchmaking data table: %s", matchMakingFile));
	}

	// Set the match making id the first time the matchmaking system is opened

	PlayerObject *playerObject = Game::getPlayerObject();

	if (playerObject != NULL)
	{
		m_matchMakingPersonalId = playerObject->getMatchMakingPersonalId();

		m_profileCheckSearchable->SetChecked(!m_matchMakingPersonalId.isBitSet(MatchMakingId::B_anonymous));

		DEBUG_REPORT_LOG(ConfigClientGame::getShowMatchMakingDebug(), ("MatchMakingId: %s %s\n", m_matchMakingPersonalId.getDebugIntString().c_str(), m_matchMakingPersonalId.isBitSet(MatchMakingId::B_anonymous) ? "Anonymous" : "Searchable"));
	}
}

//-----------------------------------------------------------------

SwgCuiCommunity::Personal::~Personal()
{
	delete m_callBack;
	m_callBack = NULL;

	m_preferenceButtonRemove = NULL;
	m_preferenceButtonAdd = NULL;
	m_preferenceButtonReset = NULL;
	m_preferenceComboCategory = NULL;
	m_preferenceComboType = NULL;
	m_preferenceComboTime = NULL;
	m_preferenceList = NULL;
	m_profileButtonRemove = NULL;
	m_profileButtonAdd = NULL;
	m_profileButtonReset = NULL;
	m_profileComboCategory = NULL;
	m_profileComboType = NULL;
	m_profileCheckSearchable = NULL;
	m_profileTextSensitive = NULL;
	m_profileSliderbarSensitive = NULL;
	m_profileList = NULL;
}

//-----------------------------------------------------------------

void SwgCuiCommunity::Personal::performActivate()
{
	// Fill out the category combo boxes

	{
		m_preferenceComboCategory->Clear();
		m_profileComboCategory->Clear();

		UIDataSource *preferenceCategoryDataSource = m_preferenceComboCategory->GetDataSource();
		UIDataSource *profileCategoryDataSource = m_profileComboCategory->GetDataSource();

		if ((preferenceCategoryDataSource != NULL) && (profileCategoryDataSource != NULL))
		{
			CategoryEntries::const_iterator iterCategoryEntries = s_categoryEntries.begin();

			for (; iterCategoryEntries != s_categoryEntries.end(); ++iterCategoryEntries)
			{
				std::string const &category = (*iterCategoryEntries);
				StringId stringId("ui_cmnty", category);
				Unicode::String localizedCategory(stringId.localize());

				if ((strstr(category.c_str(), "blood_type") != NULL) ||
				    (strstr(category.c_str(), "gender") != NULL) ||
				    (strstr(category.c_str(), "zodiac") != NULL))
				{
					localizedCategory += ' ';
					localizedCategory += CuiStringIdsCommunity::one_selection_only.localize();
				}

				m_communityMediator.addItem(*preferenceCategoryDataSource, stringId.localize(), category);
				m_communityMediator.addItem(*profileCategoryDataSource, localizedCategory, category);
			}
		}

		m_preferenceComboCategory->SetSelectedIndex(0);
		m_profileComboCategory->SetSelectedIndex(0);

		rebuildTypeComboBox(*m_preferenceComboCategory);
		rebuildTypeComboBox(*m_profileComboCategory);
	}

	rebuildList(*m_preferenceList);
	rebuildList(*m_profileList);

	// Sensitivity

	m_profileSliderbarSensitive->SetValue(static_cast<int>((MatchMakingManager::getSensitivity() * 100.0f) + 0.5f), false);
	OnSliderbarChanged(m_profileSliderbarSensitive);

	// Search time

	m_preferenceComboTime->Clear();
	IGNORE_RETURN(m_preferenceComboTime->AddItem(CuiStringIdsCommunity::search_time_never.localize(), CuiStringIdsCommunity::search_time_never.getText()));
	IGNORE_RETURN(m_preferenceComboTime->AddItem(CuiStringIdsCommunity::search_time_2_minutes.localize(), CuiStringIdsCommunity::search_time_2_minutes.getText()));
	IGNORE_RETURN(m_preferenceComboTime->AddItem(CuiStringIdsCommunity::search_time_5_minutes.localize(), CuiStringIdsCommunity::search_time_5_minutes.getText()));
	IGNORE_RETURN(m_preferenceComboTime->AddItem(CuiStringIdsCommunity::search_time_10_minutes.localize(), CuiStringIdsCommunity::search_time_10_minutes.getText()));

	if (MatchMakingManager::getUpdateTime() == s_updateEveryTwoMinutes)
	{
		m_preferenceComboTime->SetSelectedIndex(1);
	}
	else if (MatchMakingManager::getUpdateTime() == s_updateEveryFiveMinutes)
	{
		m_preferenceComboTime->SetSelectedIndex(2);
	}
	else if (MatchMakingManager::getUpdateTime() == s_updateEveryTenMinutes)
	{
		m_preferenceComboTime->SetSelectedIndex(3);
	}
	else
	{
		m_preferenceComboTime->SetSelectedIndex(0);
	}
}

//-----------------------------------------------------------------

void SwgCuiCommunity::Personal::performDeactivate()
{
	m_preferenceComboCategory->Clear();
	m_preferenceComboType->Clear();
	m_preferenceComboTime->Clear();
	m_profileComboCategory->Clear();
	m_profileComboType->Clear();

	synchronizeMatchMakingPersonalId();
}

//-----------------------------------------------------------------
void SwgCuiCommunity::Personal::OnGenericSelectionChanged(UIWidget *context)
{
	NOT_NULL(m_preferenceComboCategory);
	NOT_NULL(m_preferenceComboType);
	NOT_NULL(m_profileComboCategory);
	NOT_NULL(m_profileComboType);

	if (context == m_preferenceComboCategory)
	{
		rebuildTypeComboBox(*m_preferenceComboCategory);
	}
	else if (context == m_profileComboCategory)
	{
		rebuildTypeComboBox(*m_profileComboCategory);
	}
	else if (context == m_preferenceComboTime)
	{
		float seconds = MatchMakingManager::getUpdateTime();

		switch (m_preferenceComboTime->GetSelectedIndex())
		{
			case 0:  { seconds = s_neverUpdate; } break;
			case 1:  { seconds = s_updateEveryTwoMinutes; } break;
			case 2:  { seconds = s_updateEveryFiveMinutes; } break;
			case 3:  { seconds = s_updateEveryTenMinutes; } break;
			default: { } break;
		}

		MatchMakingManager::setUpdateTime(seconds);
	}
}

//-----------------------------------------------------------------

void SwgCuiCommunity::Personal::OnButtonPressed(UIWidget *context)
{
	if (context == m_preferenceButtonRemove)
	{
		removeSelectedItemsFromList(*m_preferenceList);
	}
	else if (context == m_preferenceButtonAdd)
	{
		addItemToListFromCategoryAndType(*m_preferenceList, *m_preferenceComboCategory, *m_preferenceComboType);
	}
	else if (context == m_preferenceButtonReset)
	{
		if (m_preferenceList->GetRowCount() > 0)
		{
			CuiMessageBox *messageBox = CuiMessageBox::createYesNoBox(CuiStringIdsCommunity::clear_preference_confirmation.localize());

			m_callBack->connect(messageBox->getTransceiverClosed(), *this, &SwgCuiCommunity::Personal::onClearPreferenceList);
		}
	}
	else if (context == m_profileButtonRemove)
	{
		removeSelectedItemsFromList(*m_profileList);
	}
	else if (context == m_profileButtonAdd)
	{
		addItemToListFromCategoryAndType(*m_profileList, *m_profileComboCategory, *m_profileComboType);
	}
	else if (context == m_profileButtonReset)
	{
		if (m_profileList->GetRowCount() > 0)
		{
			CuiMessageBox *messageBox = CuiMessageBox::createYesNoBox(CuiStringIdsCommunity::clear_profile_confirmation.localize());

			m_callBack->connect(messageBox->getTransceiverClosed(), *this, &SwgCuiCommunity::Personal::onClearProfileList);
		}
	}
}

//-----------------------------------------------------------------

void SwgCuiCommunity::Personal::OnSliderbarChanged(UIWidget *context)
{
	if (context == m_profileSliderbarSensitive)
	{
		char text[256];
		sprintf(text, "%3d", m_profileSliderbarSensitive->GetValue());
		m_profileTextSensitive->SetText(Unicode::narrowToWide(text));

		float const upperLimit = static_cast<float>(m_profileSliderbarSensitive->GetUpperLimit());
		float const value = static_cast<float>(m_profileSliderbarSensitive->GetValue());
		float const percent = (upperLimit > 0.0f) ? (value / upperLimit) : upperLimit;

		MatchMakingManager::setSensitivity(percent);
	}
}

//-----------------------------------------------------------------

void SwgCuiCommunity::Personal::OnCheckboxSet(UIWidget *context)
{
	if (context == m_profileCheckSearchable)
	{
		m_matchMakingPersonalId.resetBit(MatchMakingId::B_anonymous);

		DEBUG_REPORT_LOG(ConfigClientGame::getShowMatchMakingDebug(), ("<local id> %s <player obj id> %s\n", m_matchMakingPersonalId.getDebugIntString().c_str(), (Game::getPlayerObject() != NULL) ? Game::getPlayerObject()->getMatchMakingPersonalId().getDebugIntString().c_str() : ""));
	}
}

//-----------------------------------------------------------------

void SwgCuiCommunity::Personal::OnCheckboxUnset(UIWidget *context)
{
	if (context == m_profileCheckSearchable)
	{
		m_matchMakingPersonalId.setBit(MatchMakingId::B_anonymous);

		DEBUG_REPORT_LOG(ConfigClientGame::getShowMatchMakingDebug(), ("<local id> %s <player obj id> %s\n", m_matchMakingPersonalId.getDebugIntString().c_str(), (Game::getPlayerObject() != NULL) ? Game::getPlayerObject()->getMatchMakingPersonalId().getDebugIntString().c_str() : ""));
	}
}

//-----------------------------------------------------------------

void SwgCuiCommunity::Personal::populateTypeComboBoxFromCategoryComboBox(UIComboBox &typeComboBox, UIComboBox &categoryComboBox, UIList &list) const
{
	std::string categoryName;
	categoryComboBox.GetSelectedIndexName(categoryName);

	if (!categoryName.empty())
	{
		typeComboBox.Clear();

		// Build a list of types to put in the combo box

		typedef std::map<Unicode::String, std::string> ListItem;
		ListItem types;
		TypeEntries::const_iterator iterTypeEntries = s_typeEntries.begin();

		for (; iterTypeEntries != s_typeEntries.end(); ++iterTypeEntries)
		{
			if (iterTypeEntries->second == categoryName)
			{
				std::string const &typeName = iterTypeEntries->first;

				int const bitIndex = m_communityMediator.getMatchMakingTypeIndex(typeName);

				if (!getMatchMakingId(list).isBitSet(static_cast<MatchMakingId::Bit>(bitIndex)))
				{
					std::string const &typeString = iterTypeEntries->first;

					//// Make sure the type is not already in the list
					//
					//bool alreadyInList = false;
					//
					//for (int row = 0; row < list.GetRowCount(); ++row)
					//{
					//	std::string listText;
					//
					//	list.GetText(row, listText);
					//
					//	if (strstr(listText.c_str(), typeString.c_str()) != NULL)
					//	{
					//		alreadyInList = true;
					//		break;
					//	}
					//}
					//
					//if (!alreadyInList)
					{
						StringId stringId("ui_cmnty", typeString);

						IGNORE_RETURN(types.insert(std::make_pair(stringId.localize(), typeString)));
					}
				}
			}
		}

		// Add all the types to the combo box

		if (types.empty())
		{
			IGNORE_RETURN(typeComboBox.AddItem(CuiStringIdsCommunity::no_more_selections.localize(), ""));
		}
		else
		{
			ListItem::iterator iterTypes = types.begin();

			for (; iterTypes != types.end(); ++iterTypes)
			{
				IGNORE_RETURN(typeComboBox.AddItem(iterTypes->first, iterTypes->second));
			}
		}

		typeComboBox.SetSelectedIndex(0);
	}
}

//-----------------------------------------------------------------

void SwgCuiCommunity::Personal::addItemToListFromCategoryAndType(UIList &list, UIComboBox &categoryComboBox, UIComboBox &typeComboBox)
{
	std::string typeText;

	typeComboBox.GetSelectedIndexName(typeText);

	if (!typeText.empty())
	{
		int const bitIndex = m_communityMediator.getMatchMakingTypeIndex(typeText);
		
		if (!getMatchMakingId(list).isBitSet(static_cast<MatchMakingId::Bit>(bitIndex)))
		{
			MatchMakingId matchMakingId(getMatchMakingId(list));

			// Certain categories only allow a single entry per category

			if (&categoryComboBox == m_profileComboCategory)
			{
				std::string categoryText;
				categoryComboBox.GetSelectedIndexName(categoryText);

				if (strstr(categoryText.c_str(), "blood_type") != NULL)
				{
					removeCategory(matchMakingId, "blood_type");
				}
				else if (strstr(categoryText.c_str(), "gender") != NULL)
				{
					removeCategory(matchMakingId, "gender");
				}
				else if (strstr(categoryText.c_str(), "zodiac") != NULL)
				{
					removeCategory(matchMakingId, "zodiac");
				}
			}

			// Set the new bit

			matchMakingId.setBit(static_cast<MatchMakingId::Bit>(bitIndex));
			setMatchMakingId(list, matchMakingId);

			// Rebuild the gui

			rebuildTypeComboBox(categoryComboBox);
			rebuildList(list);
			highlightItem(list, typeText);
		}
	}
}

//-----------------------------------------------------------------

void SwgCuiCommunity::Personal::removeAllItemsFromList(UIList &list)
{
	list.Clear();

	setMatchMakingId(list, MatchMakingId());

	rebuildList(list);

	if (&list == m_preferenceList)
	{
		rebuildTypeComboBox(*m_preferenceComboCategory);
	}
	else if (&list == m_profileList)
	{
		rebuildTypeComboBox(*m_profileComboCategory);
	}
}

//-----------------------------------------------------------------

void SwgCuiCommunity::Personal::removeSelectedItemsFromList(UIList &list)
{
	SwgCuiCommunity::StringVector removedItems;
	
	m_communityMediator.removeSelectedItemsFromList(list, removedItems);

	SwgCuiCommunity::StringVector::iterator iterRemovedItems = removedItems.begin();

	DEBUG_REPORT_LOG(ConfigClientGame::getShowMatchMakingDebug(), ("removeSelectedItemsFromList() - <removed items> %d\n", static_cast<int>(removedItems.size())));

	for (; iterRemovedItems != removedItems.end(); ++iterRemovedItems)
	{
		std::string const &name = (*iterRemovedItems);

		int const bitIndex = m_communityMediator.getMatchMakingTypeIndex(name);
		MatchMakingId matchMakingId(getMatchMakingId(list));
		matchMakingId.resetBit(static_cast<MatchMakingId::Bit>(bitIndex));
		setMatchMakingId(list, matchMakingId);

		DEBUG_REPORT_LOG(ConfigClientGame::getShowMatchMakingDebug(), ("removeSelectedItemsFromList() - <bit> %d <name> %s\n", bitIndex, name.c_str()));
	}

	if (&list == m_preferenceList)
	{
		rebuildTypeComboBox(*m_preferenceComboCategory);
	}
	else if (&list == m_profileList)
	{
		rebuildTypeComboBox(*m_profileComboCategory);
	}
}

//-----------------------------------------------------------------

MatchMakingId const &SwgCuiCommunity::Personal::getMatchMakingId(UIList const &list) const
{
	if (&list == m_profileList)
	{
		return m_matchMakingPersonalId;
	}

	return MatchMakingManager::getPreferenceId();
}

//-----------------------------------------------------------------

void SwgCuiCommunity::Personal::setMatchMakingId(UIList const &list, MatchMakingId const &matchMakingId)
{
	if (&list == m_profileList)
	{
		DEBUG_REPORT_LOG(ConfigClientGame::getShowMatchMakingDebug(), ("setMatchMakingId() - <curr> %s <prev> %s\n", matchMakingId.getDebugIntString().c_str(), m_matchMakingPersonalId.getDebugIntString().c_str()));

		m_matchMakingPersonalId = matchMakingId;
	}
	else if (&list == m_preferenceList)
	{
		MatchMakingManager::setPreferenceId(matchMakingId);
	}
}

//-----------------------------------------------------------------

void SwgCuiCommunity::Personal::synchronizeMatchMakingPersonalId() const
{
	// Synchronize the personal matchmaking id if it has changed

	PlayerObject *playerObject = Game::getPlayerObject();

	if (playerObject != NULL)
	{
		DEBUG_REPORT_LOG(ConfigClientGame::getShowMatchMakingDebug(), ("<local id> %s <player obj id> %s\n", m_matchMakingPersonalId.getDebugIntString().c_str(), playerObject->getMatchMakingPersonalId().getDebugIntString().c_str()));

		if (m_matchMakingPersonalId != playerObject->getMatchMakingPersonalId())
		{
			DEBUG_REPORT_LOG(ConfigClientGame::getShowMatchMakingDebug(), ("Sending matchmaking id to server %s\n", m_matchMakingPersonalId.getDebugIntString().c_str()));

			// Send the server the new match making id

			uint32 const hash = Crc::normalizeAndCalculate("setMatchMakingPersonalId");
			std::string intString;
			m_matchMakingPersonalId.packIntString(intString);

			IGNORE_RETURN(ClientCommandQueue::enqueueCommand(hash, NetworkId::cms_invalid, Unicode::String(Unicode::narrowToWide(intString))));
		}
	}
}

//-----------------------------------------------------------------
void SwgCuiCommunity::Personal::removeCategory(MatchMakingId &matchMakingId, std::string const &categoryName)
{
	TypeEntries::const_iterator iterTypeEntries = s_typeEntries.begin();

	for (; iterTypeEntries != s_typeEntries.end(); ++iterTypeEntries)
	{
		if (iterTypeEntries->second == categoryName)
		{
			int const bitIndex = m_communityMediator.getMatchMakingTypeIndex(iterTypeEntries->first);

			matchMakingId.resetBit(static_cast<MatchMakingId::Bit>(bitIndex));
		}
	}
}

//-----------------------------------------------------------------
void SwgCuiCommunity::Personal::rebuildList(UIList &list)
{
	MatchMakingId const &matchMakingId = getMatchMakingId(list);

	list.Clear();

	TypeEntries::const_iterator iterTypeEntries = s_typeEntries.begin();

	for (; iterTypeEntries != s_typeEntries.end(); ++iterTypeEntries)
	{
		std::string const &typeName = iterTypeEntries->first;

		if (matchMakingId.isBitSet(static_cast<MatchMakingId::Bit>(m_communityMediator.getMatchMakingTypeIndex(typeName))))
		{
			std::string const &categoryName = iterTypeEntries->second;

			Unicode::String itemText;
			StringId categoryStringId("ui_cmnty", categoryName);
			itemText += categoryStringId.localize();
			itemText += Unicode::narrowToWide(" -> ");
			StringId typeStringId("ui_cmnty", typeName);
			itemText += typeStringId.localize();
		
			m_communityMediator.addItemToSortedList(list, itemText, typeName);
		}
	}
}

//-----------------------------------------------------------------
void SwgCuiCommunity::Personal::rebuildTypeComboBox(UIComboBox &categoryComboBox)
{
	if (&categoryComboBox == m_preferenceComboCategory)
	{
		populateTypeComboBoxFromCategoryComboBox(*m_preferenceComboType, *m_preferenceComboCategory, *m_preferenceList);
	}
	else if (&categoryComboBox == m_profileComboCategory)
	{
		populateTypeComboBoxFromCategoryComboBox(*m_profileComboType, *m_profileComboCategory, *m_profileList);
	}
}

//-----------------------------------------------------------------
void SwgCuiCommunity::Personal::highlightItem(UIList &list, std::string const &typeName)
{
	for (int row = 0; row < list.GetRowCount(); ++row)
	{
		std::string text;
		list.GetText(row, text);

		if (text == typeName)
		{
			list.SelectRow(row);
			list.ScrollToRow(row);
			break;
		}
	}
}

//-----------------------------------------------------------------
void SwgCuiCommunity::Personal::onClearPreferenceList(const CuiMessageBox &messageBox)
{
	if (messageBox.completedAffirmative())
	{
		removeAllItemsFromList(*m_preferenceList);
	}
}

//-----------------------------------------------------------------
void SwgCuiCommunity::Personal::onClearProfileList(const CuiMessageBox &messageBox)
{
	if (messageBox.completedAffirmative())
	{
		removeAllItemsFromList(*m_profileList);
	}
}

// ============================================================================
