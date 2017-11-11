//======================================================================
//
// CuiCraftManagerSimulator.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiCraftManagerSimulator.h"

#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/ManufactureSchematicObject.h"
#include "clientUserInterface/CuiCraftManager.h"
#include "sharedFoundation/Crc.h"
#include "sharedNetworkMessages/MessageQueueDraftSchematics.h"
#include "sharedNetworkMessages/MessageQueueDraftSlots.h"
#include "sharedObject/CachedNetworkId.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedRandom/Random.h"
#include <vector>

//======================================================================

void CuiCraftManagerSimulator::startCrafting ()
{
	MessageQueueDraftSchematics msg (CuiCraftManager::Ids::craftingToolPending, NetworkId::cms_invalid);
	
	msg.addSchematic (std::make_pair(Crc::calculate("type 1"), Crc::calculate("shared_type 1")), 1);
	msg.addSchematic (std::make_pair(Crc::calculate("type 2"), Crc::calculate("shared_type 2")), 1);
	msg.addSchematic (std::make_pair(Crc::calculate("type 3"), Crc::calculate("shared_type 3")), 1);
	msg.addSchematic (std::make_pair(Crc::calculate("type 10"), Crc::calculate("shared_type 10")), 2);
	msg.addSchematic (std::make_pair(Crc::calculate("type 20"), Crc::calculate("shared_type 20")), 2);
	msg.addSchematic (std::make_pair(Crc::calculate("type 30"), Crc::calculate("shared_type 30")), 2);
	msg.addSchematic (std::make_pair(Crc::calculate("type 100"), Crc::calculate("shared_type 100")), 3);
	msg.addSchematic (std::make_pair(Crc::calculate("type 200"), Crc::calculate("shared_type 200")), 3);
	msg.addSchematic (std::make_pair(Crc::calculate("type 300"), Crc::calculate("shared_type 300")), 3);
	
	CuiCraftManager::receiveDraftSchematics (msg);
}

//----------------------------------------------------------------------

void CuiCraftManagerSimulator::requestDraftSlots ()
{
	MessageQueueDraftSlots msg (CuiCraftManager::Ids::craftingTool, NetworkId::cms_invalid);
	msg.setSlots (CuiCraftManager::getSlots ());
	CuiCraftManager::receiveDraftSlots (msg);
}

//----------------------------------------------------------------------

void CuiCraftManagerSimulator::onGetManufacturingSchematic (CachedNetworkId & id)
{
	if (id == NetworkId::cms_invalid)
	{
		ManufactureSchematicObject * const obj = safe_cast<ManufactureSchematicObject *>(ObjectTemplate::createObject ("object/manufacture_schematic/shared_generic_schematic.iff"));
		if (obj)
		{
			obj->endBaselines ();
			id = CachedNetworkId (*obj);
		}
	}
}

//----------------------------------------------------------------------

void CuiCraftManagerSimulator::onGetSchematicPrototype (CachedNetworkId & id)
{
	if (id == NetworkId::cms_invalid)
	{
		Object * const obj = ObjectTemplate::createObject ("object/tangible/wearables/shirt/shared_shirt_s05.iff");
		if (obj)
		{
			safe_cast<ClientObject *>(obj)->endBaselines ();
			id = CachedNetworkId (*obj);
		}
	}
}

//----------------------------------------------------------------------

void CuiCraftManagerSimulator::onGetSlots (CuiCraftManager::SlotVector & s_slotVector)
{
		if (s_slotVector.empty ())
		{
			Slot slot;
			
			typedef MessageQueueDraftSlots::Option Option;
			
			Option ing;
			ing.amountNeeded = 10;
			ing.name = StringId("default", "default");
			ing.ingredient = Unicode::narrowToWide ("default ingredient");
			ing.type = Crafting::IT_resourceType;
			
			slot.options.push_back (ing);
			
			
			s_slotVector.reserve (10);
			
			for (int i = 0; i < 8; ++i)
			{
				char buf [128];
				snprintf (buf, 128, "test slot thenameis %d", i);
				slot.name = StringId ("slot_n", buf);
				slot.optional = (i % 3) == 0;
				
				if (i == 7)
				{
					for (int j = 0; j < 3; ++j)
					{
						snprintf (buf, 128, "test ingredient thenameis %d", j);
						ing.ingredient = Unicode::narrowToWide (buf);
						ing.amountNeeded = j * 32;
						slot.options.push_back (ing);
					}
				}
				s_slotVector.push_back (slot);
			}
		}
}

//----------------------------------------------------------------------

