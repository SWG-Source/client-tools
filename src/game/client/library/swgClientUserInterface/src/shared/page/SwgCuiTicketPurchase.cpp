//===================================================================
//
// SwgCuiTicketPurchase.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiTicketPurchase.h"

#include "StringId.h"
#include "UIButton.h"
#include "UICheckbox.h"
#include "UIComboBox.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIList.h"
#include "UIMessage.h"
#include "UIText.h"
#include "UIWidget.h"
#include "clientGame/ClientCommandQueue.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientGame/GuildObject.h"
#include "clientGraphics/RenderWorld.h"
#include "clientUserInterface/CuiManager.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedGame/GameScheduler.h"
#include "sharedGame/SharedBuildoutAreaManager.h"
#include "sharedGame/TravelManager.h"
#include "sharedMathArchive/VectorArchive.h"
#include "sharedMessageDispatch/Receiver.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/PlanetTravelPointListRequest.h"
#include "sharedNetworkMessages/PlanetTravelPointListResponse.h"
#include "sharedTerrain/TerrainObject.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"
#include "swgClientUserInterface/SwgCuiSpaceGroupLaunch.h"
#include <algorithm>
#include <cstdio>
#include <list>
#include <map>
#include <vector>

//===================================================================

namespace SwgCuiTicketPurchaseNamespace
{
	float const cms_maxRangeFromTerminal = 16.0f;

	std::string spaceToUnderscore (const std::string& source)
	{
		std::string result = source;
		std::replace (result.begin (), result.end (), ' ', '_');
		
		return result;
	}
	
	typedef std::pair<std::pair<int, int>, SwgCuiTicketPurchase *> SimulationData;

	//----------------------------------------------------------------------
	
	void simulationCallback (const void * context)
	{
		NOT_NULL (context);
		SimulationData * const data = const_cast<SimulationData *>(reinterpret_cast<const SimulationData *>(context));

		SwgCuiTicketPurchase * const tp = data->second;
		const int planetIndex           = data->first.first;
		const int sequenceId            = data->first.second;
		delete data;

		std::string planetName;
		if (TravelManager::getPlanetName (planetIndex, planetName))
		{
			std::vector<std::string> travelPointNameList;
			std::vector<int>         travelPointCostList;
			std::vector<Vector>      travelPointPointList;
			std::vector<bool>        travelPointInterplanetaryList;
			
			const float mapWidthInMeters   = TerrainObject::getConstInstance ()->getMapWidthInMeters ();
			const float mapWidthInMeters_4 = mapWidthInMeters * 0.25f;
			
			travelPointNameList.push_back (planetName + "_point1");
			travelPointPointList.push_back (Vector (mapWidthInMeters_4, 0.0f, mapWidthInMeters_4));
			travelPointCostList.push_back (100);
			travelPointInterplanetaryList.push_back (true);
			travelPointNameList.push_back (planetName + "_point 2");
			travelPointPointList.push_back (Vector (-mapWidthInMeters_4, 0.0f, mapWidthInMeters_4));
			travelPointCostList.push_back (100);
			travelPointInterplanetaryList.push_back (true);
			travelPointNameList.push_back (planetName + "_point3");
			travelPointPointList.push_back (Vector (-mapWidthInMeters_4, 0.0f, -mapWidthInMeters_4));
			travelPointCostList.push_back (200);
			travelPointInterplanetaryList.push_back (false);
			travelPointNameList.push_back (planetName + "_point 4");
			travelPointPointList.push_back (Vector (mapWidthInMeters_4, 0.0f, -mapWidthInMeters_4));
			travelPointCostList.push_back (200);
			travelPointInterplanetaryList.push_back (false);
			
			tp->processMessage (PlanetTravelPointListResponse (planetName, travelPointNameList, travelPointPointList, travelPointCostList, travelPointInterplanetaryList, sequenceId));
		}

		tp->release ();
	}

	std::string intToString(int const value)
	{
		char buffer[16];
		snprintf(buffer, sizeof(buffer) - 1, "%d", value);
		return buffer;
	}

	typedef std::map<std::string, float> PlanetWidthMap;
	PlanetWidthMap s_planetWidthMap;

	char const * const cs_PlanetWidthDataTable = "datatables/travel/planet_width.iff";
	char const * const cs_columnPlanet = "Planet";
	char const * const cs_columnWidth = "Width";
	float const cs_defaultPlanetWidth = 16384.0f;

	void initializePlanetWidthMap()
	{
		if (s_planetWidthMap.empty())
		{
			DataTable const * const dt = DataTableManager::getTable(cs_PlanetWidthDataTable, true);
			if (dt)
			{
				std::string planet;
				float width;

				for (int row = 0; row < dt->getNumRows(); ++row)
				{
					planet = dt->getStringValue(cs_columnPlanet, row);
					width = dt->getFloatValue(cs_columnWidth, row);

					s_planetWidthMap[planet] = width;
				}

				DataTableManager::close(cs_PlanetWidthDataTable);
			}
		}
	}

	typedef std::map<std::string, UIButton *> GalacticGCWButtonMap;
	GalacticGCWButtonMap ms_gcwButtons;

	Unicode::String const ms_gcwRebelIconStyle = Unicode::narrowToWide("/Styles.Icon.map.galactic_gcw_rebel");
	Unicode::String const ms_gcwImperialIconStyle = Unicode::narrowToWide("/Styles.Icon.map.galactic_gcw_imperial");
	Unicode::String const ms_gcwNeutralIconStyle = Unicode::narrowToWide("/Styles.Icon.map.galactic_gcw_neutral");
}

using namespace SwgCuiTicketPurchaseNamespace;

//===================================================================
// PUBLIC SwgCuiTicketPurchase::TravelPoint
//===================================================================

struct SwgCuiTicketPurchase::TravelPoint
{
public:

	std::string m_name;
	Vector      m_point_w;
	int         m_cost;
	bool        m_interplanetary;
};

//===================================================================
// PUBLIC SwgCuiTicketPurchase
//===================================================================

