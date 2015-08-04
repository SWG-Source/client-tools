package script.theme_park.nym;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.groundquests;
import script.library.utils;
import script.library.permissions;


public class nym_dungeon_collectible extends script.base_script
{
	public nym_dungeon_collectible()
	{
	}
	public static final String STF_FILE = "pet/droid_modules";
	public static final string_id RETRIEVE = new string_id("celebrity/jinkins", "retrieve");
	
	
	public int makeNewModule(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id module = createObject ("object/tangible/loot/quest/nym_droid_memory_chip.iff", self, "");
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info item) throws InterruptedException
	{
		item.addRootMenu(menu_info_types.ITEM_USE, RETRIEVE);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		if (item != menu_info_types.ITEM_USE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!groundquests.isTaskActive(player, "u16_nym_themepark_pirate_hideout", "findDroidDisk"))
		{
			sendSystemMessage(player, new string_id("quest/groundquests", "retrieve_item_no_interest"));
			return SCRIPT_CONTINUE;
		}
		
		groundquests.sendSignal(player, "hasFoundDroidDisk");
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
