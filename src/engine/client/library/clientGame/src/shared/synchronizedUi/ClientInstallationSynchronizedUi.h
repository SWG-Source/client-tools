// ======================================================================
//
// ClientInstallationSynchronizedUi.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_ClientInstallationSynchronizedUi_H
#define	INCLUDED_ClientInstallationSynchronizedUi_H

#include "clientGame/ClientSynchronizedUi.h"
#include "sharedUtility/InstallationResourceDataArchive.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkIdArchive.h"
#include "Archive/AutoDeltaVariableCallback.h"
#include "Archive/AutoDeltaVector.h"

class InstallationObject;

//----------------------------------------------------------------------

class ClientInstallationSynchronizedUi : public ClientSynchronizedUi
{
public:

	struct Messages
	{
		struct ExtractionRate;
		struct ExtractionRateMax;
		struct ResourcePool;
		struct ResourcePoolsModified;
		struct Active;
		struct Hopper;
		struct HopperMax;
		struct Condition;
		struct InstallationSynchronizedUiReady;
	};

	explicit ClientInstallationSynchronizedUi (InstallationObject &);
	        ~ClientInstallationSynchronizedUi ();

	typedef stdvector<NetworkId>::fwd                ResourcePoolVector;
	typedef stdvector<NetworkId>::fwd                ResourceTypeIdVector;
	typedef stdvector<std::string>::fwd              ResourceTypeNameVector;
	typedef stdvector<std::string>::fwd              ResourceTypeParentVector;
	typedef std::pair<NetworkId, float>              HopperContentElement;
	typedef stdvector<HopperContentElement>::fwd     HopperContentsVector;

	const ResourcePoolVector &       getResourcePools           () const;
	const ResourceTypeIdVector &     getResourceTypesId         () const;
	const ResourceTypeNameVector &   getResourceTypesName       () const;
	const ResourceTypeParentVector & getResourceTypesParent     () const;

	const NetworkId &            getSelectedResourcePool     () const;
	bool                         getActive                  () const;
	float                        getExtractionRateInstalled () const;
	float                        getExtractionRateCur       () const;
	int                          getExtractionRateMax       () const;
	float                        getHopperContentsCur       () const;
	int                          getHopperContentsMax       () const;
	const HopperContentsVector & getHopperContentsVector    () const;
	int8                         getCondition               () const;

	bool                         hasResourcePool            (const NetworkId & id) const;
	bool                         hasHopperContent           (const NetworkId & id) const;

private:
	ClientInstallationSynchronizedUi(const ClientInstallationSynchronizedUi& );
	ClientInstallationSynchronizedUi();
	ClientInstallationSynchronizedUi& operator= (const ClientInstallationSynchronizedUi &);

	void onPoolsInserted(const unsigned int, const NetworkId & );
	void onTypeIdsInserted(const unsigned int, const NetworkId & );
	void onNameInserted(const unsigned int, const std::string & s);
	void onParentInserted(const unsigned int, const std::string & s);
	void onPoolsDeleted(const unsigned int, const NetworkId & );
	void onTypeIdsDeleted(const unsigned int, const NetworkId & );
	void onNameDeleted(const unsigned int, const std::string & s);
	void onParentDeleted(const unsigned int, const std::string & s);

	virtual void onBaselinesRecieved();

	//----------------------------------------------------------------------
	//-- AutoDeltaVariable Callbacks

	struct Callbacks
	{
		template <typename S, typename U> struct DefaultCallback
		{
			void modified (ClientInstallationSynchronizedUi & target, const U & old, const U & value, bool isLocal) const;
		};

		typedef DefaultCallback<Messages::ResourcePool,          NetworkId>       ResourcePool;
		typedef DefaultCallback<Messages::ResourcePoolsModified,   uint8>         ResourcePoolsModified;
		typedef DefaultCallback<Messages::Active,                  uint8>         Active;
		typedef DefaultCallback<Messages::Hopper,                  float>         Hopper;
		typedef DefaultCallback<Messages::HopperMax,               int>           HopperMax;
		typedef DefaultCallback<Messages::ExtractionRate,          float>         ExtractionRate;
		typedef DefaultCallback<Messages::ExtractionRateMax,       int>           ExtractionRateMax;
		typedef DefaultCallback<Messages::Condition,               int8>          Condition;
	};

	//----------------------------------------------------------------------

	typedef Archive::AutoDeltaVector<NetworkId, ClientInstallationSynchronizedUi>         AutoResourcePoolVector;
	typedef Archive::AutoDeltaVector<NetworkId, ClientInstallationSynchronizedUi>         AutoResourceTypeIdVector;
	typedef Archive::AutoDeltaVector<std::string, ClientInstallationSynchronizedUi>       AutoResourceTypeNameVector;
	typedef Archive::AutoDeltaVector<std::string, ClientInstallationSynchronizedUi>       AutoResourceTypeParentVector;

