/* ----------------------------------------------*/
//          LOOT DISTRIBUTION DETAILS
/* ----------------------------------------------*/

Review this datatable for treasure types per spawn:
	//depot/swg/x1/dsrc/sku.0/sys.server/compiled/game/datatables/space_mobile/space_mobile.tab

Treasure types refer to equipment lists. A treasure type can be comprised of multiple equipment lists. 
See this table for the relationship between treasure types and equipment lists:
	//depot/swg/x1/dsrc/sku.0/sys.server/compiled/game/datatables/space_loot/loot_lookup.tab

Equipment lists are collections of specific object templates and are found in this datatable:
	//depot/swg/x1/dsrc/sku.0/sys.server/compiled/game/datatables/space_loot/loot_items.tab
	
Equipment types:
	equipment_1 = Imperial components (Seinar fleet systems, for example)
	equipment_2 = Rebel components (Incom, etc.)
	equipment_3 = Civilian / general components (non-faction-specific)
	equipment_4 = Hutt components
	equipment_5 = Blacksun components
	equipment_6 = Small civilian freighter cargo (static items, house decorations, consumables)
	equipment_7 = Medium civilian freighter cargo
	equipment_9  = UNDEFINED
	equipment_10 = UNDEFINED
	equipment_11 = UNEDFINED
	
Tresure types:
	treasure_1 = equipment_1 (Imperial spawns)
	treasure_2 = equipment_2 (Rebel spawns)
	treasure_3 = equipment_3 (Civilians)
	treasure_4 = equipment_4 (Hutt fighters)
	treasure_5 = equipment_2 + equipment_4 (Nym pirates)
	treasure_6 = equipment_6
	treasure_7 = equipment_7
	treasure_8 = equipment_1 + equipment_2 + equipment_8 (General military cargo)
	
NOTE: there is no specific Blacksun treasure as of 04/23/04 - cbarnes
