// ======================================================================
//
// ClientShipObjectInterface.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientShipObjectInterface.h"

#include "clientGame/ShipObject.h"

// ======================================================================

ClientShipObjectInterface::ClientShipObjectInterface(ShipObject const * const shipObject) :
	ShipObjectInterface(),
	m_shipObject(shipObject)
{
}

// ----------------------------------------------------------------------

ClientShipObjectInterface::~ClientShipObjectInterface()
{
}

// ----------------------------------------------------------------------

float ClientShipObjectInterface::getMaximumSpeed() const
{
	return m_shipObject->getShipActualSpeedMaximum();
}

// ----------------------------------------------------------------------

float ClientShipObjectInterface::getSpeedAcceleration() const
{
	return m_shipObject->getShipActualAccelerationRate();
}

// ----------------------------------------------------------------------

float ClientShipObjectInterface::getSpeedDeceleration() const
{
	return m_shipObject->getShipActualDecelerationRate();
}

// ----------------------------------------------------------------------

float ClientShipObjectInterface::getSlideDampener() const
{
	return m_shipObject->getSlideDampener();
}

// ----------------------------------------------------------------------

float ClientShipObjectInterface::getMaximumYaw() const
{
	return m_shipObject->getShipActualYawRateMaximum();
}

// ----------------------------------------------------------------------

float ClientShipObjectInterface::getYawAcceleration() const
{
	return m_shipObject->getShipActualYawAccelerationRate();
}

// ----------------------------------------------------------------------

float ClientShipObjectInterface::getMaximumPitch() const
{
	return m_shipObject->getShipActualPitchRateMaximum();
}

// ----------------------------------------------------------------------

float ClientShipObjectInterface::getPitchAcceleration() const
{
	return m_shipObject->getShipActualPitchAccelerationRate();
}

// ----------------------------------------------------------------------

float ClientShipObjectInterface::getMaximumRoll() const
{
	return m_shipObject->getShipActualRollRateMaximum();
}

// ----------------------------------------------------------------------

float ClientShipObjectInterface::getRollAcceleration() const
{
	return m_shipObject->getShipActualRollAccelerationRate();
}

// ======================================================================
