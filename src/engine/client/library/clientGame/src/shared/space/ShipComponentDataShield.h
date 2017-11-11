//======================================================================
//
// ShipComponentDataShield.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ShipComponentDataShield_H
#define INCLUDED_ShipComponentDataShield_H

//======================================================================

#include "clientGame/ClientShipComponentData.h"

//----------------------------------------------------------------------

class ShipComponentDataShield : public ClientShipComponentData
{
public:

	explicit ShipComponentDataShield (ShipComponentDescriptor const & shipComponentDescriptor);
	virtual ~ShipComponentDataShield ();

	virtual bool readDataFromShip      (int chassisSlot, ShipObject const & ship);

	virtual void printDebugString      (Unicode::String & result, Unicode::String const & padding) const;

	virtual void getAttributes         (stdvector<std::pair<std::string, Unicode::String> >::fwd & data) const;

private:
	ShipComponentDataShield (const ShipComponentDataShield & rhs);
	ShipComponentDataShield & operator= (const ShipComponentDataShield & rhs);

public:

	//-- these members are public because this instances of this class are short-lived temporary objects,
	//-- to simplify the conversion from the ShipObject & shipt representation of components.

	float m_shieldHitpointsFrontCurrent;
	float m_shieldHitpointsFrontMaximum;
	float m_shieldHitpointsBackCurrent;
	float m_shieldHitpointsBackMaximum;
	float m_shieldRechargeRate;
};

//======================================================================

#endif
