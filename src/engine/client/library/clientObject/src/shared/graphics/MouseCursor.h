// ======================================================================
//
// MouseCursor.h
// Portions copyright 1999 Bootprint Entertainment
// Portions copyright 2002 Sony Online Entertainment
// All Rights Reserved
//
// ======================================================================

#ifndef INCLUDED_MouseCursor_H
#define INCLUDED_MouseCursor_H

// ======================================================================

struct IoEvent;
class  VectorArgb;

// ======================================================================

class MouseCursor
{
public:

	static void setAcceleration(int newSpeed, int newThreshold1, int newThreshold2);

	enum Style
	{
		S_system,
		S_absolute,
		S_relative
	};

public:

	explicit MouseCursor(int mouseNumber, Style style=S_system);
	~MouseCursor();

	void processEvent(const IoEvent *event);
	void gotoXY(int newX, int newY);
	void center();

	int getX() const;
	int getY() const;

	void draw(const VectorArgb &color) const;

private:

	// disable these
	MouseCursor();
	MouseCursor(const MouseCursor &);
	MouseCursor &operator =(const MouseCursor &);

private:

	Style m_style;
	bool  m_relativePerFrame;
	int   m_mouseNumber;
	int   m_localX;
	int   m_localY;
	int & m_x;
	int & m_y;
};

// ======================================================================
/**
 * Get the mouse X position
 */

inline int MouseCursor::getX() const
{
	return m_x;
}

// ----------------------------------------------------------------------
/**
 * Get the mouse Y position.
 */

inline int MouseCursor::getY() const
{
	return m_y;
}

// ======================================================================

#endif
