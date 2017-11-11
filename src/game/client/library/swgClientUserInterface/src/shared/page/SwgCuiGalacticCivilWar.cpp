//===================================================================
//
// SwgCuiGalacticCivilWar.cpp
//
// copyright 2009, sony online entertainment
//
//===================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiGalacticCivilWar.h"


#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientGame/GuildObject.h"
#include "clientUserInterface/CuiManager.h"
#include "sharedFoundation/CalendarTime.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedGame/GroundZoneManager.h"
#include "sharedMessageDispatch/Receiver.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/MapLocation.h"
#include "sharedRandom/Random.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"

#include "StringId.h"

#include "UIBaseObject.h"
#include "UIButton.h"
#include "UIColorEffector.h"
#include "UIComboBox.h"
#include "UIData.h"
#include "UIDataSourceContainer.h"
#include "UIDataSource.h"
#include "UIImageStyle.h"
#include "UIList.h"
#include "UIMessage.h"
#include "UITabbedPane.h"
#include "UIText.h"
#include "UITextbox.h"
#include "UITreeView.h"
#include "UIWidget.h"

#include <map>
#include <set>
#include <vector>

//===================================================================

namespace SwgCuiGalacticCivilWarNamespace
{
	UIScalar ms_galaxyBarMaxSize;
	UIScalar ms_serverBarMaxSize;

	namespace WinnerIconStlyes
	{
		Unicode::String const rebel = Unicode::narrowToWide("/Styles.Icon.map.gcw_trophy_rebel");
		Unicode::String const imperial = Unicode::narrowToWide("/Styles.Icon.map.gcw_trophy_imperial");
		Unicode::String const neutral = Unicode::narrowToWide("/Styles.Icon.map.gcw_trophy_neutral");
	}

	namespace PlanetIconStyles
	{
		Unicode::String const ms_gcwRebelIconStyle = Unicode::narrowToWide("/Styles.Icon.map.galactic_gcw_rebel");
		Unicode::String const ms_gcwImperialIconStyle = Unicode::narrowToWide("/Styles.Icon.map.galactic_gcw_imperial");
		Unicode::String const ms_gcwNeutralIconStyle = Unicode::narrowToWide("/Styles.Icon.map.galactic_gcw_neutral");
	}

	namespace RegionIconStyles
	{
		Unicode::String const rebel = Unicode::narrowToWide("/Styles.Icon.map.gcw_rebel");
		Unicode::String const imperial = Unicode::narrowToWide("/Styles.Icon.map.gcw_imperial");
		Unicode::String const neutral = Unicode::narrowToWide("/Styles.Icon.map.gcw_neutral");
	}

	UILowerString const ms_gcwCategoryProperty = UILowerString("GCWCategory");
	UILowerString const ms_gcwCategoryIsCategory = UILowerString("GCWCategoryRoot");

	unsigned int const ms_totalRegionButtons = 30;

	typedef stdmap<std::string, stdmap<std::string, std::pair<std::pair<float, float>, float> >::fwd >::fwd GalaxyGCWRegionMap;
	typedef stdmap<std::string, std::pair<std::pair<float, float>, float> >::fwd  PlanetGCWRegionMap;
	typedef stdmap<std::string, float>::fwd PlanetWidthMap;

	PlanetWidthMap ms_planetSizes;
	float const ms_defaultPlanetWidth = 16384.0f;

	std::set<std::string> ms_serverListMap;

	UISize ms_historyTotalSize;
	UIScalar ms_historyImpYLoc;
	UIScalar ms_historyImpYSize;

	float const ms_historyUpdateFreqInSecs = 10.0f;

	const char * ms_gcwLocationPlanets[] = { "tatooine", "talus", "naboo" };
	const NetworkId::NetworkIdType ms_gcwLocationOIDs[] = { 9835358L, 9805353L, 9865353L }; // Make sure the order corresponds with the planets list in the line above.

	int const ms_totalGcwLocations = sizeof(ms_gcwLocationPlanets) / sizeof(ms_gcwLocationPlanets[0]);

	std::pair<std::string, int> ms_lastKey;

	PlanetMapManagerClient::GcwEntriesSet ms_requestSet;
}

using namespace SwgCuiGalacticCivilWarNamespace;

