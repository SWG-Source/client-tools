// ClientPlayerQuestObjectTemplate.cpp
// Copyright 2009, Sony Online Entertainment Inc., all rights reserved. 
// Author: Matt Boudreaux

//-----------------------------------------------------------------------

#include "clientGame/FirstClientGame.h"

#include "clientGame/ClientPlayerQuestObjectTemplate.h"
#include "clientGame/ClientPlayerQuestObject.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedObject/ObjectTemplateList.h"

//-----------------------------------------------------------------------

ClientPlayerQuestObjectTemplate::ClientPlayerQuestObjectTemplate(const std::string & filename) :
SharedPlayerQuestObjectTemplate(filename)
{
}

//-----------------------------------------------------------------------

ClientPlayerQuestObjectTemplate::~ClientPlayerQuestObjectTemplate()
{
}

//-----------------------------------------------------------------------

ObjectTemplate * ClientPlayerQuestObjectTemplate::create(const std::string & filename)
{
	return new ClientPlayerQuestObjectTemplate(filename);
}

//-----------------------------------------------------------------------

Object * ClientPlayerQuestObjectTemplate::createObject() const
{
	return new ClientPlayerQuestObject(this);
}

//-----------------------------------------------------------------------

void ClientPlayerQuestObjectTemplate::install()
{
	InstallTimer const installTimer("ClientPlayerQuestObjectTemplate::install");

	SharedPlayerQuestObjectTemplate::install();
	ObjectTemplateList::assignBinding(SharedPlayerQuestObjectTemplate_tag, create);
}

//-----------------------------------------------------------------------

