// ClientWaypointObject.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------
// Note: The cell id in m_cell and the cell's location  contains a masked 
// value for the building crc and the cell name crc.  Please see
// src/engine/server/library/serverScript/src/shared/ScriptMethodsWaypoint.cpp
// in the function getWaypointCellIdFromTemplateNameAndCelllName for more
// information.
//-----------------------------------------------------------------------

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientWaypointObject.h"

#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/PlayerCreatureController.h"
#include "clientGraphics/RenderWorld.h"
#include "clientUserInterface/CuiPreferences.h"
#include "sharedCollision/BaseExtent.h"
#include "sharedCollision/Floor.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"

#include "sharedGame/SharedBuildoutAreaManager.h"
#include "sharedFoundation/Timer.h"
#include "sharedGame/GroundZoneManager.h"
#include "sharedGame/SharedWaypointObjectTemplate.h"
#include "sharedGame/Waypoint.h"
#include "sharedGame/WaypointData.h"
#include "sharedMath/AxialBox.h"
#include "sharedMath/Range.h"
#include "sharedMath/VectorArgb.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/MessageQueueNetworkId.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/Controller.h"
#include "sharedObject/PortalProperty.h"
#include "sharedObject/PortalPropertyTemplate.h"
#include "sharedTerrain/TerrainObject.h"
#include "sharedUtility/Callback.h"
#include "sharedUtility/CallbackReceiver.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"
#include "sharedUtility/Location.h"
#include <algorithm>

//-----------------------------------------------------------------------

namespace ClientWaypointObjectNamespace
{

	namespace Transceivers
	{
		MessageDispatch::Transceiver<const ClientWaypointObject::Messages::ActivatedChanged::Payload &,  ClientWaypointObject::Messages::ActivatedChanged>  activatedChanged;
		MessageDispatch::Transceiver<const ClientWaypointObject::Messages::LocationChanged::Payload &,   ClientWaypointObject::Messages::LocationChanged>   locationChanged;
		MessageDispatch::Transceiver<const ClientWaypointObject::Messages::ColorChanged::Payload &,      ClientWaypointObject::Messages::ColorChanged>      colorChanged;
	}

	const char * const DEFAULT_SHARED_WAYPOINT_TEMPLATE              = "object/waypoint/shared_waypoint.iff";
	static const char * const DEFAULT_SHARED_WORLD_WAYPOINT_TEMPLATE = "object/waypoint/shared_world_waypoint.iff";
	char const * const SMALL_WAYPOINT_APPEARANCE = "appearance/pt_waypoint_ai_s01_white.prt";

	ClientWaypointObject::WaypointVector s_activeWaypoints;
	ClientWaypointObject::ConstWaypointVector s_regularWaypoints;
	ClientWaypointObject::ConstWaypointVector s_poiWaypoints;

	typedef stdmap<std::string, std::string>::fwd                         StringMap;
	typedef stdmap<std::string, std::pair<std::string, VectorArgb> >::fwd  ColorMap;
	typedef std::map<uint32 /*crc*/, std::string> CrcMap;

	StringMap s_planetAppearanceMap;
	ColorMap  s_colorMapping;
	CrcMap s_buildingAppearanceMap;

	static bool s_installed = false;

	std::string s_planetDefaultAppearance = "appearance/defaultappearance.apt";
	std::string const s_waypointEntrance        = "entrance";
	std::string const s_waypointSmall           = "small";
	std::string const s_waypointInvisible       = "invisible";

	//----------------------------------------------------------------------

	class CallbackReceiverShowGroupWaypoints: public CallbackReceiver 
	{
	public:
		CallbackReceiverShowGroupWaypoints() :
			CallbackReceiver()
		{
		}

		void performCallback()
		{
			bool const active = CuiPreferences::getShowGroupWaypoints();
			for (ClientWaypointObject::ConstWaypointVector::const_iterator i = s_regularWaypoints.begin(); i != s_regularWaypoints.end(); ++i)
			{
				ClientWaypointObject const * const o = (*i).getPointer();
				if (o && o->isGroupWaypoint())
					const_cast<ClientWaypointObject *>(o)->setWaypointActive(active);
			}
		}

	};

	CallbackReceiverShowGroupWaypoints s_callbackReceiverShowGroupWaypoints;

	void remove ()
	{
		CuiPreferences::getShowGroupWaypointsCallback().detachReceiver(s_callbackReceiverShowGroupWaypoints);
	}

