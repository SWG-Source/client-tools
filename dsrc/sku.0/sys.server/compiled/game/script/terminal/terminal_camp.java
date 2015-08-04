package script.terminal;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.camping;
import script.library.utils;


public class terminal_camp extends script.base_script
{
	public terminal_camp()
	{
	}
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		messageTo(self, "handleTimeSensitiveData", null, 4, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnGetAttributes(obj_id self, obj_id player, String[] names, String[] attribs) throws InterruptedException
	{
		int idx = utils.getValidAttributeIndex(names);
		if (idx == -1)
		{
			return super.OnGetAttributes(self, player, names, attribs);
		}
		
		int destructTime = getIntObjVar(self, camping.VAR_DESTRUCT_TIME);
		if (destructTime <= 0)
		{
			return super.OnGetAttributes(self, player, names, attribs);
		}
		
		names[idx] = "camp_destruction";
		attribs[idx] = getCalendarTimeStringLocal(destructTime);
		idx++;
		
		names[idx] = "camp_timer";
		attribs[idx] = utils.formatTimeVerbose(destructTime - getCalendarTime());
		idx++;
		
		return super.OnGetAttributes(self, player, names, attribs);
	}
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		menu_info_data data = mi.getMenuItemByType(menu_info_types.ITEM_USE);
		
		if (data != null)
		{
			data.setLabel(new string_id("camp","menu_dismantle"));
			data.setServerNotify(true);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		if (item == menu_info_types.ITEM_USE)
		{
			requestCampDismantle(self, player);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void requestCampDismantle(obj_id self, obj_id player) throws InterruptedException
	{
		if (!hasObjVar(self, "camp"))
		{
			return;
		}
		
		obj_id camp = getObjIdObjVar(self, "camp");
		
		if (!isIdValid(camp))
		{
			return;
		}
		
		if (hasObjVar(camp, "camp.owner"))
		{
			obj_id owner = utils.getObjIdObjVar(camp, "camp.owner");
			if (player == owner)
			{
				destroyObject(camp);
			}
			else
			{
				sendSystemMessage(player, new string_id("camp", "owner_dismantle"));
				return;
			}
		}
		
		return;
	}
	
	
	public int handleTimeSensitiveData(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id camp = getObjIdObjVar(self, "camp");
		if (!isValidId(camp) || !exists(camp))
		{
			return SCRIPT_CONTINUE;
		}
		
		int lifeTime = (int)getFloatObjVar(camp, camping.VAR_LIFETIME);
		if (lifeTime <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		int creationTime = getIntObjVar(camp, camping.VAR_CREATION_TIME);
		if (lifeTime <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		int destructTime = creationTime + lifeTime;
		setObjVar(self, camping.VAR_DESTRUCT_TIME, destructTime);
		return SCRIPT_CONTINUE;
	}
	
}
