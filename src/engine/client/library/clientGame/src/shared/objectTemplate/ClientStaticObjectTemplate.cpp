//========================================================================
//
// ClientStaticObjectTemplate.cpp - A wrapper around SharedStaticObjectTemplate 
// to create StaticObjects.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientStaticObjectTemplate.h"

#include "clientGame/StaticObject.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedObject/ObjectTemplateList.h"

/**
 * Class constructor.
 */
ClientStaticObjectTemplate::ClientStaticObjectTemplate(const std::string & filename) :
	SharedStaticObjectTemplate(filename)
{
}	// ClientStaticObjectTemplate::ClientStaticObjectTemplate

/**
 * Class destructor.
 */
ClientStaticObjectTemplate::~ClientStaticObjectTemplate()
{
}	// ClientStaticObjectTemplate::~ClientStaticObjectTemplate

/**
 * Maps the SharedStaticObjectTemplate tag to use our create function.
 */
void ClientStaticObjectTemplate::install(bool allowDefaultTemplateParams)
{
	InstallTimer const installTimer("ClientStaticObjectTemplate::install");

	SharedStaticObjectTemplate::install(allowDefaultTemplateParams);
	// replace the shared create function with ours
	ObjectTemplateList::assignBinding(SharedStaticObjectTemplate_tag, create);
}	// ClientStaticObjectTemplate::install

/**
 * Creates a ClientStaticObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ClientStaticObjectTemplate::create(const std::string & filename)
{
	return new ClientStaticObjectTemplate(filename);
}	// ClientStaticObjectTemplate::create

/**
 * Creates a new object from this template.
 *
 * @return the object
 */
Object * ClientStaticObjectTemplate::createObject(void) const
{
	return new StaticObject(this);
}	// ClientStaticObjectTemplate::createObject
