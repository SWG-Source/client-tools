package script.systems.veteran_reward;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.player_structure;
import script.library.prose;
import script.library.space_utils;
import script.library.static_item;
import script.library.sui;
import script.library.utils;


public class structure_storage_increase_deed extends script.base_script
{
	public structure_storage_increase_deed()
	{
	}
	public static final string_id STORAGE_INCREASE = new string_id(player_structure.STF_FILE, "use_storage_increase_deed");
	public static final string_id SID_STORAGE_INCREASE_TITLE = new string_id(player_structure.STF_FILE, "sui_storage_title");
	public static final string_id SID_STORAGE_INCREASE_PROMPT = new string_id(player_structure.STF_FILE, "sui_storage_prompt");
	public static final string_id MSG_STORAGE_INCREASED = new string_id(player_structure.STF_FILE, "msg_storage_increased");
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		obj_id structure = getTopMostContainer(player);
		
		if ((isGameObjectTypeOf(structure, GOT_building) || (isGameObjectTypeOf(structure, GOT_ship_fighter) && space_utils.isShipWithInterior(structure))) && player_structure.isOwner(structure, player))
		{
			mi.addRootMenu(menu_info_types.ITEM_USE, STORAGE_INCREASE);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		if (item == menu_info_types.ITEM_USE)
		{
			obj_id structure = getTopMostContainer(player);
			
			if (validateDeedUse(self, player, structure))
			{
				String title = utils.packStringId(SID_STORAGE_INCREASE_TITLE);
				String prompt = utils.packStringId(SID_STORAGE_INCREASE_PROMPT);
				sui.msgbox(self, player, prompt, sui.YES_NO, title, "handleStorageChoice");
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean validateDeedUse(obj_id self, obj_id player, obj_id structure) throws InterruptedException
	{
		if (!isIdValid(structure))
		{
			sendSystemMessage(player, new string_id(player_structure.STF_FILE, "storage_increase_inside_warning"));
			return false;
		}
		
		if (player_structure.isNonGenericLootItem(self))
		{
			if (player_structure.alreadyHasNonGenericStorageIncreaseOfType(structure, self))
			{
				string_id alreadyHasThatOneMsg = new string_id(player_structure.STF_FILE, "storage_increase_has_nongeneric_type");
				prose_package pp = prose.getPackage(alreadyHasThatOneMsg);
				prose.setTO(pp, new string_id("static_item_n", getStaticItemName(self)));
				sendSystemMessageProse(player, pp);
				return false;
			}
		}
		
		String template = getTemplateName(structure);
		
		if (!isGameObjectTypeOf(structure, GOT_ship_fighter))
		{
			if (!(template.indexOf("player_house_") > -1) && !(template.indexOf("player_guildhall_") > -1) && !(template.indexOf("barn_no_planet_restriction.iff") > -1) && !(template.indexOf("diner_no_planet_restriction.iff") > -1))
			{
				sendSystemMessage(player, new string_id(player_structure.STF_FILE, "storage_increase_houses_only"));
				return false;
			}
			
			if (!isGameObjectTypeOf(structure, GOT_building))
			{
				sendSystemMessage(player, new string_id(player_structure.STF_FILE, "storage_increase_inside_warning"));
				return false;
			}
			
			if (!player_structure.isOwner(structure, player))
			{
				sendSystemMessage(player, new string_id(player_structure.STF_FILE, "not_owner"));
				return false;
			}
		}
		else
		{
			if (!space_utils.isShipWithInterior(structure))
			{
				sendSystemMessage(player, new string_id(player_structure.STF_FILE, "storage_increase_inside_warning"));
				return false;
			}
			
			if (getOwner(structure) != player)
			{
				sendSystemMessage(player, new string_id(player_structure.STF_FILE, "not_owner"));
				return false;
			}
		}
		
		return true;
	}
	
	
	public int handleStorageChoice(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = sui.getPlayerId(params);
		int btn = sui.getIntButtonPressed(params);
		
		if (btn == sui.BP_CANCEL)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id structure = getTopMostContainer(player);
		
		if (validateDeedUse(self, player, structure))
		{
			int structureIncrease = 0;
			
			if (hasObjVar(self, player_structure.OBJVAR_STORAGE_AMOUNT))
			{
				structureIncrease = getIntObjVar(self, player_structure.OBJVAR_STORAGE_AMOUNT);
			}
			
			if (structureIncrease > 0 && player_structure.incrementStorageAmount(player, structure, self, structureIncrease))
			{
				String itemName = getStaticItemName(self);
				if (itemName != null && itemName.length() > 0)
				{
					dictionary itemData = new dictionary();
					itemData = dataTableGetRow(static_item.ITEM_STAT_BALANCE_TABLE, itemName);
					if (itemData != null)
					{
						String clientEffect = itemData.getString("client_effect");
						if (clientEffect != null && clientEffect.length() > 0)
						{
							playClientEffectObj(player, clientEffect, player, "");
						}
					}
				}
				
				sendSystemMessage(player, MSG_STORAGE_INCREASED);
				destroyObject(self);
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnGetAttributes(obj_id self, obj_id player, String[] names, String[] attribs) throws InterruptedException
	{
		int free = getFirstFreeIndex(names);
		
		if (free == -1)
		{
			return super.OnGetAttributes(self, player, names, attribs);
		}
		
		int structureIncrease = 0;
		
		if (hasObjVar(self, player_structure.OBJVAR_STORAGE_AMOUNT))
		{
			structureIncrease = getIntObjVar(self, player_structure.OBJVAR_STORAGE_AMOUNT);
		}
		
		names[free] = utils.packStringId(new string_id("player_structure", "storage_amount_display"));
		attribs[free++] = "" + structureIncrease;
		
		return super.OnGetAttributes(self, player, names, attribs);
	}
	
}