SwgCuiGalacticCivilWar::SwgCuiGalacticCivilWar (UIPage& page) :
UIEventCallback(),
CuiMediator("SwgCuiGalacticCivilWar", page),
m_galaxyMap(NULL),
m_planetMap(NULL),
m_planetNamePage(NULL),
m_galaxyWideRebelBar(NULL),
m_galaxyWideImperialBar(NULL),
m_galaxyWideRebelText(NULL),
m_galaxyWideImperialText(NULL),
m_serverComboBox(NULL),
m_serverRebelBar(NULL),
m_serverImperialBar(NULL),
m_serverRebelText(NULL),
m_serverImperialText(NULL),
m_serverNameText(NULL),
m_galaxyWideWinnerIcon(NULL),
m_serverWinnerIcon(NULL),
m_planetTabComposite(NULL),
m_planetTabs(NULL),
m_mapCatalog(NULL),
m_closeButton(NULL),
m_regionButtons(),
m_regionTextLabels(),
m_markerPage(NULL),
m_categoryTree(NULL),
m_sampleText(NULL),
m_refreshButton(NULL),
m_effector(NULL),
m_callback(new MessageDispatch::Callback),
m_galaxyButtons(),
m_historyBars(),
m_historyUpdateTimer(0.0f),
m_currentHistoryCategory(),
m_lastSelectedCategory(),
m_lastSelectionIsGroupCategory(false),
m_historyLabel(NULL),
m_regionNeutralIconStyle(NULL),
m_regionRebelIconStyle(NULL),
m_regionImperialIconStyle(NULL)
{
	getCodeDataObject(TUIPage, m_galaxyMap, "GalaxyMap");
	getCodeDataObject(TUIPage, m_planetMap, "PlanetMap");
	getCodeDataObject(TUIPage, m_planetNamePage, "PlanetNames");

	getCodeDataObject(TUIPage, m_galaxyWideImperialBar, "GalaxyWideImperialBar");
	getCodeDataObject(TUIText, m_galaxyWideImperialText, "GalaxyWideImperialScore");
	getCodeDataObject(TUIPage, m_galaxyWideRebelBar, "GalaxyWideRebelBar");
	getCodeDataObject(TUIText, m_galaxyWideRebelText, "GalaxyWideRebelScore");
	getCodeDataObject(TUIButton, m_galaxyWideWinnerIcon, "GalaxyWideWinnerIcon");

	getCodeDataObject(TUIPage, m_serverImperialBar, "serverImperialBar");
	getCodeDataObject(TUIText, m_serverImperialText, "serverImperialScore");
	getCodeDataObject(TUIPage, m_serverRebelBar, "serverRebelBar");
	getCodeDataObject(TUIText, m_serverRebelText, "serverRebelScore");
	getCodeDataObject(TUIButton, m_serverWinnerIcon, "serverWinnerIcon");

	m_galaxyWideWinnerIcon->SetEnabled(false);
	m_serverWinnerIcon->SetEnabled(false);

	getCodeDataObject(TUIButton, m_gcwNeutralMarker, "gcwZoneNeutral");

	getCodeDataObject(TUITabbedPane, m_planetTabs, "PlanetTabData");
	getCodeDataObject(TUIPage, m_mapCatalog, "PlanetMapCatalog");

	getCodeDataObject(TUIComboBox, m_serverComboBox, "serverComboBox");
	getCodeDataObject(TUIText, m_serverNameText, "serverLabel");
	getCodeDataObject(TUIPage, m_markerPage, "MarkerPage");
	getCodeDataObject(TUITreeView, m_categoryTree, "categoryTree");

	getCodeDataObject(TUIText, m_sampleText, "textSample");
	getCodeDataObject(TUIButton, m_refreshButton, "refreshButton");

	getCodeDataObject(TUIColorEffector, m_effector, "effector");

	UIPage * historyPage = NULL;
	getCodeDataObject(TUIPage, historyPage, "HistoryPage");

	getCodeDataObject(TUIText, m_historyLabel, "historyLabel");

	getCodeDataObject(TUIImageStyle, m_regionNeutralIconStyle, "regionNeutral");
	getCodeDataObject(TUIImageStyle, m_regionImperialIconStyle, "regionImperial");
	getCodeDataObject(TUIImageStyle, m_regionRebelIconStyle, "regionRebel");
	
	UIBaseObject::UIObjectList historyChildren;
	historyPage->GetChildren(historyChildren);
	for(unsigned int i = 0; i < historyChildren.size(); ++i)
	{
		UIBaseObject::UIObjectList::iterator iter = historyChildren.begin();
		UIPage * firstPage = static_cast<UIPage*>((*iter));
		if(firstPage)
		{
			ms_historyTotalSize = firstPage->GetSize();
			UIPage * imperialBar = static_cast<UIPage *>(firstPage->GetChild("ImperialBar"));
			if(imperialBar)
			{
				UIPoint Loc = imperialBar->GetLocation();
				ms_historyImpYLoc = Loc.y;
				UISize const & impSize = imperialBar->GetSize();
				ms_historyImpYSize = impSize.y;
			}
		}
		for(; iter != historyChildren.end(); ++iter)
		{
			if((*iter)->IsA(TUIPage))
			{
				UIPage * rebelBar = static_cast<UIPage *>((*iter)->GetChild("RebelBar"));
				UIPage * imperialBar = static_cast<UIPage *>((*iter)->GetChild("ImperialBar"));

				NOT_NULL(rebelBar);
				NOT_NULL(imperialBar);

				rebelBar->SetVisible(false);
				imperialBar->SetVisible(false);

				m_historyBars.push_back(std::make_pair<UIPage *, UIPage *>(rebelBar, imperialBar));
			}
		}
	}


	ms_galaxyBarMaxSize = m_galaxyWideImperialBar->GetWidth();
	ms_serverBarMaxSize = m_serverImperialBar->GetWidth();

	m_categoryTree->SetContextCapable(true, true);

	m_categoryTree->ClearData();
	registerMediatorObject(*m_categoryTree, true);

	m_galaxyMap->SetVisible(false);
	m_planetNamePage->SetVisible(false);
	m_gcwNeutralMarker->SetVisible(false);

	m_markerPage->SetVisible(false);

	m_sampleText->SetVisible(false);
	
	m_closeButton = getPage().FindCancelButton(true);

	m_serverNameText->SetPreLocalized(true);
	m_historyLabel->SetPreLocalized(true);

	if(m_closeButton)
		registerMediatorObject(*m_closeButton, true);

	registerMediatorObject(*m_planetTabs, true);
	registerMediatorObject(*m_serverComboBox, true);
	registerMediatorObject(*m_refreshButton, true);

	for(unsigned int i = 0; i < ms_totalRegionButtons; ++i)
	{
		UIButton * newButton = static_cast<UIButton*>(m_gcwNeutralMarker->DuplicateObject());
		newButton->Attach(0);
		newButton->AddCallback(this);

		m_regionButtons.push_back(newButton);

		UIText * newText = static_cast<UIText*>(m_sampleText->DuplicateObject());
		newText->Attach(NULL);
		newText->SetPreLocalized(true);
		newText->AddCallback(this);

		m_regionTextLabels.push_back(newText);
	}

	m_galaxyButtons.clear();
	ms_planetSizes.clear();

	char const * const cs_PlanetWidthDataTable = "datatables/travel/planet_width.iff";
	char const * const cs_columnPlanet = "Planet";
	char const * const cs_columnWidth = "Width";

	DataTable const * const dt = DataTableManager::getTable(cs_PlanetWidthDataTable, true);
	if (dt)
	{
		std::string planet;
		float width;

		for (int row = 0; row < dt->getNumRows(); ++row)
		{
			planet = dt->getStringValue(cs_columnPlanet, row);
			width = dt->getFloatValue(cs_columnWidth, row);

			if(planet.find("kashyyyk")!= std::string::npos)
				planet = "kashyyyk";
			

			ms_planetSizes[planet] = width;

			std::string planetButton = "button" + planet;
			std::string gcwButton = "gcw" + planet;

			UIButton * planetBtn = static_cast<UIButton *>(m_galaxyMap->GetChild(planetButton.c_str()));
			UIButton * gcwBtn = static_cast<UIButton *>(m_galaxyMap->GetChild(gcwButton.c_str()));
			
			NOT_NULL(planetBtn);
			NOT_NULL(gcwBtn);

			if( planet.find("kashyyyk")!= std::string::npos || planet.find("mustafar") != std::string::npos)
			{
				gcwBtn->SetVisible(false);
				continue;
			}

			registerMediatorObject(*planetBtn, true);

			m_galaxyButtons.insert(std::make_pair<std::string, std::pair<UIButton *, UIButton*> >(planet, std::make_pair<UIButton *, UIButton*>(planetBtn, gcwBtn)));

		}

		DataTableManager::close(cs_PlanetWidthDataTable);
	}


	populateServerComboBox();

	// Auto select the first entry.
	m_serverComboBox->SetSelectedIndex(0);
	std::string serverName;
	m_serverComboBox->GetSelectedIndexName(serverName);
	m_serverNameText->SetLocalText(Unicode::narrowToWide(serverName));

	updateGalaxyWideScore();
	updateServerSpecificScore();
	updateMainPage();
	
	// List of our planets/Oids for special GCW locations
	for(int i = 0; i < ms_totalGcwLocations; ++i)
	{
		ms_requestSet.insert(std::make_pair(ms_gcwLocationPlanets[i], NetworkId(ms_gcwLocationOIDs[i])));
	}

	PlanetMapManagerClient::requestGcwEntries(ms_requestSet);
	
}

SwgCuiGalacticCivilWar::~SwgCuiGalacticCivilWar()
{
	if(m_callback)
		delete m_callback;
}

void SwgCuiGalacticCivilWar::OnButtonPressed(UIWidget * Context )
{
	if(Context == m_closeButton)
		closeNextFrame();
	else if (Context == m_refreshButton)
	{
		updateGalaxyWideScore();
		updateMainPage();
		updateHistoryGraph();
	}
	else
	{
		std::map<std::string, std::pair<UIButton *, UIButton *> >::iterator iter = m_galaxyButtons.begin();
		for(; iter != m_galaxyButtons.end(); ++iter)
		{
			if(Context == (*iter).second.first)
			{
				std::string tabName = (*iter).first;

				if(tabName.find("yavin") != std::string::npos)
					tabName = "Yavin IV";

				m_lastSelectedCategory.clear();
				m_planetTabs->SetActiveTab(tabName);
				return;
			}
		}
	}
}

void SwgCuiGalacticCivilWar::OnGenericSelectionChanged(UIWidget * context)
{
	if(context == m_serverComboBox)
	{
		std::string serverName;
		m_lastSelectedCategory.clear();
		m_serverComboBox->GetSelectedIndexName(serverName);
		m_serverNameText->SetLocalText(Unicode::narrowToWide(serverName));
		updateGalaxyWideScore();
		updateServerSpecificScore();
		updateHistoryGraph();
		m_planetTabs->SetActiveTab(0);
		updateMainPage();
		
	}
	else if (context == m_categoryTree)
	{
		int const selectedTreeRow = m_categoryTree->GetLastSelectedRow();
		UIDataSourceContainer * const dsc = m_categoryTree->GetDataSourceContainerAtRow (selectedTreeRow);

		if (dsc)
		{
			UIString selected;
			dsc->GetProperty(ms_gcwCategoryProperty, selected);
			dsc->HasProperty(ms_gcwCategoryIsCategory) ? m_lastSelectionIsGroupCategory = true : m_lastSelectionIsGroupCategory = false;
			m_lastSelectedCategory = Unicode::wideToNarrow(selected);
			updateHistoryGraph();
		}

	}
}

void SwgCuiGalacticCivilWar::update(float delta)
{
	CuiMediator::update(delta);

	m_historyUpdateTimer += delta;

	if(m_historyUpdateTimer > ms_historyUpdateFreqInSecs)
	{
		updateHistoryGraph();
		m_historyUpdateTimer = 0.0f;
	}
}

