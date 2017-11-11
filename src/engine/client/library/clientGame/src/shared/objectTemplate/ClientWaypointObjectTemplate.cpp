// ClientWaypointObjectTemplate.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientWaypointObjectTemplate.h"

#include "clientGame/ClientWaypointObject.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedObject/ObjectTemplateList.h"

//-----------------------------------------------------------------------

ClientWaypointObjectTemplate::ClientWaypointObjectTemplate(const std::string & filename) :
	SharedWaypointObjectTemplate(filename)
{
}

//-----------------------------------------------------------------------

ClientWaypointObjectTemplate::~ClientWaypointObjectTemplate()
{
}

//-----------------------------------------------------------------------

ObjectTemplate * ClientWaypointObjectTemplate::create(const std::string & filename)
{
	return new ClientWaypointObjectTemplate(filename);
}

//-----------------------------------------------------------------------

Object * ClientWaypointObjectTemplate::createObject() const
{
	return new ClientWaypointObject(this);
}

//-----------------------------------------------------------------------

void ClientWaypointObjectTemplate::install()
{
	InstallTimer const installTimer("ClientWaypointObjectTemplate::install");

	SharedWaypointObjectTemplate::install();
	ObjectTemplateList::assignBinding(SharedWaypointObjectTemplate_tag, create);
}

//-----------------------------------------------------------------------

