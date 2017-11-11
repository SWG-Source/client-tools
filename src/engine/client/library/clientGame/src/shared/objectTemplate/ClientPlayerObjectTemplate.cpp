//========================================================================
//
// ClientPlayerObjectTemplate.cpp - A wrapper around SharedPlayerObjectTemplate 
// to create IntangibleObjects.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientPlayerObjectTemplate.h"

#include "clientGame/PlayerObject.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedObject/ObjectTemplateList.h"

//----------------------------------------------------------------------

/**
 * Class constructor.
 */
ClientPlayerObjectTemplate::ClientPlayerObjectTemplate(const std::string & filename) :
	SharedPlayerObjectTemplate(filename)
{
}

//----------------------------------------------------------------------

/**
 * Class destructor.
 */
ClientPlayerObjectTemplate::~ClientPlayerObjectTemplate()
{
}

//----------------------------------------------------------------------

/**
 * Maps the SharedIntangibleObjectTemplate tag to use our create function.
 */
void ClientPlayerObjectTemplate::install(bool allowDefaultTemplateParams)
{
	InstallTimer const installTimer("ClientPlayerObjectTemplate::install");

	SharedPlayerObjectTemplate::install(allowDefaultTemplateParams);
	// replace the shared create function with ours
	ObjectTemplateList::assignBinding(SharedPlayerObjectTemplate_tag, create);
}

//----------------------------------------------------------------------

/**
 * Creates a ClientPlayerObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ClientPlayerObjectTemplate::create(const std::string & filename)
{
	return new ClientPlayerObjectTemplate(filename);
}

//----------------------------------------------------------------------

/**
 * Creates a new object from this template.
 *
 * @return the object
 */
Object * ClientPlayerObjectTemplate::createObject() const
{
	return new PlayerObject(this);
}

//----------------------------------------------------------------------
