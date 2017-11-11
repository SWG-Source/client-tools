//========================================================================
//
// ClientVehicleObjectTemplate.cpp - A wrapper around SharedVehicleObjectTemplate 
// to create TangibleObjects.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientVehicleObjectTemplate.h"

#include "clientGame/VehicleObject.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedObject/ObjectTemplateList.h"

/**
 * Class constructor.
 */
ClientVehicleObjectTemplate::ClientVehicleObjectTemplate(const std::string & filename) :
	SharedVehicleObjectTemplate(filename)
{
}	// ClientVehicleObjectTemplate::ClientVehicleObjectTemplate

/**
 * Class destructor.
 */
ClientVehicleObjectTemplate::~ClientVehicleObjectTemplate()
{
}	// ClientVehicleObjectTemplate::~ClientVehicleObjectTemplate

/**
 * Maps the SharedVehicleObjectTemplate tag to use our create function.
 */
void ClientVehicleObjectTemplate::install(bool allowDefaultTemplateParams)
{
	InstallTimer const installTimer("ClientVehicleObjectTemplate::install");

	SharedVehicleObjectTemplate::install(allowDefaultTemplateParams);
	// replace the shared create function with ours
	ObjectTemplateList::assignBinding(SharedVehicleObjectTemplate_tag, create);
}	// ClientVehicleObjectTemplate::install

/**
 * Creates a ClientVehicleObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ClientVehicleObjectTemplate::create(const std::string & filename)
{
	return new ClientVehicleObjectTemplate(filename);
}	// ClientVehicleObjectTemplate::create

/**
 * Creates a new object from this template.
 *
 * @return the object
 */
Object * ClientVehicleObjectTemplate::createObject(void) const
{
	return new VehicleObject(this);
}	// ClientVehicleObjectTemplate::createObject
