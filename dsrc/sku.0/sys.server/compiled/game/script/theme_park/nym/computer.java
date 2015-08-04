package script.theme_park.nym;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.utils;
import script.library.permissions;
import script.library.groundquests;


public class computer extends script.base_script
{
	public computer()
	{
	}
	public static final String STF_FILE = "theme_park_nym/messages";
	public static final string_id RETRIEVE = new string_id("celebrity/nym", "retrieve_hard_drive");
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info item) throws InterruptedException
	{
		item.addRootMenu(menu_info_types.ITEM_USE, RETRIEVE);
		return SCRIPT_CONTINUE;
	}
	
	
	public int makeNewDrive(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id filter = createObject ("", self, "");
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		if (item != menu_info_types.ITEM_USE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!groundquests.isTaskActive(player, "u16_nym_themepark_research_facility", "getResearchData"))
		{
			sendSystemMessage(player, new string_id("quest/groundquests", "retrieve_item_no_interest"));
		}
		
		groundquests.sendSignal(player, "hasReceivedResearchData");
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		if (hasScript(self, "item.container.base.base_container"))
		{
			detachScript(self, "item.container.base.base_container");
		}
		return SCRIPT_CONTINUE;
	}
}
