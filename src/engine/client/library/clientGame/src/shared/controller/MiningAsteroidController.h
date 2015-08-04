//======================================================================
//
// MiningAsteroidController.h
// copyright (c) 2005 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_MiningAsteroidController_H
#define INCLUDED_MiningAsteroidController_H

//======================================================================

#include "clientGame/RemoteShipController.h"

//----------------------------------------------------------------------

class MiningAsteroidController : public RemoteShipController
{
public:

	explicit MiningAsteroidController(ShipObject * const owner);
	virtual ~MiningAsteroidController();

	
private:
	                         MiningAsteroidController ();
	                         MiningAsteroidController (const MiningAsteroidController & rhs);
	MiningAsteroidController & operator= (const MiningAsteroidController & rhs);

protected:

	virtual float realAlter(float elapsedTime);

private:
};

//======================================================================

#endif
