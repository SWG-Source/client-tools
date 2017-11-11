// ======================================================================
//
// UIPacking.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "_precompile.h"
#include "UIPacking.h"

#include "UiMemoryBlockManager.h"

#include <cassert>
#include <map>
#include <vector>
#include <set>

// ======================================================================

using namespace UIPacking;

//-----------------------------------------------------------------

UI_MEMORY_BLOCK_MANAGER_IMPLEMENTATION(UIPacking::LocationInfo, 2);
UI_MEMORY_BLOCK_MANAGER_IMPLEMENTATION(UIPacking::SizeInfo, 2);

//-----------------------------------------------------------------

const size_t SizeInfo::MAX_AUTO_TYPES     = static_cast<int>(SizeInfo::Proportional);
const size_t LocationInfo::MAX_REGIONS    = static_cast<int>(LocationInfo::Far);
const size_t LocationInfo::MAX_AUTO_TYPES = static_cast<int>(LocationInfo::Proportional);


//-----------------------------------------------------------------
//--
//-- local statics which define the strings associated with packing types
//--
//-----------------------------------------------------------------

static const char * const s_packSizeNames [SizeInfo::MAX_AUTO_TYPES + 1] =
{
	"Fixed",
	"Absolute",
	"HalfAbsolute",
	"Proportional",
};

static const char * const s_packSizeAbbrevs [SizeInfo::MAX_AUTO_TYPES + 1] = 
{
	"F", "A", "H", "P"
};

static const char * const s_packLocationRegionNames [LocationInfo::MAX_REGIONS + 1] =
{
	"Near",
	"Center",
	"Far"
};

static const char * const s_packLocationRegionAbbrevs [LocationInfo::MAX_REGIONS + 1] =
{
	"N", "C", "F"
};

static const char * const s_packLocationTypeNames [LocationInfo::MAX_AUTO_TYPES + 1] =
{
	"Fixed",
	"Proportional"
};

static const char * const s_packLocationTypeAbbrevs [LocationInfo::MAX_AUTO_TYPES + 1] = 
{
	"F", "P"
};

//-- backwards compatibility
static const char * s_nearFixedNearNames [] =
{
	"Left",
	"LeftFixed",
	"LeftFixedLeft",
	"Top",
	"TopFixed",
	"TopFixedTop",
	0
};

static const char * s_nearFixedFarNames [] =
{
	"Right",
	"LeftFixedRight",
	"Bottom",
	"TopFixedBottom",
	0
};

static const char * s_farFixedFarNames [] =
{
	"RightFixed",
	"RightFixedRight",
	"BottomFixed",
	0
};

static const char * s_nearFixedCenterNames [] =
{
	"Center",
	"LeftFixedCenter",
	"TopFixedCenter",
	0
};

static const char * s_farFixedCenterNames [] =
{
	"RightFixedCenter",
	"BottomFixedCenter",
	0
};

static const char * s_centerProportionalNearNames [] =
{
	"Proportional",
	"CenterProportionalLeft",
	"CenterProportionalTop",
	0,
};

static const char * s_nearProportionalNearNames [] =
{
	"LeftProportional",
	"TopProportional",
	0
};

static const char * s_farProportionalFarNames [] =
{
	"RightProportional",
	"BottomProportional",
	0
};

static const size_t NUM_REGIONS = LocationInfo::MAX_REGIONS + 1;
static const char ** const s_packLocationAliases [LocationInfo::MAX_AUTO_TYPES + 1][NUM_REGIONS][NUM_REGIONS] = 
{
	//-- fixed 3x3 matrix first
	{
		//-- widget near
		{
			s_nearFixedNearNames,
			s_nearFixedCenterNames,
			s_nearFixedFarNames
		},
		
		//-- widget center
		{
			0,
			0,
			0
		},

		//-- widget far
		{
			0,
			s_farFixedCenterNames,
			s_farFixedFarNames
		},
	},

	//-- proportional 3x3 matrix next
	{
		//-- widget near
		{
			s_nearProportionalNearNames,
			0,
			0
		},
		
		//-- widget center
		{
			s_centerProportionalNearNames,
			0,
			0
		},

		//-- widget far
		{
			0,
			0,
			s_farProportionalFarNames
		},
	}
};

//-----------------------------------------------------------------
//-----------------------------------------------------------------

//-----------------------------------------------------------------

