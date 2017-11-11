// ======================================================================
//
// DeltaFrameSwitcher.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedSwitcher/FirstSharedSwitcher.h"
#include "sharedSwitcher/DeltaFrameSwitcher.h"

// ======================================================================

DeltaFrameSwitcher::DeltaFrameSwitcher(const int numberOfStates)
: Switcher(),
	m_numberOfStates(numberOfStates),
	m_currentState(0)
{
}

// ----------------------------------------------------------------------

DeltaFrameSwitcher::~DeltaFrameSwitcher()
{
}

// ----------------------------------------------------------------------

bool DeltaFrameSwitcher::needsAlter() const
{
	return false;
}

// ----------------------------------------------------------------------

void DeltaFrameSwitcher::alter(const float /*time*/)
{
}

// ----------------------------------------------------------------------

int DeltaFrameSwitcher::getState() const
{
	const int result = m_currentState;
	if (++m_currentState == m_numberOfStates)
		m_currentState = 0;
	return result;
}

// ----------------------------------------------------------------------

int DeltaFrameSwitcher::getNumberOfStates() const
{
	return m_numberOfStates;
}

// ======================================================================
