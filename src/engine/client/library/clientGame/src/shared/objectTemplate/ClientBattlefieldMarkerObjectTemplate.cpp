// ======================================================================
//
// ClientBattlefieldMarkerObjectTemplate.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientBattlefieldMarkerObjectTemplate.h"

#include "clientGame/BattlefieldMarkerObject.h"
#include "clientGame/ClientBattlefieldMarkerOutlineObject.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedObject/ObjectTemplateList.h"

// ======================================================================

ClientBattlefieldMarkerObjectTemplate::ClientBattlefieldMarkerObjectTemplate(std::string const &filename) :
	SharedBattlefieldMarkerObjectTemplate(filename)
{
}

// ----------------------------------------------------------------------

ClientBattlefieldMarkerObjectTemplate::~ClientBattlefieldMarkerObjectTemplate()
{
}

// ----------------------------------------------------------------------

ObjectTemplate *ClientBattlefieldMarkerObjectTemplate::create(std::string const & filename)
{
	return new ClientBattlefieldMarkerObjectTemplate(filename);
}

// ----------------------------------------------------------------------

Object *ClientBattlefieldMarkerObjectTemplate::createObject() const
{
	BattlefieldMarkerObject* const object = new BattlefieldMarkerObject(this);

	if (getNumberOfPoles () > 0)
		object->addChildObject_o (new ClientBattlefieldMarkerOutlineObject (getNumberOfPoles (), getRadius ()));

	return object;
}

// ----------------------------------------------------------------------

void ClientBattlefieldMarkerObjectTemplate::install()
{
	InstallTimer const installTimer("ClientBattlefieldMarkerObjectTemplate::install");

	SharedBattlefieldMarkerObjectTemplate::install();
	ObjectTemplateList::assignBinding(SharedBattlefieldMarkerObjectTemplate_tag, create);
}

// ======================================================================