Manager::Manager () :
m_sizeInfoMap     (new SizeInfoMap_t),
m_locationInfoMap (new LocationInfoMap_t)
{
	//-- first create sizing info
	{
		for (size_t i = 0; i <= SizeInfo::MAX_AUTO_TYPES; ++i)
		{
			SizeInfo * sinfo = new SizeInfo;
			
			sinfo->m_type = static_cast<SizeInfo::Type>(i);
			
			//-- insert full name as well as abbreviation
			m_sizeInfoMap->insert (SizeInfoMap_t::value_type (s_packSizeNames[i], sinfo));
			m_sizeInfoMap->insert (SizeInfoMap_t::value_type (s_packSizeAbbrevs[i], sinfo));
		}
	}

	//-- now create the more complicated location info

	for (size_t i = 0; i <= LocationInfo::MAX_AUTO_TYPES; ++i)
	{
		// widget region
		for (size_t j = 0; j <= LocationInfo::MAX_REGIONS; ++j)
		{
			// parent region
			for (size_t k = 0; k <= LocationInfo::MAX_REGIONS; ++k)
			{

				LocationInfo * linfo = new LocationInfo;
				linfo->m_type = static_cast<LocationInfo::Type>(i);
				linfo->m_widgetRegion = static_cast<LocationInfo::Region>(j);
				linfo->m_parentRegion = static_cast<LocationInfo::Region>(k);

				//-- add fullname to multimap
				std::string abbrev;
				abbrev.append (1, s_packLocationRegionAbbrevs [j][0]);
				abbrev.append (1, s_packLocationTypeAbbrevs   [i][0]);
				abbrev.append (1, s_packLocationRegionAbbrevs [k][0]);

				m_locationInfoMap->insert (LocationInfoMap_t::value_type (abbrev, linfo));

				//-- add abbrev

				std::string fullname;
				fullname.append (s_packLocationRegionNames [j]);
				fullname.append (s_packLocationTypeNames   [i]);
				fullname.append (s_packLocationRegionNames [k]);

				m_locationInfoMap->insert (LocationInfoMap_t::value_type (fullname, linfo));

				const char ** aliases = s_packLocationAliases [i][j][k];

				if (aliases)
				{
					for (size_t t = 0; aliases[t]; ++t)
					{
						m_locationInfoMap->insert (LocationInfoMap_t::value_type (aliases [t], linfo));
					}
				}
			}
		}
	}
}
//-----------------------------------------------------------------

Manager::~Manager ()
{
	typedef std::set<LocationInfo *> LocationInfoSet_t;
	typedef std::set<SizeInfo *>     SizeInfoSet_t;

	LocationInfoSet_t locationInfoSet;
	SizeInfoSet_t     sizeInfoSet;
	
	//-- build location set and delete
	{
		for (LocationInfoMap_t::iterator iter = m_locationInfoMap->begin (); iter != m_locationInfoMap->end (); ++iter)
		{
			locationInfoSet.insert ((*iter).second);
			(*iter).second = 0;
		}
	}
	{
		for (LocationInfoSet_t::iterator iter = locationInfoSet.begin (); iter != locationInfoSet.end (); ++iter)
		{
			delete (*iter);
		}
	}
	{
		//-- build size set and delete
		for (SizeInfoMap_t::iterator iter = m_sizeInfoMap->begin (); iter != m_sizeInfoMap->end (); ++iter)
		{
			sizeInfoSet.insert ((*iter).second);
			(*iter).second = 0;
		}
	}
	{
		for (SizeInfoSet_t::iterator iter = sizeInfoSet.begin (); iter != sizeInfoSet.end (); ++iter)
		{
			delete (*iter);
		}
	}

	delete m_sizeInfoMap;
	m_sizeInfoMap = 0;

	delete m_locationInfoMap;
	m_locationInfoMap = 0;
}

//-----------------------------------------------------------------

const LocationInfo *  Manager::getLocationInfo (const std::string & str) const
{
	LocationInfoMap_t::const_iterator find_iter = m_locationInfoMap->find (str);

	return (find_iter != m_locationInfoMap->end ()) ? (*find_iter).second : 0;
}
//-----------------------------------------------------------------

const SizeInfo *      Manager::getSizeInfo     (const std::string & str) const
{
	SizeInfoMap_t::const_iterator find_iter = m_sizeInfoMap->find (str);

	return (find_iter != m_sizeInfoMap->end ()) ? (*find_iter).second : 0;

}

//-----------------------------------------------------------------
//-----------------------------------------------------------------

namespace UIPacking
{
	
	//-----------------------------------------------------------------

