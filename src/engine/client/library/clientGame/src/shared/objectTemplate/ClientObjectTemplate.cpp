//========================================================================
//
// ClientObjectTemplate.cpp - A wrapper around SharedObjectTemplate to create 
// ClientObjects.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientObjectTemplate.h"

#include "clientGame/ClientObject.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedObject/ObjectTemplateList.h"

/**
 * Class constructor.
 */
ClientObjectTemplate::ClientObjectTemplate(const std::string & filename) :
	SharedObjectTemplate(filename)
{
}	// ClientObjectTemplate::ClientObjectTemplate

/**
 * Class destructor.
 */
ClientObjectTemplate::~ClientObjectTemplate()
{
}	// ClientObjectTemplate::~ClientObjectTemplate

/**
 * Maps the SharedObjectTemplate tag to use our create function.
 */
void ClientObjectTemplate::install(bool allowDefaultTemplateParams)
{
	InstallTimer const installTimer("ClientObjectTemplate::install");

	SharedObjectTemplate::install(allowDefaultTemplateParams);
	// replace the shared create function with ours
	ObjectTemplateList::assignBinding(SharedObjectTemplate_tag, create);
}	// ClientObjectTemplate::install

/**
 * Creates a ClientObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ClientObjectTemplate::create(const std::string & filename)
{
	return new ClientObjectTemplate(filename);
}	// ClientObjectTemplate::create

/**
 * Creates a new object from this template.
 *
 * @return the object
 */
Object * ClientObjectTemplate::createObject(void) const
{
	return new ClientObject(this);
}	// ClientObjectTemplate::createObject
