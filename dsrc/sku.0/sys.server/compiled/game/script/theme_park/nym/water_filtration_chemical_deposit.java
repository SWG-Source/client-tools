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


public class water_filtration_chemical_deposit extends script.base_script
{
	public water_filtration_chemical_deposit()
	{
	}
	public static final String THEMEPARK = "theme_park_nym/messages";
	
	public static final string_id SID_NOT_WHILE_MOUNTED = new string_id(THEMEPARK, "deposit_not_while_mounted");
	public static final string_id SID_ZIP_BAR = new string_id(THEMEPARK, "deposit_zip_bar");
	public static final string_id SID_MNU_USE = new string_id(THEMEPARK, "deposit_gather_menu");
	public static final string_id SID_NOT_SURE_HOW_DESTROY = new string_id(THEMEPARK, "deposit_not_sure_what_to_do");
	public static final string_id SID_ALRDY_COMPLETED_QUEST = new string_id(THEMEPARK, "deposit_already_completed_quest");
	
	public static final int COUNTDOWN_TIMER = 3;
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		if (isDead(player) || isIncapacitated(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		int mnu2 = mi.addRootMenu (menu_info_types.ITEM_USE, SID_MNU_USE);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
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
		
		String quest = getStringObjVar(self, "strQuest");
		if (quest == null || quest.length() <= 0)
		{
			CustomerServiceLog("nyms_themepark", "water_filtration_chemical_deposit.OnObjectMenuSelect() Could not find the parent spawner quest for participating in the canister quest for canister: ("+self+"). The operation was aborted for Player: "+player);
			return SCRIPT_CONTINUE;
		}
		
		if (!groundquests.isQuestActive(player, quest))
		{
			sendSystemMessage(player, SID_NOT_SURE_HOW_DESTROY);
			CustomerServiceLog("nyms_themepark", "water_filtration_chemical_deposit.OnObjectMenuSelect() Player: "+player+" did not have the quest "+quest+" needed for this collection. canister: ("+self);
			return SCRIPT_CONTINUE;
		}
		
		if (!groundquests.isTaskActive(player, quest, "needsToGatherChemical"))
		{
			sendSystemMessage(player, SID_NOT_SURE_HOW_DESTROY);
			CustomerServiceLog("nyms_themepark", "water_filtration_chemical_deposit.OnObjectMenuSelect() Player: "+player+" did not have the quest "+quest+" needed for this collection. canister: ("+self);
			return SCRIPT_CONTINUE;
		}
		
		String questSignal = getStringObjVar(self, "strSignal");
		if (questSignal == null || questSignal.length() <= 0)
		{
			CustomerServiceLog("nyms_themepark", "water_filtration_chemical_deposit.OnObjectMenuSelect() Could not find the parent spawner quest SIGNAL for participating in the canister quest for canister: ("+self+"). The operation was aborted for Player: "+player);
			return SCRIPT_CONTINUE;
		}
		
		int startTime = 0;
		int range = 3;
		int flags = 0;
		
		flags |= sui.CD_EVENT_INCAPACITATE;
		flags |= sui.CD_EVENT_DAMAGED;
		
		stealth.testInvisNonCombatAction(player, self);
		int countdownSui = sui.smartCountdownTimerSUI(self, player, "quest_countdown_timer", SID_ZIP_BAR, startTime, COUNTDOWN_TIMER, "handleObjectSwapTimer", range, flags);
		CustomerServiceLog("nyms_themepark", "water_filtration_chemical_deposit.OnObjectMenuSelect() Player: "+player+" is destroying blackwing canister: "+self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleObjectSwapTimer(obj_id self, dictionary params) throws InterruptedException
	{
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
			CustomerServiceLog("nyms_themepark", "water_filtration_chemical_deposit.handleObjectSwapTimer() cancelled");
			
			return SCRIPT_CONTINUE;
		}
		
		else if (bp == sui.BP_REVERT)
		{
			CustomerServiceLog("nyms_themepark", "water_filtration_chemical_deposit.handleObjectSwapTimer() Player: "+player+" failed to destroy the canister because the player moved or was disrupted. Deposit: "+self);
			
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
			CustomerServiceLog("nyms_themepark", "water_filtration_chemical_deposit.handleObjectSwapTimer() no countdowntimer");
			return SCRIPT_CONTINUE;
		}
		
		int test_pid = getIntObjVar(player, sui.COUNTDOWNTIMER_SUI_VAR);
		
		if (pid != test_pid)
		{
			CustomerServiceLog("nyms_themepark", "water_filtration_chemical_deposit.handleObjectSwapTimer() pid != test_pid");
			return SCRIPT_CONTINUE;
		}
		
		forceCloseSUIPage(pid);
		detachScript(player, sui.COUNTDOWNTIMER_PLAYER_SCRIPT);
		
		String quest = getStringObjVar(self, "strQuest");
		if (quest == null || quest.length() <= 0)
		{
			CustomerServiceLog("nyms_themepark", "water_filtration_chemical_deposit.handleObjectSwapTimer() Could not find the parent spawner quest for participating in the canister quest for canister: ("+self+"). The operation was aborted for Player: "+player);
			return SCRIPT_CONTINUE;
		}
		
		if (!groundquests.isTaskActive(player, quest, "needsToGatherChemical"))
		{
			CustomerServiceLog("nyms_themepark", "water_filtration_chemical_deposit.handleObjectSwapTimer() Player: "+player+" did not have the quest "+quest+" needed for this collection. canister: ("+self);
			return SCRIPT_CONTINUE;
		}
		
		String questSignal = getStringObjVar(self, "strSignal");
		if (questSignal == null || questSignal.length() <= 0)
		{
			CustomerServiceLog("nyms_themepark", "water_filtration_chemical_deposit.handleObjectSwapTimer() Could not find the parent spawner quest SIGNAL for participating in the canister quest for canister: ("+self+"). The operation was aborted for Player: "+player);
			return SCRIPT_CONTINUE;
		}
		
		groundquests.sendSignal(player, questSignal);
		
		messageTo(self, "destroySelf", null, 0, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int destroySelf(obj_id self, dictionary params) throws InterruptedException
	{
		destroyObject(self);
		
		return SCRIPT_CONTINUE;
	}
}
