//======================================================================
//
// ZoneMapSpaceStationTable.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ZoneMapSpaceStationTable.h"

#include "sharedFile/TreeFile.h"
#include "sharedUtility/DataTableManager.h"
#include "sharedUtility/DataTable.h"

#include <vector>

//===================================================================

namespace ZoneMapSpaceStationTableNamespace
{
	static std::string const cs_baseDir("datatables/space/spacestation/");
	static std::string const cs_filenameExt(".iff");

	std::string s_currentScene;
	ZoneMapSpaceStationTable::DataVector s_dataVector;
}

//======================================================================

using namespace ZoneMapSpaceStationTableNamespace;

//======================================================================

void ZoneMapSpaceStationTable::loadForScene(std::string const & sceneName)
{
	if (s_currentScene != sceneName)
	{
		s_dataVector.clear();
		
		std::string const tableName = cs_baseDir + sceneName + cs_filenameExt;
		if(!TreeFile::exists(tableName.c_str()))
		{
			DEBUG_WARNING(true, ("Space station data table not defined for %s", sceneName.c_str()));
			return;
		}

		DataTable * const table = DataTableManager::getTable(tableName, true);

		if (table != 0)
		{
			s_currentScene = sceneName;
			
			int const numRows = table->getNumRows();

			for (int i = 0; i < numRows; ++i)
			{
				ZoneMapSpaceStationTable::Data data;

				std::string const unlocalizedName = table->getStringValue("Name", i);
				StringId const localizedName("space/space_mobile_type", unlocalizedName);

				data.m_location.x = table->getFloatValue("LocationX", i);
				data.m_location.y = table->getFloatValue("LocationY", i);
				data.m_location.z = table->getFloatValue("LocationZ", i);
				data.m_localizedName = localizedName.localize();

				s_dataVector.push_back(data);
			}
		}

		DataTableManager::close(tableName);
	}
}

ZoneMapSpaceStationTable::DataVector const & ZoneMapSpaceStationTable::getData()
{
	return s_dataVector;
}

//======================================================================
