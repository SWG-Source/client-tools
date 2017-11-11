// ======================================================================
//
// DeltaRandomFrameSwitcher.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedSwitcher/FirstSharedSwitcher.h"
#include "sharedSwitcher/DeltaRandomFrameSwitcher.h"

#include "sharedRandom/Random.h"

// ======================================================================

DeltaRandomFrameSwitcher::DeltaRandomFrameSwitcher(const int numberOfStates)
: Switcher(),
	m_numberOfStates(numberOfStates),
	m_currentState(0)
{
}

// ----------------------------------------------------------------------

DeltaRandomFrameSwitcher::~DeltaRandomFrameSwitcher()
{
}

// ----------------------------------------------------------------------

bool DeltaRandomFrameSwitcher::needsAlter() const
{
	return false;
}

// ----------------------------------------------------------------------

void DeltaRandomFrameSwitcher::alter(const float /*time*/)
{
}

// ----------------------------------------------------------------------

int DeltaRandomFrameSwitcher::getState() const
{
	m_currentState = Random::random(0, m_numberOfStates - 1);
	return m_currentState;
}

// ----------------------------------------------------------------------

int DeltaRandomFrameSwitcher::getNumberOfStates() const
{
	return m_numberOfStates;
}

// ======================================================================
