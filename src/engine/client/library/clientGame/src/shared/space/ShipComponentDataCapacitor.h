//======================================================================
//
// ShipComponentDataCapacitor.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ShipComponentDataCapacitor_H
#define INCLUDED_ShipComponentDataCapacitor_H

//======================================================================

#include "clientGame/ClientShipComponentData.h"

//----------------------------------------------------------------------

class ShipComponentDataCapacitor : public ClientShipComponentData
{
public:

	explicit ShipComponentDataCapacitor (ShipComponentDescriptor const & shipComponentDescriptor);
	virtual ~ShipComponentDataCapacitor ();

	virtual bool readDataFromShip      (int chassisSlot, ShipObject const & ship);

	virtual void printDebugString      (Unicode::String & result, Unicode::String const & padding) const;

	virtual void getAttributes         (stdvector<std::pair<std::string, Unicode::String> >::fwd & data) const;

private:
	ShipComponentDataCapacitor (const ShipComponentDataCapacitor & rhs);
	ShipComponentDataCapacitor & operator= (const ShipComponentDataCapacitor & rhs);

public:

	//-- these members are public because this instances of this class are short-lived temporary objects,
	//-- to simplify the conversion from the ShipObject & shipt representation of components.

	float m_capacitorEnergyCurrent;
	float m_capacitorEnergyMaximum;
	float m_capacitorEnergyRechargeRate;

};

//======================================================================

#endif
