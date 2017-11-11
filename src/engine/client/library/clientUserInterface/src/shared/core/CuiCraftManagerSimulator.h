//======================================================================
//
// CuiCraftManagerSimulator.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiCraftManagerSimulator_H
#define INCLUDED_CuiCraftManagerSimulator_H

//======================================================================

class  MessageQueueDraftSlotsData;
struct MessageQueueDraftSchematicsData;
class  CachedNetworkId;

//----------------------------------------------------------------------

class CuiCraftManagerSimulator
{
public:

	typedef MessageQueueDraftSlotsData                 Slot;
	typedef stdvector<Slot>::fwd                       SlotVector;

	typedef MessageQueueDraftSchematicsData            SchematicData;
	typedef stdvector<SchematicData>::fwd              SchematicVector;


	static void         startCrafting                       ();
	static void         requestDraftSlots                   ();
	static int          onFindPlayerCurrentExperimentPoints (bool randomizeSinglePlayer);
	static void         onGetSchematics                     (SchematicVector & s_schematicVector);
	static void         onGetSlots                          (SlotVector & s_slotVector);
	static void         onGetSchematicPrototype             (CachedNetworkId & id);
	static void         onGetManufacturingSchematic         (CachedNetworkId & id);

	static void         stopCrafting                        ();
};

//======================================================================

#endif
