//
// FreeCamera.h
// asommers 6-01-2000
//
// copyright 2000, verant interactive, inc.
//

#ifndef FREECAMERA_H
#define FREECAMERA_H

//-------------------------------------------------------------------

#include "clientObject/GameCamera.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"

class CellProperty;
class Object;
class MessageQueue;

//-------------------------------------------------------------------

class FreeCamera : public GameCamera
{
public:
	//public nested classes/structs
	struct Info
	{
		Vector translate;
		real   distance;
		real   yaw;
		real   pitch;
	};

	enum Mode
	{
		M_fly,
		M_pivot
	};

private:
	//private nested classes/structs
	enum Keys
	{
		K_up,
		K_down,
		K_left,
		K_right,
		K_button,
		K_COUNT
	};

public:
	//public API
	FreeCamera ();
	virtual       ~FreeCamera (void);

	void           setMessageQueue (const MessageQueue* newQueue);
	void           setTarget (const Object* newTarget);
	
	virtual void   setActive (bool newActive);
	virtual float  alter (float time);

	void           setMode (Mode mode);

	real           getPivotDistance () const;
	void           setPivotDistance (real r);

	real           getPitch () const;
	real           getYaw   () const;

	void           setPitch (real pitch);
	void           setYaw   (real yaw);

	void           setPivotPoint (const Vector & pt);
	const Vector   getPivotPoint () const;

	const Info &   getInfo () const;
	void           setInfo (const Info & info);

	void           setTargetInfo (const Info & info, real time);
	void           getTargetInfo (Info & info) const;

	void           setInterpolating (bool b);
	bool           getInterpolating () const;

	void           setInitializeFromFreeChaseCamera (bool val);
	void           setInitializeFromFreeCamera (bool val);

private:
	//disabled
	FreeCamera (const FreeCamera&);
	FreeCamera& operator= (const FreeCamera&);


private:
	//private functions
	void convertPivotPoint (CellProperty* cell);

private:
	//private data members

	const MessageQueue*      m_queue;

	bool                     m_keys [K_COUNT];

	//-- target in this context represents the object 
	// we start from when we switch to the camera
	ConstWatcher<Object>     m_target;
	Mode                     m_mode;

	//-- camera position info
	Info                     m_info;
	Info                     m_targetInfo;

	bool                     m_interpolating;

	real                     m_minPivotDistance;
	float                    m_lastSpeed;

	CellProperty*            m_currentCell;

	bool					 m_initializeFromFreeChaseCamera;
	bool                     m_initializeFromFreeCamera;
};

//-------------------------------------------------------------------

inline void FreeCamera::setMessageQueue (const MessageQueue* newQueue)
{
	m_queue = newQueue;
}

//-----------------------------------------------------------------

inline void FreeCamera::setTarget (const Object* newTarget)
{
	m_target = newTarget;
}

//-----------------------------------------------------------------

inline real FreeCamera::getPivotDistance () const
{
	return m_info.distance;
}
//-----------------------------------------------------------------

inline void FreeCamera::setPivotDistance (const real r)
{
	m_info.distance = r;
	m_mode = M_pivot;
}

//-----------------------------------------------------------------

inline real FreeCamera::getPitch () const
{
	return m_info.pitch;
}

//-----------------------------------------------------------------

inline real FreeCamera::getYaw () const
{
	return m_info.yaw;
}

//-----------------------------------------------------------------

inline void FreeCamera::setYaw (const real r)
{
	m_info.yaw = r;
}

//-----------------------------------------------------------------

inline void FreeCamera::setPitch (const real r)
{
	m_info.pitch = r;
}

//-------------------------------------------------------------------

inline void FreeCamera::setInterpolating (bool b)
{
	m_interpolating = b;
}

//-----------------------------------------------------------------

inline bool FreeCamera::getInterpolating () const
{
	return m_interpolating;
}

//-----------------------------------------------------------------

inline const FreeCamera::Info &   FreeCamera::getInfo () const
{
	return m_info;
}

//-----------------------------------------------------------------

inline void  FreeCamera::setInfo (const FreeCamera::Info & info)
{
	m_info = info;
	m_info.distance = m_info.distance;
}
//-----------------------------------------------------------------

inline void FreeCamera::setInitializeFromFreeChaseCamera(bool val)
{
	m_initializeFromFreeChaseCamera = val;
}

//-----------------------------------------------------------------

inline void FreeCamera::setInitializeFromFreeCamera(bool val)
{
	m_initializeFromFreeCamera = val;
}

//-----------------------------------------------------------------

#endif
