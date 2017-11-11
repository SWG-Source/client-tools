// ============================================================================
//
// LightningBolt.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_LightningBolt_H
#define INCLUDED_LightningBolt_H

#include "sharedCollision/BoxExtent.h"
#include "sharedMath/Vector.h"
#include "sharedMath/WaveForm.h"

class LightningAppearanceTemplate;

//-----------------------------------------------------------------------------
class LightningBolt
{
public:

	LightningBolt();

	void             setLightningAppearanceTemplate(LightningAppearanceTemplate const * const lightningAppearanceTemplate);
	void             setBuildTime(float const time);
	void             setLoopTime(float const loopTime);

	void             alter(float const deltaTime);

	Vector           getPosition_w(Vector const &startPosition_w, Vector const &endPosition_w, float const t, float const arcUp) const;
	Vector const &   getPositionOffset(unsigned int const index) const;
	float            getAge() const;
	float            getAgePercent() const;

	static unsigned int getPositionCount();

	enum
	{
		m_positionListSize = 100
	};

private:

	LightningAppearanceTemplate const *m_lightningAppearanceTemplate;
	float                    m_age;
	float                    m_buildTime;
	float                    m_loopTime;
	int                      m_loopCount;
	float                    m_agePercent;
	Vector                   m_positionOffsetList[m_positionListSize];
	WaveForm                 m_offsetX;
	WaveForm                 m_offsetY;
	WaveForm                 m_offsetZ;
	float                    m_arcUp;

	void calculateAgePercent();
	void loop();
	void buildGeometry();
	void insertControlPoints(WaveForm &waveform, int const count);
	void setupControlPoints();

private:

	LightningBolt(LightningBolt const &);
	LightningBolt &operator =(LightningBolt const &);
};

// ============================================================================

#endif // INCLUDED_LightningBolt_H
