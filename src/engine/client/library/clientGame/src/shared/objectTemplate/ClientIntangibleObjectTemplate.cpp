//========================================================================
//
// ClientIntangibleObjectTemplate.cpp - A wrapper around SharedIntangibleObjectTemplate 
// to create IntangibleObjects.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientIntangibleObjectTemplate.h"

#include "clientGame/IntangibleObject.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedObject/ObjectTemplateList.h"

//----------------------------------------------------------------------

/**
 * Class constructor.
 */
ClientIntangibleObjectTemplate::ClientIntangibleObjectTemplate(const std::string & filename) :
	SharedIntangibleObjectTemplate(filename)
{
}

//----------------------------------------------------------------------

/**
 * Class destructor.
 */
ClientIntangibleObjectTemplate::~ClientIntangibleObjectTemplate()
{
}

//----------------------------------------------------------------------

/**
 * Maps the SharedIntangibleObjectTemplate tag to use our create function.
 */
void ClientIntangibleObjectTemplate::install(bool allowDefaultTemplateParams)
{
	InstallTimer const installTimer("ClientIntangibleObjectTemplate::install");

	SharedIntangibleObjectTemplate::install(allowDefaultTemplateParams);
	// replace the shared create function with ours
	ObjectTemplateList::assignBinding(SharedIntangibleObjectTemplate_tag, create);
}

//----------------------------------------------------------------------

/**
 * Creates a ClientIntangibleObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ClientIntangibleObjectTemplate::create(const std::string & filename)
{
	return new ClientIntangibleObjectTemplate(filename);
}

//----------------------------------------------------------------------

/**
 * Creates a new object from this template.
 *
 * @return the object
 */
Object * ClientIntangibleObjectTemplate::createObject() const
{
	return new IntangibleObject(this);
}

//----------------------------------------------------------------------
