// ======================================================================
//
// SwgBattlefieldTool.cpp
//
// copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "FirstSwgBattlefieldTool.h"

#include "sharedFoundation/CommandLine.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/CrcStringTable.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedThread/SetupSharedThread.h"
#include "sharedCompression/SetupSharedCompression.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedIoWin/SetupSharedIoWin.h"
#include "sharedFile/FileNameUtils.h"
#include "sharedFile/Iff.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFile/TreeFile.h"

#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"
#include "sharedUtility/DataTableWriter.h"

#include <cstdio>
#include <string>
#include <map>

// ======================================================================

static char const *BATTLEFIELD_TABLE = "datatables/battlefield/battlefield.iff";
static char const *MARKER_TABLE = "datatables/battlefield/marker.iff";
static char const *PLANET_TABLE = "datatables/region/planets.iff";
static char const *TEMPLATE_TABLE = "misc/object_template_crc_string_table.iff";

// ======================================================================

class RegionInfo
{
public:

	RegionInfo() :
		m_x(0.0f), m_z(0.0f), m_radius(0.0f)
	{
	}

	RegionInfo(float x, float z, float radius) :
		m_x(x), m_z(z), m_radius(radius)
	{
	}

	RegionInfo(RegionInfo const &rhs) :
		m_x(rhs.m_x), m_z(rhs.m_z), m_radius(rhs.m_radius)
	{
	}

	RegionInfo &RegionInfo::operator=(RegionInfo const &rhs)
	{
		m_x = rhs.m_x;
		m_z = rhs.m_z;	
		m_radius = rhs.m_radius;
		return *this;
	}

	bool RegionInfo::operator==(RegionInfo const &rhs) const
	{
		return m_x == rhs.m_x && m_z == rhs.m_z && m_radius == rhs.m_radius;
	}

	bool RegionInfo::operator!=(RegionInfo const &rhs) const
	{
		return !operator==(rhs);
	}

public:
	float m_x, m_z, m_radius;
};

// ======================================================================

int roundToNode(float coord)
{
	int c = static_cast<int>(coord);
	return (c >= 0) ? (c/100)*100 : (((c-100)+1)/100)*100;
}

// ----------------------------------------------------------------------

void dump_marker_sql(float x, float y, float z, int template_id, std::string const &scene_id, std::string const &region_name)
{
printf(
"\n"
	"declare\n"
	"	new_obj_id number;\n"
	"begin\n"
	"	new_obj_id := objectidmanager.get_single_id();\n"
	"	insert into objects\n"
	"		(x, y, z, quaternion_w, quaternion_x, quaternion_y, quaternion_z,\n"
	"		node_x, node_y, node_z, object_template_id, type_id, scene_id,\n"
	"		controller_type, deleted, object_name, volume, contained_by,\n"
	"		slot_arrangement, player_controlled, cache_version, load_contents,\n"
	"		cash_balance, bank_balance, complexity, name_string_table,\n"
	"		name_string_text, load_with, script_list, object_id)\n"
	"		values\n"
	"		(%g, %g, %g, 1, 0, 0, 0,\n"
	"		%d, %d, %d, %d, 1112363595, '%s',\n"
	"		0, 0, '', 1, 0,\n"
	"		-1, 'N', 0, 'Y',\n"
	"		0, 0, 1, 'battlefield',\n"
	"		'battlefield_marker_name', new_obj_id, 'systems.battlefield.battlefield_region:', new_obj_id);\n"
	"	insert into tangible_objects\n"
	"		(max_hit_points, visible, interest_radius, pvp_type, pvp_faction,\n"
	"		count, condition, creator_id, source_draft_schematic, object_id)\n"
	"		values\n"
	"		(0, 'Y', 0, 0, 0,\n"
	"		0,768,0,0,new_obj_id);\n"
	"	insert into battlefield_marker_objects\n"
	"		(object_id, region_name)\n"
	"		values\n"
	"		(new_obj_id, '%s');\n"
	"end;\n"
	"/\n",
	static_cast<double>(x), static_cast<double>(y), static_cast<double>(z), roundToNode(x), roundToNode(y), roundToNode(z), template_id, scene_id.c_str(), region_name.c_str());
}