SwgCuiTicketPurchase::SwgCuiTicketPurchase (UIPage& page) :
UIEventCallback            (),
CuiMediator                ("SwgCuiTicketPurchase", page),
MessageDispatch::Receiver  (),
m_arrivePlanetComboBox     (0),
m_arriveLocationComboBox   (0),
m_roundTripCheckBox        (0),
m_nameToggleCheckBox       (0),
m_costText                 (0),
m_purchaseButton           (0),
m_cancelButton             (0),
m_sampleButton             (0),
m_galacticMapPage          (0),
m_planetsPage              (0),
m_planetNamesPage          (0),
m_textSample               (0),
m_buttonShowGalaxy         (0),
m_buttonShowPlanet         (0),
m_textLoadingStatus        (0),
m_pageCost                 (0),
m_buttonRoundTrip          (0),
m_buttonTravel             (0),
m_planetTravelPointListMap (new PlanetTravelPointListMap),
m_pointButtons             (new ButtonVector),
m_startingPlanetName       ("tatooine"),
m_startingTravelPointName  ("point1"),
m_requestsOutstanding      (0),
m_currentSequenceId        (0),
m_timer                    (0.0f),
m_callback                 (new MessageDispatch::Callback)
{
	getPage ().SetSelectable (true);
	
	getCodeDataObject (TUIComboBox, m_arrivePlanetComboBox,   "comboArrivePlanet");
	getCodeDataObject (TUIComboBox, m_arriveLocationComboBox, "comboArriveLocation");
	getCodeDataObject (TUICheckbox, m_roundTripCheckBox,      "checkRoundTrip");
	getCodeDataObject (TUICheckbox, m_nameToggleCheckBox,     "checkNameToggle");
	getCodeDataObject (TUIText,     m_costText,               "labelNumericCost");
	getCodeDataObject (TUIButton,   m_purchaseButton,         "buttonPurchase");
	getCodeDataObject (TUIButton,   m_cancelButton,           "buttonCancel");
	getCodeDataObject (TUIButton,   m_sampleButton,           "buttonSample");
	getCodeDataObject (TUIPage,     m_galacticMapPage,        "pageGalacticMap");
	getCodeDataObject (TUIPage,     m_planetsPage,            "pagePlanets");
	getCodeDataObject (TUIPage,     m_planetNamesPage,        "pagePlanetNames");
	getCodeDataObject (TUIText,     m_textSample,             "textSample");
	getCodeDataObject (TUIButton,   m_buttonShowGalaxy,       "buttonShowGalaxy");
	getCodeDataObject (TUIButton,   m_buttonShowPlanet,       "buttonShowPlanet");
	getCodeDataObject (TUIText,     m_textLoadingStatus,      "textLoadingStatus");
	getCodeDataObject (TUIPage,     m_pageCost,               "pageCost");
	getCodeDataObject (TUICheckbox, m_buttonRoundTrip,        "buttonRoundTrip");
	getCodeDataObject (TUIButton,   m_buttonTravel,           "buttonTravel");

	NON_NULL (m_arrivePlanetComboBox->GetDataSource   ())->Clear ();
	NON_NULL (m_arriveLocationComboBox->GetDataSource ())->Clear ();

	registerMediatorObject (*m_arrivePlanetComboBox,   true);
	registerMediatorObject (*m_arriveLocationComboBox, true);
	registerMediatorObject (*m_roundTripCheckBox,      true);
	registerMediatorObject (*m_nameToggleCheckBox,     true);
	registerMediatorObject (*m_costText,               true);
	registerMediatorObject (*m_purchaseButton,         true);
	registerMediatorObject (*m_cancelButton,           true);
	registerMediatorObject (*m_sampleButton,           true);
	registerMediatorObject (*m_galacticMapPage,        true);
	registerMediatorObject (*m_planetsPage,            true);
	registerMediatorObject (*m_planetNamesPage,        true);
	registerMediatorObject (*m_textSample,             false);
	registerMediatorObject (*m_buttonShowGalaxy,       true);
	registerMediatorObject (*m_buttonShowPlanet,       true);
	registerMediatorObject (getPage (),                true);
	registerMediatorObject (*m_buttonTravel,           true);

	m_sampleButton->SetVisible           (false);
	m_textSample->SetVisible             (false);
	m_textSample->SetPreLocalized        (false);
	m_textLoadingStatus->SetPreLocalized (true);
	m_textLoadingStatus->SetVisible      (false);
	setState (MS_closeable);

	initializePlanetWidthMap();

	ms_gcwButtons.clear();

	if(ms_gcwButtons.empty())
	{
		DataTable const * const dt = DataTableManager::getTable(cs_PlanetWidthDataTable, true);
		if (dt)
		{
			std::string planet;

			for (int row = 0; row < dt->getNumRows(); ++row)
			{
				planet = dt->getStringValue(cs_columnPlanet, row);
				std::string buttonName = "gcw" + planet;
				UIButton * gcwButton = NULL;

				getCodeDataObject(TUIButton, gcwButton, buttonName.c_str());

				if(!gcwButton)
					DEBUG_WARNING(true, ("Failed to find Galactic Map GCW Button for planet [%s] using codeData value [%s]", planet.c_str(), buttonName.c_str()));
				else
					ms_gcwButtons.insert(std::make_pair<std::string, UIButton *>(planet, gcwButton));
			}

			DataTableManager::close(cs_PlanetWidthDataTable);
		}
	}
}

//----------------------------------------------------------------------

SwgCuiTicketPurchase::~SwgCuiTicketPurchase ()
{
	//-- remove location buttons added
	UIPage::UIWidgetList widgetList;
	m_planetsPage->GetWidgetList (widgetList);

	UIPage::UIWidgetList::iterator iter = widgetList.begin ();
	UIPage::UIWidgetList::iterator end = widgetList.end ();
	for (; iter != end; ++iter)
	{
		UIPage* const planetPage = safe_cast<UIPage*> (*iter);
		removeButtons (planetPage);
	}

	//--	
	m_arrivePlanetComboBox   = 0;
	m_arriveLocationComboBox = 0;
	m_roundTripCheckBox      = 0;
	m_nameToggleCheckBox     = 0;
	m_costText               = 0;
	m_purchaseButton         = 0;
	m_cancelButton           = 0;
	m_galacticMapPage        = 0;
	m_planetsPage            = 0;
	m_planetNamesPage        = 0;
	m_buttonShowGalaxy       = 0;
	m_buttonShowPlanet       = 0;
	m_pageCost               = 0;
	m_buttonRoundTrip        = 0;
	m_buttonTravel           = 0;

	delete m_planetTravelPointListMap;
	delete m_pointButtons;
	if(m_callback)
		delete m_callback;

	m_planetTravelPointListMap = 0;
	m_pointButtons = 0;
}

//-------------------------------------------------------------------

