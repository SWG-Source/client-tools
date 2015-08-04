package script.item.loot_kits;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.sui;
import script.library.utils;


public class loot_kit_base extends script.base_script
{
	public loot_kit_base()
	{
	}
	public static final String STF = "loot_kit";
	
	public static final string_id INCORRECT_ITEM = new string_id(STF, "incorrect_item");
	public static final string_id ALREADY_CONTAINS = new string_id(STF, "already_contains");
	public static final string_id NEW_ITEM_CREATED = new string_id(STF, "new_item_created");
	public static final string_id ITEM_USED = new string_id(STF, "item_used");
	public static final string_id AUTOSTACK_ATTACHED_EJECTED = new string_id(STF, "austostack_attached_ejected");
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		String datatable = getStringObjVar (self, "loot_kit");
		
		String[] objvarToSet = dataTableGetStringColumnNoDefaults (datatable, "objvar_name");
		int numObjVars = objvarToSet.length;
		
		if (numObjVars == 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		int passThrough = 0;
		
		while (passThrough < numObjVars)
		{
			testAbortScript();
			String kitObjVar = dataTableGetString (datatable, passThrough, "objvar_name");
			String kitObjVarValue = dataTableGetString (datatable, passThrough, "objvar_value");
			
			setObjVar(self, kitObjVar, kitObjVarValue);
			
			passThrough = passThrough + 1;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnGetAttributes(obj_id self, obj_id player, String[] names, String[] attribs) throws InterruptedException
	{
		int idx = utils.getValidAttributeIndex(names);
		
		if (idx == -1)
		{
			return super.OnGetAttributes(self, player, names, attribs);
		}
		
		String datatable = getStringObjVar (self, "loot_kit");
		
		if (hasObjVar(self, "collectible"))
		{
			int one = getIntObjVar(self, "collectible.item_contained_01");
			int two = getIntObjVar(self, "collectible.item_contained_02");
			int three = getIntObjVar(self, "collectible.item_contained_03");
			int four = getIntObjVar(self, "collectible.item_contained_04");
			int five = getIntObjVar(self, "collectible.item_contained_05");
			int six = getIntObjVar(self, "collectible.item_contained_06");
			int seven = getIntObjVar(self, "collectible.item_contained_07");
			int eight = getIntObjVar(self, "collectible.item_contained_08");
			int nine = getIntObjVar(self, "collectible.item_contained_09");
			int ten = getIntObjVar(self, "collectible.item_contained_10");
			
			String itemNameOne = dataTableGetString (datatable, 0, "examine_item_name");
			names[idx] = itemNameOne;
			if (one == 0)
			{
				String hasItem = "No";
				attribs[idx] = hasItem;
			}
			else if (one == 1)
			{
				String hasItem = "Yes";
				attribs[idx] = hasItem;
			}
			idx++;
			
			if (idx >= names.length)
			{
				return super.OnGetAttributes(self, player, names, attribs);
			}
			
			String itemNameTwo = dataTableGetString (datatable, 1, "examine_item_name");
			names[idx] = itemNameTwo;
			if (two == 0)
			{
				String hasItem = "No";
				attribs[idx] = hasItem;
			}
			else if (two == 1)
			{
				String hasItem = "Yes";
				attribs[idx] = hasItem;
			}
			idx++;
			
			if (idx >= names.length)
			{
				return super.OnGetAttributes(self, player, names, attribs);
			}
			
			String itemNameThree = dataTableGetString (datatable, 2, "examine_item_name");
			names[idx] = itemNameThree;
			if (three == 0)
			{
				String hasItem = "No";
				attribs[idx] = hasItem;
			}
			else if (three == 1)
			{
				String hasItem = "Yes";
				attribs[idx] = hasItem;
			}
			idx++;
			
			if (idx >= names.length)
			{
				return super.OnGetAttributes(self, player, names, attribs);
			}
			
			String itemNameFour = dataTableGetString (datatable, 3, "examine_item_name");
			names[idx] = itemNameFour;
			if (four == 0)
			{
				String hasItem = "No";
				attribs[idx] = hasItem;
			}
			else if (four == 1)
			{
				String hasItem = "Yes";
				attribs[idx] = hasItem;
			}
			idx++;
			
			if (idx >= names.length)
			{
				return super.OnGetAttributes(self, player, names, attribs);
			}
			
			String itemNameFive = dataTableGetString (datatable, 4, "examine_item_name");
			names[idx] = itemNameFive;
			if (five == 0)
			{
				String hasItem = "No";
				attribs[idx] = hasItem;
			}
			else if (five == 1)
			{
				String hasItem = "Yes";
				attribs[idx] = hasItem;
			}
			idx++;
			
			if (idx >= names.length)
			{
				return super.OnGetAttributes(self, player, names, attribs);
			}
			
			String itemNameSix = dataTableGetString (datatable, 5, "examine_item_name");
			names[idx] = itemNameSix;
			if (six == 0)
			{
				String hasItem = "No";
				attribs[idx] = hasItem;
			}
			else if (six == 1)
			{
				String hasItem = "Yes";
				attribs[idx] = hasItem;
			}
			idx++;
			
			if (idx >= names.length)
			{
				return super.OnGetAttributes(self, player, names, attribs);
			}
			
			String itemNameSeven = dataTableGetString (datatable, 6, "examine_item_name");
			names[idx] = itemNameSeven;
			if (seven == 0)
			{
				String hasItem = "No";
				attribs[idx] = hasItem;
			}
			else if (seven == 1)
			{
				String hasItem = "Yes";
				attribs[idx] = hasItem;
			}
			idx++;
			
			if (idx >= names.length)
			{
				return super.OnGetAttributes(self, player, names, attribs);
			}
			
			String itemNameEight = dataTableGetString (datatable, 7, "examine_item_name");
			names[idx] = itemNameEight;
			if (eight == 0)
			{
				String hasItem = "No";
				attribs[idx] = hasItem;
			}
			else if (eight == 1)
			{
				String hasItem = "Yes";
				attribs[idx] = hasItem;
			}
			idx++;
			
			if (idx >= names.length)
			{
				return super.OnGetAttributes(self, player, names, attribs);
			}
			
			String itemNameNine = dataTableGetString (datatable, 8, "examine_item_name");
			names[idx] = itemNameNine;
			if (nine == 0)
			{
				String hasItem = "No";
				attribs[idx] = hasItem;
			}
			else if (nine == 1)
			{
				String hasItem = "Yes";
				attribs[idx] = hasItem;
			}
			idx++;
			
			if (idx >= names.length)
			{
				return super.OnGetAttributes(self, player, names, attribs);
			}
			
			String itemNameTen = dataTableGetString (datatable, 9, "examine_item_name");
			names[idx] = itemNameTen;
			if (ten == 0)
			{
				String hasItem = "No";
				attribs[idx] = hasItem;
			}
			else if (ten == 1)
			{
				String hasItem = "Yes";
				attribs[idx] = hasItem;
			}
			idx++;
			
			if (idx >= names.length)
			{
				return super.OnGetAttributes(self, player, names, attribs);
			}
		}
		
		return super.OnGetAttributes(self, player, names, attribs);
	}
	
	
	public int OnAboutToReceiveItem(obj_id self, obj_id srcContainer, obj_id transferer, obj_id item) throws InterruptedException
	{
		if (!utils.isNestedWithin(self, transferer))
		{
			return SCRIPT_OVERRIDE;
		}
		
		String datatable = getStringObjVar (self, "loot_kit");
		
		String itemObjVar = dataTableGetString(datatable, 0, "item_objvar_name");
		
		int kitPartTemplateRowNum = -1;
		if (dataTableHasColumn (datatable, "useKitPartTemplate"))
		{
			String itemTemplate = getTemplateName(item);
			if (!itemTemplate.equals(""))
			{
				kitPartTemplateRowNum = dataTableSearchColumnForString(itemTemplate, "useKitPartTemplate", datatable);
			}
		}
		if (kitPartTemplateRowNum > -1)
		{
			String kitPartObjVar = dataTableGetString(datatable, kitPartTemplateRowNum, "objvar_name");
			if (getIntObjVar(self, kitPartObjVar) == 1)
			{
				sendSystemMessage(transferer, ALREADY_CONTAINS);
				return SCRIPT_OVERRIDE;
			}
		}
		else if (hasObjVar(item, itemObjVar))
		{
			
			if (!hasScript(item, "object.autostack"))
			{
				
				attachScript(item, "object.autostack");
				
				sendSystemMessage(transferer, AUTOSTACK_ATTACHED_EJECTED);
				return SCRIPT_OVERRIDE;
			}
			
			int itemNumber = getIntObjVar(item, itemObjVar);
			
			if (itemNumber == 1)
			{
				if (getIntObjVar(self, "collectible.item_contained_01") == 1)
				{
					sendSystemMessage(transferer, ALREADY_CONTAINS);
					return SCRIPT_OVERRIDE;
				}
			}
			else if (itemNumber == 2)
			{
				if (getIntObjVar(self, "collectible.item_contained_02") == 1)
				{
					sendSystemMessage(transferer, ALREADY_CONTAINS);
					return SCRIPT_OVERRIDE;
				}
			}
			else if (itemNumber == 3)
			{
				if (getIntObjVar(self, "collectible.item_contained_03") == 1)
				{
					sendSystemMessage(transferer, ALREADY_CONTAINS);
					return SCRIPT_OVERRIDE;
				}
			}
			else if (itemNumber == 4)
			{
				if (getIntObjVar(self, "collectible.item_contained_04") == 1)
				{
					sendSystemMessage(transferer, ALREADY_CONTAINS);
					return SCRIPT_OVERRIDE;
				}
			}
			else if (itemNumber == 5)
			{
				if (getIntObjVar(self, "collectible.item_contained_05") == 1)
				{
					sendSystemMessage(transferer, ALREADY_CONTAINS);
					return SCRIPT_OVERRIDE;
				}
			}
			else if (itemNumber == 6)
			{
				if (getIntObjVar(self, "collectible.item_contained_06") == 1)
				{
					sendSystemMessage(transferer, ALREADY_CONTAINS);
					return SCRIPT_OVERRIDE;
				}
			}
			else if (itemNumber == 7)
			{
				if (getIntObjVar(self, "collectible.item_contained_07") == 1)
				{
					sendSystemMessage(transferer, ALREADY_CONTAINS);
					return SCRIPT_OVERRIDE;
				}
			}
			else if (itemNumber == 8)
			{
				if (getIntObjVar(self, "collectible.item_contained_08") == 1)
				{
					sendSystemMessage(transferer, ALREADY_CONTAINS);
					return SCRIPT_OVERRIDE;
				}
			}
			else if (itemNumber == 9)
			{
				if (getIntObjVar(self, "collectible.item_contained_09") == 1)
				{
					sendSystemMessage(transferer, ALREADY_CONTAINS);
					return SCRIPT_OVERRIDE;
				}
			}
			else if (itemNumber == 10)
			{
				if (getIntObjVar(self, "collectible.item_contained_10") == 1)
				{
					sendSystemMessage(transferer, ALREADY_CONTAINS);
					return SCRIPT_OVERRIDE;
				}
			}
		}
		else
		{
			sendSystemMessage(transferer, INCORRECT_ITEM);
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnReceivedItem(obj_id self, obj_id srcContainer, obj_id transferer, obj_id item) throws InterruptedException
	{
		String datatable = getStringObjVar (self, "loot_kit");
		
		String itemObjVar = dataTableGetString(datatable, 0, "item_objvar_name");
		
		int itemNumber = getIntObjVar(item, itemObjVar);
		
		int kitPartTemplateRowNum = -1;
		if (dataTableHasColumn (datatable, "useKitPartTemplate"))
		{
			String itemTemplate = getTemplateName(item);
			if (!itemTemplate.equals(""))
			{
				kitPartTemplateRowNum = dataTableSearchColumnForString(itemTemplate, "useKitPartTemplate", datatable);
			}
		}
		
		if (kitPartTemplateRowNum > -1)
		{
			String kitPartObjVar = dataTableGetString(datatable, kitPartTemplateRowNum, "objvar_name");
			setObjVar(self, kitPartObjVar, 1);
		}
		else if (itemNumber == 1)
		{
			setObjVar(self, "collectible.item_contained_01", 1);
		}
		else if (itemNumber == 2)
		{
			setObjVar(self, "collectible.item_contained_02", 1);
		}
		else if (itemNumber == 3)
		{
			setObjVar(self, "collectible.item_contained_03", 1);
		}
		else if (itemNumber == 4)
		{
			setObjVar(self, "collectible.item_contained_04", 1);
		}
		else if (itemNumber == 5)
		{
			setObjVar(self, "collectible.item_contained_05", 1);
		}
		else if (itemNumber == 6)
		{
			setObjVar(self, "collectible.item_contained_06", 1);
		}
		else if (itemNumber == 7)
		{
			setObjVar(self, "collectible.item_contained_07", 1);
		}
		else if (itemNumber == 8)
		{
			setObjVar(self, "collectible.item_contained_08", 1);
		}
		else if (itemNumber == 9)
		{
			setObjVar(self, "collectible.item_contained_09", 1);
		}
		else if (itemNumber == 10)
		{
			setObjVar(self, "collectible.item_contained_10", 1);
		}
		
		sendSystemMessage(transferer, ITEM_USED);
		
		decrementCount(item);
		
		obj_id[] contents = getContents(self);
		if (contents != null && contents.length > 0)
		{
			obj_id playerInv = utils.getInventoryContainer(transferer);
			for (int i = 0; i < contents.length; i++)
			{
				testAbortScript();
				putInOverloaded(contents[i], playerInv);
			}
		}
		
		checkForCompletion(self, transferer);
		return SCRIPT_CONTINUE;
	}
	
	
	public void checkForCompletion(obj_id self, obj_id player) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			return;
		}
		
		if (getIntObjVar(self, "collectible.item_contained_01") == 0)
		{
			return;
		}
		
		if (getIntObjVar(self, "collectible.item_contained_02") == 0)
		{
			return;
		}
		
		if (getIntObjVar(self, "collectible.item_contained_03") == 0)
		{
			return;
		}
		
		if (getIntObjVar(self, "collectible.item_contained_04") == 0)
		{
			return;
		}
		
		if (getIntObjVar(self, "collectible.item_contained_05") == 0)
		{
			return;
		}
		
		if (getIntObjVar(self, "collectible.item_contained_06") == 0)
		{
			return;
		}
		
		if (getIntObjVar(self, "collectible.item_contained_07") == 0)
		{
			return;
		}
		
		if (getIntObjVar(self, "collectible.item_contained_08") == 0)
		{
			return;
		}
		
		if (getIntObjVar(self, "collectible.item_contained_09") == 0)
		{
			return;
		}
		
		if (getIntObjVar(self, "collectible.item_contained_10") == 0)
		{
			return;
		}
		
		obj_id playerInv = getObjectInSlot( player, "inventory");
		if (isIdValid(playerInv))
		{
			String datatable = getStringObjVar (self, "loot_kit");
			String rewardItem = dataTableGetString(datatable, 0, "item_reward_name");
			obj_id item = createObject( rewardItem, playerInv, "");
			if (isIdValid(item))
			{
				sendSystemMessage(player, NEW_ITEM_CREATED);
				utils.setScriptVar(self, "cleaningUp", true);
				destroyObject(self);
			}
		}
		return;
	}
}