	void SizeInfo::resetInfo (const long wSize, const long pSize)
	{
		switch (m_type)
		{
		case Fixed:
			break;
		case Absolute:
			//-- numerator stores the absolute size difference
			m_proportion.set (pSize - wSize, 1);
			break;
		case HalfAbsolute:
			//-- numerator stores the absolute size difference
			m_proportion.set ((pSize / 2L) - wSize, 1);
			break;
		case Proportional:

			if (pSize)
				m_proportion.set (wSize, pSize);
			else
				m_proportion.set (1, 2);

			break;
		case UserDefined:
			break;
		}
	}

	//-----------------------------------------------------------------

	long SizeInfo::performPacking (const int wNear, const int wFar, const int pFar) const
	{

		UI_UNREF (wFar);
		// the near value is the coordinate value
		
		int result = wFar - wNear;
		
		switch (m_type)
		{
			//-- maintain constant size
		case Fixed:
			//- nothing to do
			break;
			
			//-- maintain the same difference between parentsize and widgetsize
		case Absolute:
			result = pFar - m_proportion.numerator;
			break;
			
		case HalfAbsolute:
			result = (pFar / 2L) - m_proportion.numerator;
			break;

			//-- maintain the same proportional relationship between parentsize and widgetsize
		case Proportional:
			//-- keep proportional
			{
				result = 
					static_cast<long>(static_cast<float>(
					m_proportion.numerator) * 
					static_cast<float>(pFar) / 
					static_cast<float>(m_proportion.denominator));
			}
			break;
			
			//-- maintain a user defined relationship
		case UserDefined:
			result = static_cast<long> (pFar * m_userProportion );
			break;
		}
		
		return result;
	}


	//-----------------------------------------------------------------
	//-----------------------------------------------------------------
	//-----------------------------------------------------------------

	bool LocationInfo::rulesMatch (const LocationInfo & rhs) const
	{
		return m_type == rhs.m_type && m_widgetRegion == rhs.m_widgetRegion && m_parentRegion == rhs.m_parentRegion;
	}

	//-----------------------------------------------------------------
	
	void LocationInfo::findPackingPoints (const long wNear,const long wFar,const long pFar,long & wPoint,long & pPoint) const
	{

		//-- where is the widget target point
		switch (m_widgetRegion)
		{
		case Near:
			wPoint = wNear;
			break;
		case Center:
			wPoint = (wNear + wFar) / 2;
			break;
		case Far:
			wPoint = wFar;
			break;
		default:
			assert (false); //lint !e1924 // C-style cast MSVC bug
			break;
		}

		//-- where is the parent target point
		switch (m_parentRegion)
		{
		case Near:
			pPoint = 0;
			break;
		case Center:
			pPoint = pFar / 2;
			break;
		case Far:
			pPoint = pFar;
			break;
		default:
			assert (false); //lint !e1924 // C-style cast MSVC bug
			break;
		}

	}

	//-----------------------------------------------------------------

	long LocationInfo::performPacking (const int wNear, const int wFar, const int pFar) const
	{
		switch (m_type)
		{
		case Fixed:
			{
				long wPoint = 0;
				long pPoint = 0;
				findPackingPoints (wNear, wFar, pFar, wPoint, pPoint);
				return wNear + getFixedDistance () - (wPoint - pPoint);
			}
			
		case Proportional:
			{
				long wPoint = 0;
				long pPoint = 0;
				findPackingPoints (wNear, wFar, pFar, wPoint, pPoint);

				long desiredDistance = 
					static_cast<long> (
					static_cast<float> (m_proportion.numerator) * 
					static_cast<float> (pFar) / 
					static_cast<float> (m_proportion.denominator));

				return wNear + desiredDistance - (wPoint - pPoint);
			}

		case UserDefined:
			{
				const long wWidth = wFar - wNear;

				if (wWidth)
					return static_cast<long> (m_userProportion * pFar) * 2 / wWidth;
				else
					return 0;
			}
		}

		assert (false);
		return wNear;
	}

	//-----------------------------------------------------------------
	
	void LocationInfo::resetInfo (long wNear, long wFar, long pFar)
	{
		
		long wPoint = 0;
		long pPoint = 0;
		findPackingPoints (wNear, wFar, pFar, wPoint, pPoint);		

		if (m_type == Fixed)
		{
			m_proportion.numerator = wPoint - pPoint;
		}

		else if (m_type == Proportional)
		{
			if (pFar == 0)
				m_proportion.set (1, 1);
			
			m_proportion.set (wPoint - pPoint, pFar);
		}
	}	
	//-----------------------------------------------------------------
}

// ======================================================================