void SwgCuiTicketPurchase::performActivate ()
{

	static const UILowerString prop_sizeSet = UILowerString ("SwgCuiTicketPurchaseSizeSet");

	m_callback->connect(*this, &SwgCuiTicketPurchase::onGCWValuesUpdated, static_cast<GuildObject::Messages::GCWGroupScoreUpdatedThisGalaxy *>(0));

	if (!getPage ().HasProperty (prop_sizeSet))
	{
		UIWidget * const parent = getPage ().GetParentWidget ();
		if (parent)
		{
			const long desiredHeight    = parent->GetHeight () * 9 / 10;
			const UISize & currentSize  = getPage ().GetSize ();
			const long deltaHeightWidth = desiredHeight - currentSize.y;
			if (deltaHeightWidth > 0)
			{
				const UISize & newSize = currentSize + UISize (deltaHeightWidth, deltaHeightWidth);
				getPage ().SetSize (newSize);
				getPage ().Center ();
			}
		}
		getPage ().SetPropertyBoolean (prop_sizeSet, true);
	}

	setIsUpdating(true);

	CuiManager::requestPointer (true);

	m_arrivePlanetComboBox->SetSelectedIndex (0);
	m_arriveLocationComboBox->SetSelectedIndex (0);

	//-- setup callbacks
	connectToMessage (PlanetTravelPointListResponse::cms_name);

	++m_currentSequenceId;

	m_textLoadingStatus->SetVisible (true);
	m_textLoadingStatus->Clear ();

	//-- fire off the requests for the planet travel points
	const int numberOfPlanets = TravelManager::getNumberOfPlanets ();
	m_requestsOutstanding = numberOfPlanets;

	for (int i = 0; i < numberOfPlanets; ++i)
	{
		if (Game::getSinglePlayer ())
		{
			//-- the simulator will release the ref & delete the data
			this->fetch ();
			SimulationData * const data = new SimulationData (std::make_pair(i, m_currentSequenceId), this);
			GameScheduler::addCallback (simulationCallback, data, 1.0f + 0.2f * i);
		}
		else
		{
			std::string planetName;
			if (TravelManager::getPlanetName (i, planetName))
			{
				const PlanetTravelPointListRequest message (Game::getPlayer ()->getNetworkId (), planetName, m_currentSequenceId);
				GameNetwork::send (message, true);
			}
			else
				DEBUG_WARNING (true, ("SwgCuiTicketPurchase::performActivate: could not get planet name for planet index %i", i));
		}
	}

	updateGCWIcons();
}

//-------------------------------------------------------------------

void SwgCuiTicketPurchase::performDeactivate ()
{
	CuiManager::requestPointer (false);
	
	disconnectFromMessage (PlanetTravelPointListResponse::cms_name);

	m_callback->disconnect(*this, &SwgCuiTicketPurchase::onGCWValuesUpdated, static_cast<GuildObject::Messages::GCWGroupScoreUpdatedThisGalaxy *>(0));
	
	clearPointButtons ();
	
	{
		for (PlanetTravelPointListMap::const_iterator it = m_planetTravelPointListMap->begin (); it != m_planetTravelPointListMap->end (); ++it)
		{
			const ButtonPointPair & bpp = it->second;
			UIButton * const button = bpp.first;
			if (button)
			{
				button->RemoveCallback (this);
				button->Detach (0);
			}
		}
		m_planetTravelPointListMap->clear ();
	}
	setIsUpdating(false);
}

//----------------------------------------------------------------------

void SwgCuiTicketPurchase::clearPointButtons ()
{
	for (ButtonVector::const_iterator it = m_pointButtons->begin (); it != m_pointButtons->end (); ++it)
	{
		UIButton * const button = NON_NULL (*it);
		button->RemoveCallback (this);
	}
	
	m_pointButtons->clear ();
}

//-------------------------------------------------------------------

void SwgCuiTicketPurchase::OnButtonPressed (UIWidget* const context)
{
	if (context == m_buttonShowGalaxy)
	{
		showGalacticMap ();
	}
	else if (context == m_buttonShowPlanet)
	{
		showPlanet (getSelectedArrivePlanet ());
	}
	else if (context == m_purchaseButton)
	{
		//-- send command table command to purchase ticket
		std::string result;
		result += getDepartPlanet () + " ";
		result += spaceToUnderscore (getDepartLocation ()) + " ";
		result += getSelectedArrivePlanet () + " ";
		result += spaceToUnderscore (getSelectedArriveLocation ()) + " ";
		result += m_roundTripCheckBox->IsChecked () ? "1" : "0";
		result += " 0";
		DEBUG_REPORT_LOG_PRINT (true, ("SwgCuiTicketPurchase: purchaseTicket %s\n", result.c_str ()));
		
		static const uint32 hash_purchaseTicket = Crc::normalizeAndCalculate ("purchaseTicket");
		ClientCommandQueue::enqueueCommand (hash_purchaseTicket, NetworkId::cms_invalid, Unicode::narrowToWide (result));
		//start a timer, client cannot request another ticket during that window (prevent server lag from causing player to double-buy)
		if(!Game::getSinglePlayer())
		{
			m_timer = 2.0f;
			m_purchaseButton->SetEnabled(false);
		}
	}
	else if (context == m_cancelButton)
	{
		closeThroughWorkspace ();
	}
	else if(context == m_buttonTravel)
	{
		if (m_travelType == TT_instantTravel)
		{
			//-- send command table command to instant travel
			std::string result;
			result += getDepartPlanet () + " ";
			result += spaceToUnderscore (getDepartLocation ()) + " ";
			result += getSelectedArrivePlanet () + " ";
			result += spaceToUnderscore (getSelectedArriveLocation ()) + " ";
			result += "0 1";

			DEBUG_REPORT_LOG_PRINT (true, ("SwgCuiTicketPurchase: purchaseTicket (instantTravel) [%s]\n", result.c_str ()));
			
			static const uint32 hash_purchaseTicket = Crc::normalizeAndCalculate ("purchaseTicket");
			ClientCommandQueue::enqueueCommand (hash_purchaseTicket, NetworkId::cms_invalid, Unicode::narrowToWide (result));
		}
		else
		{
			if(!m_shipControlDeviceId.isValid())
				return;

			std::string result = m_shipControlDeviceId.getValueString();

			SwgCuiSpaceGroupLaunch::NetworkIdSet const & acceptedMembers = SwgCuiSpaceGroupLaunch::getAcceptedMembers();
			SwgCuiSpaceGroupLaunch::NetworkIdSet::const_iterator ii = acceptedMembers.begin();
			SwgCuiSpaceGroupLaunch::NetworkIdSet::const_iterator iiEnd = acceptedMembers.end();

			result += " " + intToString(acceptedMembers.size());

			for (; ii != iiEnd; ++ii)
			{
				NetworkId const & Id = *ii;
				result += " " + Id.getValueString();
			}

			result += " ";
			result += getSelectedArrivePlanet();
			result += " ";
			result += spaceToUnderscore (getSelectedArriveLocation());
			ClientCommandQueue::enqueueCommand ("launchIntoSpace", m_terminalId, Unicode::narrowToWide(result));
		}

		closeThroughWorkspace();
	}
	else
	{		
		//-- search the planet button list
		
		{
			const PlanetTravelPointListMap::iterator end  = m_planetTravelPointListMap->end ();
			for (PlanetTravelPointListMap::iterator iter = m_planetTravelPointListMap->begin (); iter != end; ++iter)
			{
				const std::string & planetName = iter->first;
				const ButtonPointPair & bpp = iter->second;
				
				if (context == bpp.first)
				{
					selectArrivePlanet (planetName);
					
					showPlanet (iter->first);
					return;
				}
			}
		}
		
		//-- search the point button list
		{
			for (ButtonVector::const_iterator it = m_pointButtons->begin (); it != m_pointButtons->end (); ++it)
			{
				UIButton * const button = NON_NULL (*it);
				if (button == context)
				{
					selectArriveLocation (button->GetName ());
				}
			}
		}
	}
}

