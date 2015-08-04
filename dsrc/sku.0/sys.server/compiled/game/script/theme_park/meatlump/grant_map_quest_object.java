package script.theme_park.meatlump;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.city;
import script.library.collection;
import script.library.locations;
import script.library.groundquests;
import script.library.sui;
import script.library.utils;


public class grant_map_quest_object extends script.base_script
{
	public grant_map_quest_object()
	{
	}
	public static final string_id RADIAL_INSPECT = new string_id ("quest/ground/util/quest_giver_object", "radial_inspect");
	public static final string_id OFFER_QUEST_MSG = new string_id ("quest/ground/util/quest_giver_object", "offer_quest");
	public static final string_id SUI_TITLE = new string_id ("quest/ground/util/quest_giver_object", "sui_title");
	public static final string_id BUTTON_DECLINE = new string_id ("quest/ground/util/quest_giver_object", "button_decline");
	public static final string_id BUTTON_ACCEPT = new string_id ("quest/ground/util/quest_giver_object", "button_accept");
	public static final string_id DECLINED_QUEST = new string_id ("quest/ground/util/quest_giver_object", "declined_quest");
	public static final string_id MUST_DISMOUNT = new string_id ("quest/ground/util/quest_giver_object", "must_dismount");
	public static final string_id PLAYER_IN_CELL = new string_id ("quest/ground/util/quest_giver_object", "player_in_cell");
	public static final string_id NOT_WHILE_INCAPPED = new string_id ("quest/ground/util/quest_giver_object", "not_while_incapped");
	public static final string_id SID_ELIGIBLE_FOR_GIFT_TITLE = new string_id ("quest/ground/util/quest_giver_object", "eligible_for_gift_title");
	public static final string_id SID_ELIGIBLE_FOR_GIFT_PROMPT = new string_id ("quest/ground/util/quest_giver_object", "eligible_for_gift_prompt");
	public static final string_id YOU_ALREADY_HAVE_QUEST = new string_id ("quest/ground/util/quest_giver_object", "you_already_have_quest");
	public static final string_id SID_NOT_WHILE_IN_COMBAT = new string_id("base_player", "not_while_in_combat");
	
	public static final String PID_NAME = "map_quest_pid";
	public static final String QUEST_NAME = "chapter7_publish_gift_activation";
	
	public static final String OBJECT_OBJVAR = "quest_string";
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		int menuOption = mi.addRootMenu (menu_info_types.ITEM_USE, RADIAL_INSPECT);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		if (!isIdValid(player) || !exists(player))
		{
			return SCRIPT_CONTINUE;
		}
		else if (!hasObjVar(self, OBJECT_OBJVAR))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (item == menu_info_types.ITEM_USE)
		{
			if (getState(player, STATE_COMBAT) > 0)
			{
				sendSystemMessage(player, SID_NOT_WHILE_IN_COMBAT);
				return SCRIPT_CONTINUE;
			}
			else if (getState(player, STATE_RIDING_MOUNT) == 1)
			{
				sendSystemMessage(player, MUST_DISMOUNT);
				return SCRIPT_CONTINUE;
			}
			else if (isDead(player) || isIncapacitated(player))
			{
				sendSystemMessage(player, NOT_WHILE_INCAPPED);
				return SCRIPT_CONTINUE;
			}
			
			String questString = getStringObjVar(self, OBJECT_OBJVAR);
			if (questString == null || questString.equals(""))
			{
				return SCRIPT_CONTINUE;
			}
			
			if (groundquests.isQuestActiveOrComplete(player, questString))
			{
				
				sendSystemMessage(player, YOU_ALREADY_HAVE_QUEST);
				return SCRIPT_CONTINUE;
			}
			
			int pid = sui.msgbox(self, player, "@"+ OFFER_QUEST_MSG, sui.YES_NO, "@"+ SUI_TITLE, "handlerGetQuest");
			sui.setPid(player, pid, PID_NAME);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int handlerGetQuest(obj_id self, dictionary params) throws InterruptedException
	{
		int btn = sui.getIntButtonPressed(params);
		
		obj_id player = sui.getPlayerId(params);
		if (btn == sui.BP_CANCEL)
		{
			sui.removePid(player, PID_NAME);
			return SCRIPT_CONTINUE;
		}
		if (!sui.hasPid(player, PID_NAME))
		{
			return SCRIPT_CONTINUE;
		}
		if (btn == sui.BP_OK)
		{
			
			String questString = getStringObjVar(self, OBJECT_OBJVAR);
			if (questString == null || questString.equals(""))
			{
				return SCRIPT_CONTINUE;
			}
			
			if (groundquests.hasCompletedQuest(player, questString) || groundquests.isQuestActive(player, questString))
			{
				
				sendSystemMessage(player, YOU_ALREADY_HAVE_QUEST);
				return SCRIPT_CONTINUE;
			}
			
			groundquests.grantQuest(player, questString);
			sui.removePid(player, PID_NAME);
		}
		return SCRIPT_CONTINUE;
	}
}
