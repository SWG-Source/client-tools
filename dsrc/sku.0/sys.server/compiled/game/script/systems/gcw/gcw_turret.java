package script.systems.gcw;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.buff;
import script.library.chat;
import script.library.create;
import script.library.factions;
import script.library.gcw;
import script.library.groundquests;
import script.library.restuss_event;
import script.library.resource;
import script.library.static_item;
import script.library.sui;
import script.library.stealth;
import script.library.trial;
import script.library.utils;


public class gcw_turret extends script.base_script
{
	public gcw_turret()
	{
	}
	public static final string_id SID_MNU_REPAIR = new string_id("sui", "mnu_repair");
	public static final string_id SID_MNU_DEFEND = new string_id("sui", "mnu_defend");
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		setObjVar(self, gcw.GCW_TOOL_TEMPLATE_OBJVAR, "object/tangible/gcw/crafting_quest/gcw_turret_tool.iff");
		setObjVar(self, "questCallBack", 1);
		
		dictionary params = new dictionary();
		location loc = getLocation(self);
		params.put("particleLoc", loc);
		int playIconTime = getGameTime();
		params.put("iconMessageTime", playIconTime);
		utils.setScriptVar(self, "iconMessageTime", playIconTime);
		messageTo(self, "playQuestIcon", params, 1.0f, false);
		return SCRIPT_CONTINUE;
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
		
		int displayPower = utils.getIntScriptVar(self, "gcw.displayPower");
		
		if (displayPower > 0 && displayPower < 4)
		{
			String[] powerLevels =
			{
				"Low Firepower", "Normal Firepower", "Maximum Firepower"
			};
			
			names[idx] = "turret_level";
			attribs[idx] = powerLevels[displayPower - 1];
			idx++;
		}
		
		names[idx] = "object_repaired";
		int repairCount = getIntObjVar(self, gcw.GCW_OBJECT_REPAIR_COUNT);
		
		if (repairCount > 0)
		{
			attribs[idx] = ""+repairCount+" out of "+gcw.REPAIR_COUNT_MAX;
		}
		else
		{
			attribs[idx] = "Never Repaired";
		}
		idx++;
		
