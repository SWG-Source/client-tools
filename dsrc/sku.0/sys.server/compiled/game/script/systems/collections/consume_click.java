package script.systems.collections;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.collection;
import script.library.prose;
import script.library.stealth;
import script.library.sui;
import script.library.utils;


public class consume_click extends script.base_script
{
	public consume_click()
	{
	}
	public static final String OBJVAR_SLOT_NAME = "collection.slotName";
	public static final String VAR_ACCESS_DELAY = "delay.access_delay";
	public static final String COLLECTION_HANDLER = "modifySlot";
	public static final String NEWBIE_COMM = "collection.newbie_comm_series";
	
	public static final int MAX_RANGE_TO_COLLECT = 3;
	public static final int COLLECT_TIMER_BEGIN = 0;
	public static final int COLLECT_TIMER_END = 5;
	public static final int COLLECT_TIMER_DELAY = COLLECT_TIMER_END + 3;
	
	public static final string_id SID_CONSUME_PROMPT = new string_id("collection", "click_item_prompt");
	public static final string_id SID_CONSUME_TITLE = new string_id("collection", "consume_item_title");
	public static final string_id SID_CONSUME_ITEM = new string_id("collection", "consume_item");
	public static final string_id SID_REPORT_CONSUME_ITEM_FAIL = new string_id("collection", "report_consume_item_fail");
	public static final string_id SID_ALREADY_HAVE_SLOT = new string_id("collection", "already_have_slot");
	public static final string_id SID_ALREADY_FINISHED_COLLECTION = new string_id("collection", "already_finished_collection");
	public static final string_id COUNTDOWN_TIMER = new string_id("collection", "click_countdown_timer");
	public static final string_id SID_COUNTDOWN_LOCOMOTION = new string_id("quest/groundquests", "countdown_interrupted_locomotion");
	public static final string_id SID_INTERRUPTED_INCAPACITATED = new string_id("quest/groundquests", "countdown_interrupted_incapacitated");
	public static final string_id SID_INTERRUPTED_DAMAGED = new string_id("quest/groundquests", "countdown_interrupted_damaged");
	public static final string_id SID_NOT_CLOSE_ENOUGH = new string_id("collection", "not_close_enough");
	public static final string_id SID_INVIS_COLLECTION_FAIL = new string_id("collection", "invis_collection_failed");
	public static final string_id SID_CONTROL_ACCESS_DELAY = new string_id("collection", "access_delay");
	public static final string_id SID_GCW_SENSITIVE_DATA = new string_id("collection", "gcw_sensitive_data");
	public static final string_id SID_GCW_NO_DATA = new string_id("collection", "gcw_no_data");
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		
		obj_id collectionItem = self;
		
