package script.theme_park.nym;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.ai_lib;
import script.library.groundquests;
import script.library.locations;
import script.library.utils;


public class nym_elevator_up extends script.base_script
{
	public nym_elevator_up()
	{
	}
	public static final String STF_FILE = "theme_park_nym/messages";
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		if (!groundquests.isQuestActiveOrComplete(player, "u16_nym_themepark_nym_arena"))
		{
			
			return SCRIPT_CONTINUE;
		}
		int mnu = mi.addRootMenu (menu_info_types.ITEM_USE, new string_id(STF_FILE,"ladder_up"));
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		if (item != menu_info_types.ITEM_USE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!groundquests.isQuestActiveOrComplete(player, "u16_nym_themepark_nym_arena"))
		{
			
			return SCRIPT_CONTINUE;
		}
		if (ai_lib.isInCombat(player))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		warpPlayer(player, "lok", 474, 33, 4781, null, 0, 0, 0);
		return SCRIPT_CONTINUE;
	}
}
