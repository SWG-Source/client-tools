//======================================================================
//
// ShipComponentPowerEffectsManagerClient.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ShipComponentPowerEffectsManagerClient_H
#define INCLUDED_ShipComponentPowerEffectsManagerClient_H

//======================================================================

class ShipObject;

//----------------------------------------------------------------------

class ShipComponentPowerEffectsManagerClient
{
public:

	static void install();
	static void remove();
	static void playEnableEffect(ShipObject const & ship, int chassisSlot);
	static void playDisableEffect(ShipObject const & ship, int chassisSlot);

private:
	                         ShipComponentPowerEffectsManagerClient ();
	                         ShipComponentPowerEffectsManagerClient (const ShipComponentPowerEffectsManagerClient & rhs);
	ShipComponentPowerEffectsManagerClient & operator= (const ShipComponentPowerEffectsManagerClient & rhs);
};

//======================================================================

#endif
