//========================================================================
//
// ClientInstallationObjectTemplate.cpp - A wrapper around SharedInstallationObjectTemplate 
// to create TangibleObjects.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientInstallationObjectTemplate.h"

#include "clientGame/ClientLotManagerNotification.h"
#include "clientGame/ClientStructureFootprintObject.h"
#include "clientGame/Game.h"
#include "clientGame/InstallationObject.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedObject/ObjectTemplateList.h"

/**
 * Class constructor.
 */
ClientInstallationObjectTemplate::ClientInstallationObjectTemplate(const std::string & filename) :
	SharedInstallationObjectTemplate(filename)
{
}	// ClientInstallationObjectTemplate::ClientInstallationObjectTemplate

/**
 * Class destructor.
 */
ClientInstallationObjectTemplate::~ClientInstallationObjectTemplate()
{
}	// ClientInstallationObjectTemplate::~ClientInstallationObjectTemplate

/**
 * Maps the SharedInstallationObjectTemplate tag to use our create function.
 */
void ClientInstallationObjectTemplate::install(bool allowDefaultTemplateParams)
{
	InstallTimer const installTimer("ClientInstallationObjectTemplate::install");

	SharedInstallationObjectTemplate::install(allowDefaultTemplateParams);
	// replace the shared create function with ours
	ObjectTemplateList::assignBinding(SharedInstallationObjectTemplate_tag, create);
}	// ClientInstallationObjectTemplate::install

/**
 * Creates a ClientInstallationObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ClientInstallationObjectTemplate::create(const std::string & filename)
{
	return new ClientInstallationObjectTemplate(filename);
}	// ClientInstallationObjectTemplate::create

/**
 * Creates a new object from this template.
 *
 * @return the object
 */
Object * ClientInstallationObjectTemplate::createObject(void) const
{
	InstallationObject* const object = new InstallationObject(this);

	if (getStructureFootprint ())
	{
		DEBUG_WARNING (getNoBuildRadius () > 0.f, ("ClientInstallationObjectTemplate [%s] non-zero (%1.2f) noBuildRadius found on object with structure footprint", getName (), getNoBuildRadius ()));

		object->addNotification (ClientLotManagerNotification::getInstance ());

		if (getUseStructureFootprintOutline ())
			object->addChildObject_o (new ClientStructureFootprintObject (*getStructureFootprint ()));
	}

	return object;
}	// ClientInstallationObjectTemplate::createObject