	Archive::AutoDeltaVariableCallback <uint8, Callbacks::ResourcePoolsModified, ClientInstallationSynchronizedUi>   m_resourcePoolsModified;
	AutoResourcePoolVector                                                                                           m_resourcePools;
	AutoResourceTypeIdVector                                                                                         m_resourceTypesId;
	AutoResourceTypeNameVector                                                                                       m_resourceTypesName;
	AutoResourceTypeParentVector                                                                                     m_resourceTypesParent;
	Archive::AutoDeltaVariableCallback<NetworkId,       Callbacks::ResourcePool,     ClientInstallationSynchronizedUi>  m_resourcePoolId;
	Archive::AutoDeltaVariableCallback<uint8,     Callbacks::Active,         ClientInstallationSynchronizedUi>       m_active;
	Archive::AutoDeltaVariableCallback<float,     Callbacks::ExtractionRate, ClientInstallationSynchronizedUi>       m_extractionRateInstalled;
	Archive::AutoDeltaVariableCallback<int,       Callbacks::ExtractionRateMax, ClientInstallationSynchronizedUi>       m_extractionRateMax;
	Archive::AutoDeltaVariableCallback<float,     Callbacks::Hopper,            ClientInstallationSynchronizedUi>       m_hopperContentsCur;
	Archive::AutoDeltaVariableCallback<int,       Callbacks::HopperMax,         ClientInstallationSynchronizedUi>       m_hopperContentsMax;
	Archive::AutoDeltaVariableCallback<uint8,     Callbacks::Hopper,            ClientInstallationSynchronizedUi>       m_hopperContentsModified;
	Archive::AutoDeltaVector<HopperContentElement>                                                                   m_hopperContents;
	Archive::AutoDeltaVariableCallback<float,     Callbacks::ExtractionRate, ClientInstallationSynchronizedUi>       m_extractionRateCur;
	Archive::AutoDeltaVariableCallback<int8,      Callbacks::Condition,      ClientInstallationSynchronizedUi>       m_condition;
}; 

//----------------------------------------------------------------------

inline const NetworkId & ClientInstallationSynchronizedUi::getSelectedResourcePool () const
{
	return m_resourcePoolId.get ();
}

//----------------------------------------------------------------------

inline bool ClientInstallationSynchronizedUi::getActive           () const
{
	return m_active.get () != 0;
}

//----------------------------------------------------------------------

inline float  ClientInstallationSynchronizedUi::getExtractionRateCur    () const
{
	return static_cast<float>(m_extractionRateCur.get ());
}

//----------------------------------------------------------------------

inline int  ClientInstallationSynchronizedUi::getExtractionRateMax    () const
{
	return static_cast<int>(m_extractionRateMax.get ());
}

//----------------------------------------------------------------------

inline float  ClientInstallationSynchronizedUi::getExtractionRateInstalled   () const
{
	return static_cast<float>(m_extractionRateInstalled.get ());
}

//----------------------------------------------------------------------

inline float  ClientInstallationSynchronizedUi::getHopperContentsCur   () const
{
	return static_cast<float>(m_hopperContentsCur.get ());
}

//----------------------------------------------------------------------

inline int  ClientInstallationSynchronizedUi::getHopperContentsMax   () const
{
	return static_cast<int>(m_hopperContentsMax.get ());
}

//----------------------------------------------------------------------

inline const ClientInstallationSynchronizedUi::ResourcePoolVector & ClientInstallationSynchronizedUi::getResourcePools   () const
{
	return m_resourcePools.get ();
}

//----------------------------------------------------------------------

inline const ClientInstallationSynchronizedUi::ResourceTypeIdVector & ClientInstallationSynchronizedUi::getResourceTypesId   () const
{
	return m_resourceTypesId.get ();
}

//----------------------------------------------------------------------

inline const ClientInstallationSynchronizedUi::ResourceTypeNameVector & ClientInstallationSynchronizedUi::getResourceTypesName   () const
{
	return m_resourceTypesName.get ();
}

//----------------------------------------------------------------------

inline const ClientInstallationSynchronizedUi::ResourceTypeParentVector & ClientInstallationSynchronizedUi::getResourceTypesParent   () const
{
	return m_resourceTypesParent.get ();
}

//----------------------------------------------------------------------

inline const ClientInstallationSynchronizedUi::HopperContentsVector & ClientInstallationSynchronizedUi::getHopperContentsVector () const
{
	return m_hopperContents.get ();
}

//----------------------------------------------------------------------

inline int8 ClientInstallationSynchronizedUi::getCondition() const
{
	return m_condition.get ();
}

//----------------------------------------------------------------------

#endif
