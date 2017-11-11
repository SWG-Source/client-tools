//
// ScreenShot360.h
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_ScreenShot360_H
#define INCLUDED_ScreenShot360_H

//-------------------------------------------------------------------

#include "sharedIoWin/IoWin.h"
#include "sharedMath/Transform.h"

class CellProperty;
class GameCamera;

//-------------------------------------------------------------------

class ScreenShot360 : public IoWin
{
private:

	GameCamera* camera;

	int         state;
	Transform   transform;
	Vector      offset;
	real        pitch;
	real        yaw;
	real        zoom;
	real        radius;

private:

	ScreenShot360 (const ScreenShot360&);
	ScreenShot360& operator= (const ScreenShot360&);

public:

	ScreenShot360 (const GameCamera* sourceCamera, const CellProperty* cellProperty, const Transform& newTransform, const Vector& newOffset, real newPitch, real newYaw, real newZoom, real newRadius);
	virtual ~ScreenShot360 (void);

	virtual IoResult processEvent (IoEvent* event);
	virtual void     draw (void) const;
};

//-------------------------------------------------------------------

#endif
