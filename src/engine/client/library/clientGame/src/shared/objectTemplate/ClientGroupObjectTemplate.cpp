// ClientGroupObjectTemplate.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientGroupObjectTemplate.h"

#include "clientGame/GroupObject.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedObject/ObjectTemplateList.h"

//-----------------------------------------------------------------------

ClientGroupObjectTemplate::ClientGroupObjectTemplate(const std::string & filename) :
	SharedGroupObjectTemplate(filename)
{
}

//-----------------------------------------------------------------------

ClientGroupObjectTemplate::~ClientGroupObjectTemplate()
{
}

//-----------------------------------------------------------------------

ObjectTemplate * ClientGroupObjectTemplate::create(const std::string & filename)
{
	return new ClientGroupObjectTemplate(filename);
}

//-----------------------------------------------------------------------

Object * ClientGroupObjectTemplate::createObject() const
{
	return new GroupObject(this);
}

//-----------------------------------------------------------------------

void ClientGroupObjectTemplate::install()
{
	InstallTimer const installTimer("ClientGroupObjectTemplate::install");

	SharedGroupObjectTemplate::install();
	ObjectTemplateList::assignBinding(SharedGroupObjectTemplate_tag, create);
}

//-----------------------------------------------------------------------

