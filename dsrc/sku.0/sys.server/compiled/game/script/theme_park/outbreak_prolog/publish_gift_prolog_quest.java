package script.theme_park.outbreak_prolog;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.factions;
import script.library.groundquests;
import script.library.sui;
import script.library.utils;


public class publish_gift_prolog_quest extends script.base_script
{
	public publish_gift_prolog_quest()
	{
	}
	public static final string_id CALL_JABBA = new string_id ("quest/ground/util/quest_giver_object", "call_jabba");
	public static final string_id OFFER_QUEST_MSG = new string_id ("quest/ground/util/quest_giver_object", "offer_quest_comlink");
	public static final string_id SUI_TITLE = new string_id ("quest/ground/util/quest_giver_object", "sui_title");
	public static final string_id BUTTON_DECLINE = new string_id ("quest/ground/util/quest_giver_object", "button_decline");
	public static final string_id BUTTON_ACCEPT = new string_id ("quest/ground/util/quest_giver_object", "button_accept");
	public static final string_id DECLINED_QUEST = new string_id ("quest/ground/util/quest_giver_object", "declined_quest");
	public static final string_id NOT_WHILE_INCAPPED = new string_id ("quest/ground/util/quest_giver_object", "not_while_incapped");
	public static final string_id SID_NOT_WHILE_IN_COMBAT = new string_id("base_player", "not_while_in_combat");
	
	public static final String PID_NAME = "prologPid";
	
	public static final String QUEST_NAME_NEG_FACTION = "quest_01_comlink_call_to_adventure_neg_faction";
	public static final String QUEST_NAME_NEUT_FACTION = "quest_01_comlink_call_to_adventure_neut_faction";
	public static final String QUEST_NAME_POS_FACTION = "quest_01_comlink_call_to_adventure_pos_faction";
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		if (!isIdValid(player) || !exists(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (isFreeTrialAccount(player))
		{
			return SCRIPT_CONTINUE;
		}
		if (isInTutorialArea(player))
		{
			return SCRIPT_CONTINUE;
		}
		int menuOption = mi.addRootMenu (menu_info_types.ITEM_USE, CALL_JABBA);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		if (!isIdValid(player) || !exists(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (isFreeTrialAccount(player))
		{
			return SCRIPT_CONTINUE;
		}
		if (isInTutorialArea(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (item != menu_info_types.ITEM_USE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (getState(player, STATE_COMBAT) > 0)
		{
			sendSystemMessage(player, SID_NOT_WHILE_IN_COMBAT);
			return SCRIPT_CONTINUE;
		}
		else if (isDead(player) || isIncapacitated(player))
		{
			sendSystemMessage(player, NOT_WHILE_INCAPPED);
			return SCRIPT_CONTINUE;
		}
		
		if (((groundquests.isQuestActiveOrComplete(player, QUEST_NAME_POS_FACTION)) || (groundquests.isQuestActiveOrComplete(player, QUEST_NAME_NEG_FACTION)) || (groundquests.isQuestActiveOrComplete(player, QUEST_NAME_NEUT_FACTION))))
		{
			CustomerServiceLog("outbreak_themepark", "publish_gift_prolog_quest: Deleting Object: "+self+" because player has Deathtrooper Quest active or complete already.");
			destroySelf(self);
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
		
		if (((groundquests.isQuestActiveOrComplete(player, QUEST_NAME_POS_FACTION)) || (groundquests.isQuestActiveOrComplete(player, QUEST_NAME_NEG_FACTION)) || (groundquests.isQuestActiveOrComplete(player, QUEST_NAME_NEUT_FACTION))))
		{
			CustomerServiceLog("outbreak_themepark", "publish_gift_prolog_quest: Deleting Object: "+self+" because player has Deathtrooper Quest active or complete already.");
			destroySelf(self);
			return SCRIPT_CONTINUE;
		}
		
		int bp = sui.getIntButtonPressed(params);
		switch ( bp )
		{
			case sui.BP_OK:	
			grantTheCorrectQuest(self, player);
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
	
	
	public boolean grantTheCorrectQuest(obj_id comlink, obj_id player) throws InterruptedException
	{
		if (!isIdValid(comlink) || !exists(comlink))
		{
			return false;
		}
		
		if (!isIdValid(player) || !exists(player))
		{
			return false;
		}
		
		if (hasCompletedCollectionSlot(player, "bdg_thm_park_jabba_badge"))
		{
			groundquests.grantQuest(player, QUEST_NAME_POS_FACTION);
		}
		
		else
		{
			float factionAmount = factions.getFactionStanding(player, "jabba");
			if (factionAmount < -2000)
			{
				groundquests.grantQuest(player, QUEST_NAME_NEG_FACTION);
			}
			else if (factionAmount >= 100)
			{
				groundquests.grantQuest(player, QUEST_NAME_POS_FACTION);
			}
			else
			{
				groundquests.grantQuest(player, QUEST_NAME_NEUT_FACTION);
			}
		}
		destroySelf(comlink);
		return true;
	}
}
