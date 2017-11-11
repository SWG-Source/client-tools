//======================================================================
//
// DraftSchematicManager.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/DraftSchematicManager.h"

#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/PlayerObject.h"
#include "clientGame/ClientCommandQueue.h"
#include "clientGame/DraftSchematicInfo.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/Crc.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/MessageQueueDraftSlotsQueryResponse.h"
#include "sharedNetworkMessages/MessageQueueResourceWeights.h"
#include <algorithm>
#include <vector>

//======================================================================

namespace DraftSchematicManagerNamespace
{
	typedef DraftSchematicManager::Info         Info;
	typedef DraftSchematicManager::InfoVector   InfoVector;

	InfoVector s_slots;

	//----------------------------------------------------------------------

	namespace Transceivers
	{
		MessageDispatch::Transceiver<const DraftSchematicManager::Messages::Changed::Payload &, DraftSchematicManager::Messages::Changed>
			changed;
	}

	//----------------------------------------------------------------------

	struct SlotSorter
	{
		bool operator ()(const Info * a, const Info * b)
		{
			const Unicode::String & name_a = a->getLocalizedName ();
			const Unicode::String & name_b = b->getLocalizedName ();

			return name_a < name_b;
		}
	};

	//----------------------------------------------------------------------

	class MyCallback :
	public MessageDispatch::Callback
	{
	public:

		MyCallback () : MessageDispatch::Callback () {}

		//----------------------------------------------------------------------

		void onDraftSchematicsChanged (const PlayerObject::Messages::DraftSchematicsChanged::Payload & creature)
		{
			if (&creature == static_cast<const Object *>(Game::getPlayerObject ()))
			{
				DraftSchematicManager::refresh ();
			}
		}

		//----------------------------------------------------------------------

		void onDraftSchematicAdded (const PlayerObject::Messages::DraftSchematicAdded::Payload & msg)
		{
			if (msg.first == static_cast<const Object *>(Game::getPlayerObject ()))
			{
				DraftSchematicManager::addDraftSchematic (msg.second);
			}
		}

		//----------------------------------------------------------------------

		void onDraftSchematicRemoved (const PlayerObject::Messages::DraftSchematicRemoved::Payload & msg)
		{
			if (msg.first == static_cast<const Object *>(Game::getPlayerObject ()))
			{
				DraftSchematicManager::removeDraftSchematic (msg.second);
			}
		}
	};

	//----------------------------------------------------------------------

	MyCallback s_callback;

	bool       s_ready = false;
}

using namespace DraftSchematicManagerNamespace;

//----------------------------------------------------------------------

std::vector<const DraftSchematicInfo*> DraftSchematicManager::ms_requestDraftSlotsBatch;
std::vector<const DraftSchematicInfo*> DraftSchematicManager::ms_requestResourceWeightsBatch;

//----------------------------------------------------------------------

void DraftSchematicManager::install ()
{
	InstallTimer const installTimer("DraftSchematicManager::install");

//	s_callback.connect (s_callback, &MyCallback::onDraftSchematicsChanged,          static_cast<CreatureObject::Messages::DraftSchematicsChanged *>     (0));
	s_callback.connect (s_callback, &MyCallback::onDraftSchematicAdded,             static_cast<PlayerObject::Messages::DraftSchematicAdded *>     (0));
	s_callback.connect (s_callback, &MyCallback::onDraftSchematicRemoved,           static_cast<PlayerObject::Messages::DraftSchematicRemoved *>     (0));
	s_callback.connect (s_callback, &MyCallback::onDraftSchematicsChanged,          static_cast<CreatureObject::Messages::PlayerSetup *>     (0));
	s_ready = false;
}

//----------------------------------------------------------------------

void DraftSchematicManager::remove  ()
{
	reset ();
//	s_callback.disconnect (s_callback, &MyCallback::onDraftSchematicsChanged,           static_cast<CreatureObject::Messages::DraftSchematicsChanged *>     (0));
	s_callback.disconnect (s_callback, &MyCallback::onDraftSchematicAdded,              static_cast<PlayerObject::Messages::DraftSchematicAdded *>     (0));
	s_callback.disconnect (s_callback, &MyCallback::onDraftSchematicRemoved,            static_cast<PlayerObject::Messages::DraftSchematicRemoved *>     (0));
	s_callback.disconnect (s_callback, &MyCallback::onDraftSchematicsChanged,           static_cast<CreatureObject::Messages::PlayerSetup *>     (0));
}

//----------------------------------------------------------------------

