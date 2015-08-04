// ======================================================================
//
// AutoCommManager.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_AutoCommManager_H
#define INCLUDED_AutoCommManager_H

// ======================================================================

class ShipObject;

// ======================================================================

/**
 * A singleton to manage the AutoComm system.  This system activates the 
 * comm automatically if you get within a certain distance of certain 
 * hardpoints.
 */
class AutoCommManager
{
  public:
	  static void registerShip(ShipObject const & ship);
	  static void removeShip(ShipObject const & ship);

	  static void update(float const elapsedTime);
};

// ======================================================================

#endif