		if (hasObjVar(collectionItem, "useStringFile") && hasObjVar(collectionItem, "useMenu"))
		{
			String useStringFile = getStringObjVar(collectionItem, "useStringFile");
			if (useStringFile != null && useStringFile.length() > 0)
			{
				
				String useMenu = getStringObjVar(collectionItem, "useMenu");
				if (useMenu != null && useMenu.length() > 0)
				{
					string_id thisStringId = new string_id(useStringFile, useMenu);
					mi.addRootMenu(menu_info_types.ITEM_USE, thisStringId);
				}
			}
		}
		else
		{
			mi.addRootMenu (menu_info_types.ITEM_USE, SID_CONSUME_ITEM);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		sendDirtyObjectMenuNotification(self);
		
		obj_id collectionItem = self;
		
		if (item != menu_info_types.ITEM_USE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (hasObjVar(collectionItem, NEWBIE_COMM))
		{
			collection.giveNewbieCommWindow(player, self);
		}
		
		boolean isGCW = false;
		if (hasObjVar(collectionItem, "collection.gcw_control_check"))
		{
			
			if (!collection.gcwBaseControlCheck(collectionItem, player))
			{
				sendSystemMessage(player, SID_GCW_SENSITIVE_DATA);
				return SCRIPT_CONTINUE;
			}
			else
			{
				
				if (utils.hasScriptVar(collectionItem, "collection.gcw_lockout_time"))
				{
					sendSystemMessage(player, SID_GCW_NO_DATA);
					return SCRIPT_CONTINUE;
				}
				else
				{
					isGCW = true;
				}
			}
		}
		
		if (utils.hasScriptVar(player, VAR_ACCESS_DELAY))
		{
			int accessTime = utils.getIntScriptVar(player, VAR_ACCESS_DELAY);
			int gameTime = getGameTime();
			LOG("collection","Var Delay Exists. accessTime: "+ accessTime);
			
			if ((accessTime + COLLECT_TIMER_DELAY) > gameTime)
			{
				prose_package pp = new prose_package();
				pp = prose.setStringId(pp, SID_CONTROL_ACCESS_DELAY);
				pp = prose.setDI(pp, (accessTime + COLLECT_TIMER_DELAY) - gameTime);
				
				sendSystemMessageProse(player, pp);
				
				return SCRIPT_CONTINUE;
			}
			else
			{
				utils.removeScriptVar(player, VAR_ACCESS_DELAY);
			}
		}
		
		String baseSlotName = getStringObjVar(collectionItem, OBJVAR_SLOT_NAME);
		
		String[] splitSlotNames = split(baseSlotName, ':');
		
		String slotName = splitSlotNames[1];
		String collectionName = splitSlotNames[0];
		
		if (isGCW && utils.hasScriptVar(collectionItem, "collection.gcwSlotName"))
		{
			slotName = utils.getStringScriptVar(collectionItem, "collection.gcwSlotName");
			utils.removeScriptVar(collectionItem, "collection.gcwSlotName");
		}
		
		if (!hasCompletedCollectionSlotPrereq(player, slotName))
		{
			
			sendSystemMessage(player, collection.SID_NEED_TO_ACTIVATE_COLLECTION);
			return SCRIPT_CONTINUE;
		}
		
		if (hasCompletedCollection(player, collectionName))
		{
			
			sendSystemMessage(player, SID_ALREADY_FINISHED_COLLECTION);
			return SCRIPT_CONTINUE;
		}
		
		if ((slotName == null || slotName.equals("")) || hasCompletedCollectionSlot(player, slotName))
		{
			
			sendSystemMessage(player, SID_ALREADY_HAVE_SLOT);
			return SCRIPT_CONTINUE;
		}
		
		boolean currentState = collection.checkState(player);
		if (!currentState)
		{
			return SCRIPT_CONTINUE;
		}
		
		boolean isCloseEnough = collection.checkDistance(self, player, MAX_RANGE_TO_COLLECT);
		if (!isCloseEnough)
		{
			LOG("collection","Not Close ENOUGH");
			sendSystemMessage(player, SID_NOT_CLOSE_ENOUGH);
			return SCRIPT_CONTINUE;
		}
		
		LOG("collection","Player collecting "+ slotName);
		
		int flags = 0;
		
		flags |= sui.CD_EVENT_INCAPACITATE;
		flags |= sui.CD_EVENT_DAMAGED;
		flags |= sui.CD_EVENT_COMBAT;
		flags |= sui.CD_EVENT_LOCOMOTION;
		
		stealth.testInvisNonCombatAction(player, self);
		collection.giveAreaMobsHate(self, player);
		
		int collectTimerEnd = COLLECT_TIMER_END;
		
		if (hasObjVar(self, "collection.timerLength"))
		{
			collectTimerEnd = getIntObjVar(self, "collection.timerLength");
		}
		
		if (utils.hasScriptVar(player, "collection.qa.clickBypass"))
		{
			collectTimerEnd = 1;
		}
		
		if (collectTimerEnd > 0)
		{
			int pid = sui.smartCountdownTimerSUI(player, player, "quest_countdown_timer", COUNTDOWN_TIMER, COLLECT_TIMER_BEGIN, collectTimerEnd, COLLECTION_HANDLER, MAX_RANGE_TO_COLLECT, flags);
			sui.setPid(player, pid, collection.CONSUME_PID_NAME);
			utils.setScriptVar(player, "col.id", pid);
			utils.setScriptVar(player, "col.collectionItem", collectionItem);
			utils.setScriptVar(player, "col.slotName", slotName);
			
			if (isGCW)
			{
				utils.setScriptVar(player, "col.isGCW", 1);
			}
		}
		else
		{
			
			if (modifyCollectionSlotValue(player, slotName, 1))
			{
				CustomerServiceLog("CollectionConsume: ", "collectionItem ("+ collectionItem + ")"+ " was consumed into a collection, for player "+ getFirstName(player) + "("+ player + ").");
			}
			else
			{
				
				CustomerServiceLog("CollectionConsume: ", "collectionItem ("+ collectionItem + ")"+ " was NOT consumed into a collection, for player "+ getFirstName(player) + "("+ player + ").");
				
				sendSystemMessage(player, SID_REPORT_CONSUME_ITEM_FAIL);
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int removeGcwCollectionLockout(obj_id self, dictionary params) throws InterruptedException
	{
		utils.removeScriptVar(self, "collection.gcw_lockout_time");
		return SCRIPT_CONTINUE;
	}
}
