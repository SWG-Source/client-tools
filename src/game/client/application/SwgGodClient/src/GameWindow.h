// ======================================================================
//
// GameWindow.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_GameWindow_H
#define INCLUDED_GameWindow_H

// ======================================================================

#include "BaseGameWindow.h"

// ======================================================================

/* *
* The GameWindow is the Widget which contains the GameWidget and other status information.
*/
class GameWindow : public BaseGameWindow
{
	Q_OBJECT; //lint !e1516 !e19 !e1924 !e1762 various deficiencies in the Qt macro

public:
	explicit GameWindow(QWidget *parent=0, const char *name=0);
	virtual ~GameWindow();

public slots:
	void onGameExclusiveFocusChanged (bool b);
	void onInteriorCameraFocusChanged(bool b);
	void onPivotDistanceChanged      (float distance);
	void onGameFocusAllowedChanged   (bool);
	void onCursorWorldPositionChanged(float positionX, float positionY, float positionZ);
	void onBuildoutRegionChanged     (const char *, const char *);

private:
	bool m_interiorCameraOn;
};

// ======================================================================

#endif