	void install ()
	{
		InstallTimer const installTimer("ClientWaypointObject::install");

		if (s_installed)
			return;

		s_installed = true;

		static const std::string table_name ("datatables/player/waypoint_planets.iff");
		const DataTable * const table = DataTableManager::getTable (table_name, true);
		if (table)
		{
			static const std::string planet_default ("default");
			static const std::string appname_prefix ("appearance/");

			const int numRows = table->getNumRows ();

			for (int i = 0; i < numRows; ++i)
			{
				const std::string & planet     = table->getStringValue (0, i);
				const std::string & appearance = appname_prefix + table->getStringValue (1, i);

				if (planet == planet_default)
					s_planetDefaultAppearance = appearance;
				else
					s_planetAppearanceMap.insert (std::make_pair (planet, appearance));

				uint32 crc = Crc::normalizeAndCalculate(planet.c_str());
				s_buildingAppearanceMap[ crc ] = appearance;
			}
			DataTableManager::close (table_name);
		}
		else
			WARNING (true, ("could not load data table [%s]", table_name.c_str ()));

		
		static const std::string wp_table_name ("datatables/player/waypoint_map.iff");
		DataTable const * const wp_table = DataTableManager::getTable (wp_table_name, true);
		if (wp_table)
		{
			s_colorMapping.clear();
			const int numRows = wp_table->getNumRows ();

			for (int i = 0; i < numRows; ++i)
			{
				std::string const & color = wp_table->getStringValue (0, i);
				std::string const & appearance = wp_table->getStringValue (1, i);
				float const argbcolorA  = wp_table->getFloatValue (2, i);
				float const argbcolorR  = wp_table->getFloatValue (3, i);
				float const argbcolorG  = wp_table->getFloatValue (4, i);
				float const argbcolorB  = wp_table->getFloatValue (5, i);

				s_colorMapping [color]    = std::make_pair (appearance, VectorArgb (argbcolorA, argbcolorR, argbcolorG, argbcolorB));
			}
			DataTableManager::close (wp_table_name);
		}
		else
			WARNING (true, ("could not load waypoint colormap [%s]", wp_table_name.c_str ()));

		CuiPreferences::getShowGroupWaypointsCallback().attachReceiver(s_callbackReceiverShowGroupWaypoints);

		ExitChain::add(remove, "ClientWaypointObject");
	}

	//----------------------------------------------------------------------
	
	const std::string & getAppearanceForPlanet (const std::string & planet)
	{
		if (!s_installed)
			install ();
		
		StringMap::const_iterator it = s_planetAppearanceMap.find (planet);
		if (it == s_planetAppearanceMap.end ())
		{
			return s_planetDefaultAppearance;
		}
		
		return (*it).second;
	}
	
	//----------------------------------------------------------------------
	
	char const * const getAppearanceForBuilding(uint32 buildingCrc)
	{
		if (!s_installed)
			install();
		
		char const * buildingAppearance = NULL;

		CrcMap::const_iterator const it = s_buildingAppearanceMap.find(buildingCrc);
		if (it != s_buildingAppearanceMap.end())
		{
			buildingAppearance = it->second.c_str();
		}

		return buildingAppearance;
	}
	
	//----------------------------------------------------------------------

	ClientWaypointObject * createWorldWaypointForColor (const std::string & color)
	{
		if (!s_installed)
			install ();

		ClientWaypointObject * newWp = 0;

		if (color.empty ())
		{
			newWp = safe_cast<ClientWaypointObject *>(ObjectTemplate::createObject (DEFAULT_SHARED_WORLD_WAYPOINT_TEMPLATE));
			if(newWp)
				newWp->endBaselines();
			return newWp;
		}

		ColorMap::const_iterator it = s_colorMapping.find (color);
		if (it == s_colorMapping.end ())
		{
			WARNING (true, ("Waypoint attempt to set invalid color [%s]", color.c_str ()));
			newWp = safe_cast<ClientWaypointObject *>(ObjectTemplate::createObject (DEFAULT_SHARED_WORLD_WAYPOINT_TEMPLATE));
			if(newWp)
				newWp->endBaselines();
			return newWp;
		}

		const std::string & templateName = (*it).second.first;
		newWp = safe_cast<ClientWaypointObject *>(ObjectTemplate::createObject (templateName.c_str ()));
		if(newWp)
			newWp->endBaselines();
		return newWp;
	}

	//----------------------------------------------------------------------

	const std::string & getRandomColor ()
	{
		if (!s_installed)
			install ();

		const size_t size  = s_colorMapping.size ();
		if (size > 0 && Random::random (0, size)) // sometimes choose the default
		{
			const size_t index = Random::random (0, size - 1);			
			ColorMap::const_iterator it = s_colorMapping.begin ();
			std::advance (it, index);
			return (*it).first;
		}

		static const std::string empty;
		return empty;
	}

	//----------------------------------------------------------------------

	bool s_singleWaypointMode =  false;
	ClientWaypointObject::WaypointVector s_oldActiveWaypoints;