void DraftSchematicManager::reset   ()
{
	for (InfoVector::iterator it = s_slots.begin (); it != s_slots.end (); ++it)
	{
		const DraftSchematicInfo * const info = NON_NULL (*it);
		delete info;
	}

	s_slots.clear ();
	s_ready = false;
}

//----------------------------------------------------------------------

void DraftSchematicManager::getPlayerDraftSchematics (InfoVector & iv)
{
	if (!s_ready)
		refresh ();

	const CreatureObject * const player = Game::getPlayerCreature ();
	if (player)
	{
		const std::map<std::pair<uint32,uint32>,int> & sv = player->getDraftSchematics ();
		for (std::map<std::pair<uint32,uint32>,int>::const_iterator it = sv.begin (); it != sv.end (); ++it)
		{
			const DraftSchematicInfo * const dsi = findDraftSchematic((*it).first);

			if (dsi)
				iv.push_back (dsi);
			else
				WARNING (true, ("DraftSchematicManager null draft for [%u]", (*it).first.first));
		}
	}
}

//----------------------------------------------------------------------

void DraftSchematicManager::refresh ()
{
	reset ();
	s_ready = true;

	const CreatureObject * const player = Game::getPlayerCreature ();
	if (player)
	{
		const std::map<std::pair<uint32,uint32>,int> & sv = player->getDraftSchematics ();

		s_slots.reserve (sv.size ());

		for (std::map<std::pair<uint32,uint32>,int>::const_iterator it = sv.begin (); it != sv.end (); ++it)
		{
			s_slots.push_back (new DraftSchematicInfo ((*it).first));
		}

		std::sort (s_slots.begin (), s_slots.end (), SlotSorter ());

		Transceivers::changed.emitMessage (*player);
	}
}

//----------------------------------------------------------------------

const DraftSchematicInfo * DraftSchematicManager::cacheDraftSchematic (const std::pair<uint32, uint32> & crc)
{
	if (!s_ready)
		refresh ();

	const DraftSchematicInfo * dsi = findDraftSchematic (crc);
	if (!dsi)
	{

		dsi = new DraftSchematicInfo (crc);
		s_slots.push_back (dsi);
		std::sort (s_slots.begin (), s_slots.end (), SlotSorter ());
	}

	return dsi;
}


//----------------------------------------------------------------------

void DraftSchematicManager::addDraftSchematic (const std::pair<uint32, uint32> & crc)
{
	cacheDraftSchematic (crc);
	const CreatureObject * const player = Game::getPlayerCreature ();
	if (player)
		Transceivers::changed.emitMessage (*player);
}

//----------------------------------------------------------------------

void DraftSchematicManager::removeDraftSchematic (const std::pair<uint32, uint32> & crc)
{
	if (!s_ready)
		refresh ();

	for (InfoVector::iterator it = s_slots.begin (); it != s_slots.end (); ++it)
	{
		const DraftSchematicInfo * const info = NON_NULL (*it);
		if (info->getDraftSchematicTemplate() == crc)
		{
			delete info;
			s_slots.erase (it);
			break;
		}
	}

	const CreatureObject * const player = Game::getPlayerCreature ();
	if (player)
		Transceivers::changed.emitMessage (*player);
}

//----------------------------------------------------------------------

bool DraftSchematicManager::playerHasDraftSchematic (const std::pair<uint32, uint32> & crc)
{
	const CreatureObject * const player = Game::getPlayerCreature ();
	if (player)
		return (player->getDraftSchematics().find(crc) != player->getDraftSchematics().end());

	return false;
}

//----------------------------------------------------------------------

const DraftSchematicInfo * DraftSchematicManager::findDraftSchematicForObject (const ClientObject & obj)
{
	if (!s_ready)
		refresh ();

	for (InfoVector::iterator it = s_slots.begin (); it != s_slots.end (); ++it)
	{
		const DraftSchematicInfo * const info = NON_NULL (*it);
		if (info->getClientObject () == &obj)
			return info;
	}
	return 0;
}

//----------------------------------------------------------------------

const DraftSchematicInfo * DraftSchematicManager::findDraftSchematic(const std::pair<uint32, uint32> & crc)
{
	if (!s_ready)
		refresh ();

	for (InfoVector::iterator it = s_slots.begin (); it != s_slots.end (); ++it)
	{
		const DraftSchematicInfo * const info = NON_NULL (*it);
		if (info->getDraftSchematicTemplate() == crc)
			return info;
	}

	return 0;
}

//----------------------------------------------------------------------

