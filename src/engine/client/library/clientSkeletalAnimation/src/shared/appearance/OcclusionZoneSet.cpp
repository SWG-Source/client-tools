// ==================================================================
//
// OcclusionZoneSet.cpp
// copyright 2001 Sony Online Entertainment
// 
// ==================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/OcclusionZoneSet.h"

#include "boost/smart_ptr.hpp"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"

#include <set>
#include <vector>

// ======================================================================
// lint supression
// ======================================================================

//error 754: local struction member not referenced
//lint -esym(754, OcclusionZone::OcclusionZone)
//lint -esym(754, OcclusionZone::operator=)

//lint -e1734 // had difficulty compiling template function

// ======================================================================

namespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct OcclusionZone
	{
	public:

		class LessNameComparator
		{
		public:
		
			bool operator()(const boost::shared_ptr<OcclusionZone> &lhs, const boost::shared_ptr<OcclusionZone> &rhs) const;
			bool operator()(const boost::shared_ptr<OcclusionZone> &lhs, const boost::shared_ptr<CrcLowerString> &rhs) const;
			bool operator()(const boost::shared_ptr<CrcLowerString> &lhs, const boost::shared_ptr<OcclusionZone> &rhs) const;

		};

	public:

		OcclusionZone(const boost::shared_ptr<CrcLowerString> &name, int id);

	public:

		boost::shared_ptr<CrcLowerString> m_name;
		int                               m_id;

	private:
		// disabled
		OcclusionZone();
		OcclusionZone(const OcclusionZone&);
		OcclusionZone &operator =(const OcclusionZone&);
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef std::vector<boost::shared_ptr<OcclusionZone> >                     OcclusionZoneVector;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool                  ms_installed;

	OcclusionZoneVector  *ms_occlusionZonesByName;
	OcclusionZoneVector  *ms_occlusionZonesById;
}

// ======================================================================
// class OcclusionZone::LessNameComparator
// ======================================================================

inline bool OcclusionZone::LessNameComparator::operator()(const boost::shared_ptr<OcclusionZone> &lhs, const boost::shared_ptr<OcclusionZone> &rhs) const
{
	return *(lhs->m_name) < *(rhs->m_name);
}

// ----------------------------------------------------------------------

inline bool OcclusionZone::LessNameComparator::operator()(const boost::shared_ptr<OcclusionZone> &lhs, const boost::shared_ptr<CrcLowerString> &rhs) const
{
	return *(lhs->m_name) < *rhs;
}

// ----------------------------------------------------------------------

inline bool OcclusionZone::LessNameComparator::operator()(const boost::shared_ptr<CrcLowerString> &lhs, const boost::shared_ptr<OcclusionZone> &rhs) const
{
	return *lhs < *(rhs->m_name);
}

// ======================================================================
// OcclusionZone
// ======================================================================

OcclusionZone::OcclusionZone(const boost::shared_ptr<CrcLowerString> &name, int id)
:	m_name(name),
	m_id(id)
{
}

// ======================================================================
// class OcclusionZoneSet
// ======================================================================

void OcclusionZoneSet::install()
{
	DEBUG_FATAL(ms_installed, ("OcclusionZoneSet already installed"));

	ExitChain::add(remove, "OcclusionZoneSet");

	ms_occlusionZonesByName = new OcclusionZoneVector();
	ms_occlusionZonesById   = new OcclusionZoneVector();

	ms_installed = true;
}

// ----------------------------------------------------------------------

void OcclusionZoneSet::remove()
{
	DEBUG_FATAL(!ms_installed, ("OcclusionZoneSet not installed"));

	delete ms_occlusionZonesByName;
	ms_occlusionZonesByName = NULL;

	delete ms_occlusionZonesById;
	ms_occlusionZonesById = NULL;

	ms_installed = false;
}

// ----------------------------------------------------------------------

