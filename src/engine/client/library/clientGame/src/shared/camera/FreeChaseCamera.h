//===================================================================
//
// FreeChaseCamera.h
// asommers 5-31-2000
//
// copyright 2000, verant interactive, inc.
//
//===================================================================

#ifndef INCLUDED_FreeChaseCamera_H
#define INCLUDED_FreeChaseCamera_H

//===================================================================

#include "clientObject/GameCamera.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"
#include "swgSharedUtility/Postures.h"
#include "swgSharedUtility/States.h"

class MessageQueue;

//===================================================================

class FreeChaseCamera : public GameCamera
{
public:

	FreeChaseCamera ();
	virtual ~FreeChaseCamera ();

	virtual void  setActive (bool active);
	virtual float alter (float time);

	void          setMessageQueue (const MessageQueue* queue);
	void          setTarget (const Object* object, bool force = false, bool overwriteYaw = true);
	void          setZoomMultiplier(float multiplier);

	//-- used for 3d screenshot support
	const Object* getTarget () const;
	const Vector& getOffset () const;
	float         getPitch () const;
	float         getYaw () const;
	float         getZoom () const;
	float         getRadius () const;

	bool          isFirstPerson () const;

	typedef void (*ModeCallback) (void* context);
	void          setModeCallback (ModeCallback modeCallback, void* context);

	static void		  setOffsetCamera(bool b);
	static bool		  getOffsetCamera();

	enum CameraMode
	{
		CM_free,
		CM_chase
	};

	static void         setCameraMode (CameraMode mode);
	static CameraMode   getCameraMode ();

	static void         setCameraZoomSpeed (float f);
	static float        getCameraZoomSpeed ();

	static void         setCameraSimpleCollision (bool cameraSimpleCollision);
	static bool         getCameraSimpleCollision ();
	static bool         getCameraSimpleCollisionDefault ();

	static void			setCameraHeight(float f);
	static float		getCameraHeight();
	static float		getCameraHeightDefault();
	

	static float        ms_vehicleCameraOffsetY;

private:

	FreeChaseCamera (const FreeChaseCamera&);
	FreeChaseCamera& operator= (const FreeChaseCamera&);

	void setFirstPerson (bool firstPerson);

private:

	void                   alterCheckPostureOffsets ();

	const MessageQueue*    m_queue;
	ConstWatcher<Object>   m_target;

	Postures::Enumerator   m_currentPosture;
	uint64                 m_currentStates;
	bool                   m_currentFirstPerson;
	Vector                 m_desiredOffset;
	Vector                 m_offset;
	float                  m_yaw_w;
	float                  m_offsetYaw_w;
	float                  m_zoom;
	float                  m_zoomMultiplier;
	float                  m_radius;
	float                  m_currentZoom;
	bool                   m_firstPerson;
	float                  m_lastTurnRate;

	//-- loaded from data file
	int                    m_numberOfSettings;
	float*                 m_settings;

	float                  m_firstPersonDistance;

	Vector                 m_offsetDefault;
	float                  m_thirdPersonXOffset;

	typedef stdmap<int, Vector>::fwd PostureMap;
	PostureMap* const      m_offsetPostureMap;

	typedef stdmap<int, Vector>::fwd PostureStateMap;
	PostureStateMap* const m_offsetPostureStateMap;

	ModeCallback           m_modeCallback;
	void*                  m_context;
	bool                   m_currentMode;

	bool                   m_spinning;
	bool                   m_colliding;

	float				   m_reticleOffsetCenter;
	float				   m_reticleOffsetShoulder;
};

//===================================================================

#endif

