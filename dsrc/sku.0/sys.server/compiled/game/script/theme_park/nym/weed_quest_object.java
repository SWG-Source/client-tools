package script.theme_park.nym;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.buff;
import script.library.collection;
import script.library.colors;
import script.library.create;
import script.library.factions;
import script.library.groundquests;
import script.library.holiday;
import script.library.money;
import script.library.pclib;
import script.library.pet_lib;
import script.library.prose;
import script.library.static_item;
import script.library.stealth;
import script.library.sui;
import script.library.trial;
import script.library.utils;


public class weed_quest_object extends script.base_script
{
	public weed_quest_object()
	{
	}
	public static final String QUEST_NAME = "u16_nym_themepark_weed_pulling";
	public static final String GENERAL_QUEST_TASK = "fulfillRequisition";
	
	public static final String TASK_WEED_TYPE_1 = "lowGrowingWeeds";
	public static final String TASK_WEED_TYPE_2 = "tallStalkWeed";
	public static final String TASK_WEED_TYPE_3 = "paddleCactiWeed";
	public static final String TASK_WEED_TYPE_4 = "yellowPodWeed";
	public static final String TASK_WEED_TYPE_5 = "floweringCactiWeed";
	public static final String SIGNAL_WEED_TYPE_1 = "lowGrowingWeedsFound";
	public static final String SIGNAL_WEED_TYPE_2 = "tallStalkWeedFound";
	public static final String SIGNAL_WEED_TYPE_3 = "paddleCactiWeedFound";
	public static final String SIGNAL_WEED_TYPE_4 = "yellowPodWeedFound";
	public static final String SIGNAL_WEED_TYPE_5 = "floweringCactiWeedFound";
	
	public static final String THEMEPARK = "theme_park_nym/messages";
	
	public static final string_id SID_NOT_WHILE_MOUNTED = new string_id(THEMEPARK, "pull_weed_not_while_mounted");
	public static final string_id SID_ZIP_BAR = new string_id(THEMEPARK, "weed_zip_bar");
	public static final string_id SID_ZIP_BAR_PADDLE = new string_id(THEMEPARK, "weed_zip_bar_paddle");
	public static final string_id SID_ZIP_BAR_LOW = new string_id(THEMEPARK, "weed_zip_bar_low");
	public static final string_id SID_ZIP_BAR_FLOWER = new string_id(THEMEPARK, "weed_zip_bar_flower");
	public static final string_id SID_ZIP_BAR_YELLOW = new string_id(THEMEPARK, "weed_zip_bar_yellow");
	public static final string_id SID_ZIP_BAR_TALL = new string_id(THEMEPARK, "weed_zip_bar_tall");
	public static final string_id SID_DONT_NEED_WEED = new string_id(THEMEPARK, "dont_need_weed");
	public static final string_id SID_CHA_CHING = new string_id(THEMEPARK, "received_credits");
	
	public static final string_id SID_MNU_USE = new string_id(THEMEPARK, "pull_weed");
	public static final string_id SID_NOT_SURE_HOW_DESTROY = new string_id(THEMEPARK, "weed_not_sure_what_to_do");
	public static final string_id SID_ALRDY_COMPLETED_QUEST = new string_id(THEMEPARK, "weed_already_completed_quest");
	
