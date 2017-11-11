// ==================================================================
//
// OcclusionZoneSet.h
// copyright 2001 Sony Online Entertainment
// 
// ==================================================================

#ifndef OCCLUSION_ZONE_SET
#define OCCLUSION_ZONE_SET

// ==================================================================

class CrcLowerString;

namespace boost
{
	template <typename T>
	class shared_ptr;
}

// ======================================================================
/**
 * Class that manages sets of occlusion zones.
 *
 * This class is used internally by the CompositeMesh and MeshGenerator
 * classes within the character system.  This class has nothing to do
 * with global scene occlusion.  This class and name came about before
 * we had a global scene-level occlusion system.
 */

class OcclusionZoneSet
{
public:

	typedef stdvector<boost::shared_ptr<CrcLowerString> >::fwd  CrcLowerStringVector;
	typedef stdvector<int>::fwd                                 IntVector;

public:

	static void install();

	static void                  registerOcclusionZones(const CrcLowerStringVector &occlusionZoneNames, IntVector &occlusionZoneIds);
	static const CrcLowerString &getOcclusionZoneName(int occlusionZoneId);

public:

	OcclusionZoneSet();
	~OcclusionZoneSet();

	void  addZone(int zoneId);
	bool  hasZone(int zoneId) const;

	void  clear();
	void  insertSet(const OcclusionZoneSet &set);

	bool  allZonesPresent(const OcclusionZoneSet &testZones) const;
	bool  allZonesPresent(const IntVector &testZones) const;

private:

	static void remove();

private:

	typedef stdset<int>::fwd  IntSet;

private:

	IntSet *m_occlusionZones;

private:

	// disable these
	OcclusionZoneSet(const OcclusionZoneSet&);
	OcclusionZoneSet &operator =(const OcclusionZoneSet&);

};

// ==================================================================

#endif
