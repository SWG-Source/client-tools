//======================================================================
//
// ClientRegionManager.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientRegionManager.h"

#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedGame/ProsePackage.h"
#include "sharedMath/SpatialSubdivision.h"
#include "sharedMessageDispatch/Message.h"
#include "sharedMessageDispatch/Receiver.h"
#include "sharedObject/Object.h"
#include "sharedTerrain/TerrainObject.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"

#include "clientGame/ConfigClientGame.h"
#include "clientGame/Game.h"
#include "clientGame/ProsePackageManagerClient.h"
#include "clientGraphics/Graphics.h"

#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiSystemMessageManager.h"

#include <algorithm>
#include <map>

//======================================================================

SphereTree<ClientRegionManager::ClientRegionSphere*, ClientRegionManager::ClientRegionSphereExtentAccessor> ClientRegionManager::m_clientRegionsSphereTree;
std::vector<SpatialSubdivisionHandle*>   ClientRegionManager::m_objectsInSphereTree;
std::multimap<StringId, ClientRegionManager::ClientRegionSphere*>               ClientRegionManager::m_objectStringMap;
SphereTree<ClientRegionManager::ClientRegionSphere*, ClientRegionManager::ClientRegionSphereExtentAccessor> ClientRegionManager::m_tempClientRegionsSphereTree;
std::vector<SpatialSubdivisionHandle*>   ClientRegionManager::m_tempObjectsInSphereTree;
std::multimap<StringId, ClientRegionManager::ClientRegionSphere*>               ClientRegionManager::m_tempObjectStringMap;

//======================================================================

namespace ClientRegionManagerNamespace
{
	class Listener : public MessageDispatch::Receiver
	{
	public:
		Listener ()
		{
			connectToMessage(Game::Messages::SCENE_CHANGED);
		}

		~Listener ()
		{
			disconnectFromMessage(Game::Messages::SCENE_CHANGED);
		}

		void receiveMessage(const MessageDispatch::Emitter& , const MessageDispatch::MessageBase& message)
		{
			if(message.isType(Game::Messages::SCENE_CHANGED))
			{
				if(!Game::getSceneId().empty())
					ClientRegionManager::loadPlanet(Game::getSceneId());
			}
		}
	};
	Listener* s_listener;

	const float s_yValue             = 0.0f;
	const std::string s_baseDir     ("datatables/clientregion/");
	const std::string s_filenameExt (".iff");
	const float s_regionCheckTimeOut = 5.0f;

	bool s_installed                 = false;
	bool s_wrongSceneWarningShown    = false;
	float s_timer                    = 0.0f;
	std::string s_currentPlanet;
	ClientRegionManager::ClientRegionSphere s_lastCurrentRegion;
	bool s_drawRegions               = false;
}

using namespace ClientRegionManagerNamespace;

// =====================================================================

void ClientRegionManager::install ()
{
	InstallTimer const installTimer("ClientRegionManager::install");

	DEBUG_FATAL (s_installed, ("ClientRegionManager double install"));

	s_currentPlanet.clear();
	s_timer = 0.0f;
	s_wrongSceneWarningShown = false;
	s_installed = true;
	clearSphereTree();
	s_lastCurrentRegion.m_name.clear();
	s_listener = new Listener;

	DebugFlags::registerFlag(s_drawRegions,  "ClientGame", "drawClientRegions");
}

//----------------------------------------------------------------------

void ClientRegionManager::remove  ()
{
	DEBUG_FATAL (!s_installed, ("ClientRegionManager not installed"));

	delete s_listener;
	s_listener = NULL;

	s_currentPlanet.clear();
	s_timer = 0.0f;
	s_wrongSceneWarningShown = false;
	clearSphereTree();
	s_lastCurrentRegion.m_name.clear();
	s_installed = false;
}

//----------------------------------------------------------------------

void ClientRegionManager::update  (float deltaTime)
{
	DEBUG_FATAL (!s_installed, ("ClientRegionManager not installed"));

	//check for region changes every s_regionCheckTimeOut seconds (it will be a pretty cheap call, but this doesn't have to happen ALL that often)
	s_timer += deltaTime;
	if(s_timer >= s_regionCheckTimeOut)
	{
		checkCurrentRegion();
		s_timer = 0.0f;
	}
}

//----------------------------------------------------------------------

void ClientRegionManager::clearLastRegion ()
{
	s_lastCurrentRegion.m_name.clear();
}

//----------------------------------------------------------------------

