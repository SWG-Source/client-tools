//======================================================================
//
// ZoneMapSpaceStationTable.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ZoneMapSpaceStationTable_H
#define INCLUDED_ZoneMapSpaceStationTable_H

//======================================================================

#include "sharedMath/Vector.h"
#include <string>

//======================================================================

class ZoneMapSpaceStationTable
{
public:
	struct Data
	{
		Vector m_location;
		Unicode::String m_localizedName;
	};

	typedef stdvector<Data>::fwd DataVector;

	static void loadForScene(std::string const & sceneName);
	static DataVector const & getData();

private:
	ZoneMapSpaceStationTable();
	ZoneMapSpaceStationTable(ZoneMapSpaceStationTable const &);
	ZoneMapSpaceStationTable & operator=(const ZoneMapSpaceStationTable &);
};

//======================================================================

#endif
