//======================================================================
//
// SwgCuiPlanetMap.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiPlanetMap.h"

#include "UIButton.h"
#include "UICheckbox.h"
#include "UIColorEffector.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIDataSourceContainer.h"
#include "UIManager.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UIPopupMenu.h"
#include "UISliderbar.h"
#include "UIText.h"
#include "UITreeView.h"
#include "UIUtils.h"
#include "clientGame/ClientMissionObject.h"
#include "clientGame/ClientWaypointObject.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GuildObject.h"
#include "clientUserInterface/CuiInventoryManager.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMenuInfoTypes.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiPoiManager.h"
#include "clientUserInterface/CuiRadialMenuManager.h"
#include "clientUserInterface/CuiSettings.h"
#include "clientUserInterface/CuiStringIdsPlanetMap.h"
#include "clientUserInterface/CuiUtils.h"
#include "sharedGame/GroundZoneManager.h"
#include "sharedGame/PlanetMapManager.h"
#include "sharedGame/SharedBuildoutAreaManager.h"
#include "sharedMath/Vector2d.h"
#include "sharedMessageDispatch/Receiver.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/MapLocation.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedTerrain/ProceduralTerrainAppearance.h"
#include "sharedTerrain/TerrainObject.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"
#include <list>
#include <map>

//======================================================================

namespace SwgCuiPlanetMapNamespace
{
	const long s_sliderRange   = 10;
	const float s_sliderFactor = RECIP (static_cast<float>(s_sliderRange));

	namespace Properties
	{
		const UILowerString Category         = UILowerString ("Category");
		const UILowerString SubCategory      = UILowerString ("SubCategory");
		const UILowerString EntryName        = UILowerString ("EntryName");
		const UILowerString EntryId          = UILowerString ("EntryId");
		const UILowerString IsCategory       = UILowerString ("IsCategory");
		const UILowerString IsSubCategory    = UILowerString ("IsSubCategory");
		const UILowerString MapWidth         = UILowerString ("MapWidth");
		const UILowerString Position         = UILowerString ("Position");
	}

	namespace PopupItems
	{
		const std::string waypoint_create     = "waypoint_create";
		const std::string waypoint_activate   = "waypoint_activate";
		const std::string waypoint_deactivate = "waypoint_deactivate";
		const std::string waypoint_destroy    = "waypoint_destroy";
		const std::string waypoint_set_name   = "waypoint_set_name";
	}

	namespace Settings
	{
		const std::string lastPlanet               = "lastPlanet";
		const std::string lastSelectedLocationPath = "lastSelectedLocationPath";
		const std::string showCities               = "showCities";
		const std::string showNames                = "showNames";
		const std::string showWaypoints            = "showWaypoints";
		const std::string showGCWRregions		   = "showGCWRegions";
	}

	namespace RegionIconStyles
	{
		Unicode::String const rebel = Unicode::narrowToWide("/Styles.Icon.map.gcw_rebel");
		Unicode::String const imperial = Unicode::narrowToWide("/Styles.Icon.map.gcw_imperial");
		Unicode::String const neutral = Unicode::narrowToWide("/Styles.Icon.map.gcw_neutral");
	}

	class Listener :
	public MessageDispatch::Receiver
	{
	public:
		//----------------------------------------------------------------------
		
		Listener(SwgCuiPlanetMap & planetMap) : 
		  MessageDispatch::Receiver (),
			  m_planetMap(planetMap)

		  {
			  connectToMessage(Game::Messages::SCENE_CHANGED);
		  }
		  
		  //----------------------------------------------------------------------
		  
		  void receiveMessage (const MessageDispatch::Emitter &, const MessageDispatch::MessageBase & message)
		  {
			  //----------------------------------------------------------------------
			  if(message.isType(Game::Messages::SCENE_CHANGED))
			  {
				  m_planetMap.onSceneChanged();
			  }
		  }
	
	private:
		Listener();
		Listener & operator=(Listener const & rhs);

		SwgCuiPlanetMap & m_planetMap;
	};
	
	Listener * s_listener = 0;

	float const cms_planetSizeEdge = 16384.0f;

	struct UnusedMapPixels
	{
		int left;
		int right;
		int top;
		int bottom;

		UnusedMapPixels(): left(0), right(0), top(0), bottom(0) {}
	};

	std::string const ms_mapAdjustmentsFile("datatables/planetary_map/map_adjustments.iff");
	std::map<std::string, UnusedMapPixels> ms_mapAdjustments;

	typedef stdmap<std::string, UIPoint>::fwd NamedPointMap;
	NamedPointMap ms_mapOffset;

	float const ms_mapTextureSize = 512.0f;
	std::string const ms_regionNameTable("gcw_regions");
}

using namespace SwgCuiPlanetMapNamespace;

//----------------------------------------------------------------------

SwgCuiPlanetMap::SwgCuiPlanetMap (UIPage & page) :
CuiMediator                ("SwgCuiPlanetMap", page),
UIEventCallback            (),
m_pagePlanet               (0),
m_sliderZoom               (0),
m_maxPlanetScrollExtent    (3072, 3072),
m_lastScrollLocation       (),
m_centerPointTarget        (),
m_centerPoint              (),
m_interpolating            (false),
m_ignoreNextScroll         (false),
m_centerPointOffset        (),
m_checkShowCities          (0),
m_checkShowWaypoints       (0),
m_checkShowNames           (0),
m_checkShowGCWRegions      (0),
m_treeLocations            (0),
m_iconHere                 (0),
m_zoneSize                 (cms_planetSizeEdge, cms_planetSizeEdge),
m_buttonSample             (0),
m_textSample               (0),
m_pageMarkers              (0),
m_textPosition             (0),
m_textHeadingRange         (0),
m_effectorWaypointFlash    (0),
m_freeListButton           (new ButtonVector),
m_freeListText             (new TextVector),
m_textPlanetName           (0),
m_pageMaps                 (0),
m_callback                 (new MessageDispatch::Callback),
m_lastSize                 (),
m_lastZone                 (),
m_lastSelectedLocationPath (),
m_buttonTextMap            (new ButtonTextMap),
m_buttonRefresh            (0),
m_sampleButtonMap          (new SampleButtonMap),
m_iconPathInactive         (),
m_iconPathActive           (),
m_buttonZoom               (NULL),
m_zoomLevel                (ZL_Planet),
m_zoneWorldCoordinateOffset(),
m_entriesRequested         (false)
{

	setState (MS_closeable);
	setState (MS_closeDeactivates);

	getCodeDataObject (TUIPage,           m_pagePlanet,            "pagePlanet");
	getCodeDataObject (TUISliderbar,      m_sliderZoom,            "sliderZoom");
	getCodeDataObject (TUICheckbox,       m_checkShowCities,       "checkShowCities");
	getCodeDataObject (TUICheckbox,       m_checkShowWaypoints,    "checkShowWaypoints");
	getCodeDataObject (TUICheckbox,       m_checkShowNames,        "checkShowNames");
	getCodeDataObject (TUICheckbox,       m_checkShowGCWRegions,   "checkShowGCWRegions");
	getCodeDataObject (TUITreeView,       m_treeLocations,         "treeLocations");
	getCodeDataObject (TUIWidget,         m_iconHere,              "iconHere");
	getCodeDataObject (TUIButton,         m_buttonSample,          "buttonSampleGeneric");
	getCodeDataObject (TUIColorEffector,  m_effectorWaypointFlash, "effectorWaypointFlash");

	getCodeDataObject (TUIText,           m_textSample,            "textSample");
	getCodeDataObject (TUIPage,           m_pageMarkers,           "pageMarkers");
	getCodeDataObject (TUIText,           m_textPosition,          "textPosition");
	getCodeDataObject (TUIText,           m_textHeadingRange,      "textHeadingRange");
	
	getCodeDataObject (TUIText,           m_textPlanetName,        "textPlanetName");
	getCodeDataObject (TUIPage,           m_pageMaps,              "pageMaps");

	getCodeDataObject (TUIButton,         m_buttonRefresh,         "buttonRefresh");
	getCodeDataObject (TUIButton,         m_buttonZoom,            "buttonZoom");

	const UIData * const codeData = getCodeData ();
	NOT_NULL (codeData);
	if (codeData)
	{
		if (!codeData->GetProperty (UILowerString ("IconPathInactive"), m_iconPathInactive))
			DEBUG_FATAL (true, ("SwgCuiPlanetMap must have property IconPathInactive"));
		if (!codeData->GetProperty (UILowerString ("IconPathActive"), m_iconPathActive))
			DEBUG_FATAL (true, ("SwgCuiPlanetMap must have property IconPathActive"));

		if (!UIManager::gUIManager ().GetObjectFromPath (Unicode::wideToNarrow (m_iconPathActive).c_str (), TUIImageStyle))
			DEBUG_FATAL (true, ("SwgCuiPlanetMap IconPathActive [%s] not found", Unicode::wideToNarrow (m_iconPathActive).c_str ()));
		if (!UIManager::gUIManager ().GetObjectFromPath (Unicode::wideToNarrow (m_iconPathInactive).c_str (), TUIImageStyle))
			DEBUG_FATAL (true, ("SwgCuiPlanetMap IconPathInactive [%s] not found", Unicode::wideToNarrow (m_iconPathInactive).c_str ()));
	}

	m_textPosition->SetPreLocalized        (true);
	m_textHeadingRange->SetPreLocalized    (true);
	m_buttonSample->SetVisible             (false);
	m_textPlanetName->SetPreLocalized      (true);
	m_textSample->SetVisible               (false);

	m_sliderZoom->SetUpperLimit (s_sliderRange);

	m_pagePlanet->SetContextCapable    (true, true);
	m_treeLocations->SetContextCapable (true, true);

	m_treeLocations->ClearData ();

	registerMediatorObject (*m_sliderZoom,         true);
	registerMediatorObject (*m_pagePlanet,         true);
	registerMediatorObject (*m_checkShowCities,    true);
	registerMediatorObject (*m_checkShowWaypoints, true);
	registerMediatorObject (*m_checkShowNames,     true);
	registerMediatorObject (*m_checkShowGCWRegions, true);
	registerMediatorObject (*m_treeLocations,      true);
	registerMediatorObject (*m_buttonRefresh,      true);
	registerMediatorObject (*m_buttonZoom,         true);

	m_checkShowNames->SetChecked      (false);
	m_checkShowCities->SetChecked     (true);	
	m_checkShowWaypoints->SetChecked  (true);
	m_checkShowGCWRegions->SetChecked (true);

	m_pagePlanet->SetLocalTooltip (CuiStringIdsPlanetMap::tooltip_map.localize ());
	
	static const std::pair<std::string, std::string> s_buttonSampleNames [] =
	{
		std::make_pair (std::string ("bank"),             std::string ("buttonSampleBank")),
		std::make_pair (std::string ("cantina"),          std::string ("buttonSampleCantina")),
		std::make_pair (std::string ("guild"),            std::string ("buttonSampleGuild")),
		std::make_pair (std::string ("hotel"),            std::string ("buttonSampleHotel")),
		std::make_pair (std::string ("shuttleport"),      std::string ("buttonSampleShuttle")),
		std::make_pair (std::string ("starport"),         std::string ("buttonSampleStar")),
		std::make_pair (std::string ("themepark"),        std::string ("buttonSampleTheme")),
		std::make_pair (std::string ("city"),             std::string ("buttonSampleCity")),
		std::make_pair (std::string ("waypoint"),         std::string ("buttonSampleWaypoint")),
		std::make_pair (std::string ("poi"),              std::string ("buttonSampleWaypoint")),
		std::make_pair (std::string ("gcw_region"),       std::string ("buttonSampleGCW"))
	};
	
	static const size_t s_numButtonSampleNames = sizeof (s_buttonSampleNames) / sizeof (s_buttonSampleNames [0]);

	for (int i = 0; i < s_numButtonSampleNames; ++i)
	{
		const std::string & categoryName     = s_buttonSampleNames [i].first;
		const std::string & codeDataName     = s_buttonSampleNames [i].second;
		const uint8         category         = PlanetMapManager::findCategory (categoryName);

		UIButton * & button = (*m_sampleButtonMap) [category] = 0;

		getCodeDataObject (TUIButton,         button, codeDataName.c_str ());
		button->SetVisible (false);
	}

	NON_NULL (m_buttonSample->GetParentWidget ())->SetVisible (false);

	UnusedMapPixels unusedPixels;
	ms_mapAdjustments.clear();
	DataTable * const table = DataTableManager::getTable(ms_mapAdjustmentsFile, true);
	if(table)
	{
		int const numRows = table->getNumRows ();
		for (int i = 0; i < numRows; ++i)
		{
			std::string const & areaName = table->getStringValue("Zone", i);

			bool unused = table->getIntValue("unused", i);

			if (unused)
			{
				unusedPixels.left   = table->getIntValue("UnusedMapPixelsOnLeft",   i);
				unusedPixels.right  = table->getIntValue("UnusedMapPixelsOnRight",  i);
				unusedPixels.top    = table->getIntValue("UnusedMapPixelsOnTop",    i);
				unusedPixels.bottom = table->getIntValue("UnusedMapPixelsOnBottom", i);
				ms_mapAdjustments[areaName] = unusedPixels;
			}
			else
			{
				UIPoint pt;
				pt.x = table->getIntValue("offsetX", i);
				pt.y = table->getIntValue("offsetY", i);
				ms_mapOffset[areaName] = pt;
			}
		}
		DataTableManager::close(ms_mapAdjustmentsFile);
	}

	s_listener = new Listener(*this);
}

