// ============================================================================
//
// ParticleTiming.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "clientParticle/FirstClientParticle.h"
#include "clientParticle/ParticleTiming.h"

#include "sharedFile/Iff.h"
#include "sharedRandom/Random.h"

// ============================================================================
//
// ParticleTiming
//
// ============================================================================

Tag const ParticleTiming::m_tag = TAG(P,T,I,M);

//-----------------------------------------------------------------------------
ParticleTiming::ParticleTiming()
 : m_startDelayMin(0.0f)
 , m_startDelayMax(0.0f)
 , m_loopDelayMin(0.0f)
 , m_loopDelayMax(0.0f)
 , m_loopCountMin(-1)
 , m_loopCountMax(-1)
{
}

//-----------------------------------------------------------------------------
ParticleTiming::ParticleTiming(ParticleTiming const &rhs)
 : m_startDelayMin(rhs.m_startDelayMin)
 , m_startDelayMax(rhs.m_startDelayMax)
 , m_loopDelayMin(rhs.m_loopDelayMin)
 , m_loopDelayMax(rhs.m_loopDelayMax)
 , m_loopCountMin(rhs.m_loopCountMin)
 , m_loopCountMax(rhs.m_loopCountMax)
{
	validate();
}

//-----------------------------------------------------------------------------
ParticleTiming &ParticleTiming::operator =(ParticleTiming const &rhs)
{
	if (this != &rhs)
	{
		m_startDelayMin = rhs.m_startDelayMin;
		m_startDelayMax = rhs.m_startDelayMax;
		m_loopDelayMin = rhs.m_loopDelayMin;
		m_loopDelayMax = rhs.m_loopDelayMax;
		m_loopCountMin = rhs.m_loopCountMin;
		m_loopCountMax = rhs.m_loopCountMax;
	}

	validate();

	return *this;
}

//-----------------------------------------------------------------------------
void ParticleTiming::reset()
{
	m_startDelayMin = 0.0f;
	m_startDelayMax = 0.0f;
	m_loopDelayMin = 0.0f;
	m_loopDelayMax = 0.0f;
	m_loopCountMin = -1;
	m_loopCountMax = -1;
}

//-----------------------------------------------------------------------------
void ParticleTiming::setStartDelay(float const min, float const max)
{
	m_startDelayMin = min;
	m_startDelayMax = max;

	validate();
}

//-----------------------------------------------------------------------------
void ParticleTiming::setLoopDelay(float const min, float const max)
{
	m_loopDelayMin = min;
	m_loopDelayMax = max;

	validate();
}

//-----------------------------------------------------------------------------
void ParticleTiming::setLoopCount(int const min, int const max)
{
	m_loopCountMin = min;
	m_loopCountMax = max;

	validate();
}

//-----------------------------------------------------------------------------
float ParticleTiming::getStartDelayMin() const
{
	return m_startDelayMin;
}

//-----------------------------------------------------------------------------
float ParticleTiming::getStartDelayMax() const
{
	return m_startDelayMax;
}

//-----------------------------------------------------------------------------
float ParticleTiming::getLoopDelayMin() const
{
	return m_loopDelayMin;
}

//-----------------------------------------------------------------------------
float ParticleTiming::getLoopDelayMax() const
{
	return m_loopDelayMax;
}

//-----------------------------------------------------------------------------
int ParticleTiming::getLoopCountMin() const
{
	return m_loopCountMin;
}

//-----------------------------------------------------------------------------
int ParticleTiming::getLoopCountMax() const
{
	return m_loopCountMax;
}

//-----------------------------------------------------------------------------
float ParticleTiming::getRandomStartDelay() const
{
	return Random::randomReal(m_startDelayMin, m_startDelayMax);
}

//-----------------------------------------------------------------------------
float ParticleTiming::getRandomLoopDelay() const
{
	return Random::randomReal(m_loopDelayMin, m_loopDelayMax);
}

//-----------------------------------------------------------------------------
int ParticleTiming::getRandomLoopCount() const
{
	return Random::random(m_loopCountMin, m_loopCountMax);
}