void ClientRegionManager::loadPlanet (const std::string& planetName)
{
	DEBUG_FATAL (!s_installed, ("ClientRegionManager not installed"));

	clearSphereTree();
	s_currentPlanet = planetName;

	//don't bother trying to load test scene data (since there isn't any)
	if(planetName == "simple" || planetName == "tutorial")
		return;

	std::string tableName = s_baseDir + planetName + s_filenameExt;
	if(!TreeFile::exists(tableName.c_str()))
	{
		return;
	}

	DataTable* table = DataTableManager::getTable(tableName, true);
	if(table)
	{
		const int numRows = table->getNumRows ();

		for (int i = 0; i < numRows; ++i)
		{
			const std::string & stringid = table->getStringValue(0, i);
			float x                      = table->getFloatValue (1, i);
			float z                      = table->getFloatValue (2, i);
			float radius                 = table->getFloatValue (3, i);

			StringId name(stringid);
			ClientRegionSphere* s = new ClientRegionSphere;
			s->setCenter(x, s_yValue, z);
			s->setRadius(radius);
			s->m_name = stringid;

			SpatialSubdivisionHandle* handle = m_clientRegionsSphereTree.addObject(s);
			if(handle)
				m_objectsInSphereTree.push_back(handle);
			m_objectStringMap.insert(std::make_pair(name, s));
		}
	}
	else
	{
		DEBUG_WARNING(true, ("Client region data not defined for %s", planetName.c_str()));
	}
}

//----------------------------------------------------------------------

void ClientRegionManager::tempLoadPlanet(const std::string& planetName)
{
	DEBUG_FATAL (!s_installed, ("ClientRegionManager not installed"));

	clearTempSphereTree();

	std::string tableName = s_baseDir + planetName + s_filenameExt;
	DataTable* table = DataTableManager::getTable(tableName, true);
	if(table)
	{
		const int numRows = table->getNumRows ();

		for (int i = 0; i < numRows; ++i)
		{
			const std::string & stringid = table->getStringValue(0, i);
			float x                      = table->getFloatValue (1, i);
			float z                      = table->getFloatValue (2, i);
			float radius                 = table->getFloatValue (3, i);

			StringId name(stringid);
			ClientRegionSphere* s = new ClientRegionSphere;
			s->setCenter(x, s_yValue, z);
			s->setRadius(radius);
			s->m_name = stringid;

			SpatialSubdivisionHandle* handle = m_tempClientRegionsSphereTree.addObject(s);
			if(handle)
				m_tempObjectsInSphereTree.push_back(handle);
			m_tempObjectStringMap.insert(std::make_pair(name, s));
		}
	}
	else
	{
		DEBUG_WARNING(true, ("Couldn't open table %s", tableName.c_str()));
	}
}

//----------------------------------------------------------------------

void ClientRegionManager::drawRegions()
{
	if(s_drawRegions)
	{
		std::vector<std::pair<ClientRegionSphere*, Sphere> > result;
		m_clientRegionsSphereTree.dumpSphereTree(result);

		Vector loc;
		float height;
		for(std::vector<std::pair<ClientRegionSphere*, Sphere> >::iterator i = result.begin(); i != result.end(); ++i)
		{
			ClientRegionSphere* region = i->first;
			if(region)
			{
				loc = region->getCenter();
				//drop sphere to terrain if possible
				TerrainObject* terrain = TerrainObject::getInstance();
				if(terrain)
				{
					terrain->getHeight(loc, height);
					loc.set(loc.x, height, loc.z);
				}
				Graphics::drawCylinder(loc, region->getRadius(), 250, 16, 16, 16, 16, VectorArgb::solidRed);
			}
		}
	}
}

//----------------------------------------------------------------------

void ClientRegionManager::checkCurrentRegion ()
{
	DEBUG_FATAL (!s_installed, ("ClientRegionManager not installed"));

	//make sure we're in a scene with a player to work with
	Object* player = Game::getPlayer();
	if(!player)
	{
		//this can legitimately happen (quit from a scene, etc.)
		return;
	}

	//if we somehow get out of sync, don't display region data for the wrong scene
	if(Game::getSceneId() != s_currentPlanet)
	{
		if(!s_wrongSceneWarningShown)
		{
			DEBUG_WARNING(true, ("Current scene is %s, but ClientRegionManager is loaded with scene %s", Game::getSceneId().c_str(), s_currentPlanet.c_str()));
			s_wrongSceneWarningShown = true;
		}
		return;
	}

	Vector loc = player->getPosition_w();
	//push location down to 0y, where the region spheres are centered
	loc.y = 0.0f;
	std::vector<ClientRegionSphere*> result;
	m_clientRegionsSphereTree.findInRange(loc, 1.0, result);

	//find the smallest location that the player is in
	ClientRegionSphere s;
	ClientRegionSphere* newCurrentRegion = NULL;
	for(std::vector<ClientRegionSphere*>::iterator i = result.begin(); i != result.end(); ++i)
	{
		if(!newCurrentRegion)
			newCurrentRegion = *i;
		else
		{
			if((*i)->getRadius() < newCurrentRegion->getRadius())
				newCurrentRegion = *i;
		}
	}

	//see if we left a region
	if(!newCurrentRegion || newCurrentRegion->m_name != s_lastCurrentRegion.m_name)
	{
		if(!s_lastCurrentRegion.m_name.empty())
		{
			ProsePackage p;
			p.stringId = CuiStringIds::region_left;
			p.target.stringId = StringId(s_lastCurrentRegion.m_name);
			Unicode::String result;
			ProsePackageManagerClient::appendTranslation(p, result);
			CuiSystemMessageManager::sendFakeSystemMessage (result);
			s_lastCurrentRegion.m_name.clear();
		}
	}

	//if we're not in any region, clear out the last region so that we can possibly reenter it
	if(!newCurrentRegion)
		s_lastCurrentRegion.m_name.clear();

	//see if we entered a new region
	if(newCurrentRegion && s_lastCurrentRegion.m_name != newCurrentRegion->m_name)
	{
		s_lastCurrentRegion = *newCurrentRegion;

		if(ConfigClientGame::getShowClientRegionChanges())
		{
			ProsePackage p;
			p.stringId = CuiStringIds::region_entered;
			p.target.stringId = StringId(s_lastCurrentRegion.m_name);
			Unicode::String result;
			ProsePackageManagerClient::appendTranslation(p, result);
			CuiSystemMessageManager::sendFakeSystemMessage (result);
		}
	}
}

