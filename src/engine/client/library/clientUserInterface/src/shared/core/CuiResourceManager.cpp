//======================================================================
//
// CuiResourceManager.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiResourceManager.h"

#include "UnicodeUtils.h"
#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientInstallationSynchronizedUi.h"
#include "clientGame/ClientObject.h"
#include "clientGame/Game.h"
#include "clientGame/InstallationObject.h"
#include "clientGame/ResourceTypeManager.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiStringIdsResource.h"
#include "sharedMessageDispatch/Receiver.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/MessageQueueGenericResponse.h"
#include "sharedNetworkMessages/MessageQueueHarvesterResourceData.h"
#include "sharedNetworkMessages/MessageQueueResourceEmptyHopper.h"
#include "sharedObject/CachedNetworkId.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedObject/Controller.h"

//======================================================================

class PlayerCreatureController;

namespace
{

	CachedNetworkId s_harvesterId;

	typedef CuiResourceManager::ResourceInfo    ResourceInfo;
	typedef CuiResourceManager::ResourceInfoMap ResourceInfoMap;

	ResourceInfoMap s_infoMap;
	
	//----------------------------------------------------------------------

	namespace Transceivers
	{
		MessageDispatch::Transceiver<const int &, CuiResourceManager::Messages::PoolsChanged > 
			poolsChanged;
		MessageDispatch::Transceiver<const bool &, CuiResourceManager::Messages::EmptyCompleted > 
			emptyCompleted;
	}

	//----------------------------------------------------------------------

	namespace MessageBoxes
	{
		CuiMessageBox * requestEmptyHopper = 0;
		CuiMessageBox * confirmDumpHopper = 0;
	}
	
	//----------------------------------------------------------------------

	namespace Sequence
	{
		uint8 requestEmptyHopper;
	}

	//----------------------------------------------------------------------

	namespace DumpInfo
	{
		NetworkId resource;
		int       amount;
	}

	//----------------------------------------------------------------------

	struct Listener : public MessageDispatch::Receiver 
	{
		Listener () :
			MessageDispatch::Receiver (),
			m_boxPtrs ()
		{
			m_boxPtrs.push_back (BoxPtr (&MessageBoxes::requestEmptyHopper,          &Sequence::requestEmptyHopper));
			m_boxPtrs.push_back (BoxPtr (&MessageBoxes::confirmDumpHopper,           0));
		}

		void cancelRequest (CuiMessageBox *& box)
		{
			for (BoxPtrs::iterator it = m_boxPtrs.begin (); it != m_boxPtrs.end (); ++it)
			{
				if ((*it).first == &box)
				{
					if (box)
						box->closeMessageBox ();
					if ((*it).second)
						++(*(*it).second);
				}
			}
		}

		void receiveMessage(const MessageDispatch::Emitter & , const MessageDispatch::MessageBase & message)
		{
			const CuiMessageBox::BoxMessage * const abm = dynamic_cast<const CuiMessageBox::BoxMessage *>(&message);
			
			if (abm)
			{
				if (message.isType (CuiMessageBox::Messages::COMPLETED))
				{
					const CuiMessageBox::CompletedMessage * const cmsg = dynamic_cast<const CuiMessageBox::CompletedMessage *>(&message);
					NOT_NULL (cmsg);

					if (cmsg->getButtonType () == CuiMessageBox::GBT_Ok)
					{
						if (abm->getMessageBox () == MessageBoxes::confirmDumpHopper)
							CuiResourceManager::emptyHopper (DumpInfo::resource, DumpInfo::amount, true, true);
					}
					else if (cmsg->getButtonType () == CuiMessageBox::GBT_Cancel)
					{
						for (BoxPtrs::iterator it = m_boxPtrs.begin (); it != m_boxPtrs.end (); ++it)
						{
							if (*(*it).first == abm->getMessageBox () && (*it).second)
								++(*(*it).second);
						}
					}
				}
				else if (message.isType (CuiMessageBox::Messages::CLOSED))
				{
					for (BoxPtrs::iterator it = m_boxPtrs.begin (); it != m_boxPtrs.end (); ++it)
					{
						if (*(*it).first == abm->getMessageBox ())
							*(*it).first = 0;
					}
				}
			}
		}

