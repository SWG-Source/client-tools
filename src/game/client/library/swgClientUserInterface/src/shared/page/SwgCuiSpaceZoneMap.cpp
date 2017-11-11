//===================================================================
//
// SwgCuiSpaceZoneMap.cpp
// copyright 2004, sony online entertainment
// tford
//
//===================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiSpaceZoneMap.h"

#include "clientGame/ClientWaypointObject.h"
#include "clientGame/Game.h"
#include "clientGame/GuildObject.h"
#include "clientGame/ShipObject.h"
#include "clientGame/PlayerObject.h"
#include "clientGame/PlayerShipController.h"
#include "clientGraphics/RenderWorld.h"
#include "clientUserInterface/CuiInventoryManager.h"
#include "clientUserInterface/CuiIoWin.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMenuInfoTypes.h"
#include "clientUserInterface/CuiPoiManager.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiRadialMenuManager.h"
#include "clientUserInterface/CuiUtils.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "sharedGame/HyperspaceManager.h"
#include "sharedMath/Ray3d.h"
#include "sharedMessageDispatch/Message.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/PulseDynamics.h"
#include "sharedUtility/CallbackReceiver.h"
#include "sharedUtility/Callback.h"
#include "UIButton.h"
#include "UICheckbox.h"
#include "UICursor.h"
#include "UIDataSourceContainer.h"
#include "UIDataSource.h"
#include "UIManager.h"
#include "UIMessage.h"
#include "UIPalette.h"
#include "UIPopupMenu.h"
#include "UISliderbar.h"
#include "UITreeView.h"
#include "UIText.h"
#include <list>

//===================================================================

namespace SwgCuiSpaceZoneMapNamespace
{
	namespace DataProperties
	{
		UILowerString const NetworkID = UILowerString("NetworkID");
		UILowerString const SimpleIdentifierKey = UILowerString("SimpleIdentifierKey");
		UILowerString const Location = UILowerString("Location");
		UILowerString const IsHyperspacePoint = UILowerString("IsHyperspacePoint");
		UILowerString const IsSpaceStation = UILowerString("IsSpaceStation");
		UILowerString const IsActiveWaypoint = UILowerString("IsActiveWaypoint");
		UILowerString const IsInactiveWaypoint = UILowerString("IsInactiveWaypoint");
		UILowerString const IsActivePoiWaypoint = UILowerString("IsActivePoiWaypoint");
		UILowerString const IsInactivePoiWaypoint = UILowerString("IsInactivePoiWaypoint");
		UILowerString const IsMissionCritical = UILowerString("IsMissionCritical");

		// membrane shader palette property lookups
		UILowerString const ZoneDefault1("notcolor_ZoneDefault1");
		UILowerString const ZoneDefault2("notcolor_ZoneDefault2");
		UILowerString const ZoneShip1("notcolor_ZoneShip1");
		UILowerString const ZoneShip2("notcolor_ZoneShip2");
		UILowerString const ZoneSelect1("notcolor_ZoneSelect1");
		UILowerString const ZoneSelect2("notcolor_ZoneSelect2");
		UILowerString const ZoneWaypointActive1("notcolor_ZoneWaypointActive1");
		UILowerString const ZoneWaypointActive2("notcolor_ZoneWaypointActive2");
		UILowerString const ZoneWaypointInactive1("notcolor_ZoneWaypointInactive1");
		UILowerString const ZoneWaypointInactive2("notcolor_ZoneWaypointInactive2");
		UILowerString const ZoneWaypointPoiActive1("notcolor_ZoneWaypointPoiActive1");
		UILowerString const ZoneWaypointPoiActive2("notcolor_ZoneWaypointPoiActive2");
		UILowerString const ZoneWaypointPoiInactive1("notcolor_ZoneWaypointPoiInactive1");
		UILowerString const ZoneWaypointPoiInactive2("notcolor_ZoneWaypointPoiInactive2");
		UILowerString const ZoneMissionCritical1("notcolor_ZoneMissionCritical1");
		UILowerString const ZoneMissionCritical2("notcolor_ZoneMissionCritical2");
		UILowerString const ZoneSpaceStation1("notcolor_ZoneSpaceStation1");
		UILowerString const ZoneSpaceStation2("notcolor_ZoneSpaceStation2");
		UILowerString const ZoneHyperspace1("notcolor_ZoneHyperspace1");
		UILowerString const ZoneHyperspace2("notcolor_ZoneHyperspace2");
		UILowerString const ZoneAsteroid1("notcolor_ZoneAsteroid1");
		UILowerString const ZoneAsteroid2("notcolor_ZoneAsteroid2");
	}

	namespace PopupItems
	{
		std::string const target("target");
		std::string const autopilot("autopilot");
		std::string const waypoint_create("waypoint_create");
		std::string const waypoint_activate("waypoint_activate");
		std::string const waypoint_deactivate("waypoint_deactivate");
		std::string const waypoint_destroy("waypoint_destroy");
		std::string const waypoint_set_name("waypoint_set_name");
		std::string const waypoint_create_at_focus_marker("waypoint_create_at_focus_marker");
		std::string const reset_camera("reset_camera");
	}

	char const * const cs_spaceStations = "spaceStations";
	char const * const cs_hyperspacePoints = "hyperspacePoints";
	char const * const cs_activeWaypoints = "activeWaypoints";
	char const * const cs_inactiveWaypoints = "inactiveWaypoints";
	char const * const cs_activePoiWaypoints = "ActivePoiWaypoints";
	char const * const cs_inactivePoiWaypoints = "InactivePoiWaypoints";
	char const * const cs_missionCritical = "missionCritical";
	char const * const cs_dragCursorStyle = "/styles.cursors.zonemap_move";
	char const * const cs_rotateCursor = "/styles.cursors.zonemap_rotate";
	char const * const cs_selectCursor = "/styles.cursors.zonemap_select";
	char const * const cs_zoomInCursor = "/styles.cursors.zonemap_zoomin";
	char const * const cs_zoomOutCursor = "/styles.cursors.zonemap_zoomout";

	float const cs_missionCriticalPollTimeSeconds = 15.0f;
	float const cs_maximumZoom = 40.0f;
	float const cs_minimumZoom = 1.0f;
	float const cs_cameraMoveDelta = 1.0f;
	float const cs_zoomScrollDelta = 0.2f;
	float const cs_parametricZoomDefault = 0.7f;
	float const cs_resetParametricZoomDefault = 0.25f;

	NetworkId getNetworkId(UIDataSourceContainer const * item);
	int getSimpleIdentifierKey(UIDataSourceContainer const * item);
	Vector getLocation(UIDataSourceContainer const * item);
	Unicode::String getLocalizedName(UIDataSourceContainer const * item);
	bool getIsSpaceStation(UIDataSourceContainer const * item);
	bool getIsHyperspacePoint(UIDataSourceContainer const * item);
	bool getIsActiveWaypoint(UIDataSourceContainer const * item);
	bool getIsInactiveWaypoint(UIDataSourceContainer const * item);
	bool getIsActivePoiWaypoint(UIDataSourceContainer const * item);
	bool getIsInactivePoiWaypoint(UIDataSourceContainer const * item);
	bool getIsMissionCritical(UIDataSourceContainer const * item);

	void setItemData(UIDataSourceContainer * item, NetworkId const & networkId, ZoneMapObject::ObjectData const & data, UILowerString const & DataIsaCategory);

	void followShip(NetworkId const & networkId);
	void setTarget(NetworkId const & networkId);
	void autopilotToLocation(Vector const & location);

	void setZoneMapMembraneCustomizationFields();

	class PaletteResetCallback : public CallbackReceiver
	{
	public:
		PaletteResetCallback();
		virtual ~PaletteResetCallback();
		virtual void performCallback();
	};

	PaletteResetCallback * s_paletteResetCallback = 0;
	bool s_paletteChanged = false;

	//
	// *****WARNING WARNING WARNING WARNING WARNING*****
	// this list must be kept in sync in
	// PvpNamespace::loadGcwScoreCategoryTable()
	// SwgCuiSpaceZoneMap::SwgCuiSpaceZoneMap()
	// SwgCuiHyperspaceMap::SwgCuiHyperspaceMap()
	// and gcw.scriptlib (validScenes and defaultRegions)
	// *****WARNING WARNING WARNING WARNING WARNING*****
	//
	std::map<std::string, std::string> s_gcwScoreCategoryForSpaceZone;
}

//----------------------------------------------------------------------

NetworkId SwgCuiSpaceZoneMapNamespace::getNetworkId(UIDataSourceContainer const * const item)
{
	std::string stringId;
	item->GetPropertyNarrow(DataProperties::NetworkID, stringId);
	return NetworkId(stringId);
}

//----------------------------------------------------------------------

int SwgCuiSpaceZoneMapNamespace::getSimpleIdentifierKey(UIDataSourceContainer const * item)
{
	int value = 0;
	item->GetPropertyInteger(DataProperties::SimpleIdentifierKey, value);
	return value;
}

//----------------------------------------------------------------------

Vector SwgCuiSpaceZoneMapNamespace::getLocation(UIDataSourceContainer const * item)
{
	Unicode::String stringVector;
	item->GetProperty(DataProperties::Location, stringVector);
	Vector location;
	CuiUtils::ParseVector(stringVector, location);
	return location;
}

