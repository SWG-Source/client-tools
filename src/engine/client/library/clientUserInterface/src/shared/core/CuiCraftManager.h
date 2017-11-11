//======================================================================
//
// CuiCraftManager.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiCraftManager_H
#define INCLUDED_CuiCraftManager_H

//======================================================================

//@todo: the header must be cleaned up & stripped down
#include "sharedGame/CraftingData.h"

class MessageQueueDraftSlotsData;
class CachedNetworkId;
class ClientObject;
class CuiCraftManagerSimulator;
class ManufactureSchematicObject;
class MessageQueueCraftAttributes;
class MessageQueueDraftSchematics;
class MessageQueueDraftSlots;
class MessageQueueGenericIntResponse;
class MessageQueueGenericResponse;
class MessageQueueStringList;
class NetworkId;
class Object;
struct MessageQueueCraftAttributesData;
struct MessageQueueDraftSchematicsData;


//----------------------------------------------------------------------

class CuiCraftManager
{
public:

	typedef MessageQueueCraftAttributesData            Attrib;
	typedef stdvector<Attrib>::fwd                     AttribVector;
	typedef MessageQueueDraftSlotsData                 Slot;
	typedef stdvector<Slot>::fwd                       SlotVector;
	typedef MessageQueueDraftSchematicsData            SchematicData;
	typedef stdvector<SchematicData>::fwd              SchematicVector;
	typedef stdvector<std::pair<int, int> >::fwd       IntPairVector;
	typedef stdvector<int>::fwd                        IntVector;

	//----------------------------------------------------------------------
	
	struct Ids
	{
		static CachedNetworkId craftingTool;
		static CachedNetworkId craftingToolPending;
		static CachedNetworkId craftingToolHopperInput;
		static CachedNetworkId manufactureSchematic;
		static CachedNetworkId schematicPrototype;
	};

	struct Messages
	{		
		struct DraftSchematicsReceived
		{
			typedef SchematicVector Payload;
		};

		struct DraftSlotsReceived
		{
			typedef SlotVector Payload;
		};

		struct StageChange
		{
			typedef int Payload;
		};

		struct Assembling
		{
			typedef int Payload;
		};

		struct DoneAssembling
		{
			typedef int Payload;
		};

		struct Complete
		{
			typedef bool Payload;
		};

		struct Customize
		{
			typedef int Payload;
		};

		struct Experiment
		{
			typedef int Payload;
		};

		struct SessionEnded
		{
			typedef bool Payload;
		};

		struct SlotTransferStatusChanged
		{
			typedef bool Payload;
		};
	};

	enum FinalState
	{
		FS_none,
		FS_proto,
		FS_schem
	};

	static void                               install                                 ();
	static void                               remove                                  ();

	static void                               startCrafting                           (const NetworkId & toolId);
	static void                               stopCrafting                            (bool confirmed, bool emitEndedMessage = false);
	static void                               restartCraftingBROKEN                   (bool confirmed);

	static void                               receiveDraftSchematics                  (const MessageQueueDraftSchematics &);
	static void                               receiveDraftSlots                       (const MessageQueueDraftSlots &);
	static void                               receiveGenericResponse                  (const MessageQueueGenericIntResponse &);
	static void                               receiveNextCraftingStageResult          (const MessageQueueGenericIntResponse & resp);
	static void                               receiveExperimentResult                 (const MessageQueueGenericIntResponse & resp);
	static void                               receiveSessionEnded                     (bool normalExit);

	static void                               requestDraftSlots                       (int draftIndex);

	static void                               transferToSlot                          (const NetworkId & id, int slot, int option);
	static void                               removeFromSlot                          (int slot, const NetworkId & targetContainer);

	static void                               assemble                                (bool confirmed);

	static void                               complete                                (bool confirmed, bool practice);

	static void                               customize                               ();

	static void                               experiment                              (const IntPairVector & v, int corelevel = -1);

	static ClientObject *                     getCraftingTool                         ();
	static ClientObject *                     getCraftingStation                      ();
	static ClientObject *                     getCraftingStationHopperInput           ();
	static ClientObject *                     getCraftingStationHopperInput           (const ClientObject * station);        
	static ManufactureSchematicObject *       getManufactureSchematic                 ();
	static ClientObject *                     getSchematicPrototype                   ();
	static const std::pair<uint32, uint32> &  getCurrentDraftSchematicCrc             ();

	static const SlotVector &                 getSlots                                ();
	static const Slot *                       getSlot                                 (int index);

	static const SchematicVector &            getSchematics                           ();

	static ClientObject *                     findHopperOutputContent                 (ClientObject & tool);

	static FinalState                         getFinalState                           ();
	static void                               setFinalState                           (FinalState fs);

	static Crafting::CraftingStage            findCurrentStage                        ();
	static int                                findCraftingLevel                       ();

	static bool                               isCrafting                              ();

	static int                                findPlayerCurrentExperimentPoints       (bool randomizeSinglePlayer = false);

	static bool                               findManufactureSchematicAttributeValues (int index, StringId & id, float & min, float & max, float & cur, float & resourceMax);

	static bool                               getValidSlotOptions                     (const ClientObject & objectToInsert, int slotIndex, IntVector & validOptions);
	static bool                               isReadyToAssemble                       ();

	static bool                               canManufacture                          ();
	static bool                               canExperiment                           ();

	static const std::string &                getHopperOutputSlotName                 ();
	
	static void                               reportAssemblyResult                    (Crafting::CraftingResult craftingResult);
	static void                               reportExperimentationResult             (Crafting::CraftingResult craftingResult);

	static bool                               objectDerivesFromTemplate               (const Object & obj, const std::string & potentialAncestorName);

	static const Unicode::String &            getLocalizedAssemblyResult              ();

	static void                               setCraftingBioLinkedItem                (bool isBioLinked);

	//----------------------------------------------------------------------

	static void                               confirmDestroy                          ();

	static const IntVector &                  getSlotTransferStatus                   ();
	static int                                getSlotTransferStatus                   (int slot);

	static bool canStartCrafting(NetworkId const & station);
	static bool isCrafting(NetworkId const & object);

	static bool isCraftingStartCommand(uint32 commandHash);
	static void CuiCraftManager::abortCraftingStart();


private:

	static void                               transitionToFinished ();
};

//----------------------------------------------------------------------

inline bool CuiCraftManager::isCrafting ()
{
	return findCurrentStage () != 0;
}

//======================================================================

#endif
