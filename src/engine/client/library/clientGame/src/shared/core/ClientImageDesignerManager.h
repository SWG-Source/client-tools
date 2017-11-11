// ======================================================================
//
// ClientImageDesignerManager.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ClientImageDesignerManager_H
#define INCLUDED_ClientImageDesignerManager_H

// ======================================================================

#include "sharedGame/CustomizationManager.h"
#include "sharedGame/SharedImageDesignerManager.h"

// ======================================================================

class CreatureObject;
class CustomizationData;

namespace MessageDispatch
{
	class Callback;
};

//-----------------------------------------------------------------------

class ClientImageDesignerManager
{
public:
	static void setMorphCustomization(CreatureObject & creature, std::string const & customizationName, float value);
	static void setIndexCustomization(CreatureObject & creature, std::string const & customizationName, int value);
	static float getMorphCustomization(CreatureObject & creature, std::string const & customizationName);
	static int getIndexCustomization(CreatureObject & creature, std::string const & customizationName);
	static SharedImageDesignerManager::SkillMods getSkillModsForDesigner(NetworkId const & designerId);
	static NetworkId getNextClientNetworkId();
	static CustomizationData * fetchCustomizationDataForCustomization(CreatureObject & creature, CustomizationManager::Customization const & customization);

public:
	static void install();

private:
	//disabled
	ClientImageDesignerManager (ClientImageDesignerManager const & rhs);
	ClientImageDesignerManager & operator= (ClientImageDesignerManager const & rhs);

private:
	static void remove();
};

// ======================================================================

#endif