//-------------------------------------------------------------------

void SwgCuiTicketPurchase::OnCheckboxSet (UIWidget* const context)
{
	if (context == m_nameToggleCheckBox)
		m_planetNamesPage->SetVisible (m_galacticMapPage->IsVisible ());

	update ();
}

//-------------------------------------------------------------------

void SwgCuiTicketPurchase::OnCheckboxUnset (UIWidget* const context)
{
	if (context == m_nameToggleCheckBox)
		m_planetNamesPage->SetVisible (false);

	update ();
}

//-------------------------------------------------------------------

void SwgCuiTicketPurchase::OnGenericSelectionChanged (UIWidget* const context)
{
	if (context == m_arrivePlanetComboBox)
	{
		updateArriveLocationNames (getDepartPlanet () == getSelectedArrivePlanet (), getDepartLocation ());

		showPlanet (getSelectedArrivePlanet ());
	}

	update ();
}

//-------------------------------------------------------------------

//-- return value means keep processing
bool SwgCuiTicketPurchase::OnMessage (UIWidget* const context, const UIMessage& msg)
{
	if (context != &getPage ())
		return true;

	if (msg.Type == UIMessage::KeyDown && msg.Keystroke == UIMessage::Escape)
	{
		deactivate ();
		return false;
	}

	return true;
}

//-------------------------------------------------------------------

void SwgCuiTicketPurchase::receiveMessage (const MessageDispatch::Emitter& /*source*/, const MessageDispatch::MessageBase& message)
{
	if (message.isType (PlanetTravelPointListResponse::cms_name))
	{
		Archive::ReadIterator readIterator = dynamic_cast<const GameNetworkMessage &> (message).getByteStream ().begin ();
		const PlanetTravelPointListResponse planetTravelPointListResponse (readIterator);
		processMessage (planetTravelPointListResponse);
	}
}

//-------------------------------------------------------------------

bool SwgCuiTicketPurchase::setData (const std::string& planetName, const std::string& travelPointName)
{
	m_startingPlanetName      = planetName;
	m_startingTravelPointName = travelPointName;

	return true;
}

//===================================================================
// PRIVATE SwgCuiTicketPurchase
//===================================================================

void SwgCuiTicketPurchase::update ()
{
	//-- no cost for instant travel
	if (m_travelType == TT_instantTravel)
		return;

	//-- get travel point 1 cost
	const int travelPoint1Cost = getTravelPointCost (getDepartPlanet (), getDepartLocation ());

	//-- get travel point 2 cost
	const int travelPoint2Cost = getTravelPointCost (getSelectedArrivePlanet (), getSelectedArriveLocation ());

	//-- compute the cost between planets
	int planetCost = 0;

	bool const travelOk = (m_travelType == TT_personalShip) || (TravelManager::getPlanetSingleHopCost(getDepartPlanet(), getSelectedArrivePlanet(), planetCost));
	if (!travelOk)
	{
		DEBUG_WARNING (true, ("SwgCuiTicketPurchase::update: planets %s <-> %s do not have a valid route between them", getDepartPlanet ().c_str (), getSelectedArrivePlanet ().c_str ()));
	}

 	const int cost = (planetCost + travelPoint1Cost + travelPoint2Cost) * (m_roundTripCheckBox->IsChecked () ? 2 : 1);
	char buffer [64];
	sprintf (buffer, "%i", cost);

	m_costText->SetLocalText (Unicode::narrowToWide (buffer));
	m_costText->SetPreLocalized (true);
}

//-------------------------------------------------------------------

void SwgCuiTicketPurchase::updateArriveLocationNames ()
{
	const UIData* const data = m_arrivePlanetComboBox->GetDataAtIndex (m_arrivePlanetComboBox->GetSelectedIndex ());
	if (data)
		addArriveLocationUiText (data->GetName());
}

//-------------------------------------------------------------------

void SwgCuiTicketPurchase::updateArriveLocationNames (const bool skipTravelPoint, const std::string& skipTravelPointName)
{
	const UIData* const data = m_arrivePlanetComboBox->GetDataAtIndex (m_arrivePlanetComboBox->GetSelectedIndex ());
	if (data)
		addArriveLocationUiText (data->GetName(), skipTravelPoint, &skipTravelPointName);
}

//-------------------------------------------------------------------

void SwgCuiTicketPurchase::update (float deltaTimeSecs)
{
	CuiMediator::update (deltaTimeSecs);

	if(m_timer > 0)
	{
		m_timer -= deltaTimeSecs;
		if(m_timer <= 0)
		{
			m_purchaseButton->SetEnabled(true);
		}
	}
}

//-------------------------------------------------------------------

