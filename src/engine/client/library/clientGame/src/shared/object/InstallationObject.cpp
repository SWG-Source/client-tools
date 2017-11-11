//===================================================================
//
// InstallationObject.cpp
// copyright 2001, Sony Online Entertainment
//
//===================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/InstallationObject.h"

#include "clientAudio/Audio.h"
#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientDataFile.h"
#include "clientGame/ClientInstallationObjectTemplate.h"
#include "clientGame/ClientInstallationSynchronizedUi.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/Crc.h"

//===================================================================

InstallationObject::InstallationObject(const SharedInstallationObjectTemplate* newTemplate) :
	TangibleObject (newTemplate),
	m_activated(false),
	m_power(0),
	m_powerRate(0),
	m_turretObject (0)
{
	m_activated.setSourceObject    (this);

	addSharedVariable(m_activated);
	addSharedVariable(m_power);
	addSharedVariable(m_powerRate);

	//Installation objects have real time updated UI
	setSynchronizedUi (createSynchronizedUi ());
}

//-------------------------------------------------------------------

InstallationObject::~InstallationObject()
{
	// 	m_synchronizedUi deleted by superclass
}

//----------------------------------------------------------------------

void InstallationObject::Callbacks::InstallationActive::modified(InstallationObject& /*object*/, const bool& /*oldValue*/, const bool& /*newValue*/, bool) const
{
}

//----------------------------------------------------------------------

ClientSynchronizedUi * InstallationObject::createSynchronizedUi ()
{
	return new ClientInstallationSynchronizedUi(*this);
}

//----------------------------------------------------------------------

void InstallationObject::setHarvesterActive(Object & player, bool active)
{
static const uint32 activate = Crc::normalizeAndCalculate("harvesterActivate");
static const uint32 deactivate = Crc::normalizeAndCalculate("harvesterDeactivate");

	UNREF(player);

	uint32 commandHash;
	if (active)
		commandHash = activate;
	else
		commandHash = deactivate;
	// build params
//	char buf[64];
//	snprintf(buf, sizeof (buf), "%d %d %d ", ...);
//	Unicode::String params(Unicode::narrowToWide(buf));
	ClientCommandQueue::enqueueCommand(commandHash, getNetworkId(), Unicode::String());
}

//----------------------------------------------------------------------

void InstallationObject::listenToHarvester(Object & player, bool listen)
{
static const uint32 listenCmd = Crc::normalizeAndCalculate("synchronizedUiListen");
static const uint32 stopListenCmd = Crc::normalizeAndCalculate("synchronizedUiStopListening");

	UNREF(player);

	uint32 commandHash;
	if (listen)
		commandHash = listenCmd;
	else
		commandHash = stopListenCmd;
	ClientCommandQueue::enqueueCommand(commandHash, getNetworkId(), Unicode::String());
}

//----------------------------------------------------------------------

void InstallationObject::getHarvesterResourceData  (Object & player)
{
	UNREF(player);

	const uint32 commandHash = Crc::normalizeAndCalculate("harvesterGetResourceData");
	ClientCommandQueue::enqueueCommand(commandHash, getNetworkId(), Unicode::String());
}

//----------------------------------------------------------------------

void InstallationObject::selectHarvesterResource   (Object & player, const NetworkId & resourcePoolId)
{
	UNREF(player);

	const uint32 commandHash = Crc::normalizeAndCalculate("harvesterSelectResource");
	Unicode::String params(Unicode::narrowToWide(resourcePoolId.getValueString()));
	ClientCommandQueue::enqueueCommand(commandHash, getNetworkId(), params);
}

// ----------------------------------------------------------------------

void InstallationObject::setTurretObject (TurretObject * turretObject)
{
	m_turretObject = turretObject;
}

// ----------------------------------------------------------------------

TurretObject * InstallationObject::getTurretObject ()
{
	return m_turretObject;
}

//===================================================================