//----------------------------------------------------------------------

SwgCuiPlanetMap::~SwgCuiPlanetMap()
{
	delete m_sampleButtonMap;
	m_sampleButtonMap = 0;

	delete m_buttonTextMap;
	m_buttonTextMap = 0;

	delete m_freeListButton;
	m_freeListButton = 0;

	delete m_freeListText;
	m_freeListText = 0;

	delete m_callback;
	m_callback = 0;

	delete s_listener;
	s_listener = 0;

	m_buttonZoom = 0;

	ms_mapAdjustments.clear();
}

//----------------------------------------------------------------------

void SwgCuiPlanetMap::performActivate()
{
	//This call causes the Poi manager to load its data if it hasn't already.  It doesn't cause a copy.
	CuiPoiManager::getData();

	CuiManager::requestPointer(true);

	getPage ().ForcePackChildren ();
	m_lastSize = getPage ().GetSize ();

	const std::string & planet = Game::getSceneId ();
	std::string zone;
	bool zoneChanged = false;
	Object const * const player = Game::getPlayer ();
	if (player)
	{
		BuildoutArea const * const ba = GroundZoneManager::getZoneName(planet, player->getPosition_w(), zone);
		bool const isAZone = NULL != ba;
		if(isAZone)
		{
			m_zoomLevel = ZL_Zone;
			zone = Game::getSceneId() + "__" + zone;
		}
		else
			m_zoomLevel = ZL_Planet;

		zoneChanged = m_lastZone != zone;
		if (zoneChanged)
		{
			m_lastZone = zone;
			setupCurrentZone();
			m_sliderZoom->SetValue(m_sliderZoom->GetUpperLimit (), false);
			m_treeLocations->SelectRow(-1);
			reset();
		}
		else if (m_textPlanetName)
		{
			// refresh window title as GCW planetary score may have changed
			GuildObject const * const go = GuildObject::getGuildObject();
			if (go)
			{
				std::map<std::string, int> const & gcwScoreMap = go->getGcwGroupImperialScorePercentileThisGalaxy();
				std::map<std::string, int>::const_iterator const iterFind = gcwScoreMap.find(zone);
				if (iterFind != gcwScoreMap.end())
				{
					char buffer[64];
					snprintf(buffer, sizeof(buffer)-1, " (R:%d%% I:%d%%)", (100 - iterFind->second), iterFind->second);
					buffer[sizeof(buffer)-1] = '\0';

					Unicode::String const windowTitle = StringId("zone_n", zone).localize() + Unicode::narrowToWide(buffer);
					m_textPlanetName->SetLocalText(windowTitle);
				}
			}
		}
	}

	updateZoom ();

	if (zoneChanged)
		computeCenterPoint (m_iconHere->GetLocation ());

	m_ignoreNextScroll = false;

	setIsUpdating (true);

	m_textPosition->Clear ();
	m_textHeadingRange->Clear ();

	m_callback->connect (*this, &SwgCuiPlanetMap::onAddedRemovedContainer,          static_cast<ClientObject::Messages::AddedToContainer *>            (0));
	m_callback->connect (*this, &SwgCuiPlanetMap::onAddedRemovedContainer,          static_cast<ClientObject::Messages::RemovedFromContainer *>        (0));
	m_callback->connect (*this, &SwgCuiPlanetMap::onLocationsRequested,             static_cast<PlanetMapManagerClient::Messages::LocationsRequested *>(0));
	m_callback->connect (*this, &SwgCuiPlanetMap::onLocationsReceived,              static_cast<PlanetMapManagerClient::Messages::LocationsReceived *> (0));
	m_callback->connect (*this, &SwgCuiPlanetMap::onObjectNameChange,               static_cast<ClientObject::Messages::NameChanged *>                 (0));
	m_callback->connect (*this, &SwgCuiPlanetMap::onWaypointChange,                 static_cast<ClientWaypointObject::Messages::ColorChanged *>        (0));
	m_callback->connect (*this, &SwgCuiPlanetMap::onWaypointChange,                 static_cast<ClientWaypointObject::Messages::LocationChanged *>     (0));
	m_callback->connect (*this, &SwgCuiPlanetMap::onWaypointChange,                 static_cast<ClientWaypointObject::Messages::ActivatedChanged *>    (0));
	m_callback->connect (*this, &SwgCuiPlanetMap::onGCWGroupValuesUpdated,          static_cast<GuildObject::Messages::GCWGroupScoreUpdatedThisGalaxy *>(0));

	m_pagePlanet->SetFocus ();

	// must set m_entriesRequested before calling requestEntries() because
	// calling requestEntries() will result in an immediate callback on the
	// onLocationsRequested() handler, which checks m_entriesRequested
	m_entriesRequested = true;
	PlanetMapManagerClient::requestEntries();

	m_buttonRefresh->SetVisible(!PlanetMapManagerClient::isRequestInProgress ());
}

//----------------------------------------------------------------------