		typedef std::pair<CuiMessageBox **, uint8 *> BoxPtr;
		typedef std::vector <BoxPtr> BoxPtrs;
		BoxPtrs	m_boxPtrs;

	};

	//----------------------------------------------------------------------

	Listener * s_listener = 0;

	//----------------------------------------------------------------------

	class MyCallback : 
	public MessageDispatch::Callback
	{
	public:
		MyCallback () : MessageDispatch::Callback () {}

		void receiveResourcePoolData      (const MessageQueueHarvesterResourceData & data)
		{
			if (data.getHarvesterId () != CuiResourceManager::getHarvesterId ())
				return;

			s_infoMap.clear ();

			const MessageQueueHarvesterResourceData::DataVector & dv = data.getDataVector ();

			for (MessageQueueHarvesterResourceData::DataVector::const_iterator it = dv.begin (); it != dv.end (); ++it)
			{
				const InstallationResourceData & ird = *it;
				IGNORE_RETURN (s_infoMap.insert (std::make_pair (ird.m_id, ResourceInfo (Unicode::narrowToWide(ird.m_name), ird.m_efficiency))));
				ResourceTypeManager::setTypeInfo(ird.m_id, Unicode::narrowToWide(ird.m_name), ird.m_parentName);
			}

			Transceivers::poolsChanged.emitMessage (0);
		}
	};
	
	MyCallback s_callback;

	bool s_installed = false;
}

//----------------------------------------------------------------------

void CuiResourceManager::install ()
{
	DEBUG_FATAL (s_installed, ("installed"));
	s_callback.connect (s_callback, &MyCallback::receiveResourcePoolData,        static_cast<PlayerCreatureController *>         (0));
	s_listener  = new Listener;
	s_installed = true;
}

//----------------------------------------------------------------------

void CuiResourceManager::remove ()
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	delete s_listener;
	s_listener  = 0;
	s_callback.disconnect (s_callback, &MyCallback::receiveResourcePoolData,        static_cast<PlayerCreatureController *>         (0));
	s_installed = false;
}

//----------------------------------------------------------------------

CuiResourceManager::ResourceInfo::ResourceInfo (const Unicode::String & theName, uint8 theEfficiency) :
name (theName),
efficiency (theEfficiency)
{}

//----------------------------------------------------------------------

CuiResourceManager::ResourceInfo::ResourceInfo () :
name (),
efficiency (0)
{}

//----------------------------------------------------------------------

const CachedNetworkId & CuiResourceManager::getHarvesterId  ()
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	return s_harvesterId;
}

//----------------------------------------------------------------------

ClientObject * CuiResourceManager::getHarvester    ()
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	return safe_cast<ClientObject *>(s_harvesterId.getObject ());
}

//----------------------------------------------------------------------

InstallationObject * CuiResourceManager::getHarvesterInstallation ()
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	return dynamic_cast<InstallationObject *>(s_harvesterId.getObject ());
}

//----------------------------------------------------------------------

CuiResourceManager::Synchronized * CuiResourceManager::getSynchronized ()
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	ClientObject * const obj = getHarvester ();
	return obj ? dynamic_cast<Synchronized *> (obj->getClientSynchronizedUi ()) : 0;
}

//----------------------------------------------------------------------

void CuiResourceManager::setHarvester    (ClientObject * obj)
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	InstallationObject * harvester = getHarvesterInstallation ();

	if (obj)
		s_harvesterId = *obj;
	else
		s_harvesterId = NetworkId::cms_invalid;

	harvester = getHarvesterInstallation ();

	s_infoMap.clear ();
}

//----------------------------------------------------------------------

void CuiResourceManager::startListening()
{
	Object * const player = Game::getPlayer ();
	InstallationObject * harvester = getHarvesterInstallation ();

	if (harvester && player)
		harvester->listenToHarvester (*player, true);
}

//----------------------------------------------------------------------

