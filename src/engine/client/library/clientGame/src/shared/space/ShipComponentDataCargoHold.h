//======================================================================
//
// ShipComponentDataCargoHold.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ShipComponentDataCargoHold_H
#define INCLUDED_ShipComponentDataCargoHold_H

//======================================================================

#include "clientGame/ClientShipComponentData.h"

//----------------------------------------------------------------------

class ShipComponentDataCargoHold : public ClientShipComponentData
{
public:

	explicit ShipComponentDataCargoHold (ShipComponentDescriptor const & shipComponentDescriptor);
	virtual ~ShipComponentDataCargoHold ();

	virtual bool readDataFromShip      (int chassisSlot, ShipObject const & ship);

	virtual void printDebugString      (Unicode::String & result, Unicode::String const & padding) const;

	virtual void getAttributes         (stdvector<std::pair<std::string, Unicode::String> >::fwd & data) const;

private:
	ShipComponentDataCargoHold (const ShipComponentDataCargoHold & rhs);
	ShipComponentDataCargoHold & operator= (const ShipComponentDataCargoHold & rhs);

public:

	//-- these members are public because this instances of this class are short-lived temporary objects,
	//-- to simplify the conversion from the ShipObject & shipt representation of components.

	int m_cargoHoldContentsMaximum;
	int m_cargoHoldContentsCurrent;

};

//======================================================================

#endif