void CuiCraftManagerSimulator::onGetSchematics (CuiCraftManager::SchematicVector & s_schematicVector)
{
	static const std::pair<uint32, uint32> test_schems [] =
	{
		std::make_pair(Crc::calculate("object/draft_schematic/armor/armor_module_heavy.iff"), Crc::calculate("object/draft_schematic/armor/shared_armor_module_heavy.iff")),
		std::make_pair(Crc::calculate("object/draft_schematic/armor/armor_module_light.iff"), Crc::calculate("object/draft_schematic/armor/shared_armor_module_light.iff")),
		std::make_pair(Crc::calculate("object/draft_schematic/chemistry/medpack_wound_stamina.iff"), Crc::calculate("object/draft_schematic/chemistry/shared_medpack_wound_stamina.iff")),
		std::make_pair(Crc::calculate("object/draft_schematic/chemistry/medpack_wound_strength.iff"), Crc::calculate("object/draft_schematic/chemistry/shared_medpack_wound_strength.iff")),
		std::make_pair(Crc::calculate("object/draft_schematic/chemistry/stimpack_sm_s1.iff"), Crc::calculate("object/draft_schematic/chemistry/shared_stimpack_sm_s1.iff")),
		std::make_pair(Crc::calculate("object/draft_schematic/clothing/clothing_armor_bone_bicep_l.iff"), Crc::calculate("object/draft_schematic/clothing/shared_clothing_armor_bone_bicep_l.iff")),
		std::make_pair(Crc::calculate("object/draft_schematic/clothing/clothing_armor_tantel_chest.iff"), Crc::calculate("object/draft_schematic/clothing/shared_clothing_armor_tantel_chest.iff")),
		std::make_pair(Crc::calculate("object/draft_schematic/clothing/clothing_armor_tantel_helmet.iff"), Crc::calculate("object/draft_schematic/clothing/shared_clothing_armor_tantel_helmet.iff")),
		std::make_pair(Crc::calculate("object/draft_schematic/clothing/clothing_vest_field.iff"), Crc::calculate("object/draft_schematic/clothing/shared_clothing_vest_field.iff")),
		std::make_pair(Crc::calculate("object/draft_schematic/clothing/clothing_vest_formal.iff"), Crc::calculate("object/draft_schematic/clothing/shared_clothing_vest_formal.iff")),
		std::make_pair(Crc::calculate("object/draft_schematic/weapon/axe.iff"), Crc::calculate("object/draft_schematic/weapon/shared_axe.iff")),
		std::make_pair(Crc::calculate("object/draft_schematic/weapon/axe_vibro.iff"), Crc::calculate("object/draft_schematic/weapon/shared_axe_vibro.iff")),
		std::make_pair(Crc::calculate("object/draft_schematic/weapon/blaster_cannon.iff"), Crc::calculate("object/draft_schematic/weapon/shared_blaster_cannon.iff")),
		std::make_pair(Crc::calculate("object/draft_schematic/weapon/pistol_blaster_short_range_combat.iff"), Crc::calculate("object/draft_schematic/weapon/shared_pistol_blaster_short_range_combat.iff")),
		std::make_pair(Crc::calculate("object/draft_schematic/weapon/pistol_disrupter_dx2.iff"), Crc::calculate("object/draft_schematic/weapon/shared_pistol_disrupter_dx2.iff")),
		std::make_pair(Crc::calculate("object/draft_schematic/weapon/pistol_flechette_fwg5.iff"), Crc::calculate("object/draft_schematic/weapon/shared_pistol_flechette_fwg5.iff")),
		std::make_pair(Crc::calculate("object/draft_schematic/weapon/rifle_spray_stick_stohli.iff"), Crc::calculate("object/draft_schematic/weapon/shared_rifle_spray_stick_stohli.iff")),
		std::make_pair(Crc::calculate("object/draft_schematic/weapon/rifle_tangle_gun7.iff"), Crc::calculate("object/draft_schematic/weapon/shared_rifle_tangle_gun7.iff")),
		std::make_pair(Crc::calculate("object/draft_schematic/weapon/staff.iff"), Crc::calculate("object/draft_schematic/weapon/shared_staff.iff")),
		std::make_pair(Crc::calculate("object/draft_schematic/weapon/sword.iff"), Crc::calculate("object/draft_schematic/weapon/shared_sword.iff")),
		std::make_pair(Crc::calculate("object/draft_schematic/weapon/sword_ryyk_blade.iff"), Crc::calculate("object/draft_schematic/weapon/shared_sword_ryyk_blade.iff"))
	};
	
	const int num_schems = sizeof (test_schems) / sizeof (test_schems [0]);
	
	if (s_schematicVector.empty ())
	{
		CreatureObject * const player = Game::getPlayerCreature ();
		
		if (player)
		{
			
			std::map<std::pair<uint32,uint32>,int> sv = player->getDraftSchematics ();
			
			for (int i = 0; i < num_schems; ++i)
			{
				if (sv.find(test_schems [i]) == sv.end())
					player->clientGrantSchematic(test_schems[i].first, test_schems[i].second);
				
				const int category = 1 << (i % 3);
				const SchematicData sdata (test_schems[i].first, test_schems[i].second, category);
				
				s_schematicVector.push_back (sdata);
			}
		}
	}
}

//----------------------------------------------------------------------

int CuiCraftManagerSimulator::onFindPlayerCurrentExperimentPoints (bool randomizeSinglePlayer)
{
	static int s_expPoints = 5;
	
	if (randomizeSinglePlayer)
		s_expPoints = Random::random (1, 30);

	return s_expPoints;
}

//----------------------------------------------------------------------

void CuiCraftManagerSimulator::stopCrafting                        ()
{
}

//======================================================================
