// ============================================================================
//
// PlotterGroup.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/PlotterGroup.h"

#if PRODUCTION == 0

#include "clientGame/Plotter.h"
#include <map>

// ============================================================================
//
// PlotterGroup
//
// ============================================================================

//-----------------------------------------------------------------------------
PlotterGroup::PlotterGroup()
 : m_x(0)
 , m_y(0)
 , m_width(0)
 , m_height(0)
 , m_plotterList(NULL)
{
	m_plotterList = new PlotterList;
}

//-----------------------------------------------------------------------------
PlotterGroup::~PlotterGroup()
{
	if (m_plotterList != NULL)
	{
		while (!m_plotterList->empty())
		{
			PlotterList::iterator iterPlotterList = m_plotterList->begin();
			Plotter *plotter = iterPlotterList->second;

			m_plotterList->erase(iterPlotterList);

			delete plotter;
			plotter = NULL;
		}
	}

	delete m_plotterList;
	m_plotterList = NULL;
}

//-----------------------------------------------------------------------------
void PlotterGroup::setFrame(int const x, int const y, int const width, int const height)
{
	DEBUG_FATAL((width <= 2), ("width must be > 2"));

	if (width > 2)
	{
		m_x = x;
		m_y = y;
		m_width = width;
		m_height = height;

		PlotterList::iterator iterPlotterList = m_plotterList->begin();

		for (; iterPlotterList != m_plotterList->end(); ++iterPlotterList)
		{
			Plotter *plotter = iterPlotterList->second;

			plotter->setFrame(x, y, width, height);
		}
	}
}

//-----------------------------------------------------------------------------
void PlotterGroup::addPlotter(char const * const name)
{
	PlotterList::const_iterator iterPlotterGroup = m_plotterList->find(name);

	if (iterPlotterGroup == m_plotterList->end())
	{
		Plotter *plotter = new Plotter;
		plotter->setFrame(m_x, m_y, m_width, m_height);
		m_plotterList->insert(std::make_pair(name, plotter));
	}
}

//-----------------------------------------------------------------------------
void PlotterGroup::setColor(char const * const name, VectorArgb const &color)
{
	addPlotter(name);

	PlotterList::const_iterator iterPlotterGroup = m_plotterList->find(name);

	if (iterPlotterGroup != m_plotterList->end())
	{
		iterPlotterGroup->second->setColor(color);
	}
	else
	{
		DEBUG_WARNING(true, ("Unable to find plotter: %s", name));
	}
}

//-----------------------------------------------------------------------------
void PlotterGroup::setRange(char const * const name, float const min, float const max)
{
	addPlotter(name);

	PlotterList::const_iterator iterPlotterGroup = m_plotterList->find(name);

	if (iterPlotterGroup != m_plotterList->end())
	{
		iterPlotterGroup->second->setRange(min, max);
	}
	else
	{
		DEBUG_WARNING(true, ("Unable to find plotter: %s", name));
	}
}

//-----------------------------------------------------------------------------
void PlotterGroup::addValue(char const * const name, float const value)
{
	addPlotter(name);

	PlotterList::const_iterator iterPlotterGroup = m_plotterList->find(name);

	if (iterPlotterGroup != m_plotterList->end())
	{
		iterPlotterGroup->second->addValue(value);
	}
	else
	{
		DEBUG_WARNING(true, ("Unable to find plotter: %s", name));
	}
}

//-----------------------------------------------------------------------------
void PlotterGroup::draw()
{
	PlotterList::const_iterator iterPlotterList = m_plotterList->begin();

	for (; iterPlotterList != m_plotterList->end(); ++iterPlotterList)
	{
		Plotter *plotter = iterPlotterList->second;

		plotter->draw();
	}
}

#endif // PRODUCTION == 0

// ============================================================================
