package script.terminal;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.player_structure;
import script.library.sui;
import script.library.utils;
import script.library.prose;



public class gcw_publish_gift_terminal extends script.terminal.base.base_terminal
{
	public gcw_publish_gift_terminal()
	{
	}
	
	public int OnDestroy(obj_id self) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		return super.OnInitialize(self);
	}
	
	
	public int OnGetAttributes(obj_id self, obj_id player, String[] names, String[] attribs) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		menu_info_data data = mi.getMenuItemByType (menu_info_types.ITEM_USE);
		return super.OnObjectMenuRequest(self, player, mi);
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
}