void SwgCuiPlanetMap::performDeactivate      ()
{
	CuiManager::requestPointer(false);

	m_callback->disconnect (*this, &SwgCuiPlanetMap::onAddedRemovedContainer,          static_cast<ClientObject::Messages::AddedToContainer *>            (0));
	m_callback->disconnect (*this, &SwgCuiPlanetMap::onAddedRemovedContainer,          static_cast<ClientObject::Messages::RemovedFromContainer *>        (0));
	m_callback->disconnect (*this, &SwgCuiPlanetMap::onLocationsRequested,             static_cast<PlanetMapManagerClient::Messages::LocationsRequested *>(0));
	m_callback->disconnect (*this, &SwgCuiPlanetMap::onLocationsReceived,              static_cast<PlanetMapManagerClient::Messages::LocationsReceived *> (0));
	m_callback->disconnect (*this, &SwgCuiPlanetMap::onObjectNameChange,               static_cast<ClientObject::Messages::NameChanged *>                 (0));
	m_callback->disconnect (*this, &SwgCuiPlanetMap::onWaypointChange,                 static_cast<ClientWaypointObject::Messages::ColorChanged *>        (0));
	m_callback->disconnect (*this, &SwgCuiPlanetMap::onWaypointChange,                 static_cast<ClientWaypointObject::Messages::LocationChanged *>     (0));
	m_callback->disconnect (*this, &SwgCuiPlanetMap::onWaypointChange,                 static_cast<ClientWaypointObject::Messages::ActivatedChanged *>    (0));
	m_callback->disconnect (*this, &SwgCuiPlanetMap::onGCWGroupValuesUpdated,          static_cast<GuildObject::Messages::GCWGroupScoreUpdatedThisGalaxy *>(0));
	
	setIsUpdating (false);

	{
		clearMarkers ();

		for (ButtonVector::const_iterator it = m_freeListButton->begin (); it != m_freeListButton->end (); ++it)
		{
			UIButton * const button = *it;
			button->RemoveCallback (this);
			button->Detach (0);
		}
	}
	m_freeListButton->clear ();

	std::for_each (m_freeListText->begin (), m_freeListText->end (), UIBaseObject::DetachFunctor (0));
	m_freeListText->clear ();
}

//----------------------------------------------------------------------

