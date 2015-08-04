// ============================================================================
// 
// MatchMakingResult.h
// Copyright Sony Online Entertainment, Inc.
//
// ============================================================================

#ifndef INCLUDED_MatchMakingResult_H
#define INCLUDED_MatchMakingResult_H

//-----------------------------------------------------------------------------
class MatchMakingResult
{
public:

	MatchMakingResult();

	void setPlayerName(Unicode::String const &name);
	void setSystemTime(time_t const time);
	void setPreferenceToProfileMatchPercent(float const percent); // Your preference to their profile
	void setProfileMatchPercent(float const percent);             // Your profile to their profile

	Unicode::String const &getPlayerName() const;
	time_t                 getSystemTime() const;
	float                  getPreferenceToProfileMatchPercent() const;
	float                  getProfileMatchPercent() const;

private:

	Unicode::String m_playerName;
	time_t          m_systemTime;
	float           m_preferenceToProfileMatchPercent;
	float           m_profileMatchPercentPercent;
};

// ============================================================================

#endif // INCLUDED_MatchMakingResult_H
