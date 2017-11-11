// ======================================================================
//
// UIBuilderLoader.h
// copyright (c) 2005 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_UIBuilderLoader_H
#define INCLUDED_UIBuilderLoader_H

// ======================================================================

#include "UILoader.h"

#include <map>
#include <string>

// ======================================================================

class UIBuilderLoader : public UILoader
{
public:

	UIBuilderLoader() : ReadOnlyFilesInInput(false) {}

	virtual bool LoadStringFromResource(const UINarrowString &ResourceName, UINarrowString &Out);

	// ----------------------------------------------------

	bool ReadOnlyFilesInInput;

	// ----------------------------------------------------

	typedef std::map<std::string, int> FileTimeMap;

	static FileTimeMap s_fileTimes;
};

// ======================================================================

#endif
