// ======================================================================
//
// DeltaPingPongSwitcher.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "sharedSwitcher/FirstSharedSwitcher.h"
#include "sharedSwitcher/DeltaPingPongSwitcher.h"

#include "sharedRandom/Random.h"

#include <algorithm>

// ======================================================================

DeltaPingPongSwitcher::DeltaPingPongSwitcher(const float minimumFrameTime, const float maximumFrameTime, const int numberOfStates)
: Switcher(),
	m_minimumFrameTime(std::min(minimumFrameTime, maximumFrameTime)),
	m_maximumFrameTime(std::max(minimumFrameTime, maximumFrameTime)),
	m_frameTime(Random::randomReal(m_minimumFrameTime, m_maximumFrameTime)),
	m_currentTime(0.f),
	m_numberOfStates(numberOfStates),
	m_currentState(0),
    m_stateDelta(1)
{
}

// ----------------------------------------------------------------------

DeltaPingPongSwitcher::~DeltaPingPongSwitcher()
{
}

// ----------------------------------------------------------------------

bool DeltaPingPongSwitcher::needsAlter() const
{
	return (m_numberOfStates > 1);
}

// ----------------------------------------------------------------------

void DeltaPingPongSwitcher::alter(const float time)
{
	m_currentTime += time;

	while (m_currentTime >= m_frameTime)
	{
		m_currentTime -= m_frameTime;
		m_frameTime = Random::randomReal(m_minimumFrameTime, m_maximumFrameTime);

		if (m_stateDelta == 1)
		{
				if (m_currentState == m_numberOfStates - 1)
				 m_stateDelta = -1;
		}
		else
		{
            DEBUG_FATAL(m_stateDelta != -1, ("invalid state delta %d should be 1 or -1", m_stateDelta));
            if (m_currentState == 0)
                m_stateDelta = 1;
	    }

        m_currentState += m_stateDelta;
	}
}

// ----------------------------------------------------------------------

int DeltaPingPongSwitcher::getState() const
{
	return m_currentState;
}

// ----------------------------------------------------------------------

int DeltaPingPongSwitcher::getNumberOfStates() const
{
	return m_numberOfStates;
}

// ======================================================================
