// ======================================================================
//
// Intro.h
// asommers
//
// copyright 2000, verant interactive
//
// ======================================================================

#ifndef INCLUDED_Intro_H
#define INCLUDED_Intro_H

// ======================================================================

#include "clientAudio/SoundId.h"
#include "sharedIoWin/IoWin.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/TemporaryCrcString.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Timer.h"

// ======================================================================

class CreatureObject;
class GameCamera;
class Object;
class ObjectList;
class Shader;

// ======================================================================

class Intro : public IoWin
{
public:

	Intro();
	virtual ~Intro();

	virtual IoResult processEvent(IoEvent *event);
	virtual void draw () const;

	void setCameraHardpointName (const CrcString& cameraHardpointName);

private:

	struct Data;

private:

	void load (const char* filename);

	void preloadAssets ();
	void update (float elapsedTime);
	void leaveIntro ();

private:

	Intro(const Intro &);
	Intro &operator =(const Intro &);

private:

	Data* const             m_data;
	GameCamera* const       m_camera;
	ObjectList* const       m_postureObjectList;
	Object* const           m_logoCrawlObject;

	TemporaryCrcString      m_cameraHardpointName;

	Object*                 m_skyboxObject;
	Object*                 m_starsObject;
	CreatureObject*         m_cameraObject;

	Timer                   m_timer;
	int                     m_state;

	SoundId                 m_introSoundId;

	const Shader*           m_longShader;
	float                   m_longAlpha;
	float                   m_screenAlpha;
};

// ======================================================================

#endif