void SwgCuiTicketPurchase::addArrivePlanetUiText ()
{
	// we want the planet name displayed in the combo box to match
	// the planet name displayed in the galactic map; this may seem
	// like a hack to store the list of planet display name here, but
	// there isn't single place where there is a "correct" list of planet
	// display name; planet_n.stf has kashyyyk_main as Kachirho, not Kashyyyk;
	// zone_n.stf has kashyyyk_main as Kashyyyk, but doesn't have mustafar;
	// and neither one of them has yavin4 as Yavin IV which is how yavin4 is
	// displayed in the galactic map; I could try to get the display planet
	// name from m_planetNamesPage which is where the name displayed in the
	// galactic map comes from, but there's no guarantee that all planets will
	// have an entry in m_planetNamesPage either (at the time of this writing,
	// by design, kashyyyk_main will not have entry in m_planetNamesPage);
	// I could try m_planetNamesPage first, then try zone_n, then try planets_n,
	// but it seems more simple to just keep a list right here instead;
	// besides, new planets don't get added very often so this list should
	// not need to be updated that often; every planet in
	// dsrc/sku.0/sys.shared/compiled/game/datatables/travel/travel.tab
	// should have an entry here
	static std::map<std::string, std::string> s_planetDisplayName;
	if (s_planetDisplayName.empty())
	{
		s_planetDisplayName[std::string("corellia")] = std::string("Corellia");
		s_planetDisplayName[std::string("dantooine")] = std::string("Dantooine");
		s_planetDisplayName[std::string("dathomir")] = std::string("Dathomir");
		s_planetDisplayName[std::string("endor")] = std::string("Endor");
		s_planetDisplayName[std::string("lok")] = std::string("Lok");
		s_planetDisplayName[std::string("naboo")] = std::string("Naboo");
		s_planetDisplayName[std::string("rori")] = std::string("Rori");
		s_planetDisplayName[std::string("talus")] = std::string("Talus");
		s_planetDisplayName[std::string("tatooine")] = std::string("Tatooine");
		s_planetDisplayName[std::string("yavin4")] = std::string("Yavin IV");
		s_planetDisplayName[std::string("mustafar")] = std::string("Mustafar");
		s_planetDisplayName[std::string("kashyyyk_main")] = std::string("Kashyyyk");
	}

	UIDataSource* const dataSource = m_arrivePlanetComboBox->GetDataSource ();
	dataSource->Clear ();

	bool const interplanetary = isInterplanetary();

	PlanetTravelPointListMap::iterator end  = m_planetTravelPointListMap->end ();
	for (PlanetTravelPointListMap::iterator iter = m_planetTravelPointListMap->begin (); iter != end; ++iter)
	{
		const std::string planetName = iter->first;

		if (!interplanetary && getDepartPlanet() != planetName)
			continue;

		std::string displayPlanetName;
		std::map<std::string, std::string>::const_iterator iterDisplayPlanetName = s_planetDisplayName.find (planetName);
		if (iterDisplayPlanetName != s_planetDisplayName.end ())
		{
			displayPlanetName = iterDisplayPlanetName->second;
		}
		else
		{
			displayPlanetName = planetName;

			// include a "reminder" to update the list with the planet display name
			displayPlanetName += " (update s_planetDisplayName in SwgCuiTicketPurchase.cpp with the planet display name)";
		}
		
		UIData* const data = new UIData ();
		data->SetName (planetName);
		data->SetPropertyNarrow (UIList::DataProperties::TEXT, displayPlanetName);
		dataSource->AddChild (data);
	}

	m_arrivePlanetComboBox->SetSelectedIndex (0, true);
}

//-------------------------------------------------------------------

void SwgCuiTicketPurchase::addArriveLocationUiText (const std::string& planetName, const bool skipTravelPoint, const std::string* const skipTravelPointName)
{
	UIDataSource* const dataSource = m_arriveLocationComboBox->GetDataSource ();
	dataSource->Clear ();

	//-- find the planet
	PlanetTravelPointListMap::iterator iter = m_planetTravelPointListMap->find (planetName);
	if (iter == m_planetTravelPointListMap->end ())
	{
		DEBUG_WARNING (true, ("SwgCuiTicketPurchase::addTravelPointUiText: planet %s not found in travel point map", planetName.c_str ()));
		return;
	}

	//-- add the planet travel points to the combo box
	const TravelPointList& travelPointList = iter->second.second;

	uint i;
	for (i = 0; i < travelPointList.size (); ++i)
	{
		const std::string& travelPointName = travelPointList [i].m_name;

		if (skipTravelPoint && *skipTravelPointName == travelPointName)
			continue;

		if ((m_travelType == TT_personalShip && !travelPointList [i].m_interplanetary) || 
			  (getDepartPlanet () != getSelectedArrivePlanet () && !travelPointList [i].m_interplanetary))
			continue;

		UIData* const data = new UIData ();
		data->SetName (travelPointName);
		data->SetPropertyNarrow (UIList::DataProperties::TEXT, travelPointName);
		dataSource->AddChild (data);
	}

	m_arriveLocationComboBox->SetSelectedIndex (0, true);
}

//-------------------------------------------------------------------

void SwgCuiTicketPurchase::selectArrivePlanet (const std::string& selectionName)
{
	const UIDataSource* const dataSource = m_arrivePlanetComboBox->GetDataSource ();
	const int numberOfChildren = dataSource->GetChildCount ();

	//-- find index
	bool found = false;

	int i;
	for (i = 0; i < numberOfChildren; ++i)
	{
		const UIData* const data = m_arrivePlanetComboBox->GetDataAtIndex (i);
		if (!data)
			break;

		if (data->GetName() == selectionName)
		{
			found = true;
			break;
		}
	}

	m_arrivePlanetComboBox->SetSelectedIndex (found ? i : 0, true);

	if (m_galacticMapPage->IsVisible())
	{
		m_buttonShowGalaxy->SetVisible(false);
		m_buttonShowPlanet->SetVisible(true);
	}

	update ();
}

//-------------------------------------------------------------------

void SwgCuiTicketPurchase::selectArriveLocation (const std::string& selectionName)
{
	const UIDataSource* const dataSource = m_arriveLocationComboBox->GetDataSource ();
	const int numberOfChildren = dataSource->GetChildCount ();

	//-- find index
	bool found = false;

	int i;
	for (i = 0; i < numberOfChildren; ++i)
	{
		const UIData* const data = m_arriveLocationComboBox->GetDataAtIndex (i);
		if (!data)
			break;

		UINarrowString property;
		if (data->GetPropertyNarrow (UIList::DataProperties::TEXT, property) && selectionName == property)
		{
			found = true;
			break;
		}
	}

	m_arriveLocationComboBox->SetSelectedIndex (found ? i : 0, true);

	if (m_galacticMapPage->IsVisible())
	{
		m_buttonShowGalaxy->SetVisible(false);
		m_buttonShowPlanet->SetVisible(true);
	}

	update ();
}