//----------------------------------------------------------------------

Unicode::String SwgCuiSpaceZoneMapNamespace::getLocalizedName(UIDataSourceContainer const * const item)
{
	Unicode::String localizedName;
	item->GetProperty(UITreeView::DataProperties::LocalText, localizedName);
	return localizedName;
}

//----------------------------------------------------------------------

bool SwgCuiSpaceZoneMapNamespace::getIsSpaceStation(UIDataSourceContainer const * item)
{
	bool value = false;
	return ((item->GetPropertyBoolean(DataProperties::IsSpaceStation, value)) && (value));
}

//----------------------------------------------------------------------

bool SwgCuiSpaceZoneMapNamespace::getIsHyperspacePoint(UIDataSourceContainer const * item)
{
	bool value = false;
	return ((item->GetPropertyBoolean(DataProperties::IsHyperspacePoint, value)) && (value));
}

//----------------------------------------------------------------------

bool SwgCuiSpaceZoneMapNamespace::getIsActiveWaypoint(UIDataSourceContainer const * item)
{
	bool value = false;
	return ((item->GetPropertyBoolean(DataProperties::IsActiveWaypoint, value)) && (value));
}

//----------------------------------------------------------------------

bool SwgCuiSpaceZoneMapNamespace::getIsInactiveWaypoint(UIDataSourceContainer const * item)
{
	bool value = false;
	return ((item->GetPropertyBoolean(DataProperties::IsInactiveWaypoint, value)) && (value));
}

//----------------------------------------------------------------------

bool SwgCuiSpaceZoneMapNamespace::getIsActivePoiWaypoint(UIDataSourceContainer const * item)
{
	bool value = false;
	return ((item->GetPropertyBoolean(DataProperties::IsActivePoiWaypoint, value)) && (value));
}

//----------------------------------------------------------------------

bool SwgCuiSpaceZoneMapNamespace::getIsInactivePoiWaypoint(UIDataSourceContainer const * item)
{
	bool value = false;
	return ((item->GetPropertyBoolean(DataProperties::IsInactivePoiWaypoint, value)) && (value));
}

//----------------------------------------------------------------------

bool SwgCuiSpaceZoneMapNamespace::getIsMissionCritical(UIDataSourceContainer const * item)
{
	bool value = false;
	return ((item->GetPropertyBoolean(DataProperties::IsMissionCritical, value)) && (value));
}

//----------------------------------------------------------------------

void SwgCuiSpaceZoneMapNamespace::setItemData(UIDataSourceContainer * const item, NetworkId const & networkId, ZoneMapObject::ObjectData const & data, UILowerString const & DataIsaCategory)
{
	Unicode::String stringVector;
	Vector const location(data.getTransform_w().getPosition_p());
	CuiUtils::FormatVector(stringVector, location);

	item->SetName(Unicode::wideToNarrow(data.getLocalizedName()));
	item->SetProperty(UITreeView::DataProperties::LocalText, data.getLocalizedName());
	item->SetPropertyNarrow(DataProperties::NetworkID, networkId.getValueString());
	item->SetPropertyBoolean(DataIsaCategory, true);
	item->SetPropertyInteger(DataProperties::SimpleIdentifierKey, data.getSimpleIdentifierKey());
	item->SetProperty(DataProperties::Location, stringVector);
}

//----------------------------------------------------------------------

