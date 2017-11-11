//===================================================================
//
// CuiLocationManager.cpp
// asommers
//
// copyright 2001, sony online entertainment
//
//===================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiLocationManager.h"

#include "sharedFoundation/PointerDeleter.h"
#include "sharedMath/Vector.h"

#include <cstdio>
#include <algorithm>
#include <string>
#include <vector>

//===================================================================

namespace
{
	char* changeUnderscoresToSpaces (char* const name)
	{
		for (char* current = name; *current; ++current)
			if (*current == '_')
				*current =  ' ';

		return name;
	}
}

//===================================================================

class CuiLocationManager::Planet
{
public:

	explicit Planet (const std::string& name, const std::string& displayName);

	void               addLocation (const std::string& locationName, const Vector& locationPosition);
	void               setLastLocationSnapshotName(const std::string& snapshotName);
	void               setLastLocationCameraPathName(const std::string& cameraPathName);

	const std::string& getName () const;
	const std::string& getDisplayName () const;
	int                getNumberOfLocations () const;
	const std::string& getLocationName (int locationIndex) const;
	const Vector&      getLocationPosition (int locationIndex) const;
	const std::string& getLocationSnapshotName (int locationIndex) const;
	const std::string& getLocationCameraPathName (int locationIndex) const;

private:

	std::string              m_name;
	std::string              m_displayName;
	std::vector<std::string> m_locationNameList;
	std::vector<Vector>      m_locationPositionList;
	std::vector<std::string> m_locationSnapshotList;
	std::vector<std::string> m_locationCameraPathList;
};

//-------------------------------------------------------------------

CuiLocationManager::Planet::Planet (const std::string& name, const std::string& displayName) :
	m_name (name),
	m_displayName (displayName),
	m_locationNameList (),
	m_locationPositionList (),
	m_locationSnapshotList (),
	m_locationCameraPathList ()
{
}

//-------------------------------------------------------------------

void CuiLocationManager::Planet::addLocation (const std::string& name, const Vector& position)
{
	m_locationNameList.push_back (name);
	m_locationPositionList.push_back (position);
	m_locationSnapshotList.push_back ("");
	m_locationCameraPathList.push_back ("");
}

//-------------------------------------------------------------------

void CuiLocationManager::Planet::setLastLocationSnapshotName(const std::string& snapshotName)
{
	DEBUG_FATAL(m_locationSnapshotList.empty(), ("Cannot set snapshot before planet"));
	m_locationSnapshotList.back() = snapshotName;
}

//-------------------------------------------------------------------

void CuiLocationManager::Planet::setLastLocationCameraPathName(const std::string& cameraPathName)
{
	DEBUG_FATAL(m_locationCameraPathList.empty(), ("Cannot set camera path before planet"));
	m_locationCameraPathList.back() = cameraPathName;
}

//-------------------------------------------------------------------

const std::string& CuiLocationManager::Planet::getName () const
{
	return m_name;
}

//-------------------------------------------------------------------

const std::string& CuiLocationManager::Planet::getDisplayName () const
{
	return m_displayName;
}
//-------------------------------------------------------------------

int CuiLocationManager::Planet::getNumberOfLocations () const
{
	return static_cast<int> (m_locationNameList.size ());
}

//-------------------------------------------------------------------

const std::string& CuiLocationManager::Planet::getLocationName (int locationIndex) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, locationIndex, getNumberOfLocations ());

	return m_locationNameList [locationIndex];
}

//-------------------------------------------------------------------

const Vector& CuiLocationManager::Planet::getLocationPosition (int locationIndex) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, locationIndex, static_cast<int> (m_locationPositionList.size ()));

	return m_locationPositionList [locationIndex];
}

//-------------------------------------------------------------------

const std::string& CuiLocationManager::Planet::getLocationSnapshotName (int locationIndex) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, locationIndex, getNumberOfLocations ());

	return m_locationSnapshotList [locationIndex];
}

//-------------------------------------------------------------------

const std::string& CuiLocationManager::Planet::getLocationCameraPathName (int locationIndex) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, locationIndex, getNumberOfLocations ());

	return m_locationCameraPathList [locationIndex];
}

//===================================================================

CuiLocationManager::CuiLocationManager (const std::string& filename) :
	m_planetList (new PlanetList)
{
	load (filename);
}

//-------------------------------------------------------------------

CuiLocationManager::~CuiLocationManager ()
{
	std::for_each (m_planetList->begin (), m_planetList->end (), PointerDeleter ());
	delete m_planetList;
	m_planetList = 0;
}

