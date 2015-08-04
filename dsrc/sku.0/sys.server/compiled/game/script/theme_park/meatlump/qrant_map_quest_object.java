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


public class qrant_map_quest_object extends script.base_script
{
	public qrant_map_quest_object()
	{
	}
	public static final string_id RADIAL_INSPECT = new string_id ("quest/ground/util/quest_giver_object", "radial_inspect");
	public static final string_id OFFER_QUEST_MSG = new string_id ("quest/ground/util/quest_giver_object", "offer_quest_teleport");
	public static final string_id SUI_TITLE = new string_id ("quest/ground/util/quest_giver_object", "sui_title");
	public static final string_id BUTTON_DECLINE = new string_id ("quest/ground/util/quest_giver_object", "button_decline");
	public static final string_id BUTTON_ACCEPT = new string_id ("quest/ground/util/quest_giver_object", "button_accept");
	public static final string_id DECLINED_QUEST = new string_id ("quest/ground/util/quest_giver_object", "declined_quest");
	public static final string_id MUST_DISMOUNT = new string_id ("quest/ground/util/quest_giver_object", "must_dismount");
	public static final string_id PLAYER_IN_CELL = new string_id ("quest/ground/util/quest_giver_object", "player_in_cell");
	public static final string_id NOT_WHILE_INCAPPED = new string_id ("quest/ground/util/quest_giver_object", "not_while_incapped");
	public static final string_id SID_ELIGIBLE_FOR_GIFT_TITLE = new string_id ("quest/ground/util/quest_giver_object", "eligible_for_gift_title");
	public static final string_id SID_ELIGIBLE_FOR_GIFT_PROMPT = new string_id ("quest/ground/util/quest_giver_object", "eligible_for_gift_prompt");
	public static final string_id SID_NOT_WHILE_IN_COMBAT = new string_id("base_player", "not_while_in_combat");
	
	public static final String PID_NAME = "getCollectionGiftsHasCompletedQuests";
	public static final String QUEST_NAME = "chapter7_publish_gift_activation";
	
	
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
		
