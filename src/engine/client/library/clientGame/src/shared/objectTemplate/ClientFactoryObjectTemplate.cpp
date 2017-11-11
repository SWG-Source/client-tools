//========================================================================
//
// ClientFactoryObjectTemplate.cpp - A wrapper around SharedFactoryObjectTemplate 
// to create IntangibleObjects.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientFactoryObjectTemplate.h"

#include "clientGame/FactoryObject.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedObject/ObjectTemplateList.h"

//----------------------------------------------------------------------

/**
 * Class constructor.
 */
ClientFactoryObjectTemplate::ClientFactoryObjectTemplate(const std::string & filename) :
	SharedFactoryObjectTemplate(filename)
{
}

//----------------------------------------------------------------------

/**
 * Class destructor.
 */
ClientFactoryObjectTemplate::~ClientFactoryObjectTemplate()
{
}

//----------------------------------------------------------------------

/**
 * Maps the SharedFactoryObjectTemplate tag to use our create function.
 */
void ClientFactoryObjectTemplate::install(bool allowDefaultTemplateParams)
{
	InstallTimer const installTimer("ClientFactoryObjectTemplate::install");

	SharedFactoryObjectTemplate::install(allowDefaultTemplateParams);
	// replace the shared create function with ours
	ObjectTemplateList::assignBinding(SharedFactoryObjectTemplate_tag, create);
}

//----------------------------------------------------------------------

/**
 * Creates a ClientFactoryObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ClientFactoryObjectTemplate::create(const std::string & filename)
{
	return new ClientFactoryObjectTemplate(filename);
}

//----------------------------------------------------------------------

/**
 * Creates a new object from this template.
 *
 * @return the object
 */
Object * ClientFactoryObjectTemplate::createObject() const
{
	return new FactoryObject(this);
}

//----------------------------------------------------------------------
