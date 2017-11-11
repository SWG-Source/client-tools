// ClientMissionObjectTemplate.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientMissionObject.h"

#include "clientGame/ClientMissionObjectTemplate.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedObject/ObjectTemplateList.h"

//-----------------------------------------------------------------------

ClientMissionObjectTemplate::ClientMissionObjectTemplate(const std::string & filename) :
	SharedMissionObjectTemplate(filename)
{
}

//-----------------------------------------------------------------------

ClientMissionObjectTemplate::~ClientMissionObjectTemplate()
{
}

//-----------------------------------------------------------------------

ObjectTemplate * ClientMissionObjectTemplate::create(const std::string & filename)
{
	return new ClientMissionObjectTemplate(filename);
}

//-----------------------------------------------------------------------

Object * ClientMissionObjectTemplate::createObject() const
{
	return new ClientMissionObject(this);
}

//-----------------------------------------------------------------------

void ClientMissionObjectTemplate::install()
{
	InstallTimer const installTimer("ClientMissionObjectTemplate::install");

	SharedMissionObjectTemplate::install();
	ObjectTemplateList::assignBinding(SharedMissionObjectTemplate_tag, create);
}

//-----------------------------------------------------------------------

