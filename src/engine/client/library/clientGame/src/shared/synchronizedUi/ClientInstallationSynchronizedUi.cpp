// ======================================================================
//
// ClientInstallationSynchronizedUi.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientInstallationSynchronizedUi.h"

#include "clientGame/InstallationObject.h"
#include "sharedMessageDispatch/Transceiver.h"

//----------------------------------------------------------------------

namespace ClientInstallationSynchronizedUiNamespace
{
	namespace Transceivers
	{
		MessageDispatch::Transceiver<const ClientInstallationSynchronizedUi &,                       ClientInstallationSynchronizedUi::Messages::ExtractionRate>        s_extractionRate;
		MessageDispatch::Transceiver<const ClientInstallationSynchronizedUi &,                       ClientInstallationSynchronizedUi::Messages::ExtractionRateMax>     s_extractionRateMax;
		MessageDispatch::Transceiver<const ClientInstallationSynchronizedUi &,                       ClientInstallationSynchronizedUi::Messages::Active>                s_active;
		MessageDispatch::Transceiver<const ClientInstallationSynchronizedUi &,                       ClientInstallationSynchronizedUi::Messages::ResourcePool>          s_resourcePool;
		MessageDispatch::Transceiver<const ClientInstallationSynchronizedUi &,                       ClientInstallationSynchronizedUi::Messages::ResourcePoolsModified> s_resourcePoolsModified;
		MessageDispatch::Transceiver<const ClientInstallationSynchronizedUi &,                       ClientInstallationSynchronizedUi::Messages::Hopper>                s_hopper;
		MessageDispatch::Transceiver<const ClientInstallationSynchronizedUi &,                       ClientInstallationSynchronizedUi::Messages::HopperMax>             s_hopperMax;
		MessageDispatch::Transceiver<const ClientInstallationSynchronizedUi &,                       ClientInstallationSynchronizedUi::Messages::Condition>             s_condition;
		MessageDispatch::Transceiver<const ClientInstallationSynchronizedUi &,                       ClientInstallationSynchronizedUi::Messages::InstallationSynchronizedUiReady>             s_installationSynchronizedUiReady;
	}

	bool s_profileNetworkTraffic = false;
}

using namespace ClientInstallationSynchronizedUiNamespace;

//----------------------------------------------------------------------

//#include "clientGame/InstallationObject.h"
//@todo once we get Installation Objects replace.
//-----------------------------------------------------------------------

ClientInstallationSynchronizedUi::ClientInstallationSynchronizedUi (InstallationObject& owner) :
ClientSynchronizedUi      (owner),
m_resourcePoolsModified   (0),
m_resourcePools           (),
m_resourceTypesId         (),
m_resourceTypesName       (),
m_resourceTypesParent     (),
m_resourcePoolId          (NetworkId::cms_invalid),
m_active                  (0),
m_extractionRateInstalled (0),
m_extractionRateCur       (0),
m_extractionRateMax       (0),
m_hopperContentsCur       (0),
m_hopperContentsMax       (0),
m_hopperContentsModified  (0),
m_hopperContents          (),
m_condition               (0)
{
	addToUiPackage (m_resourcePoolsModified);
	addToUiPackage (m_resourcePools);
	addToUiPackage (m_resourceTypesId);
	addToUiPackage (m_resourceTypesName);
	addToUiPackage (m_resourceTypesParent);
	addToUiPackage (m_resourcePoolId);
	addToUiPackage (m_active);
	addToUiPackage (m_extractionRateMax);
	addToUiPackage (m_extractionRateCur);
	addToUiPackage (m_extractionRateInstalled);
	addToUiPackage (m_hopperContentsCur);
	addToUiPackage (m_hopperContentsMax);
	addToUiPackage (m_hopperContentsModified);
	addToUiPackage (m_hopperContents);
	addToUiPackage (m_condition);

	m_resourcePoolsModified.setSourceObject   (this);
	m_resourcePoolId.setSourceObject          (this);
	m_active.setSourceObject                  (this);
	m_extractionRateMax.setSourceObject       (this);
	m_extractionRateCur.setSourceObject       (this);
	m_extractionRateInstalled.setSourceObject (this);
	m_hopperContentsCur.setSourceObject       (this);
	m_hopperContentsMax.setSourceObject       (this);
	m_hopperContentsModified.setSourceObject  (this);

	m_resourcePools.setOnInsert(this, &ClientInstallationSynchronizedUi::onPoolsInserted);
	m_resourceTypesId.setOnInsert(this, &ClientInstallationSynchronizedUi::onTypeIdsInserted);
	m_resourceTypesName.setOnInsert(this, &ClientInstallationSynchronizedUi::onNameInserted);
	m_resourceTypesParent.setOnInsert(this, &ClientInstallationSynchronizedUi::onParentInserted);

	m_resourcePools.setOnErase(this, &ClientInstallationSynchronizedUi::onPoolsDeleted);
	m_resourceTypesId.setOnErase(this, &ClientInstallationSynchronizedUi::onTypeIdsDeleted);
	m_resourceTypesName.setOnErase(this, &ClientInstallationSynchronizedUi::onNameDeleted);
	m_resourceTypesParent.setOnErase(this, &ClientInstallationSynchronizedUi::onParentDeleted);
}

//-----------------------------------------------------------------------

ClientInstallationSynchronizedUi::~ClientInstallationSynchronizedUi()
{
}

//-----------------------------------------------------------------------

void ClientInstallationSynchronizedUi::Callbacks::ResourcePool::modified(ClientInstallationSynchronizedUi & target, const NetworkId &, const NetworkId &, bool) const
{
	if (target.getOwner ())
		Transceivers::s_resourcePool.emitMessage (target);
}

