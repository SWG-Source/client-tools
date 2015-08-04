// ======================================================================
//
// DeltaTimeSwitcher.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedSwitcher/FirstSharedSwitcher.h"
#include "sharedSwitcher/DeltaTimeSwitcher.h"

// ======================================================================

DeltaTimeSwitcher::DeltaTimeSwitcher(const float frameTime, const int numberOfStates)
: Switcher(),
	m_frameTime(frameTime),
	m_currentTime(0),
	m_numberOfStates(numberOfStates),
	m_currentState(0)
{
}

// ----------------------------------------------------------------------

DeltaTimeSwitcher::~DeltaTimeSwitcher()
{
}

// ----------------------------------------------------------------------

bool DeltaTimeSwitcher::needsAlter() const
{
	return true;
}

// ----------------------------------------------------------------------

void DeltaTimeSwitcher::alter(const float time)
{
	m_currentTime += time;

	while (m_currentTime >= m_frameTime)
	{
		m_currentTime -= m_frameTime;
		if (++m_currentState >= m_numberOfStates)
			m_currentState = 0;
	}
}

// ----------------------------------------------------------------------

int DeltaTimeSwitcher::getState() const
{
	return m_currentState;
}

// ----------------------------------------------------------------------

int DeltaTimeSwitcher::getNumberOfStates() const
{
	return m_numberOfStates;
}

// ======================================================================
