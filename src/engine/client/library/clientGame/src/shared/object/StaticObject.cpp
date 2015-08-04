//========================================================================
//
// StaticObject.cpp
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/StaticObject.h"

#include "clientGame/ClientCollisionProperty.h"
#include "clientGame/ClientDataFile.h"
#include "clientGame/ClientObject.h"
#include "clientGame/Game.h"
#include "sharedGame/SharedStaticObjectTemplate.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"

// ======================================================================
// class StaticObject
// ======================================================================

StaticObject::StaticObject(const SharedStaticObjectTemplate* newTemplate) :
	ClientObject(newTemplate, ClientWorld::getTangibleNotTargetableNotification())
{
	//-- create appearance @todo this should be lower in ClientObject instead of here and tangibleobject
	if (!Game::isClient () || (Game::isClient () && !newTemplate->getOnlyVisibleInTools ()))
	{
		const std::string &appearanceString = newTemplate->getAppearanceFilename();
		if (appearanceString.size() != 0)
		{
			Appearance *const appearance = AppearanceTemplateList::createAppearance(appearanceString.c_str());
			setAppearance(appearance);
		}
	}

	//-- add collision property
	addProperty(*new ClientCollisionProperty(*this));
}

//-----------------------------------------------------------------------

StaticObject::~StaticObject()
{
}

// ----------------------------------------------------------------------

StaticObject * StaticObject::asStaticObject()
{
	return this;
}

// ----------------------------------------------------------------------

StaticObject const * StaticObject::asStaticObject() const
{
	return this;
}

// ----------------------------------------------------------------------

void StaticObject::endBaselines ()
{
	ClientObject::endBaselines ();

	if (getClientData ())
		getClientData ()->applyOnOff (this, true);
}

// ======================================================================