//-------------------------------------------------------------------

int SwgCuiTicketPurchase::getTravelPointCost (const std::string& planetName, const std::string& travelPointName) const
{
	PlanetTravelPointListMap::iterator iter = m_planetTravelPointListMap->find (planetName);
	if (iter == m_planetTravelPointListMap->end ())
		return 0;

	const TravelPointList& travelPointList = iter->second.second;
	uint i;
	for (i = 0; i < travelPointList.size (); ++i)
		if (travelPointList [i].m_name == travelPointName)
			return travelPointList [i].m_cost;

	return 0;
}

//-------------------------------------------------------------------

const std::string& SwgCuiTicketPurchase::getDepartPlanet () const
{
	return m_startingPlanetName;
}

//-------------------------------------------------------------------

const std::string& SwgCuiTicketPurchase::getDepartLocation () const
{
	return m_startingTravelPointName;
}

//-------------------------------------------------------------------

std::string SwgCuiTicketPurchase::getSelectedArrivePlanet () const
{
	std::string result;

	const UIData* const data = m_arrivePlanetComboBox->GetDataAtIndex (m_arrivePlanetComboBox->GetSelectedIndex ());
	if (data)
		result = data->GetName();

	return result;
}

//-------------------------------------------------------------------

std::string SwgCuiTicketPurchase::getSelectedArriveLocation () const
{
	std::string result;

	const UIData* const data = m_arriveLocationComboBox->GetDataAtIndex (m_arriveLocationComboBox->GetSelectedIndex ());
	if (data)
	{
		UINarrowString property;
		if (data->GetPropertyNarrow (UIList::DataProperties::TEXT, property))
			result = property;
	}

	return result;
}

//-------------------------------------------------------------------

void SwgCuiTicketPurchase::showGalacticMap ()
{
	m_buttonShowGalaxy->SetVisible (false);
	m_buttonShowPlanet->SetVisible (!getSelectedArrivePlanet ().empty ());

	m_galacticMapPage->SetVisible (true);
	m_planetNamesPage->SetVisible (m_nameToggleCheckBox->IsChecked ());
	m_nameToggleCheckBox->SetVisible(true);

	updateGCWIcons();
}

//-------------------------------------------------------------------

bool SwgCuiTicketPurchase::isInterplanetary() const
{
	if (m_travelType == TT_personalShip)
		return true;

	if (m_travelType == TT_instantTravel)
		return false;

	// check if the departure location associated with this ticket terminal is interplanetary
	PlanetTravelPointListMap::iterator planetTravelPointListMapIter = m_planetTravelPointListMap->find(getDepartPlanet());

	if (planetTravelPointListMapIter != m_planetTravelPointListMap->end())
	{
		TravelPointList const & travelPointList = planetTravelPointListMapIter->second.second;

		for (uint j = 0; j < travelPointList.size (); ++j)
		{
			const TravelPoint& travelPoint = travelPointList[j];

			if (travelPoint.m_name == getDepartLocation() && travelPoint.m_interplanetary)
				return true;
		}
	}
	else
	{
		// Empty m_planetTravelPointListMap - assuming interplanetary
		return true;
	}

	return false;
}

//-------------------------------------------------------------------

void SwgCuiTicketPurchase::showPlanet (const std::string& planetName)
{
	if (m_requestsOutstanding > 0)
		return;

	clearPointButtons ();

	m_buttonShowPlanet->SetVisible (false);

	m_planetNamesPage->SetVisible  (false);
	m_galacticMapPage->SetVisible  (false);
	m_planetsPage->SetVisible      (true);
	m_nameToggleCheckBox->SetVisible(false);

	GalacticGCWButtonMap::iterator iter = ms_gcwButtons.begin();
	for(; iter != ms_gcwButtons.end(); ++iter)
		iter->second->SetVisible(false);

	//-- for each travel point
	PlanetTravelPointListMap::iterator planetTravelPointListMapIter = m_planetTravelPointListMap->find (planetName);
	if (planetTravelPointListMapIter == m_planetTravelPointListMap->end ())
	{
		DEBUG_WARNING (true, ("SwgCuiTicketPurchase::showPlanet: planet %s not found in travel point map", planetName.c_str ()));
		return;
	}

	bool foundPlanetPage = false;

	//-- set the appropriate planet as visible
	UIPage::UIWidgetList planetsPageWidgetList;
	m_planetsPage->GetWidgetList (planetsPageWidgetList);

	UIPage::UIWidgetList::iterator planetsPageIter = planetsPageWidgetList.begin ();
	UIPage::UIWidgetList::iterator planetsPageEnd = planetsPageWidgetList.end ();
	for (; planetsPageIter != planetsPageEnd; ++planetsPageIter)
	{
		UIPage* const planetPage = safe_cast<UIPage*> (*planetsPageIter);

		if (_stricmp (planetPage->GetName ().c_str (), planetName.c_str ()) == 0)
		{
			//-- set visible
			foundPlanetPage = true;
			planetPage->SetVisible (true);

			//-- remove existing child buttons
			removeButtons (planetPage);
			
			//-- add the planet travel points to the combo box
			const TravelPointList& travelPointList = planetTravelPointListMapIter->second.second;
			float mapWidthInMeters = TerrainObject::getConstInstance()->getMapWidthInMeters();

			// Use the terrain width if the planet matches the current scene otherwise use the planet width table
			if (Game::getSceneId() != planetName)
			{
				PlanetWidthMap::const_iterator iterPlanetWidthMap = s_planetWidthMap.find(planetName);
				if (iterPlanetWidthMap != s_planetWidthMap.end())
				{
					mapWidthInMeters = iterPlanetWidthMap->second;
				}
				else
				{
					mapWidthInMeters = cs_defaultPlanetWidth;
					DEBUG_WARNING(true, ("Missing entry in '%s' for planet '%s' assuming default width of '%f'",
						cs_PlanetWidthDataTable, planetName, cs_defaultPlanetWidth));
				}
			}

			const float mapWidthInMeters_2 = mapWidthInMeters * 0.5f;
			const UISize size = planetPage->GetSize ();

			uint j;
			for (j = 0; j < travelPointList.size (); ++j)
			{
				const TravelPoint& travelPoint = travelPointList [j];

				if ((m_travelType == TT_personalShip && !travelPoint.m_interplanetary) || 
					  (planetName != getDepartPlanet () && !travelPoint.m_interplanetary))
					continue;

				float x = 0.0f;
				float z = 0.0f;

				BuildoutArea const * const ba = SharedBuildoutAreaManager::findBuildoutAreaAtPosition(planetName, travelPoint.m_point_w.x, travelPoint.m_point_w.z, true);
				if (ba)
				{
					Vector2d const & size = ba->getSize(true);

					Vector baPos = ba->getRelativePosition(travelPoint.m_point_w, true);
					baPos.x += size.x * 0.5f;
					baPos.z += size.y * 0.5f;

					x = clamp (0.f, (baPos.x) / size.x, 1.f);
					z = 1.f - clamp (0.f, (baPos.z) / size.y, 1.f);
					
				}
				else
				{
					//-- get widget space coordinates for travel point
					x = clamp (0.f, (travelPoint.m_point_w.x + mapWidthInMeters_2) / mapWidthInMeters, 1.f);
					z = 1.f - clamp (0.f, (travelPoint.m_point_w.z + mapWidthInMeters_2) / mapWidthInMeters, 1.f);
				}

				//-- create button
				UIButton* const button = safe_cast<UIButton*> (m_sampleButton->DuplicateObject ());

				{
					button->SetVisible (true);
					button->SetName (travelPoint.m_name);
					UIPoint buttonLocation (static_cast<UIScalar> (x * size.x), static_cast<UIScalar> (z * size.y));
					buttonLocation -= button->GetSize () / 2L;
					button->SetLocation (buttonLocation);
					planetPage->InsertChildAfter (button, 0);
					button->Link ();
					button->Attach (0);
					button->AddCallback (this);
					m_pointButtons->push_back (button);
				}
				
				//-- create text label
				{
					UIText * const text = safe_cast<UIText *>(m_textSample->DuplicateObject ());
					planetPage->InsertChildAfter (text, 0);
					text->Attach (0);
					text->Link ();

					text->SetVisible (true);
					text->SetName (std::string ("text") + travelPoint.m_name);
					text->SetText (Unicode::narrowToWide (travelPoint.m_name));
					text->SizeToContent ();

					UIPoint textLoc (button->GetLocation ());
					textLoc.x += button->GetWidth () / 2L - text->GetWidth () / 2L;
					textLoc.y += button->GetHeight ();

					text->SetLocation (textLoc);
				}
			}
		}
		else
			planetPage->SetVisible (false);
	}

	if (!foundPlanetPage)
	{
		// if there was no planet page to show, just show the galactic
		// map, because that is better than showing a blank page
		showGalacticMap ();
		m_buttonShowPlanet->SetVisible (false);
	}
	else
	{
		m_buttonShowGalaxy->SetVisible (isInterplanetary());
	}
}