//--------------------------------------------------------------------------
bool ParticleTiming::load(Iff &iff)
{
	bool result = true;

	if (iff.enterForm(m_tag, true))
	{
		switch (iff.getCurrentName())
		{
			case TAG_0000:
				{
					load_0000(iff);
					break;
				}
			case TAG_0001:
				{
					load_0001(iff);
					break;
				}
			default:
				{
					result = false;

					char currentTagName[256];
					ConvertTagToString(iff.getCurrentName(), currentTagName);
					DEBUG_FATAL(true, ("Unsupported data version: %s", currentTagName));
				}
		}

		iff.exitForm(m_tag);
	}
	else
	{
		result = false;

		char expectedTagName[256];
		ConvertTagToString(m_tag, expectedTagName);
		char currentTagName[256];
		ConvertTagToString(iff.getCurrentName(), currentTagName);
		DEBUG_FATAL(true, ("Unable to load particle description. Expecting tag(%s) but encountered(%s)", expectedTagName, currentTagName));
	}

	if (!result)
	{
		reset();
	}

	validate();

	return result;
}

//-----------------------------------------------------------------------------
void ParticleTiming::load_0000(Iff &iff)
{
	iff.enterChunk(TAG_0000);
	{
		m_startDelayMin = iff.read_float();
		m_startDelayMax = iff.read_float();
		m_loopDelayMin = iff.read_float();
		m_loopDelayMax = iff.read_float();
		m_loopCountMin = iff.read_int32();
		m_loopCountMax = iff.read_int32();
	}
	iff.exitChunk(TAG_0000);

	// Slam this because there was a bug in the data for this version

	if (m_loopCountMin <= -1)
	{
		m_loopCountMax = -1;
	}
}

//-----------------------------------------------------------------------------
void ParticleTiming::load_0001(Iff &iff)
{
	iff.enterChunk(TAG_0001);
	{
		m_startDelayMin = iff.read_float();
		m_startDelayMax = iff.read_float();
		m_loopDelayMin = iff.read_float();
		m_loopDelayMax = iff.read_float();
		m_loopCountMin = iff.read_int32();
		m_loopCountMax = iff.read_int32();
	}
	iff.exitChunk(TAG_0001);
}

//-----------------------------------------------------------------------------
void ParticleTiming::write(Iff &iff) const
{
	iff.insertForm(m_tag);
	{
		iff.insertChunk(TAG_0001);
		{
			iff.insertChunkData(m_startDelayMin);
			iff.insertChunkData(m_startDelayMax);
			iff.insertChunkData(m_loopDelayMin);
			iff.insertChunkData(m_loopDelayMax);
			iff.insertChunkData(m_loopCountMin);
			iff.insertChunkData(m_loopCountMax);
		}
		iff.exitChunk(TAG_0001);
	}
	iff.exitForm(m_tag);
}

#ifdef _DEBUG
//-----------------------------------------------------------------------------
void ParticleTiming::validate()
{
	DEBUG_FATAL((m_startDelayMin < 0.0f), ("m_startDelayMin(%f) < 0", m_startDelayMin));
	DEBUG_FATAL((m_startDelayMax < 0.0f), ("m_startDelayMin(%f) < 0", m_startDelayMax));
	DEBUG_FATAL((m_startDelayMin > m_startDelayMax), ("m_startDelayMin(%f) > m_startDelayMax(%f)", m_startDelayMin, m_startDelayMax));

	DEBUG_FATAL((m_loopDelayMin < 0.0f), ("m_loopDelayMin(%f) < 0", m_loopDelayMin));
	DEBUG_FATAL((m_loopDelayMax < 0.0f), ("m_loopDelayMin(%f) < 0", m_loopDelayMax));
	DEBUG_FATAL((m_loopDelayMin > m_loopDelayMax), ("m_loopDelayMin(%f) > m_loopDelayMax(%f)", m_loopDelayMin, m_loopDelayMax));

	DEBUG_FATAL((m_loopCountMin < -1), ("m_loopCountMin(%d) < -1", m_loopCountMin));
	DEBUG_FATAL((m_loopCountMax < -1), ("m_loopCountMax(%d) < -1", m_loopCountMax));

	DEBUG_FATAL(((m_loopCountMin == -1) || (m_loopCountMax == -1)) && (m_loopCountMin != m_loopCountMax), ("Infinite loop count mismatch."));
}
#endif // _DEBUG

// ============================================================================
