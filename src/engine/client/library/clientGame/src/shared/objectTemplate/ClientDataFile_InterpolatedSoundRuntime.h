//======================================================================
//
// ClientDataFile_InterpolatedSoundRuntime.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ClientDataFile_InterpolatedSoundRuntime_H
#define INCLUDED_ClientDataFile_InterpolatedSoundRuntime_H

//======================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"

class ClientObject;
class InterpolatedSound;
class SoundId;

//----------------------------------------------------------------------

class InterpolatedSoundRuntime
{
public:

	typedef Watcher<ClientObject> ClientObjectWatcher;

	InterpolatedSoundRuntime();
	~InterpolatedSoundRuntime();

	void update(bool enabled, float desiredInterpolationLevel, float elapsedTimeSecs);

	void setObject(ClientObject & object, InterpolatedSound const & interpolatedSound);
	void setObject(ClientObject * object);
	bool isValid() const;
	bool isActive() const;
	void setActive(bool active);

private:

	InterpolatedSoundRuntime(InterpolatedSoundRuntime const &);
	InterpolatedSoundRuntime & operator=(InterpolatedSoundRuntime const &);

	ClientObjectWatcher * m_clientObjectWatcher;
	InterpolatedSound * m_interpolatedSound;
	float m_currentInterpolationLevel;
	SoundId * m_soundId;
	bool m_active;
};

//======================================================================

#endif
