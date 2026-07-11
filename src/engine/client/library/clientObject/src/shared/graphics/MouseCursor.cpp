// ======================================================================
//
// MouseCursor.cpp
// Portions copyright 1999 Bootprint Entertainment
// Portions copyright 2002 Sony Online Entertainment
// All Rights Reserved
//
// ======================================================================

#include "clientObject/FirstClientObject.h"
#include "clientObject/MouseCursor.h"

#include "clientGraphics/Graphics.h"
#include "sharedIoWin/IoWin.def"
#include "sharedFoundation/Os.h"

// ======================================================================

namespace MouseCursorNamespace
{
	int ms_speed;
	int ms_threshold1;
	int ms_threshold2;
	int ms_globalX;
	int ms_globalY;

	int accelerate(int value);
}
using namespace MouseCursorNamespace;

// ======================================================================
//
// If speed is 0, the mouse has no acceleration.
//
// If speed greather than 0, and the mouse moves further than threshold1,
// the movement is doubled.
//
// If speed greather than 1, and the mouse moves further than threshold2,
// the movement is doubled again.
//
//@param speed        Acceleration type
//@param threshold1   Threshold for doubling the movement
//@param threshold2   Threshold for doubling the movement again

void MouseCursor::setAcceleration(int speed, int threshold1, int threshold2)
{
	speed = clamp(0, (speed & 0xff), 2);
	ms_speed = speed;
	ms_threshold1 = threshold1;
	ms_threshold2 = threshold2;
}

// ----------------------------------------------------------------------

int MouseCursorNamespace::accelerate(int value)
{
	if (ms_speed > 1 && abs(value) > ms_threshold2)
		value *= 4;
	else
		if (ms_speed > 0 && abs(value) > ms_threshold1)
			value *= 2;

	return value;
}

// ======================================================================
// Construct a new MouseCursor
//
// Remarks:
//
//   The global flag is used to determine whether to use a single 'global'
//   cursor position.  This is useful when there are multiple MouseCursor
//   objects that all want to control the same cursor.  For instance, each
//   IoWin will likely have its own MouseCursor, but changing IoWins should
//   not move the cursor around.
//
// @param mouseNumber  The mouse number to watch
// @param style        Whether to use the "global" mouse position or not

MouseCursor::MouseCursor(int mouseNumber, Style style)
:
	m_mouseNumber(mouseNumber),
	m_style(style),
	m_localX(0),
	m_localY(0),
	m_x(m_style == S_system ? ms_globalX : m_localX),
	m_y(m_style == S_system ? ms_globalY : m_localY)
{
	if (m_style == S_absolute)
		center();
}

// ----------------------------------------------------------------------
/**
 * Destroy a MouseCursor.
 */

MouseCursor::~MouseCursor()
{
}

// ----------------------------------------------------------------------
/**
 * Update the cursor position.
 * 
 * This routine will ignore events that are not of the proper type or
 * mouse number.
 * 
 * Acceleration may be applied to the motion.
 */

