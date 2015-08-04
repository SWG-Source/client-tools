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
import script.library.create;
import script.library.factions;
import script.library.groundquests;
import script.library.holiday;
import script.library.pclib;
import script.library.pet_lib;
import script.library.static_item;
import script.library.stealth;
import script.library.sui;
import script.library.trial;
import script.library.utils;


public class dumpster_quest_object extends script.base_script
{
	public dumpster_quest_object()
	{
	}
	public static final String QUEST_NAME = "u16_nym_themepark_weed_pulling";
	public static final String QUEST_TASK = "disposeOfWeeds";
	public static final String QUEST_SIGNAL = "hasDisposedOfWeeds";
	
	public static final String THEMEPARK = "theme_park_nym/messages";
	
	public static final string_id SID_NOT_WHILE_MOUNTED = new string_id(THEMEPARK, "dispose_weed_not_while_mounted");
	public static final string_id SID_ZIP_BAR = new string_id(THEMEPARK, "dispose_weed_zip_bar");
	public static final string_id SID_MNU_USE = new string_id(THEMEPARK, "dispose_weed");
	public static final string_id SID_NOT_SURE_HOW_DESTROY = new string_id(THEMEPARK, "dispose_weed_not_sure_what_to_do");
	public static final string_id SID_ALRDY_COMPLETED_QUEST = new string_id(THEMEPARK, "dispose_weed_already_completed_quest");
	
	public static final int COUNTDOWN_TIMER = 3;
	
	
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
		
		if (!groundquests.isTaskActive(player, QUEST_NAME, QUEST_TASK))
		{
			sendSystemMessage(player, SID_NOT_SURE_HOW_DESTROY);
			CustomerServiceLog("nyms_themepark", "weed_quest_object.OnObjectMenuSelect() Player: "+player+" did not have the quest "+QUEST_NAME+" needed for this collection. canister: ("+self);
			return SCRIPT_CONTINUE;
		}
		
		int startTime = 0;
		int range = 3;
		int flags = 0;
		
		flags |= sui.CD_EVENT_INCAPACITATE;
		flags |= sui.CD_EVENT_DAMAGED;
		
		stealth.testInvisNonCombatAction(player, self);
		int countdownSui = sui.smartCountdownTimerSUI(self, player, "quest_countdown_timer", SID_ZIP_BAR, startTime, COUNTDOWN_TIMER, "handleObjectSwapTimer", range, flags);
		CustomerServiceLog("nyms_themepark", "weed_quest_object.OnObjectMenuSelect() Player: "+player+" is destroying blackwing canister: "+self);
		
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
		
		if (!groundquests.isTaskActive(player, QUEST_NAME, QUEST_TASK))
		{
			CustomerServiceLog("nyms_themepark", "weed_quest_object.handleObjectSwapTimer() Player: "+player+" did not have the quest "+QUEST_NAME+" needed for this collection. canister: ("+self);
			return SCRIPT_CONTINUE;
		}
		
		groundquests.sendSignal(player, QUEST_SIGNAL);
		return SCRIPT_CONTINUE;
	}
}
