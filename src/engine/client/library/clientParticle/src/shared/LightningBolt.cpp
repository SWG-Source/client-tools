// ============================================================================
//
// LightningBolt.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "clientParticle/FirstClientParticle.h"
#include "clientParticle/LightningBolt.h"

#include "sharedRandom/Random.h"
#include "sharedMath/CatmullRomSpline.h"

// ============================================================================
//
// LightningBolt
//
// ============================================================================

//--------------------------------------------------------------------------
LightningBolt::LightningBolt()
 : m_lightningAppearanceTemplate(NULL)
 , m_age(0.0f)
 , m_buildTime(0.5f)
 , m_loopTime(1.0f)
 , m_loopCount(0)
 , m_agePercent(0.0f)
 , m_positionOffsetList()
 , m_offsetX()
 , m_offsetY()
 , m_offsetZ()
{
	setupControlPoints();
}

//--------------------------------------------------------------------------
void LightningBolt::setupControlPoints()
{
	int const controlPoints = 20;
	insertControlPoints(m_offsetX, controlPoints);
	insertControlPoints(m_offsetY, controlPoints);
	insertControlPoints(m_offsetZ, controlPoints);

	// Store the offset positions

	WaveFormControlPointIter iterOffsetX;
	WaveFormControlPointIter iterOffsetY;
	WaveFormControlPointIter iterOffsetZ;

	iterOffsetX.reset(m_offsetX.getIteratorBegin());
	iterOffsetY.reset(m_offsetY.getIteratorBegin());
	iterOffsetZ.reset(m_offsetZ.getIteratorBegin());

	for (unsigned int index = 0; index < m_positionListSize; ++index)
	{
		float const t = static_cast<float>(index) / static_cast<float>(m_positionListSize - 1);

		float const offsetX = m_offsetX.getValue(iterOffsetX, t);
		float const offsetY = m_offsetY.getValue(iterOffsetY, t);
		float const offsetZ = m_offsetZ.getValue(iterOffsetZ, t);

		m_positionOffsetList[index] = Vector(offsetX, offsetY, offsetZ);
	}
}

//--------------------------------------------------------------------------
void LightningBolt::insertControlPoints(WaveForm &waveform, int const count)
{
	float const min = -1.0f;
	float const max = +1.0f;

	waveform.clear();
	waveform.insert(WaveFormControlPoint(0.0f, Random::randomReal(min, max)));
	waveform.insert(WaveFormControlPoint(1.0f, Random::randomReal(min, max)));

	while (waveform.getControlPointCount() < count)
	{
		float const percent = Random::randomReal();

		waveform.insert(WaveFormControlPoint(percent, Random::randomReal(min, max)));
	}
}

//--------------------------------------------------------------------------
void LightningBolt::setBuildTime(float const time)
{
	m_buildTime = time;
}

//--------------------------------------------------------------------------
void LightningBolt::setLoopTime(float const loopTime)
{
	m_loopTime = loopTime;
}

//--------------------------------------------------------------------------
unsigned int LightningBolt::getPositionCount()
{
	return m_positionListSize;
}

//--------------------------------------------------------------------------
Vector LightningBolt::getPosition_w(Vector const &startPosition_w, Vector const &endPosition_w, float const t, float const arcUp) const
{
	DEBUG_WARNING((t < 0.0f || t > 1.0f), ("Invalid value for t: %f (valid is 0.0 >= t <= 1.0)", t));

	float const arcModifier = 4.0f * (endPosition_w - startPosition_w).magnitude();
	Vector c1(startPosition_w.x, startPosition_w.y - arcUp * arcModifier, startPosition_w.z);
	Vector c2(startPosition_w);
	Vector c3(endPosition_w);
	Vector c4(endPosition_w.x, endPosition_w.y - arcUp * arcModifier, endPosition_w.z);
	Vector result;

	CatmullRomSpline::getCatmullRomSplinePoint3d(c1, c2, c3, c4, clamp(0.0f, t, 1.0f), result);

	return result;
}

//--------------------------------------------------------------------------
Vector const &LightningBolt::getPositionOffset(unsigned int const index) const
{
	unsigned int const min = 0;
	unsigned int const max = getPositionCount() - 1;

	return m_positionOffsetList[clamp(min, index, max)];
}

//--------------------------------------------------------------------------
void LightningBolt::calculateAgePercent()
{
	if (m_loopTime > 0.0f)
	{
		m_agePercent = clamp(0.0f, m_age / m_loopTime, 1.0f);
	}
	else
	{
		m_agePercent = 0.0f;
	}
}

//--------------------------------------------------------------------------
float LightningBolt::getAge() const
{
	return m_age;
}

//--------------------------------------------------------------------------
float LightningBolt::getAgePercent() const
{
	return m_agePercent;
}

//--------------------------------------------------------------------------
void LightningBolt::alter(float const deltaTime)
{
	if (m_age >= m_loopTime)
	{
		m_age -= m_loopTime;
		++m_loopCount;
	}

	m_age = clamp(0.0f, m_age + deltaTime, m_loopTime);

	calculateAgePercent();
}

//--------------------------------------------------------------------------
void LightningBolt::setLightningAppearanceTemplate(LightningAppearanceTemplate const * lightningAppearanceTemplate)
{
	NOT_NULL(lightningAppearanceTemplate);
	m_lightningAppearanceTemplate = lightningAppearanceTemplate;

	m_age = 0.0f;
}

// ============================================================================
