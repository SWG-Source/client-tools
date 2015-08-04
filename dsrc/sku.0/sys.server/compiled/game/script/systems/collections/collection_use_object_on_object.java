package script.systems.collections;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.buff;
import script.library.collection;
import script.library.groundquests;
import script.library.static_item;
import script.library.sui;
import script.library.utils;


public class collection_use_object_on_object extends script.base_script
{
	public collection_use_object_on_object()
	{
	}
	public static final string_id SID_INVALID_TARGET = new string_id("collection", "invalid_target");
	public static final string_id NOT_WHILE_INCAPPED = new string_id ("quest/ground/util/quest_giver_object", "not_while_incapped");
	public static final string_id MUST_DISMOUNT = new string_id ("collection", "must_dismount");
	public static final string_id CREATURE_IS_DEAD = new string_id ("collection", "creature_is_dead");
	public static final string_id OBJECT_DESTROYED = new string_id ("collection", "object_destroyed");
	
	public static final String OBJECT_KEY = "quest.object_key";
	public static final String COLLECTION_OBJECT_TABLE = "datatables/collection/use_object_on_object.iff";
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		if (!canManipulate(player, self, true, true, 15, true))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!utils.isNestedWithinAPlayer(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		menu_info_data mid = mi.getMenuItemByType(menu_info_types.ITEM_USE);
		if (mid != null)
		{
			mid.setServerNotify(true);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		sendDirtyObjectMenuNotification(self);
		
		if (utils.getContainingPlayer(self) != player)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (item != menu_info_types.ITEM_USE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (isIncapacitated(player) || isDead(player))
		{
			sendSystemMessage(player, new string_id("player_structure", "while_dead"));
			return SCRIPT_CONTINUE;
		}
		if (getState(player, STATE_RIDING_MOUNT) == 1)
		{
			sendSystemMessage(player, MUST_DISMOUNT);
			return SCRIPT_CONTINUE;
		}
		if (isDead(player) || isIncapacitated(player))
		{
			sendSystemMessage(player, NOT_WHILE_INCAPPED);
			return SCRIPT_CONTINUE;
		}
		
		obj_id intended = getIntendedTarget(player);
		
		if (!isIdValid(intended))
		{
			
			sendSystemMessage(player, SID_INVALID_TARGET);
		}
		else
		{
			setObjVar(self, "owner", player);
			
			checkValidTarget(player, intended, self);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean checkValidTarget(obj_id player, obj_id target, obj_id self) throws InterruptedException
	{
		
		if (!isValidId(player) || !exists(player))
		{
			return false;
		}
		
		if (!isValidId(target) || !exists(target))
		{
			return false;
		}
		
		if (!isValidId(self) || !exists(self))
		{
			return false;
		}
		
		if (!hasObjVar(self, OBJECT_KEY))
		{
			if (hasScript(self, "item.special.nodestroy"))
			{
				detachScript(self, "item.special.nodestroy");
			}
			messageTo(self, "handleDestroy", null, 0, false);
			return false;
		}
		
		String keyString = getStringObjVar(self, OBJECT_KEY);
		if (keyString == null || keyString.length() <= 0)
		{
			if (hasScript(self, "item.special.nodestroy"))
			{
				detachScript(self, "item.special.nodestroy");
			}
			messageTo(self, "handleDestroy", null, 0, false);
			return false;
		}
		
		int tableRow = dataTableSearchColumnForString(keyString, "objectKey", COLLECTION_OBJECT_TABLE);
		if (tableRow < 0)
		{
			return false;
		}
		
		dictionary collectionDataDict = dataTableGetRow(COLLECTION_OBJECT_TABLE, tableRow);
		if (collectionDataDict == null)
		{
			return false;
		}
		
		String collectionName = collectionDataDict.getString("collectionName");
		if (collectionName == null || collectionName.length() <= 0)
		{
			return false;
		}
		
		int questRequired = collectionDataDict.getInt("questRequired");
		
		String questName = collectionDataDict.getString("questName");
		
		String taskName = collectionDataDict.getString("taskName");
		
		if (questRequired > 0 && ((questName == null || questName.length() <= 0) || (taskName == null || taskName.length() <= 0)))
		{
			return false;
		}
		
		if (hasCompletedCollection(player, collectionName))
		{
			messageTo(self, "handleDestroy", null, 0, false);
			return false;
		}
		
		if (hasCompletedCollection(player, collectionName))
		{
			if (hasScript(self, "item.special.nodestroy"))
			{
				detachScript(self, "item.special.nodestroy");
			}
			messageTo(self, "handleDestroy", null, 0, false);
			return false;
			
		}
		
		if (questRequired > 0)
		{
			if (!groundquests.isQuestActive(player, questName))
			{
				if (hasScript(self, "item.special.nodestroy"))
				{
					detachScript(self, "item.special.nodestroy");
				}
				messageTo(self, "handleDestroy", null, 0, false);
				return false;
			}
			if (!groundquests.isTaskActive(player, questName, taskName))
			{
				if (hasScript(self, "item.special.nodestroy"))
				{
					detachScript(self, "item.special.nodestroy");
				}
				messageTo(self, "handleDestroy", null, 0, false);
				return false;
			}
		}
		
		if (!keyString.equals(getStringObjVar(target, OBJECT_KEY)))
		{
			return false;
		}
		
		if (isMob(target) && (isDead(target) || isIncapacitated(target)))
		{
			sendSystemMessage(player, CREATURE_IS_DEAD);
			return false;
		}
		
		String collectonSlot = collectionDataDict.getString("collectonSlot");
		if (collectonSlot != null && collectonSlot.length() > 0)
		{
			
			String[] allSlots = split(collectonSlot, ',');
			for (int j = 0; j < allSlots.length; j++)
			{
				testAbortScript();
				if (!hasCompletedCollectionSlotPrereq(player, allSlots[j]))
				{
					
					break;
				}
				if (hasCompletedCollectionSlot(player, allSlots[j]))
				{
					
					continue;
				}
				
				modifyCollectionSlotValue(player, allSlots[j], 1);
				
				String handleTarget = collectionDataDict.getString("handleTarget");
				if (handleTarget != null && handleTarget.length() > 0)
				{
					messageTo(target, handleTarget, null, 0, false);
				}
				break;
			}
		}
		return true;
	}
	
	
	public int handleDestroy(obj_id self, dictionary params) throws InterruptedException
	{
		if (hasObjVar(self, "owner"))
		{
			obj_id player = getObjIdObjVar(self, "owner");
			sendSystemMessage(player, OBJECT_DESTROYED);
		}
		destroyObject(self);
		return SCRIPT_CONTINUE;
	}
}
