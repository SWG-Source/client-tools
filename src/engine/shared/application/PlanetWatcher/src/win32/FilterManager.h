// ============================================================================
//
// FilterManager.h
// copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_FilterManager_H
#define INCLUDED_FilterManager_H

#include "PlanetWatcherRenderer.h"

//-----------------------------------------------------------------------------
class FilterManager
{
public:
	static void install();
	static void remove();

	static void getFilterTypes(stdvector<std::string>::fwd & filterTypes);
	static void getConditions(stdvector<std::string>::fwd & conditions, std::string const & filterType);
	static bool passesFilter(std::string const & filterType, std::string const & condition, std::string const & valueStr, PlanetWatcherRenderer::MiniMapObject const & object);
	static bool validateValue(std::string const & filterType, std::string const & value);
	static std::string const & getValidationFailedMessage(std::string const & filterType);
	static void setValidValues(std::string const & filterType, stdvector<std::string>::fwd const & validValues);

private:
	// Disabled
	FilterManager();
	FilterManager(FilterManager const &);
	~FilterManager();
	FilterManager &operator =(FilterManager const &);
};

// ============================================================================

#endif // FilterManager_H
