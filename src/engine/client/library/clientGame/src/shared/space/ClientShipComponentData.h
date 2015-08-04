//======================================================================
//
// ClientShipComponentData.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ClientShipComponentData_H
#define INCLUDED_ClientShipComponentData_H

//======================================================================

#include "sharedGame/ShipComponentData.h"

//----------------------------------------------------------------------

class ClientShipComponentData : public ShipComponentData
{
public:

	explicit ClientShipComponentData (ShipComponentDescriptor const & shipComponentDescriptor);
	virtual ~ClientShipComponentData ();

	virtual bool readDataFromShip      (int chassisSlot, ShipObject const & ship);
	virtual void writeDataToShip       (int chassisSlot, ShipObject & ship) const;
	virtual bool readDataFromComponent (TangibleObject const & component);
	virtual void writeDataToComponent  (TangibleObject & component) const;

	virtual void printDebugString      (Unicode::String & result, Unicode::String const & padding) const;

	virtual void getAttributes         (stdvector<std::pair<std::string, Unicode::String> >::fwd & data) const;

private:
	ClientShipComponentData (const ClientShipComponentData & rhs);
	ClientShipComponentData & operator= (const ClientShipComponentData & rhs);
};

//======================================================================

#endif
