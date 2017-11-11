// ======================================================================
//
// ShipStation.h
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ShipStation_H
#define INCLUDED_ShipStation_H

// ======================================================================

namespace ShipStation
{
	enum Type
	{
		ShipStation_None = 0,
		ShipStation_Pilot = 1,
		ShipStation_Droid = 2,
		ShipStation_Operations = 3,
		ShipStation_Gunner_First = 4,
		ShipStation_Gunner_0 = ShipStation_Gunner_First,
		ShipStation_Gunner_1,
		ShipStation_Gunner_2,
		ShipStation_Gunner_3,
		ShipStation_Gunner_4,
		ShipStation_Gunner_5,
		ShipStation_Gunner_6,
		ShipStation_Gunner_7,
		ShipStation_Gunner_Last = ShipStation_Gunner_7
	};
	
	// ----------------------------------------------------------------------
	
	inline bool isGunnerStation(int shipStation)
	{
		return shipStation >= ShipStation_Gunner_First && shipStation <= ShipStation_Gunner_Last;
	}
	
	// ----------------------------------------------------------------------
	
	inline int getWeaponIndexForGunnerStation(int shipStation)
	{
		return (shipStation >= ShipStation_Gunner_First && shipStation <= ShipStation_Gunner_Last) ? shipStation-ShipStation_Gunner_First : -1;
	}
	
}
// ======================================================================

#endif // INCLUDED_ShipStation_H
