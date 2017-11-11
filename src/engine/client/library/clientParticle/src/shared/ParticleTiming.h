// ============================================================================
//
// ParticleTiming.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_ParticleTiming_H
#define INCLUDED_ParticleTiming_H

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"

class Iff;

//-----------------------------------------------------------------------------
class ParticleTiming
{
public:

	ParticleTiming();
	ParticleTiming(ParticleTiming const &rhs);
	ParticleTiming &operator =(ParticleTiming const &rhs);

	void write(Iff &iff) const;
	bool load(Iff &iff);

	// Set a default state

	void reset();

	// Set the individual min/max values

	void setStartDelay(float const min, float const max);
	void setLoopDelay(float const min, float const max);
	void setLoopCount(int const min, int const max);

	// Get the individual min/max values

	float getStartDelayMin() const;
	float getStartDelayMax() const;
	float getLoopDelayMin() const;
	float getLoopDelayMax() const;
	int   getLoopCountMin() const;
	int   getLoopCountMax() const;

	// Get a random value

	float getRandomStartDelay() const;
	float getRandomLoopDelay() const;
	int   getRandomLoopCount() const;

#ifdef _DEBUG
	void validate();
#else
	void validate() {}
#endif // _DEBUG

private:

	static Tag const m_tag;

	float m_startDelayMin;
	float m_startDelayMax;
	float m_loopDelayMin;
	float m_loopDelayMax;
	int   m_loopCountMin;
	int   m_loopCountMax;

	void load_0000(Iff &iff);
	void load_0001(Iff &iff);
};

// ============================================================================

#endif // INCLUDED_ParticleTiming_H