void SwgCuiSpaceZoneMapNamespace::followShip(NetworkId const & networkId)
{
	// follow ship
	ShipObject * const shipObject = Game::getPlayerPilotedShip();
	if (shipObject != 0)
	{
		PlayerShipController * const playerShipController = dynamic_cast<PlayerShipController *>(shipObject->getController());
		if (playerShipController != 0)
		{
			Object const * const targetObject = NetworkIdManager::getObjectById(networkId);
			ClientObject const * const targetClientObject = (targetObject != 0) ? targetObject->asClientObject() : 0;
			ShipObject const * const targetShipObject = (targetClientObject != 0) ? targetClientObject->asShipObject() : 0;

			if (targetShipObject != 0)
			{
				playerShipController->engageAutopilotFollow(*targetShipObject);
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiSpaceZoneMapNamespace::autopilotToLocation(Vector const & location)
{
	// follow ship
	ShipObject * const shipObject = Game::getPlayerPilotedShip();
	if (shipObject != 0)
	{
		PlayerShipController * const playerShipController = dynamic_cast<PlayerShipController *>(shipObject->getController());
		if (playerShipController != 0)
		{
			playerShipController->engageAutopilotToLocation(location);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiSpaceZoneMapNamespace::setTarget(NetworkId const & networkId)
{
	// set target
	Object * const playerObject = Game::getPlayer();
	ClientObject * const playerClientObject = (playerObject != 0) ? playerObject->asClientObject() : 0;
	CreatureObject * const playerCreatureObject = (playerClientObject != 0) ? playerClientObject->asCreatureObject() : 0;

	if (playerCreatureObject != 0)
	{
		playerCreatureObject->setLookAtTarget(networkId);
	}
}

//----------------------------------------------------------------------

void SwgCuiSpaceZoneMapNamespace::setZoneMapMembraneCustomizationFields()
{
	UIPalette const * const palette = UIPalette::GetInstance();
	if (palette != 0)
	{
		int first = 0;
		int second = 0;

		palette->GetPropertyInteger(DataProperties::ZoneDefault1, first);
		palette->GetPropertyInteger(DataProperties::ZoneDefault2, second);
		ZoneMapObject::setDefaultMembraneCustomizationFields(first, second);

		palette->GetPropertyInteger(DataProperties::ZoneShip1, first);
		palette->GetPropertyInteger(DataProperties::ZoneShip2, second);
		ZoneMapObject::setShipMembraneCustomizationFields(first, second);

		palette->GetPropertyInteger(DataProperties::ZoneSelect1, first);
		palette->GetPropertyInteger(DataProperties::ZoneSelect2, second);
		ZoneMapObject::setSelectMembraneCustomizationFields(first, second);

		palette->GetPropertyInteger(DataProperties::ZoneWaypointActive1, first);
		palette->GetPropertyInteger(DataProperties::ZoneWaypointActive2, second);
		ZoneMapObject::setActiveWaypointMembraneCustomizationFields(first, second);

		palette->GetPropertyInteger(DataProperties::ZoneWaypointInactive1, first);
		palette->GetPropertyInteger(DataProperties::ZoneWaypointInactive2, second);
		ZoneMapObject::setInactiveWaypointMembraneCustomizationFields(first, second);

		palette->GetPropertyInteger(DataProperties::ZoneWaypointPoiActive1, first);
		palette->GetPropertyInteger(DataProperties::ZoneWaypointPoiActive2, second);
		ZoneMapObject::setActivePoiWaypointMembraneCustomizationFields(first, second);

		palette->GetPropertyInteger(DataProperties::ZoneWaypointPoiInactive1, first);
		palette->GetPropertyInteger(DataProperties::ZoneWaypointPoiInactive2, second);
		ZoneMapObject::setInactivePoiWaypointMembraneCustomizationFields(first, second);

		palette->GetPropertyInteger(DataProperties::ZoneMissionCritical1, first);
		palette->GetPropertyInteger(DataProperties::ZoneMissionCritical2, second);
		ZoneMapObject::setMissionCriticalMembraneCustomizationFields(first, second);

		palette->GetPropertyInteger(DataProperties::ZoneSpaceStation1, first);
		palette->GetPropertyInteger(DataProperties::ZoneSpaceStation2, second);
		ZoneMapObject::setSpaceStationMembraneCustomizationFields(first, second);

		palette->GetPropertyInteger(DataProperties::ZoneHyperspace1, first);
		palette->GetPropertyInteger(DataProperties::ZoneHyperspace2, second);
		ZoneMapObject::setHyperspaceMembraneCustomizationFields(first, second);

		palette->GetPropertyInteger(DataProperties::ZoneAsteroid1, first);
		palette->GetPropertyInteger(DataProperties::ZoneAsteroid2, second);
		ZoneMapObject::setAsteroidMembraneCustomizationFields(first, second);
	}
}

//======================================================================

SwgCuiSpaceZoneMapNamespace::PaletteResetCallback::PaletteResetCallback()
: CallbackReceiver()
{
}

//----------------------------------------------------------------------

SwgCuiSpaceZoneMapNamespace::PaletteResetCallback::~PaletteResetCallback()
{
}

//----------------------------------------------------------------------

void SwgCuiSpaceZoneMapNamespace::PaletteResetCallback::performCallback()
{
	setZoneMapMembraneCustomizationFields();
	s_paletteChanged = true;
}

//======================================================================

using namespace SwgCuiSpaceZoneMapNamespace;

//======================================================================

SwgCuiSpaceZoneMap::SwgCuiSpaceZoneMap(UIPage& page)
: UIEventCallback()
, CuiMediator("SwgCuiSpaceZoneMap",page)
, MessageDispatch::Receiver()
, m_exit(0)
, m_showAsteroids(0)
, m_showNebulas(0)
, m_showHyperspacePoint(0)
, m_showSpaceStations(0)
, m_showWaypoints(0)
, m_showMissionCritical(0)
, m_showPlayerShip(0)
, m_showNames(0)
, m_detailTree(0)
, m_zoneName(0)
, m_zoomSlider(0)
, m_resetView(0)
, m_viewer(0)
, m_dragCursor(0)
, m_rotateCursor(0)
, m_selectCursor()
, m_zoomInCursor(0)
, m_zoomOutCursor(0)
, m_callback(new MessageDispatch::Callback)
, m_zoneMapObject(0)
, m_sceneId()
, m_refreshZoneMap(false)
, m_modifyWaypoints(false)
, m_modifyMissionCriticals(false)
, m_displayingCurrentZone(false)
, m_elapsedTime(0.0f)
, m_lookAtTarget()
, m_previousLookAtTarget()
, m_lerpPreviousLookAtTargetToLookAtTargetSeconds(0.0f)
, m_parametricZoomTime(cs_parametricZoomDefault)
, m_lastMousePoint()
, m_highlightedObjectNameAndLocation()
, m_textAndLocationsChanged(true)
, m_numberOfTextAndLocations(0)
{
	getCodeDataObject(TUIButton, m_exit, "exit");
	getCodeDataObject(TUICheckbox, m_showAsteroids, "showAsteroids");
	getCodeDataObject(TUICheckbox, m_showHyperspacePoint, "showHyperspacePoint");
	getCodeDataObject(TUICheckbox, m_showNebulas, "showNebulas");
	getCodeDataObject(TUICheckbox, m_showSpaceStations, "showSpaceStations");
	getCodeDataObject(TUICheckbox, m_showWaypoints, "showWaypoints");
	getCodeDataObject(TUICheckbox, m_showMissionCritical, "showMissionCritical");
	getCodeDataObject(TUICheckbox, m_showPlayerShip, "showPlayerShip");
	getCodeDataObject(TUICheckbox, m_showNames, "showNames");
	getCodeDataObject(TUITreeView, m_detailTree, "detailTree");
	getCodeDataObject(TUIText, m_zoneName, "zoneName");
	getCodeDataObject(TUISliderbar, m_zoomSlider, "zoomSlider");
	getCodeDataObject(TUIButton, m_resetView, "resetView");

	UIWidget * wid = 0;
	getCodeDataObject(TUIWidget, wid, "viewer");
	m_viewer = safe_cast<CuiWidget3dObjectListViewer *>(wid);
	m_viewer->setRotateNeedsControlKey(true);
	m_viewer->SetContextCapable(true, true);
	m_viewer->SetSelectable(true);
	m_viewer->setRenderObjectEffects(true);

	UIPage const * const rootPage = UIManager::gUIManager().GetRootPage();
	NOT_NULL(rootPage);

	m_dragCursor = dynamic_cast<UICursor*>(rootPage->GetObjectFromPath(cs_dragCursorStyle, TUICursor));
	m_rotateCursor = dynamic_cast<UICursor*>(rootPage->GetObjectFromPath(cs_rotateCursor, TUICursor));
	m_selectCursor = dynamic_cast<UICursor*>(rootPage->GetObjectFromPath(cs_selectCursor, TUICursor));
	m_zoomInCursor = dynamic_cast<UICursor*>(rootPage->GetObjectFromPath(cs_zoomInCursor, TUICursor));
	m_zoomOutCursor = dynamic_cast<UICursor*>(rootPage->GetObjectFromPath(cs_zoomOutCursor, TUICursor));

	registerMediatorObject(*m_exit, true);
	registerMediatorObject(*m_showAsteroids, true);
	registerMediatorObject(*m_showHyperspacePoint, true);
	registerMediatorObject(*m_showNebulas, true);
	registerMediatorObject(*m_showSpaceStations, true);
	registerMediatorObject(*m_showWaypoints, true);
	registerMediatorObject(*m_showMissionCritical, true);
	registerMediatorObject(*m_showPlayerShip, true);
	registerMediatorObject(*m_showNames, true);
	registerMediatorObject(*m_detailTree, true);
	registerMediatorObject(*m_resetView, true);

	registerMediatorObject(*m_viewer, true);

	m_detailTree->SetContextCapable(true, true);

	setState(MS_closeable);
	setState(MS_closeDeactivates);

	setZoneMapMembraneCustomizationFields();

	if (s_paletteResetCallback == 0)
	{
		s_paletteResetCallback = new PaletteResetCallback;
		CuiPreferences::getPaletteChangedCallback().attachReceiver(*s_paletteResetCallback);
	}

	// build list of gcw categories for the space zones
	//
	// *****WARNING WARNING WARNING WARNING WARNING*****
	// this list must be kept in sync in
	// PvpNamespace::loadGcwScoreCategoryTable()
	// SwgCuiSpaceZoneMap::SwgCuiSpaceZoneMap()
	// SwgCuiHyperspaceMap::SwgCuiHyperspaceMap()
	// and gcw.scriptlib (validScenes and defaultRegions)
	// *****WARNING WARNING WARNING WARNING WARNING*****
	//
	if (s_gcwScoreCategoryForSpaceZone.empty())
	{
		s_gcwScoreCategoryForSpaceZone["space_tatooine"] = "gcw_region_tatooine_12";
		s_gcwScoreCategoryForSpaceZone["space_corellia"] = "gcw_region_corellia_14";
		s_gcwScoreCategoryForSpaceZone["space_dantooine"] = "gcw_region_dantooine_17";
		s_gcwScoreCategoryForSpaceZone["space_dathomir"] = "gcw_region_dathomir_12";
		s_gcwScoreCategoryForSpaceZone["space_endor"] = "gcw_region_endor_16";
		s_gcwScoreCategoryForSpaceZone["space_lok"] = "gcw_region_lok_14";
		s_gcwScoreCategoryForSpaceZone["space_naboo"] = "gcw_region_naboo_14";
		s_gcwScoreCategoryForSpaceZone["space_yavin4"] = "gcw_region_yavin4_18";
	}
}

//----------------------------------------------------------------------

SwgCuiSpaceZoneMap::~SwgCuiSpaceZoneMap()
{
	m_exit = 0;
	m_showAsteroids = 0;
	m_showHyperspacePoint = 0;
	m_showNebulas = 0;
	m_showSpaceStations = 0;
	m_showWaypoints = 0;
	m_showMissionCritical = 0;
	m_showPlayerShip = 0;
	m_showNames = 0;
	m_detailTree = 0;
	m_zoneName = 0;
	m_zoomSlider = 0;
	m_resetView = 0;

	m_viewer = 0;
	m_dragCursor = 0;
	m_rotateCursor = 0;
	m_selectCursor = 0;
	m_zoomInCursor = 0;
	m_zoomOutCursor = 0;

	delete m_callback;
	m_callback = 0;
	delete m_zoneMapObject;
	m_zoneMapObject = 0;

	if (s_paletteResetCallback != 0)
	{
		CuiPreferences::getPaletteChangedCallback().detachReceiver(*s_paletteResetCallback);
		delete s_paletteResetCallback;
		s_paletteResetCallback = 0;
	}
}

//----------------------------------------------------------------------

void SwgCuiSpaceZoneMap::setSceneId(std::string const & sceneId)
{
	if (m_sceneId != sceneId)
	{
		m_sceneId = sceneId;
		m_refreshZoneMap = true;
	}
}

//----------------------------------------------------------------------

void SwgCuiSpaceZoneMap::performActivate()
{
	//This call causes the Poi manager to load its data if it hasn't already.  It doesn't cause a copy.
	IGNORE_RETURN(CuiPoiManager::getData());

	CuiMediator::performActivate();
	CuiManager::requestPointer(true);
	m_viewer->setPaused(false);
	setIsUpdating(true);
	m_callback->connect(*this, &SwgCuiSpaceZoneMap::onPlayerWaypointsChanged, static_cast<PlayerObject::Messages::WaypointsChanged *>(0));
	m_callback->connect(*this, &SwgCuiSpaceZoneMap::onPlayerMissionCriticalsChanged, static_cast<CreatureObject::Messages::GroupMissionCriticalObjectsChanged *>(0));
	m_zoomSlider->AddCallback(this);
	m_modifyWaypoints = true;
	m_modifyMissionCriticals = true;

	connectToMessage(CuiIoWin::Messages::CONTROL_KEY_DOWN);
	connectToMessage(CuiIoWin::Messages::CONTROL_KEY_UP);

	setIsUpdating(true);
	m_refreshZoneMap = true;
}

//-------------------------------------------------------------------

void SwgCuiSpaceZoneMap::performDeactivate()
{
	CuiMediator::performDeactivate();
	CuiManager::requestPointer(false);
	m_viewer->SetMouseCursor(0);
	m_viewer->setPaused(true);
	setIsUpdating(false);
	m_callback->disconnect(*this, &SwgCuiSpaceZoneMap::onPlayerWaypointsChanged, static_cast<PlayerObject::Messages::WaypointsChanged *>(0));
	m_callback->disconnect(*this, &SwgCuiSpaceZoneMap::onPlayerMissionCriticalsChanged, static_cast<CreatureObject::Messages::GroupMissionCriticalObjectsChanged *>(0));
	m_zoomSlider->RemoveCallback(this);

	disconnectFromMessage(CuiIoWin::Messages::CONTROL_KEY_DOWN);
	disconnectFromMessage(CuiIoWin::Messages::CONTROL_KEY_UP);

	setIsUpdating(false);

	if (m_zoneMapObject != 0)
	{
		delete m_zoneMapObject;
	}
	m_zoneMapObject = 0;
}

//-------------------------------------------------------------------

void SwgCuiSpaceZoneMap::receiveMessage(const MessageDispatch::Emitter & , const MessageDispatch::MessageBase & message)
{
	if (message.isType(CuiIoWin::Messages::CONTROL_KEY_DOWN))
	{
		if ((m_viewer->GetMouseCursor() != m_zoomInCursor)
			&& (m_viewer->GetMouseCursor() != m_zoomOutCursor))
		{
			m_viewer->SetMouseCursor(m_rotateCursor);
		}	
	}

	if (message.isType(CuiIoWin::Messages::CONTROL_KEY_UP))
	{
		if (m_viewer != 0)
		{
			ZoneMapObject::ObjectData const * picked = 0;
			Vector begin_w;
			Vector end_w;

			if ((m_zoneMapObject != 0) && (m_viewer->findWorldLocation(m_lastMousePoint, begin_w, end_w)))
			{
				Vector const begin_o(m_zoneMapObject->rotateTranslate_w2o(begin_w));
				Vector const end_o(m_zoneMapObject->rotateTranslate_w2o(end_w));

				Vector direction_o(end_o - begin_o);

				if (direction_o.normalize())
				{
					Ray3d const ray_o(begin_o, direction_o);
					picked = m_zoneMapObject->closestIntersectedObject(ray_o);
				}
			}
			m_viewer->SetMouseCursor((picked != 0) ? m_selectCursor : m_dragCursor);
		}
	}
}

//-------------------------------------------------------------------

void SwgCuiSpaceZoneMap::update(float deltaTimeSecs)
{
	if ((m_refreshZoneMap) || (s_paletteChanged))
	{
		populateZoneMap(!s_paletteChanged);
		populateSpaceStationTree();
		populateHyperspacePointTree();
		populateActiveWaypointTree();
		populateInactiveWaypointTree();
		populateActivePoiWaypointTree();
		populateInactivePoiWaypointTree();
		populateMissionCriticalTree();
		m_refreshZoneMap = false;
		s_paletteChanged = false;
	}
	
	if (m_zoneMapObject != 0)
	{
		m_lerpPreviousLookAtTargetToLookAtTargetSeconds += deltaTimeSecs;

		if (m_modifyWaypoints)
		{
			m_modifyWaypoints = false;
			ZoneMapObject::modifyWaypoints(m_zoneMapObject);
			populateActiveWaypointTree();
			populateInactiveWaypointTree();
			populateActivePoiWaypointTree();
			populateInactivePoiWaypointTree();
		}

		if (m_displayingCurrentZone)
		{
			if (m_modifyMissionCriticals)
			{
				m_modifyMissionCriticals = false;
				ZoneMapObject::modifyMissionCriticals(m_zoneMapObject);
				populateMissionCriticalTree();
			}

			m_elapsedTime += deltaTimeSecs;
			if (m_elapsedTime > cs_missionCriticalPollTimeSeconds)
			{
				ZoneMapObject::updateMissionCriticalLocations(m_zoneMapObject);
				m_elapsedTime = 0.0f;
			}
		}

		populateNamesInViewer();

		if (m_zoomSlider != 0)
		{
			float const f = getParametricZoomTime() * 100.0f;
			long sliderValue = static_cast<long>(f + 0.5f);
			if (sliderValue != m_zoomSlider->GetValue())
			{
				m_zoomSlider->SetValue(sliderValue, false);
			}
		}

		if (m_viewer != 0)
		{
			Vector const lookAtTarget(calculateLookAtTarget());

			m_zoneMapObject->setLookAtTargetLocationInZoneSpace(lookAtTarget);

			m_viewer->setCameraLookAt(lookAtTarget, false);
			m_viewer->setFitDistanceFactor(calculateZoom());
			m_viewer->setRotateSpeed(0.0f);
		}
	}
}

//-------------------------------------------------------------------

void SwgCuiSpaceZoneMap::OnButtonPressed(UIWidget * const context)
{
	if (context == m_exit)
	{
		closeThroughWorkspace();
	}
	else if (context == m_resetView)
	{
		resetView();
	}
}

//-------------------------------------------------------------------

void SwgCuiSpaceZoneMap::OnCheckboxSet(UIWidget * context)
{
	if ((context == m_showAsteroids) && (m_zoneMapObject != 0))
	{
		m_zoneMapObject->showAsteroid(true);
	}
	else if ((context == m_showNebulas) && (m_zoneMapObject != 0))
	{
		m_zoneMapObject->showNebula(true);
	}
	else if ((context == m_showHyperspacePoint) && (m_zoneMapObject != 0))
	{
		m_zoneMapObject->showHyperspacePoint(true);
	}
	else if ((context == m_showSpaceStations) && (m_zoneMapObject != 0))
	{
		m_zoneMapObject->showSpaceStations(true);
	}
	else if ((context == m_showWaypoints) && (m_zoneMapObject != 0))
	{
		m_zoneMapObject->showWaypoint(true);
	}
	else if ((context == m_showMissionCritical) && (m_zoneMapObject != 0))
	{
		m_zoneMapObject->showMissionCritical(true);
	}
	else if ((context == m_showPlayerShip) && (m_zoneMapObject != 0))
	{
		m_zoneMapObject->showPlayerShip(true);
	}
	else if (context == m_showNames)
	{
		m_textAndLocationsChanged = true;
	}
}

//-------------------------------------------------------------------

void SwgCuiSpaceZoneMap::OnCheckboxUnset(UIWidget * context)
{
	if ((context == m_showAsteroids) && (m_zoneMapObject != 0))
	{
		m_zoneMapObject->showAsteroid(false);
	}
	else if ((context == m_showNebulas) && (m_zoneMapObject != 0))
	{
		m_zoneMapObject->showNebula(false);
	}
	else if ((context == m_showHyperspacePoint) && (m_zoneMapObject != 0))
	{
		m_zoneMapObject->showHyperspacePoint(false);
	}
	else if ((context == m_showSpaceStations) && (m_zoneMapObject != 0))
	{
		m_zoneMapObject->showSpaceStations(false);
	}
	else if ((context == m_showWaypoints) && (m_zoneMapObject != 0))
	{
		m_zoneMapObject->showWaypoint(false);
	}
	else if ((context == m_showMissionCritical) && (m_zoneMapObject != 0))
	{
		m_zoneMapObject->showMissionCritical(false);
	}
	else if ((context == m_showPlayerShip) && (m_zoneMapObject != 0))
	{
		m_zoneMapObject->showPlayerShip(false);
	}
	else if (context == m_showNames)
	{
		m_textAndLocationsChanged = true;
	}
}

//-------------------------------------------------------------------

void SwgCuiSpaceZoneMap::OnSliderbarChanged(UIWidget * const context)
{
	if ((context == m_zoomSlider) && (m_zoomSlider != 0))
	{
		long const value = m_zoomSlider->GetValue();
		float const percent = static_cast<float>(value) / 100.0f;
		setParametricZoomTime(percent);
	}
}

//-------------------------------------------------------------------

bool SwgCuiSpaceZoneMap::OnMessage(UIWidget * const context, UIMessage const & msg)
{
	if (m_zoneMapObject == 0)
	{
		return true;
	}

	if ((context == m_viewer) && (m_viewer != 0))
	{
		bool highlightTheTarget = false;
		bool focusCameraOnTheTarget = false;
		bool createContext = false;

		ZoneMapObject::ObjectData const * picked = 0;

		{
			Vector begin_w;
			Vector end_w;

			if (m_viewer->findWorldLocation(msg.MouseCoords, begin_w, end_w))
			{
				Vector const begin_o(m_zoneMapObject->rotateTranslate_w2o(begin_w));
				Vector const end_o(m_zoneMapObject->rotateTranslate_w2o(end_w));

				Vector direction_o(end_o - begin_o);

				if (direction_o.normalize())
				{
					Ray3d const ray_o(begin_o, direction_o);
					picked = m_zoneMapObject->closestIntersectedObject(ray_o);
				}
			}
		}

		if (msg.Type == UIMessage::MouseExit)
		{
			m_viewer->SetMouseCursor(0);
		}

		if (msg.Type == UIMessage::MouseMove)
		{
			if (!msg.Modifiers.isControlDown())
			{
				m_viewer->SetMouseCursor((picked != 0) ? m_selectCursor : m_dragCursor);

				if (msg.Modifiers.LeftMouseDown)
				{
					UIPoint delta(m_lastMousePoint - msg.MouseCoords);
					Vector const frame_k(m_viewer->getCameraFrame_k());

					Vector const right(frame_k.cross(Vector::unitY));
					Vector upOffset(frame_k.cross(right) * static_cast<float>(delta.y));
					Vector rightOffset(right * static_cast<float>(-delta.x));

					Vector offset(upOffset + rightOffset);
					if (offset.normalize())
					{
						moveCamera(offset);
					}
				}
			}
			else
			{
				m_viewer->SetMouseCursor((picked != 0) ? m_selectCursor : m_rotateCursor);
			}
		}
		else if ((msg.Type == UIMessage::MouseWheel) && (msg.Modifiers.isControlDown()))
		{
			m_viewer->SetMouseCursor((msg.Data > 0.0f) ? m_zoomInCursor : m_zoomOutCursor);
			setParametricZoomTime(getParametricZoomTime() + (msg.Data * -cs_zoomScrollDelta));
		}
		else if (msg.Type == UIMessage::LeftMouseDown)
		{
			highlightTheTarget = true;
		}
		else if (msg.Type == UIMessage::LeftMouseDoubleClick)
		{
			highlightTheTarget = true;
			focusCameraOnTheTarget = true;
		}
		else if (msg.Type == UIMessage::ContextRequest)
		{
			highlightTheTarget = true;
			createContext = true;
		}

		UIDataSourceContainer * item = 0;

		if (picked != 0)
		{
			if (highlightTheTarget)
			{
				pickTargetToLookAt(picked->getObject(), focusCameraOnTheTarget);
				synchronizeSelection(picked);

				if (createContext)
				{
					item = findItemFromZoneObjectData(picked);
				}
			}
		}

		if (createContext)
		{
			createContextMenu(picked, item);
		}
	}

	if ((context == m_detailTree) && (m_detailTree != 0))
	{
		int const selectedTreeRow = m_detailTree->GetLastSelectedRow ();
		UIDataSourceContainer const * const item = m_detailTree->GetDataSourceContainerAtRow(selectedTreeRow);

		if (item != 0)
		{
			bool highLight = false;
			bool setFocusTo = false;
			bool createContext = false;

			if (msg.Type == UIMessage::LeftMouseDown)
			{
				highLight = true;
			}
			else if (msg.Type == UIMessage::LeftMouseDoubleClick)
			{
				highLight = true;
				setFocusTo = true;
			}
			else if (msg.Type == UIMessage::ContextRequest)
			{
				highLight = true;
				createContext = true;
			}
			
			if (highLight)
			{
				ZoneMapObject::ObjectData const * const picked = findZoneObjectDataFromItem(item);
				if (picked != 0)
				{
					pickTargetToLookAt(picked->getObject(), setFocusTo);
					m_highlightedObjectNameAndLocation = ZoneMapObject::makeTextAndLocation(*picked);
					m_textAndLocationsChanged = true;

					if (createContext)
					{
						createContextMenu(picked, item);
					}
				}
			}
		}
	}

	m_lastMousePoint = msg.MouseCoords;

	return true;
}

//-------------------------------------------------------------------

UIDataSourceContainer * SwgCuiSpaceZoneMap::findItemFromZoneObjectData(ZoneMapObject::ObjectData const * const objectData) const
{
	NOT_NULL(m_zoneMapObject);
	NOT_NULL(objectData);

	UIDataSourceContainer * const rootSourceContainer = (m_detailTree != 0) ? m_detailTree->GetDataSourceContainer() : 0;
	if (rootSourceContainer != 0)
	{
		UIBaseObject::UIObjectList itemsToTest;

		UIDataSourceContainer * category = 0;

		if (objectData->getDataType() == ZoneMapObject::ObjectData::DT_spaceStation)
		{
			category = safe_cast<UIDataSourceContainer *>(rootSourceContainer->GetChild(cs_spaceStations));
			if (category != 0)
			{
				category->GetChildren(itemsToTest);
			}
		}

		else if (objectData->getDataType() == ZoneMapObject::ObjectData::DT_hyperspacePoint)
		{
			category = safe_cast<UIDataSourceContainer *>(rootSourceContainer->GetChild(cs_hyperspacePoints));
			if (category != 0)
			{
				category->GetChildren(itemsToTest);
			}
		}

		else if (objectData->getDataType() == ZoneMapObject::ObjectData::DT_wayPoint)
		{
			category = safe_cast<UIDataSourceContainer *>(rootSourceContainer->GetChild(cs_activeWaypoints));
			if (category != 0)
			{
				category->GetChildren(itemsToTest);
			}

			category = safe_cast<UIDataSourceContainer *>(rootSourceContainer->GetChild(cs_inactiveWaypoints));
			if (category != 0)
			{
				category->GetChildren(itemsToTest);
			}

			category = safe_cast<UIDataSourceContainer *>(rootSourceContainer->GetChild(cs_activePoiWaypoints));
			if (category != 0)
			{
				category->GetChildren(itemsToTest);
			}

			category = safe_cast<UIDataSourceContainer *>(rootSourceContainer->GetChild(cs_inactivePoiWaypoints));
			if (category != 0)
			{
				category->GetChildren(itemsToTest);
			}
		}

		else if (objectData->getDataType() == ZoneMapObject::ObjectData::DT_missionCritical)
		{
			category = safe_cast<UIDataSourceContainer *>(rootSourceContainer->GetChild(cs_missionCritical));
			if (category != 0)
			{
				category->GetChildren(itemsToTest);
			}
		}

		UIBaseObject::UIObjectList::const_iterator ii = itemsToTest.begin();
		UIBaseObject::UIObjectList::const_iterator iiEnd = itemsToTest.end();

		for (; ii != iiEnd; ++ii)
		{
			UIDataSourceContainer * const item = safe_cast<UIDataSourceContainer *>(*ii);
			if (item != 0)
			{
				int const simpleIdentifierKey = getSimpleIdentifierKey(item);
				if (simpleIdentifierKey == objectData->getSimpleIdentifierKey())
				{
					return item;
				}
			}
		}
	}
	return 0;
}

//-------------------------------------------------------------------

ZoneMapObject::ObjectData const * SwgCuiSpaceZoneMap::findZoneObjectDataFromItem(UIDataSourceContainer const * const item) const
{
	if (m_zoneMapObject == 0)
	{
		return 0;
	}

	NetworkId const networkId(getNetworkId(item));
	int const simpleIdentifierKey = getSimpleIdentifierKey(item);

	if (getIsSpaceStation(item) && m_showSpaceStations != 0)
	{
		// currently no NetworkId to key off of so key off of the simple key
		ZoneMapObject::StaticZoneObjects const & statics = m_zoneMapObject->getSpaceStations();
		ZoneMapObject::StaticZoneObjects::const_iterator ii = statics.begin();
		ZoneMapObject::StaticZoneObjects::const_iterator iiEnd = statics.end();

		for (; ii != iiEnd; ++ii)
		{
			ZoneMapObject::ObjectData const & data = *ii;

			if (data.getSimpleIdentifierKey() == simpleIdentifierKey)
			{
				if (!m_showSpaceStations->IsChecked())
				{
					m_showSpaceStations->SetChecked(true, false);
					m_zoneMapObject->showSpaceStations(true);
				}
				return &data;
			}
		}
	}
	else if (getIsHyperspacePoint(item) && m_showHyperspacePoint != 0)
	{
		// no NetworkId to key off of so key off of the simple key
		ZoneMapObject::StaticZoneObjects const & statics = m_zoneMapObject->getHyperspacePoints();
		ZoneMapObject::StaticZoneObjects::const_iterator ii = statics.begin();
		ZoneMapObject::StaticZoneObjects::const_iterator iiEnd = statics.end();

		for (; ii != iiEnd; ++ii)
		{
			ZoneMapObject::ObjectData const & data = *ii;

			if (data.getSimpleIdentifierKey() == simpleIdentifierKey)
			{
				if (!m_showHyperspacePoint->IsChecked())
				{
					m_showHyperspacePoint->SetChecked(true, false);
					m_zoneMapObject->showHyperspacePoint(true);
				}
				return &data;
			}
		}
	}
	else if (getIsActiveWaypoint(item) && m_showWaypoints != 0)
	{
		ZoneMapObject::DynamicZoneObjects const & dynamics = m_zoneMapObject->getActiveWaypoints();
		ZoneMapObject::DynamicZoneObjects::const_iterator ii = dynamics.find(networkId);
		if (ii != dynamics.end())
		{
			if (!m_showWaypoints->IsChecked())
			{
				m_showWaypoints->SetChecked(true, false);
				m_zoneMapObject->showWaypoint(true);
			}
			return &ii->second;
		}
	}
	else if (getIsInactiveWaypoint(item) && m_showWaypoints != 0)
	{
		ZoneMapObject::DynamicZoneObjects const & dynamics = m_zoneMapObject->getInactiveWaypoints();
		ZoneMapObject::DynamicZoneObjects::const_iterator ii = dynamics.find(networkId);
		if (ii != dynamics.end())
		{
			if (!m_showWaypoints->IsChecked())
			{
				m_showWaypoints->SetChecked(true, false);
				m_zoneMapObject->showWaypoint(true);
			}
			return &ii->second;
		}
	}
	else if (getIsActivePoiWaypoint(item) && m_showWaypoints != 0)
	{
		ZoneMapObject::DynamicZoneObjects const & dynamics = m_zoneMapObject->getActivePoiWaypoints();
		ZoneMapObject::DynamicZoneObjects::const_iterator ii = dynamics.find(networkId);
		if (ii != dynamics.end())
		{
			if (!m_showWaypoints->IsChecked())
			{
				m_showWaypoints->SetChecked(true, false);
				m_zoneMapObject->showWaypoint(true);
			}
			return &ii->second;
		}
	}
	else if (getIsInactivePoiWaypoint(item) && m_showWaypoints != 0)
	{
		ZoneMapObject::DynamicZoneObjects const & dynamics = m_zoneMapObject->getInactivePoiWaypoints();
		ZoneMapObject::DynamicZoneObjects::const_iterator ii = dynamics.find(networkId);
		if (ii != dynamics.end())
		{
			if (!m_showWaypoints->IsChecked())
			{
				m_showWaypoints->SetChecked(true, false);
				m_zoneMapObject->showWaypoint(true);
			}
			return &ii->second;
		}
	}
	else if (getIsMissionCritical(item) && m_showMissionCritical != 0)
	{
		ZoneMapObject::DynamicZoneObjects const & dynamics = m_zoneMapObject->getMissionCriticals();
		ZoneMapObject::DynamicZoneObjects::const_iterator ii = dynamics.find(networkId);
		if (ii != dynamics.end())
		{
			if (!m_showMissionCritical->IsChecked())
			{
				m_showMissionCritical->SetChecked(true, false);
				m_zoneMapObject->showMissionCritical(true);
			}
			return &ii->second;
		}
	}
	return 0;
}

//-------------------------------------------------------------------

void SwgCuiSpaceZoneMap::synchronizeSelection(ZoneMapObject::ObjectData const * const objectData)
{
	ZoneMapObject::TextAndLocation original(m_highlightedObjectNameAndLocation);

	if (objectData != 0)
	{
		UIDataSourceContainer * const item = findItemFromZoneObjectData(objectData);
		if ((item != 0) && (m_detailTree != 0))
		{
			int row = 0;
			UITreeView::DataNode * const dataNode = m_detailTree->FindDataNodeByDataSource(*item, row);
			if (dataNode != 0)
			{
				m_detailTree->SelectRow(row);
				m_detailTree->ExpandParentNodes(*dataNode);
				m_detailTree->ScrollToRow(row);
			}
		}
		m_highlightedObjectNameAndLocation = ZoneMapObject::makeTextAndLocation(*objectData);
	}
	else
	{
		m_highlightedObjectNameAndLocation = ZoneMapObject::TextAndLocation();
	}

	m_textAndLocationsChanged = original != m_highlightedObjectNameAndLocation;
}

//-------------------------------------------------------------------

void SwgCuiSpaceZoneMap::targetItem(UIDataSourceContainer const * const item) const
{
	NetworkId const networkId(getNetworkId(item));

	// set the target
	if (networkId != NetworkId::cms_invalid)
	{
		setTarget(networkId);
	}
}

//-------------------------------------------------------------------

void SwgCuiSpaceZoneMap::autopilotToItem(UIDataSourceContainer const * const item) const
{
	NetworkId const id(getNetworkId(item));
	Object const * const object = NetworkIdManager::getObjectById(id);
	Vector target((object != 0) ? object->getPosition_w() : getLocation(item));

	ClientWaypointObject const * const waypoint = dynamic_cast<ClientWaypointObject const *>(object);
	if (waypoint != 0)
	{
		target = waypoint->getLocation();
	}

	autopilotToLocation(target);
}

//-------------------------------------------------------------------

void SwgCuiSpaceZoneMap::onPlayerWaypointsChanged(PlayerObject::Messages::WaypointsChanged::Payload const &)
{
	m_modifyWaypoints = true;
}

//-------------------------------------------------------------------

void SwgCuiSpaceZoneMap::onPlayerMissionCriticalsChanged(CreatureObject::Messages::GroupMissionCriticalObjectsChanged::Payload const &)
{
	m_modifyMissionCriticals = true;
}

//-------------------------------------------------------------------

Vector const & SwgCuiSpaceZoneMap::calculateLookAtTarget()
{
	if ((m_zoneMapObject != 0) && (m_zoneMapObject->isFocusedOnHighLightedTarget()))
	{
		float time = clamp(0.0f, m_lerpPreviousLookAtTargetToLookAtTargetSeconds, 1.0f);
		Vector const target(m_zoneMapObject->getHighLightedTargetLocationInZoneSpace());
		m_lookAtTarget = Vector::linearInterpolate(m_previousLookAtTarget, target, time);
	}
	return m_lookAtTarget;
}

//----------------------------------------------------------------------

float SwgCuiSpaceZoneMap::getParametricZoomTime() const
{
	return m_parametricZoomTime;
}

//----------------------------------------------------------------------

void SwgCuiSpaceZoneMap::setParametricZoomTime(float const time)
{
	m_parametricZoomTime = clamp(0.0f, time, 1.0f);
}

//----------------------------------------------------------------------

float SwgCuiSpaceZoneMap::calculateZoom() const
{
	return linearInterpolate(cs_minimumZoom, cs_maximumZoom, m_parametricZoomTime);
}

//----------------------------------------------------------------------

void SwgCuiSpaceZoneMap::moveCamera(Vector const & direction)
{
	if (m_zoneMapObject != 0)
	{
		m_zoneMapObject->showLookAtFocus(true);
		Vector const potential(m_lookAtTarget + (direction * cs_cameraMoveDelta));
		if (m_zoneMapObject->isPointInZone(potential))
		{
			m_lookAtTarget = potential;
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiSpaceZoneMap::OnPopupMenuSelection(UIWidget * context)
{
	UIPopupMenu * const popupMenu = dynamic_cast<UIPopupMenu *>(context);
	if (popupMenu != 0)
	{
		std::string const selection(popupMenu->GetSelectedName());
		if ((!selection.empty()) && (m_zoneMapObject != 0) && (m_detailTree != 0))
		{
			int const selectedTreeRow = m_detailTree->GetLastSelectedRow ();
			UIDataSourceContainer const * const item = m_detailTree->GetDataSourceContainerAtRow(selectedTreeRow);

			if (item != 0)
			{
				if (selection == PopupItems::target)
				{
					targetItem(item);
				}
				else if (selection == PopupItems::autopilot)
				{
					autopilotToItem(item);
				}
				else if (selection == PopupItems::waypoint_create)
				{
					Vector const targetInZoneSpace(m_zoneMapObject->getHighLightedTargetLocationInZoneSpace());
					Vector const targetInWorldSpace(m_zoneMapObject->zoneLocationToWorldLocation(targetInZoneSpace));

					StringId const sid("planet_n", Game::calculateNonInstanceSceneId(m_zoneMapObject->getSceneId()));
					ClientWaypointObject::requestWaypoint(sid.localize(), targetInWorldSpace);
				}
				else if (selection == PopupItems::waypoint_create_at_focus_marker)
				{
					Vector const targetInZoneSpace(m_zoneMapObject->getLookAtTargetLocationInZoneSpace());
					Vector const targetInWorldSpace(m_zoneMapObject->zoneLocationToWorldLocation(targetInZoneSpace));

					StringId const sid("planet_n", Game::calculateNonInstanceSceneId(m_zoneMapObject->getSceneId()));
					ClientWaypointObject::requestWaypoint(sid.localize(), targetInWorldSpace);
				}
				else if (selection == PopupItems::reset_camera)
				{
					resetView();
				}
				else
				{
					NetworkId const networkId(getNetworkId(item));
					ClientWaypointObject * const waypoint = dynamic_cast<ClientWaypointObject *>(NetworkIdManager::getObjectById(networkId));

					if (waypoint != 0)
					{
						if (selection == PopupItems::waypoint_activate)
						{
							IGNORE_RETURN(waypoint->setWaypointActive(true));
							if (waypoint->isPoi())
							{
								m_modifyWaypoints = true;
							}
						}
						else if (selection == PopupItems::waypoint_deactivate)
						{
							IGNORE_RETURN(waypoint->setWaypointActive(false));
							if (waypoint->isPoi())
							{
								m_modifyWaypoints = true;
							}
						}
						else if (selection == PopupItems::waypoint_destroy)
						{
							CuiInventoryManager::destroyObject(waypoint->getNetworkId (), false);
						}
						else if (selection == PopupItems::waypoint_set_name)
						{
							CuiRadialMenuManager::performDefaultAction(*waypoint, false, static_cast<int>(Cui::MenuInfoTypes::SET_NAME));
						}
					}
				}
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiSpaceZoneMap::createContextMenu(ZoneMapObject::ObjectData const * const pickedObject, UIDataSourceContainer const * const pickedItem)
{
	typedef std::vector<std::string> Options;
	Options options;

	// build the string representations for the context menu
	if ((pickedObject != 0) && (pickedItem != 0))
	{
		WARNING_DEBUG_FATAL(getSimpleIdentifierKey(pickedItem) != pickedObject->getSimpleIdentifierKey(), ("SwgCuiSpaceZoneMap::createContextMenu: pickedObject and pickedItem should represent the same object."));

		if (pickedObject->getDataType() == ZoneMapObject::ObjectData::DT_missionCritical)
		{
			options.push_back(PopupItems::target);
		}

		if ((pickedObject->getDataType() == ZoneMapObject::ObjectData::DT_wayPoint)
			|| (pickedObject->getDataType() == ZoneMapObject::ObjectData::DT_spaceStation)
			|| (pickedObject->getDataType() == ZoneMapObject::ObjectData::DT_missionCritical)
			|| (pickedObject->getDataType() == ZoneMapObject::ObjectData::DT_hyperspacePoint))
		{
			options.push_back(PopupItems::autopilot);
		}

		if (pickedObject->getDataType() != ZoneMapObject::ObjectData::DT_wayPoint)
		{
			options.push_back(PopupItems::waypoint_create);
		}

		if (pickedObject->getDataType() == ZoneMapObject::ObjectData::DT_wayPoint)
		{
			if (getIsInactiveWaypoint(pickedItem))
			{
				options.push_back(PopupItems::waypoint_activate);
			}

			if (getIsActiveWaypoint(pickedItem))
			{
				options.push_back(PopupItems::waypoint_deactivate);
			}

			if (getIsActivePoiWaypoint(pickedItem))
			{
				options.push_back(PopupItems::waypoint_deactivate);
			}
			else if (getIsInactivePoiWaypoint(pickedItem))
			{
				options.push_back(PopupItems::waypoint_activate);			
			}
			else
			{
				options.push_back(PopupItems::waypoint_destroy);
				options.push_back(PopupItems::waypoint_set_name);
			}
		}
	}
	else
	{
		if (pickedObject != 0)
		{
			options.push_back(PopupItems::waypoint_create);
		}
		else
		{
			options.push_back(PopupItems::waypoint_create_at_focus_marker);
		}
	
		options.push_back(PopupItems::reset_camera);
	}

	if (!options.empty())
	{
		UIPopupMenu * const popupMenu = new UIPopupMenu(&getPage());
		popupMenu->SetStyle(getPage().FindPopupStyle());

		Unicode::String title = StringId("planet_n", Game::getNonInstanceSceneId()).localize();

		if (pickedItem != 0)
		{
			pickedItem->GetProperty(UITreeView::DataProperties::LocalText, title);
		}

		UIDataSource * const titleDataSource = popupMenu->AddItem ("label", title);
		titleDataSource->SetPropertyBoolean(UIPopupMenu::DataProperties::IsLabel, false);

		Options::const_iterator ii = options.begin();
		Options::const_iterator iiEnd = options.end();

		for (; ii != iiEnd; ++ii)
		{
			popupMenu->AddItem(*ii, StringId("zone_map_context", *ii).localize());
		}

		popupMenu->SetOffsetIndex(1);
		popupMenu->SetVisible(true);
		popupMenu->AddCallback(this);
		popupMenu->SetLocation(UIManager::gUIManager().GetLastMouseCoord());
		UIManager::gUIManager().PushContextWidget(*popupMenu);
	} //lint !e429 pop not freed (UIManager gains ownership with PushContextWidget call)
}

//----------------------------------------------------------------------

void SwgCuiSpaceZoneMap::resetView()
{
	if (m_zoneMapObject != 0)
	{
		pickTargetToLookAt(0, true);
		m_zoneMapObject->showLookAtFocus(false);
		setParametricZoomTime(cs_resetParametricZoomDefault);
	}
}

//----------------------------------------------------------------------

void SwgCuiSpaceZoneMap::pickTargetToLookAt(Object * const target, bool const setFocusTo)
{
	if (m_zoneMapObject != 0)
	{
		m_zoneMapObject->highlightTarget(target, setFocusTo);
		m_zoneMapObject->showLookAtFocus(target == 0);
		m_previousLookAtTarget = m_lookAtTarget;
		if (setFocusTo)
		{
			m_lerpPreviousLookAtTargetToLookAtTargetSeconds = 0.0f;
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiSpaceZoneMap::populateZoneMap(bool const reset)
{
	if (m_zoneMapObject != 0)
	{
		delete m_zoneMapObject;
	}

	m_zoneMapObject = new ZoneMapObject(Game::calculateNonInstanceSceneId(m_sceneId), Game::getPlayerContainingShip());
	RenderWorld::addObjectNotifications(*m_zoneMapObject);

	ZoneMapObject::createAsteroids(m_zoneMapObject);
	ZoneMapObject::createNebulas(m_zoneMapObject);
	ZoneMapObject::createHyperspacePoints(m_zoneMapObject);
	ZoneMapObject::createSpaceStations(m_zoneMapObject);

	m_zoneMapObject->showAsteroid((m_showAsteroids != 0) && (m_showAsteroids->IsChecked()));
	m_zoneMapObject->showNebula((m_showNebulas != 0) && (m_showNebulas->IsChecked()));
	m_zoneMapObject->showHyperspacePoint((m_showHyperspacePoint != 0) && (m_showHyperspacePoint->IsChecked()));
	m_zoneMapObject->showSpaceStations((m_showSpaceStations != 0) && (m_showSpaceStations->IsChecked()));

	m_displayingCurrentZone = (Game::calculateNonInstanceSceneId(m_sceneId) == Game::getNonInstanceSceneId());
	// if we are displaying a zone other than the player's current zone
	// then we want to disable all of the zone specific content

	if (reset)
	{
		resetView();
	}

	if (m_displayingCurrentZone)
	{
		ZoneMapObject::modifyWaypoints(m_zoneMapObject);
		ZoneMapObject::modifyMissionCriticals(m_zoneMapObject);

		if (m_showWaypoints != 0)
		{
			m_showWaypoints->SetEnabled(true);
			m_zoneMapObject->showWaypoint(m_showWaypoints->IsChecked());
		}
		if (m_showMissionCritical != 0)
		{
			m_showMissionCritical->SetEnabled(true);
			m_zoneMapObject->showMissionCritical(m_showMissionCritical->IsChecked());
		}
		if (m_showPlayerShip != 0)
		{
			m_showPlayerShip->SetEnabled(true);
			m_zoneMapObject->showPlayerShip(m_showPlayerShip->IsChecked());
		}
	}
	else
	{
		if (m_showWaypoints != 0)
		{
			m_showWaypoints->SetChecked(false, false);
			m_showWaypoints->SetEnabled(false);
		}
		if (m_showMissionCritical != 0)
		{
			m_showMissionCritical->SetChecked(false, false);
			m_showMissionCritical->SetEnabled(false);
		}
		if (m_showPlayerShip != 0)
		{
			m_showPlayerShip->SetChecked(false, false);
			m_showPlayerShip->SetEnabled(false);
		}
	}

	{
		std::string const nonInstanceSceneId(Game::calculateNonInstanceSceneId(m_sceneId));
		Unicode::String windowTitle = StringId("planet_n", nonInstanceSceneId).localize();

		// include GCW zone score in window title
		std::map<std::string, std::string>::const_iterator const iterFindGcwScoreCategory = s_gcwScoreCategoryForSpaceZone.find(nonInstanceSceneId);
		if (iterFindGcwScoreCategory != s_gcwScoreCategoryForSpaceZone.end())
		{
			GuildObject const * const go = GuildObject::getGuildObject();
			if (go)
			{
				std::map<std::string, int> const & gcwScoreMap = go->getGcwImperialScorePercentileThisGalaxy();
				std::map<std::string, int>::const_iterator const iterFindScore = gcwScoreMap.find(iterFindGcwScoreCategory->second);
				if (iterFindScore != gcwScoreMap.end())
				{
					char buffer[64];
					snprintf(buffer, sizeof(buffer)-1, " (R:%d%% I:%d%%)", (100 - iterFindScore->second), iterFindScore->second);
					buffer[sizeof(buffer)-1] = '\0';

					windowTitle += Unicode::narrowToWide(buffer);
				}
			}
		}

		m_zoneName->SetText(windowTitle);
	}

	if (m_viewer != 0)
	{
		m_viewer->clearObjects();
		m_viewer->addObject(*m_zoneMapObject);

		if (reset)
		{
			m_viewer->setViewDirty(true);

			// set zoom to 1.0f so we can get a cool zoom out
			m_viewer->setFitDistanceFactor(1.0f);

			// center the target for now
			m_viewer->setCameraForceTarget(true);
			m_viewer->recomputeZoom();

			// unlock it from the target
			m_viewer->setCameraForceTarget(false);
			m_viewer->setRotateSpeed(0.0f);

			m_viewer->setCameraZoomInWhileTurn(false);
			m_viewer->setCameraAutoZoom(false);

			// when not set to auto zoom, the zoom will always be set
			// to the distance factor which defaults to 1.0f
			m_viewer->setFitDistanceFactor(calculateZoom());
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiSpaceZoneMap::populateSpaceStationTree()
{
	if ((m_zoneMapObject != 0) && (m_detailTree != 0))
	{
		UIDataSourceContainer * const rootSourceContainer = m_detailTree->GetDataSourceContainer ();

		if (rootSourceContainer != 0)
		{
			UIDataSourceBase * const category = dynamic_cast<UIDataSourceBase *>(rootSourceContainer->GetChild(cs_spaceStations));

			if (category != 0)
			{
				StringId const sid("ui_zone_map", "space_stations");
				IGNORE_RETURN(category->SetProperty(UITreeView::DataProperties::LocalText, sid.localize()));
				category->Clear();

				ZoneMapObject::StaticZoneObjects const & zoneObjects = m_zoneMapObject->getSpaceStations();
				ZoneMapObject::StaticZoneObjects::const_iterator ii = zoneObjects.begin();
				ZoneMapObject::StaticZoneObjects::const_iterator iiEnd = zoneObjects.end();

				for (; ii != iiEnd; ++ii)
				{
					ZoneMapObject::ObjectData const & data = *ii;

					UIDataSourceContainer * const item = new UIDataSourceContainer;				
					setItemData(item, NetworkId::cms_invalid, data, DataProperties::IsSpaceStation);
					category->AddChild (item);
				}
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiSpaceZoneMap::populateHyperspacePointTree()
{
	if ((m_zoneMapObject != 0) && (m_detailTree != 0))
	{
		UIDataSourceContainer * const rootSourceContainer = m_detailTree->GetDataSourceContainer ();

		if (rootSourceContainer != 0)
		{
			UIDataSourceBase * const category = dynamic_cast<UIDataSourceBase *>(rootSourceContainer->GetChild(cs_hyperspacePoints));

			if (category != 0)
			{
				StringId const sid("ui_zone_map", "hyperspace_points");
				IGNORE_RETURN(category->SetProperty(UITreeView::DataProperties::LocalText, sid.localize()));
				category->Clear();

				ZoneMapObject::StaticZoneObjects const & zoneObjects = m_zoneMapObject->getHyperspacePoints();
				ZoneMapObject::StaticZoneObjects::const_iterator ii = zoneObjects.begin();
				ZoneMapObject::StaticZoneObjects::const_iterator iiEnd = zoneObjects.end();

				for (; ii != iiEnd; ++ii)
				{
					ZoneMapObject::ObjectData const & data = *ii;

					UIDataSourceContainer * const item = new UIDataSourceContainer;				
					setItemData(item, NetworkId::cms_invalid, data, DataProperties::IsHyperspacePoint);
					category->AddChild (item);
				}
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiSpaceZoneMap::populateActiveWaypointTree()
{
	if ((m_zoneMapObject != 0) && (m_detailTree != 0))
	{
		UIDataSourceContainer * const rootSourceContainer = m_detailTree->GetDataSourceContainer ();

		if (rootSourceContainer != 0)
		{
			UIDataSourceBase * const category = dynamic_cast<UIDataSourceBase *>(rootSourceContainer->GetChild(cs_activeWaypoints));

			if (category != 0)
			{
				StringId const sid("ui_zone_map", "active_waypoints");
				IGNORE_RETURN(category->SetProperty(UITreeView::DataProperties::LocalText, sid.localize()));
				category->Clear();

				ZoneMapObject::DynamicZoneObjects const & zoneObjects = m_zoneMapObject->getActiveWaypoints();
				ZoneMapObject::DynamicZoneObjects::const_iterator ii = zoneObjects.begin();
				ZoneMapObject::DynamicZoneObjects::const_iterator iiEnd = zoneObjects.end();

				for (; ii != iiEnd; ++ii)
				{
					ZoneMapObject::ObjectData const & data = ii->second;

					UIDataSourceContainer * const item = new UIDataSourceContainer;
					setItemData(item, ii->first, data, DataProperties::IsActiveWaypoint);
					category->AddChild (item);
				}
			}
		}
	}
}


//----------------------------------------------------------------------

void SwgCuiSpaceZoneMap::populateInactiveWaypointTree()
{
	if ((m_zoneMapObject != 0) && (m_detailTree != 0))
	{
		UIDataSourceContainer * const rootSourceContainer = m_detailTree->GetDataSourceContainer ();

		if (rootSourceContainer != 0)
		{
			UIDataSourceBase * const category = dynamic_cast<UIDataSourceBase *>(rootSourceContainer->GetChild(cs_inactiveWaypoints));

			if (category != 0)
			{
				StringId const sid("ui_zone_map", "inactive_waypoints");
				IGNORE_RETURN(category->SetProperty(UITreeView::DataProperties::LocalText, sid.localize()));
				category->Clear();

				ZoneMapObject::DynamicZoneObjects const & zoneObjects = m_zoneMapObject->getInactiveWaypoints();
				ZoneMapObject::DynamicZoneObjects::const_iterator ii = zoneObjects.begin();
				ZoneMapObject::DynamicZoneObjects::const_iterator iiEnd = zoneObjects.end();

				for (; ii != iiEnd; ++ii)
				{
					ZoneMapObject::ObjectData const & data = ii->second;

					UIDataSourceContainer * const item = new UIDataSourceContainer;
					setItemData(item, ii->first, data, DataProperties::IsInactiveWaypoint);
					category->AddChild (item);
				}
			}
		}
	}
}


//----------------------------------------------------------------------

void SwgCuiSpaceZoneMap::populateActivePoiWaypointTree()
{
	if ((m_zoneMapObject != 0) && (m_detailTree != 0))
	{
		UIDataSourceContainer * const rootSourceContainer = m_detailTree->GetDataSourceContainer ();

		if (rootSourceContainer != 0)
		{
			UIDataSourceBase * const category = dynamic_cast<UIDataSourceBase *>(rootSourceContainer->GetChild(cs_activePoiWaypoints));

			if (category != 0)
			{
				StringId const sid("ui_zone_map", "active_poi_waypoints");
				IGNORE_RETURN(category->SetProperty(UITreeView::DataProperties::LocalText, sid.localize()));
				category->Clear();

				ZoneMapObject::DynamicZoneObjects const & zoneObjects = m_zoneMapObject->getActivePoiWaypoints();
				ZoneMapObject::DynamicZoneObjects::const_iterator ii = zoneObjects.begin();
				ZoneMapObject::DynamicZoneObjects::const_iterator iiEnd = zoneObjects.end();

				for (; ii != iiEnd; ++ii)
				{
					ZoneMapObject::ObjectData const & data = ii->second;

					UIDataSourceContainer * const item = new UIDataSourceContainer;
					setItemData(item, ii->first, data, DataProperties::IsActivePoiWaypoint);
					category->AddChild (item);
				}
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiSpaceZoneMap::populateInactivePoiWaypointTree()
{
	if ((m_zoneMapObject != 0) && (m_detailTree != 0))
	{
		UIDataSourceContainer * const rootSourceContainer = m_detailTree->GetDataSourceContainer ();

		if (rootSourceContainer != 0)
		{
			UIDataSourceBase * const category = dynamic_cast<UIDataSourceBase *>(rootSourceContainer->GetChild(cs_inactivePoiWaypoints));

			if (category != 0)
			{
				StringId const sid("ui_zone_map", "inactive_poi_waypoints");
				IGNORE_RETURN(category->SetProperty(UITreeView::DataProperties::LocalText, sid.localize()));
				category->Clear();

				ZoneMapObject::DynamicZoneObjects const & zoneObjects = m_zoneMapObject->getInactivePoiWaypoints();
				ZoneMapObject::DynamicZoneObjects::const_iterator ii = zoneObjects.begin();
				ZoneMapObject::DynamicZoneObjects::const_iterator iiEnd = zoneObjects.end();

				for (; ii != iiEnd; ++ii)
				{
					ZoneMapObject::ObjectData const & data = ii->second;

					UIDataSourceContainer * const item = new UIDataSourceContainer;
					setItemData(item, ii->first, data, DataProperties::IsInactivePoiWaypoint);
					category->AddChild (item);
				}
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiSpaceZoneMap::populateMissionCriticalTree()
{
	if ((m_zoneMapObject != 0) && (m_detailTree != 0))
	{
		UIDataSourceContainer * const rootSourceContainer = m_detailTree->GetDataSourceContainer ();

		if (rootSourceContainer != 0)
		{
			UIDataSourceBase * const category = dynamic_cast<UIDataSourceBase *>(rootSourceContainer->GetChild(cs_missionCritical));

			if (category != 0)
			{
				StringId const sid("ui_zone_map", "mission_critical");
				IGNORE_RETURN(category->SetProperty(UITreeView::DataProperties::LocalText, sid.localize()));
				category->Clear();

				ZoneMapObject::DynamicZoneObjects const & zoneObjects = m_zoneMapObject->getMissionCriticals();
				ZoneMapObject::DynamicZoneObjects::const_iterator ii = zoneObjects.begin();
				ZoneMapObject::DynamicZoneObjects::const_iterator iiEnd = zoneObjects.end();

				for (; ii != iiEnd; ++ii)
				{
					ZoneMapObject::ObjectData const & data = ii->second;

					UIDataSourceContainer * const item = new UIDataSourceContainer;
					setItemData(item, ii->first, data, DataProperties::IsMissionCritical);
					category->AddChild (item);
				}
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiSpaceZoneMap::populateNamesInViewer()
{
	if (m_viewer == 0)
	{
		return;
	}

	if (m_zoneMapObject == 0)
	{
		return;
	}

	int const numberOfTextAndLocations = static_cast<int>(m_zoneMapObject->getAllTextAndLocations().size());

	if ((m_textAndLocationsChanged) || (m_numberOfTextAndLocations != numberOfTextAndLocations))
	{
		m_viewer->clearText3d();

		if (m_showNames->IsChecked())
		{
			ZoneMapObject::TextAndLocationVector::const_iterator ii = m_zoneMapObject->getAllTextAndLocations().begin();
			ZoneMapObject::TextAndLocationVector::const_iterator iiEnd = m_zoneMapObject->getAllTextAndLocations().end();

			for (; ii != iiEnd; ++ii)
			{
				UIString const & text = ii->first;
				Vector const & location = ii->second;
				UIColor const & color = UIColor::white;
				float backgroundOpacity = 0.0f;

				m_viewer->addText3d(location, text, color, backgroundOpacity);
			}
		}
		else
		{
			UIString const & text = m_highlightedObjectNameAndLocation.first;
			Vector const & location = m_highlightedObjectNameAndLocation.second;
			UIColor const & color = UIColor::white;
			float backgroundOpacity = 0.0f;
			m_viewer->addText3d(location, text, color, backgroundOpacity);
		}
		m_textAndLocationsChanged = false;
		m_numberOfTextAndLocations = numberOfTextAndLocations;
	}
}

//======================================================================