void SwgCuiGalacticCivilWar::performActivate()
{
	m_callback->connect(*this, &SwgCuiGalacticCivilWar::onGCWScoreUpdatedThisGalaxy, static_cast<GuildObject::Messages::GCWScoreUpdatedThisGalaxy *> (0));
	m_callback->connect(*this, &SwgCuiGalacticCivilWar::onGCWGroupScoreUpdatedThisGalaxy, static_cast<GuildObject::Messages::GCWGroupScoreUpdatedThisGalaxy *> (0));
	m_callback->connect(*this, &SwgCuiGalacticCivilWar::onGCWScoreUpdatedOtherGalaxies, static_cast<GuildObject::Messages::GCWScoreUpdatedOtherGalaxies *> (0));
	m_callback->connect(*this, &SwgCuiGalacticCivilWar::onGCWGroupScoreUpdatedOtherGalaxies, static_cast<GuildObject::Messages::GCWGroupScoreUpdatedOtherGalaxies *> (0));
	m_callback->connect(*this, &SwgCuiGalacticCivilWar::onGCWRegionEntriesReceived, static_cast<PlanetMapManagerClient::Messages::GcwEventLocationsReceived *> (0));
	
	PlanetMapManagerClient::requestGcwEntries(ms_requestSet);

	CuiManager::requestPointer(true);
	updateGalaxyWideScore();
	updateServerSpecificScore();
	updateMainPage();
	updateHistoryGraph();
	updateGalaxyScoreDropDownList();

	setIsUpdating(true);
}

void SwgCuiGalacticCivilWar::performDeactivate()
{
	m_callback->disconnect(*this, &SwgCuiGalacticCivilWar::onGCWScoreUpdatedThisGalaxy, static_cast<GuildObject::Messages::GCWScoreUpdatedThisGalaxy *> (0));
	m_callback->disconnect(*this, &SwgCuiGalacticCivilWar::onGCWGroupScoreUpdatedThisGalaxy, static_cast<GuildObject::Messages::GCWGroupScoreUpdatedThisGalaxy *> (0));
	m_callback->disconnect(*this, &SwgCuiGalacticCivilWar::onGCWScoreUpdatedOtherGalaxies, static_cast<GuildObject::Messages::GCWScoreUpdatedOtherGalaxies *> (0));
	m_callback->disconnect(*this, &SwgCuiGalacticCivilWar::onGCWGroupScoreUpdatedOtherGalaxies, static_cast<GuildObject::Messages::GCWGroupScoreUpdatedOtherGalaxies *> (0));
	m_callback->disconnect(*this, &SwgCuiGalacticCivilWar::onGCWRegionEntriesReceived, static_cast<PlanetMapManagerClient::Messages::GcwEventLocationsReceived *> (0));

	CuiManager::requestPointer(false);

	setIsUpdating(false);
}

void SwgCuiGalacticCivilWar::AdvanceHistoryGraph()
{
	// Skip the first one since that will almost be the most recent.
	if(!GuildObject::getGuildObject())
		return;

	std::map<std::pair<std::string, int>, int> const & thisGalaxyHistory = GuildObject::getGuildObject()->getGcwGroupImperialScorePercentileHistoryThisGalaxy();
	std::map<std::pair<std::string, int>, int>::const_iterator iter = thisGalaxyHistory.upper_bound(ms_lastKey);

	if(iter == thisGalaxyHistory.end() || _stricmp(iter->first.first.c_str(), ms_lastKey.first.c_str()))
	{
		//DEBUG_WARNING(true, ("SwgCuiGalacticCivilWar - no new update for key [%s]", ms_lastKey.first.c_str()));
		return;
	}

	unsigned int totalVisible = 0;
	for(unsigned int i = 0; i < m_historyBars.size(); ++i)
	{
		if(m_historyBars[i].first->IsVisible())
			++totalVisible;
	}
	
	UISize previousImpSize = m_historyBars[0].first->GetSize();
	UISize previousRebelSize = m_historyBars[0].second->GetSize();
	UIPoint const & oldLoc = m_historyBars[0].second->GetLocation();
	UIScalar previousImpY = oldLoc.y;
	Unicode::String previousTooltip = m_historyBars[0].first->GetLocalTooltip();
	for(unsigned int i = 0; i < totalVisible; ++i)
	{
		if(i + 1 >= m_historyBars.size())
			break;

		UISize tempSizeImp = m_historyBars[i].first->GetSize();
		UISize tempSizeReb = m_historyBars[i].second->GetSize();
		Unicode::String tempTool = m_historyBars[i].first->GetLocalTooltip();
		UIPoint const & tempLoc = m_historyBars[i].second->GetLocation();
		UIScalar tempImpY = tempLoc.y;

		m_historyBars[i + 1].first->SetVisible(true);
		m_historyBars[i + 1].second->SetVisible(true);
		
		m_historyBars[i + 1].first->SetSize(previousImpSize);
		m_historyBars[i + 1].second->SetSize(previousRebelSize);

		m_historyBars[i + 1].first->SetLocalTooltip(previousTooltip);
		m_historyBars[i + 1].second->SetLocalTooltip(previousTooltip);
		
		UIPoint const & newLoc = m_historyBars[i + 1].second->GetLocation();
		m_historyBars[i + 1].second->SetLocation(newLoc.x, previousImpY);

		previousImpSize = tempSizeImp;
		previousRebelSize = tempSizeReb;
		previousTooltip = tempTool;
		previousImpY = tempImpY;
	}

	ms_lastKey = iter->first;

	int imperialScore = iter->second;
	UIScalar impSize = static_cast<UIScalar>( static_cast<float>(ms_historyTotalSize.y) * (static_cast<float>(imperialScore) * 0.01f) );

	char buffer[32];
	sprintf(buffer, "R:%d%% I:%d%% ", 100 - imperialScore, imperialScore);

	Unicode::String toolTip = Unicode::narrowToWide(buffer) + Unicode::narrowToWide(CalendarTime::convertEpochToTimeStringLocal(iter->second));

	UISize newImpSize = ms_historyTotalSize;
	newImpSize.y = impSize;

	UISize newRebSize = ms_historyTotalSize;
	newRebSize.y -= impSize;

	m_historyBars[0].first->SetSize(newImpSize);
	m_historyBars[0].second->SetSize(newRebSize);

	m_historyBars[0].first->SetLocalTooltip(toolTip);
	m_historyBars[0].second->SetLocalTooltip(toolTip);

	UIPoint const & updatedLoc = m_historyBars[0].second->GetLocation();
	UIScalar newY = newImpSize.y - ms_historyImpYSize;
	m_historyBars[0].second->SetLocation(updatedLoc.x, ms_historyImpYLoc - newY);

}

void SwgCuiGalacticCivilWar::ResetHistoryGraph()
{
	for(unsigned int i = 0; i < m_historyBars.size(); ++i)
	{
		m_historyBars[i].first->SetVisible(false);
		m_historyBars[i].second->SetVisible(false);
	}

	m_historyLabel->SetLocalText(Unicode::narrowToWide("Score History - (Not Available)"));
}