void SwgCuiPlanetMap::OnButtonPressed        (UIWidget * context)
{
	if (context->GetParent () == m_pageMarkers)
	{
		UIButton * const button = safe_cast<UIButton *>(context);

		UIPoint pt = button->GetLocation () + UIPoint (0L, button->GetHeight ());// + (button->GetSize () / 2);
		computeCenterPoint (pt);
//		pt.y -= button->GetHeight ();

//		UIPoint offset = UIManager::gUIManager ().GetLastMouseCoord () - m_pagePlanet->GetWorldLocation ();
		
//		offset.y = -offset.y;

		m_centerPointOffset = (pt - m_pagePlanet->GetScrollLocation ()) - (m_pagePlanet->GetSize () / 2L);
		m_sliderZoom->SetValue (m_sliderZoom->GetValue () - (s_sliderRange / 5), true);
	}
	else if (m_buttonRefresh == context)
	{
		// must set m_entriesRequested before calling requestEntries() because
		// calling requestEntries() will result in an immediate callback on the
		// onLocationsRequested() handler, which checks m_entriesRequested
		m_entriesRequested = true;
		PlanetMapManagerClient::requestEntries();
	}
	else if (m_buttonZoom == context)
	{
		if(m_zoomLevel == ZL_Zone)
		{
			if(PlanetMapManager::sceneHasSupermap(Game::getSceneId()))
			{
				setZoneSupermapMode(true);
			}
		}
		else if(m_zoomLevel == ZL_ZoneSupermap)
		{
			setZoneSupermapMode(false);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiPlanetMap::OnSliderbarChanged (UIWidget * context)
{
	if (context == m_sliderZoom)
	{
		updateZoom ();
	}
}

//----------------------------------------------------------------------

void SwgCuiPlanetMap::OnGenericSelectionChanged (UIWidget * context)
{
	if (context == m_treeLocations)
	{
		UIDataSourceContainer * const dsc          = m_treeLocations->GetDataSourceContainer      ();
		NOT_NULL (dsc);
		UIDataSourceContainer * const dsc_selected = m_treeLocations->GetDataSourceContainerAtRow (m_treeLocations->GetLastSelectedRow ());

		if (dsc_selected)
			dsc->GetPathTo (m_lastSelectedLocationPath, dsc_selected);
		else
			m_lastSelectedLocationPath.clear();

		updateMarkers ();
	}
}

//----------------------------------------------------------------------

void SwgCuiPlanetMap::OnCheckboxSet             (UIWidget * context)
{
	handleCheckbox (*safe_cast<UICheckbox *>(context));
}

//----------------------------------------------------------------------

void SwgCuiPlanetMap::OnCheckboxUnset           (UIWidget * context)
{
	handleCheckbox (*safe_cast<UICheckbox *>(context));
}

//----------------------------------------------------------------------

void SwgCuiPlanetMap::handleCheckbox (const UICheckbox & box)
{
	if (&box == m_checkShowNames)
	{
		updateLabelVisibility ();
	}
	else if (&box == m_checkShowCities || &box == m_checkShowWaypoints || &box == m_checkShowGCWRegions)
	{
		updateMarkers ();
	}
}

//----------------------------------------------------------------------

void SwgCuiPlanetMap::OnPopupMenuSelection (UIWidget * context)
{
	UIPopupMenu * const pop = NON_NULL (safe_cast<UIPopupMenu *>(context));
	const std::string & selection = pop->GetSelectedName ();
	
	Unicode::String vstr;
	Vector2d pos;
	pop->GetProperty (Properties::Position, vstr);
	CuiUtils::ParseVector2d (vstr, pos);
	Unicode::String entryName;

	pop->GetProperty    (Properties::EntryName, entryName);

	std::string idStr;
	pop->GetPropertyNarrow  (Properties::EntryId, idStr);
	const NetworkId id (idStr);
	
	if (selection == PopupItems::waypoint_create)
	{
		ClientWaypointObject::requestWaypoint (StringId::decodeString (entryName), Vector (pos.x, 0.0f, pos.y));
	}
	else
	{
		ClientWaypointObject * const waypoint = dynamic_cast<ClientWaypointObject *>(NetworkIdManager::getObjectById (id));

		if (!waypoint)
		{
			WARNING (true, ("bad waypoint"));
			return;
		}

		if (selection == PopupItems::waypoint_destroy)
		{
			CuiInventoryManager::destroyObject (waypoint->getNetworkId (), false);
		}
		else if (selection == PopupItems::waypoint_activate)
		{
			waypoint->setWaypointActive (true);
		}
		else if (selection == PopupItems::waypoint_deactivate)
		{
			waypoint->setWaypointActive (false);
		}
		else if (selection == PopupItems::waypoint_set_name)
		{
			CuiRadialMenuManager::performDefaultAction (*waypoint, false, static_cast<int>(Cui::MenuInfoTypes::SET_NAME));
		}
	}
}

//----------------------------------------------------------------------

bool SwgCuiPlanetMap::OnMessage (UIWidget * context, const UIMessage & msg)
{
	const UIPoint & pt = msg.MouseCoords;
	if (context == m_pagePlanet)
	{
		const UIPoint & scrolledPt = m_pagePlanet->GetScrollLocation () + pt;
		if (msg.Type == UIMessage::LeftMouseDoubleClick)
		{
			UIWidget * const child = m_pagePlanet->GetWidgetFromPoint (scrolledPt, true);
			if (!child || child == m_pagePlanet)
			{
				const int sliderValue = m_sliderZoom->GetValue ();
				
				if (sliderValue > 0)
				{
					computeCenterPoint (scrolledPt);
					m_centerPointOffset = pt - (m_pagePlanet->GetSize () / 2L);
					m_sliderZoom->SetValue (m_sliderZoom->GetValue () - (s_sliderRange / 5), true);
				}
				else
				{
					return true;
//					computeCenterPoint (scrolledPt);
//					m_centerPointOffset = UIPoint::zero;
				}
				
				m_interpolating     = true;

				return false;
			}
		}
		else if (msg.Type == UIMessage::MouseWheel)
		{
			if (msg.Modifiers.isControlDown ())
			{
				computeCenterPoint (scrolledPt);
				m_centerPointOffset = pt - (m_pagePlanet->GetSize () / 2L);
				m_sliderZoom->SetValue (m_sliderZoom->GetValue () - (msg.Data * s_sliderRange / 10), true);
				return false;
			}
		}

		else if (msg.Type == UIMessage::MouseMove)
		{
			const Vector2d & v = transformPageToZone (scrolledPt);
			char buf [64];
			snprintf (buf, sizeof (buf), "%5.0f, %5.0f", v.x, v.y);
			m_textPosition->SetLocalText (Unicode::narrowToWide (buf));

			const Object * const player = Game::getPlayer ();
			if (player)
			{
				if(m_zoomLevel == ZL_ZoneSupermap)
				{
					m_textPosition->Clear ();
					m_textHeadingRange->Clear ();
				}
				else
				{
					Vector pos_w = GroundZoneManager::transformWorldLocationToZoneLocation(Game::getSceneId(), player->getPosition_w ());
					std::string zoneName;
					GroundZoneManager::getZoneName(Game::getSceneId(), player->getPosition_w(), zoneName);
					pos_w        = GroundZoneManager::adjustForNonSquareZones(zoneName, pos_w);
					m_textHeadingRange->SetLocalText (PlanetMapManagerClient::localizeHeadingRange (Vector2d (pos_w.x, pos_w.z), v));
				}
			}
		}
		else if (msg.Type == UIMessage::MouseExit)
		{
			m_textPosition->Clear ();
			m_textHeadingRange->Clear ();
		}

		else if (msg.Type == UIMessage::ContextRequest)
		{
			UIWidget * const child = m_pagePlanet->GetWidgetFromPoint (scrolledPt, true);
			if (!child || child == m_pagePlanet)
			{
				//-- create popup directly on planet map page
				Vector2d pos = transformPageToZone (scrolledPt);
				pos.x += m_zoneWorldCoordinateOffset.x;
				pos.y += m_zoneWorldCoordinateOffset.z;
				UIPopupMenu * const pop = createPopupForEntry (pos, Unicode::emptyString, NetworkId::cms_invalid);
				pop->SetLocation (m_pagePlanet->GetWorldLocation () + pt + pop->GetOffset ());
				UIManager::gUIManager ().PushContextWidget (*pop);
				return false;
			}
		}
		else if (msg.Type == UIMessage::Character)
		{
			if (msg.Keystroke == '-' || msg.Keystroke == '_')
			{
				computeCenterPoint (scrolledPt);
				m_centerPointOffset = pt - (m_pagePlanet->GetSize () / 2L);				
				m_sliderZoom->SetValue (m_sliderZoom->GetValue () - (s_sliderRange / 5), true);
			}
			else if (msg.Keystroke == '+' || msg.Keystroke == '=')
			{
				computeCenterPoint (scrolledPt);
				m_centerPointOffset = pt - (m_pagePlanet->GetSize () / 2L);				
				m_sliderZoom->SetValue (m_sliderZoom->GetValue () + (s_sliderRange / 5), true);
			}
		}
		
		return true;
	}
	
	if (context->GetParent () == m_pageMarkers && context->IsA (TUIButton))
	{
		UIButton * const button = safe_cast<UIButton *>(context);
		if (msg.Type == UIMessage::ContextRequest)
		{
			Unicode::String vstr;
			Unicode::String entryName;
			Vector2d pos;
			button->GetProperty (Properties::Position, vstr);
			CuiUtils::ParseVector2d (vstr, pos);
			button->GetProperty     (Properties::EntryName, entryName);

			std::string idStr;
			button->GetPropertyNarrow  (Properties::EntryId, idStr);
			const NetworkId id (idStr);
			std::string gcwCheck = Unicode::wideToNarrow(entryName);
			if(gcwCheck.find("gcw_") != std::string::npos)
			{
				StringId regionTitle("gcw_regions", gcwCheck);
				entryName = regionTitle.localize();
			}

			//-- create popup directly on planet map marker button (icons, waypoints, and other markers)
			UIPopupMenu * const pop = createPopupForEntry (pos, entryName, id);			
			pop->SetLocation (button->GetWorldLocation () + pt + pop->GetOffset ());
			UIManager::gUIManager ().PushContextWidget (*pop);

			return false;
		}
		else if (msg.Type == UIMessage::MouseEnter)
		{
			UIText * const text = (*m_buttonTextMap) [button];
			NOT_NULL (text);
			text->SetEnabled (true);
		}
		else if (msg.Type == UIMessage::MouseExit)
		{
			UIText * const text = (*m_buttonTextMap) [button];
			NOT_NULL (text);

			int cat = 0;
			button->GetPropertyInteger (Properties::Category, cat);
			text->SetEnabled (m_checkShowNames->IsChecked () || cat == PlanetMapManager::getCityCategory ());
		}

		return true;
	}

	if (context == m_treeLocations)
	{
		if (msg.Type == UIMessage::LeftMouseDoubleClick)
		{
			const int selectedTreeRow = m_treeLocations->GetLastSelectedRow ();
			UIDataSourceContainer * const dsc = m_treeLocations->GetDataSourceContainerAtRow (selectedTreeRow);
			
			if (dsc)
			{
				Unicode::String vstr;
				if (dsc->GetProperty (Properties::Position, vstr))
				{
					Vector savedPos;
					CuiUtils::ParseVector (vstr, savedPos);
					const UIPoint & savedPt  = transformWorldToPage (savedPos);
					computeCenterPoint (savedPt);
					m_centerPointOffset = UIPoint::zero;
					m_interpolating     = true;
					m_sliderZoom->SetValue (m_sliderZoom->GetValue () + (s_sliderRange / 5), true);
					return false;
				}
			}
		}
		else if (msg.Type == UIMessage::ContextRequest)
		{
			long row = -1;
			if (m_treeLocations->GetRowFromPoint (pt, row, 0))
			{
				const UIDataSourceContainer * const dsc = m_treeLocations->GetDataSourceContainerAtRow  (row);
				
				if (dsc)
				{
					Unicode::String vstr;
					if (dsc->GetProperty (Properties::Position, vstr))
					{
						Vector2d pos;
						CuiUtils::ParseVector2d (vstr, pos);
						Unicode::String entryName;
						if (dsc->GetProperty   (Properties::EntryName, entryName) && !entryName.empty ())
						{
							std::string idStr;
							dsc->GetPropertyNarrow  (Properties::EntryId, idStr);
							const NetworkId id (idStr);
							
							//-- create popup on tree view

							UIPopupMenu * const pop = createPopupForEntry (pos, entryName, id);
							pop->SetLocation (context->GetWorldLocation () + pt + pop->GetOffset ());
							UIManager::gUIManager ().PushContextWidget (*pop);
							return false;
						}						
					}
				}
			}
		}
		
		return true;
	}
	
	return true;
}

//----------------------------------------------------------------------

UIPopupMenu * SwgCuiPlanetMap::createPopupForEntry (const Vector2d & pos, const Unicode::String & entryName, const NetworkId & id)
{
	UIPopupMenu * const pop = new UIPopupMenu(&getPage());
	pop->AddCallback (this);
	
	pop->SetStyle (m_pagePlanet->FindPopupStyle ());
	
	ClientWaypointObject * waypoint = 0;
	
	if (!entryName.empty ())
		waypoint = dynamic_cast<ClientWaypointObject *>(NetworkIdManager::getObjectById (id));

	UIDataSource * ds = 0;	
	if (entryName.empty ())
	{
		const std::string & sceneId = Game::getSceneId ();
		const Unicode::String & localPlanetName = StringId ("planet_n", sceneId).localize ();
		ds = pop->AddItem ("label", localPlanetName);
	}
	else
		ds = pop->AddItem ("label", entryName);

	pop->SetOffsetIndex (1);

	NOT_NULL (ds);
	ds->SetPropertyBoolean (UIPopupMenu::DataProperties::IsLabel, false);

	if (waypoint)
	{
		if (waypoint->isWaypointActive ())
			pop->AddItem (PopupItems::waypoint_deactivate, CuiStringIdsPlanetMap::popup_waypoint_deactivate.localize ());
		else
			pop->AddItem (PopupItems::waypoint_activate,   CuiStringIdsPlanetMap::popup_waypoint_activate.localize ());

		if (!waypoint->isPoi() && !waypoint->isGroupWaypoint())
		{
			pop->AddItem (PopupItems::waypoint_set_name,       CuiStringIdsPlanetMap::popup_waypoint_set_name.localize ());
			pop->AddItem (PopupItems::waypoint_destroy,        CuiStringIdsPlanetMap::popup_waypoint_destroy.localize ());
		}
	}
	else
		pop->AddItem (PopupItems::waypoint_create, CuiStringIdsPlanetMap::popup_waypoint_create.localize ());
	
	Unicode::String vstr;
	CuiUtils::FormatVector2d (vstr, pos);

	pop->SetProperty       (Properties::Position,  vstr);
	pop->SetProperty       (Properties::EntryName, entryName);
	pop->SetPropertyNarrow (Properties::EntryId,   id.getValueString ());

	return pop;
}

//----------------------------------------------------------------------

void SwgCuiPlanetMap::updateZoom ()
{
	getPage ().ForcePackChildren ();

	const float f = static_cast<float>(s_sliderRange - m_sliderZoom->GetValue ()) * s_sliderFactor;

	UIPoint extent = UIPoint::lerp (m_pagePlanet->GetSize (), m_maxPlanetScrollExtent, f);
	extent.y = extent.x = std::max (extent.y, extent.x);
	m_pagePlanet->SetMinimumScrollExtent (extent);
	m_pagePlanet->SetScrollExtent        (extent);

	m_interpolating = false;
	const UIPoint & scrollLocation = getTargetPoint ();
	m_pagePlanet->ScrollToPoint (scrollLocation);

	m_ignoreNextScroll = true;

	m_pagePlanet->SetScrollSizes (m_pagePlanet->GetSize () / 8L, UISize (4L, 4L));

	//-- update here icon

	updateMarkers ();
}

//----------------------------------------------------------------------

void SwgCuiPlanetMap::updatePlayerIcon ()
{
	if(m_zoomLevel == ZL_ZoneSupermap)
		return;

	CreatureObject * const player = Game::getPlayerCreature ();
	if (player)
	{
		const UIPoint & pt = transformWorldToPage (player->getPosition_w ());
		m_iconHere->SetLocation (pt - (m_iconHere->GetSize () / 2L));

		const float   frameK_w   = player->getObjectFrameK_w ().theta ();

		m_iconHere->SetRotation (frameK_w / PI_TIMES_2);
	}
}

//----------------------------------------------------------------------

void SwgCuiPlanetMap::update (float deltaTimeSecs)
{
	CuiMediator::update (deltaTimeSecs);

	CreatureObject * const player = Game::getPlayerCreature ();
	if (!player)
		return;

	if (!PlanetMapManagerClient::isPlanetaryMapEnabled(Game::getSceneId(), player->getPosition_w()))
	{
		deactivate();
		return;
	}

	const UIPoint & scrollLocation = m_pagePlanet->GetScrollLocation ();

	if (!m_ignoreNextScroll)
	{
		if (m_lastScrollLocation != scrollLocation)
		{
			const UIPoint & center = scrollLocation + (m_pagePlanet->GetSize () / 2L);
			computeCenterPoint (center);
			m_centerPointOffset = UIPoint::zero;
			m_interpolating = false;
		}
	}

	m_lastScrollLocation = scrollLocation;
	m_ignoreNextScroll   = false;

	if (m_interpolating)
	{
		const UIPoint & oldScrollLocation = m_pagePlanet->GetScrollLocation ();
		const UIPoint & scrollLocationTarget = getTargetPoint ();

		UIPoint newPoint = oldScrollLocation;
		
		static float factor = 1000.0f;
		const long delta = static_cast<long>(std::max (10.0f, deltaTimeSecs * factor));

		if (scrollLocationTarget.x > newPoint.x)
			newPoint.x = std::min (scrollLocationTarget.x, newPoint.x + delta);
		else if (scrollLocationTarget.x < newPoint.x)
			newPoint.x = std::max (scrollLocationTarget.x, newPoint.x - delta);

		if (scrollLocationTarget.y > newPoint.y)
			newPoint.y = std::min (scrollLocationTarget.y, newPoint.y + delta);
		else if (scrollLocationTarget.y < newPoint.y)
			newPoint.y = std::max (scrollLocationTarget.y, newPoint.y - delta);

		if (newPoint == oldScrollLocation || newPoint == scrollLocationTarget)
		{
			m_pagePlanet->ScrollToPoint (scrollLocationTarget);
			m_interpolating = false;
		}
		else
			m_pagePlanet->ScrollToPoint (newPoint);

		m_lastScrollLocation = m_pagePlanet->GetScrollLocation ();
	}

	getPage ().PackIfDirty ();

	const UIPoint & size = getPage ().GetSize ();

	if (m_lastSize != size)
	{
		updateZoom    ();
		m_lastSize = size;
	}
	else
		updatePlayerIcon ();
}

//----------------------------------------------------------------------

void SwgCuiPlanetMap::computeCenterPoint (const UIPoint & center)
{
	UISize extent;
	m_pagePlanet->GetScrollExtent (extent);
	
	if (extent.x)
		m_centerPointTarget.x = static_cast<float>(center.x) / static_cast<float>(extent.x);
	if (extent.y)
		m_centerPointTarget.y = static_cast<float>(center.y) / static_cast<float>(extent.y);
}

//----------------------------------------------------------------------

UIPoint SwgCuiPlanetMap::getTargetPoint () const
{
	UISize extent;
	m_pagePlanet->GetScrollExtent (extent);
	const UIPoint centerPoint    = UIPoint (static_cast<long>(extent.x * m_centerPointTarget.x), static_cast<long>(extent.y * m_centerPointTarget.y));	
	return centerPoint - ((m_pagePlanet->GetSize () / 2L) + m_centerPointOffset);
}

//----------------------------------------------------------------------

UIPoint SwgCuiPlanetMap::transformWorldToPage      (const Vector & vect) const
{
	UIPoint pt;

	Vector location = GroundZoneManager::transformWorldLocationToZoneLocation(Game::getSceneId(), vect);
	std::string zoneName;
	Object const * const player = Game::getPlayer();
	BuildoutArea const * const ba = GroundZoneManager::getZoneName(Game::getSceneId(), player->getPosition_w(), zoneName);

	if (ba)
		zoneName = Game::getSceneId() + "__" + zoneName;

	UISize extent;
	m_pagePlanet->GetScrollExtent (extent);

	if(ms_mapAdjustments.find(zoneName) != ms_mapAdjustments.end())
	{
		//these are the unused pixels from the 512 by 512 map
		UnusedMapPixels unusedPixels = ms_mapAdjustments[zoneName];

		//adjust for the actual map size on screen
		unusedPixels.left   = static_cast<int>(static_cast<float>(unusedPixels.left)   * static_cast<float>(extent.x) / ms_mapTextureSize);
		unusedPixels.right  = static_cast<int>(static_cast<float>(unusedPixels.right)  * static_cast<float>(extent.x) / ms_mapTextureSize);
		unusedPixels.top    = static_cast<int>(static_cast<float>(unusedPixels.top)    * static_cast<float>(extent.y) / ms_mapTextureSize);
		unusedPixels.bottom = static_cast<int>(static_cast<float>(unusedPixels.bottom) * static_cast<float>(extent.y) / ms_mapTextureSize);
		int const xMapSize = (extent.x - unusedPixels.left - unusedPixels.right);
		int const zMapSize = (extent.y - unusedPixels.top  - unusedPixels.bottom);

		//resize to map's size
		pt.x = static_cast<long>(location.x * xMapSize / m_zoneSize.x);
		pt.y = static_cast<long>(location.z * zMapSize / m_zoneSize.y);

		//convert to map space (0, 0 at top left)
		pt.x = pt.x    + (xMapSize / 2);
		pt.y = (-pt.y) + (zMapSize / 2);

		//adjust for unused pixels
		pt.x += unusedPixels.left;
		pt.y += unusedPixels.top;
	}
	else
	{
		Vector const & v = location + Vector (m_zoneSize.x * 0.5f, 0.0f, m_zoneSize.y * 0.5f);
		pt.x = static_cast<long>(static_cast<float>(extent.x) * (v.x / m_zoneSize.x));
		pt.y = extent.y - (static_cast<long>(static_cast<float>(extent.y) * (v.z / m_zoneSize.y)));
	}

	NamedPointMap::const_iterator const offset_it = ms_mapOffset.find(zoneName);
	if (offset_it != ms_mapOffset.end())
	{	
		UIPoint const & offset = offset_it->second;
		UIPoint const scaledOffset = offset * (std::min(extent.x, extent.y) * 0.5f / ms_mapTextureSize);
		pt += scaledOffset;
	}

	return pt;
}

//----------------------------------------------------------------------

UIPoint SwgCuiPlanetMap::transformWorldToPage (const Vector2d & vect) const
{
	return transformWorldToPage (Vector (vect.x, 0.0f, vect.y));
}

//----------------------------------------------------------------------

Vector SwgCuiPlanetMap::transformPageToZone3d    (const UIPoint & pto)  const
{
	UIFloatPoint fp;
	UISize extent;
	m_pagePlanet->GetScrollExtent (extent);
	
	UIPoint pt = pto;
	NamedPointMap::const_iterator const offset_it = ms_mapOffset.find(m_lastZone);
	if (offset_it != ms_mapOffset.end())
	{	
		UIPoint const & offset = offset_it->second;
		UIPoint const scaledOffset = offset * (std::min(extent.x, extent.y) * 0.5f / ms_mapTextureSize);
		pt -= scaledOffset;
	}

	if (extent.x)
		fp.x = static_cast<float>(pt.x) / static_cast<float>(extent.x);
	if (extent.y)
		fp.y = static_cast<float>(extent.y - pt.y) / static_cast<float>(extent.y);

	Vector v (fp.x * m_zoneSize.x, 0.0f, fp.y * m_zoneSize.y);

	v.x -= m_zoneSize.x * 0.5f;
	v.z -= m_zoneSize.y * 0.5f;

	return v;
}

//----------------------------------------------------------------------

Vector2d SwgCuiPlanetMap::transformPageToZone (const UIPoint & pt) const
{
	const Vector & v = transformPageToZone3d (pt);
	return Vector2d (v.x, v.z);
}

//----------------------------------------------------------------------

void SwgCuiPlanetMap::reset()
{
	UIDataSourceContainer * const dsc = m_treeLocations->GetDataSourceContainer ();
	dsc->Attach (0);
	m_treeLocations->SetDataSourceContainer (0);
	dsc->Clear ();
	
	typedef stdmap<Unicode::String, uint8>::fwd CategoryMap;
	CategoryMap categoryMap;
	
	{
		PlanetMapManagerClient::CategoryVector sv;
		PlanetMapManagerClient::getCategories (sv);	
		sv.push_back (PlanetMapManager::getWaypointCategory ());
		sv.push_back (PlanetMapManager::getGCWRegionCategory());
		for (PlanetMapManagerClient::CategoryVector::const_iterator it = sv.begin (); it != sv.end (); ++it)
		{
			const uint8 category = (*it);
			categoryMap.insert (std::make_pair (PlanetMapManagerClient::findCategoryString (category), category));
		}
	}
	
	for (CategoryMap::const_iterator it = categoryMap.begin (); it != categoryMap.end (); ++it)
	{
		const uint8 category                      = (*it).second;
		const Unicode::String & localizedCategory = (*it).first;
		const std::string & categoryName          = PlanetMapManager::findCategoryName (category);

		const bool isGCWCategory				  = (category == PlanetMapManager::getGCWRegionCategory());
		
		m_treeLocations->SetVisible (true);
		
		PlanetMapManagerClient::MapLocationVector mlv;
		PlanetMapManagerClient::getMapLocations (category, mlv);
		
		UIDataSourceContainer * const dsc_category = new UIDataSourceContainer;
		dsc_category->SetName            (categoryName);
		dsc_category->SetProperty        (UITreeView::DataProperties::LocalText, localizedCategory);
		dsc_category->SetPropertyBoolean (Properties::IsCategory,   true);
		dsc_category->SetPropertyInteger (Properties::Category, category);
		
		dsc->AddChild (dsc_category);
		
		typedef stdmap<uint8, UIDataSourceContainer *>::fwd SubCategoryDataMap;
		SubCategoryDataMap scdm;
		
		std::map<std::string, MapLocation const > childMap;


		for (PlanetMapManagerClient::MapLocationVector::const_iterator mit = mlv.begin (); mit != mlv.end (); ++mit)
		{
			const MapLocation & ml = (*mit);

			if(isGCWCategory)
			{
				StringId categoryName(ms_regionNameTable, Unicode::wideToNarrow(ml.m_locationName));
				childMap.insert(std::make_pair(Unicode::wideToNarrow(categoryName.localize()), ml));
				continue;
			}
			
			UIDataSourceContainer * const child = new UIDataSourceContainer;
			if (!ml.m_subCategory)
				dsc_category->AddChild (child);
			else
			{
				UIDataSourceContainer * dsc_subCategory = scdm [ml.m_subCategory];
				
				if (!dsc_subCategory)
				{
					dsc_subCategory = new UIDataSourceContainer;
					dsc_subCategory->SetName            (PlanetMapManager::findCategoryName (ml.m_subCategory));
					dsc_subCategory->SetProperty        (UITreeView::DataProperties::LocalText, PlanetMapManagerClient::findCategoryString (ml.m_subCategory));
					dsc_subCategory->SetPropertyBoolean (Properties::IsSubCategory,   true);
					dsc_subCategory->SetPropertyInteger (Properties::Category,        ml.m_category);
					dsc_subCategory->SetPropertyInteger (Properties::SubCategory,     ml.m_subCategory);
					
					dsc_category->AddChild (dsc_subCategory);
					scdm [ml.m_subCategory] = dsc_subCategory;
				}
				dsc_subCategory->AddChild (child);
			}
			

			child->SetName        (Unicode::wideToNarrow (ml.m_locationName));
			child->SetProperty    (UITreeView::DataProperties::LocalText, PlanetMapManagerClient::getLocalizedEncoded (ml.m_locationName));
			child->SetProperty        (Properties::EntryName,                 ml.m_locationName);
			child->SetPropertyNarrow  (Properties::EntryId,                   ml.m_locationId.getValueString ());
			child->SetPropertyInteger (Properties::Category,                  ml.m_category);
			child->SetPropertyInteger (Properties::SubCategory,               ml.m_subCategory);

			if (ml.isActive ())
				child->SetProperty (UITreeView::DataProperties::Icon, m_iconPathActive);
			else if (ml.isInactive ())
				child->SetProperty (UITreeView::DataProperties::Icon, m_iconPathInactive);
			else
				child->RemoveProperty (UITreeView::DataProperties::Icon);

			Unicode::String vstr;
			CuiUtils::FormatVector2d (vstr, ml.m_location);
			child->SetProperty       (Properties::Position, vstr);
		}

		std::map<std::string, MapLocation const >::const_iterator childIter = childMap.begin();
		for(; childIter != childMap.end(); ++childIter)
		{
			const MapLocation & ml = childIter->second;

			UIDataSourceContainer * const child = new UIDataSourceContainer;

			child->SetName        (Unicode::wideToNarrow (ml.m_locationName));

			StringId categoryName(ms_regionNameTable, Unicode::wideToNarrow(ml.m_locationName));
			child->SetProperty    (UITreeView::DataProperties::LocalText, categoryName.localize());
			child->SetProperty        (Properties::EntryName,                 ml.m_locationName);
			child->SetPropertyNarrow  (Properties::EntryId,                   ml.m_locationId.getValueString ());
			child->SetPropertyInteger (Properties::Category,                  ml.m_category);
			child->SetPropertyInteger (Properties::SubCategory,               ml.m_subCategory);

			if (ml.isActive ())
				child->SetProperty (UITreeView::DataProperties::Icon, m_iconPathActive);
			else if (ml.isInactive ())
				child->SetProperty (UITreeView::DataProperties::Icon, m_iconPathInactive);
			else
				child->RemoveProperty (UITreeView::DataProperties::Icon);

			Unicode::String vstr;
			CuiUtils::FormatVector2d (vstr, ml.m_location);
			child->SetProperty       (Properties::Position, vstr);

			dsc_category->AddChild (child);
		}
		
	}
	
	m_treeLocations->SetDataSourceContainer (dsc);
	dsc->Detach (0);

	UIDataSourceContainer * const dsc_selected = safe_cast<UIDataSourceContainer *>(dsc->GetObjectFromPath (m_lastSelectedLocationPath.c_str (), TUIDataSourceContainer));
	if (dsc_selected)
	{
		int row = 0;
		UITreeView::DataNode * const node = m_treeLocations->FindDataNodeByDataSource (*dsc_selected, row);

		if (node)
		{
			m_treeLocations->ExpandParentNodes   (*node);
			m_treeLocations->SelectRow           (row);
			m_treeLocations->ScrollToRow         (row);
		}
		else
		{
			if (!m_lastSelectedLocationPath.empty()) 
			{
				m_lastSelectedLocationPath.clear();
				m_treeLocations->SelectRow(-1);

				// must set m_entriesRequested before calling requestEntries() because
				// calling requestEntries() will result in an immediate callback on the
				// onLocationsRequested() handler, which checks m_entriesRequested
				m_entriesRequested = true;
				PlanetMapManagerClient::requestEntries();
			}
		}
	}
	
	updateMarkers ();
}

//----------------------------------------------------------------------

void SwgCuiPlanetMap::clearMarkers ()
{
	const UIBaseObject::UIObjectList & olist = m_pageMarkers->GetChildrenRef ();

	for (UIBaseObject::UIObjectList::const_iterator it = olist.begin (); it != olist.end (); ++it)
	{
		UIBaseObject * const obj = *it;
		if (obj->IsA (TUIText))
		{
			obj->Attach (0);
			m_freeListText->push_back (safe_cast<UIText *>(obj));
		}
		else if (obj->IsA (TUIButton))
		{
			obj->Attach (0);
			m_freeListButton->push_back (safe_cast<UIButton *>(obj));
		}
	}

	m_pageMarkers->Clear ();
	m_buttonTextMap->clear ();
}

//----------------------------------------------------------------------

void SwgCuiPlanetMap::updateMarkers ()
{
	updatePlayerIcon ();

	clearMarkers ();

	if(m_zoomLevel == ZL_ZoneSupermap)
		return;

	bool isSubCategory = false;
	bool isCategory    = false;

	uint8 const cityCategory = PlanetMapManager::getCityCategory();
	uint8 const waypointCategory = PlanetMapManager::getWaypointCategory();
	uint8 const poiCategory = PlanetMapManager::getPoiCategory();
	uint8 const GCWRegionCategory = PlanetMapManager::getGCWRegionCategory();

	int const selectedTreeRow = m_treeLocations->GetLastSelectedRow();
	UIDataSourceContainer * const dsc = m_treeLocations->GetDataSourceContainerAtRow (selectedTreeRow);
	uint8 category = 0;
	
	if (dsc)
	{			
		Unicode::String entryName;
				
		dsc->GetPropertyBoolean (Properties::IsSubCategory, isSubCategory);
		dsc->GetPropertyBoolean (Properties::IsCategory,    isCategory);

		std::string idStr;
		dsc->GetPropertyNarrow  (Properties::EntryId, idStr);
		const NetworkId id (idStr);
			
		if (!isCategory && !isSubCategory)
			dsc->GetProperty (Properties::EntryName, entryName);
		
		int subCategory = 0;
		dsc->GetPropertyInteger (Properties::SubCategory, subCategory);
		
		int tmpCategory = 0;
		dsc->GetPropertyInteger (Properties::Category,    tmpCategory);
		category = static_cast<uint8>(tmpCategory);
				
		const bool isWaypoint = !isCategory && !isSubCategory && (category == waypointCategory);
		bool skipActiveWaypoints = !isWaypoint;
		
		if(category == GCWRegionCategory)
			setupMarkersForType (category, static_cast<uint8>(subCategory), id, false, skipActiveWaypoints, Unicode::wideToNarrow(entryName));
		else
			setupMarkersForType (category, static_cast<uint8>(subCategory), id, false, skipActiveWaypoints);

		setupMarkersForType (poiCategory, 0, NetworkId::cms_invalid, true, false);
	}

	//-- show all cities unless the player has a specific city selected
	if (m_checkShowCities->IsChecked () && category != cityCategory)
		setupMarkersForType (PlanetMapManager::getCityCategory (), 0, NetworkId::cms_invalid, false, true);

	//-- show all waypoints unless the player has a specific waypoint selected
	if (m_checkShowWaypoints->IsChecked () && category != waypointCategory)
		setupMarkersForType (waypointCategory, 0, NetworkId::cms_invalid, false, true);

	// -- show all GCW regions
	if (m_checkShowGCWRegions->IsChecked() && category != GCWRegionCategory)
		setupMarkersForType (GCWRegionCategory, 0, NetworkId::cms_invalid, false, false);

	//-- show all active waypoints if the selected treeview item is a category or subcategory or not a waypoint
	if (isSubCategory || isCategory || category != waypointCategory)
		setupMarkersForType (waypointCategory, 0, NetworkId::cms_invalid, true, false);
}

//----------------------------------------------------------------------

void SwgCuiPlanetMap::setupMarkersForType (uint8 category, uint8 subCategory, const NetworkId & id, bool activeWaypointsOnly, bool skipActiveWaypoints, std::string const locationName)
{
	UISize extent;
	m_pagePlanet->GetScrollExtent (extent);

	static PlanetMapManagerClient::MapLocationVector mlv;
	mlv.clear ();
	PlanetMapManagerClient::getMapLocations (category, mlv);
	
	UIButton * sampleButton = m_buttonSample;

	{
		const SampleButtonMap::const_iterator it = m_sampleButtonMap->find (category);
		if (it !=  m_sampleButtonMap->end ())
			sampleButton = (*it).second;
	}
	
	const bool isCityCategory     = category == PlanetMapManager::getCityCategory ();
	const bool isGCWCategory      = category == PlanetMapManager::getGCWRegionCategory();
	const bool isWaypointCategory = (!isGCWCategory && !isCityCategory && (category == PlanetMapManager::getWaypointCategory ())) || (category == PlanetMapManager::getPoiCategory ());
	
	NOT_NULL (sampleButton);
	
	for (PlanetMapManagerClient::MapLocationVector::const_iterator it = mlv.begin (); it != mlv.end (); ++it)
	{
		const MapLocation & ml = (*it);

		int imperialScore = 50;
		int rebelScore = 0;
		
		if (subCategory && subCategory != ml.m_subCategory)
			continue;
		
		if (id.isValid () && id != ml.m_locationId)
			continue;

		UIPoint pt = transformWorldToPage (ml.m_location);

		if (pt.x < 0 || pt.x > extent.x || pt.y < 0 || pt.y > extent.y)
		{
			WARNING (true, ("SwgCuiPlanetMap bad entry position [%s] [%5.0f,%5.0f] [%d,%d] pagesize=[%d,%d]", 
				Unicode::wideToNarrow (ml.m_locationName).c_str (), ml.m_location.x, ml.m_location.y, pt.x, pt.y, extent.x, extent.y));
			continue;
		}

		if(isGCWCategory)
		{
			std::string narrowName = Unicode::wideToNarrow(ml.m_locationName);

			if(!locationName.empty() && _stricmp(locationName.c_str(), narrowName.c_str()) != 0)
				continue;

			if(GuildObject::getGuildObject() == NULL)
				continue;

			// Guild Object should always be valid. It's a universe object.
			std::map<std::string, int> const & GCWScores = GuildObject::getGuildObject()->getGcwImperialScorePercentileThisGalaxy();
			std::map<std::string, int>::const_iterator iter = GCWScores.find(narrowName);
			if(iter != GCWScores.end())
			{
				imperialScore = iter->second;
				rebelScore = 100 - imperialScore;
			}
			else
				DEBUG_WARNING(true, ("SwgCuiPlanetMap: Could not find a GCW Score for key [%s]", narrowName));
		}

		ClientWaypointObject * waypoint = 0;
		if (isWaypointCategory)
		{
			waypoint = dynamic_cast<ClientWaypointObject *>(NetworkIdManager::getObjectById (ml.m_locationId));

			if (waypoint)
			{
				const bool active = waypoint->isWaypointActive ();

				if(waypoint->getColor() == "invisible")
					continue;

				if (active && skipActiveWaypoints)
					continue;

				if (!active && activeWaypointsOnly)
					continue;
			}
			else
				WARNING (true, ("SwgCuiPlanetMap placing marker for waypoint [%s] [%s] but no such waypoint found.", ml.m_locationId.getValueString ().c_str (), Unicode::wideToNarrow (ml.m_locationName)));
		}

		UIButton * button = 0;
		UIText * text     = 0;

		if (m_freeListButton->empty ())
		{
			button = safe_cast<UIButton *>(sampleButton->DuplicateObject ());
			button->Attach (0);
			button->AddCallback        (this);
		}
		else
		{
			button = m_freeListButton->back ();
			m_freeListButton->pop_back ();
		}

		if (m_freeListText->empty ())
		{
			text     = safe_cast<UIText *>(m_textSample->DuplicateObject ());
			text->SetPreLocalized (true);
			text->Attach (0);
		}
		else
		{
			text = m_freeListText->back ();
			m_freeListText->pop_back ();
		}
		
		Unicode::String vstr;
		CuiUtils::FormatVector2d  (vstr, ml.m_location);
		button->SetProperty       (Properties::Position, vstr);
		button->SetProperty       (Properties::EntryName, ml.m_locationName);
		button->SetPropertyNarrow (Properties::EntryId,   ml.m_locationId.getValueString ());
				
		m_buttonTextMap->insert (std::make_pair (button, text));

		if (isWaypointCategory)
			m_pageMarkers->InsertChildAfter (button, 0);
		else
			m_pageMarkers->AddChild (button);

		m_pageMarkers->AddChild (text);
		
		button->CopyPropertiesFrom (*sampleButton);
		button->SetContextCapable  (true, true);

		button->Link ();
		text->Link ();

		button->Detach (0);
		text->Detach   (0);
		
		bool waypointColorSet = false;
		button->CancelEffector (*m_effectorWaypointFlash);
		button->SetPropertyInteger (Properties::Category, ml.m_category);

		if (waypoint)
		{
			button->SetColor (CuiUtils::convertColor (waypoint->getColorArgb ()));
			waypointColorSet = true;
			if (waypoint->isWaypointActive ())
				button->ExecuteEffector (*m_effectorWaypointFlash);
		}
	
		if (!waypointColorSet)
			button->SetColor (UIColor::white);
	
		button->SetVisible (true);
		text->SetVisible   (true);

		text->SetOpacity   (0.0f);
		if (m_checkShowNames->IsChecked () || ml.m_category == PlanetMapManager::getCityCategory () || ml.m_category == PlanetMapManager::getGCWRegionCategory())
		{
			text->SetEnabled   (false);
			text->SetEnabled   (true);
		}
		else
		{
			text->SetEnabled   (true);
			text->SetEnabled   (false);
		}
		
		Unicode::String localName;
		if(isGCWCategory)
		{
			char buffer[32];
			memset(buffer, 0, 32);

			if(imperialScore > rebelScore)
			{
				sprintf(buffer, "%d%%", imperialScore);
				button->SetProperty(UIButton::PropertyName::Icon, RegionIconStyles::imperial);

			}
			else if (rebelScore > imperialScore)
			{
				sprintf(buffer, "%d%%", rebelScore);
				button->SetProperty(UIButton::PropertyName::Icon, RegionIconStyles::rebel);
			}
			else // tied
			{
				sprintf(buffer, "%d%%", rebelScore);
				button->SetProperty(UIButton::PropertyName::Icon, RegionIconStyles::neutral);
			}


			localName = Unicode::narrowToWide(buffer);
		}
		else
			localName = PlanetMapManagerClient::getLocalizedEncoded (ml.m_locationName);

		text->SetLocalText (localName);
		text->SizeToContent ();
		text->SetPropertyInteger (Properties::Category, ml.m_category);

		UIPoint buttonLoc = pt;
		
		if(isGCWCategory)
		{
			UISize currentSize = button->GetSize();

			UISize extent;
			m_pagePlanet->GetScrollExtent (extent);
			float halfXZone = m_zoneSize.x / 2;
			float halfZZone = m_zoneSize.y / 2;
			
			float regionXPercentage = ml.m_size / halfXZone;
			float regionZPercentage = ml.m_size / halfZZone;
			
			// Region size
			currentSize.x = static_cast<UIScalar>(static_cast<float>(extent.x) * regionXPercentage);
			currentSize.y = static_cast<UIScalar>(static_cast<float>(extent.y) * regionZPercentage);
			
			button->SetSize(currentSize);

			buttonLoc.x -= currentSize.x / 2L;
			buttonLoc.y -= currentSize.y / 2L;

		}
		else
			buttonLoc.y -= button->GetHeight ();
		
		button->SetLocation (buttonLoc);
		
		UIPoint textLoc;
				
		textLoc.y = pt.y;
		textLoc.x = pt.x - text->GetWidth () / 2L;
		
		if ((textLoc.x + text->GetWidth ()) > extent.x)
			textLoc.x = extent.x - text->GetWidth ();
		if ((textLoc.y + text->GetHeight ()) > extent.y)
			textLoc.y = extent.y - text->GetHeight ();
		
		textLoc.x = std::max (0L, textLoc.x);
		textLoc.y = std::max (0L, textLoc.y);
		
		text->SetLocation (textLoc);
	}
	
	{
		for (ButtonTextMap::const_iterator bit = m_buttonTextMap->begin (); bit != m_buttonTextMap->end (); ++bit)
		{
			UIText * const text = (*bit).second;
			text->SetTextFlag(UIText::TF_drawLast, true);
		}
	}
	 
}
//----------------------------------------------------------------------

void SwgCuiPlanetMap::updateLabelVisibility () const
{
	const bool textVisible = m_checkShowNames->IsChecked ();

	const UIBaseObject::UIObjectList & olist = m_pageMarkers->GetChildrenRef ();

	for (UIBaseObject::UIObjectList::const_iterator it = olist.begin (); it != olist.end (); ++it)
	{
		UIBaseObject * const obj = *it;
		if (obj->IsA (TUIText))
		{
			int cat = 0;
			obj->GetPropertyInteger (Properties::Category, cat);
			safe_cast<UIText *>(obj)->SetEnabled (textVisible || cat == PlanetMapManager::getCityCategory ());
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiPlanetMap::setupCurrentZone ()
{
	CreatureObject * const player = Game::getPlayerCreature ();
	if (!player)
		return;

	Vector const & pos_w = player->getPosition_w();

	std::string zoneName;
	BuildoutArea const * const ba = GroundZoneManager::getZoneName(Game::getSceneId(), pos_w, zoneName);

	if(m_zoomLevel == ZL_Zone)
	{
		if(PlanetMapManager::sceneHasSupermap(zoneName))
		{
			m_buttonZoom->SetVisible(true);
			m_buttonZoom->SetLocalText(CuiStringIdsPlanetMap::show_planet.localize ());
		}
		else
		{
			m_buttonZoom->SetVisible(false);
		}
	}
	else if(m_zoomLevel == ZL_Planet)
	{
		m_buttonZoom->SetVisible(false);
	}
	else if(m_zoomLevel == ZL_ZoneSupermap)
		zoneName = PlanetMapManager::getSceneSupermap(Game::getSceneId());

	if (ba)
		zoneName = Game::getSceneId() + "__" + zoneName;

	Unicode::String windowTitle = StringId ("zone_n", zoneName).localize ();

	// include GCW planetary score in window title
	{
		GuildObject const * const go = GuildObject::getGuildObject();
		if (go)
		{
			std::map<std::string, int> const & gcwScoreMap = go->getGcwGroupImperialScorePercentileThisGalaxy();
			std::map<std::string, int>::const_iterator const iterFind = gcwScoreMap.find(zoneName);
			if (iterFind != gcwScoreMap.end())
			{
				char buffer[64];
				snprintf(buffer, sizeof(buffer)-1, " (R:%d%% I:%d%%)", (100 - iterFind->second), iterFind->second);
				buffer[sizeof(buffer)-1] = '\0';

				windowTitle += Unicode::narrowToWide(buffer);
			}
		}
	}

	m_textPlanetName->SetLocalText (windowTitle);

	UIWidget * widget = dynamic_cast<UIWidget *>(m_pageMaps->GetChild (zoneName.c_str ()));
	if (!widget)
	{
		widget = dynamic_cast<UIWidget *>(m_pageMaps->GetChild ("default"));
		WARNING (true, ("could not find zone map for [%s], using default", zoneName.c_str ()));
	}
	NOT_NULL (widget);

	//----------------------------------------------------------------------

	const UIBaseObject::UIObjectList & olist = m_pageMaps->GetChildrenRef ();
	for (UIBaseObject::UIObjectList::const_iterator it = olist.begin (); it != olist.end (); ++it)
	{
		UIBaseObject * const obj = *it;
		if (obj->IsA (TUIWidget))
		{
			UIWidget * const child = safe_cast<UIWidget *>(obj);
			child->SetVisible (child == widget);
		}
	}

	//----------------------------------------------------------------------

	if(ba)
	{
		Vector2d const & zoneSize = ba->getSize(true);
		m_zoneSize.Set(zoneSize.x, zoneSize.y);

		m_zoneWorldCoordinateOffset.x = ba->getXCenterPoint(true);
		m_zoneWorldCoordinateOffset.z = ba->getZCenterPoint(true);
	}
	else
	{
		m_zoneWorldCoordinateOffset.x = m_zoneWorldCoordinateOffset.z = 0.0f;

		const TerrainObject * const terrain = TerrainObject::getInstance ();
		if (terrain)
		{
			const ProceduralTerrainAppearance * const pta = dynamic_cast<const ProceduralTerrainAppearance *>(terrain->getAppearance ());
			if (pta)
				m_zoneSize.x = m_zoneSize.y = pta->getMapWidthInMeters ();
		}
	}

	UIString fpStr;
	if (widget->GetProperty (Properties::MapWidth, fpStr))
		UIUtils::ParseFloatPoint (fpStr, m_zoneSize);
}

//----------------------------------------------------------------------

void SwgCuiPlanetMap::onAddedRemovedContainer (const ClientObject::Messages::ContainerMsg & msg)
{	
	//@todo: attempt to only update the waypoint part of the tree

	CreatureObject * const player = Game::getPlayerCreature ();
	if (!player)
		return;
	
	ClientObject * const datapad = player->getDatapadObject ();
	if (datapad == msg.first)
	{
		reset ();
	}
}


//----------------------------------------------------------------------

void SwgCuiPlanetMap::onLocationsReceived (const PlanetMapManagerClient::Messages::LocationsReceived::Payload &)
{
	if (m_entriesRequested)
	{
		m_entriesRequested = false;
		m_buttonRefresh->SetVisible(true);
		reset();
	}
}

//----------------------------------------------------------------------

void SwgCuiPlanetMap::onLocationsRequested (const bool &)
{
	if (m_entriesRequested)
		m_buttonRefresh->SetVisible(false);
}

//----------------------------------------------------------------------


void SwgCuiPlanetMap::saveSettings         () const
{
	CuiMediator::saveSettings ();

	CuiSettings::saveString  (getMediatorDebugName (), Settings::lastSelectedLocationPath, Unicode::narrowToWide (m_lastSelectedLocationPath));

	CuiSettings::saveBoolean (getMediatorDebugName (), Settings::showCities,               m_checkShowCities->IsChecked ());
	CuiSettings::saveBoolean (getMediatorDebugName (), Settings::showWaypoints,            m_checkShowWaypoints->IsChecked ());
	CuiSettings::saveBoolean (getMediatorDebugName (), Settings::showNames,                m_checkShowNames->IsChecked ());
	CuiSettings::saveBoolean (getMediatorDebugName (), Settings::showGCWRregions,          m_checkShowGCWRegions->IsChecked ());

}

//----------------------------------------------------------------------

void SwgCuiPlanetMap::loadSettings         ()
{
	CuiMediator::loadSettings ();

	Unicode::String tmpString;


	CuiSettings::loadString (getMediatorDebugName (), Settings::lastSelectedLocationPath, tmpString);
	m_lastSelectedLocationPath = Unicode::wideToNarrow (tmpString);

	bool tmpBool = false;

	if (CuiSettings::loadBoolean (getMediatorDebugName (), Settings::showCities,               tmpBool))
		m_checkShowCities->SetChecked (tmpBool, false);

	if (CuiSettings::loadBoolean (getMediatorDebugName (), Settings::showWaypoints,            tmpBool))
		m_checkShowWaypoints->SetChecked (tmpBool, false);

	if (CuiSettings::loadBoolean (getMediatorDebugName (), Settings::showNames,                tmpBool))
		m_checkShowNames->SetChecked  (tmpBool, false);

	if (CuiSettings::loadBoolean (getMediatorDebugName (), Settings::showGCWRregions,          tmpBool))
		m_checkShowGCWRegions->SetChecked  (tmpBool, false);

	reset ();
}

//----------------------------------------------------------------------

void  SwgCuiPlanetMap::onObjectNameChange        (const ClientObject & obj)
{
	if (dynamic_cast<const ClientWaypointObject *>(&obj))
		updateMarkers ();
}

//----------------------------------------------------------------------

void SwgCuiPlanetMap::onWaypointChange          (const ClientWaypointObject &)
{
	updateMarkers ();
}

//----------------------------------------------------------------------

void SwgCuiPlanetMap::onSceneChanged()
{
	m_lastZone.clear();
}

//----------------------------------------------------------------------

void SwgCuiPlanetMap::onGCWGroupValuesUpdated(const GuildObject::Messages::GCWGroupScoreUpdatedThisGalaxy::Payload & obj)
{
	if(_stricmp(obj.first.c_str(), m_lastZone.c_str()) == 0)
		updateMarkers();
}

//----------------------------------------------------------------------

void SwgCuiPlanetMap::setZoneSupermapMode(bool active)
{
	m_sliderZoom->SetUpperLimit (s_sliderRange);
	updateZoom();
	m_iconHere->SetVisible(!active);
	m_checkShowCities->SetVisible(!active);
	m_checkShowWaypoints->SetVisible(!active);
	m_checkShowNames->SetVisible(!active);
	m_buttonRefresh->SetVisible(!active);
	if(active)
	{
		clearMarkers();
		m_buttonZoom->SetText(CuiStringIdsPlanetMap::show_zone.localize ());
		m_zoomLevel = ZL_ZoneSupermap;
	}
	else
	{
		updateMarkers();
		m_buttonZoom->SetText(CuiStringIdsPlanetMap::show_planet.localize ());
		m_zoomLevel = ZL_Zone;
	}

	setupCurrentZone();
}

//======================================================================