// ----------------------------------------------------------------------

void run()
{
	// grab the template crc info
	CrcStringTable objectTemplateTable;
	objectTemplateTable.load(TEMPLATE_TABLE);

	// grab the battlefield marker info
	std::map<float, int> markers;
	{
		DataTable *t = DataTableManager::getTable(MARKER_TABLE, true);
		FATAL(!t, ("Could not find %s!", MARKER_TABLE));
		for (int row = 0; row < t->getNumRows(); ++row)
		{
			float radius = t->getFloatValue("radius", row);
			markers[radius] = static_cast<int>(objectTemplateTable.lookUp(t->getStringValue("template", row)).getCrc());
		}
		DataTableManager::close(MARKER_TABLE);
	}

	// grab the region info
	std::map<std::string, std::map<std::string, RegionInfo> > regions;
	{
		std::map<std::string, std::string> planetRegionTables;
		{
			DataTable *t = DataTableManager::getTable(PLANET_TABLE, true);
			FATAL(!t, ("Could not find %s!", PLANET_TABLE));
			for (int row = 0; row < t->getNumRows(); ++row)
				planetRegionTables[t->getStringValue("planet", row)] = t->getStringValue("regionFile", row);
			DataTableManager::close(PLANET_TABLE);
		}
		for (std::map<std::string, std::string>::const_iterator i = planetRegionTables.begin(); i != planetRegionTables.end(); ++i)
		{
			DataTable *t = DataTableManager::getTable((*i).second.c_str(), true);
			FATAL(!t, ("Could not find %s!", (*i).second.c_str()));
			for (int row = 0; row < t->getNumRows(); ++row)
				regions[(*i).first][t->getStringValue("regionName", row)] = RegionInfo(
					t->getFloatValue("minX_or_centerX", row),
					t->getFloatValue("minZ_or_centerZ", row),
					t->getFloatValue("maxX_or_radius", row));
		}
	}

	// generate the sql, and validate region info as we go
	printf("update objects set deleted=1 where exists (select 1 from battlefield_marker_objects b where objects.object_id = b.object_id);\n");
	printf("delete from battlefield_participants;\n");

	{
		DataTable *t = DataTableManager::getTable(BATTLEFIELD_TABLE, true);
		FATAL(!t, ("Could not find %s!", BATTLEFIELD_TABLE));
		for (int row = 0; row < t->getNumRows(); ++row)
		{
			std::string bfname = t->getStringValue("battlefield", row);
			if (!bfname.empty())
			{
				std::string bfregion("@battlefield:");
				bfregion += bfname;
				std::string bfscene = t->getStringValue("area", row);
				float x = t->getFloatValue("x1", row);
				float y = t->getFloatValue("y1", row);
				float z = t->getFloatValue("z1", row);
				float radius = t->getFloatValue("radius", row);
				FATAL(regions[bfscene][bfregion] != RegionInfo(x, z, radius), ("Battlefield region does not match in region datatables! (%s in %s x=%g,z=%g,r=%g) -> (x=%g,z=%g,r=%g)",
					bfregion.c_str(), bfscene.c_str(), x, z, radius,
					regions[bfscene][bfregion].m_x, regions[bfscene][bfregion].m_z, regions[bfscene][bfregion].m_radius));
				dump_marker_sql(x, y, z, markers[radius], bfscene, bfregion);
			}
		}
		DataTableManager::close(BATTLEFIELD_TABLE);
	}
}

// ----------------------------------------------------------------------

int main(int argc, char **argv)
{
	SetupSharedThread::install();
	SetupSharedDebug::install(4096);

	{
		SetupSharedFoundation::Data data(SetupSharedFoundation::Data::D_console);
		data.argc  = argc;
		data.argv  = argv;
		SetupSharedFoundation::install(data);
	}

	//-- file
	SetupSharedCompression::install();
	SetupSharedFile::install(false);
	DataTableManager::install();

	//-- iowin
	SetupSharedIoWin::install();

	SetupSharedFoundation::callbackWithExceptionHandling(run);
	SetupSharedFoundation::remove();
	SetupSharedThread::remove();

	return 0;
}

// ======================================================================
