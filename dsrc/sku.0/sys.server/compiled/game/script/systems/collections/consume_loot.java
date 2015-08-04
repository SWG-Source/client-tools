package script.systems.collections;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.collection;
import script.library.sui;
import script.library.utils;


public class consume_loot extends script.base_script
{
	public consume_loot()
	{
	}
	public static final String PID_NAME = "collectionConsume";
	public static final String SCRIPTVAR_LIST = "collections.availableCollections";
	
	public static final string_id SID_CONSUME_PROMPT = new string_id("collection", "consume_item_prompt");
	public static final string_id SID_CONSUME_TITLE = new string_id("collection", "consume_item_title");
	public static final string_id SID_CONSUME_ITEM = new string_id("collection", "consume_item");
	public static final string_id SID_REPORT_CONSUME_ITEM_FAIL = new string_id("collection", "report_consume_item_fail");
	public static final string_id SID_ALREADY_HAVE_SLOT = new string_id("collection", "already_have_slot");
	public static final string_id SID_ALREADY_FINISHED_COLLECTION = new string_id("collection", "already_finished_collection");
	public static final string_id STR_COLLECTION_LIST_PROMPT = new string_id("collection", "collection_list_prompt");
	public static final string_id STR_COLLECTION_LIST_TITLE = new string_id("collection", "collection_list_title");
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		
		obj_id collectionItem = self;
		