	public static final int COUNTDOWN_TIMER = 3;
	public static final int CASH_AMOUNT = 25;
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		if (!isValidId(self) || !exists(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (isDead(player) || isIncapacitated(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!hasObjVar(self, "weed_type"))
		{
			return SCRIPT_CONTINUE;
		}
		
		int mnu2 = mi.addRootMenu (menu_info_types.ITEM_USE, SID_MNU_USE);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		if (!isValidId(self) || !exists(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (item != menu_info_types.ITEM_USE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!hasObjVar(self, "weed_type"))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (isDead(player) || isIncapacitated(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (pet_lib.isMounted(player))
		{
			sendSystemMessage(player, SID_NOT_WHILE_MOUNTED);
			return SCRIPT_CONTINUE;
		}
		
		if (!groundquests.isQuestActive(player, QUEST_NAME))
		{
			sendSystemMessage(player, SID_NOT_SURE_HOW_DESTROY);
			CustomerServiceLog("nyms_themepark", "weed_quest_object.OnObjectMenuSelect() Player: "+player+" did not have the quest "+QUEST_NAME+" needed for this collection. canister: ("+self);
			return SCRIPT_CONTINUE;
		}
		
		if (!groundquests.isTaskActive(player, QUEST_NAME, GENERAL_QUEST_TASK))
		{
			sendSystemMessage(player, SID_NOT_SURE_HOW_DESTROY);
			CustomerServiceLog("nyms_themepark", "weed_quest_object.OnObjectMenuSelect() Player: "+player+" did not have the quest "+QUEST_NAME+" needed for this collection. canister: ("+self);
			return SCRIPT_CONTINUE;
		}
		
		int weedType = getIntObjVar(self, "weed_type");
		if (weedType < 0)
		{
			CustomerServiceLog("nyms_themepark", "weed_quest_object.OnObjectMenuSelect() Weed: "+self+" did not have a valid weed type var");
			return SCRIPT_CONTINUE;
		}
		
		string_id zipBar = SID_ZIP_BAR;
		
		switch(weedType)
		{
			case 1:
			zipBar = SID_ZIP_BAR_LOW;
			break;
			case 2:
			zipBar = SID_ZIP_BAR_TALL;
			break;
			case 3:
			zipBar = SID_ZIP_BAR_PADDLE;
			break;
			case 4:
			zipBar = SID_ZIP_BAR_YELLOW;
			break;
			case 5:
			zipBar = SID_ZIP_BAR_FLOWER;
			break;
			default:
			break;
		}
		
		int startTime = 0;
		int range = 3;
		int flags = 0;
		
		flags |= sui.CD_EVENT_INCAPACITATE;
		flags |= sui.CD_EVENT_DAMAGED;
		
		stealth.testInvisNonCombatAction(player, self);
		int countdownSui = sui.smartCountdownTimerSUI(self, player, "quest_countdown_timer", zipBar, startTime, COUNTDOWN_TIMER, "handleObjectSwapTimer", range, flags);
		CustomerServiceLog("nyms_themepark", "weed_quest_object.OnObjectMenuSelect() Player: "+player+" is destroying Nym Weed: "+self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleObjectSwapTimer(obj_id self, dictionary params) throws InterruptedException
	{
		if (!isValidId(self) || !exists(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (params == null || params.equals(""))
		{
			return SCRIPT_CONTINUE;
		}
		
		int pid = params.getInt("id");
		obj_id player = params.getObjId("player");
		if (!isIdValid(player) || !exists(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		int bp = sui.getIntButtonPressed(params);
		
		if (bp == sui.BP_CANCEL)
		{
			detachScript(player, sui.COUNTDOWNTIMER_PLAYER_SCRIPT);
			CustomerServiceLog("nyms_themepark", "weed_quest_object.handleObjectSwapTimer() cancelled");
			
			return SCRIPT_CONTINUE;
		}
		
		else if (bp == sui.BP_REVERT)
		{
			CustomerServiceLog("nyms_themepark", "weed_quest_object.handleObjectSwapTimer() Player: "+player+" failed to destroy the canister because the player moved or was disrupted. Deposit: "+self);
			
			int event = params.getInt("event");
			
			if (event == sui.CD_EVENT_LOCOMOTION)
			{
				sendSystemMessage(player, new string_id("quest/groundquests", "countdown_interrupted_locomotion"));
			}
			else if (event == sui.CD_EVENT_INCAPACITATE)
			{
				sendSystemMessage(player, new string_id("quest/groundquests", "countdown_interrupted_incapacitated"));
			}
			else if (event == sui.CD_EVENT_DAMAGED)
			{
				sendSystemMessage(player, new string_id("quest/groundquests", "countdown_interrupted_damaged"));
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (!hasObjVar(player, sui.COUNTDOWNTIMER_SUI_VAR))
		{
			CustomerServiceLog("nyms_themepark", "weed_quest_object.handleObjectSwapTimer() no countdowntimer");
			return SCRIPT_CONTINUE;
		}
		
		int test_pid = getIntObjVar(player, sui.COUNTDOWNTIMER_SUI_VAR);
		
		if (pid != test_pid)
		{
			CustomerServiceLog("nyms_themepark", "weed_quest_object.handleObjectSwapTimer() pid != test_pid");
			return SCRIPT_CONTINUE;
		}
		
		forceCloseSUIPage(pid);
		detachScript(player, sui.COUNTDOWNTIMER_PLAYER_SCRIPT);
		
		if (!groundquests.isTaskActive(player, QUEST_NAME, GENERAL_QUEST_TASK))
		{
			CustomerServiceLog("nyms_themepark", "weed_quest_object.handleObjectSwapTimer() Player: "+player+" did not have the quest "+QUEST_NAME+" needed for this collection. canister: ("+self);
			return SCRIPT_CONTINUE;
		}
		
		int weedType = getIntObjVar(self, "weed_type");
		if (weedType < 0)
		{
			CustomerServiceLog("nyms_themepark", "weed_quest_object.OnObjectMenuSelect() Weed: "+self+" did not have a valid weed type var");
			return SCRIPT_CONTINUE;
		}
		
		String questTask = "";
		String questSignal = "";
		
		switch(weedType)
		{
			case 1:
			questTask = TASK_WEED_TYPE_1;
			questSignal = SIGNAL_WEED_TYPE_1;
			break;
			case 2:
			questTask = TASK_WEED_TYPE_2;
			questSignal = SIGNAL_WEED_TYPE_2;
			break;
			case 3:
			questTask = TASK_WEED_TYPE_3;
			questSignal = SIGNAL_WEED_TYPE_3;
			break;
			case 4:
			questTask = TASK_WEED_TYPE_4;
			questSignal = SIGNAL_WEED_TYPE_4;
			break;
			case 5:
			questTask = TASK_WEED_TYPE_5;
			questSignal = SIGNAL_WEED_TYPE_5;
			break;
			default:
			break;
		}
		
		if (questTask == null || questTask.length() <= 0)
		{
			CustomerServiceLog("nyms_themepark", "weed_quest_object.OnObjectMenuSelect() Player: "+player+" could not attain a valid quest task off the weed object: "+self);
			return SCRIPT_CONTINUE;
		}
		
		if (questSignal == null || questSignal.length() <= 0)
		{
			CustomerServiceLog("nyms_themepark", "weed_quest_object.OnObjectMenuSelect() Player: "+player+" could not attain a valid quest signal off the weed object: "+self);
			return SCRIPT_CONTINUE;
		}
		
		if (groundquests.isTaskActive(player, QUEST_NAME, questTask))
		{
			groundquests.sendSignal(player, questSignal);
		}
		else
		{
			sendSystemMessage(player, SID_DONT_NEED_WEED);
			if (!utils.hasScriptVar(player, "commPlayerWeeds"))
			{
				String npc = "object/mobile/nym_themepark_ran_machado.iff";
				String sound = "sound/sys_comm_rebel_male.snd";
				prose_package pp = new prose_package();
				prose.setStringId(pp, new string_id("theme_park_nym/messages", "extra_credit_weeds"));
				commPlayers(player, npc, sound, 10f, player, pp);
				
				utils.setScriptVar(player, "commPlayerWeeds", true);
			}
			money.bankTo(money.ACCT_NEW_PLAYER_QUESTS, player, CASH_AMOUNT);
			play2dNonLoopingSound(player, groundquests.MUSIC_QUEST_RECEIVED_CREDITS);
			showFlyText(player, SID_CHA_CHING, 3.0f, colors.TOMATO);
			
		}
		
		messageTo(self, "destroySelf", null, 0, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int destroySelf(obj_id self, dictionary params) throws InterruptedException
	{
		
		if (!hasObjVar(self, "mySpawner"))
		{
			CustomerServiceLog("nyms_themepark", "weed_quest_object.destroySelf() The Weed: "+self+" did not have a parent objvar. The parent cannot be told to destroy self.");
		}
		
		obj_id parent = getObjIdObjVar(self, "mySpawner");
		if (!isValidId(parent))
		{
			CustomerServiceLog("nyms_themepark", "weed_quest_object.destroySelf() The Weed: "+self+" did not have a valid parent objvar. The parent may already be destroyed.");
		}
		
		messageTo(parent, "destroySelf", null, 0, false);
		destroyObject(self);
		
		return SCRIPT_CONTINUE;
	}
}
