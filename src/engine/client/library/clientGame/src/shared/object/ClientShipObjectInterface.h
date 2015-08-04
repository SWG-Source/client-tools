// ======================================================================
//
// ClientShipObjectInterface.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_ClientShipObjectInterface_H
#define INCLUDED_ClientShipObjectInterface_H

// ======================================================================

#include "sharedGame/ShipObjectInterface.h"

class ShipObject;

// ======================================================================

class ClientShipObjectInterface : public ShipObjectInterface
{
public:

	ClientShipObjectInterface(ShipObject const * shipObject);
	virtual ~ClientShipObjectInterface();

	virtual float getMaximumSpeed() const;
	virtual float getSpeedAcceleration() const;
	virtual float getSpeedDeceleration() const;
	virtual float getSlideDampener() const;
	virtual float getMaximumYaw() const;
	virtual float getYawAcceleration() const;
	virtual float getMaximumPitch() const;
	virtual float getPitchAcceleration() const;
	virtual float getMaximumRoll() const;
	virtual float getRollAcceleration() const;

protected:

	ShipObject const * const m_shipObject;

private:

	ClientShipObjectInterface();
	ClientShipObjectInterface(ClientShipObjectInterface const &);
	ClientShipObjectInterface & operator=(ClientShipObjectInterface const &);
};

// ======================================================================

#endif