void MouseCursor::processEvent(const IoEvent *event)
{
	NOT_NULL(event);

	// UI scale support (2026-05-16): m_x/m_y are LOGICAL UI-canvas coords
	// (what gets fed to UI hit-test and rendered through the canvas Scale
	// transform). OS event coords / deltas are PHYSICAL pixels. Divide raw
	// input by uiScale so cursor moves 1:1 visually with the physical
	// mouse while UI sees scaled-down coords. Clamp to UI canvas size
	// (smaller than physical) so cursor doesn't drift past the rendered
	// edge. At uiScale == 1.0 (default) all of this is a no-op.
	const float uiScale = Graphics::getUiCanvasScale();
	const int maxX = Graphics::getUiCanvasWidth()  - 1;
	const int maxY = Graphics::getUiCanvasHeight() - 1;

	if (m_style == S_system)
	{
		if (event->type == IOET_SetSystemMouseCursorPosition)
		{
			DEBUG_WARNING(m_x < 0 || m_x > maxX || m_y < 0 || m_y > maxY, ("Mouse cursor out of range %d,%d", m_x, m_y));
			m_x = clamp(0, static_cast<int>(static_cast<float>(event->arg1) / uiScale), maxX);
			m_y = clamp(0, static_cast<int>(static_cast<float>(event->arg2) / uiScale), maxY);
		}
	}
	else
	{
		if (event->type == IOET_Prepare)
		{
			if (m_style == S_relative)
			{
				m_x = 0;
				m_y = 0;
			}
		}
		else
			if (event->type == IOET_MouseMove && event->arg1 == m_mouseNumber)
			{
				// UI scale: divide by uiScale ONLY for S_absolute (the on-screen
				// cursor whose m_x/m_y feeds UI hit-test in logical canvas
				// coords). For S_relative (mouse-look / free-look camera mode)
				// the deltas accumulate into m_x/m_y as a per-frame "mouse
				// movement in pixels" -- the consumer (GroundScene::
				// handleInputMapUpdate) divides those by the camera viewport
				// width which is in PHYSICAL pixels, so the relative delta
				// must also stay in physical pixels or the camera turn
                // sensitivity gets cut by 1/uiScale (user-visible at
                // uiScale>1 as the pitch axis barely moving past horizon).
				const int accelDelta = accelerate(static_cast<int>(event->arg3));
				const int delta = (m_style == S_absolute)
				    ? static_cast<int>(static_cast<float>(accelDelta) / uiScale)
				    : accelDelta;

				if (event->arg2 == IOMT_TranslateX)
				{
					m_x += delta;
					if (m_style == S_absolute)
						m_x = clamp(0, m_x, maxX);
				}

				if (event->arg2 == IOMT_TranslateY)
				{
					m_y += delta;
					if (m_style == S_absolute)
						m_y = clamp(0, m_y, maxY);
				}
			}
	}
}

// ----------------------------------------------------------------------
/**
 * Move the mouse cursor.
 * 
 * @param newX  New X position for the mouse cursor
 * @param newY  New Y position for the mouse cursor
 */

void MouseCursor::gotoXY(int x, int y)
{
	// UI scale: x, y arrive in LOGICAL UI-canvas coords (callers like
	// CuiIoWin use m_mouseCursor->getX/Y values that are already logical).
	// Clamp to UI canvas size, not physical pixels. setSystemMouseCursorPosition
	// expects physical pixels for the actual OS cursor.
	const int maxX = Graphics::getUiCanvasWidth()  - 1;
	const int maxY = Graphics::getUiCanvasHeight() - 1;
	const float uiScale = Graphics::getUiCanvasScale();

	switch(m_style)
	{
		case S_absolute:
			m_x = clamp(0, x, maxX);
			m_y = clamp(0, y, maxY);
			break;

		case S_system:
			m_x = clamp(0, x, maxX);
			m_y = clamp(0, y, maxY);
			// OS cursor position is in physical pixels -- multiply back up.
			Graphics::setSystemMouseCursorPosition(
				static_cast<int>(static_cast<float>(x) * uiScale),
				static_cast<int>(static_cast<float>(y) * uiScale));
			break;

		case S_relative:
			DEBUG_FATAL(true, ("cannot call this on a relative cursor"));
			break;

		default:
			DEBUG_FATAL(true, ("unknown case"));
			break;
	}

}

// ----------------------------------------------------------------------
/**
 * Center the mouse cursor on the screen.
 */

void MouseCursor::center()
{
	// UI canvas center, not physical pixel center -- gotoXY expects logical coords.
	gotoXY(Graphics::getUiCanvasWidth() / 2, Graphics::getUiCanvasHeight() / 2);
}

// ----------------------------------------------------------------------
/**
 * Draw a mouse cursor.
 * 
 * Currently this routine draws a 3x3 pixel plus sign in the specified
 * color.
 */

void MouseCursor::draw(const VectorArgb &color) const
{
	const int x = getX();
	const int y = getY();
	Graphics::drawLine(x-3, y, x+4, y, color);
	Graphics::drawLine(x, y-3, x, y+4, color);
}

// ======================================================================
