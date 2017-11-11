//======================================================================
//
// ShipComponentDataBooster.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ShipComponentDataBooster_H
#define INCLUDED_ShipComponentDataBooster_H

//======================================================================

#include "clientGame/ClientShipComponentData.h"

//----------------------------------------------------------------------

class ShipComponentDataBooster : public ClientShipComponentData
{
public:

	explicit ShipComponentDataBooster (ShipComponentDescriptor const & shipComponentDescriptor);
	virtual ~ShipComponentDataBooster ();

	virtual bool readDataFromShip      (int chassisSlot, ShipObject const & ship);

	virtual void printDebugString      (Unicode::String & result, Unicode::String const & padding) const;

	virtual void getAttributes         (stdvector<std::pair<std::string, Unicode::String> >::fwd & data) const;

private:
	ShipComponentDataBooster (const ShipComponentDataBooster & rhs);
	ShipComponentDataBooster & operator= (const ShipComponentDataBooster & rhs);

public:

	//-- these members are public because this instances of this class are short-lived temporary objects,
	//-- to simplify the conversion from the ShipObject & shipt representation of components.

	float m_boosterEnergyCurrent;
	float m_boosterEnergyMaximum;
	float m_boosterEnergyRechargeRate;
	float m_boosterEnergyConsumptionRate;
	float m_boosterAcceleration;
	float m_boosterSpeedMaximum;
};

//======================================================================

#endif
