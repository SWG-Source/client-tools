// ============================================================================
//
// Plotter.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_Plotter_H
#define INCLUDED_Plotter_H

#if PRODUCTION == 0

#include "sharedMath/VectorArgb.h"

//-----------------------------------------------------------------------------
class Plotter
{
public:

	Plotter();
	~Plotter();

	void setFrame(int const x, int const y, int const width, int const height);
	void setFramePosition(int const x, int const y);
	void setColor(VectorArgb const &color);
	void setRange(float const min, float const max);

	void addValue(float const value);
	void draw();

private:

	typedef stdvector<float>::fwd ValueList;

	int          m_x;
	int          m_y;
	int          m_width;
	int          m_height;
	float        m_minValue;
	float        m_maxValue;
	VectorArgb   m_color;
	unsigned int m_index;
	ValueList *  m_valueList;
};

#endif // PRODUCTION == 0

// ============================================================================

#endif // INCLUDED_Plotter_H