bool DraftSchematicManager::formatDescriptionIfNewer   (const ClientObject & obj, Unicode::String & header, Unicode::String & desc, Unicode::String & attribs, bool minimalAttribs, int & lastFrameUpdate, ResourceContainerObject* considerResource)
{
	const DraftSchematicInfo * const info = findDraftSchematicForObject (obj);

	if (info)
	{
		if (lastFrameUpdate == 0 || info->getLastFrameUpdate () != lastFrameUpdate)
		{
			lastFrameUpdate = info->getLastFrameUpdate ();
			header.clear ();
			desc.clear ();
			attribs.clear ();

			info->requestDataIfNeeded ();
			return info->formatDescription (header, desc, attribs, minimalAttribs, considerResource);
		}
	}

	return false;
}

//----------------------------------------------------------------------

void DraftSchematicManager::receiveDraftSlotsQueryResponse (const MessageQueueDraftSlotsQueryResponse & msg)
{
	const std::pair<uint32, uint32> & draftSchematicCrc = msg.getDraftSchematicCrc();
	const SlotVector &  slots                           = msg.getSlots ();

	DraftSchematicInfo * const info = const_cast<DraftSchematicInfo *>(findDraftSchematic(draftSchematicCrc));

	if (!info)
		WARNING (true, ("Received draft slots query response for unknown draft schematic: [%lu]", draftSchematicCrc.first));
	else
	{
		const int complexity                 = msg.getComplexity ();
		const int manufactureSchematicVolume = msg.getVolume     ();

		info->setSlots                      (slots);
		info->setComplexity                 (complexity);
		info->setManufactureSchematicVolume (manufactureSchematicVolume);
	}
}

//----------------------------------------------------------------------

void DraftSchematicManager::receiveResourceWeights (const MessageQueueResourceWeights & msg)
{
	const std::pair<uint32, uint32> & draftSchematicCrc = msg.getDraftSchematicCrc ();

	DraftSchematicInfo * const info = const_cast<DraftSchematicInfo *>(findDraftSchematic(draftSchematicCrc));

	if (!info)
		WARNING (true, ("Received draft slots query response for unknown draft schematic: [%lu]", draftSchematicCrc.first));
	else
	{
		info->setResourceWeights(msg.getAllAssemblyWeights (), msg.getAllResourceMaxWeights ());
	}
}

//----------------------------------------------------------------------

void DraftSchematicManager::requestDraftSlots     (const DraftSchematicInfo* info)
{
	if(!info)
		return;
	ms_requestDraftSlotsBatch.push_back(info);
}

//----------------------------------------------------------------------

void DraftSchematicManager::requestResourceWeights(const DraftSchematicInfo* info)
{
	if(!info)
		return;
	ms_requestResourceWeightsBatch.push_back(info);
}

//----------------------------------------------------------------------

void DraftSchematicManager::update(float )
{
	if(!ms_requestDraftSlotsBatch.empty())
	{
		std::string params;
		char buffer[64];
		memset(buffer, 0, sizeof(buffer));
		static const uint32 hash_reqSlotsBatch   = Crc::normalizeAndCalculate("requestDraftSlotsBatch");
		for(std::vector<const DraftSchematicInfo*>::iterator i = ms_requestDraftSlotsBatch.begin(); i != ms_requestDraftSlotsBatch.end(); ++i)
		{
			if(!(*i))
				continue;
			sprintf(buffer, "%lu %lu", (*i)->getServerDraftSchematicTemplate(), (*i)->getSharedDraftSchematicTemplate());
			params += buffer;
			params += " ";
			memset(buffer, 0, sizeof(buffer));
		}
		ClientCommandQueue::enqueueCommand (hash_reqSlotsBatch, NetworkId::cms_invalid, Unicode::narrowToWide (params));
		ms_requestDraftSlotsBatch.clear();
	}

	if(!ms_requestResourceWeightsBatch.empty())
	{
		std::string params;
		char buffer[64];
		memset(buffer, 0, sizeof(buffer));
		static const uint32 hash_reqWeightsBatch = Crc::normalizeAndCalculate("requestResourceWeightsBatch");
		for(std::vector<const DraftSchematicInfo*>::iterator i = ms_requestResourceWeightsBatch.begin(); i != ms_requestResourceWeightsBatch.end(); ++i)
		{
			if(!(*i))
				continue;
			_itoa((*i)->getServerDraftSchematicTemplate(), buffer, 10);
			params += buffer;
			params += " ";
			memset(buffer, 0, sizeof(buffer));
		}
		ClientCommandQueue::enqueueCommand (hash_reqWeightsBatch, NetworkId::cms_invalid, Unicode::narrowToWide (params));
		ms_requestResourceWeightsBatch.clear();
	}
}

//======================================================================
