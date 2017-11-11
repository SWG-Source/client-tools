// ============================================================================
//
// PlotterGroup.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_PlotterGroup_H
#define INCLUDED_PlotterGroup_H

#if PRODUCTION == 0

class Plotter;
class VectorArgb;

//-----------------------------------------------------------------------------
class PlotterGroup
{
public:

	PlotterGroup();
	~PlotterGroup();

	void setFrame(int const x, int const y, int const width, int const height);
	void setColor(char const * const name, VectorArgb const &color);
	void setRange(char const * const name, float const min, float const max);

	void addValue(char const * const name, float const value);
	void draw();

private:

	void addPlotter(char const * const name);

	typedef stdmap<std::string, Plotter *>::fwd PlotterList;

	int          m_x;
	int          m_y;
	int          m_width;
	int          m_height;
	PlotterList *m_plotterList;
};

#endif // PRODUCTION == 0

// ============================================================================

#endif // INCLUDED_PlotterGroup_H
