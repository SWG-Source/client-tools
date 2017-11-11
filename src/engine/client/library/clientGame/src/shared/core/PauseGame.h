//
// PauseGame.h
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_PauseGame_H
#define INCLUDED_PauseGame_H

//-------------------------------------------------------------------

#include "sharedIoWin/IoWin.h"
#include "clientObject/MouseCursor.h"
#include "sharedMath/Vector2d.h"

class GameCamera;
class Transform;

//-------------------------------------------------------------------

class PauseGame : public IoWin
{
private:

	enum Keys
	{
		K_left,
		K_right,
		K_up,
		K_down,
		K_button,
		K_rollLeft,
		K_rollRight,

		K_COUNT
	};

private:

	bool             keys [K_COUNT];

	GameCamera*      camera;

	real             yaw;
	real             pitch;

	MouseCursor      mouseCursor;
	Vector2d         lastMousePosition;
	Vector2d         originalMousePosition;

	bool             advanceFrame;
	bool             freeMode;

private:

	PauseGame (const PauseGame&);
	PauseGame& operator= (const PauseGame&);

public:

	PauseGame (const GameCamera* sourceCamera, bool newFreeMode);
	virtual ~PauseGame (void);

	virtual IoResult processEvent (IoEvent* event);
	virtual void     draw (void) const;
};

//-------------------------------------------------------------------

#endif
