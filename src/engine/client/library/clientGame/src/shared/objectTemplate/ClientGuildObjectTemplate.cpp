// ======================================================================
//
// ClientGuildObjectTemplate.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientGuildObjectTemplate.h"

#include "clientGame/GuildObject.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedObject/ObjectTemplateList.h"

// ======================================================================

ClientGuildObjectTemplate::ClientGuildObjectTemplate(std::string const &filename) :
	SharedGuildObjectTemplate(filename)
{
}

// ----------------------------------------------------------------------

ClientGuildObjectTemplate::~ClientGuildObjectTemplate()
{
}

// ----------------------------------------------------------------------

ObjectTemplate *ClientGuildObjectTemplate::create(std::string const & filename)
{
	return new ClientGuildObjectTemplate(filename);
}

// ----------------------------------------------------------------------

Object *ClientGuildObjectTemplate::createObject() const
{
	return new GuildObject(this);
}

// ----------------------------------------------------------------------

void ClientGuildObjectTemplate::install()
{
	InstallTimer const installTimer("ClientGuildObjectTemplate::install");

	SharedGuildObjectTemplate::install();
	ObjectTemplateList::assignBinding(SharedGuildObjectTemplate_tag, create);
}

// ======================================================================

