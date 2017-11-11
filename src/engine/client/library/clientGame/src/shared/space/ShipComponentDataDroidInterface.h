//======================================================================
//
// ShipComponentDataDroidInterface.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ShipComponentDataDroidInterface_H
#define INCLUDED_ShipComponentDataDroidInterface_H

//======================================================================

#include "clientGame/ClientShipComponentData.h"

//----------------------------------------------------------------------

class ShipComponentDataDroidInterface : public ClientShipComponentData
{
public:

	explicit ShipComponentDataDroidInterface (ShipComponentDescriptor const & shipComponentDescriptor);
	virtual ~ShipComponentDataDroidInterface ();

	virtual bool readDataFromShip      (int chassisSlot, ShipObject const & ship);

	virtual void printDebugString      (Unicode::String & result, Unicode::String const & padding) const;

	virtual void getAttributes         (stdvector<std::pair<std::string, Unicode::String> >::fwd & data) const;

private:
	ShipComponentDataDroidInterface (const ShipComponentDataDroidInterface & rhs);
	ShipComponentDataDroidInterface & operator= (const ShipComponentDataDroidInterface & rhs);

public:

	//-- these members are public because this instances of this class are short-lived temporary objects,
	//-- to simplify the conversion from the ShipObject & shipt representation of components.

	float m_droidInterfaceCommandSpeed;

};

//======================================================================

#endif
