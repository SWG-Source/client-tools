//======================================================================
//
// ClientSharedObjectTemplateInterface.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientSharedObjectTemplateInterface.h"

#include "clientGame/ClientObject.h"
#include "clientGame/ShipObject.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedObject/Object.h"

//======================================================================

ClientSharedObjectTemplateInterface::ClientSharedObjectTemplateInterface()
: SharedObjectTemplateInterface()
{
}

//----------------------------------------------------------------------

ClientSharedObjectTemplateInterface::~ClientSharedObjectTemplateInterface()
{
}

//----------------------------------------------------------------------

SharedObjectTemplate const * ClientSharedObjectTemplateInterface::getSharedTemplate(Object const * const object) const
{
	return safe_cast<SharedObjectTemplate const *>(object->getObjectTemplate());
}

//----------------------------------------------------------------------

bool ClientSharedObjectTemplateInterface::isShipObject(Object const * const object) const
{
	ClientObject const * const clientObject = (object != 0) ? object->asClientObject() : 0;
	ShipObject const * const shipObject = (clientObject != 0) ? clientObject->asShipObject() : 0;
	return shipObject != 0;
}

//----------------------------------------------------------------------

uint32 ClientSharedObjectTemplateInterface::getShipChassisType(Object const * const object) const
{
	ClientObject const * const clientObject = (object != 0) ? object->asClientObject() : 0;
	ShipObject const * const shipObject = (clientObject != 0) ? clientObject->asShipObject() : 0;
	return (shipObject != 0) ? shipObject->getChassisType() : 0;
}

//----------------------------------------------------------------------

uint32 ClientSharedObjectTemplateInterface::getShipComponentCrc(Object const * const object, int chassisSlot) const
{
	ClientObject const * const clientObject = (object != 0) ? object->asClientObject() : 0;
	ShipObject const * const shipObject = (clientObject != 0) ? clientObject->asShipObject() : 0;
	return (shipObject != 0) ? shipObject->getComponentCrc(chassisSlot) : 0;
}

//======================================================================
