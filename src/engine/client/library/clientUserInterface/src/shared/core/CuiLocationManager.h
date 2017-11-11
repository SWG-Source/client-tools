//===================================================================
//
// CuiLocationManager.h
// asommers
//
// copyright 2001, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_CuiLocationManager_H
#define INCLUDED_CuiLocationManager_H

//===================================================================

class Vector;

//===================================================================

class CuiLocationManager
{
public:

	explicit CuiLocationManager (const std::string& filename);
	~CuiLocationManager ();

	int                getNumberOfPlanets () const;
	const std::string& getPlanetName (int planetIndex) const;
	const std::string& getPlanetDisplayName (int planetIndex) const;

	int                getNumberOfLocations (int planetIndex) const;
	const std::string& getLocationName (int planetIndex, int locationIndex) const;
	const Vector&      getLocationPosition (int planetIndex, int locationIndex) const;
	const std::string& getLocationSnapshotName (int planetIndex, int locationIndex) const;
	const std::string& getLocationCameraPathName (int planetIndex, int locationIndex) const;

private:

	CuiLocationManager ();

	void load (const std::string& filename);

private:

	class Planet;
	typedef stdvector<Planet*>::fwd PlanetList;

private:

	PlanetList* m_planetList;
};

//===================================================================

#endif
