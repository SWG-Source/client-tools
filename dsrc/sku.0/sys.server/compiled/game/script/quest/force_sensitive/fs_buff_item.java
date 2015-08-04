package script.quest.force_sensitive;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.utils;
import script.library.player_structure;
import script.library.static_item;


public class fs_buff_item extends script.base_script
{
	public fs_buff_item()
	{
	}
	public static final String STRING_TABLE_NAME = "quest/force_sensitive/utils";
	
	public static final String VAR_TIMESTAMP = "item.time.timestamp";
	public static final String VAR_REUSE_TIME = "item.time.reuse_time";
	
	public static final String VAR_BUFF_TYPE = "item.buff.type";
	public static final String VAR_BUFF_VALUE = "item.buff.value";
	public static final String VAR_BUFF_DURATION = "item.buff.duration";
	
	public static final String[] BUFF_STRING_NAMES =
	{
		"health",
		"strength", 
		"constitution",
		"action",
		"quickness", 
		"stamina",
		"mind",
		"focus", 
		"willpower"
	};
	
	public static final int DEFAULT_REUSE_TIME = 60*60*24*3;
	
	public static final int BUFF_TYPE = 0;
	public static final int BUFF_VALUE = 1000;
	public static final int BUFF_DURATION = 60*60;
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		
		if (hasObjVar(self, "item.buff.type"))
		{
			int buffType = getIntObjVar(self, "item.buff.type");
			if (buffType == 6)
			{
				obj_id home = getContainedBy(self);
				if (!isIdValid(home))
				{
					CustomerServiceLog("FSMindCrystal: ", "Old Crystal ("+ self + ")"+ " was not deleted, because its container's id was null");
					return SCRIPT_CONTINUE;
				}
				
				obj_id newCrystal = static_item.createNewItemFunction("item_force_crystal_04_02", home);
				
				if (isIdValid(newCrystal))
				{
					CustomerServiceLog("FSMindCrystal: ", "Old Crystal ("+ self + ")"+ " is contained in player's "+ getFirstName(utils.getContainingPlayer(self)) + "("+ utils.getContainingPlayer(self) + ") inventory.");
					CustomerServiceLog("FSMindCrystal: ", "Old Crystal ("+ self + ")"+ " is to be destroyed and replaced with new Crystal "+ getStaticItemName(newCrystal) + "("+ newCrystal+ ") in "+ getFirstName(utils.getContainingPlayer(self)) + "("+ utils.getContainingPlayer(self) + ") inventory.");
					destroyObject(self);
					CustomerServiceLog("FSMindCrystal: ", "Old Crystal was successfully destroyed "+ !exists(self) + "and replaced with "+ getStaticItemName(newCrystal) + "("+ newCrystal+ ") in "+ getFirstName(utils.getContainingPlayer(self)) + "("+ utils.getContainingPlayer(self) + ") inventory.");
				}
			}
			else if (buffType == 0)
			{
				obj_id home = getContainedBy(self);
				if (!isIdValid(home))
				{
					CustomerServiceLog("FSMindCrystal: ", "Old FS VILLAGE Focused Crystal ("+ self + ")"+ " was not deleted, because its container's id was null");
					return SCRIPT_CONTINUE;
				}
				
				obj_id newCrystal = static_item.createNewItemFunction("item_force_crystal_04_01", home);
				
				if (isIdValid(newCrystal))
				{
					CustomerServiceLog("FSMindCrystal: ", "Old FS VILLAGE Focused Crystal ("+ self + ")"+ " is contained in player's "+ getFirstName(utils.getContainingPlayer(self)) + "("+ utils.getContainingPlayer(self) + ") inventory.");
					CustomerServiceLog("FSMindCrystal: ", "Old FS VILLAGE Focused Crystal ("+ self + ")"+ " is to be destroyed and replaced with new Crystal "+ getStaticItemName(newCrystal) + "("+ newCrystal+ ") in "+ getFirstName(utils.getContainingPlayer(self)) + "("+ utils.getContainingPlayer(self) + ") inventory.");
					destroyObject(self);
					CustomerServiceLog("FSMindCrystal: ", "Old FS VILLAGE Focused Crystal was successfully destroyed "+ !exists(self) + "and replaced with "+ getStaticItemName(newCrystal) + "("+ newCrystal+ ") in "+ getFirstName(utils.getContainingPlayer(self)) + "("+ utils.getContainingPlayer(self) + ") inventory.");
				}
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		
		if (hasObjVar(self, "item.buff.type"))
		{
			int buffType = getIntObjVar(self, "item.buff.type");
			if (buffType == 6)
			{
				obj_id home = getContainedBy(self);
				if (!isIdValid(home))
				{
					CustomerServiceLog("FSMindCrystal: ", "Old Crystal ("+ self + ")"+ " was not deleted, because its container's id was null");
					return SCRIPT_CONTINUE;
				}
				
				obj_id newCrystal = static_item.createNewItemFunction("item_force_crystal_04_02", home);
				
				if (isIdValid(newCrystal))
				{
					CustomerServiceLog("FSMindCrystal: ", "Old Crystal ("+ self + ")"+ " is contained in player's "+ getFirstName(utils.getContainingPlayer(self)) + "("+ utils.getContainingPlayer(self) + ") inventory.");
					CustomerServiceLog("FSMindCrystal: ", "Old Crystal ("+ self + ")"+ " is to be destroyed and replaced with new Crystal "+ getStaticItemName(newCrystal) + "("+ newCrystal+ ") in "+ getFirstName(utils.getContainingPlayer(self)) + "("+ utils.getContainingPlayer(self) + ") inventory.");
					destroyObject(self);
					CustomerServiceLog("FSMindCrystal: ", "Old Crystal was successfully destroyed "+ !exists(self) + "and replaced with "+ getStaticItemName(newCrystal) + "("+ newCrystal+ ") in "+ getFirstName(utils.getContainingPlayer(self)) + "("+ utils.getContainingPlayer(self) + ") inventory.");
				}
			}
			else if (buffType == 0)
			{
				obj_id home = getContainedBy(self);
				if (!isIdValid(home))
				{
					CustomerServiceLog("FSMindCrystal: ", "Old FS VILLAGE Focused Crystal ("+ self + ")"+ " was not deleted, because its container's id was null");
					return SCRIPT_CONTINUE;
				}
				
				obj_id newCrystal = static_item.createNewItemFunction("item_force_crystal_04_01", home);
				
				if (isIdValid(newCrystal))
				{
					CustomerServiceLog("FSMindCrystal: ", "Old FS VILLAGE Focused Crystal ("+ self + ")"+ " is contained in player's "+ getFirstName(utils.getContainingPlayer(self)) + "("+ utils.getContainingPlayer(self) + ") inventory.");
					CustomerServiceLog("FSMindCrystal: ", "Old FS VILLAGE Focused Crystal ("+ self + ")"+ " is to be destroyed and replaced with new Crystal "+ getStaticItemName(newCrystal) + "("+ newCrystal+ ") in "+ getFirstName(utils.getContainingPlayer(self)) + "("+ utils.getContainingPlayer(self) + ") inventory.");
					destroyObject(self);
					CustomerServiceLog("FSMindCrystal: ", "Old FS VILLAGE Focused Crystal was successfully destroyed "+ !exists(self) + "and replaced with "+ getStaticItemName(newCrystal) + "("+ newCrystal+ ") in "+ getFirstName(utils.getContainingPlayer(self)) + "("+ utils.getContainingPlayer(self) + ") inventory.");
				}
			}
		}
		
		mi.addRootMenu(menu_info_types.SERVER_MENU1, new string_id(STRING_TABLE_NAME, "use_special_effect"));
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		
		if (!hasObjVar (self, VAR_TIMESTAMP))
		{
			setObjVar(self, VAR_TIMESTAMP, getGameTime());
		}
		else
		{
			int timestamp = getIntObjVar(self,VAR_TIMESTAMP);
			int current_time = getGameTime();
			int time_delta = current_time - timestamp;
			if (!hasObjVar (self, VAR_REUSE_TIME))
			{
				setObjVar(self, VAR_REUSE_TIME, DEFAULT_REUSE_TIME);
			}
			
			int reuse_time = getIntObjVar(self, VAR_REUSE_TIME);
			if (time_delta < reuse_time)
			{
				sendSystemMessage(player, new string_id(STRING_TABLE_NAME, "timer_not_up"));
				return SCRIPT_CONTINUE;
			}
		}
		
		if (item == menu_info_types.SERVER_MENU1)
		{
			setObjVar(self, VAR_TIMESTAMP, getGameTime());
			
			applyBuff (player,self);
			
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
		
		names[idx] = "reuse_time";
		int timestamp = -1;
		if (hasObjVar(self,VAR_TIMESTAMP))
		{
			timestamp = getIntObjVar(self,VAR_TIMESTAMP);
		}
		int current_time = getGameTime();
		int time_delta = current_time - timestamp;
		if (!hasObjVar (self, VAR_REUSE_TIME))
		{
			setObjVar(self, VAR_REUSE_TIME, DEFAULT_REUSE_TIME);
		}
		int reuse_time = getIntObjVar(self, VAR_REUSE_TIME);
		int time_remaining = reuse_time - time_delta;
		
		if (timestamp == -1 || time_remaining <= 0)
		{
			attribs[idx] = "0";
		}
		else
		{
			attribs[idx] = player_structure.assembleTimeRemaining(player_structure.convertSecondsTime(time_remaining));
		}
		idx++;
		if (idx >= names.length)
		{
			return super.OnGetAttributes(self, player, names, attribs);
		}
		
		if (hasObjVar(self, VAR_BUFF_TYPE))
		{
			names[idx] = "examine_dot_attribute";
			attribs[idx] = BUFF_STRING_NAMES[getIntObjVar(self, VAR_BUFF_TYPE)];
			idx++;
			if (idx >= names.length)
			{
				return super.OnGetAttributes(self, player, names, attribs);
			}
		}
		
		if (hasObjVar(self, VAR_BUFF_VALUE))
		{
			names[idx] = "potency";
			attribs[idx] = "+"+getIntObjVar(self, VAR_BUFF_VALUE);
			idx++;
			if (idx >= names.length)
			{
				return super.OnGetAttributes(self, player, names, attribs);
			}
		}
		if (hasObjVar(self, VAR_BUFF_DURATION))
		{
			names[idx] = "duration";
			attribs[idx] = player_structure.assembleTimeRemaining(player_structure.convertSecondsTime(getIntObjVar(self, VAR_BUFF_DURATION)));
			idx++;
			if (idx >= names.length)
			{
				return super.OnGetAttributes(self, player, names, attribs);
			}
		}
		
		return super.OnGetAttributes(self, player, names, attribs);
	}
	
	
	public void applyBuff(obj_id player, obj_id item) throws InterruptedException
	{
		int buff_type = BUFF_TYPE;
		if (hasObjVar(item, VAR_BUFF_TYPE))
		{
			buff_type = getIntObjVar(item, VAR_BUFF_TYPE);
		}
		
		int buff_value = BUFF_VALUE;
		if (hasObjVar(item, VAR_BUFF_VALUE))
		{
			buff_value = getIntObjVar(item, VAR_BUFF_VALUE);
		}
		
		int buff_duration = BUFF_DURATION;
		if (hasObjVar(item, VAR_BUFF_DURATION))
		{
			buff_duration = getIntObjVar(item, VAR_BUFF_DURATION);
		}
		
		boolean wasBuffApplied = false;
		String buff_class = "medical_enhance_";
		String buff_type_name = BUFF_STRING_NAMES[buff_type];
		if (buff_type == 6)
		{
			buff_class = "performance_enhance_dance_";
		}
		if (buff_type == 7)
		{
			buff_class = "performance_enhance_music_";
		}
		if (buff_type == 8)
		{
			buff_class = "performance_enhance_music_";
		}
		
		if (wasBuffApplied = !hasAttribModifier(player, buff_class + buff_type_name))
		{
			addAttribModifier(player, buff_class + buff_type_name , buff_type, buff_value, buff_duration, 0.0f, 10.0f, true, false, true);
		}
		
		if (wasBuffApplied)
		{
			sendSystemMessage(player, new string_id(STRING_TABLE_NAME, "buff_applied"));
		}
		else
		{
			sendSystemMessage(player, new string_id(STRING_TABLE_NAME, "have_buff"));
			removeObjVar (item, VAR_TIMESTAMP);
		}
	}
}