void SwgCuiGalacticCivilWar::updateHistoryGraph()
{
	if(!GuildObject::getGuildObject())
		return;

	std::string selectedServer;
	m_serverComboBox->GetSelectedIndexName(selectedServer);

	std::string tabName;
	m_planetTabs->GetTabName(m_planetTabs->GetActiveTab(), tabName);

	if(_stricmp(tabName.c_str(), "summary") == 0)
	{
		tabName = "galaxy";
	}

	bool otherGalaxy = false;
	if(_stricmp(selectedServer.c_str(), GameNetwork::getCentralServerName().c_str()) != 0)
	{
		otherGalaxy = true;
	}

	if(!otherGalaxy)
	{
		tabName = Unicode::toLower(tabName);
		if(tabName.find("yavin") != std::string::npos)
			tabName = "yavin4";
		
		if(!m_lastSelectedCategory.empty())
			tabName = m_lastSelectedCategory;

		unsigned int currentCount = 0;

		std::map<std::pair<std::string, int>, int> const & thisGalaxyHistory = (m_lastSelectedCategory.empty() || m_lastSelectionIsGroupCategory) ? GuildObject::getGuildObject()->getGcwGroupImperialScorePercentileHistoryThisGalaxy() : GuildObject::getGuildObject()->getGcwImperialScorePercentileHistoryThisGalaxy();
		std::map<std::pair<std::string, int>, int>::const_iterator iter = thisGalaxyHistory.lower_bound(std::make_pair(tabName.c_str(), 0));
		
		if(iter == thisGalaxyHistory.end())
			return;
		
		if(!m_currentHistoryCategory.empty())
		{
			if(_stricmp(m_currentHistoryCategory.c_str(), tabName.c_str()) == 0)
			{
				AdvanceHistoryGraph();
				return;
			}
			else
				ResetHistoryGraph();
		}

		m_currentHistoryCategory = tabName;
		StringId regionString("gcw_regions", tabName);
		m_historyLabel->SetLocalText(Unicode::narrowToWide("Score History - ") + regionString.localize());

		while(iter != thisGalaxyHistory.end() && _stricmp(iter->first.first.c_str(), tabName.c_str()) == 0)
		{
			if(currentCount >= m_historyBars.size())
				return;

			ms_lastKey = iter->first;

			int imperialScore = iter->second;
			UIScalar impSize = static_cast<UIScalar>( static_cast<float>(ms_historyTotalSize.y) * (static_cast<float>(imperialScore) * 0.01f) );

			UISize newImpSize = ms_historyTotalSize;
			newImpSize.y = impSize;

			UISize newRebSize = ms_historyTotalSize;
			newRebSize.y -= impSize;

			char buffer[32];
			sprintf(buffer, "R:%d%% I:%d%% ", 100 - imperialScore, imperialScore);
			
			Unicode::String toolTip = Unicode::narrowToWide(buffer) + Unicode::narrowToWide(CalendarTime::convertEpochToTimeStringLocal(iter->first.second));

			m_historyBars[currentCount].first->SetVisible(true);
			m_historyBars[currentCount].first->SetSize(newRebSize);
			m_historyBars[currentCount].first->SetLocalTooltip(toolTip);

			m_historyBars[currentCount].second->SetVisible(true);
			m_historyBars[currentCount].second->SetSize(newImpSize);
			m_historyBars[currentCount].second->SetLocalTooltip(toolTip);
			UIPoint const & updatedLoc = m_historyBars[currentCount].second->GetLocation();
			UIScalar newY = newImpSize.y - ms_historyImpYSize;
			m_historyBars[currentCount].second->SetLocation(updatedLoc.x, ms_historyImpYLoc - newY);

			++currentCount;

			++iter;
		}
	}
	else
	{
		ResetHistoryGraph();
	}
}

void SwgCuiGalacticCivilWar::updateGalaxyScoreDropDownList()
{
	GuildObject const * const go = GuildObject::getGuildObject();
	if (!go)
		return;

	std::string galaxyName;
	bool foundGalaxyScore;
	int galaxyImpScore;
	char buffer[256];
	int const currentSelection = static_cast<int>(m_serverComboBox->GetSelectedIndex());
	int const count = m_serverComboBox->GetItemCount();
	for (int i = 0; i < count; ++i)
	{
		if (m_serverComboBox->GetIndexName(i, galaxyName))
		{
			foundGalaxyScore = false;
			galaxyImpScore = 50;
			if (GameNetwork::getCentralServerName() == galaxyName)
			{
				std::map<std::string, int> const & gcwScoreMap = go->getGcwGroupImperialScorePercentileThisGalaxy();
				std::map<std::string, int>::const_iterator const iterFind = gcwScoreMap.find("galaxy");
				if (iterFind != gcwScoreMap.end())
				{
					foundGalaxyScore = true;
					galaxyImpScore = iterFind->second;
				}
			}
			else
			{
				std::map<std::pair<std::string, std::string>, int> const & gcwScoreMap = go->getGcwGroupImperialScorePercentileOtherGalaxies();
				std::map<std::pair<std::string, std::string>, int>::const_iterator const iterFind = gcwScoreMap.find(std::make_pair(galaxyName, "galaxy"));
				if (iterFind != gcwScoreMap.end())
				{
					foundGalaxyScore = true;
					galaxyImpScore = iterFind->second;
				}
			}

			if (foundGalaxyScore)
			{
				UIData * const data = m_serverComboBox->GetDataAtIndex(static_cast<long>(i));
				if (data)
				{
					snprintf(buffer, sizeof(buffer)-1, "%s (R:%d%% I:%d%%)", galaxyName.c_str(), (100 - galaxyImpScore), galaxyImpScore);
					buffer[sizeof(buffer)-1] = '\0';

					data->SetProperty(UIList::DataProperties::LOCALTEXT, Unicode::narrowToWide(buffer));

					if (i == currentSelection)
					{
						UITextbox * textboxValue = static_cast<UITextbox *> (m_serverComboBox->GetChild("ComboTextbox"));
						if (textboxValue)
							textboxValue->SetText(Unicode::narrowToWide(buffer));
					}
				}
			}
		}
	}
}

void SwgCuiGalacticCivilWar::updateGalaxyWideScore()
{
	if(!GuildObject::getGuildObject())
		return;

	int ImpScore = 50;

	std::map<std::pair<std::string, std::string>, int> const & otherGalaxiesMap = GuildObject::getGuildObject()->getGcwGroupImperialScorePercentileOtherGalaxies();

	std::map<std::pair<std::string, std::string>, int>::const_iterator SWGGalaxy = otherGalaxiesMap.find(std::make_pair<std::string, std::string>("SWG","galaxy"));
	if(SWGGalaxy == otherGalaxiesMap.end())
	{
		DEBUG_WARNING(true, ("SwgCuiGalacticCivilWar - Failed to find 'SWG' Galaxy"));
		
		std::map<std::string, int> const & thisGalaxy = GuildObject::getGuildObject()->getGcwGroupImperialScorePercentileThisGalaxy();
		std::map<std::string, int>::const_iterator findIter = thisGalaxy.find("galaxy");
		if(findIter == thisGalaxy.end())
		{
			DEBUG_WARNING(true, ("SwgCuiGalacticCivilWar - Failed to find own Galaxy score with key [galaxy]"));
		}
		else
			ImpScore = (*findIter).second;
	}
	else
		ImpScore = (*SWGGalaxy).second;
	
	int RebScore = 100 - ImpScore;

	float ImpPercentage = static_cast<float>(ms_galaxyBarMaxSize) * (static_cast<float>(ImpScore) * 0.01f);
	UIScalar RebPercentage = ms_galaxyBarMaxSize - static_cast<UIScalar>(ImpPercentage);

	m_galaxyWideImperialBar->SetWidth(static_cast<UIScalar>(ImpPercentage));
	m_galaxyWideRebelBar->SetWidth(RebPercentage);

	char buffer[32];
	memset(buffer, 0, 32);
	sprintf(buffer, "%d%%", ImpScore);

	m_galaxyWideImperialText->SetLocalText(Unicode::narrowToWide(buffer));

	sprintf(buffer, "%d%%", RebScore);

	m_galaxyWideRebelText->SetLocalText(Unicode::narrowToWide(buffer));

	if(ImpScore > RebScore)
		m_galaxyWideWinnerIcon->SetProperty(UIButton::PropertyName::Icon, WinnerIconStlyes::imperial);
	else if (RebScore > ImpScore)
		m_galaxyWideWinnerIcon->SetProperty(UIButton::PropertyName::Icon, WinnerIconStlyes::rebel);
	else
		m_galaxyWideWinnerIcon->SetProperty(UIButton::PropertyName::Icon, WinnerIconStlyes::neutral);


}

