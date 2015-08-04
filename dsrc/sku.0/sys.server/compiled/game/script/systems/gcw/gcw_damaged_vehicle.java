package script.systems.gcw;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.create;
import script.library.factions;
import script.library.gcw;
import script.library.groundquests;
import script.library.resource;
import script.library.utils;


public class gcw_damaged_vehicle extends script.base_script
{
	public gcw_damaged_vehicle()
	{
	}
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		messageTo(self, "handleSetup", null, 1.0f, false);
		setObjVar(self, gcw.GCW_TOOL_TEMPLATE_OBJVAR, "object/tangible/gcw/crafting_quest/gcw_vehicle_tool.iff");
		return SCRIPT_CONTINUE;
	}
	
	
	public void updateName(obj_id self) throws InterruptedException
	{
		String name = utils.getStringScriptVar(self, "gcw.name");
		
		int questsCompleted = getIntObjVar(self, gcw.GCW_OBJECT_REPAIR_COUNT);
		
		String suffix = "";
		
		if (questsCompleted >= 4)
		{
			suffix = " (100% Repaired)";
		}
		else if (questsCompleted <= 0)
		{
			suffix = " (0% Repaired)";
		}
		else
		{
			suffix = " ("+ (int)(((float)questsCompleted / 4.0f) * 100.0f) + "% Repaired)";
		}
		
		setName(self, name + suffix);
	}
	
	
	public int OnGetAttributes(obj_id self, obj_id player, String[] names, String[] attribs) throws InterruptedException
	{
		if (!exists(self))
		{
			return super.OnGetAttributes(self, player, names, attribs);
		}
		
		int idx = utils.getValidAttributeIndex(names);
		
		if (idx == -1)
		{
			return super.OnGetAttributes(self, player, names, attribs);
		}
		
		names[idx] = "object_repaired";
		int repairCount = getIntObjVar(self, gcw.GCW_OBJECT_REPAIR_COUNT);
		
		if (repairCount > 0)
		{
			attribs[idx] = "" + repairCount + " out of 4";
		}
		else
		{
			attribs[idx] = "Never Repaired";
		}
		idx++;
		
		return super.OnGetAttributes(self, player, names, attribs);
	}
	
	
	public int handleSetup(obj_id self, dictionary params) throws InterruptedException
	{
		gcw.playQuestIconParticle(self);
		
		utils.setObjVar(self, gcw.GCW_OBJECT_REPAIR_COUNT, 0);
		
		setPosture(self, POSTURE_KNOCKED_DOWN);
		setCreatureStatic(self, true);
		updateName(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int playQuestIcon(obj_id self, dictionary params) throws InterruptedException
	{
		int repairs = getIntObjVar(self, gcw.GCW_OBJECT_REPAIR_COUNT);
		
		if (repairs < 4)
		{
			gcw.playQuestIconHandler(self, params);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		LOG("gcw_damaged_vehicle", "OnObjectMenuRequest");
		
		if (!utils.hasScriptVar(self, "faction"))
		{
			LOG("gcw_damaged_vehicle","no faction on turret obj");
			return SCRIPT_CONTINUE;
		}
		
		if (!factions.isPlayerSameGcwFactionAsSchedulerObject(player, self))
		{
			LOG("gcw_damaged_vehicle","faction invalid on turret obj");
			return SCRIPT_CONTINUE;
		}
		else
		{
			if (utils.isProfession(player, utils.TRADER))
			{
				LOG("gcw_damaged_vehicle","player is trader");
				
				int repairs = getIntObjVar(self, gcw.GCW_OBJECT_REPAIR_COUNT);
				
				if (repairs > 3)
				{
					LOG("gcw_damaged_vehicle","OnObjectMenuRequest no repair needed");
					
					sendSystemMessage(player, gcw.SID_DOESNT_NEED_REPAIR);
					return SCRIPT_CONTINUE;
				}
				
				if (!gcw.hasConstructionOrRepairTool(player, self))
				{
					LOG("gcw_damaged_vehicle","OnObjectMenuRequest no resources");
					
					gcw.playerSystemMessageResourceNeeded(player, self, false);
					return SCRIPT_CONTINUE;
				}
				
				menu_info_data data = mi.getMenuItemByType(menu_info_types.ITEM_USE);
				
				int mnu = mi.addRootMenu(menu_info_types.ITEM_USE, new string_id("",""));
				
				if (data != null)
				{
					data.setServerNotify(true);
				}
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		LOG("gcw_damaged_vehicle", "OnObjectMenuSelect");
		
		if (!utils.hasScriptVar(self, "faction"))
		{
			LOG("gcw_damaged_vehicle","no faction on patrol obj");
			return SCRIPT_CONTINUE;
		}
		
		int faction = utils.getIntScriptVar(self, "faction");
		
		if (faction < 0)
		{
			LOG("gcw_damaged_vehicle","faction invalid on patrol obj");
			return SCRIPT_CONTINUE;
		}
		
		if (!factions.isPlayerSameGcwFactionAsSchedulerObject(player, self))
		{
			LOG("gcw_damaged_vehicle","Wrong Faction");
			return SCRIPT_CONTINUE;
		}
		else
		{
			if (utils.isProfession(player, utils.TRADER))
			{
				LOG("gcw_damaged_vehicle","player is trader");
				
				int repairs = getIntObjVar(self, gcw.GCW_OBJECT_REPAIR_COUNT);
				
				if (repairs > 3)
				{
					sendSystemMessage(player, gcw.SID_DOESNT_NEED_REPAIR);
					return SCRIPT_CONTINUE;
				}
				
				if (!gcw.hasConstructionOrRepairTool(player, self))
				{
					gcw.playerSystemMessageResourceNeeded(player, self, false);
					return SCRIPT_CONTINUE;
				}
				
				if (groundquests.isQuestActive(player, gcw.GCW_REPAIR_DAMAGED_VEHICLE_QUEST))
				{
					if (gcw.useGcwObjectForQuest(player, self, gcw.GCW_REPAIR_DAMAGED_VEHICLE_QUEST))
					{
						gcw.useConstructionOrRepairTool(player, self);
					}
					
					return SCRIPT_CONTINUE;
				}
				
				if (groundquests.hasCompletedQuest(player, gcw.GCW_REPAIR_DAMAGED_VEHICLE_QUEST))
				{
					groundquests.clearQuest(player, gcw.GCW_REPAIR_DAMAGED_VEHICLE_QUEST);
				}
				if (!groundquests.isQuestActive(player, gcw.GCW_REPAIR_DAMAGED_VEHICLE_QUEST))
				{
					groundquests.grantQuest(player, gcw.GCW_REPAIR_DAMAGED_VEHICLE_QUEST);
				}
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int repairComplete(obj_id self, dictionary params) throws InterruptedException
	{
		int repairs = getIntObjVar(self, gcw.GCW_OBJECT_REPAIR_COUNT);
		
		updateName(self);
		
		if (repairs > 3)
		{
			obj_id creator = utils.getObjIdScriptVar(self, "creator");
			
			if (isIdValid(creator))
			{
				messageTo(creator, "createRepairedVehicle", null, 0.0f, false);
			}
			
			destroyObject(self);
		}
		
		return SCRIPT_CONTINUE;
	}
}
