package script.systems.collections;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.collection;
import script.library.groundquests;
import script.library.sui;
import script.library.utils;


public class grant_collection_quest extends script.base_script
{
	public grant_collection_quest()
	{
	}
	public static final boolean LOGGING_ENABLED = true;
	
	public static final String STF = "som/som_quest";
	public static final string_id EXAMINE = new string_id(STF, "blistmok_heart_examine");
	public static final string_id DESTROY = new string_id(STF, "blistmok_heart_destroy");
	public static final string_id ALREADY = new string_id(STF, "blistmok_heart_already");
	public static final string_id UNABLE = new string_id(STF, "unable_to_examine");
	public static final string_id DECLINE = new string_id(STF, "quest_decline");
	
	public static final String QUEST_OBJVAR = "collection.quest";
	
	
	public void blog(String identifier, String text) throws InterruptedException
	{
		if (LOGGING_ENABLED)
		{
			LOG(identifier, text);
		}
	}
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		if (!isIdValid(player) || !exists(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (utils.getContainingPlayer(self) == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		blog("COLLECTIONS", "Menu Request ");
		
		int menu = mi.addRootMenu(menu_info_types.ITEM_USE, EXAMINE);
		
		menu_info_data mid = mi.getMenuItemByType(menu_info_types.ITEM_USE);
		if (mid == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		mid.setServerNotify(true);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		blog("COLLECTIONS", "OnObjectMenuSelect");
		
		if (!isIdValid(player) || !exists(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (item != menu_info_types.ITEM_USE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (utils.getContainingPlayer(self) == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		blog("COLLECTIONS", "initial checks done");
		
		String quest = getStringObjVar(self, QUEST_OBJVAR);
		if (quest == null || quest.equals(""))
		{
			return SCRIPT_CONTINUE;
		}
		
		blog("COLLECTIONS", "obj objvar: "+quest);
		
		if (!groundquests.isQuestActive(player, quest))
		{
			collection.activateQuestAcceptSUI(player, self);
		}
		else
		{
			sendSystemMessage(player, ALREADY);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleQuestOfferResponse(obj_id self, dictionary params) throws InterruptedException
	{
		blog("COLLECTIONS", "handleQuestOfferResponse");
		
		if ((params == null) || (params.isEmpty()))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		int bp = sui.getIntButtonPressed(params);
		
		String quest = getStringObjVar(self, QUEST_OBJVAR);
		if (quest == null || quest.equals(""))
		{
			return SCRIPT_CONTINUE;
		}
		blog("COLLECTIONS", "handleQuestOfferResponse "+ quest);
		blog("COLLECTIONS", "player: "+ player);
		
		switch(bp)
		{
			case sui.BP_OK:	
			groundquests.grantQuest(player, quest);
			sendSystemMessage(player, DESTROY);
			destroyObject(self);
			break;
			case sui.BP_CANCEL: 
			sendSystemMessage(player, DECLINE);
			break;
		}
		return SCRIPT_CONTINUE;
	}
}