void SwgCuiGalacticCivilWar::updateServerSpecificScore()
{
	std::string selectedServer;
	m_serverComboBox->GetSelectedIndexName(selectedServer);
	

	bool otherGalaxy = false;
	if(_stricmp(selectedServer.c_str(), GameNetwork::getCentralServerName().c_str()) != 0)
	{
		otherGalaxy = true;
	}
	int ImpScore = 50;

	if(otherGalaxy)
	{
		std::map<std::pair<std::string, std::string>, int> const & otherGalaxiesMap = GuildObject::getGuildObject()->getGcwGroupImperialScorePercentileOtherGalaxies();

		std::map<std::pair<std::string, std::string>, int>::const_iterator OtherGalaxy = otherGalaxiesMap.find(std::make_pair<std::string, std::string>(selectedServer, "galaxy"));
		if(OtherGalaxy == otherGalaxiesMap.end())
		{
			DEBUG_WARNING(true, ("SwgCuiGalacticCivilWar - Failed to find [%s] Galaxy score with key [galaxy]", selectedServer.c_str()));
			return;
		}

		ImpScore = (*OtherGalaxy).second;
	}
	else
	{
		std::map<std::string, int> const & thisGalaxy = GuildObject::getGuildObject()->getGcwGroupImperialScorePercentileThisGalaxy();
		std::map<std::string, int>::const_iterator findIter = thisGalaxy.find("galaxy");
		if(findIter == thisGalaxy.end())
		{
			DEBUG_WARNING(true, ("SwgCuiGalacticCivilWar - Failed to find own Galaxy score with key [galaxy]"));
			return;
		}

		ImpScore = (*findIter).second;
	}

	int RebScore = 100 - ImpScore;

	float ImpPercentage = static_cast<float>(ms_serverBarMaxSize) * (static_cast<float>(ImpScore) * 0.01f);
	UIScalar RebPercentage = ms_serverBarMaxSize - static_cast<UIScalar>(ImpPercentage);

	m_serverImperialBar->SetWidth(static_cast<UIScalar>(ImpPercentage));
	m_serverRebelBar->SetWidth(RebPercentage);

	char buffer[32];
	memset(buffer, 0, 32);
	sprintf(buffer, "%d%%", ImpScore);

	m_serverImperialText->SetLocalText(Unicode::narrowToWide(buffer));

	sprintf(buffer, "%d%%", RebScore);

	m_serverRebelText->SetLocalText(Unicode::narrowToWide(buffer));

	if(ImpScore > RebScore)
		m_serverWinnerIcon->SetProperty(UIButton::PropertyName::Icon, WinnerIconStlyes::imperial);
	else if (RebScore > ImpScore)
		m_serverWinnerIcon->SetProperty(UIButton::PropertyName::Icon, WinnerIconStlyes::rebel);
	else
		m_serverWinnerIcon->SetProperty(UIButton::PropertyName::Icon, WinnerIconStlyes::neutral);
}

void SwgCuiGalacticCivilWar::updateMainPage()
{
	std::string tabName;

	m_planetTabs->GetTabName(m_planetTabs->GetActiveTab(), tabName);

	if(_stricmp(tabName.c_str(), "summary") == 0)
	{
		updateGalaxyMapIcons();
		m_galaxyMap->SetVisible(true);
		m_planetNamePage->SetVisible(true);
		m_planetMap->SetVisible(false);
		for(unsigned int i = 0; i < ms_totalRegionButtons; ++i)
		{
			UIButton * newButton = m_regionButtons[i];
			newButton->SetVisible(false);
		}
		m_markerPage->SetVisible(false);
		m_refreshButton->SetVisible(true);
		m_refreshButton->CancelEffector(*m_effector);

		m_markerPage->Clear();

	}
	else
	{
		// Fix up Yavin's name.
		if(tabName.find("Yavin") != std::string::npos)
			tabName = "yavin4";

		m_galaxyMap->SetVisible(false);
		m_planetNamePage->SetVisible(false);
		m_planetMap->SetVisible(true);
		m_markerPage->SetVisible(true);
		m_refreshButton->SetVisible(false);
		m_refreshButton->CancelEffector(*m_effector);

		m_markerPage->Clear();

		updatePlanetRegions(tabName);

	}
	updateCategoryTree(tabName);

}