	void updateSingleWaypoint(ClientWaypointObject * activeWaypoint)
	{
		if (s_singleWaypointMode)
		{
			if (!s_activeWaypoints.empty())
			{
				if (!activeWaypoint)
				{
					// Find first non group or poi waypoint.
					ClientWaypointObject * lastPoiWaypoint = NULL;

					for (ClientWaypointObject::WaypointVector::iterator i = s_activeWaypoints.begin(); i != s_activeWaypoints.end(); ++i)
					{
						ClientWaypointObject * const waypoint = const_cast<ClientWaypointObject *>(i->getPointer());
						if (waypoint && !waypoint->isGroupWaypoint())
						{
							if (waypoint->isPoi()) 
							{
								lastPoiWaypoint = waypoint;
							}
							else
							{
								activeWaypoint = waypoint;
								break;
							}
						}
					}

					if (!activeWaypoint) 
					{
						activeWaypoint = lastPoiWaypoint;
					}
				}

				if (activeWaypoint && activeWaypoint->isWaypointActive() && !activeWaypoint->isGroupWaypoint())
				{
					ClientWaypointObject::WaypointVector activeWaypoints = s_activeWaypoints;
					for (ClientWaypointObject::WaypointVector::iterator i = activeWaypoints.begin(); i != activeWaypoints.end(); ++i)
					{
						ClientWaypointObject * const waypoint = const_cast<ClientWaypointObject *>(i->getPointer());
						if (waypoint && waypoint != activeWaypoint && !waypoint->isGroupWaypoint())
						{
							waypoint->setWaypointActive(false);
						}
					}
				}
			}
		}
	}

	//----------------------------------------------------------------------
	
	void updateCachedActiveWaypointList(bool enteringSingleMode)
	{
		// If entering single mode, track if it was active already.
		if (enteringSingleMode)
		{
			s_oldActiveWaypoints.clear();

			for (ClientWaypointObject::WaypointVector::iterator i = s_activeWaypoints.begin(); i != s_activeWaypoints.end(); ++i)
			{
				ClientWaypointObject * const waypoint = i->getPointer();
				if (waypoint)
				{
					s_oldActiveWaypoints.push_back(Watcher<ClientWaypointObject>(waypoint));
				}
			}
		}
		else
		{
			// Now, restore them back.
			for (ClientWaypointObject::WaypointVector::iterator i = s_oldActiveWaypoints.begin(); i != s_oldActiveWaypoints.end(); ++i)
			{
				ClientWaypointObject * const waypoint = i->getPointer();
				if (waypoint && !waypoint->isWaypointActive())
				{
					waypoint->setWaypointActive(true);
				}
			}

			s_oldActiveWaypoints.clear();
		}
	}

	float const s_playerPositionTolerance = 15.0f;
}

using namespace ClientWaypointObjectNamespace;

//----------------------------------------------------------------------

ClientWaypointObject::ClientWaypointObject (const SharedWaypointObjectTemplate * t) :
IntangibleObject   (t),
m_waypointActive   (false),
m_cell             (),
m_location         (),
m_planetName       (),
m_regionName       (),
m_waypointVisible  (true),
m_waypointColor    (),
m_worldObject      (0),
m_description      (),
m_isPoi            (false),
m_isGroupWaypoint  (false),
m_isBuildoutWaypoint(false)
{
	addSharedVariable    (m_location);
	addSharedVariable    (m_waypointActive);
	addSharedVariable    (m_cell);
	addSharedVariable    (m_planetName);
	addSharedVariable    (m_regionName);
	addSharedVariable    (m_waypointVisible);
	addSharedVariable    (m_waypointColor);

	m_waypointActive.setSourceObject (this);
	m_location.setSourceObject       (this);
	m_planetName.setSourceObject     (this);
	m_waypointColor.setSourceObject  (this);
	m_cell.setSourceObject(this);

	if(getNetworkId() == NetworkId::cms_invalid)
		setNetworkId(ClientObject::getNextFakeNetworkId());

	if (!_stricmp (t->getName (), DEFAULT_SHARED_WAYPOINT_TEMPLATE))
		s_regularWaypoints.push_back (ConstWatcher<ClientWaypointObject>(this));

	m_isBuildoutWaypoint = NULL != SharedBuildoutAreaManager::findBuildoutAreaAtPosition(getLocation(), true);
}

//-----------------------------------------------------------------------

ClientWaypointObject::~ClientWaypointObject()
{
	s_regularWaypoints.erase (std::remove (s_regularWaypoints.begin (), s_regularWaypoints.end (), this), s_regularWaypoints.end ());
	s_poiWaypoints.erase     (std::remove (s_poiWaypoints.begin (),     s_poiWaypoints.end (), this),     s_poiWaypoints.end ());
	setWaypointActive(false);
	removeWorldObject();
}

//-----------------------------------------------------------------------

const NetworkId & ClientWaypointObject::getCell() const
{
	return m_cell.get();
}

//-----------------------------------------------------------------------

const std::string ClientWaypointObject::getPlanetName() const
{
	return Game::calculateNonInstanceSceneId(m_planetName.get());
}

//-----------------------------------------------------------------------

