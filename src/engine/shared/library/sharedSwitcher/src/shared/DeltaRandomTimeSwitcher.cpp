// ======================================================================
//
// DeltaRandomTimeSwitcher.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedSwitcher/FirstSharedSwitcher.h"
#include "sharedSwitcher/DeltaRandomTimeSwitcher.h"

#include "sharedRandom/Random.h"

#include <algorithm>

// ======================================================================

DeltaRandomTimeSwitcher::DeltaRandomTimeSwitcher(const float minimumFrameTime, const float maximumFrameTime, const int numberOfStates)
: Switcher(),
	m_minimumFrameTime(std::min(minimumFrameTime, maximumFrameTime)),
	m_maximumFrameTime(std::max(minimumFrameTime, maximumFrameTime)),
	m_frameTime(Random::randomReal(m_minimumFrameTime, m_maximumFrameTime)),
	m_currentTime(0.f),
	m_numberOfStates(numberOfStates),
	m_currentState(0)
{
}

// ----------------------------------------------------------------------

DeltaRandomTimeSwitcher::~DeltaRandomTimeSwitcher()
{
}

// ----------------------------------------------------------------------

bool DeltaRandomTimeSwitcher::needsAlter() const
{
	return true;
}

// ----------------------------------------------------------------------

void DeltaRandomTimeSwitcher::alter(const float time)
{
	m_currentTime += time;

	while (m_currentTime >= m_frameTime)
	{
		m_currentTime -= m_frameTime;
		m_frameTime = Random::randomReal(m_minimumFrameTime, m_maximumFrameTime);
		if (++m_currentState >= m_numberOfStates)
			m_currentState = 0;
	}
}

// ----------------------------------------------------------------------

int DeltaRandomTimeSwitcher::getState() const
{
	return m_currentState;
}

// ----------------------------------------------------------------------

int DeltaRandomTimeSwitcher::getNumberOfStates() const
{
	return m_numberOfStates;
}

// ======================================================================
