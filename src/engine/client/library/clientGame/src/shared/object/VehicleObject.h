//===================================================================
//
// VehicleObject.h
// copyright 2001, Sony Online Entertainment
//
//===================================================================

#ifndef INCLUDED_VehicleObject_H
#define INCLUDED_VehicleObject_H

//===================================================================

class SharedVehicleObjectTemplate;

#include "clientGame/TangibleObject.h"

//===================================================================

class VehicleObject : public TangibleObject
{
public:

	explicit VehicleObject (const SharedVehicleObjectTemplate* newTemplate);
	virtual ~VehicleObject ();

private:

 	VehicleObject ();
	VehicleObject (const VehicleObject& rhs);
	VehicleObject&	operator= (const VehicleObject& rhs);

private:
};

//===================================================================

#endif
