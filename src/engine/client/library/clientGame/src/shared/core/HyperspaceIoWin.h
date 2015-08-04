//
// HyperspaceIoWin.h
// tford
//
// copyright 2004, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_HyperspaceIoWin_H
#define INCLUDED_HyperspaceIoWin_H

//-------------------------------------------------------------------

#include "sharedIoWin/IoWin.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"
#include "clientGame/CockpitCamera.h"
#include <queue>

//-------------------------------------------------------------------

class GameCamera;
class Object;
class ShipObject;

//-------------------------------------------------------------------

class HyperspaceIoWin
: public IoWin
{
public:
	enum State
	{
		S_enter,
		S_leave
	};

public:
	static void install();
	static bool getPlayHyperspaceBeforeSceneChange();
	static void setPlayHyperspaceBeforeSceneChange(bool const playHyperspaceBeforeSceneChange);

public:
	HyperspaceIoWin(CockpitCamera * const sourceCamera, ShipObject * const playerShip, State const state, bool testing = false);
	virtual ~HyperspaceIoWin();
	virtual IoResult processEvent(IoEvent* event);
	virtual void draw() const;
	void onSceneChanged(bool const &);

public:
	class Stage;

private:
	HyperspaceIoWin();
	HyperspaceIoWin(HyperspaceIoWin const &);
	HyperspaceIoWin& operator=(HyperspaceIoWin const &);

private:
	std::queue<Stage *> m_stages;
	Watcher<CockpitCamera> m_sourceCamera;
	Watcher<ShipObject> m_playerShip;
	Stage * m_currentStageToRender;
	int m_cameraModeToRestore;
	bool const m_showCockpit;
	float const m_fieldOfView;
	State const m_currentState;
	bool const m_clampMovementToZoneVolume;
};

//-------------------------------------------------------------------

#endif
