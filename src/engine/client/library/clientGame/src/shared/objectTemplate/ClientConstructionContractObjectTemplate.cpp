// ClientConstructionContractObjectTemplate.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientConstructionContractObjectTemplate.h"

#include "sharedDebug/InstallTimer.h"
#include "sharedObject/ObjectTemplateList.h"

//-----------------------------------------------------------------------

ClientConstructionContractObjectTemplate::ClientConstructionContractObjectTemplate(const std::string & filename) :
	SharedConstructionContractObjectTemplate(filename)
{
}

//-----------------------------------------------------------------------

ClientConstructionContractObjectTemplate::~ClientConstructionContractObjectTemplate()
{
}

//-----------------------------------------------------------------------

void ClientConstructionContractObjectTemplate::install()
{
	InstallTimer const installTimer("ClientConstructionContractObjectTemplate::install");

	SharedConstructionContractObjectTemplate::install();
	ObjectTemplateList::assignBinding(SharedConstructionContractObjectTemplate_tag, create);
}

//-----------------------------------------------------------------------

ObjectTemplate * ClientConstructionContractObjectTemplate::create(const std::string & filename)
{
	return new ClientConstructionContractObjectTemplate(filename);
}

//-----------------------------------------------------------------------

Object * ClientConstructionContractObjectTemplate::createObject() const
{
	return 0;
//	return new ConstructionContract(this);
}

//-----------------------------------------------------------------------

