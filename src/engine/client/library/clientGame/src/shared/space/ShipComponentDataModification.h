//======================================================================
//
// ShipComponentDataModification.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ShipComponentDataModification_H
#define INCLUDED_ShipComponentDataModification_H

//======================================================================

#include "clientGame/ClientShipComponentData.h"

//----------------------------------------------------------------------

class ShipComponentDataModification : public ClientShipComponentData
{
public:

	explicit ShipComponentDataModification (ShipComponentDescriptor const & shipComponentDescriptor);
	virtual ~ShipComponentDataModification ();

	virtual bool readDataFromShip      (int chassisSlot, ShipObject const & ship);

	virtual void printDebugString      (Unicode::String & result, Unicode::String const & padding) const;

	virtual void getAttributes         (stdvector<std::pair<std::string, Unicode::String> >::fwd & data) const;

private:
	ShipComponentDataModification (const ShipComponentDataModification & rhs);
	ShipComponentDataModification & operator= (const ShipComponentDataModification & rhs);

public:

	//-- these members are public because this instances of this class are short-lived temporary objects,
	//-- to simplify the conversion from the ShipObject & shipt representation of components.
};

//======================================================================

#endif