const Unicode::String & ClientWaypointObject::getRegionName() const
{
	return m_regionName.get();
}

//-----------------------------------------------------------------------

const Vector & ClientWaypointObject::getLocation() const
{
	return m_location.get();
}

//----------------------------------------------------------------------

bool ClientWaypointObject::equals (const WaypointDataBase & wd) const
{
	Vector const & location = getLocation ();

	if (location.x != wd.m_location.getCoordinates().x || location.z != wd.m_location.getCoordinates().z)
		return false;

	if (getPlanetName() != Game::calculateNonInstanceSceneId(wd.m_location.getSceneId()))
		return false;
	
	if (Game::isSpaceSceneName(getPlanetName()))
	{
		if (location.y != wd.m_location.getCoordinates().y)
			return false;
	}

	if (!wd.m_name.empty ())
	{
		if (getLocalizedName () != wd.m_name)
			return false;
	}

	return true;
}

//----------------------------------------------------------------------
// In our list of active waypoints for a given quest is one of the
// points an entrance specific point
//----------------------------------------------------------------------
bool ClientWaypointObject::isEntranceUsed() const
{
	bool isUsingEntrance = false;

	for (ClientWaypointObject::WaypointVector::iterator i = s_activeWaypoints.begin(); i != s_activeWaypoints.end(); ++i)
	{
		ClientWaypointObject * const waypoint = i->getPointer();
	
		if (waypoint && waypoint->isEntrance())
		{
			isUsingEntrance = true;
			break;
		}
	}

	return isUsingEntrance;
}

//----------------------------------------------------------------------

ClientWaypointObject & ClientWaypointObject::operator = (const Waypoint & rhs)
{
	static const Unicode::String planet_prefix = Unicode::narrowToWide ("@planet_n:");

	m_waypointActive     = rhs.isActive();
	const Location & loc = rhs.getLocation();
	m_cell               = loc.getCell();
	m_location           = loc.getCoordinates();
	m_planetName         = loc.getSceneId();
	m_waypointColor      = Waypoint::getColorNameById(rhs.getColor());

	if (!rhs.getName ().empty ())
		setObjectName (StringId::decodeString(rhs.getName()));
	else
		setObjectName (planet_prefix + Unicode::narrowToWide(getPlanetName()));

	return *this;
}

//----------------------------------------------------------------------

ClientWaypointObject * ClientWaypointObject::createClientWaypoint (const Waypoint & wd, bool isGroupWaypoint)
{
	Vector          pos_w (wd.getLocation().getCoordinates().x, wd.getLocation().getCoordinates().y, wd.getLocation().getCoordinates().z);
	Unicode::String name;

	if (!wd.getName().empty ())
	{
		name = wd.getName();
	}
	else
	{
		const char * const sceneId = Location::getSceneNameByCrc(wd.getLocation().getSceneIdCrc());
		if(strlen(sceneId) != 0)
		{
			StringId sid("planet_n", sceneId);
			name = sid.localize();
		}
		else
			name = Unicode::emptyString;
	}

	bool const active = wd.isActive() && (!isGroupWaypoint || CuiPreferences::getShowGroupWaypoints());
	ClientWaypointObject * wp = createClientWaypoint (name, wd.getLocation().getSceneId(), pos_w, Waypoint::getColorNameById(wd.getColor()), active, 0, false, isGroupWaypoint);
	if (wp)
	{
		wp->setNetworkId(wd.getNetworkId());
		wp->m_cell = wd.getLocation().getCell();
	}

	return wp;
}

//----------------------------------------------------------------------

ClientWaypointObject * ClientWaypointObject::createClientWaypoint(const Unicode::String & name, const std::string planetName, const Vector & location,  const std::string& color, bool active, const char * appearance, bool isPoi, bool isGroupWaypoint)
{
	ClientWaypointObject * const waypoint = dynamic_cast<ClientWaypointObject *>(ObjectTemplate::createObject (DEFAULT_SHARED_WAYPOINT_TEMPLATE));

	if (!waypoint)
	{
		WARNING (true, ("Failed to create client waypoint"));
		return 0;
	}

	waypoint->endBaselines();
	waypoint->m_cell       = NetworkId::cms_invalid;
	waypoint->m_location   = location;
	waypoint->m_planetName = Game::calculateNonInstanceSceneId(planetName);

	waypoint->setObjectName (StringId::decodeString(name));
	waypoint->setWaypointActive(active);

	waypoint->m_waypointColor     = color;

	if(appearance)
		waypoint->m_appearance = appearance;

	if(!waypoint->m_appearance.empty())
	{
		waypoint->setAppearanceByName (appearance);
	}

	if (waypoint->isSmallWaypoint() && !appearance)
	{
		waypoint->setAppearanceByName(SMALL_WAYPOINT_APPEARANCE);
		waypoint->m_appearance = SMALL_WAYPOINT_APPEARANCE;
	}

	waypoint->m_isPoi = isPoi;
	if (waypoint->m_isPoi)
	{
		//move wp from the regular list to the poi list
		for (ConstWaypointVector::iterator i = s_regularWaypoints.begin(); i != s_regularWaypoints.end(); ++i)
		{
			ConstWaypointVector::iterator it = std::find(s_regularWaypoints.begin(), s_regularWaypoints.end(), waypoint);
			if (it != s_regularWaypoints.end())
			{
				s_regularWaypoints.erase(it);
				break;
			}
		}
		s_poiWaypoints.push_back(ConstWatcher<ClientWaypointObject>(waypoint));
	}

	waypoint->m_isGroupWaypoint = isGroupWaypoint;
	waypoint->setVolumeClientSideOnly(0);

	return waypoint;
}

