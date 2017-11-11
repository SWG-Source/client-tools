//======================================================================
//
// ClientDataFile_InterpolatedSound.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ClientDataFile_InterpolatedSound_H
#define INCLUDED_ClientDataFile_InterpolatedSound_H

class Iff;

//======================================================================

class InterpolatedSound
{

public:
	
	void load (Iff & iff);
	InterpolatedSound ();
	~InterpolatedSound ();
	
public:
	
	std::string m_hardpointName;
	std::string m_soundTemplateName;
	float       m_volumeMin;
	float       m_volumeMax;
	float       m_pitchDeltaMin;
	float       m_pitchDeltaMax;
	float       m_interpolationRateUp;
	float       m_interpolationRateDown;
};

//======================================================================

#endif
