// ============================================================================
//
// PlotterManager.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_PlotterManager_H
#define INCLUDED_PlotterManager_H

#if PRODUCTION == 0

//-----------------------------------------------------------------------------
class PlotterManager
{
public:

	static void install();
	static void draw();

private:

	static void remove();

	PlotterManager();
	~PlotterManager();
	PlotterManager(PlotterManager const &);
	PlotterManager & operator=(PlotterManager const &);
};

#endif // PRODUCTION == 0

// ============================================================================

#endif // INCLUDED_PlotterManager_H
