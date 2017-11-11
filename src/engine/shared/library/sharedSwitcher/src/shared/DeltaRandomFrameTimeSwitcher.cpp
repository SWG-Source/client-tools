// ======================================================================
//
// DeltaRandomFrameTimeSwitcher.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "sharedSwitcher/FirstSharedSwitcher.h"
#include "sharedSwitcher/DeltaRandomFrameTimeSwitcher.h"

#include "sharedRandom/Random.h"

#include <algorithm>

// ======================================================================

DeltaRandomFrameTimeSwitcher::DeltaRandomFrameTimeSwitcher(const float minimumFrameTime, const float maximumFrameTime, const int numberOfStates)
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

DeltaRandomFrameTimeSwitcher::~DeltaRandomFrameTimeSwitcher()
{
}

// ----------------------------------------------------------------------

bool DeltaRandomFrameTimeSwitcher::needsAlter() const
{
	return (m_numberOfStates > 1);
}

// ----------------------------------------------------------------------

void DeltaRandomFrameTimeSwitcher::alter(const float time)
{
    m_currentTime += time;

    if (m_currentTime >= m_frameTime)
    {
        m_currentState = Random::random(0, m_numberOfStates - 1);
        m_currentTime -= m_frameTime;

        if (m_minimumFrameTime == m_maximumFrameTime)
        {
            // don't need to calculate m_frameTime here.

            if (m_currentTime >= m_frameTime)
            {
                m_currentTime = fmodf(m_currentTime, m_frameTime);
            }
        }
        else
        {
            m_frameTime = Random::randomReal(m_minimumFrameTime, m_maximumFrameTime);

            if (m_currentTime >= m_frameTime)
            {
                m_currentTime = m_frameTime - Random::randomReal(m_minimumFrameTime, m_maximumFrameTime);
            }
        }
    }
}

// ----------------------------------------------------------------------

int DeltaRandomFrameTimeSwitcher::getState() const
{
	return m_currentState;
}

// ----------------------------------------------------------------------

int DeltaRandomFrameTimeSwitcher::getNumberOfStates() const
{
	return m_numberOfStates;
}

// ======================================================================
