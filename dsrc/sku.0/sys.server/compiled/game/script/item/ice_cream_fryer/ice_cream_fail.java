package script.item.ice_cream_fryer;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.buff;
import script.library.utils;


public class ice_cream_fail extends script.base_script
{
	public ice_cream_fail()
	{
	}
	public static final string_id SID_ALREADY_HAVE_BUFF = new string_id("base_player", "food_already_have_buff");
	public static final string_id SID_BUFF_WONT_STACK = new string_id("base_player", "food_buff_wont_stack");
	
	public static final String ALREADY_SETUP = "itemFoodScriptRemoved";
	public static final String ITEM_FOOD_SCRIPT = "item.food";
	public static final String BUFF_NAME = "ice_cream_debuff";
	
	public static final String OWNER_OID = "owner";
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		if (!hasObjVar(self, ALREADY_SETUP))
		{
			blog("OnAttach - NO OBJVAR FOUND");
			
			messageTo(self, "setUpBuff", null, 3, false);
			messageTo(self, "removeItemFoodScript", null, 3, false);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnGetAttributes(obj_id self, obj_id player, String[] names, String[] attribs) throws InterruptedException
	{
		int idx = utils.getValidAttributeIndex(names);
		if (idx == -1)
		{
			return super.OnGetAttributes(self, player, names, attribs);
		}
		
		names[idx] = "manf_attribs";
		attribs[idx] = "<UNKNOWN>";
		idx++;
		
		return super.OnGetAttributes(self, player, names, attribs);
		
	}
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		menu_info_data mid = mi.getMenuItemByType(menu_info_types.ITEM_USE);
		if (mid != null)
		{
			mid.setServerNotify(true);
		}
		
		mid = mi.getMenuItemByType(menu_info_types.EXAMINE);
		if (mid != null)
		{
			mid.setServerNotify(true);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		if (isDead(player) || isIncapacitated(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (item == menu_info_types.ITEM_USE)
		{
			eat(self, player);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void eat(obj_id self, obj_id player) throws InterruptedException
	{
		if (!isIdValid(self) || !isIdValid(player))
		{
			return;
		}
		
		if (buff.hasBuff(player, "spice_downer"))
		{
			return;
		}
		
		obj_id owner = getObjIdObjVar(self, OWNER_OID);
		if (player != owner)
		{
			sendSystemMessageTestingOnly(player, "temp message: you cannot eat this item. ");
			return;
		}
		
		String buff_name = getStringObjVar(self, "buff_name");
		int[] filling = new int[2];
		
		if (hasObjVar(self, "filling"))
		{
			filling = getIntArrayObjVar(self, "filling");
		}
		else if (hasObjVar(self, "filling_type") && hasObjVar(self, "filling_amount"))
		{
			filling[0] = 0;
			filling[1] = 0;
		}
		else
		{
			filling[0] = 0;
			filling[1] = 0;
		}
		
		if (buff_name == null || buff_name.equals(""))
		{
			return;
		}
		
		if (hasObjVar(self, "pet_only"))
		{
			
			return;
		}
		
		float eff = 1f;
		float dur = 1f;
		
		if (hasObjVar(self, "effectiveness"))
		{
			eff = getFloatObjVar(self, "effectiveness");
		}
		
		if (hasObjVar(self, "duration"))
		{
			dur = getFloatObjVar(self, "duration");
		}
		
		boolean success = true;
		
		if (!buff.canApplyBuff(player, buff_name))
		{
			sendSystemMessage(player, SID_BUFF_WONT_STACK);
			return;
		}
		
		float value = buff.getEffectValue(buff_name, 1);
		float duration = buff.getDuration(buff_name);
		
		value *= eff;
		duration *= dur;
		
		success = buff.applyBuff(player, buff_name, duration, value);
		
		if (success)
		{
			
			String snd = "clienteffect/";
			switch(getSpecies(player))
			{
				case SPECIES_MON_CALAMARI:
				case SPECIES_RODIAN:
				case SPECIES_TRANDOSHAN: snd += "reptile_";
				break;
				case SPECIES_WOOKIEE: snd += "wookiee_";
				break;
				default: snd += "human_";
			}
			
			switch(getGender(player))
			{
				case GENDER_FEMALE: snd += "female_eat.cef";
				break;
				default: snd += "male_eat.cef";
			}
			
			playClientEffectLoc(player, snd, getLocation(player), getScale(player));
			
			int count = getCount(self);
			count--;
			
			if (count <= 0)
			{
				destroyObject(self);
			}
			else
			{
				setCount(self, count);
			}
		}
	}
	
	
	public String formatTime(int seconds) throws InterruptedException
	{
		String result = "";
		
		int hours = (seconds / 3600);
		seconds -= (hours * 3600);
		
		int minutes = (seconds / 60);
		seconds -= (minutes * 60);
		
		if (hours > 0)
		{
			result += ""+ hours + ":";
		}
		
		if (minutes > 0 || hours > 0)
		{
			if (hours > 0 && minutes < 10)
			{
				result += "0";
			}
			
			result += ""+ minutes + ":";
		}
		
		if (minutes > 0 && seconds < 10)
		{
			result += "0";
		}
		
		result += ""+ seconds;
		
		return result;
	}
	
	
	public int removeItemFoodScript(obj_id self, dictionary params) throws InterruptedException
	{
		detachScript(self, ITEM_FOOD_SCRIPT);
		return SCRIPT_CONTINUE;
	}
	
	
	public int setUpBuff(obj_id self, dictionary params) throws InterruptedException
	{
		setObjVar(self, "buff_name", BUFF_NAME);
		setObjVar(self, "noTrade", true);
		setObjVar(self, ALREADY_SETUP, true);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean blog(String message) throws InterruptedException
	{
		LOG("fryer_log", message);
		return true;
	}
	
}