//-------------------------------------------------------------------

void SwgCuiTicketPurchase::processMessage (const PlanetTravelPointListResponse& planetTravelPointListResponse)
{
	/*
	if (planetTravelPointListResponse.getSequenceId () != m_currentSequenceId)
	{
		WARNING (true, ("Ignoring stale PlanetTravelPointListResponse"));
		return;
	}
	*/

	--m_requestsOutstanding;
	WARNING (m_requestsOutstanding < 0, ("SwgCuiTicketPurchase m_requestsOutstanding [%d] went negative", m_requestsOutstanding));

	const std::string& planetName = planetTravelPointListResponse.getPlanetName ();
	DEBUG_REPORT_LOG (true, ("SwgCuiTicketPurchase received planet list for %s\n", planetName.c_str ()));
	
	int planetCost = 0;
	bool messageOk = m_travelType == TT_personalShip || (TravelManager::getPlanetSingleHopCost (getDepartPlanet (), planetName, planetCost));

	if (m_travelType == TT_instantTravel && planetName != getDepartPlanet())
		messageOk = false;
	
	if (messageOk)
	{
		const std::vector<std::string>& travelPointNameList = planetTravelPointListResponse.getTravelPointNameList ();
		messageOk = !travelPointNameList.empty ();
		
		if (messageOk)
		{		
			const std::vector<int>& travelPointCostList = planetTravelPointListResponse.getTravelPointCostList ();
			DEBUG_FATAL (travelPointNameList.size () != travelPointCostList.size (), ("PlanetTravelPointListResponse name/cost mismatch for planet (%i != %i)", travelPointNameList.size(), travelPointCostList.size ()));
			const std::vector<Vector>& travelPointPointList = planetTravelPointListResponse.getTravelPointPointList ();
			DEBUG_FATAL (travelPointNameList.size () != travelPointPointList.size (), ("PlanetTravelPointListResponse name/point mismatch for planet (%i != %i)", travelPointNameList.size (), travelPointPointList.size ()));
			const std::vector<bool>& travelPointInterplanetaryList = planetTravelPointListResponse.getTravelPointInterplanetaryList ();
			DEBUG_FATAL (travelPointNameList.size () != travelPointInterplanetaryList.size (), ("PlanetTravelPointListResponse name/interplanetary mismatch for planet (%i != %i)", travelPointNameList.size (), travelPointInterplanetaryList.size ()));
			
			TravelPointList travelPointList;
			
			// add camp travel points alphabetically to the bottom of the list
			typedef std::multimap<std::string, TravelPoint> MapCampTravelPoint;
			MapCampTravelPoint campTravelPoint;

			for (size_t i = 0; i < travelPointNameList.size (); ++i)
			{
				TravelPoint travelPoint;
				travelPoint.m_name = travelPointNameList [i];
				travelPoint.m_point_w = travelPointPointList [i];
				travelPoint.m_cost = travelPointCostList [i];
				travelPoint.m_interplanetary = travelPointInterplanetaryList [i];

				std::string::size_type pos = travelPointNameList[i].rfind("'s Camp");
				if ((pos != std::string::npos) && ((pos + 7) == travelPointNameList[i].size()))
				{
					std::string travelPointName = Unicode::toUpper(travelPointNameList[i]);
					while (!travelPointName.empty() && !::isalnum(travelPointName[0]))
						travelPointName = travelPointName.substr(1);

					if (travelPointName.empty())
						travelPointName = Unicode::toUpper(travelPointNameList[i]);

					IGNORE_RETURN(campTravelPoint.insert(std::make_pair(travelPointName, travelPoint)));
					DEBUG_REPORT_LOG (true, ("  %i (camp %s)  %s <%1.1f, %1.1f> %i %s\n", i, travelPointName.c_str(), travelPoint.m_name.c_str (), travelPoint.m_point_w.x, travelPoint.m_point_w.z, travelPoint.m_cost, travelPoint.m_interplanetary ? "true" : "false"));
				}
				else
				{
					travelPointList.push_back (travelPoint);
					DEBUG_REPORT_LOG (true, ("  %i (non-camp)  %s <%1.1f, %1.1f> %i %s\n", i, travelPoint.m_name.c_str (), travelPoint.m_point_w.x, travelPoint.m_point_w.z, travelPoint.m_cost, travelPoint.m_interplanetary ? "true" : "false"));
				}
			}

			for (MapCampTravelPoint::const_iterator iterCampTravelPoint = campTravelPoint.begin(); iterCampTravelPoint != campTravelPoint.end(); ++iterCampTravelPoint)
				travelPointList.push_back(iterCampTravelPoint->second);

			//-- find button associated with planet
			char buttonName [64];
			snprintf (buttonName, sizeof (buttonName), "button%s", planetName.c_str ());
			buttonName [6] = static_cast<char> (toupper (buttonName [6]));
			
			UIButton* button = 0;
			if (getCodeDataObject (TUIButton, button, buttonName, true))
			{
				button->Attach (0);
				button->AddCallback (this);
			}
			
			//-- insert button into list
			(*m_planetTravelPointListMap) [planetName] = std::make_pair (button, travelPointList);
		}
	}
	
	if (m_requestsOutstanding <= 0)
	{
		addArrivePlanetUiText();

		m_textLoadingStatus->SetVisible (false);
		//show the appropriate buttons
		updateTravelType();

		updateArriveLocationNames (getDepartPlanet () == getSelectedArrivePlanet (), getDepartLocation ());

		//-- set the starting arrive planet
		selectArrivePlanet (getDepartPlanet ());

		if (isInterplanetary())
			showGalacticMap();
		else
			showPlanet(getDepartPlanet());
	}
	else if (messageOk)
	{
		m_textLoadingStatus->SetVisible (true);
		m_textLoadingStatus->SetLocalText (Unicode::narrowToWide ("Received: ") + StringId ("planet_n", planetName).localize ());
	}
}

