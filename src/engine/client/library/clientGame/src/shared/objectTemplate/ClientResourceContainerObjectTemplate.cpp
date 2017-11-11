//========================================================================
//
// ClientResourceContainerObjectTemplate.cpp - A wrapper around SharedResourceContainerObjectTemplate 
// to create TangibleObjects.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientResourceContainerObjectTemplate.h"

#include "clientGame/ResourceContainerObject.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedObject/ObjectTemplateList.h"

/**
 * Class constructor.
 */
ClientResourceContainerObjectTemplate::ClientResourceContainerObjectTemplate(const std::string & filename) :
	SharedResourceContainerObjectTemplate(filename)
{
}	// ClientResourceContainerObjectTemplate::ClientResourceContainerObjectTemplate

/**
 * Class destructor.
 */
ClientResourceContainerObjectTemplate::~ClientResourceContainerObjectTemplate()
{
}	// ClientResourceContainerObjectTemplate::~ClientResourceContainerObjectTemplate

/**
 * Maps the SharedResourceContainerObjectTemplate tag to use our create function.
 */
void ClientResourceContainerObjectTemplate::install(bool allowDefaultTemplateParams)
{
	InstallTimer const installTimer("ClientResourceContainerObjectTemplate::install");

	SharedResourceContainerObjectTemplate::install(allowDefaultTemplateParams);
	// replace the shared create function with ours
	ObjectTemplateList::assignBinding(SharedResourceContainerObjectTemplate_tag, create);
}	// ClientResourceContainerObjectTemplate::install

/**
 * Creates a ClientResourceContainerObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ClientResourceContainerObjectTemplate::create(const std::string & filename)
{
	return new ClientResourceContainerObjectTemplate(filename);
}	// ClientResourceContainerObjectTemplate::create

/**
 * Creates a new object from this template.
 *
 * @return the object
 */
Object * ClientResourceContainerObjectTemplate::createObject(void) const
{
	return new ResourceContainerObject(this);
}	// ClientResourceContainerObjectTemplate::createObject
