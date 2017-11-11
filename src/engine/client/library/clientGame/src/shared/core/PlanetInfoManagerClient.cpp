//======================================================================
//
// PlanetInfoManagerClient.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/PlanetInfoManagerClient.h"

#include "clientGame/PlanetInfoClient.h"
#include "clientGame/TangibleObject.h"
#include <map>
#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedUtility/DataTable.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/RotationDynamics.h"

//======================================================================

namespace
{
	typedef stdmap<std::string, PlanetInfoClient>::fwd PlanetInfoMap;
	PlanetInfoMap s_planetInfoMap;

	bool s_installed = false;

	enum DtCols
	{
		DC_name,
		DC_objectTemplate,
		DC_pitchDegrees,
		DC_appearanceRotationPeriod,
		DC_appearanceRadius,
		DC_mapTexturePath
	};

	void install ()
	{
		if (s_installed)
			return;

		static const char * const filename = "datatables/creation/planet_info.iff";
		
		Iff iff;
		
		if (!iff.open (filename, true))
			WARNING (true, ("Data file %s not available.", filename));
		else
		{			
			DataTable dt;
			dt.load (iff);
			iff.close ();

			const int numRows = dt.getNumRows ();

			for (int i = 0; i < numRows; ++i)
			{
				PlanetInfoClient info;
				info.name                           = dt.getStringValue (DC_name,                           i);
				info.objectTemplate                 = dt.getStringValue (DC_objectTemplate,                 i);
				info.pitchRadians                   = dt.getFloatValue  (DC_pitchDegrees,                   i) * PI / 180.0f;
				info.appearanceRotationPeriod       = dt.getFloatValue  (DC_appearanceRotationPeriod,       i);
				info.appearanceRadius               = dt.getFloatValue  (DC_appearanceRadius,               i);
				info.mapTexturePath                 = dt.getStringValue (DC_mapTexturePath,                 i);

				s_planetInfoMap.insert (std::make_pair (info.name, info));
			}
		}

		s_installed = true;
	}
}

//----------------------------------------------------------------------

const PlanetInfoClient * PlanetInfoManagerClient::findInfoForPlanet           (const std::string & planet)
{
	if (!s_installed)
		install ();

	const PlanetInfoMap::const_iterator it = s_planetInfoMap.find (planet);
	if (it != s_planetInfoMap.end ())
		return &(*it).second;

	return 0;
}

//----------------------------------------------------------------------

const std::string        PlanetInfoManagerClient::findMapTexturePathForPlanet (const std::string & planet)
{
	const PlanetInfoClient * const info = findInfoForPlanet (planet);
	if (info)
		return info->mapTexturePath;

	return std::string ();
}

//----------------------------------------------------------------------

TangibleObject *   PlanetInfoManagerClient::createObjectForPlanet       (const std::string & planet, bool dummyOnFailure)
{
	TangibleObject * planetObject = 0;
	const PlanetInfoClient * const info = findInfoForPlanet (planet);
	if (info)
	{	
		planetObject = safe_cast<TangibleObject *>(ObjectTemplate::createObject (info->objectTemplate.c_str ()));
		if (planetObject)
		{			
			planetObject->pitch_o (info->pitchRadians);
			
			float rps = 0.0f;

			if (info->appearanceRotationPeriod)
				rps = RECIP (info->appearanceRotationPeriod);

			RotationDynamics* const dynamics = new RotationDynamics (planetObject, Vector (-rps, 0.0f, 0.0f));
			planetObject->setDynamics    (dynamics);
			planetObject->setScale       (Vector::xyz111 * info->appearanceRadius);
			dynamics->setState           (true);
		}
		else
			WARNING (true, ( "createObjectForPlanet Unable to load planet [%s] for [%s]", info->objectTemplate.c_str (), planet.c_str ()));
	}
	else
		WARNING (true, ( "createObjectForPlanet Unable to find planet info for [%s]", planet.c_str ()));


	if (!planetObject)
	{
		if (dummyOnFailure)
			planetObject = safe_cast<TangibleObject *>(ObjectTemplate::createObject ("object/tangible/ui/shared_ui_planet_corellia.iff"));
	}

	if (planetObject)
	{
		planetObject->setDebugName   (planet.c_str ());
		planetObject->endBaselines ();
	}

	return planetObject;
}

//======================================================================
