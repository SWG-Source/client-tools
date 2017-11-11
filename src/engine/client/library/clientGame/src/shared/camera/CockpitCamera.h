// ======================================================================
//
// CockpitCamera.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_CockpitCamera_H
#define INCLUDED_CockpitCamera_H

// ======================================================================

#include "clientObject/GameCamera.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Timer.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"

class AppearanceTemplate;
class Iff;
class MessageQueue;

// ======================================================================

class CockpitCamera : public GameCamera
{
public:

	static void install();

	static void setShowCockpit(bool showCockpit);
	static bool getShowCockpit();

public:

	CockpitCamera();
	virtual ~CockpitCamera();

	virtual void setActive(bool active);
	virtual float alter(float elapsedTime);

	void setMessageQueue(MessageQueue const * queue);
	void setTarget(Object const * object);

	bool isFirstPerson() const;
	float getMaximumZoomOutSetting() const;

	// returns true if the camera really is can be first person
	bool calculatePilotCameraLocation(bool tryForFirstPerson, CellProperty * & targetCellProperty_p, Transform & targetTransform_p, Transform & targetTransform_w);

	// returns true if the frame object can be created
	bool createFrame(Object * & object, Transform * const cameraOffset_p = 0) const;

	void setCameraLockTarget(bool b);
	bool getCameraLockTarget() const;

	Transform const & getHyperspaceCameraOffset() const;
	void setIsInHyperspace(bool isInHyperspace);
	
	bool isZoomWithinFirstPersonDistance() const;

private:

	CockpitCamera(CockpitCamera const &);
	CockpitCamera & operator=(CockpitCamera const &);

	void load(char const * fileName);
	void load_0000(Iff & iff);

	void setDustEnabled(bool dustEnabled);

	void setYawAndYawTarget(float yaw, float yawTarget);
	void setPitchAndPitchTarget(float pitch, float pitchTarget);

	void _setFrameAppearanceTemplate(AppearanceTemplate const *);
	
private:

	MessageQueue const * m_queue;
	ConstWatcher<Object> m_target;
	ConstWatcher<Object> m_shipTarget;

	float m_yaw;
	float m_pitch;
	bool m_locked;

	Object * m_dustObject;
	AppearanceTemplate const * m_frameAppearanceTemplate;
	Transform m_cameraOffset_p;
	Transform m_hyperspaceOffset_p;

	typedef stdvector<float>::fwd ZoomSettingList;
	ZoomSettingList * const m_zoomSettingList;
	int m_zoomSetting;
	float m_zoom;
	float m_currentZoom;
	float m_firstPersonDistance;

	class CameraDampener;
	CameraDampener * const m_cameraDampener;

	Timer m_timerUntilCameraRecenter;
	bool m_useTimerForCameraRecenter;

	float m_yawTarget;
	float m_pitchTarget;
	bool m_povHatActive;

	float m_povHatPanLastSpeedFactor;

	bool m_cameraLockTarget;
	bool m_rearView;
	bool m_wasShowCockpitBeforeRearView;

	bool m_isInHyperspace;
};

// ======================================================================

#endif

