//======================================================================
//
// ShipHitEffectsManagerClient.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ShipHitEffectsManagerClient_H
#define INCLUDED_ShipHitEffectsManagerClient_H

//======================================================================

class MessageQueueShipHit;
class ShipObject;

//----------------------------------------------------------------------

class ShipHitEffectsManagerClient
{
public:

	static void install();
	static void remove();
	static void handleShipHit(ShipObject & ship, MessageQueueShipHit const & msg);

private:
	                         ShipHitEffectsManagerClient ();
	                         ShipHitEffectsManagerClient (const ShipHitEffectsManagerClient & rhs);
	ShipHitEffectsManagerClient & operator= (const ShipHitEffectsManagerClient & rhs);
};

//======================================================================

#endif