//----------------------------------------------------------------------

ClientWaypointObject * ClientWaypointObject::createClientWaypoint (const ClientObject &target)
{	
	ClientWaypointObject * const waypoint = createClientWaypoint (target.getLocalizedName (), Game::getNonInstanceSceneId(), target.findPosition_w (), getRandomColor (), true);

	if (!waypoint)
	{
		WARNING (true, ("Failed to create client waypoint"));
		return 0;
	}

	const CellProperty * const cellProperty = target.getParentCell ();

	waypoint->m_cell       = cellProperty ? cellProperty->getOwner ().getNetworkId () : NetworkId::cms_invalid; // rls - this will not work!
	waypoint->setObjectNameStringId (target.getObjectNameStringId ());

	return waypoint;
}

//----------------------------------------------------------------------

bool ClientWaypointObject::setWaypointActive (bool b)
{
	static const Unicode::String on = Unicode::narrowToWide("on");
	static const Unicode::String off = Unicode::narrowToWide("off");
	const Unicode::String * status;
	
	if((b && m_waypointActive.get()) || (!b && !m_waypointActive.get()))
		return true; // already in the right state
	
	if (b && Game::getNonInstanceSceneId() != getPlanetName())
		return false; // wrong planet, not allowed to activate

	if (b)
	{
		Vector location_w;
		if (!computeBuildoutLocation_w(location_w))
			return false;
	}

	if(b)
		status = &on;
	else
		status = &off;

	if (   getNetworkId() != NetworkId::cms_invalid
	    && !ClientObject::isFakeNetworkId(getNetworkId())
	    && !Game::getSinglePlayer()
	    && !isGroupWaypoint())
		ClientCommandQueue::enqueueCommand("setWaypointActiveStatus", getNetworkId(), *status);

	m_waypointActive.set(b);

	return true;
}

//----------------------------------------------------------------------

const ClientWaypointObject::WaypointVector & ClientWaypointObject::getActiveWaypoints ()
{
	return s_activeWaypoints;
}

//----------------------------------------------------------------------

const ClientWaypointObject::ConstWaypointVector & ClientWaypointObject::getRegularWaypoints ()
{
	return s_regularWaypoints;
}

//----------------------------------------------------------------------

const ClientWaypointObject::ConstWaypointVector & ClientWaypointObject::getPoiWaypoints ()
{
	return s_poiWaypoints;
}

//-----------------------------------------------------------------

void ClientWaypointObject::requestWaypoint (const ClientObject & target)
{
	CreatureObject * const player = Game::getPlayerCreature ();
	if (!player)
		return;
	
	if (Game::getSinglePlayer ())
	{
		ClientObject * const datapad = player->getDatapadObject ();
		if (datapad)
		{
			ClientWaypointObject * const waypoint = createClientWaypoint(target);
			
			if (waypoint)
				ContainerInterface::transferItemToVolumeContainer(*datapad, *waypoint);
		}
		else
			WARNING (true, ("No client datapad"));
		
		return;
	}
	else
	{
		Controller * const controller = NON_NULL (player->getController());
		
		MessageQueueNetworkId * const msg = new MessageQueueNetworkId(target.getNetworkId ());
		
		//-- enqueue message
		controller->appendMessage (CM_getWaypointForObject, 0.0f, msg, 
			GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_SERVER);
	}
}

//----------------------------------------------------------------------

void ClientWaypointObject::requestWaypoint      (const WaypointDataBase & wd)
{
	requestWaypoint(
		wd.m_name,
		wd.m_location.getSceneId(),
		Vector(wd.m_location.getCoordinates().x, wd.m_location.getCoordinates().y, wd.m_location.getCoordinates().z),
		wd.m_color);
}

//----------------------------------------------------------------------

void ClientWaypointObject::requestWaypoint(const Unicode::String & name, const Vector & pos_w)
{
	requestWaypoint (name, Game::getNonInstanceSceneId(), pos_w, static_cast<uint8>(Waypoint::Blue));
}

//----------------------------------------------------------------------

