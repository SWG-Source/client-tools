// ======================================================================
//
// GraphicsOptionTags.cpp
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/GraphicsOptionTags.h"

#include "sharedFoundation/ConfigFile.h"

#include <map>

// ======================================================================

namespace GraphicsOptionTagsNamespace
{
	typedef stdmap<Tag, bool>::fwd Map;

	bool &findOrCreate(Tag option);
	bool * find(Tag option);

	Map  ms_map;
}
using namespace GraphicsOptionTagsNamespace;

// ======================================================================

void GraphicsOptionTags::install()
{
	ConfigFile::Section *section = ConfigFile::getSection("ClientGraphics");
	if (section)
	{
		ConfigFile::Key *key = section->findKey("disableOptionTag");
		if (key)
		{
			const int count = key->getCount();
			for (int i = 0; i < count; ++i)
				set(key->getAsTag(i, 0), false);
		}
	}

	// this is here to make sure the compiler includes the function in the link
	bool *(*f)(Tag) = &find;
	UNREF(f);
}

// ----------------------------------------------------------------------

void GraphicsOptionTags::set(Tag option, bool enabled)
{
	ms_map[option] = enabled;
}

// ----------------------------------------------------------------------

bool &GraphicsOptionTagsNamespace::findOrCreate(Tag option)
{
	Map::iterator i = ms_map.find(option);
	if (i == ms_map.end())
		i = ms_map.insert(Map::value_type(option, true)).first;

	return i->second;
}

// ----------------------------------------------------------------------

bool GraphicsOptionTags::get(Tag option)
{
	return findOrCreate(option);
}

// ----------------------------------------------------------------------

void GraphicsOptionTags::toggle(Tag option)
{
	bool &value = findOrCreate(option);
	value = !value;
}

// ----------------------------------------------------------------------

bool *GraphicsOptionTagsNamespace::find(Tag option)
{
	bool &value = findOrCreate(option);
	return &value;
}

// ======================================================================