void SwgCuiGalacticCivilWar::updatePlanetRegions(std::string & planetName)
{
	UIWidget * widget = dynamic_cast<UIWidget *>(m_mapCatalog->GetChild (planetName.c_str ()));
	if (!widget)
	{
		widget = dynamic_cast<UIWidget *>(m_mapCatalog->GetChild ("default"));
		WARNING (true, ("could not find planet map for [%s], using default", planetName.c_str ()));
	}
	NOT_NULL (widget);

	const UIBaseObject::UIObjectList & olist = m_mapCatalog->GetChildrenRef ();
	for (UIBaseObject::UIObjectList::const_iterator it = olist.begin (); it != olist.end (); ++it)
	{
		UIBaseObject * const obj = *it;
		if (obj->IsA (TUIWidget))
		{
			UIWidget * const child = safe_cast<UIWidget *>(obj);
			child->SetVisible (child == widget);
		}
	}

	UISize const & mapSize = widget->GetSize();

	if(!GuildObject::getGuildObject())
		return;

	GalaxyGCWRegionMap const & regionMap = GameNetwork::getGcwScoreCategoryRegions();

	planetName = Unicode::toLower(planetName);

	GalaxyGCWRegionMap::const_iterator planetRegions = regionMap.find(planetName);
	if(planetRegions == regionMap.end())
	{
		DEBUG_WARNING(true, ("SwgCuiGalacticCivilWar: Failed to find GCW Regions for planet[%s]", planetName.c_str()));
		return;
	}

	PlanetGCWRegionMap const & planetRegionList = (*planetRegions).second;

	m_markerPage->Clear();

	std::string selectedServer;
	m_serverComboBox->GetSelectedIndexName(selectedServer);
	bool otherGalaxy = false;
	if(_stricmp(selectedServer.c_str(), GameNetwork::getCentralServerName().c_str()) != 0)
	{
		otherGalaxy = true;
	}

	unsigned int buttonCounter = 0;
	unsigned int textCounter = 0;
	PlanetGCWRegionMap::const_iterator iter = planetRegionList.begin();
	for(; iter != planetRegionList.end(); ++iter)
	{
		UIButton * marker = NULL;
		UIText * text = NULL;

		if(buttonCounter >= m_regionButtons.size()) // We ran out of buttons! Make a new one.
		{
			marker = static_cast<UIButton *>(m_gcwNeutralMarker->DuplicateObject());
			marker->Attach(0);
			marker->AddCallback(this);
			m_regionButtons.push_back(marker);
		}
		else
		{
			marker = m_regionButtons[buttonCounter];
		}

		if(textCounter >= m_regionTextLabels.size())
		{
			text = static_cast<UIText*>(m_sampleText->DuplicateObject());
			text->Attach(NULL);
			text->SetPreLocalized(true);
			text->AddCallback(this);
			m_regionTextLabels.push_back(text);
		}
		else
			text = m_regionTextLabels[textCounter];

		if(!marker || !text)
			continue;

		++buttonCounter;
		++textCounter;

		// Get region information.
		std::string name = (*iter).first;
		float x    = (*iter).second.first.first;
		float z    = (*iter).second.first.second;
		float size = (*iter).second.second;

		//Get region score.
		int imperialScore = 50;

		if(!otherGalaxy)
		{
			std::map<std::string, int> const & GCWScores = GuildObject::getGuildObject()->getGcwImperialScorePercentileThisGalaxy();
			std::map<std::string, int>::const_iterator scoreIter = GCWScores.find(name);
			if(scoreIter != GCWScores.end())
			{
				imperialScore = scoreIter->second;
			}
			else
				DEBUG_WARNING(true, ("SwgCuiGalacticCivilWar: Could not find a GCW Score for key [%s]", name.c_str()));
		}
		else
		{
			std::map<std::pair<std::string, std::string>, int> const & groupMap = GuildObject::getGuildObject()->getGcwImperialScorePercentileOtherGalaxies();
			std::map<std::pair<std::string, std::string>, int>::const_iterator groupIter = groupMap.find(std::make_pair(selectedServer.c_str(), name));
			if(groupIter != groupMap.end())
			{
				imperialScore = groupIter->second;
			}
			else
				DEBUG_WARNING(true, ("SwgCuiGalacticCivilWar: Could not find a GCW Score for server [%s] key [%s]", selectedServer.c_str(), name.c_str()));
		}

		int rebelScore = 100 - imperialScore;

		if(imperialScore > rebelScore)
			marker->SetIcon(m_regionImperialIconStyle);
		else if (rebelScore > imperialScore)
			marker->SetIcon(m_regionRebelIconStyle);
		else // tied
			marker->SetIcon(m_regionNeutralIconStyle);

		StringId regionString("gcw_regions", name);

		int displayScore = imperialScore > rebelScore ? imperialScore : rebelScore;

		char buffer[256];
		memset(buffer, 0, 256);
		sprintf(buffer, "%s\n%d%%", Unicode::wideToNarrow(regionString.localize()).c_str(), displayScore);

		text->SetVisible(true);
		text->SetLocalText(Unicode::narrowToWide(buffer));
		text->SizeToContent();
		text->SetTextFlag(UIText::TF_drawLast, true);

		// Now figure out placing and size on the map.
		float planetSize = 0.0f;
		PlanetWidthMap::const_iterator planetWidthIter = ms_planetSizes.find(planetName);
		if(planetWidthIter == ms_planetSizes.end())
		{
			DEBUG_WARNING(true, ("SwgCuiGalacticCivilWar: Could not find planet width for key [%s], using default size", planetName.c_str()));
			planetSize = ms_defaultPlanetWidth;
		}
		else
			planetSize = planetWidthIter->second;

		float halfPlanetX = planetSize * 0.5f;
		float halfPlanetZ = planetSize * 0.5f;

		float regionXPercentage = size / halfPlanetX;
		float regionZPercentage = size / halfPlanetZ;

		// Region size
		UIScalar buttonSizeX = static_cast<UIScalar>(static_cast<float>(mapSize.x) * regionXPercentage);
		UIScalar buttonSizeZ = static_cast<UIScalar>(static_cast<float>(mapSize.y) * regionZPercentage);

		marker->SetWidth(buttonSizeX);
		marker->SetHeight(buttonSizeZ);
		
		// Location.
		UIPoint pt;
		Vector location = GroundZoneManager::transformWorldLocationToZoneLocation(planetName, Vector(x, 0, z));
		Vector const & v = location + Vector (planetSize * 0.5f, 0.0f, planetSize * 0.5f);
		pt.x = static_cast<long>(static_cast<float>(mapSize.x) * (v.x / planetSize));
		pt.y = mapSize.y - (static_cast<long>(static_cast<float>(mapSize.y) * (v.z / planetSize)));

		text->SetLocation(pt.x - text->GetWidth() / 2L, pt.y - text->GetHeight()/2L);
		
		pt.x -= buttonSizeX / 2L;
		pt.y -= buttonSizeZ / 2L;

		marker->SetLocation(pt.x, pt.y);
		marker->SetVisible(true);
		m_markerPage->AddChild(marker);
		
		m_markerPage->AddChild(text);

		marker->SetContextCapable(true, true);
		marker->Link();
		text->SetContextCapable(true, true);
		text->Link();

		//marker->Detach(0);
	}

	if(!otherGalaxy)
	{
		// Plot the special GCW Location entry for this planet, if it has one.
		// This is basically a copy of all the logic above, I couldn't think of a good way to add the final
		// entry in, and really didn't want to rework all of this into a method (or series of methods).
		MapLocation gcwLocation;
		if(PlanetMapManagerClient::getGcwEntryForPlanet(planetName, gcwLocation))
		{
			UIText * text = NULL;

			if(textCounter >= m_regionTextLabels.size())
			{
				text = static_cast<UIText*>(m_sampleText->DuplicateObject());
				text->Attach(NULL);
				text->SetPreLocalized(true);
				text->AddCallback(this);
				m_regionTextLabels.push_back(text);
			}
			else
				text = m_regionTextLabels[textCounter];

			++textCounter;

			text->SetVisible(true);
			text->SetLocalText(gcwLocation.getLocationName());
			text->SizeToContent();
			text->SetTextFlag(UIText::TF_drawLast, true);

			float x    = gcwLocation.getLocation().x;
			float z    = gcwLocation.getLocation().y;
			float size = 2000.0f;
			// Now figure out placing and size on the map.
			float planetSize = 0.0f;
			PlanetWidthMap::const_iterator planetWidthIter = ms_planetSizes.find(planetName);
			if(planetWidthIter == ms_planetSizes.end())
			{
				DEBUG_WARNING(true, ("SwgCuiGalacticCivilWar: Could not find planet width for key [%s], using default size", planetName.c_str()));
				planetSize = ms_defaultPlanetWidth;
			}
			else
				planetSize = planetWidthIter->second;

			float halfPlanetX = planetSize * 0.5f;
			float halfPlanetZ = planetSize * 0.5f;

			float regionXPercentage = size / halfPlanetX;
			float regionZPercentage = size / halfPlanetZ;

			// Region size
			UIScalar buttonSizeX = static_cast<UIScalar>(static_cast<float>(mapSize.x) * regionXPercentage);
			UIScalar buttonSizeZ = static_cast<UIScalar>(static_cast<float>(mapSize.y) * regionZPercentage);

			// Location.
			UIPoint pt;
			Vector location = GroundZoneManager::transformWorldLocationToZoneLocation(planetName, Vector(x, 0, z));
			Vector const & v = location + Vector (planetSize * 0.5f, 0.0f, planetSize * 0.5f);
			pt.x = static_cast<long>(static_cast<float>(mapSize.x) * (v.x / planetSize));
			pt.y = mapSize.y - (static_cast<long>(static_cast<float>(mapSize.y) * (v.z / planetSize)));

			text->SetLocation(pt.x - text->GetWidth() / 2L, pt.y - (text->GetHeight() + text->GetHeight()/4L));

			pt.x -= buttonSizeX / 2L;
			pt.y -= buttonSizeZ / 2L;

			m_markerPage->AddChild(text);

			text->SetContextCapable(true, true);
			text->Link();
		}
	}
	
	if(buttonCounter < m_regionButtons.size())
	{
		ButtonVector::iterator hideIter = &m_regionButtons[buttonCounter];
		for(; hideIter != m_regionButtons.end(); ++hideIter)
		{
			(*hideIter)->SetVisible(false);
		}
	}

	if(textCounter < m_regionTextLabels.size())
	{
		TextVector::iterator hideIter = &m_regionTextLabels[textCounter];
		for(; hideIter != m_regionTextLabels.end(); ++hideIter)
			(*hideIter)->SetVisible(false);
	}

}

