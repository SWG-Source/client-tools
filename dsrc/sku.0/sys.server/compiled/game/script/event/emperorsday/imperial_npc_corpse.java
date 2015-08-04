package script.event.emperorsday;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.ai_lib;
import script.library.factions;
import script.library.groundquests;
import script.library.holiday;


public class imperial_npc_corpse extends script.base_script
{
	public imperial_npc_corpse()
	{
	}
	public static final String EMPIREDAY = "event/empire_day";
	public static final string_id SID_MNU_REBEL = new string_id(EMPIREDAY, "imperial_crash_victim_rebel_menu");
	public static final string_id SID_MNU_IMPERIAL = new string_id(EMPIREDAY, "imeprial_crash_victim_imperial_menu");
	public static final string_id SID_YOU_FIND_NOTHING = new string_id(EMPIREDAY, "you_find_nothing");
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		messageTo(self, "knockDown", null, 10, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		messageTo(self, "knockDown", null, 10, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		if (isDead(player) || isIncapacitated(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!groundquests.isQuestActive(player, holiday.EMPIREDAYQUEST_REB_COMBAT_01))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!groundquests.isTaskActive(player, holiday.EMPIREDAYQUEST_REB_COMBAT_01, holiday.EMPIREDAYQUEST_REB_CORPSE_TASK))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (factions.isImperial(player) || !factions.isRebel(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		menu_info_data data = mi.getMenuItemByType (menu_info_types.SERVER_MENU2);
		mi.addRootMenu(menu_info_types.SERVER_MENU2, SID_MNU_REBEL);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		if (isDead(player) || isIncapacitated(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!groundquests.isQuestActive(player, holiday.EMPIREDAYQUEST_REB_COMBAT_01))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!groundquests.isTaskActive(player, holiday.EMPIREDAYQUEST_REB_COMBAT_01, holiday.EMPIREDAYQUEST_REB_CORPSE_TASK))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (factions.isImperial(player) || !factions.isRebel(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!hasObjVar(self, "hasIntel"))
		{
			sendSystemMessage(player, SID_YOU_FIND_NOTHING);
			return SCRIPT_CONTINUE;
		}
		groundquests.sendSignal(player, holiday.EMPIREDAYQUEST_REB_CORPSE_SIGNAL);
		return SCRIPT_CONTINUE;
	}
	
	
	public int knockDown(obj_id self, dictionary params) throws InterruptedException
	{
		ai_lib.aiSetPosture(self, POSTURE_KNOCKED_DOWN);
		return SCRIPT_CONTINUE;
	}
}
