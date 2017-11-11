//===================================================================
//
// ClientLotManagerNotification.cpp
// asommers
//
// copyright 2001, sony online entertainment
//
//===================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientLotManagerNotification.h"

#include "sharedGame/SharedTangibleObjectTemplate.h"
#include "sharedObject/Object.h"

//===================================================================
// STATIC PUBLIC ClientLotManagerNotification
//===================================================================

ClientLotManagerNotification ClientLotManagerNotification::ms_instance;

//-------------------------------------------------------------------

ClientLotManagerNotification& ClientLotManagerNotification::getInstance ()
{
	return ms_instance;
}

//===================================================================
// PUBLIC ClientLotManagerNotification
//===================================================================

ClientLotManagerNotification::ClientLotManagerNotification () :
	LotManagerNotification ()
{
}

//-------------------------------------------------------------------
	
ClientLotManagerNotification::~ClientLotManagerNotification ()
{
}

//-------------------------------------------------------------------

void ClientLotManagerNotification::addToWorld (Object& object) const
{
	const SharedTangibleObjectTemplate* const tangibleObjectTemplate = safe_cast<const SharedTangibleObjectTemplate*> (object.getObjectTemplate ());
	const StructureFootprint* const structureFootprint = tangibleObjectTemplate->getStructureFootprint ();
	NOT_NULL (structureFootprint);

	if (structureFootprint)
		LotManagerNotification::addToWorld (object, *structureFootprint);
}

//===================================================================