void SwgCuiGalacticCivilWar::updateCategoryTree(std::string & rootCategory)
{
	UNREF(rootCategory);

	m_categoryTree->ClearData();
	bool summaryScreen = false;

	if(_stricmp(rootCategory.c_str(), "summary") == 0)
		summaryScreen = true;

	if(!GuildObject::getGuildObject())
		return;

	UIDataSourceContainer * const dsc = m_categoryTree->GetDataSourceContainer ();
	dsc->Clear ();

	std::string selectedServer;
	m_serverComboBox->GetSelectedIndexName(selectedServer);
	bool otherGalaxy = false;
	if(_stricmp(selectedServer.c_str(), GameNetwork::getCentralServerName().c_str()) != 0)
	{
		otherGalaxy = true;
	}

	if(summaryScreen)
	{
		std::map<std::string, std::map<std::string, int> > const & categoryMap = GameNetwork::getGcwScoreCategoryGroups();
		std::map<std::string, std::map<std::string, int> >::const_iterator categoryIter = categoryMap.begin();
		for(; categoryIter != categoryMap.end(); ++categoryIter)
		{
			UIDataSourceContainer * const dsc_category = new UIDataSourceContainer;
			dsc_category->SetName            ((*categoryIter).first);
			StringId catName("gcw_regions", (*categoryIter).first);
			
			int groupImpScore = 50;

			if(otherGalaxy)
			{
				std::map<std::pair<std::string, std::string>, int> const & groupMap = GuildObject::getGuildObject()->getGcwGroupImperialScorePercentileOtherGalaxies();
				std::map<std::pair<std::string, std::string>, int>::const_iterator groupIter = groupMap.find(std::make_pair(selectedServer.c_str(),(*categoryIter).first));
				if(groupIter != groupMap.end())
					groupImpScore = groupIter->second;
			}
			else
			{
				std::map<std::string, int> const & groupMap = GuildObject::getGuildObject()->getGcwGroupImperialScorePercentileThisGalaxy();
				std::map<std::string, int>::const_iterator groupIter = groupMap.find((*categoryIter).first);
				if(groupIter != groupMap.end())
					groupImpScore = groupIter->second;
			}


			int groupRebScore = 100 - groupImpScore;

			char groupBuffer[32];
			memset(groupBuffer, 0, 32);
			sprintf(groupBuffer, " R:%d%% I:%d%%", groupRebScore, groupImpScore);

			dsc_category->SetProperty        (UITreeView::DataProperties::LocalText, catName.localize() + Unicode::narrowToWide(groupBuffer));
			dsc_category->SetProperty		 (ms_gcwCategoryProperty, Unicode::narrowToWide((*categoryIter).first));
			dsc_category->SetProperty        (ms_gcwCategoryIsCategory, Unicode::narrowToWide("yes"));

			std::map<std::string, std::pair<Unicode::String, Unicode::String> > childMap;

			std::map<std::string, int>::const_iterator iter = (*categoryIter).second.begin();
			for(; iter != (*categoryIter).second.end(); ++iter)
			{
				std::string entryString;
				int ImperialScore = 50;
				int RebelScore = 100 - ImperialScore;

				if(otherGalaxy)
				{
					std::map<std::pair<std::string, std::string>, int> const & scoreMap = GuildObject::getGuildObject()->getGcwImperialScorePercentileOtherGalaxies();
					
					std::map<std::pair<std::string, std::string>, int>::const_iterator scoreIter = scoreMap.find(std::make_pair(selectedServer, iter->first));

					if(scoreIter != scoreMap.end())
					{
						ImperialScore = (*scoreIter).second;
						RebelScore = 100 - ImperialScore;
						entryString = iter->first;
					}
					else
						DEBUG_WARNING(true, ("SwgCuiGalacticCivilWar - Failed to find a score value for key [%s] on galaxy [%s]", (*iter).first.c_str(), selectedServer.c_str()));
				}
				else
				{
					std::map<std::string, int> const & scoreMap = GuildObject::getGuildObject()->getGcwImperialScorePercentileThisGalaxy();

					std::map<std::string, int>::const_iterator scoreIter = scoreMap.find((*iter).first);

					if(scoreIter != scoreMap.end())
					{
						ImperialScore = (*scoreIter).second;
						RebelScore = 100 - ImperialScore;
						entryString = iter->first;
					}
					else
						DEBUG_WARNING(true, ("SwgCuiGalacticCivilWar - Failed to find a score value for key [%s]", (*iter).first.c_str()));
				}
				
				if(entryString.empty())
					continue;

				StringId entryName("gcw_regions", entryString);

				char buffer[32];
				memset(buffer, 0, 32);
				sprintf(buffer, " R:%d%% I:%d%%", RebelScore, ImperialScore);

				// Key = Localized Name
				// Value Pair = Localized Name with Score, Category(internal) name;
				childMap.insert(std::make_pair(Unicode::wideToNarrow(entryName.localize()), std::make_pair(entryName.localize() + Unicode::narrowToWide(buffer), Unicode::narrowToWide(entryString))));
			}

			std::map<std::string, std::pair<Unicode::String, Unicode::String> >::const_iterator childIter = childMap.begin();
			for(; childIter != childMap.end(); ++childIter)
			{
				UIDataSourceContainer * const dsc_item = new UIDataSourceContainer;
				dsc_item->SetName            (childIter->first);
				dsc_item->SetProperty        (UITreeView::DataProperties::LocalText, childIter->second.first);
				dsc_item->SetProperty		 (ms_gcwCategoryProperty, childIter->second.second);
				dsc_category->AddChild(dsc_item);
			}

			dsc->AddChild (dsc_category);
		}
	}
	else
	{
		rootCategory = Unicode::toLower(rootCategory);
		std::map<std::string, std::map<std::string, int> > const & categoryMap = GameNetwork::getGcwScoreCategoryGroups();
		std::map<std::string, std::map<std::string, int> >::const_iterator categoryIter = categoryMap.find(rootCategory);
		if(categoryIter != categoryMap.end())
		{
			std::map<std::string, std::pair<Unicode::String, Unicode::String> > childMap;

			std::map<std::string, int>::const_iterator iter = (*categoryIter).second.begin();
			for(; iter != (*categoryIter).second.end(); ++iter)
			{
				std::string const name = (*iter).first;
				int ImperialScore = 50;
				if(!otherGalaxy)
				{
					std::map<std::string, int> const & GCWScores = GuildObject::getGuildObject()->getGcwImperialScorePercentileThisGalaxy();
					std::map<std::string, int>::const_iterator scoreIter = GCWScores.find(name);
					if(scoreIter != GCWScores.end())
					{
						ImperialScore = scoreIter->second;
					}
					else
						DEBUG_WARNING(true, ("SwgCuiGalacticCivilWar: Could not find a GCW Score for key [%s]", name.c_str()));
				}
				else
				{
					std::map<std::pair<std::string, std::string>, int> const & groupMap = GuildObject::getGuildObject()->getGcwImperialScorePercentileOtherGalaxies();
					std::map<std::pair<std::string, std::string>, int>::const_iterator groupIter = groupMap.find(std::make_pair(selectedServer.c_str(), name));
					if(groupIter != groupMap.end())
					{
						ImperialScore = groupIter->second;
					}
					else
						DEBUG_WARNING(true, ("SwgCuiGalacticCivilWar: Could not find a GCW Score for server [%s] key [%s]", selectedServer.c_str(), name.c_str()));
				}

				int RebelScore = 100 - ImperialScore;

				StringId entryName("gcw_regions", name);

				char buffer[32];
				memset(buffer, 0, 32);
				sprintf(buffer, " R:%d%% I:%d%%", RebelScore, ImperialScore);

				// Key = Localized Name
				// Value Pair = Localized Name with Score, Category(internal) name
				childMap.insert(std::make_pair(Unicode::wideToNarrow(entryName.localize()), std::make_pair(entryName.localize() + Unicode::narrowToWide(buffer), Unicode::narrowToWide(iter->first))));
			}

			std::map<std::string, std::pair<Unicode::String, Unicode::String> >::const_iterator childIter = childMap.begin();
			for(; childIter != childMap.end(); ++childIter)
			{
				UIDataSourceContainer * const dsc_item = new UIDataSourceContainer;
				dsc_item->SetName            (childIter->first);
				dsc_item->SetProperty        (UITreeView::DataProperties::LocalText, childIter->second.first);
				dsc_item->SetProperty		 (ms_gcwCategoryProperty, childIter->second.second);
				dsc->AddChild(dsc_item);
			}
		}
		else
			DEBUG_WARNING(true, ("SwgCuiGalacticCivilWar - Could not find categories for key [%s]", rootCategory.c_str()));



	}


	m_categoryTree->SetDataSourceContainer (dsc);

}

void SwgCuiGalacticCivilWar::OnTabbedPaneChanged(UIWidget * Context)
{
	UNREF(Context);
	m_lastSelectedCategory.clear();
	updateMainPage();
	updateHistoryGraph();
}

