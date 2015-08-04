//===================================================================
//
// DebugPortalCamera.h
// asommers 6-01-2000
//
// copyright 2000, verant interactive, inc.
//
//===================================================================

#ifndef INCLUDED_DebugPortalCamera_H
#define INCLUDED_DebugPortalCamera_H

//===================================================================

#include "clientObject/GameCamera.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"

class DeadReckoningModel;
class Object;
class MessageQueue;
class PlayerCreatureController;

//===================================================================

class DebugPortalCamera : public GameCamera
{
public:

	DebugPortalCamera ();
	virtual       ~DebugPortalCamera ();

	void           setMessageQueue (const MessageQueue* queue);
	void           setTarget (const Object* newTarget);
	
	virtual void   setActive (bool newActive);
	virtual float  alter (float elapsedTime);

	bool           loadPath (const char* filename);
	bool           savePath (const char* filename);
	void           clearPath ();
	float          getPathTime () const;
	void           setPathTime (float pathTime);
	float          getMinimumFrameRate () const;
	float          getMaximumFrameRate () const;
	float          getAverageFrameRate () const;

private:

	enum Keys
	{
		K_up,
		K_down,
		K_left,
		K_right,
		K_button,

		K_COUNT
	};

	enum Mode
	{
		M_normal,
		M_record,
		M_playback
	};

private:

	typedef stdvector<float>::fwd     FloatList;
	typedef stdvector<Transform>::fwd TransformList;

	const MessageQueue*        m_queue;

	bool                       m_keys [K_COUNT];
	int                        m_vtuneCounter;

	ConstWatcher<Object>       m_target;

	//-- recording/playback
	Mode                       m_mode;
	TransformList*             m_transformList;

	uint                       m_pathLeg;
	float                      m_pathTime;
	Timer                      m_pathLegTimer;

	float                      m_minimumFrameRate;
	float                      m_maximumFrameRate;
	FloatList*                 m_frameRateList;
	FloatList*                 m_frameTimeList;
	DeadReckoningModel*        m_deadReckoningModel;
	PlayerCreatureController * m_playerCreatureController;

private:

	DebugPortalCamera (const DebugPortalCamera&);
	DebugPortalCamera& operator= (const DebugPortalCamera&);

protected:

	virtual void drawScene () const;
};

//===================================================================

inline void DebugPortalCamera::setMessageQueue (const MessageQueue* queue)
{
	m_queue = queue;
}

//-----------------------------------------------------------------

inline void DebugPortalCamera::setTarget (const Object* target)
{
	m_target = target;
}

//===================================================================

#endif
