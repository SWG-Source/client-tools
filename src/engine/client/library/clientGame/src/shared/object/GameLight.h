//
// GameLight.h
// asommers 7-7-99
//
// copyright 1999, bootprint entertainment
//

#ifndef GAMELIGHT_H
#define GAMELIGHT_H

//-------------------------------------------------------------------

#include "clientGraphics/Light.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Timer.h"

//-------------------------------------------------------------------

class GameLight : public Light
{
private:

	VectorArgb minColor;
	VectorArgb maxColor;

	real       minRange;
	real       maxRange;

	real       minTime;
	real       maxTime;

	Timer      timer;

	bool       m_flicker;
	bool       m_dayNightAware;
	bool       m_onOffAware;

private:

	GameLight (const GameLight&);
	GameLight& operator= (const GameLight&);

public:

	GameLight (void);
	virtual ~GameLight (void);

	virtual float alter (float time);

	void setFlicker (const bool flicker);
	void setColor (const VectorArgb& newMinColor, const VectorArgb& newMaxColor);  //lint !e1511 //-- hides non-virtual member
	void setRange (real newMinRange, real newMaxRange);                            //lint !e1511 //-- hides non-virtual member
	void setTime  (real newMinTime,  real newMaxTime);
	void setDayNightAware (bool dayNightAware);
	void setOnOffAware (bool onOffAware);

	//-- may create either a GameLight or a Light
	static Light* createLight (Iff* iff);

	static void setNight (bool night);
};

//-------------------------------------------------------------------

inline void GameLight::setRange (real newMinRange, real newMaxRange)
{
	minRange = newMinRange;
	maxRange = newMaxRange;
}

inline void GameLight::setColor (const VectorArgb& newMinColor, const VectorArgb& newMaxColor)
{
	minColor = newMinColor;
	maxColor = newMaxColor;
}

inline void GameLight::setTime  (real newMinTime,  real newMaxTime)
{
	minTime = newMinTime;
	maxTime = newMaxTime;
}

inline void GameLight::setFlicker (const bool flicker)
{
	m_flicker = flicker;
}

inline void GameLight::setDayNightAware (const bool dayNightAware)
{
	m_dayNightAware = dayNightAware;
}

inline void GameLight::setOnOffAware (const bool onOffAware)
{
	m_onOffAware = onOffAware;
}

//-------------------------------------------------------------------

#endif
