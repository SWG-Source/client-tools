//========================================================================
//
// ClientCreatureObjectTemplate.cpp - A wrapper around SharedCreatureObjectTemplate 
// to create TangibleObjects.
//
// copyright 2001, 2002 Sony Online Entertainment, Inc.
//
//========================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientCreatureObjectTemplate.h"

#include "clientGame/CreatureObject.h"
#include "clientGame/GameAnimationMessageCallback.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedObject/ObjectTemplateList.h"

// ======================================================================
/**
 * Class constructor.
 */
ClientCreatureObjectTemplate::ClientCreatureObjectTemplate(const std::string & filename) :
	SharedCreatureObjectTemplate(filename)
{
}	// ClientCreatureObjectTemplate::ClientCreatureObjectTemplate

/**
 * Class destructor.
 */
ClientCreatureObjectTemplate::~ClientCreatureObjectTemplate()
{
}	// ClientCreatureObjectTemplate::~ClientCreatureObjectTemplate

/**
 * Maps the SharedCreatureObjectTemplate tag to use our create function.
 */
void ClientCreatureObjectTemplate::install(bool allowDefaultTemplateParams)
{
	InstallTimer const installTimer("ClientCreatureObjectTemplate::install");

	SharedCreatureObjectTemplate::install(allowDefaultTemplateParams);
	// replace the shared create function with ours
	IGNORE_RETURN(ObjectTemplateList::assignBinding(SharedCreatureObjectTemplate_tag, create));
}	// ClientCreatureObjectTemplate::install

/**
 * Creates a ClientCreatureObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ClientCreatureObjectTemplate::create(const std::string & filename)
{
	return new ClientCreatureObjectTemplate(filename);
}	// ClientCreatureObjectTemplate::create

/**
 * Creates a new object from this template.
 *
 * @return the object
 */
Object * ClientCreatureObjectTemplate::createObject(void) const
{
	//-- Create the object.
	CreatureObject *const object =  new CreatureObject(this);

	//-- Add the animation message callback.
	GameAnimationMessageCallback::addCallback(*object);

	return object;
}	// ClientCreatureObjectTemplate::createObject