void CuiResourceManager::stopListening()
{
	Object * const player = Game::getPlayer ();
	InstallationObject * harvester = getHarvesterInstallation ();

	if (harvester && player)
		harvester->listenToHarvester (*player, false);
}

//----------------------------------------------------------------------

void CuiResourceManager::setHarvester    (const NetworkId & id)
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	setHarvester (safe_cast<ClientObject *>(CachedNetworkId (id).getObject ()));
}

//----------------------------------------------------------------------

void CuiResourceManager::requestResourceData ()
{	
	DEBUG_FATAL (!s_installed, ("not installed"));

	Object * const player                = Game::getPlayer ();
	InstallationObject * const harvester = CuiResourceManager::getHarvesterInstallation ();

	if (harvester && player)
		harvester->getHarvesterResourceData (*player);
}

//----------------------------------------------------------------------

void CuiResourceManager::emptyHopper (const NetworkId & resourceTypeId, int amount, bool discard, bool confirmed)
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	if (confirmed)
	{
		ClientObject * const player = Game::getClientPlayer ();
		Controller * const controller = player ? player->getController () : 0;

		if (player && controller)
		{
			s_listener->cancelRequest (MessageBoxes::requestEmptyHopper);

			MessageQueueResourceEmptyHopper * const msg = 
				new MessageQueueResourceEmptyHopper (player->getNetworkId (), getHarvesterId (), resourceTypeId, amount, discard, Sequence::requestEmptyHopper);
			
			controller->appendMessage(static_cast<int>(CM_clientResourceHarvesterEmptyHopper), 0.0f, msg, 
				GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_SERVER);

			Unicode::String str;
			CuiStringIdsResource::res_wait_empty_hopper.localize (str);
			MessageBoxes::requestEmptyHopper = CuiMessageBox::createWaitingBox (str, NULL, false);
			MessageBoxes::requestEmptyHopper->connectToMessages (*s_listener);

			Transceivers::emptyCompleted.emitMessage (true);
		}
	}
	else
	{
		if (discard)
		{
			DumpInfo::resource = resourceTypeId;
			DumpInfo::amount   = amount;

			s_listener->cancelRequest (MessageBoxes::confirmDumpHopper);

			Unicode::String str;
			CuiStringIdsResource::res_confirm_dump_hopper.localize (str);
			MessageBoxes::confirmDumpHopper = CuiMessageBox::createOkCancelBox (str);
			MessageBoxes::confirmDumpHopper->connectToMessages (*s_listener);
		}
		else
			emptyHopper (resourceTypeId, amount, discard, true);
	}
}

//----------------------------------------------------------------------

void CuiResourceManager::dumpAll()
{
	ClientCommandQueue::enqueueCommand("harvesterDiscardHopper", s_harvesterId, Unicode::emptyString);
}

//----------------------------------------------------------------------

CuiResourceManager::ResourceInfoMap & CuiResourceManager::getResourceInfoMap ()
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	return s_infoMap;
}

//----------------------------------------------------------------------

const CuiResourceManager::ResourceInfo * CuiResourceManager::findResourceInfo (const NetworkId & resourceId)
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	const ResourceInfoMap::const_iterator it = s_infoMap.find (resourceId);

	return it == s_infoMap.end () ? 0 : &(*it).second;
}

//----------------------------------------------------------------------

void CuiResourceManager::receiveGenericResponse (const MessageQueueGenericResponse & resp)
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	switch (resp.getRequestId ())
	{
	case CM_clientResourceHarvesterEmptyHopper:
		{
			if (Sequence::requestEmptyHopper != resp.getSequenceId ())
				WARNING (true, ("discarding stale empty hopper resp"));
			else
			{
				s_listener->cancelRequest (MessageBoxes::requestEmptyHopper);
				if (!resp.getSuccess ())
				{
					Unicode::String str;
					CuiStringIdsResource::res_err_empty_hopper.localize (str);
					CuiMessageBox::createInfoBox (str);
				}

				Transceivers::emptyCompleted.emitMessage (resp.getSuccess ());
			}
		}
		break;
	default:
		break;
	}
}

//======================================================================