//-------------------------------------------------------------------

void SwgCuiTicketPurchase::removeButtons (UIPage* const page) const
{
	UIPage::UIWidgetList widgetList;
	page->GetWidgetList (widgetList);

	UIPage::UIWidgetList::iterator iter = widgetList.begin ();
	UIPage::UIWidgetList::iterator end  = widgetList.end ();

	while (iter != end)
	{
		UIWidget* const widget = *iter;

		if (widget && (widget->IsA (TUIButton) || widget->IsA (TUIText)))
		{
			page->RemoveChild (widget);
			widget->Detach (0);
			iter = widgetList.erase (iter);
			end = widgetList.end ();
		}
		else
			++iter;
	}
}

//-------------------------------------------------------------------

void SwgCuiTicketPurchase::setTravelType(SwgCuiTicketPurchase::TravelType const type)
{
	m_travelType = type;
}

//-------------------------------------------------------------------

void SwgCuiTicketPurchase::updateTravelType()
{
	if(m_travelType == TT_buyTicket)
	{
		m_pageCost->SetVisible(true);
		m_buttonRoundTrip->SetVisible(true);

		m_buttonTravel->SetVisible(false);
		m_purchaseButton->SetVisible(true);
		m_arrivePlanetComboBox->SetEnabled(true);

		setAssociatedObjectId(NetworkId::cms_invalid);
	}
	else if(m_travelType == TT_personalShip)
	{
		m_pageCost->SetVisible(false);
		m_buttonRoundTrip->SetVisible(false);

		m_buttonTravel->SetVisible(true);
		m_purchaseButton->SetVisible(false);
		m_arrivePlanetComboBox->SetEnabled(true);

		setAssociatedObjectId(m_terminalId);
	}
	else if(m_travelType == TT_instantTravel)
	{
		m_pageCost->SetVisible(false);
		m_buttonRoundTrip->SetVisible(false);

		m_buttonTravel->SetVisible(true);
		m_purchaseButton->SetVisible(false);
		m_arrivePlanetComboBox->SetEnabled(false);

		setAssociatedObjectId(NetworkId::cms_invalid);
	}
}

//-------------------------------------------------------------------

void SwgCuiTicketPurchase::setTerminalId(NetworkId const & terminalId)
{
	m_terminalId = terminalId;
	setMaxRangeFromObject(cms_maxRangeFromTerminal);
}

//-------------------------------------------------------------------

void SwgCuiTicketPurchase::setShipControlDeviceId(NetworkId const & shipControlDeviceId)
{
	m_shipControlDeviceId = shipControlDeviceId;
}

//-------------------------------------------------------------------

void SwgCuiTicketPurchase::updateGCWIcons()
{
	if(GuildObject::getGuildObject() == NULL) // This should never happen, but let's be safe.
		return;

	GalacticGCWButtonMap::iterator iter = ms_gcwButtons.begin();
	for(; iter != ms_gcwButtons.end(); ++iter)
	{
		std::string planet = iter->first;
		UIButton * gcwIcon = iter->second;

		if(!gcwIcon) // again, should be impossible.
			continue;

		std::map<std::string, int> const & gcwScoreMap = GuildObject::getGuildObject()->getGcwGroupImperialScorePercentileThisGalaxy();

		std::map<std::string, int>::const_iterator scoreIter = gcwScoreMap.find(planet);
		if(scoreIter != gcwScoreMap.end())
		{
			gcwIcon->SetVisible(true);

			int imperialScore = scoreIter->second;
			int rebelScore = 100 - imperialScore;

			if(imperialScore > rebelScore)
				gcwIcon->SetProperty(UIButton::PropertyName::Icon, ms_gcwImperialIconStyle);
			else if (rebelScore > imperialScore)
				gcwIcon->SetProperty(UIButton::PropertyName::Icon, ms_gcwRebelIconStyle);
			else
				gcwIcon->SetProperty(UIButton::PropertyName::Icon, ms_gcwNeutralIconStyle);
		}
		else
		{
			DEBUG_WARNING(true, ("GalacticMap - updateGCWIcons: Could not find GCW score for planet[%s]", planet.c_str()));
			gcwIcon->SetVisible(false);
		}
	}
}

void SwgCuiTicketPurchase::onGCWValuesUpdated(bool)
{
	if(m_galacticMapPage->IsVisible())
		updateGCWIcons();
}

//===================================================================

