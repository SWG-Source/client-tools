//======================================================================
//
// CuiDamageManager.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiDamageManager_H
#define INCLUDED_CuiDamageManager_H

//======================================================================

class CreatureObject;
class NetworkId;
class ShipDamageMessage;
class ShipObject;
class TangibleObject;
class Vector;
class VectorArgb;

//----------------------------------------------------------------------

class CuiDamageManager
{
public:
	struct Messages
	{
		struct ShipDamage 
		{
			typedef ShipDamageMessage Payload;
		};
	};

	static void install ();
	static void remove  ();

	static void update  (float deltaTime);

	static void findHamColor         (VectorArgb & color, int h, int a, int m);
	static void handleShipDamage     (ShipDamageMessage const & shipDamageMsg);
};

//======================================================================

#endif