void SwgCuiGalacticCivilWar::populateServerComboBox()
{
	if(!GuildObject::getGuildObject())
		return;

	m_serverComboBox->Clear();

	// Our galaxy
	if(!GameNetwork::getCentralServerName().empty())
	{
		m_serverComboBox->AddItem(Unicode::narrowToWide(GameNetwork::getCentralServerName()), GameNetwork::getCentralServerName());
	}

	// Other galaxies
	static std::string const minGroupName("\x00");
	static std::string const maxGroupName("\xFF");
	std::map<std::pair<std::string, std::string>, int> const & otherGalaxies = GuildObject::getGuildObject()->getGcwImperialScorePercentileOtherGalaxies();

	// The SWG galaxy, if available
	static std::string const swg("SWG");
	std::map<std::pair<std::string, std::string>, int>::const_iterator const iterFindSWG = otherGalaxies.lower_bound(std::make_pair(swg, minGroupName));
	if ((iterFindSWG != otherGalaxies.end()) && (iterFindSWG->first.first == swg))
		m_serverComboBox->AddItem(Unicode::narrowToWide(swg), swg);

	// Populate other servers name.
	// Using an optimized method Huy suggested here...
	std::map<std::pair<std::string, std::string>, int>::const_iterator iter = otherGalaxies.begin();
	for(; iter != otherGalaxies.end(); )
	{
		std::string serverName = (*iter).first.first;
		
		if(_stricmp(serverName.c_str(), swg.c_str()) != 0)
		{	
			ms_serverListMap.insert(serverName);
			m_serverComboBox->AddItem(Unicode::narrowToWide(serverName), serverName);
		}

		iter = otherGalaxies.lower_bound(std::make_pair<std::string, std::string>(serverName, maxGroupName));
	}
}

void SwgCuiGalacticCivilWar::onGCWScoreUpdatedThisGalaxy(GuildObject::Messages::GCWScoreUpdatedThisGalaxy::Payload const & obj)
{
	std::string currentServer;
	m_serverComboBox->GetSelectedIndexName(currentServer);

	if(_stricmp(currentServer.c_str(), GameNetwork::getCentralServerName().c_str()) == 0)
	{
		updateServerSpecificScore();
		updateGalaxyWideScore();

		std::string tabName;
		m_planetTabs->GetTabName(m_planetTabs->GetActiveTab(), tabName);

		if(tabName.find("Yavin")!= std::string::npos)
			tabName = "yavin4";
		
		if(obj.first.find(tabName)!= std::string::npos)
		{
			updatePlanetRegions(tabName);
			updateHistoryGraph();
		}		
	}
}

void SwgCuiGalacticCivilWar::onGCWGroupScoreUpdatedThisGalaxy(GuildObject::Messages::GCWGroupScoreUpdatedThisGalaxy::Payload const & obj)
{
	UNREF(obj);

	std::string currentServer;
	m_serverComboBox->GetSelectedIndexName(currentServer);

	if(_stricmp(currentServer.c_str(), GameNetwork::getCentralServerName().c_str()) == 0)
	{
		updateServerSpecificScore();
		updateGalaxyWideScore();

		std::string tabName;
		m_planetTabs->GetTabName(m_planetTabs->GetActiveTab(), tabName);

		if(tabName.find("Yavin")!= std::string::npos)
			tabName = "yavin4";

		if(_stricmp(tabName.c_str(), "summary") == 0)
		{
			m_refreshButton->ExecuteEffector(*m_effector);
		}
	}

	updateGalaxyScoreDropDownList();
}

void SwgCuiGalacticCivilWar::onGCWScoreUpdatedOtherGalaxies(GuildObject::Messages::GCWScoreUpdatedOtherGalaxies::Payload const & obj)
{
	std::string currentServer;
	m_serverComboBox->GetSelectedIndexName(currentServer);

	if(_stricmp(obj.first.first.c_str(), currentServer.c_str()) == 0)
	{
		updateServerSpecificScore();
		updateGalaxyWideScore();

		std::string tabName;
		m_planetTabs->GetTabName(m_planetTabs->GetActiveTab(), tabName);

		if(tabName.find("Yavin")!= std::string::npos)
			tabName = "yavin4";

		if(obj.first.second.find(tabName)!= std::string::npos)
		{
			updatePlanetRegions(tabName);
			updateHistoryGraph();
		}	
	}
}

void SwgCuiGalacticCivilWar::onGCWGroupScoreUpdatedOtherGalaxies(GuildObject::Messages::GCWGroupScoreUpdatedOtherGalaxies::Payload const & obj)
{
	UNREF(obj);

	std::string currentServer;
	m_serverComboBox->GetSelectedIndexName(currentServer);

	if(_stricmp("SWG", obj.first.first.c_str()) == 0 && _stricmp("galaxy", obj.first.second.c_str()) == 0)
		updateGalaxyWideScore();

	if(_stricmp(obj.first.first.c_str(), currentServer.c_str()) == 0)
	{
		updateServerSpecificScore();
		updateGalaxyWideScore();

		std::string tabName;
		m_planetTabs->GetTabName(m_planetTabs->GetActiveTab(), tabName);

		if(tabName.find("Yavin")!= std::string::npos)
			tabName = "yavin4";

		if(_stricmp(tabName.c_str(), "summary") == 0)
		{
			m_refreshButton->ExecuteEffector(*m_effector);
		}
	}

	updateGalaxyScoreDropDownList();
}

void SwgCuiGalacticCivilWar::updateGalaxyMapIcons()
{
	std::string selectedServer;
	m_serverComboBox->GetSelectedIndexName(selectedServer);


	bool otherGalaxy = false;
	if(_stricmp(selectedServer.c_str(), GameNetwork::getCentralServerName().c_str()) != 0)
	{
		otherGalaxy = true;
	}

	std::map<std::string, std::pair<UIButton *, UIButton *> >::iterator iter = m_galaxyButtons.begin();

	for(; iter != m_galaxyButtons.end(); ++iter)
	{
		std::string planet = (*iter).first;
		UIButton * gcwIcon = (*iter).second.second;
		int ImpScore = 50;

		if(otherGalaxy)
		{
			std::map<std::pair<std::string, std::string>, int> const & otherGalaxiesMap = GuildObject::getGuildObject()->getGcwGroupImperialScorePercentileOtherGalaxies();

			std::map<std::pair<std::string, std::string>, int>::const_iterator OtherGalaxy = otherGalaxiesMap.find(std::make_pair<std::string, std::string>(selectedServer, planet));
			if(OtherGalaxy == otherGalaxiesMap.end())
			{
				gcwIcon->SetVisible(false);
				DEBUG_WARNING(true, ("SwgCuiGalacticCivilWar - Failed to find [%s] Galaxy score with key [%s]", selectedServer.c_str(), planet.c_str()));
				continue;
			}

			ImpScore = (*OtherGalaxy).second;
		}
		else
		{
			std::map<std::string, int> const & thisGalaxy = GuildObject::getGuildObject()->getGcwGroupImperialScorePercentileThisGalaxy();
			std::map<std::string, int>::const_iterator findIter = thisGalaxy.find(planet);
			if(findIter == thisGalaxy.end())
			{
				gcwIcon->SetVisible(false);
				DEBUG_WARNING(true, ("SwgCuiGalacticCivilWar - Failed to find own Galaxy score with key [%s]", planet.c_str()));
				continue;
			}

			ImpScore = (*findIter).second;
		}

		int RebScore = 100 - ImpScore;

		gcwIcon->SetVisible(true);

		if(ImpScore > RebScore)
			gcwIcon->SetProperty(UIButton::PropertyName::Icon, PlanetIconStyles::ms_gcwImperialIconStyle);
		else if (RebScore > ImpScore)
			gcwIcon->SetProperty(UIButton::PropertyName::Icon, PlanetIconStyles::ms_gcwRebelIconStyle);
		else
			gcwIcon->SetProperty(UIButton::PropertyName::Icon, PlanetIconStyles::ms_gcwNeutralIconStyle);

	}

}

void SwgCuiGalacticCivilWar::onGCWRegionEntriesReceived(PlanetMapManagerClient::Messages::GcwEventLocationsReceived::Payload const & obj)
{
	UNREF(obj);

	std::string currentServer;
	m_serverComboBox->GetSelectedIndexName(currentServer);

	if(_stricmp(GameNetwork::getCentralServerName().c_str(), currentServer.c_str()) == 0)
	{
		std::string tabName;
		m_planetTabs->GetTabName(m_planetTabs->GetActiveTab(), tabName);

		if(_stricmp("summary", tabName.c_str()) != 0) // We're on our server and we're NOT looking at the summary page. Go ahead and refresh the data since the GCW entry might be related.
			updateMainPage();
	}
}