		if (utils.isNestedWithinAPlayer(collectionItem))
		{
			mi.addRootMenu (menu_info_types.SERVER_MENU3, SID_CONSUME_ITEM);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		sendDirtyObjectMenuNotification(self);
		
		obj_id collectionItem = self;
		CustomerServiceLog("CollectionConsume: ", "collection OnObjectMenuSelect worked");
		
		if (item == menu_info_types.SERVER_MENU3 && utils.isNestedWithinAPlayer(collectionItem))
		{
			if (sui.hasPid(player, PID_NAME))
			{
				int pid = sui.getPid(player, PID_NAME);
				forceCloseSUIPage(pid);
			}
			CustomerServiceLog("CollectionConsume: ", "collection OnObjectMenuSelect nested in player worked");
			
			String[] availableCollections = collection.getAllAvailableCollectionsForItem(player, self);
			if (availableCollections.length >= 1)
			{
				CustomerServiceLog("CollectionConsume: ", "collection OnObjectMenuSelect getAllAvailableCollectionsForItem worked");
				
				boolean success = getUiConsumeMessageBox(self, player);
				return SCRIPT_CONTINUE;
			}
			else
			{
				
				sendSystemMessage(player, collection.SID_NEED_TO_ACTIVATE_COLLECTION);
				return SCRIPT_CONTINUE;
			}
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
		
		if (exists(self) && !hasObjVar(self, "relic.doNotShowAsCollectionItem"))
		{
			String[] collectionsForItem = collection.getAllCollectionsForItem(self);
			String[] slotsForItem = collection.getAllSlotsForItem(self);
			
			if (collectionsForItem != null && collectionsForItem.length > 0)
			{
				for (int i = 0; i < collectionsForItem.length; ++i)
				{
					testAbortScript();
					names[idx] = "collection_name";
					if (!hasCompletedCollection(player,collectionsForItem[i]))
					{
						if (!hasCompletedCollectionSlot(player, slotsForItem[i]))
						{
							attribs[idx] = "@collection_n:"+collectionsForItem[i] + "_unfinished";
						}
						else
						{
							attribs[idx] = "@collection_n:"+collectionsForItem[i] + "_finished";
						}
					}
					else
					{
						attribs[idx] = "@collection_n:"+collectionsForItem[i] + "_finished";
					}
					idx++;
				}
			}
		}
		return super.OnGetAttributes(self, player, names, attribs);
	}
	
	
	public int handlerSuiAddToCollection(obj_id self, dictionary params) throws InterruptedException
	{
		
		obj_id collectionItem = self;
		
		if (params == null || params.isEmpty())
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		
		if (!isIdValid(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		int bp = sui.getIntButtonPressed(params);
		
		if (bp == sui.BP_CANCEL)
		{
			sui.removePid(player, PID_NAME);
			return SCRIPT_CONTINUE;
		}
		
		if (!sui.hasPid(player, PID_NAME))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!utils.isNestedWithin(self, player))
		{
			sui.removePid(player, PID_NAME);
			CustomerServiceLog("CollectionConsume: ", "collectionItem ("+ collectionItem + ")"+ " was NOT consumed into a collection, for player "+ getFirstName(player) + "("+ player + "). collectionItem("+ collectionItem + "). Player attempted to collect an item that was not in their inventory.");
			return SCRIPT_CONTINUE;
		}
		
		String[] availableCollections = collection.getAllAvailableCollectionsForItem(player, self);
		String[] availableSlots = collection.getAllAvailableSlotsForItem(player, self);
		
		String slotName = "";
		String collectionName = "";
		
		if (availableCollections.length < 1)
		{
			CustomerServiceLog("CollectionConsume: ", "collectionItem ("+ collectionItem + ")"+ " was NOT consumed into a collection, for player "+ getFirstName(player) + "("+ player + "). collectionItem("+ collectionItem + "). Player did not have the collection activated.");
			sui.removePid(player, PID_NAME);
			
			sendSystemMessage(player, collection.SID_NEED_TO_ACTIVATE_COLLECTION);
			return SCRIPT_CONTINUE;
		}
		
		if (availableCollections != null && availableCollections.length > 1)
		{
			
			String[] dataStored = (String[])availableCollections.clone();
			utils.setScriptVar(player, SCRIPTVAR_LIST, dataStored);
			
			for (int i = 0; i < availableCollections.length; ++i)
			{
				testAbortScript();
				availableCollections[i] = utils.packStringId(new string_id("collection", availableCollections[i]));
			}
			
			int pid = sui.listbox(self, player, "@"+ STR_COLLECTION_LIST_PROMPT, sui.OK_CANCEL, "@"+ STR_COLLECTION_LIST_TITLE, availableCollections, "onCollectionListResponse", true, false);
			sui.setPid(player, pid, PID_NAME);
			return SCRIPT_CONTINUE;
		}
		
		else if (availableCollections != null && availableCollections.length > 0)
		{
			slotName = availableSlots[0];
			collectionName = availableCollections[0];
		}
		
		if (!hasCompletedCollection(player, collectionName))
		{
			
			if (!hasCompletedCollectionSlot(player, slotName) && !slotName.equals(""))
			{
				
				if (modifyCollectionSlotValue(player, slotName, 1))
				{
					CustomerServiceLog("CollectionConsume: ", "collectionItem ("+ collectionItem + ")"+ " was consumed into a collection, for player "+ getFirstName(player) + "("+ player + "). collectionItem("+ collectionItem + ") will now be destroyed.");
					decrementCount(collectionItem);
				}
				else
				{
					
					CustomerServiceLog("CollectionConsume: ", "collectionItem ("+ collectionItem + ")"+ " was NOT consumed into a collection, for player "+ getFirstName(player) + "("+ player + "). collectionItem("+ collectionItem + ") will NOT be destroyed.");
					
					sendSystemMessage(player, SID_REPORT_CONSUME_ITEM_FAIL);
				}
				sui.removePid(player, PID_NAME);
				return SCRIPT_CONTINUE;
			}
			else
			{
				
				sendSystemMessage(player, SID_ALREADY_HAVE_SLOT);
				sui.removePid(player, PID_NAME);
				return SCRIPT_CONTINUE;
			}
		}
		else
		{
			
			sendSystemMessage(player, SID_ALREADY_FINISHED_COLLECTION);
		}
		sui.removePid(player, PID_NAME);
		return SCRIPT_CONTINUE;
	}
	
	
	public int onCollectionListResponse(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null || params.isEmpty())
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id collectionItem = self;
		
		obj_id player = sui.getPlayerId(params);
		
		if (!isIdValid(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		int bp = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (bp == sui.BP_CANCEL)
		{
			sui.removePid(player, PID_NAME);
			return SCRIPT_CONTINUE;
		}
		
		if (idx < 0)
		{
			
			sui.removePid(player, PID_NAME);
			return SCRIPT_CONTINUE;
		}
		
		if (!sui.hasPid(player, PID_NAME))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!utils.isNestedWithin(self, player))
		{
			sui.removePid(player, PID_NAME);
			CustomerServiceLog("CollectionConsume: ", "collectionItem ("+ collectionItem + ")"+ " was NOT consumed into a collection, for player "+ getFirstName(player) + "("+ player + "). collectionItem("+ collectionItem + "). Player attempted to collect an item that was not in their inventory.");
			return SCRIPT_CONTINUE;
		}
		
		String[] availableCollections = utils.getStringArrayScriptVar(player, SCRIPTVAR_LIST);
		String[] baseSlotNames = split(getStringObjVar(collectionItem, collection.OBJVAR_SLOT_NAME), '|');
		
		utils.removeScriptVarTree(player, "collections");
		
		String slotName = "";
		String collectionName = availableCollections[idx];
		
		for (int i = 0; i < baseSlotNames.length; ++i)
		{
			testAbortScript();
			boolean found = false;
			
			String[] splitSlotNames = split(baseSlotNames[i], ':');
			
			for (int j = 0; j < splitSlotNames.length; j += 2)
			{
				testAbortScript();
				if (collectionName.equals(splitSlotNames[j]))
				{
					
					if (!hasCompletedCollection(player, collectionName))
					{
						
						if (!hasCompletedCollectionSlot(player, splitSlotNames[j + 1]))
						{
							
							slotName = splitSlotNames[j + 1];
							found = true;
							break;
						}
					}
				}
			}
			if (found)
			{
				break;
			}
		}
		
		if (!hasCompletedCollection(player, collectionName))
		{
			
			if (!hasCompletedCollectionSlot(player, slotName) && !slotName.equals(""))
			{
				
				if (modifyCollectionSlotValue(player, slotName, 1))
				{
					CustomerServiceLog("CollectionConsume: ", "collectionItem ("+ collectionItem + ")"+ " was consumed into a collection, for player "+ getFirstName(player) + "("+ player + "). collectionItem("+ collectionItem + ") will now be destroyed.");
					decrementCount(collectionItem);
				}
				else
				{
					
					CustomerServiceLog("CollectionConsume: ", "collectionItem ("+ collectionItem + ")"+ " was NOT consumed into a collection, for player "+ getFirstName(player) + "("+ player + "). collectionItem("+ collectionItem + ") will NOT be destroyed.");
					
					sendSystemMessage(player, SID_REPORT_CONSUME_ITEM_FAIL);
				}
				sui.removePid(player, PID_NAME);
				return SCRIPT_CONTINUE;
			}
			else
			{
				
				sendSystemMessage(player, SID_ALREADY_HAVE_SLOT);
				sui.removePid(player, PID_NAME);
				return SCRIPT_CONTINUE;
			}
		}
		else
		{
			
			sendSystemMessage(player, SID_ALREADY_FINISHED_COLLECTION);
		}
		sui.removePid(player, PID_NAME);
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean getUiConsumeMessageBox(obj_id self, obj_id player) throws InterruptedException
	{
		if (!isValidId(self) || !isValidId(player))
		{
			return false;
		}
		CustomerServiceLog("CollectionConsume: ", "collection getUiConsumeMessageBox worked");
		
		String questName = getStringObjVar(self, "collection.questName");
		if (questName == null || questName.equals(""))
		{
			int pid = sui.msgbox(self, player, "@"+ SID_CONSUME_PROMPT, sui.YES_NO, "@"+ SID_CONSUME_TITLE, "handlerSuiAddToCollection");
			sui.setPid(player, pid, PID_NAME);
			return true;
		}
		CustomerServiceLog("CollectionConsume: ", "collection questName: "+questName);
		
		string_id questCollectionString = new string_id("collection", questName);
		
		int pid = sui.msgbox(self, player, "@"+ questCollectionString + "_d", sui.YES_NO, "@"+ questCollectionString + "_n", "handlerSuiAddToCollection");
		sui.setPid(player, pid, PID_NAME);
		return true;
		
	}
}
