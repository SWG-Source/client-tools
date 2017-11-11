//===================================================================
//
// VehicleObject.cpp
// copyright 2001, Sony Online Entertainment
//
//===================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/VehicleObject.h"

#include "clientGame/ClientVehicleObjectTemplate.h"

//===================================================================

VehicleObject::VehicleObject(const SharedVehicleObjectTemplate* newTemplate) :
	TangibleObject(newTemplate)
{
}

//-------------------------------------------------------------------
	
VehicleObject::~VehicleObject()
{
}

//===================================================================
