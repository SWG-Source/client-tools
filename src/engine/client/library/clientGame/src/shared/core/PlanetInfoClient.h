//======================================================================
//
// PlanetInfoClient.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_PlanetInfoClient_H
#define INCLUDED_PlanetInfoClient_H

//======================================================================

class PlanetInfoClient
{
public:
	std::string name;
	std::string objectTemplate;
	float       pitchRadians;
	float       appearanceRotationPeriod;
	float       appearanceRadius;
	std::string mapTexturePath;

	PlanetInfoClient ();
};

//======================================================================

#endif
