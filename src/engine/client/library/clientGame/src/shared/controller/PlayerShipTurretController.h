// ======================================================================
//
// PlayerShipTurretController.h
//
// Copyright 2004 Sony Online Entertainment
// 
// ======================================================================

#ifndef INCLUDED_PlayerShipTurretController_H
#define INCLUDED_PlayerShipTurretController_H

// ======================================================================

#include "clientGame/ClientController.h"

// ======================================================================

class PlayerShipTurretController: public ClientController
{
public:
	static void install();

public:
	explicit PlayerShipTurretController(Object *owner);
	virtual ~PlayerShipTurretController();

protected:
	virtual float realAlter(float elapsedTime);

private:
	PlayerShipTurretController();
	PlayerShipTurretController(PlayerShipTurretController const &);
	PlayerShipTurretController & operator=(PlayerShipTurretController const &);

	float m_yaw;
	float m_pitch;
};

// ======================================================================

#endif // INCLUDED_PlayerShipTurretController_H
