//======================================================================
//
// ClientDataFile_VehicleThrusterSoundRuntimeData.h
// copyright(c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ClientDataFile_VehicleThrusterSoundRuntimeData_H
#define INCLUDED_ClientDataFile_VehicleThrusterSoundRuntimeData_H

//======================================================================

#include "clientAudio/SoundId.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"
class Object;
class VehicleThrusterSoundData;

//----------------------------------------------------------------------

class VehicleThrusterSoundRuntimeData
{
public:

	typedef Watcher<Object> ObjectWatcher;

	VehicleThrusterSoundRuntimeData();
	
	enum SoundState
	{
		SS_idle,
		SS_accel,
		SS_decel,
		SS_run
	};

	SoundId & getSoundIdByState(SoundState s);
	SoundState getOppositeState(SoundState s) const;
	SoundState getIntermediateTransitionState(SoundState s) const;
	int & getTotalTimeHolder(SoundState s);
	
	void update(float elapsedTime, float desiredSpeed);
	void fadeSoundIn(SoundId & id, float fadeInTime, int offsetStartMs);
	
	void transitionToIntermediateState();
	void transitionToFinalState();
	void transitionToReverseState();

	void setMaximumSpeed(float maximumSpeed);
	void setObject(Object * object);

	void updateSoundData (const VehicleThrusterSoundData & vtsd, bool damage);

	Object const * getObject() const;

	void stopAllSounds();

private:

	void updateSoundId (SoundId & soundId, const std::string & soundTemplateName);

private:
	SoundId m_idle;
	SoundId m_run;
	SoundId m_accel;
	SoundId m_decel;
	
	float      m_fadeInVolume;
	float      m_fadeInRate;
	float      m_fadeInOffset;
	bool       m_fadeInLoaded;
	SoundId    m_fadeInSoundId;
	
	SoundState m_stateCurrent;
	SoundState m_stateTarget;
	
	int        m_accelTotalSoundTime;
	int        m_decelTotalSoundTime;

	float m_maximumSpeed;;
	ObjectWatcher m_object;
	std::string m_hardpointName;
};

//----------------------------------------------------------------------

inline SoundId & VehicleThrusterSoundRuntimeData::getSoundIdByState(SoundState s)
{
	if (s == SS_idle)
		return m_idle;
	else if (s == SS_accel)
		return m_accel;
	else if (s == SS_decel)
		return m_decel;
	else
		return m_run;
}

//----------------------------------------------------------------------

inline VehicleThrusterSoundRuntimeData::SoundState VehicleThrusterSoundRuntimeData::getOppositeState(SoundState s) const
{
	if (s == SS_idle)
		return SS_run;
	else if (s == SS_accel)
		return SS_decel;
	else if (s == SS_decel)
		return SS_accel;
	else
		return SS_idle;
}

//----------------------------------------------------------------------

inline VehicleThrusterSoundRuntimeData::SoundState VehicleThrusterSoundRuntimeData::getIntermediateTransitionState(SoundState s)  const
{
	if (s == SS_idle)
		return SS_accel;
	else if (s == SS_run)
		return SS_decel;
	
	return SS_run;
}

//----------------------------------------------------------------------

inline int & VehicleThrusterSoundRuntimeData::getTotalTimeHolder(SoundState s)
{
	if (s == SS_accel)
		return m_accelTotalSoundTime;
	
	return m_decelTotalSoundTime;
}

//======================================================================

#endif
