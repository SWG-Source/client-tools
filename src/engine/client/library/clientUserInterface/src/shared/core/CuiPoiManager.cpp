//======================================================================
//
// CuiPoiManager.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiPoiManager.h"

#include "sharedFoundation/PointerDeleter.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"
#include "clientGame/ClientWaypointObject.h"

#include "clientUserInterface/ConfigClientUserInterface.h"

#include <algorithm>
#include <vector>

//======================================================================

namespace CuiPoiManagerNamespace
{
	bool s_installed = false;

	const std::string s_tableName   ("datatables/clientpoi/clientpoi.iff");
	bool s_fatalOnBadPoiEntry = true;
	bool s_loaded = false;
}

using namespace CuiPoiManagerNamespace;

//----------------------------------------------------------------------

std::vector<ClientWaypointObject*> CuiPoiManager::ms_poiWps;

//----------------------------------------------------------------------

void CuiPoiManager::install ()
{
	DEBUG_FATAL (s_installed, ("installed"));
	s_installed = true;

	s_fatalOnBadPoiEntry = ConfigClientUserInterface::getFatalOnBadPoiEntry();
}

//----------------------------------------------------------------------

void CuiPoiManager::remove ()
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	clearData();

	s_installed = false;
}

//----------------------------------------------------------------------

void CuiPoiManager::reloadData()
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	clearData();
	loadData();
}

//----------------------------------------------------------------------

void CuiPoiManager::clearData()
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	std::for_each(ms_poiWps.begin(), ms_poiWps.end(), PointerDeleter());
	ms_poiWps.clear();
	s_loaded = false;
}

//----------------------------------------------------------------------

void CuiPoiManager::loadData()
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	const DataTable * const fileTable = DataTableManager::getTable (s_tableName.c_str(), true);
	if (NULL == fileTable)
	{
		WARNING(true, ("CuiPoiManager::loadData failed for data table [%s]", s_tableName.c_str()));
		return;
	}

	const int numRows = fileTable->getNumRows ();

	for (int i = 0; i < numRows; ++i)
	{
		const std::string & name        = fileTable->getStringValue (0, i);
		const std::string & desc        = fileTable->getStringValue (1, i);
		const std::string & planet      = fileTable->getStringValue (2, i);
		const std::string & appearance  = fileTable->getStringValue (3, i);
		float x                         = fileTable->getFloatValue  (4, i);
		float y                         = fileTable->getFloatValue  (5, i);
		float z                         = fileTable->getFloatValue  (6, i);

		ClientWaypointObject * const newWp = ClientWaypointObject::createClientWaypoint (StringId(name).localize(), planet, Vector(x, y, z),  "yellow", false, appearance.c_str(), true);
		if(newWp)
		{
			newWp->setDescription(StringId(desc));
			ms_poiWps.push_back(newWp);
		}
	}

	s_loaded = true;
}

//----------------------------------------------------------------------

const std::vector<ClientWaypointObject*> & CuiPoiManager::getData()
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	if (!s_loaded)
		loadData();

	return ms_poiWps;
}

//======================================================================
