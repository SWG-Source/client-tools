//===================================================================
//
// BuildingObject.h
// copyright 2001, Sony Online Entertainment
//
//===================================================================

#ifndef INCLUDED_BuildingObject_H
#define INCLUDED_BuildingObject_H

//===================================================================

class SharedBuildingObjectTemplate;

#include "clientGame/TangibleObject.h"

//===================================================================
/** 
  *  A BuildingObject is the base class for all large structures that players will be placing.
  */

class BuildingObject : public TangibleObject
{
	friend class ClientBuildingObjectTemplate;

public:

	explicit BuildingObject (const SharedBuildingObjectTemplate* newTemplate);
	virtual ~BuildingObject ();

	virtual void addToWorld();
	virtual void removeFromWorld ();
	virtual InteriorLayoutReaderWriter const * getInteriorLayout() const;

private:

 	BuildingObject ();
	BuildingObject (const BuildingObject& rhs);
	BuildingObject&	operator= (const BuildingObject& rhs);
};

//===================================================================

#endif