//----------------------------------------------------------------------

void ClientRegionManager::clearSphereTree ()
{
	DEBUG_FATAL (!s_installed, ("ClientRegionManager not installed"));

	SpatialSubdivisionHandle* handle;
	for(std::vector<SpatialSubdivisionHandle*>::iterator i = m_objectsInSphereTree.begin(); i != m_objectsInSphereTree.end(); ++i)
	{
		handle = *i;
		if(handle)
			m_clientRegionsSphereTree.removeObject(handle);
	}
	m_objectsInSphereTree.clear();

	std::for_each(m_objectStringMap.begin(), m_objectStringMap.end(), PointerDeleterPairSecond());
	m_objectStringMap.clear();
}

//----------------------------------------------------------------------

void ClientRegionManager::clearTempSphereTree ()
{
	DEBUG_FATAL (!s_installed, ("ClientRegionManager not installed"));

	SpatialSubdivisionHandle* handle;
	for(std::vector<SpatialSubdivisionHandle*>::iterator i = m_tempObjectsInSphereTree.begin(); i != m_tempObjectsInSphereTree.end(); ++i)
	{
		handle = *i;
		if(handle)
			m_tempClientRegionsSphereTree.removeObject(handle);
	}
	m_tempObjectsInSphereTree.clear();

	std::for_each(m_tempObjectStringMap.begin(), m_tempObjectStringMap.end(), PointerDeleterPairSecond());
	m_tempObjectStringMap.clear();
}

//======================================================================

StringId ClientRegionManager::getRegionAtPoint(const std::string& planet, const Vector& point)
{
	if(planet == s_currentPlanet)
	{
		return getRegionOnCurrentPlanetAtPoint(point);
	}
	else
	{
		StringId sid;
		tempLoadPlanet(planet);
		const ClientRegionSphere* region = getSmallestRegionAtPoint(m_tempClientRegionsSphereTree, point);
		if(region)
			sid = StringId(region->m_name);
		clearTempSphereTree();
		return sid;
	}
}

//======================================================================

const ClientRegionManager::ClientRegionSphere* ClientRegionManager::getSmallestRegionAtPoint(const SphereTree<ClientRegionManager::ClientRegionSphere*, ClientRegionSphereExtentAccessor>& sphereTree, const Vector& point)
{
	Vector loc = point;
	//push location down to 0y, where the region spheres are centered
	loc.y = 0.0f;
	std::vector<ClientRegionSphere*> result;
	sphereTree.findInRange(loc, 1.0, result);

	//find the smallest location that the player is in
	ClientRegionSphere s;
	ClientRegionSphere* region = NULL;
	for(std::vector<ClientRegionSphere*>::iterator i = result.begin(); i != result.end(); ++i)
	{
		if(!region)
			region = *i;
		else
		{
			if((*i)->getRadius() < region->getRadius())
				region = *i;
		}
	}
	return region;
}

//======================================================================

StringId ClientRegionManager::getRegionOnCurrentPlanetAtPoint(const Vector& point)
{
	Vector loc = point;
	//push location down to 0y, where the region spheres are centered
	loc.y = 0.0f;
	std::vector<ClientRegionSphere*> result;
	m_clientRegionsSphereTree.findInRange(loc, 1.0, result);

	//find the smallest location that the point is in
	ClientRegionSphere s;
	ClientRegionSphere* region = NULL;
	for(std::vector<ClientRegionSphere*>::iterator i = result.begin(); i != result.end(); ++i)
	{
		if(!region)
			region = *i;
		else
		{
			if((*i)->getRadius() < region->getRadius())
				region = *i;
		}
	}

	if(region)
	{
		return StringId(region->m_name);
	}
	//else return an invalid StringId
	return StringId();
}

//======================================================================

StringId ClientRegionManager::getLastCurrentRegion()
{
	return StringId(s_lastCurrentRegion.m_name);
}

//======================================================================

