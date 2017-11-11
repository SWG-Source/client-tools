// ============================================================================
// 
// MatchMakingResult.cpp
// Copyright Sony Online Entertainment, Inc.
//
// ============================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/MatchMakingResult.h"

// ============================================================================
//
// MatchMakingResult
//
// ============================================================================

//-----------------------------------------------------------------------------
MatchMakingResult::MatchMakingResult()
 : m_playerName()
 , m_systemTime()
 , m_preferenceToProfileMatchPercent(0.0f)
 , m_profileMatchPercentPercent(0.0f)
{
}

//-----------------------------------------------------------------------------
void MatchMakingResult::setPlayerName(Unicode::String const &name)
{
	m_playerName = name;
}

//-----------------------------------------------------------------------------
void MatchMakingResult::setSystemTime(time_t const time)
{
	m_systemTime = time;
}

//-----------------------------------------------------------------------------
void MatchMakingResult::setPreferenceToProfileMatchPercent(float const percent)
{
	m_preferenceToProfileMatchPercent = percent;
}

//-----------------------------------------------------------------------------
void MatchMakingResult::setProfileMatchPercent(float const percent)
{
	m_profileMatchPercentPercent = percent;
}

//-----------------------------------------------------------------------------
Unicode::String const &MatchMakingResult::getPlayerName() const
{
	return m_playerName;
}

//-----------------------------------------------------------------------------
time_t MatchMakingResult::getSystemTime() const
{
	return m_systemTime;
}

//-----------------------------------------------------------------------------
float MatchMakingResult::getPreferenceToProfileMatchPercent() const
{
	return m_preferenceToProfileMatchPercent;
}

//-----------------------------------------------------------------------------
float MatchMakingResult::getProfileMatchPercent() const
{
	return m_profileMatchPercentPercent;
}

// ============================================================================
