//======================================================================
//
// ShipComponentDataBridge.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ShipComponentDataBridge_H
#define INCLUDED_ShipComponentDataBridge_H

//======================================================================

#include "clientGame/ClientShipComponentData.h"

//----------------------------------------------------------------------

class ShipComponentDataBridge : public ClientShipComponentData
{
public:

	explicit ShipComponentDataBridge (ShipComponentDescriptor const & shipComponentDescriptor);
	virtual ~ShipComponentDataBridge ();

	virtual bool readDataFromShip      (int chassisSlot, ShipObject const & ship);

	virtual void printDebugString      (Unicode::String & result, Unicode::String const & padding) const;

	virtual void getAttributes         (stdvector<std::pair<std::string, Unicode::String> >::fwd & data) const;

private:
	ShipComponentDataBridge (const ShipComponentDataBridge & rhs);
	ShipComponentDataBridge & operator= (const ShipComponentDataBridge & rhs);

public:

	//-- these members are public because this instances of this class are short-lived temporary objects,
	//-- to simplify the conversion from the ShipObject & shipt representation of components.
};

//======================================================================

#endif
