package script.systems.gcw;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.buff;
import script.library.create;
import script.library.factions;
import script.library.gcw;
import script.library.groundquests;
import script.library.resource;
import script.library.restuss_event;
import script.library.static_item;
import script.library.trial;
import script.library.utils;


public class gcw_vehicle_patrol extends script.base_script
{
	public gcw_vehicle_patrol()
	{
	}
	public static final String[] lowImperials = 
	{
		"gcw_city_at_st_1"
	};
	
	public static final String[] normalImperials = 
	{
		"gcw_city_at_st_2"
	};
	
	public static final String[] eliteImperials = 
	{
		"gcw_city_at_st_3"
	};
	
	public static final String[] lowRebels = 
	{
		"gcw_city_at_xt_1"
	};
	
	public static final String[] normalRebels = 
	{
		"gcw_city_at_xt_2"
	};
	
	public static final String[] eliteRebels = 
	{
		"gcw_city_at_xt_3"
	};
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		setObjVar(self, gcw.GCW_TOOL_TEMPLATE_OBJVAR, "object/tangible/gcw/crafting_quest/gcw_vehicle_tool.iff");
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		messageTo(self, "handleGCWPatrol", null, 120, false);
		
		return SCRIPT_CONTINUE;
	};
	
	
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
		if (hasObjVar(self, gcw.GCW_OBJECT_REPAIR_COUNT))
		{
			int repairCount = getIntObjVar(self, gcw.GCW_OBJECT_REPAIR_COUNT);
			if (repairCount > 0)
			{
				attribs[idx] = ""+repairCount;
				idx++;
				return super.OnGetAttributes(self, player, names, attribs);
			}
		}
		
		attribs[idx] = "Never Repaired";
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
				menu_info_data data = mi.getMenuItemByType(menu_info_types.ITEM_USE);
				
				if (data != null)
				{
					data.setServerNotify (true);
				}
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
		
		if (item != menu_info_types.ITEM_USE)
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
				LOG("gcw_patrol_point","player can repair");
				if (groundquests.isQuestActive(player, gcw.GCW_REPAIR_VEHICLE_PATROL_QUEST))
				{
					if (gcw.useGcwObjectForQuest(player, self, gcw.GCW_REPAIR_VEHICLE_PATROL_QUEST))
					{
						gcw.useConstructionOrRepairTool(player, self);
					}
					return SCRIPT_CONTINUE;
				}
				
				if (groundquests.hasCompletedQuest(player, gcw.GCW_REPAIR_VEHICLE_PATROL_QUEST))
				{
					groundquests.clearQuest(player, gcw.GCW_REPAIR_VEHICLE_PATROL_QUEST);
				}
				if (!groundquests.isQuestActive(player, gcw.GCW_REPAIR_VEHICLE_PATROL_QUEST))
				{
					groundquests.grantQuest(player, gcw.GCW_REPAIR_VEHICLE_PATROL_QUEST);
				}
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int getConstructionQuestsCompleted(obj_id pylon) throws InterruptedException
	{
		int completed = 0;
		
		if (!isIdValid(pylon) || !exists(pylon))
		{
			return 0;
		}
		
		if (utils.hasScriptVar(pylon, "gcw.constructionQuestsCompleted"))
		{
			completed = utils.getIntScriptVar(pylon, "gcw.constructionQuestsCompleted");
		}
		
		return completed;
	}
	
	
	public int handleGCWPatrol(obj_id self, dictionary params) throws InterruptedException
	{
		int faction = -1;
		
		if (utils.hasScriptVar(self, "faction"))
		{
			faction = utils.getIntScriptVar(self, "faction");
		}
		
		obj_id kit = utils.getObjIdScriptVar(self, "creator");
		
		if (!isIdValid(kit) || !exists(kit))
		{
			return SCRIPT_CONTINUE;
		}
		
		int patrolType = getIntObjVar(kit, "patrolType");
		int construction = getConstructionQuestsCompleted(kit);
		
		String npcName = "";
		
		if (construction < 1)
		{
			return SCRIPT_CONTINUE;
		}
		else if (construction < gcw.GCW_CONSTRUCTION_MAXIMUM / 2)
		{
			if (faction == factions.FACTION_FLAG_REBEL)
			{
				npcName = lowRebels[rand(0, lowRebels.length - 1)];
			}
			else if (faction == factions.FACTION_FLAG_IMPERIAL)
			{
				npcName = lowImperials[rand(0, lowImperials.length - 1)];
			}
		}
		else if (construction < gcw.GCW_CONSTRUCTION_MAXIMUM)
		{
			if (faction == factions.FACTION_FLAG_REBEL)
			{
				npcName = normalRebels[rand(0, normalRebels.length - 1)];
			}
			else if (faction == factions.FACTION_FLAG_IMPERIAL)
			{
				npcName = normalImperials[rand(0, normalImperials.length - 1)];
			}
		}
		else if (construction >= gcw.GCW_CONSTRUCTION_MAXIMUM)
		{
			if (faction == factions.FACTION_FLAG_REBEL)
			{
				npcName = eliteRebels[rand(0, eliteRebels.length - 1)];
			}
			else if (faction == factions.FACTION_FLAG_IMPERIAL)
			{
				npcName = eliteImperials[rand(0, eliteImperials.length - 1)];
			}
		}
		
		if (getSchedulerNPCs(kit, "gcwPatrol") < 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = null;
		
		if (patrolType == 0 && getSchedulerNPCs(kit, "gcwPatrol") < 500)
		{
			obj_id currentPatrol = utils.getObjIdScriptVar(kit, "currentPatrol");
			
			if (!isIdValid(currentPatrol) || !exists(currentPatrol) || isIncapacitated(currentPatrol) || isDead(currentPatrol))
			{
				npc = createSchedulerNPC(kit, npcName);
				
				utils.setScriptVar(kit, "currentPatrol", npc);
			}
		}
		
		if (patrolType == 1 && getSchedulerNPCs(kit, "gcwPatrol") < 500)
		{
			npc = createSchedulerNPC(kit, npcName);
		}
		
		if (isIdValid(npc) && exists(npc))
		{
			detachScript(npc, "systems.vehicle_system.vehicle_base");
			detachScript(npc, "systems.vehicle_system.vehicle_ping");
		}
		
		messageTo(self, "handleGCWPatrol", null, 240.0f + rand(0, 30), false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int getSchedulerNPCs(obj_id kit, String npcID) throws InterruptedException
	{
		if (!isIdValid(kit) || !exists(kit))
		{
			return -1;
		}
		
		obj_id kitParent = trial.getParent(kit);
		
		if (!isIdValid(kitParent) || !exists(kitParent))
		{
			return -1;
		}
		
		obj_id[] npcs = trial.getObjectsInInstanceBySpawnId(trial.getParent(kit), npcID);
		
		if (npcs == null)
		{
			return 0;
		}
		
		return npcs.length;
	}
	
	
	public obj_id createSchedulerNPC(obj_id kit, String npcName) throws InterruptedException
	{
		
		if (!isIdValid(kit) || !exists(kit))
		{
			return null;
		}
		
		obj_id kitParent = trial.getParent(kit);
		
		if (!isIdValid(kitParent) || !exists(kitParent))
		{
			return null;
		}
		
		location loc = getLocation(kit);
		
		obj_id npc = create.object(npcName, loc);
		
		if (!isIdValid(npc) || !exists(npc))
		{
			return null;
		}
		
		trial.markAsTempObject(npc, true);
		trial.setParent(kitParent, npc, true);
		trial.setInterest(npc);
		
		setObjVar(npc, "spawn_id", "gcwPatrol");
		
		trial.storeSpawnedChild(kitParent, npc, "gcwPatrol");
		
		String patrol = getStringObjVar(kit, "patrolPoint");
		
		if (patrol != null && patrol.length() > 0)
		{
			dictionary path_data = utils.hasScriptVar(kitParent, trial.SEQUENCER_PATH_DATA) ? utils.getDictionaryScriptVar(kitParent, trial.SEQUENCER_PATH_DATA) : null;
			
			if (path_data != null && !path_data.isEmpty())
			{
				utils.setScriptVar(npc, trial.SEQUENCER_PATH_DATA, utils.getDictionaryScriptVar(kitParent, trial.SEQUENCER_PATH_DATA));
			}
			
			setObjVar(npc, "patrol_path", patrol);
			setHibernationDelay(npc, 3600.0f);
		}
		
		attachScript(npc, "systems.dungeon_sequencer.ai_controller");
		
		return npc;
	}
	
	
	public int destroyGCWPatrol(obj_id self, dictionary params) throws InterruptedException
	{
		trial.cleanupObject(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnDeath(obj_id self, obj_id killer, obj_id corpseId) throws InterruptedException
	{
		handleDestroyPatrol(self, killer);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectDisabled(obj_id self, obj_id killer) throws InterruptedException
	{
		handleDestroyPatrol(self, killer);
		return SCRIPT_CONTINUE;
	}
	
	
	public void handleDestroyPatrol(obj_id self, obj_id killer) throws InterruptedException
	{
		location death = getLocation( self );
		playClientEffectObj(killer, "clienteffect/combat_explosion_lair_large.cef", self, "");
		playClientEffectLoc(killer, "clienteffect/combat_explosion_lair_large.cef", death, 0);
		
		setInvulnerable(self, true);
		messageTo(self, "destroyGCWPatrol", null, 1f, false);
		
		return;
	}
}
