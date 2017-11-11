// ======================================================================
//
// ClientSynchronizedUi.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientSynchronizedUi.h"

#include "ClientGame/ClientObject.h"
#include "sharedNetworkMessages/BaselinesMessage.h"
#include "sharedNetworkMessages/DeltasMessage.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedObject/NetworkIdManager.h"

#include <vector>

//-----------------------------------------------------------------------
ClientSynchronizedUi::ClientSynchronizedUi(ClientObject & obj) :
m_owner(&obj)
{
}
//-----------------------------------------------------------------------

ClientSynchronizedUi::~ClientSynchronizedUi()
{
	m_owner = 0;
}

//-----------------------------------------------------------------------

void ClientSynchronizedUi::addToUiPackage(Archive::AutoDeltaVariableBase & source)
{
	m_uiPackage.addVariable(source);
}

//-----------------------------------------------------------------------

void ClientSynchronizedUi::applyBaselines(const BaselinesMessage& source)
{
	DEBUG_FATAL(source.getPackageId() != BaselinesMessage::BASELINES_UI, ("Synchrnoized UI received bad package type"));
	Archive::ReadIterator bs = source.getPackage().begin();
	m_uiPackage.unpack(bs);
	onBaselinesRecieved();
}

//-----------------------------------------------------------------------

void ClientSynchronizedUi::applyDeltas(const DeltasMessage& source)
{
	DEBUG_FATAL(source.getPackageId() != DeltasMessage::DELTAS_UI, ("Synchrnoized UI received bad package type"));
	Archive::ReadIterator bs = source.getPackage().begin();
	m_uiPackage.unpackDeltas(bs);
}

//-----------------------------------------------------------------------

void ClientSynchronizedUi::clearDeltas()
{
	m_uiPackage.clearDeltas();
}

//-----------------------------------------------------------------------

void ClientSynchronizedUi::onBaselinesRecieved()
{
}

//-----------------------------------------------------------------------