void ClientWaypointObject::requestWaypoint      (const Unicode::String & name, const std::string & planet, const Vector & pos_w, uint8 color)
{
	CreatureObject * const player = Game::getPlayerCreature ();
	if (!player)
		return;
	
	if (planet.empty ())
	{
		WARNING (true, ("Waypoint::requestWaypoint with empty planet name"));
		return;		
	}
	
	if (Game::getSinglePlayer ())
	{		
		ClientObject * const datapad = player->getDatapadObject ();
		if (datapad)
		{
			ClientWaypointObject * const waypoint = createClientWaypoint (name, Game::getNonInstanceSceneId(), pos_w, getRandomColor (), true);
			
			if (waypoint)
				ContainerInterface::transferItemToVolumeContainer(*datapad, *waypoint);
		}
		else
			WARNING (true, ("No client datapad"));
		
		return;
	}
	else
	{
		static const uint32 has_req_waypoint = Crc::normalizeAndCalculate ("requestWaypointAtPosition");
		char buf [256];

		// obfuscation to prevent player from manually entering the
		// requestWaypointAtPosition command and specifying the waypoint color
		snprintf (buf, sizeof (buf), "(^-,=+_)color_%s(,+-=_^)=%u %s %f %f %f ", player->getNetworkId().getValueString().c_str(), color, planet.c_str (), pos_w.x, pos_w.y, pos_w.z);
		Unicode::String params (Unicode::narrowToWide (buf));
		params += name;
		ClientCommandQueue::enqueueCommand (has_req_waypoint, NetworkId::cms_invalid, params);
	}
}

//-----------------------------------------------------------------

void ClientWaypointObject::Callbacks::ActiveChange::modified (ClientWaypointObject & target, const bool & old, const bool & value, bool) const
{
	if (value)
	{
		if (std::find (s_activeWaypoints.begin (), s_activeWaypoints.end (), static_cast<ClientWaypointObject const *>(&target)) == s_activeWaypoints.end ())
			s_activeWaypoints.push_back (Watcher<ClientWaypointObject>(&target));		
	}
	else
		s_activeWaypoints.erase (std::remove (s_activeWaypoints.begin (), s_activeWaypoints.end (), static_cast<ClientWaypointObject const *>(&target)), s_activeWaypoints.end ());

	if (old && !value)
		target.removeWorldObject ();
	else if (value && !old)
		target.addWorldObject ();

	updateSingleWaypoint(&target);

	Transceivers::activatedChanged.emitMessage (target);
}

//-----------------------------------------------------------------------

void ClientWaypointObject::Callbacks::LocationChange::modified (ClientWaypointObject & target, const Vector &, const Vector &, bool) const
{
	target.m_isBuildoutWaypoint = NULL != SharedBuildoutAreaManager::findBuildoutAreaAtPosition(target.getLocation(), true);

	if (target.isWaypointActive())
	{
		target.removeWorldObject();
		target.addWorldObject();		
	}

	Transceivers::locationChanged.emitMessage (target);
}

//-----------------------------------------------------------------------

void ClientWaypointObject::Callbacks::ColorChange::modified (ClientWaypointObject & target, const std::string &, const std::string & , bool) const
{
	if (target.isWaypointActive ())
	{
		target.removeWorldObject ();
		target.addWorldObject    ();
	}

	Transceivers::colorChanged.emitMessage (target);

}

//----------------------------------------------------------------------

void ClientWaypointObject::Callbacks::PlanetChange::modified (ClientWaypointObject & target, const std::string &, const std::string & val, bool) const
{
	if(target.m_appearance.empty())
	{
		const std::string & appName = getAppearanceForPlanet (val);
		target.setAppearanceByName (appName.c_str ());
	}

	if (target.isWaypointActive())
	{
		target.removeWorldObject();
		target.addWorldObject();		
	}
}


//-----------------------------------------------------------------------

void ClientWaypointObject::Callbacks::CellChange::modified(ClientWaypointObject & target, const NetworkId &, const NetworkId &, bool) const
{
	target.updateCustomAppearance();
}

//-----------------------------------------------------------------

void ClientWaypointObject::removeWorldObject ()
{
	if (m_worldObject)
	{
		m_worldObject->removeFromWorld ();
		delete m_worldObject.getPointer ();
		m_worldObject = 0;
	}
}

//-----------------------------------------------------------------------

void ClientWaypointObject::setVisible (const bool isVisible)
{
	m_waypointVisible.set (isVisible);
}

//-----------------------------------------------------------------