//----------------------------------------------------------------------

void ClientInstallationSynchronizedUi::Callbacks::Active::modified(ClientInstallationSynchronizedUi & target, const uint8 &oldValue , const uint8 & value, bool) const
{
	UNREF(oldValue);
	UNREF(value);
	if (target.getOwner ())
	{
		if(oldValue != value)
		{
			Transceivers::s_active.emitMessage (target);
		}
	}
}

//----------------------------------------------------------------------

void ClientInstallationSynchronizedUi::Callbacks::Hopper::modified(ClientInstallationSynchronizedUi & target, const float & , const float &, bool ) const
{
	if (target.getOwner ())
		Transceivers::s_hopper.emitMessage (target);
}

//----------------------------------------------------------------------

void ClientInstallationSynchronizedUi::Callbacks::HopperMax::modified(ClientInstallationSynchronizedUi & target, const int & , const int &, bool ) const
{
	if (target.getOwner ())
		Transceivers::s_hopperMax.emitMessage (target);
}

//----------------------------------------------------------------------

void ClientInstallationSynchronizedUi::Callbacks::ExtractionRate::modified(ClientInstallationSynchronizedUi & target, const float & , const float &, bool ) const
{
	if (target.getOwner ())
		Transceivers::s_extractionRate.emitMessage (target);
}

//----------------------------------------------------------------------

void ClientInstallationSynchronizedUi::Callbacks::ExtractionRateMax::modified(ClientInstallationSynchronizedUi & target, const int & , const int &, bool ) const
{
	if (target.getOwner ())
		Transceivers::s_extractionRateMax.emitMessage (target);
}

//----------------------------------------------------------------------

void ClientInstallationSynchronizedUi::Callbacks::ResourcePoolsModified::modified(ClientInstallationSynchronizedUi & target, const uint8 & , const uint8 &, bool ) const
{
	if (target.getOwner ())
		Transceivers::s_resourcePoolsModified.emitMessage (target);
}

//----------------------------------------------------------------------

void ClientInstallationSynchronizedUi::Callbacks::Condition::modified(ClientInstallationSynchronizedUi & target, const int8 & , const int8 &, bool ) const
{
	if (target.getOwner ())
		Transceivers::s_condition.emitMessage (target);
}


//----------------------------------------------------------------------

/**
* optimize by keeping ids sorted & using binary_search?
*/
bool ClientInstallationSynchronizedUi::hasResourcePool            (const NetworkId & id) const
{
	return std::find (m_resourcePools.begin (), m_resourcePools.end (), id) != m_resourcePools.end ();
}

//----------------------------------------------------------------------

bool ClientInstallationSynchronizedUi::hasHopperContent           (const NetworkId & id) const
{
	for (HopperContentsVector::const_iterator it = m_hopperContents.begin (); it != m_hopperContents.end (); ++it)
	{
		if ((*it).first == id)
			return true;
	}

	return false;
}

//----------------------------------------------------------------------

void ClientInstallationSynchronizedUi::onPoolsInserted(const unsigned int c, const NetworkId & nid)
{
	if(s_profileNetworkTraffic)
		REPORT_LOG(true, ("Inserted the %dth pool %s\n", c, nid.getValueString()));
}

//----------------------------------------------------------------------

void ClientInstallationSynchronizedUi::onTypeIdsInserted(const unsigned int c, const NetworkId & nid)
{
	if(s_profileNetworkTraffic)
		REPORT_LOG(true, ("Inserted the %dth typeid %s\n", c, nid.getValueString()));
}

//----------------------------------------------------------------------

void ClientInstallationSynchronizedUi::onNameInserted(const unsigned int c, const std::string & s)
{
	if(s_profileNetworkTraffic)
		REPORT_LOG(true, ("Inserted the %dth name %s\n", c, s.c_str()));
}

//----------------------------------------------------------------------

void ClientInstallationSynchronizedUi::onParentInserted(const unsigned int c, const std::string & s)
{
	if(s_profileNetworkTraffic)
		REPORT_LOG(true, ("Inserted the %dth parent %s\n", c, s.c_str()));
}

//----------------------------------------------------------------------

void ClientInstallationSynchronizedUi::onPoolsDeleted(const unsigned int c, const NetworkId & nid)
{
	if(s_profileNetworkTraffic)
		REPORT_LOG(true, ("Deleted the %dth pool %s\n", c, nid.getValueString()));
}

//----------------------------------------------------------------------

void ClientInstallationSynchronizedUi::onTypeIdsDeleted(const unsigned int c, const NetworkId & nid)
{
	if(s_profileNetworkTraffic)
		REPORT_LOG(true, ("Deleted the %dth typeid %s\n", c, nid.getValueString()));
}

//----------------------------------------------------------------------

void ClientInstallationSynchronizedUi::onNameDeleted(const unsigned int c, const std::string & s)
{
	if(s_profileNetworkTraffic)
		REPORT_LOG(true, ("Deleted the %dth name %s\n", c, s.c_str()));
}

//----------------------------------------------------------------------

void ClientInstallationSynchronizedUi::onParentDeleted(const unsigned int c, const std::string & s)
{
	if(s_profileNetworkTraffic)
		REPORT_LOG(true, ("Deleted the %dth parent %s\n", c, s.c_str()));
}

//----------------------------------------------------------------------

void ClientInstallationSynchronizedUi::onBaselinesRecieved()
{
	Transceivers::s_installationSynchronizedUiReady.emitMessage(*this);
}

//----------------------------------------------------------------------
