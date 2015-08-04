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


public class nym_elevator_down extends script.base_script
{
	public nym_elevator_down()
	{
	}
	public static final String STF_FILE = "theme_park_nym/messages";
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		messageTo (self, "makeElevatorWork", null, 5, true);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		if (!hasObjVar(self, "stronghold_lobby"))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		if (!groundquests.isQuestActiveOrComplete(player, "u16_nym_themepark_nym_arena"))
		{
			
			return SCRIPT_CONTINUE;
		}
		int mnu = mi.addRootMenu (menu_info_types.ITEM_USE, new string_id(STF_FILE,"ladder_down"));
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		if (!hasObjVar(self, "stronghold_lobby"))
		{
			
			return SCRIPT_CONTINUE;
		}
		
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
		
		obj_id strongHoldLobby = getObjIdObjVar(self, "stronghold_lobby");
		if (!isValidId(strongHoldLobby))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		warpPlayer(player, "lok", 0f, 0f, 0f, strongHoldLobby, 0f, 8.9f, -32.0f);
		return SCRIPT_CONTINUE;
	}
	
	
	public int makeElevatorWork(obj_id self, dictionary params) throws InterruptedException
	{
		String stronghold = "6595508";
		obj_id building = utils.stringToObjId(stronghold);
		if (!isValidId(building))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id room = getCellId(building, "lobby");
		if (!isValidId(room))
		{
			return SCRIPT_CONTINUE;
		}
		
		setObjVar(self, "stronghold_lobby", room);
		return SCRIPT_CONTINUE;
	}
}
