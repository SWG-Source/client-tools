//
// SpaceDeath.h
// tford
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_SpaceDeath_H
#define INCLUDED_SpaceDeath_H

//-------------------------------------------------------------------

#include "sharedIoWin/IoWin.h"
#include "sharedMath/Transform.h"
#include "sharedMath/WaveForm3D.h"
#include "sharedMessageDispatch/Receiver.h"

#include <queue>

//-------------------------------------------------------------------

class GameCamera;
class ShipObject;

//-------------------------------------------------------------------

class SpaceDeath
: public IoWin
, public MessageDispatch::Receiver
{
public:

	static void install();

public:

	SpaceDeath(GameCamera const * sourceCamera, ShipObject * playerShip, float destructionSeverity, bool testing = false);
	virtual ~SpaceDeath();

	void onSceneChanged(bool const &);
	void receiveMessage(const MessageDispatch::Emitter & , const MessageDispatch::MessageBase & message);
	virtual IoResult processEvent(IoEvent* event);
	virtual void draw() const;

private:

	enum State
	{
		S_cutSceneAsPassenger,
		S_cutSceneInCockpit,
		S_cutSceneShipDestruction,
		S_cutSceneShipDestructionPadding,
		S_done
	};

private:

	SpaceDeath(SpaceDeath const &);
	SpaceDeath& operator=(SpaceDeath const &);

	void nextCutSceneIfReady(float maxTimeThisCutScene);
	float const getCurrentCutSceneTime() const;

	// the scenes below are played or skipped in the order that they are listed.

	// inside a ship but not flying it
	void updateCutSceneAsPassenger(float elapsedTime);
	void drawCutSceneAsPassenger() const;

	// first person view and flying the ship
	void updateCutSceneInCockpit(float elapsedTime);
	void drawCutSceneInCockpit() const;

	// outside view of the ship being destroyed
	void updateCutSceneShipDestruction(float elapsedTime);
	void drawCutSceneShipDestruction() const;

	// padding clip while waiting for the server to trigger scene change
	void updateCutSceneShipDestructionPadding(float elapsedTime);
	void drawCutSceneShipDestructionPadding() const;

private:

	GameCamera * const m_camera;
	ShipObject * const m_playerShip;
	float const m_destructionSeverity;
	bool const m_testing;

	WaveForm3D m_waveForm3D;
	Transform m_cameraOffset_p;

	float m_currentCutSceneTime;

	State m_state;

	std::queue<float> m_shipDestructionEffectTimes;

	class CameraTarget;
	CameraTarget * const m_cameraTarget;
};

//-------------------------------------------------------------------

#endif