//-------------------------------------------------------------------

void CuiLocationManager::load (const std::string& filename)
{
	FILE* infile = fopen (filename.c_str (), "rt");
	if (infile)
	{
		Planet* planet = 0;
		bool    quit   = false;
		char    buffer [1000];

		while (!quit && fgets (buffer, 1000, infile))
		{
			std::string line (buffer);

			//-- trim whitespace
			std::string::size_type notwhite = line.find_first_not_of (" \t\n");
			line.erase (0, notwhite);

			notwhite = line.find_last_not_of (" \t\n");
			line.erase (notwhite + 1);

			//-- are we left with nothing?
			if (line.length () == 0)
				continue;

			char  type [1000];
			sscanf (line.c_str (), "%s", type);

			switch (type [0])
			{
			case '/':
				continue;

			case '.':
				{
					char indirectName [1000];
					sscanf (line.c_str (), "%s %s", type, indirectName);
					load (indirectName);
				}
				break;

			case 'p':
			case 'P':
				{
					char name [1000];
					char displayName [1000];
					sscanf (line.c_str (), "%s %s %s", type, name, displayName);
					changeUnderscoresToSpaces (displayName);

					const std::string stringName(name);
					const std::string stringDisplayName(displayName);
					planet = new Planet (stringName, stringDisplayName);
					m_planetList->push_back (planet);
				}
				break;

			case 'l':
			case 'L':
				{
					char   name [1000];
					Vector position;
					int const count = sscanf(line.c_str (), "%s %s %f %f %f", type, name, &position.x, &position.y, &position.z);
					WARNING(count != 4 && count != 5, ("Malformed location line '%s'", line.c_str()));
					if (count == 4) // only xz specified
					{
						position.z = position.y;
						position.y = 0.f;
					}
					changeUnderscoresToSpaces (name);

					planet->addLocation (name, position);
				}
				break;

			case 's':
			case 'S':
				{
					char   snapshotName [1000];
					sscanf (line.c_str (), "%s %s", type, snapshotName);
					planet->setLastLocationSnapshotName (snapshotName);
				}
				break;

			case 'c':
			case 'C':
				{
					char   cameraPathName [1000];
					sscanf (line.c_str (), "%s %s", type, cameraPathName);
					planet->setLastLocationCameraPathName (cameraPathName);
				}
				break;

			default:
				quit = true;
				break;
			}
		}

		fclose (infile);
	}
}

//-------------------------------------------------------------------

int CuiLocationManager::getNumberOfPlanets () const
{
	return static_cast<int> (m_planetList->size ());
}

//-------------------------------------------------------------------

const std::string& CuiLocationManager::getPlanetName (int planetIndex) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, planetIndex, getNumberOfPlanets ());

	return (*m_planetList) [planetIndex]->getName ();
}

//-------------------------------------------------------------------

const std::string& CuiLocationManager::getPlanetDisplayName (int planetIndex) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, planetIndex, getNumberOfPlanets ());

	return (*m_planetList) [planetIndex]->getDisplayName ();
}

//-------------------------------------------------------------------

int CuiLocationManager::getNumberOfLocations (int planetIndex) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, planetIndex, getNumberOfPlanets ());

	return (*m_planetList) [planetIndex]->getNumberOfLocations ();
}

//-------------------------------------------------------------------

const std::string& CuiLocationManager::getLocationName (int planetIndex, int locationIndex) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, planetIndex, getNumberOfPlanets ());

	return (*m_planetList) [planetIndex]->getLocationName (locationIndex);
}

//-------------------------------------------------------------------

const Vector& CuiLocationManager::getLocationPosition (int planetIndex, int locationIndex) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, planetIndex, getNumberOfPlanets ());

	return (*m_planetList) [planetIndex]->getLocationPosition (locationIndex);
}

//-------------------------------------------------------------------

const std::string& CuiLocationManager::getLocationSnapshotName (int planetIndex, int locationIndex) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, planetIndex, getNumberOfPlanets ());

	return (*m_planetList) [planetIndex]->getLocationSnapshotName (locationIndex);
}

//-------------------------------------------------------------------

const std::string& CuiLocationManager::getLocationCameraPathName (int planetIndex, int locationIndex) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, planetIndex, getNumberOfPlanets ());

	return (*m_planetList) [planetIndex]->getLocationCameraPathName (locationIndex);
}

//===================================================================
