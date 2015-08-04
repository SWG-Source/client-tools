//========================================================================
//
// FactoryObject.cpp
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/FactoryObject.h"
#include "clientGame/ClientFactoryObjectTemplate.h"
#include "clientGame/ContainerInterface.h"
#include "sharedObject/VolumeContainer.h"


//-----------------------------------------------------------------------

FactoryObject::FactoryObject(const SharedFactoryObjectTemplate* newTemplate) :
	TangibleObject(newTemplate)
{
}	


//-----------------------------------------------------------------------

FactoryObject::~FactoryObject()
{
}

//-----------------------------------------------------------------------

/**
 * Returns the name of our content's template.
 *
 * @return the template name
 */
const char * FactoryObject::getContainedTemplateName() const
{
	if (getCount() > 0)
	{
		const VolumeContainer * container = ContainerInterface::getVolumeContainer(*this);
		const CachedNetworkId & id = *(container->begin());
		const Object * obj = id.getObject();
		if (obj != NULL && obj->asClientObject() != NULL)
			return obj->asClientObject()->getTemplateName();
	}
	return NULL;
}	// FactoryObject::getContainedTemplateName

//-----------------------------------------------------------------------

/**
 * Returns our content's template.
 *
 * @return the template
 */
const ObjectTemplate * FactoryObject::getContainedObjectTemplate() const
{
	if (getCount() > 0)
	{
		const VolumeContainer * container = ContainerInterface::getVolumeContainer(*this);
		const CachedNetworkId & id = *(container->begin());
		const Object * obj = id.getObject();
		if (obj != NULL && obj->asClientObject() != NULL)
			return obj->asClientObject()->getObjectTemplate();
	}
	return NULL;
}	// FactoryObject::getContainedObjectTemplate

//-----------------------------------------------------------------------

/**
 * Returns our contents.
 *
 * @return the contents object
 */
const ClientObject * FactoryObject::getContainedObject() const
{
	if (getCount() > 0)
	{
		const VolumeContainer * container = ContainerInterface::getVolumeContainer(*this);
		const CachedNetworkId & id = *(container->begin());
		const Object * obj = id.getObject();
		if (obj != NULL && obj->asClientObject() != NULL)
			return obj->asClientObject();
	}
	return NULL;
}	// FactoryObject::getContainedObject


// ======================================================================