		return super.OnGetAttributes(self, player, names, attribs);
	}
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		if (!utils.hasScriptVar(self, "faction"))
		{
			LOG("gcw_patrol_point","no faction on turret obj");
			return SCRIPT_CONTINUE;
		}
		
		if (!factions.isPlayerSameGcwFactionAsSchedulerObject(player, self))
		{
			LOG("gcw_patrol_point","faction invalid on turret obj");
			return SCRIPT_CONTINUE;
		}
		else
		{
			if (utils.isProfession(player, utils.TRADER))
			{
				LOG("gcw_patrol_point","player is trader");
				
				if (!gcw.canGcwObjectBeRepaired(self))
				{
					LOG("gcw_patrol_point","OnObjectMenuRequest no repair needed");
					
					sendSystemMessage(player, gcw.SID_DOESNT_NEED_REPAIR);
					return SCRIPT_CONTINUE;
				}
				if (!gcw.hasConstructionOrRepairTool(player, self))
				{
					LOG("gcw_patrol_point","OnObjectMenuRequest no resources");
					
					gcw.playerSystemMessageResourceNeeded(player, self, false);
					return SCRIPT_CONTINUE;
				}
				mi.addRootMenu(menu_info_types.SERVER_MENU1, SID_MNU_REPAIR);
			}
			else
			{
				mi.addRootMenu(menu_info_types.SERVER_MENU2, SID_MNU_DEFEND);
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		LOG("gcw_patrol_point","OnObjectMenuSelect");
		
		if (!utils.hasScriptVar(self, "faction"))
		{
			LOG("gcw_patrol_point","no faction on patrol obj");
			return SCRIPT_CONTINUE;
		}
		
		int faction = utils.getIntScriptVar(self, "faction");
		if (faction < 0)
		{
			LOG("gcw_patrol_point","faction invalid on patrol obj");
			return SCRIPT_CONTINUE;
		}
		
		if ((item != menu_info_types.SERVER_MENU1 && item != menu_info_types.SERVER_MENU2))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!factions.isPlayerSameGcwFactionAsSchedulerObject(player, self))
		{
			LOG("gcw_patrol_point","Wrong Faction");
			return SCRIPT_CONTINUE;
		}
		else
		{
			if (item == menu_info_types.SERVER_MENU1)
			{
				if (utils.isProfession(player, utils.TRADER))
				{
					LOG("gcw_patrol_point","player is trader");
					
					if (!gcw.canGcwObjectBeRepaired(self))
					{
						sendSystemMessage(player, gcw.SID_DOESNT_NEED_REPAIR);
						return SCRIPT_CONTINUE;
					}
					if (!gcw.hasConstructionOrRepairTool(player, self))
					{
						gcw.playerSystemMessageResourceNeeded(player, self, false);
						return SCRIPT_CONTINUE;
					}
					
					if (groundquests.isQuestActive(player, gcw.GCW_REPAIR_TURRET_QUEST))
					{
						if (gcw.useGcwObjectForQuest(player, self, gcw.GCW_REPAIR_TURRET_QUEST))
						{
							gcw.useConstructionOrRepairTool(player, self);
						}
						return SCRIPT_CONTINUE;
					}
					
					if (groundquests.hasCompletedQuest(player, gcw.GCW_REPAIR_TURRET_QUEST))
					{
						groundquests.clearQuest(player, gcw.GCW_REPAIR_TURRET_QUEST);
					}
					if (!groundquests.isQuestActive(player, gcw.GCW_REPAIR_TURRET_QUEST))
					{
						groundquests.grantQuest(player, gcw.GCW_REPAIR_TURRET_QUEST);
					}
				}
			}
			else
			{
				LOG("gcw_patrol_point","OnObjectMenuSelect server menu 2");
				
				String questName = gcw.GCW_DEFEND_TURRET_REBEL;
				if (faction == factions.FACTION_FLAG_IMPERIAL)
				{
					questName = gcw.GCW_DEFEND_TURRET_IMPERIAL;
				}
				
				if (groundquests.isQuestActive(player, questName))
				{
					return SCRIPT_CONTINUE;
				}
				
				if (groundquests.hasCompletedQuest(player, questName))
				{
					groundquests.clearQuest(player, questName);
				}
				if (!groundquests.isQuestActive(player, questName))
				{
					setObjVar(player,"defend_target", self);
					groundquests.grantQuest(player, questName);
				}
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnDestroy(obj_id self) throws InterruptedException
	{
		int faction = utils.getIntScriptVar(self, "faction");
		if (faction < 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (faction == factions.FACTION_FLAG_IMPERIAL)
		{
			gcw.signalAllParticipantsForDamage(self, gcw.GCW_DESTROY_TURRET, gcw.GCW_IMP_DESTROY_TURRET_TASK, gcw.GCW_IMP_DESTROY_TURRET_SIGNAL);
		}
		else if (faction == factions.FACTION_FLAG_REBEL)
		{
			gcw.signalAllParticipantsForDamage(self, gcw.GCW_DESTROY_TURRET, gcw.GCW_REB_DESTROY_TURRET_TASK, gcw.GCW_REB_DESTROY_TURRET_SIGNAL);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int playQuestIcon(obj_id self, dictionary params) throws InterruptedException
	{
		params.put("offset", 2.0f);
		
		gcw.playQuestIconHandler(self, params);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleQuestCallBack(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!params.containsKey("questComplete"))
		{
			return SCRIPT_CONTINUE;
		}
		if (!params.containsKey("player"))
		{
			return SCRIPT_CONTINUE;
		}
		if (params.getBoolean("questComplete"))
		{
			obj_id player = params.getObjId("player");
			if (!isValidId(player))
			{
				return SCRIPT_CONTINUE;
			}
			
			trial.addNonInstanceFactionParticipant(player, self);
		}
		return SCRIPT_CONTINUE;
	}
	
}