void ClientWaypointObject::addWorldObject()
{
	// @todo: test waypoint planet name vs. Game::getScene () name
	if (m_worldObject)
		return;

	Vector location_w;
	if (!computeBuildoutLocation_w(location_w))
	{
		WARNING(true, ("ClientWaypointObject attempted to create a client waypoint in the world in invalid buildout"));
		return;
	}

	std::string const & color = m_waypointColor.get();

	ClientWaypointObject * const wp = createWorldWaypointForColor (color);

	m_worldObject = wp;
	m_worldObject->m_waypointColor = color;

	if (m_worldObject)
	{
		RenderWorld::addObjectNotifications(*m_worldObject);
		m_worldObject->addToWorld();

		TerrainObject const * const terrain = TerrainObject::getInstance ();
		
		Vector pos (location_w);
		if (_isnan(pos.x) || _isnan(pos.z))
		{
			pos = Vector::zero;
		}

		if (_isnan(pos.y))
		{
			pos.y = 0.0f;
		}
		
		if (terrain && !Game::isSpace())
			terrain->getHeight (pos, pos.y);
		
		m_worldObject->setPosition_w(pos);
	}
}

//----------------------------------------------------------------------

float ClientWaypointObject::alter (float time)
{
	if (IntangibleObject::alter (time) == AlterResult::cms_kill)
		return AlterResult::cms_kill;

	CreatureObject const * const playerCreature = Game::getPlayerCreature();
	if (playerCreature) 
	{		
		if (m_worldObject)
		{
			CellProperty * const cellProperty = playerCreature->getParentCell() ? playerCreature->getParentCell() : CellProperty::getWorldCellProperty();

			if (m_cell.get().getValue()) 
			{
				// update the appearance based on the cell.
				CellProperty * const desiredWorldObjectCell = cellProperty ? cellProperty : CellProperty::getWorldCellProperty();
				CellProperty * const currentWorldObjectCell = m_worldObject->getParentCell() ? m_worldObject->getParentCell() : CellProperty::getWorldCellProperty();

				if (desiredWorldObjectCell)
				{
					if (!desiredWorldObjectCell->isWorldCell())
						m_worldObject->setParentCell(desiredWorldObjectCell);

					if (desiredWorldObjectCell->isWorldCell() != currentWorldObjectCell->isWorldCell()) 
					{
						if (desiredWorldObjectCell->isWorldCell()) 
						{
							removeWorldObject();
							addWorldObject();

							if (isEntranceUsed() && !m_worldObject->isEntrance())
								m_worldObject->setAppearanceByName(SMALL_WAYPOINT_APPEARANCE);
						}
						else
						{
							if (!m_worldObject->isEntrance())
							{
								if (m_appearance != SMALL_WAYPOINT_APPEARANCE)
									m_worldObject->setAppearanceByName(SMALL_WAYPOINT_APPEARANCE);
							}
						}

					}
					else
					{
						if (desiredWorldObjectCell->isWorldCell())
						{
							if (!m_worldObject->isEntrance() && m_worldObject->isSmallWaypoint())
							{
								if (m_appearance != SMALL_WAYPOINT_APPEARANCE)
									m_worldObject->setAppearanceByName(SMALL_WAYPOINT_APPEARANCE);
							}
						}

					}
				}
			}
			else
			{
				m_worldObject->setParentCell(cellProperty);
			}
		}

		std::string const & sceneName = Game::getSceneId();
		Vector const & playerPosition_w = playerCreature->getPosition_w();
		m_relativeWaypointPosition = GroundZoneManager::getRelativePositionFromPlayer(sceneName.c_str(), playerPosition_w, m_worldObject ? m_location.get() : getPosition_w());
	
		if (!m_worldObject && isInWorld())
		{
			TerrainObject const * const terrain = TerrainObject::getInstance();		
			if (terrain && !Game::isSpace())
			{
				float y = 0.0f;
				if (terrain->getHeight(m_relativeWaypointPosition, y))
				{
					m_relativeWaypointPosition.set(m_relativeWaypointPosition.x, y, m_relativeWaypointPosition.z);
				}
			}
		}
		
		setPosition_w(m_relativeWaypointPosition);
	}

	// @todo set to proper value.
	return AlterResult::cms_alterNextFrame;
}

//----------------------------------------------------------------------

const VectorArgb & ClientWaypointObject::getColorArgb         () const
{
	if (!s_installed)
		ClientWaypointObjectNamespace::install ();
	
	ColorMap::const_iterator it = s_colorMapping.find (m_waypointColor.get ());
	if (it == s_colorMapping.end ())
		return VectorArgb::solidWhite;
	
	return (*it).second.second;
}

//----------------------------------------------------------------------

void ClientWaypointObject::setDescription (const StringId & desc)
{
	m_description = desc;
}

//----------------------------------------------------------------------

const StringId & ClientWaypointObject::getDescription () const
{
	return m_description;
}

//-----------------------------------------------------------------------

