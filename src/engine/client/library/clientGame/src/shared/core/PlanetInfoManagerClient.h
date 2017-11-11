//======================================================================
//
// PlanetInfoManagerClient.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_PlanetInfoManagerClient_H
#define INCLUDED_PlanetInfoManagerClient_H

class PlanetInfoClient;
class TangibleObject;

//======================================================================

class PlanetInfoManagerClient
{
public:

	static const PlanetInfoClient * findInfoForPlanet           (const std::string & planet);
	static const std::string        findMapTexturePathForPlanet (const std::string & planet);
	static TangibleObject *         createObjectForPlanet       (const std::string & planet, bool dummyOnFailure = false);
};

//======================================================================

#endif
