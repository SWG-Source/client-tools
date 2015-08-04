//========================================================================
//
// IntangibleObject.cpp
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/IntangibleObject.h"

#include "sharedFoundation/DebugInfoManager.h"
#include "sharedFoundation/Production.h"
#include "sharedGame/SharedIntangibleObjectTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedTerrain/TerrainModificationHelper.h"
#include "clientGame/ClientWorld.h"

static const std::string THEATER_FLATTEN_LAYER("terrain/poi_small.lay");

//-----------------------------------------------------------------------

namespace IntangibleObjectNamespace
{
	std::string const & ms_debugInfoSectionName = "IntangibleObject";
}

using namespace IntangibleObjectNamespace;

//-----------------------------------------------------------------------

IntangibleObject::IntangibleObject(const SharedIntangibleObjectTemplate* newTemplate) :
	ClientObject(newTemplate, ClientWorld::getIntangibleNotification ()),
	m_count(0)
{
	addSharedVariable  (m_count);

	//-- create appearance
	const std::string & appearanceString = newTemplate->getAppearanceFilename();
	if (!appearanceString.empty())
		setAppearance(AppearanceTemplateList::createAppearance(appearanceString.c_str()));
}	


//-----------------------------------------------------------------------

IntangibleObject::~IntangibleObject()
{
}

//-----------------------------------------------------------------------

void IntangibleObject::makeFlattenedTheater()
{
	TerrainGenerator::Layer * layer = TerrainModificationHelper::importLayer(THEATER_FLATTEN_LAYER.c_str());
	if (layer != NULL)
	{
		setLayer(layer);
	}
}

//-----------------------------------------------------------------------

void IntangibleObject::getObjectInfo(std::map<std::string, std::map<std::string, Unicode::String> > & propertyMap) const
{
#if PRODUCTION == 0
/**
  When adding a variable to this class, please add it here.  Variable that aren't easily displayable are still listed, for tracking purposes.
*/

/**
	Don't compile in production build because this maps human-readable values to data members and makes hacking easier
*/

	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "Count", m_count.get());

	ClientObject::getObjectInfo(propertyMap);

#else
	UNREF(propertyMap);
#endif
}

// ======================================================================