		if (item == menu_info_types.ITEM_USE)
		{
			
			if (!wildLocation(player))
			{
				return SCRIPT_CONTINUE;
			}
			
			if (getState(player, STATE_COMBAT) > 0)
			{
				sendSystemMessage(player, SID_NOT_WHILE_IN_COMBAT);
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
			
			if (getTopMostContainer(player) != player)
			{
				sendSystemMessage(player, PLAYER_IN_CELL);
				return SCRIPT_CONTINUE;
			}
			
			if (groundquests.hasCompletedQuest(player, QUEST_NAME) || groundquests.isQuestActive(player, QUEST_NAME))
			{
				return SCRIPT_CONTINUE;
			}
			
			if (utils.hasScriptVar(player, "questGiver.openSui"))
			{
				int oldSui = utils.getIntScriptVar(player, "questGiver.openSui");
				utils.removeScriptVar(player, "questGiver.openSui");
				if (oldSui > -1)
				{
					forceCloseSUIPage(oldSui);
				}
			}
			
			if ((groundquests.isQuestActiveOrComplete(player, "chapter7_beginning_click_collection") || groundquests.isQuestActiveOrComplete(player, "chapter7_beginning_counter_collection") || groundquests.isQuestActiveOrComplete(player, "chapter7_beginning_inventory_collection_01") || groundquests.isQuestActiveOrComplete(player, "chapter7_beginning_inventory_collection_02") || groundquests.isQuestActiveOrComplete(player, "chapter7_beginning_inventory_collection_03") || groundquests.isQuestActiveOrComplete(player, "chapter7_beginning_inventory_collection_04")))
			{
				
				String title = utils.packStringId(SID_ELIGIBLE_FOR_GIFT_TITLE);
				String prompt = utils.packStringId(SID_ELIGIBLE_FOR_GIFT_PROMPT);
				
				int pid = sui.msgbox(self, player, prompt, sui.YES_NO, title, "handleGrantPublishGiftFromComlink");
				sui.setPid(player, pid, PID_NAME);
			}
			else
			{
				string_id sid_offerText = SUI_TITLE;
				
				String title = utils.packStringId(sid_offerText);
				String testMsg = utils.packStringId(OFFER_QUEST_MSG);
				String ok_button = utils.packStringId(BUTTON_ACCEPT);
				String cancel_button = utils.packStringId(BUTTON_DECLINE);
				
				int pid = sui.createSUIPage( sui.SUI_MSGBOX, self, player, "handleQuestOfferResponse");
				
				setSUIProperty(pid, sui.MSGBOX_TITLE, sui.PROP_TEXT, title);
				setSUIProperty(pid, sui.MSGBOX_PROMPT, sui.PROP_TEXT, testMsg);
				
				sui.msgboxButtonSetup(pid, sui.YES_NO);
				setSUIProperty(pid, sui.MSGBOX_BTN_OK, sui.PROP_TEXT, ok_button);
				setSUIProperty(pid, sui.MSGBOX_BTN_CANCEL, sui.PROP_TEXT, cancel_button);
				
				utils.setScriptVar(player, "questGiver.openSui", pid);
				sui.showSUIPage( pid );
				
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleGrantPublishGiftFromComlink(obj_id self, dictionary params) throws InterruptedException
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
			groundquests.grantQuest(player, "chapter7_give_comlink_reward");
			groundquests.grantQuest(player, "chapter7_give_datapad_reward");
			destroySelf(self);
			sui.removePid(player, PID_NAME);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleQuestOfferResponse(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		
		if (!isIdValid(player) || !exists(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!wildLocation(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (groundquests.hasCompletedQuest(player, QUEST_NAME) || groundquests.isQuestActive(player, QUEST_NAME))
		{
			return SCRIPT_CONTINUE;
		}
		
		int bp = sui.getIntButtonPressed(params);
		switch ( bp )
		{
			case sui.BP_OK:	
			callShuttle(self, player);
			break;
			
			case sui.BP_CANCEL: 
			sendSystemMessage(player, DECLINED_QUEST);
			break;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public void destroySelf(obj_id self) throws InterruptedException
	{
		detachScript(self,"item.special.nodestroy");
		destroyObject(self);
	}
	
	
	public boolean wildLocation(obj_id player) throws InterruptedException
	{
		if (!isIdValid(player) || !exists(player))
		{
			return false;
		}
		
		location loc = getLocation(player);
		
		if (isIdValid(loc.cell))
		{
			sendSystemMessage(player, new string_id("combat_effects", "not_indoors"));
			return false;
		}
		
		if (locations.isInMissionCity(loc))
		{
			sendSystemMessage(player, new string_id("combat_effects", "not_in_city"));
			return false;
		}
		
		if (city.isInCity(loc))
		{
			sendSystemMessage(player, new string_id("combat_effects", "not_in_city"));
			return false;
		}
		return true;
	}
	
	
	public boolean callShuttle(obj_id self, obj_id player) throws InterruptedException
	{
		if (!isIdValid(player) || !exists(player))
		{
			return false;
		}
		
		if (!wildLocation(player))
		{
			return false;
		}
		
		location loc = getLocation(player);
		float treasureXdelta = rand(15f, 18f);
		float treasureZdelta = rand(15f, 18f);
		
		location landingLoc = (location)loc.clone();
		landingLoc.x += treasureXdelta;
		landingLoc.z += treasureZdelta;
		
		landingLoc.y = getHeightAtLocation(landingLoc.x, landingLoc.z);
		
		obj_id shuttle = createObject("object/creature/npc/theme_park/event_transport.iff", landingLoc);
		
		if (!isIdValid(shuttle))
		{
			return false;
		}
		
		groundquests.grantQuest(player, QUEST_NAME);
		modifyCollectionSlotValue(player, "publish_chapter7_click_activation_01", 1);
		setObjVar(player,"publish_chapter7_click_activated", true);
		
		dictionary d = new dictionary();
		d.put("owner", player);
		
		attachScript(shuttle, "systems.collections.publish_gift.publish_gift_shuttle");
		
		messageTo(shuttle, "startLandingSequence", d, 0.0f, false);
		destroySelf(self);
		return true;
	}
	
}
