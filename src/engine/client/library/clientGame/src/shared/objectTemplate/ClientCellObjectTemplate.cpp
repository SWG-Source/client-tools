//========================================================================
//
// ClientCellObjectTemplate.cpp - A wrapper around SharedCellObjectTemplate 
// to create CellObjects.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientCellObjectTemplate.h"

#include "clientGame/CellObject.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedObject/ObjectTemplateList.h"

/**
 * Class constructor.
 */
ClientCellObjectTemplate::ClientCellObjectTemplate(const std::string & filename) :
	SharedCellObjectTemplate(filename)
{
}	// ClientCellObjectTemplate::ClientCellObjectTemplate

/**
 * Class destructor.
 */
ClientCellObjectTemplate::~ClientCellObjectTemplate()
{
}	// ClientCellObjectTemplate::~ClientCellObjectTemplate

/**
 * Maps the SharedCellObjectTemplate tag to use our create function.
 */
void ClientCellObjectTemplate::install(bool allowDefaultTemplateParams)
{
	InstallTimer const installTimer("ClientCellObjectTemplate::install");

	SharedCellObjectTemplate::install(allowDefaultTemplateParams);
	// replace the shared create function with ours
	ObjectTemplateList::assignBinding(SharedCellObjectTemplate_tag, create);
}	// ClientCellObjectTemplate::install

/**
 * Creates a ClientCellObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ClientCellObjectTemplate::create(const std::string & filename)
{
	return new ClientCellObjectTemplate(filename);
}	// ClientCellObjectTemplate::create

/**
 * Creates a new object from this template.
 *
 * @return the object
 */
Object * ClientCellObjectTemplate::createObject(void) const
{
	return new CellObject(this);
}	// ClientCellObjectTemplate::createObject
