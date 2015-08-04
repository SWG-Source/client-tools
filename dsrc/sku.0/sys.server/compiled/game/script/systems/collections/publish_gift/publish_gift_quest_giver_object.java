package script.systems.collections.publish_gift;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.city;
import script.library.locations;
import script.library.groundquests;
import script.library.sui;
import script.library.utils;


public class publish_gift_quest_giver_object extends script.base_script
{
	public publish_gift_quest_giver_object()
	{
	}
	public static final string_id RADIAL_INSPECT = new string_id ("quest/ground/util/quest_giver_object", "radial_inspect");
	public static final string_id OFFER_QUEST_MSG = new string_id ("quest/ground/util/quest_giver_object", "offer_quest");
	public static final string_id SUI_TITLE = new string_id ("quest/ground/util/quest_giver_object", "sui_title");
	public static final string_id BUTTON_DECLINE = new string_id ("quest/ground/util/quest_giver_object", "button_decline");
	public static final string_id BUTTON_ACCEPT = new string_id ("quest/ground/util/quest_giver_object", "button_accept");
	
	public static final string_id ALREADY_COMPLETED_QUEST = new string_id ("quest/ground/util/quest_giver_object", "already_completed_quest");
	public static final string_id ALREADY_HAS_QUEST = new string_id ("quest/ground/util/quest_giver_object", "already_has_quest");
	public static final string_id DECLINED_QUEST = new string_id ("quest/ground/util/quest_giver_object", "declined_quest");
	public static final string_id OBJECT_UPLOADED = new string_id ("quest/ground/util/quest_giver_object", "object_uploaded");
	
	public static final string_id IMPERIAL_FACTION_REQ = new string_id ("quest/ground/util/quest_giver_object", "imperial_faction_required");
	public static final string_id REBEL_FACTION_REQ = new string_id ("quest/ground/util/quest_giver_object", "rebel_faction_required");
	
	public static final String DATATABLE = "datatables/quest/ground/quest_giver_object.iff";
	
	public static final String PRE_QUEST_NAME = "pre_quest_name";
	public static final String QUEST_OFFER_TEXT = "quest_offer_text";
	public static final String QUEST_IS_REPEATABLE = "quest_is_repeatable";
	public static final String DO_NOT_DESTROY = "do_not_destroy";
	public static final String IMPERIAL_ONLY = "imperial_only";
	public static final String REBEL_ONLY = "rebel_only";
	
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
		
		if (locations.isInCity(loc))
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
		
		obj_id shuttle = createObject("object/creature/npc/theme_park/lambda_shuttle.iff", loc);
		
		if (!isIdValid(shuttle))
		{
			return false;
		}
		
		groundquests.grantQuest(player, QUEST_NAME);
		modifyCollectionSlotValue(player, "publish_chapter7_click_activation_01", 1);
		
		dictionary d = new dictionary();
		d.put("owner", player);
		
		attachScript(shuttle, "systems.collections.publish_gift.publish_gift_shuttle");
		
		messageTo(shuttle, "startLandingSequence", d, 2.0f, false);
		destroySelf(self);
		return true;
	}
	
}
