//======================================================================
//
// DraftSchematicManager.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_DraftSchematicManager_H
#define INCLUDED_DraftSchematicManager_H

//======================================================================

class ClientObject;
class CreatureObject;
class DraftSchematicInfo;
class MessageQueueDraftSlotsData;
class MessageQueueDraftSlotsQueryResponse;
class MessageQueueResourceWeights;
class ResourceContainerObject;

//----------------------------------------------------------------------


//----------------------------------------------------------------------

class DraftSchematicManager
{
public:

	struct Messages
	{		
		struct Changed
		{
			typedef CreatureObject Payload;
		};
	};

	typedef DraftSchematicInfo                         Info;
	typedef stdvector<const Info *>::fwd               InfoVector;
	typedef MessageQueueDraftSlotsData                 Slot;
	typedef stdvector<Slot>::fwd                       SlotVector;

	static void           install ();
	static void           remove  ();
	static void           reset   ();
	static void           getPlayerDraftSchematics (InfoVector & iv);
	static void           refresh                  ();
	static void           update                   (float elapsedTime);

	static const DraftSchematicInfo * cacheDraftSchematic     (const std::pair<uint32, uint32> & crc);
	static void           addDraftSchematic       (const std::pair<uint32, uint32> & crc);
	static void           removeDraftSchematic    (const std::pair<uint32, uint32> & crc);
	static bool           playerHasDraftSchematic (const std::pair<uint32, uint32> & crc);

	static bool           formatDescriptionIfNewer    (const ClientObject & obj, Unicode::String & header, Unicode::String & desc, Unicode::String & attribs, bool minimalAttribs, int & lastFrameUpdate, ResourceContainerObject* considerResource = NULL);

	static const DraftSchematicInfo * findDraftSchematicForObject (const ClientObject & obj);
	const static DraftSchematicInfo * findDraftSchematic          (const std::pair<uint32, uint32> & crc);

	static void           receiveDraftSlotsQueryResponse (const MessageQueueDraftSlotsQueryResponse & msg);
	static void           receiveResourceWeights         (const MessageQueueResourceWeights & msg);

	static void           requestDraftSlots     (const DraftSchematicInfo* info);
	static void           requestResourceWeights(const DraftSchematicInfo* info);

private:
	static stdvector<const DraftSchematicInfo*>::fwd ms_requestDraftSlotsBatch;
	static stdvector<const DraftSchematicInfo*>::fwd ms_requestResourceWeightsBatch;
};

//======================================================================

#endif