void OcclusionZoneSet::registerOcclusionZones(const CrcLowerStringVector &occlusionZoneNames, IntVector &occlusionZoneIds)
{
	DEBUG_FATAL(!ms_installed, ("OcclusionZoneSet not installed"));
	NOT_NULL(ms_occlusionZonesByName);
	NOT_NULL(ms_occlusionZonesById);

	const size_t newOcclusionZoneCount = occlusionZoneNames.size();

	occlusionZoneIds.clear();
	occlusionZoneIds.reserve(newOcclusionZoneCount);

	OcclusionZoneVector  zonesToAdd;
	zonesToAdd.reserve(newOcclusionZoneCount);

	const OcclusionZoneVector::const_iterator nameIt    = ms_occlusionZonesByName->begin();
	const OcclusionZoneVector::const_iterator nameItEnd = ms_occlusionZonesByName->end();

	//-- check each occlusion zone name to see if it needs to be added
	const CrcLowerStringVector::const_iterator testItEnd = occlusionZoneNames.end();
	for (CrcLowerStringVector::const_iterator testIt = occlusionZoneNames.begin(); testIt != testItEnd; ++testIt)
	{
		//-- search for test name in registered names
		std::pair<OcclusionZoneVector::const_iterator, OcclusionZoneVector::const_iterator> findResult = std::equal_range(nameIt, nameItEnd, *testIt, OcclusionZone::LessNameComparator()); //lint !e64 // type mismatch // huh?
		if (findResult.first == findResult.second)
		{
			// this occlusion zone name is new, add it

			//-- generate the new zone id
			const int newZoneId = static_cast<int>(ms_occlusionZonesById->size() + zonesToAdd.size());
			occlusionZoneIds.push_back(newZoneId);

			//-- create the zone
			boost::shared_ptr<OcclusionZone> newOcclusionZone(new OcclusionZone(*testIt, newZoneId));

			//-- add to list we'll append momentarily
			zonesToAdd.push_back(newOcclusionZone);

		}
		else
		{
			//-- this occlusion zone name already exists
			occlusionZoneIds.push_back((*findResult.first)->m_id);
		}
	}

	//-- add the zones to both sorted lists
	ms_occlusionZonesByName->insert(ms_occlusionZonesByName->end(), zonesToAdd.begin(), zonesToAdd.end());
	ms_occlusionZonesById->insert(ms_occlusionZonesById->end(), zonesToAdd.begin(), zonesToAdd.end());

	//-- resort the named list
	std::sort(ms_occlusionZonesByName->begin(), ms_occlusionZonesByName->end(), OcclusionZone::LessNameComparator());
}

// ----------------------------------------------------------------------

const CrcLowerString &OcclusionZoneSet::getOcclusionZoneName(int occlusionZoneId)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, occlusionZoneId, static_cast<int>(ms_occlusionZonesById->size()));
	
	return *((*ms_occlusionZonesById)[static_cast<size_t>(occlusionZoneId)]->m_name.get());
}

// ==================================================================

OcclusionZoneSet::OcclusionZoneSet(void)
:	m_occlusionZones(new IntSet())
{
}

// ------------------------------------------------------------------

OcclusionZoneSet::~OcclusionZoneSet(void)
{
	delete m_occlusionZones;
}

// ----------------------------------------------------------------------

void OcclusionZoneSet::addZone(int zoneId)
{
	IGNORE_RETURN(m_occlusionZones->insert(zoneId));
}

// ----------------------------------------------------------------------

bool OcclusionZoneSet::hasZone(int zoneId) const
{
	IntSet::const_iterator it = m_occlusionZones->find(zoneId);
	return (it != m_occlusionZones->end());
}

// ----------------------------------------------------------------------

void OcclusionZoneSet::clear()
{
	m_occlusionZones->clear();
}

// ----------------------------------------------------------------------

void OcclusionZoneSet::insertSet(const OcclusionZoneSet &set)
{
	m_occlusionZones->insert(set.m_occlusionZones->begin(), set.m_occlusionZones->end());
}

// ----------------------------------------------------------------------
/**
 * Determine if all zones in the given OcclusionZoneSet are present in
 * this object.
 *
 * @arg testZones  the zones we want to check for presence in *this.
 *
 * @return         true if every zone in testZones is in *this; false otherwise.
 */

bool OcclusionZoneSet::allZonesPresent(const OcclusionZoneSet &testZones) const
{
	const IntSet::const_iterator itEnd = testZones.m_occlusionZones->end();
	for (IntSet::const_iterator it = testZones.m_occlusionZones->begin(); it != itEnd; ++it)
	{
		//-- check if this test zone is within this object's occlusion zone set
		IntSet::const_iterator findIt = m_occlusionZones->find(*it);
		if (findIt == m_occlusionZones->end())
		{
			//-- at least one zone is not present, return false
			return false;
		}
	}

	//-- no zones were missing, so all are present
	return true;
}

// ----------------------------------------------------------------------
/**
 * Determine if all zones in the given zone id vector are present in
 * this object.
 *
 * @arg testZones  the zones we want to check for presence in *this.
 *
 * @return         true if every zone in testZones is in *this; false otherwise.
 */

bool OcclusionZoneSet::allZonesPresent(const IntVector &testZones) const
{
	const IntVector::const_iterator itEnd = testZones.end();
	for (IntVector::const_iterator it = testZones.begin(); it != itEnd; ++it)
	{
		//-- check if this test zone is within this object's occlusion zone set
		IntSet::const_iterator findIt = m_occlusionZones->find(*it);
		if (findIt == m_occlusionZones->end())
		{
			//-- at least one zone is not present, return false
			return false;
		}
	}

	//-- no zones were missing, so all are present
	return true;
}

// ======================================================================