void ClientWaypointObject::checkWaypoints(float const deltaTime)
{
	std::string const & sceneId = Game::getNonInstanceSceneId();

	static Timer waypointCheckTimer(1.0);
	if (waypointCheckTimer.updateZero(deltaTime))
	{
		std::vector<ClientWaypointObject *> waypointsToDeactivate;

		for (ClientWaypointObject::WaypointVector::const_iterator i=s_activeWaypoints.begin(); i!=s_activeWaypoints.end(); ++i)
		{
			Object * const object = i->getPointer();
			ClientWaypointObject * const waypoint = dynamic_cast<ClientWaypointObject *>(object);
			if (waypoint && waypoint->m_waypointActive.get())
			{
				if (sceneId != waypoint->getPlanetName())
				{
					waypointsToDeactivate.push_back(waypoint);
					continue;
				}
				
				if (!waypoint->m_worldObject.getPointer())
					continue;

				Vector location_w;
				if (waypoint->computeBuildoutLocation_w(location_w))
					waypoint->m_worldObject->setPosition_w(location_w);
				else
					waypointsToDeactivate.push_back(waypoint);
			}
		}

		for (std::vector<ClientWaypointObject *>::iterator j=waypointsToDeactivate.begin(); j!=waypointsToDeactivate.end(); ++j)
		{
			(*j)->setWaypointActive (false);
		}
	}
}

//----------------------------------------------------------------------

bool ClientWaypointObject::isEntrance() const
{
	if (m_waypointColor.get() == s_waypointEntrance)
		return true;

	return false;
}

//----------------------------------------------------------------------

bool ClientWaypointObject::isSmallWaypoint() const
{
	if (m_waypointColor.get() == s_waypointSmall)
		return true;

	return false;
}

//----------------------------------------------------------------------

bool ClientWaypointObject::isWaypointVisible() const
{
	if (m_waypointColor.get() != s_waypointInvisible)
		return true;

	return false;
}

//----------------------------------------------------------------------

bool ClientWaypointObject::computeBuildoutLocation_w(Vector & location_w) const
{
	CreatureObject * const player = Game::getPlayerCreature ();
	if (!player)
		return false;

	std::string const & sceneId = Game::getNonInstanceSceneId();

	//-- wrong planet
	if (sceneId != getPlanetName())
		return false;

	location_w = getLocation();

//	if (!m_isBuildoutWaypoint)
//		return true;

	BuildoutArea const * const playerBuildoutArea = SharedBuildoutAreaManager::findBuildoutAreaAtPosition(player->getPosition_w(), true);
	BuildoutArea const * const ba = SharedBuildoutAreaManager::findBuildoutAreaAtPosition(location_w, true);

	if (!playerBuildoutArea)
	{
		if (ba && ba->isolated)
			return false;

		return true;
	}
	else if (!ba)
	{
		if (playerBuildoutArea->isolated)
			return false;
		
		return true;
	}
	//-- both buildouts exist
	else
	{
		if (ba == playerBuildoutArea)
			return true;
		
		Rectangle2d const & baRect = ba->getRectangle(true);
		Rectangle2d const & playerBuildoutAreaRect = playerBuildoutArea->getRectangle(true);
		
		if (baRect == playerBuildoutAreaRect)
			return true;
		
		std::string const & playerBuildoutAreaName = playerBuildoutArea->getReferenceDisplayName();
		std::string const & baName = ba->getReferenceDisplayName();
		
		//-- different names, check isolation
		if (baName != playerBuildoutAreaName)
		{
			if (ba->isolated || playerBuildoutArea->isolated)
				return false;
		}
		
		//-- area has the same name, but is in a different location, compute
		//-- new buildout location for waypoint
		
		location_w = ba->getRelativePosition(location_w, true);
		
		Vector2d const & center = playerBuildoutAreaRect.getCenter();
		location_w.x += center.x;
		location_w.z += center.y;
		
		return true;
	}
}


//----------------------------------------------------------------------

void ClientWaypointObject::setSingleWaypointMode(bool enable, ClientObject * selectedObject, bool forceUpdate)
{
	if (!boolEqual(s_singleWaypointMode, enable) || forceUpdate) 
	{
		s_singleWaypointMode = enable;
		
		updateCachedActiveWaypointList(s_singleWaypointMode);
		
		if (s_singleWaypointMode) 
		{
			updateSingleWaypoint(dynamic_cast<ClientWaypointObject*>(selectedObject));
		}
	}
}

//-----------------------------------------------------------------------

Vector const & ClientWaypointObject::getRelativeWaypointPosition() const
{
	return m_worldObject ? m_worldObject->m_relativeWaypointPosition : m_relativeWaypointPosition;
}

//-----------------------------------------------------------------------

void ClientWaypointObject::updateCustomAppearance()
{
	char const * buildingAppearance = NULL;
	uint64 const buildingCell = static_cast<uint64>(m_cell.get().getValue()) >> static_cast<uint64>(32);
	if (buildingCell)
	{
		buildingAppearance = getAppearanceForBuilding(static_cast<uint32>(buildingCell));
		if (buildingAppearance) 
		{
			setAppearanceByName(buildingAppearance);
			m_appearance = buildingAppearance;
		}
	}
}
