//========================================================================
//
// ClientManufactureSchematicObjectTemplate.cpp - A wrapper around SharedManufactureSchematicObjectTemplate 
// to create TangibleObjects.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientManufactureSchematicObjectTemplate.h"

#include "clientGame/ManufactureSchematicObject.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedObject/ObjectTemplateList.h"

//----------------------------------------------------------------------

/**
 * Class constructor.
 */
ClientManufactureSchematicObjectTemplate::ClientManufactureSchematicObjectTemplate(const std::string & filename) :
	SharedManufactureSchematicObjectTemplate (filename)
{
}

//----------------------------------------------------------------------

/**
 * Class destructor.
 */
ClientManufactureSchematicObjectTemplate::~ClientManufactureSchematicObjectTemplate()
{
}

//----------------------------------------------------------------------

/**
 * Maps the SharedManufactureSchematicObjectTemplate tag to use our create function.
 */
void ClientManufactureSchematicObjectTemplate::install(bool allowDefaultTemplateParams)
{
	InstallTimer const installTimer("ClientManufactureSchematicObjectTemplate::install");

	SharedManufactureSchematicObjectTemplate::install(allowDefaultTemplateParams);
	// replace the shared create function with ours
	ObjectTemplateList::assignBinding(SharedManufactureSchematicObjectTemplate_tag, create);
}

//----------------------------------------------------------------------

/**
 * Creates a ClientManufactureSchematicObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ClientManufactureSchematicObjectTemplate::create(const std::string & filename)
{
	return new ClientManufactureSchematicObjectTemplate(filename);
}

//----------------------------------------------------------------------

/**
 * Creates a new object from this template.
 *
 * @return the object
 */
Object * ClientManufactureSchematicObjectTemplate::createObject() const
{
	return new ManufactureSchematicObject(this);
}

//----------------------------------------------------------------------
