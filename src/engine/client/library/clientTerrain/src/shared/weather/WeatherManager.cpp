// ======================================================================
//
// WeatherManager.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
// ======================================================================

#include "clientTerrain/FirstClientTerrain.h"
#include "clientTerrain/WeatherManager.h"

#include "sharedMath/Vector.h"

#include <vector>

// ======================================================================
// WeatherManagerNamespace
// ======================================================================

namespace WeatherManagerNamespace
{
	float  ms_windScale;
	Vector ms_normalizedWindVelocity_w (1.f, 0.f, 0.f);

	typedef std::vector<WeatherManager::WeatherChangedFunction> WeatherChangedFunctionList;
	WeatherChangedFunctionList ms_weatherChangedFunctionList;
}

using namespace WeatherManagerNamespace;

// ======================================================================
// STATIC PUBLIC WeatherManager
// ======================================================================

void WeatherManager::addWeatherChangedFunction (WeatherChangedFunction weatherChangedFunction)
{
	NOT_NULL (weatherChangedFunction);
	if (weatherChangedFunction)
	{
		//-- verify weather changed function isn't already in list
		if (std::find (ms_weatherChangedFunctionList.end (), ms_weatherChangedFunctionList.begin (), weatherChangedFunction) != ms_weatherChangedFunctionList.end())
		{
			//-- add to list
			ms_weatherChangedFunctionList.push_back (weatherChangedFunction);

			//-- call weatherChangedFunction with weather data
			weatherChangedFunction (getScaledWindVelocity_w ());
		}
	}
}

// ----------------------------------------------------------------------

void WeatherManager::removeWeatherChangedFunction (WeatherChangedFunction weatherChangedFunction)
{
	NOT_NULL (weatherChangedFunction);
	if (weatherChangedFunction)
	{
		WeatherChangedFunctionList::iterator iter;
		while ((iter = std::find (ms_weatherChangedFunctionList.begin (), ms_weatherChangedFunctionList.end (), weatherChangedFunction)) != ms_weatherChangedFunctionList.end ())
			ms_weatherChangedFunctionList.erase (iter);
	}
}

// ----------------------------------------------------------------------

void WeatherManager::setWindScale (float const windScale)
{
	ms_windScale = windScale;

	weatherChanged ();
}

// ----------------------------------------------------------------------

void WeatherManager::setNormalizedWindVelocity_w (Vector const & normalizedWindVelocity_w)
{
	ms_normalizedWindVelocity_w = normalizedWindVelocity_w;
	ms_normalizedWindVelocity_w.normalize ();

	weatherChanged ();
}

// ----------------------------------------------------------------------

Vector const WeatherManager::getScaledWindVelocity_w ()
{
	return ms_normalizedWindVelocity_w * ms_windScale;
}

// ======================================================================
// STATIC PRIVATE WeatherManager
// ======================================================================

void WeatherManager::weatherChanged ()
{
	WeatherChangedFunctionList::iterator end = ms_weatherChangedFunctionList.end ();
	for (WeatherChangedFunctionList::iterator iter = ms_weatherChangedFunctionList.begin (); iter != end; ++iter)
	{
		WeatherChangedFunction weatherChangedFunction = *iter;
		NOT_NULL (weatherChangedFunction);
		if (weatherChangedFunction)
			weatherChangedFunction (getScaledWindVelocity_w ());
	}
}

// ======================================================================

