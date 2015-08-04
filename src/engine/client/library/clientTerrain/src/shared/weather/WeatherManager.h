// ======================================================================
//
// WeatherManager.h
// asommers
//
// copyright 2002, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_WeatherManager_H
#define INCLUDED_WeatherManager_H

// ======================================================================

class Vector;

// ======================================================================

class WeatherManager
{
public:

	typedef void (*WeatherChangedFunction) (Vector const & velocity_w);

public:

	//-- used to register a callback that will be called whenever the input routines are called
	static void         addWeatherChangedFunction (WeatherChangedFunction weatherChangedFunction);
	static void         removeWeatherChangedFunction (WeatherChangedFunction weatherChangedFunction);

	//-- input routines (set by server and/or terrain system)
	static void         setWindScale (float windScale);
	static void         setNormalizedWindVelocity_w (Vector const & normalizedWindVelocity_w);

	//-- this is what should be used when asking for the final wind velocity
	static Vector const getScaledWindVelocity_w ();

private:

	static void weatherChanged ();

private:

	WeatherManager ();
	WeatherManager (WeatherManager const &);
	WeatherManager & operator= (WeatherManager const &);
	~WeatherManager ();
};

// ======================================================================

#endif

