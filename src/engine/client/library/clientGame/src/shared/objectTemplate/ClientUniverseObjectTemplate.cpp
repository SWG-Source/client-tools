//========================================================================
//
// ClientUniverseObjectTemplate.cpp - A wrapper around SharedUniverseObjectTemplate 
// to create UniverseObjects.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientUniverseObjectTemplate.h"

#include "clientGame/UniverseObject.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedObject/ObjectTemplateList.h"

//----------------------------------------------------------------------

/**
 * Class constructor.
 */
ClientUniverseObjectTemplate::ClientUniverseObjectTemplate(const std::string & filename) :
	SharedUniverseObjectTemplate(filename)
{
}

//----------------------------------------------------------------------

/**
 * Class destructor.
 */
ClientUniverseObjectTemplate::~ClientUniverseObjectTemplate()
{
}

//----------------------------------------------------------------------

/**
 * Maps the SharedUniverseObjectTemplate tag to use our create function.
 */
void ClientUniverseObjectTemplate::install(bool allowDefaultTemplateParams)
{
	InstallTimer const installTimer("ClientUniverseObjectTemplate::install");

	SharedUniverseObjectTemplate::install(allowDefaultTemplateParams);
	// replace the shared create function with ours
	ObjectTemplateList::assignBinding(SharedUniverseObjectTemplate_tag, create);
}

//----------------------------------------------------------------------

/**
 * Creates a ClientUniverseObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ClientUniverseObjectTemplate::create(const std::string & filename)
{
	return new ClientUniverseObjectTemplate(filename);
}

//----------------------------------------------------------------------

/**
 * Creates a new object from this template.
 *
 * @return the object
 */
Object * ClientUniverseObjectTemplate::createObject() const
{
	return new UniverseObject(this);
}

//----------------------------------------------------------------------
